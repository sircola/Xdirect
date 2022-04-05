
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>

#include <xlib.h>
#include <xlua.h>

// #include <DebugHlp.h>


RCSID( "$Id: luashell.cpp,v 1.1.1.1 2011/03/17 22:00:25 bernie Exp $" )


#define LUADLLNAME "lua5.1.dll"
// #define LUADLLNAME "lua52.dll"

static HINSTANCE hDll = NULL;


static lua_State *Lua_VM = NULL;
static char *current_lua_file = NULL;		// XMAX_PATH * 2
static char *current_lua_string = NULL;

lua_All_functions LuaFunctions;


static int last_return = 0;
static BOOL screen_activity = FALSE;

#define BUFSIZE (16*1024)



//
//
//
static void printError( char *funcname = NULL ) {

	if( last_return == 0 )
		return;

	char error_str[BUFSIZE];
	strcpy(error_str, lua_tostring(Lua_VM, -1) );
	lua_pop(Lua_VM, 1); // remove error message
	xprintf("%s error: (rc = %d)\n", funcname?funcname:"Lua", last_return );
	if( !strnicmp( error_str, "[string \"", 9) ) {
		// beteszi a filenevet, hogy jobban azonosíthító legyen
		char *p;
		if( (p=strstr(&error_str[9], "\"]:")) != NULL ) {
			char temp[BUFSIZE];
			strcpy( temp, GetFilenameNoPath(current_lua_file) );
			strcat( temp, &p[2] );
			strcpy( error_str, temp );
		}
	}
	char *p = error_str;
	char *s = error_str;
	char *e = &error_str[ strlen(error_str) ];
	while( p <= e ) {
		while( *p != 10 && *p != 13 && *p != 0 )
			++p;
		if( (p[0] == 10 && p[1] == 13) || (p[0] == 13 && p[1] == 10) ) {
			p[0] = 0;
			p[1] = 0;
			p += 2;
		}
		*p = 0;
		xprintf("%s\n", s );
		++p;
		s = p;
	}

	return;
}



//
//
//
BOOL LUA_CheckGlobal( char *name ) {

	if( Lua_VM == NULL ) {
		xprintf("LUA_CheckGlobal: Lua not inited.\n" );
		return FALSE;
	}

	lua_getglobal(Lua_VM,name);

	if( lua_isnil(Lua_VM, -1) ) {
		// xprintf(l,"error running init: %s",lua_tostring(l,-1));
		xprintf("LUA_CheckGlobal: missing \"%s\" global variable in \"%s\" file!\n", name, current_lua_file?current_lua_file:"(null)" );
		return FALSE;
	}

	lua_pop(Lua_VM,1);

	return TRUE;
}


//
//
//
FLOAT LUA_GetNumber( char *name ) {

	if( Lua_VM == NULL ) {
		xprintf("LUA_GetNumber: Lua not inited.\n" );
		return 0.0f;
	}

	lua_getglobal(Lua_VM,name);

	if( !lua_isnumber(Lua_VM, -1) )
		; // xprintf("LUA_GetNumber: \"%s\" is not a number.\n", name );

	FLOAT d = (FLOAT)lua_tonumber(Lua_VM,-1); /* extracts the value, leaves stack unchanged */

	lua_pop(Lua_VM,1);          /* pop the value to leave stack balanced */

	return d;
}



//
//
//
BOOL LUA_GetString( char *name, char *s ) {

	if( Lua_VM == NULL ) {
		xprintf("LUA_GetString: Lua not inited.\n" );
		return FALSE;
	}

	lua_getglobal(Lua_VM,name);

	if(!lua_isnil(Lua_VM,-1) )
		strcpy(s, lua_tostring(Lua_VM,-1));
	else
		xprintf("LUA_GetString: \"%s\" is not a string.\n", name );

	lua_pop(Lua_VM,1);

	return TRUE;
}


//
//
//
char *LUA_GetResultString( void ) {

	if( Lua_VM == NULL )
		return NULL;

	if( !lua_isnil(Lua_VM,-1) )
		return (char *)lua_tostring(Lua_VM,-1);

	return NULL;
}


//
//
//
FLOAT LUA_GetResultNumber( void ) {

	if( Lua_VM == NULL )
		return 0;

	if( !lua_isnumber(Lua_VM, -1) )
		return (FLOAT)lua_tonumber(Lua_VM,-1);

	return 0;
}



//
// MsgBox( "text", ["titel"] )
//
static int MsgBox_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "MsgBox_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *text = (char *)lua_tostring(L, 1);
	if( text == NULL ) {
		lua_pushstring(L, "MsgBox_wrapper: need a string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *title = NULL;
	if( n>=2 && lua_isstring(L, 2) )
		title = (char *)lua_tostring(L, 2);

	if( strlen(text) < 128 )
		XLIB_Message( text, title?title:(char*)"Lua" );
	else {
		winTextBox( title?title:(char*)"Lua", text );
		// FIXME: amig Browser addig kell
		screen_activity = TRUE;
	}

	return 0;
}



//
// AskYesNo( "text", ["titel"] )
//
static int AskYesNo_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "AskYesNo_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *text = (char *)lua_tostring(L, 1);
	if( text == NULL ) {
		lua_pushstring(L, "AskYesNo_wrapper: need a string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *title = NULL;
	if( n>=2 && lua_isstring(L, 2) )
		title = (char *)lua_tostring(L, 2);

	BOOL res = winAskYesNo( title?title:(char*)"Lua",text );
	if( res )
		lua_pushboolean(L, TRUE);
	else
		lua_pushboolean(L, FALSE);


	return 1;
}





void luaTypeToString( lua_State *L, int index, int type, char *out, int size );

//
// ListBox( "text1", ["text2","text3"] )
// ListBox( "title", {"text2","text3"} ) -- with title
//
static int ListBox_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "ListBox_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *ptr;
	char title[XLIB_WLBMAXSTRLEN] = "Lua:";
	int tableIndex = 2;

	if( n>=2 && lua_isstring(L,1) && lua_istable(L,tableIndex) ) {

		char *text = (char *)lua_tostring(L, 1);
		if( text )
			strcpy( title, text );

		n = lua_objlen(L,tableIndex);

		ALLOCMEM( ptr, XLIB_WLBMAXSTRLEN * n );

		lua_pushnil(L);

		int i = 0;
		while( lua_next(L, tableIndex) != 0 ) {

			int type = lua_type(L, -1);
			if( type == LUA_TTABLE )
				--n; // writeLuaTable( L, lua_gettop(L), f );
			else {
				if( lua_type(L, -2) == LUA_TSTRING )
					const char *keyString = lua_tostring(L, -2);

				luaTypeToString( L, -1, type, &ptr[XLIB_WLBMAXSTRLEN * i++], XLIB_WLBMAXSTRLEN );
			}

			lua_pop(L, 1);
		}
	}
	else {
		ALLOCMEM( ptr, XLIB_WLBMAXSTRLEN * n )

		for( int i=1; i<=n; i++ ) {
			char *text = (char *)lua_tostring(L, i);
			if( text )
				strncpy( &ptr[XLIB_WLBMAXSTRLEN*(i-1)], text, XLIB_WLBMAXSTRLEN );
			else
				ptr[XLIB_WLBMAXSTRLEN*(i-1)] = 0;
		}
	}

	lua_pushnumber(L, winListBox( ptr, n, title ) );

	FREEMEM( ptr );

	return 1;
}



//
// Wget( "http://index.hu" )
//
static int Wget_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "Wget_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *addr = (char *)lua_tostring(L, 1);
	if( addr == NULL ) {
		lua_pushstring(L, "Wget_wrapper: need a string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// stringbe kéri-e?
	BOOL bString = FALSE;
	char *type_string = (char *)lua_tostring(L, 2);
	if( type_string != NULL )
		if( !stricmp(type_string,"string") )
			bString = TRUE;

	winShowPanel(addr, "Downloading");

	UCHAR *buf = NULL;

	// char *ie6_agent = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; WOW64; SV1)";
	// char *ff13_agent = "Mozilla/5.0 (Windows NT 5.2; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1";
	// char *linux_agent = "Mozilla/5.0 (X11; Linux x86_64; rv:7.0.1) Gecko/20100101 Firefox/7.0.1";
	// char *ipad_agent = "Mozilla/5.0 (iPad; U; CPU OS 3_2 like Mac OS X; en-us) AppleWebKit/531.21.10 (KHTML, like Gecko) Version/4.0.4 Mobile/7B334b Safari/531.21.102011-10-16 20:23:50";
	char *ie8_agent = "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.2; WOW64; Trident/4.0; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; .NET CLR 1.1.4322)";

	char *agent = ie8_agent;

	winInetAgent( agent );
	// XLIB_SetCurlAgent( agent );

	int size = XLIB_winGetUrlDocument( addr, (UCHAR **)&buf );
	// int size = XLIB_GetCurlURL( addr, (UCHAR **)&buf );

	winHidePanel();

	if( size <= 0 || buf == NULL ) {
		char err[256];
		sprintf(err,"Wget_wrapper: %s", winInetErrorStr(NULL) );
		lua_pushstring(L, err );
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( bString ) {

		if( (buf = (UCHAR *)realloc( buf, size+1)) != NULL ) {
			buf[size] = 0;
			lua_pushstring(L, (const char*)buf);
		}
		else
			lua_pushstring(L, (const char*)"(out of memory)");
	}
	else {
		// return as table
		lua_newtable(L);
		int top = lua_gettop(L);
		int index = 1;

		for( int index = 1; index <= size; index++ )  {

			// key
			lua_pushinteger(L,index);

			// value
			lua_pushinteger(L,buf[index-1]);

			// set the table entry
			lua_settable(L, -3);
		}
		// push the new table
		lua_pushvalue(L,-1);
	}

	if( buf ) free( buf );

	return 1;
}





//
// LoadFile( "valami.bin", ["string" | "text" | "t"] )
//
static int LoadFile_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "LoadFile_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *filename = (char *)lua_tostring(L, 1);
	if( filename == NULL ) {
		lua_pushstring(L, "LoadFile_wrapper: need a string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// stringbe kéri-e?
	BOOL bString = FALSE;
	{
	char *str = (char *)lua_tostring(L, 2);
	if( str != NULL )
		if( !stricmp(str,"string") || !stricmp(str,"text") || !stricmp(str,"t") )
			bString = TRUE;
	}

	UCHAR *buf = NULL;

	int size = LoadFile( filename, PTR(buf) );

	if( size <= 0 || buf == NULL ) {
		lua_pushstring(L, "LoadFile_wrapper: can't load file");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( bString ) {
		char *p;
		ALLOCMEM( p, size+1 );
		memcpy(p,buf,size);
		p[size] = 0;
		lua_pushstring(L, (const char*)p);
		FREEMEM(p);
	}
	else {
		// return as table
		lua_newtable(L);
		int top = lua_gettop(L);
		int index = 1;

		for( int index = 1; index <= size; index++ )  {

			// key
			lua_pushinteger(L,index);

			// value
			lua_pushinteger(L,buf[index-1]);

			// set the table entry
			lua_settable(L, -3);
		}

		// push the new table
		lua_pushvalue(L,-1);
	}

	SAFE_FREEMEM( buf );

	return 1;
}



//
// FileExist( "valami.bin", [true | false] )
//
static int FileExist_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "FileExist_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *filename = (char *)lua_tostring(L, 1);
	if( filename == NULL ) {
		lua_pushstring(L, "FileExist_wrapper: need a string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( FileExist(filename) == TRUE )
		lua_pushboolean(L, TRUE);
	else
		lua_pushboolean(L, FALSE);

	return 1;
}



//
//
//
static int SetFont_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "SetFont_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( n >= 1 ) {
		const char *str = lua_tostring(L, 1);
		if( str == NULL ) {
			lua_pushstring(L, "SetFont_wrapper: can't get font name");
			lua_error(L);
		}
		else
			SETFONT((char *)str);
	}

	int r=255,g=255,b=255,a=255;
	BOOL haveRGB = FALSE;

	// red
	if( lua_isnumber(L, 2) ) {
		double val = lua_tonumber(L, 2);
		r = ftoi(val);
		haveRGB = TRUE;
	}
	else {
		// lua_pushstring(L, "SetFont_wrapper: can't get Red");
		// lua_error(L);
	}

	// green
	if( lua_isnumber(L, 3) ) {
		double val = lua_tonumber(L, 3);
		g = ftoi(val);
		haveRGB = TRUE;
	}

	// blue
	if( lua_isnumber(L, 4) ) {
		double val = lua_tonumber(L, 4);
		b = ftoi(val);
		haveRGB = TRUE;
	}

	// alpha
	if( lua_isnumber(L, 5) ) {
		double val = lua_tonumber(L, 5);
		a = ftoi(val);
		haveRGB = TRUE;
	}

	if( haveRGB )
		SetFontAlpha( FA_ALL, r,g,b, a );

	lua_pushnumber(L, 1);

	return 1;
}





//
//
//
static int GetString_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		// lua_pushstring(L, "WriteText_wrapper: not enough arguments");
		lua_pushnil(L);
		return 1;
	}

	// id
	if( !lua_isnumber(L, 1) ) {
		// lua_pushstring(L, "WriteText_wrapper: X should be number");
		lua_pushnil(L);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int id = ftoi(val);

	lua_pushstring(L, GetString(id));

	return 1;
}




//
// PutSpritePart( 0,0, "valami.jpg", sx,sy, sw,sh )
//
static int PutSpritePart_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 3 ) {
		lua_pushstring(L, "PutSpritePart_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// X
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "PutSpritePart_wrapper: X should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int screenx = ftoi(val);

	// Y
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "PutSpritePart_wrapper: Y should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 2);
	int screeny = ftoi(val);

	// str
	const char *str = lua_tostring(L, 3);
	if( str == NULL ) {
		lua_pushstring(L, "PutSpritePart_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// sprite X
	if( !lua_isnumber(L, 4) ) {
		lua_pushstring(L, "PutSpritePart_wrapper: SX should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 4);
	int sx = ftoi(val);

	// sprite Y
	if( !lua_isnumber(L, 5) ) {
		lua_pushstring(L, "PutSpritePart_wrapper: SY should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 5);
	int sy = ftoi(val);

	// sprite W
	if( !lua_isnumber(L, 6) ) {
		lua_pushstring(L, "PutSpritePart_wrapper: SW should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 6);
	int sw = ftoi(val);

	// sprite H
	if( !lua_isnumber(L, 7) ) {
		lua_pushstring(L, "PutSpritePart_wrapper: SH should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 7);
	int sh = ftoi(val);

	BOOL bAlpha = FALSE;
	BOOL bChroma = FALSE;
	FLOAT fok = 0.0f;
	BOOL flag = BIGSPR_NOFLAG;

	int handle = LoadBigSprite( (char *)str, (bAlpha)?TRUE:FALSE, (bChroma), FALSE, flag, -1,-1,-1,-1, fok );
	PutBigSpritePart( handle, screenx, screeny, sw,sh, sx,sy, sw,sh );

	screen_activity = TRUE;

	lua_pushnumber(L, 1);

	return 1;
}




//
// PutSprite( 0,0, "screen1.gif", false, false, SCREENW, param )
//
static int PutSprite_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 3 ) {
		lua_pushstring(L, "PutSprite_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// X
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "PutSprite_wrapper: X should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int x = ftoi(val);

	// Y
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "PutSprite_wrapper: Y should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 2);
	int y = ftoi(val);

	// str
	const char *str = lua_tostring(L, 3);
	if( str == NULL ) {
		lua_pushstring(L, "PutSprite_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	BOOL bAlpha = FALSE;
	BOOL bChroma = FALSE;

	// Alpha
	if( lua_isboolean(L, 4) )
		bAlpha = lua_toboolean(L, 4);
	else
		bAlpha = FALSE;

	// Chroma
	if( lua_isboolean(L, 5) )
		bChroma = lua_toboolean(L, 5);
	else
		bChroma = FALSE;

	int iScalew = -1;
	int iScaleh = -1;

	// scalew
	if( lua_isnumber(L, 6) ) {
		double val = lua_tonumber(L, 6);
		iScalew = ftoi(val);
	}
	else
		iScalew = -1;

	// scaleh
	if( lua_isnumber(L, 7) ) {
		double val = lua_tonumber(L, 7);
		iScaleh = ftoi(val);
	}
	else
		iScaleh = -1;

	FLOAT fok = 0.0f;

	// rotation
	if( lua_isnumber(L, 8) )
		fok = (FLOAT)lua_tonumber(L, 8);
	else
		fok = 0.0f;

	// xprintf("spr: %s, w: %d  h: %d  alpha: %s  chroma: %s  fok: %f\n", (char *)str,iScalew,iScaleh,bAlpha?"True":"False",bChroma?"True":"False", fok);

// FIXME: ha alfásan van betöltve nem lehet alfa nékül kirakni és viszont
#define ittPUTPICTUREPARAMSCALEFLAGROT(name,x,y,bAlpha,bChroma,iScalew,iScaleh,flag,fok) { 		\
	int handle = LoadBigSprite( (name), (bAlpha)?TRUE:FALSE, (bChroma), FALSE, flag, fok ); 	\
	BOOL spr_alpha;											\
	if( GetBigSpriteParam( handle, NULL, NULL, &spr_alpha ) )					\
		if( BOOLEAN(bAlpha) != BOOLEAN(spr_alpha) )						\
			LoadBigSprite( (name), (bAlpha)?TRUE:FALSE, (bChroma), FALSE, TRUE, fok );	\
	if( fok != 0.0f ) 										\
		SetBigSpriteParam( handle, -1, -1, -1, fok );						\
	PutBigSprite( handle, (x), (y), (iScalew),(iScaleh), (bAlpha)?255:-1, (bChroma)  ); }

	PUTPICTUREPARAMSCALEFLAGROT((char *)str,x,y,bAlpha,bChroma,iScalew,iScaleh,FALSE,fok)

	screen_activity = TRUE;

	lua_pushnumber(L, 1);

	return 1;
}




//
// local id = LoadSprite("valami.png")
//
static int LoadSprite_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "LoadSprite_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// str
	const char *str = lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "LoadSprite_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	BOOL bAlpha = FALSE;
	BOOL bChroma = FALSE;

	// Alpha
	if( lua_isboolean(L, 2) )
		bAlpha = lua_toboolean(L, 2);
	else
		bAlpha = FALSE;

	// Chroma
	if( lua_isboolean(L, 3) )
		bChroma = lua_toboolean(L, 3);
	else
		bChroma = FALSE;

	int handle = LoadBigSprite( (char *)str, (bAlpha)?TRUE:FALSE, (bChroma) );

	lua_pushnumber(L, handle);

	return 1;
}



//
// WriteSprite("valami.png", id, ["png" | "jpg" | "pcx"])
//
static int WriteSprite_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "WriteSprite_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *filename = (char *)lua_tostring(L, 1);
	if( filename == NULL ) {
		lua_pushstring(L, "WriteSprite_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// id
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "WriteSprite_wrapper: id should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 2);
	int handle = ftoi(val);

	char *ext = NULL;
	if( n>=3 && lua_isstring(L, 3) )
		ext = (char *)lua_tostring(L, 3);

	UCHAR *spr = GetBigSpriteData(handle);
	if( spr == NULL ) {
		lua_pushstring(L, "WriteSprite_wrapper: can't find sprite");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( !ext )
		ext = GetExtension(filename);

	if( !stricmp(ext,"spr") ) {
		// PushTomFlag();
		// TomFlag(NOFLAG);
		WriteSprite(filename,spr);
		// PopTomFlag();
	}
	else
	if( !stricmp(ext,"bmp") )
		SaveBmp(filename,spr);
	else
	if( !stricmp(ext,"pcx") )
		SavePcx(filename,spr);
	else
		// a filenévből dönti el mi lesz
		SaveFree(filename,spr);

	lua_pushnumber(L, handle);

	return 1;
}



//
// FreeSprite( id )
//
static int FreeSprite_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "FreeSprite_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// id
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "FreeSprite_wrapper: id should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int handle = ftoi(val);

	FreeBigSprite(handle);

	return 0;
}


//
// local r,g,b = SPR(id,x,y)
//
static int SPR_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 3 ) {
		lua_pushstring(L, "SPR_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// id
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "SPR_wrapper: id should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int id = ftoi(val);

	// X
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "SPR_wrapper: X should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 2);
	int x = ftoi(val);

	// Y
	if( !lua_isnumber(L, 3) ) {
		lua_pushstring(L, "SPR_wrapper: Y should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 3);
	int y = ftoi(val);

	int r=100,g=120,b=130;

	if( GetBigSpritePixel(id,x,y,&r,&g,&b) == FALSE ) {
		lua_pushstring(L, "SPR_wrapper: can't find sprite");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);

	return 3;
}





//
//
//
static int SPRITEW_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "SPRITEW_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	const char *str;
	int w;

	if( lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		int handle = ftoi(val);
		if( GetBigSpriteParam( handle, &w ) == FALSE ) {
			char str[256];
			sprintf(str, "SPRITEW_wrapper: can't find %d sprite", handle );
			lua_pushstring(L, str);
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}
	}
	else
	if( lua_isstring(L, 1) && ((str = lua_tostring(L, 1)) != NULL) )
		// handle = LoadBigSprite( (char *)str );
		GetFreeDim( (char *)str, &w );
	else {
		lua_pushstring(L, "SPRITEW_wrapper: no sprite name or handler");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, w);

	return 1;
}


//
//
//
static int SPRITEH_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "SPRITEH_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	const char *str;
	int h;

	if( lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		int handle = ftoi(val);
		if( GetBigSpriteParam( handle, NULL, &h ) == FALSE ) {
			lua_pushstring(L, "SPRITEH_wrapper: can't find sprite");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}
	}
	else
	if( lua_isstring(L, 1) && ((str = lua_tostring(L, 1)) != NULL) )
		// handle = LoadBigSprite( (char *)str );
		GetFreeDim( (char *)str, NULL, &h );
	else {
		lua_pushstring(L, "SPRITEH_wrapper: no sprite name or handler");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, h);

	return 1;
}




//
// CompositeSprite( dst, src )
//
static int CompositeSprite_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "CompositeSprite_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// dst
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "CompositeSprite_wrapper: id should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int dst = ftoi(val);

	// src
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "CompositeSprite_wrapper: id should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 2);
	int src = ftoi(val);

	int x = 0;
	int y = 0;

	if( n>=3 && lua_isnumber(L, 3) ) {
		val = lua_tonumber(L, 3);
		x = ftoi(val);
	}

	if( n>=4 && lua_isnumber(L, 4) ) {
		val = lua_tonumber(L, 4);
		y = ftoi(val);
	}

	CompositeSprite( GetBigSpriteData(dst), GetBigSpriteData(src), x,y );

	return 0;
}




//
// SpriteSheet( dst, src, maxw, maxh )
//
static int SpriteSheet_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "SpriteSheet_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// int tableIndex = 1;
	// || lua_istable(L,tableIndex)

	char *listname = (char *)lua_tostring(L, 1);
	if( listname == NULL ) {
		lua_pushstring(L, "SpriteSheet_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char outname[XMAX_PATH] = "";

	if( n>=2 && lua_isstring(L,2) ) {

		char *text = (char *)lua_tostring(L, 2);
		if( text )
			strcpy( outname, text );
	}

	xprintf("out: %s\n",outname);
	BOOL res;

	if( strlen(outname) > 1 )
		res = SpriteSheet( listname, outname );
	else
		res = SpriteSheet( listname );

	if( res == FALSE ) {
		lua_pushstring(L, "SpriteSheet returned error");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	return 0;
}




//
// GammaSprite( id, gamma )
//
static int GammaSprite_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "GammaSprite_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// dst
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "GammaSprite_wrapper: id should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int handle = ftoi(val);

	// gamma
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "GammaSprite_wrapper: gamma should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	gamma_corr.value = lua_tonumber(L, 2);

	GammaSprite( GetBigSpriteData(handle), NULL );

	return 0;
}




//
//
//
static int GetValue_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "GetValue_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	const char *str = lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "GetValue_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( !stricmp(str, "mousex") )
		lua_pushnumber(L, mousex);
	else
	if( !stricmp(str, "mousey") )
		lua_pushnumber(L, mousey);
	else
	if( !stricmp(str, "mousedx") )
		lua_pushnumber(L, mousedx);
	else
	if( !stricmp(str, "mousedy") )
		lua_pushnumber(L, mousedy);
	else
	if( !stricmp(str, "mousedz") )
		lua_pushnumber(L, mousedz);
	else
	if( !stricmp(str, "mousebl") )
		lua_pushboolean(L, mousebl);
	else
	if( !stricmp(str, "mousebld") )
		lua_pushboolean(L, mousebld);
	else
	if( !stricmp(str, "mousebr") )
		lua_pushboolean(L, mousebr);
	else
	if( !stricmp(str, "mousebrd") )
		lua_pushboolean(L, mousebrd);
	else
	if( !stricmp(str, "mousebm") )
		lua_pushboolean(L, mousebm);
	else
	if( !stricmp(str, "mousebmd") )
		lua_pushboolean(L, mousebmd);
	else
	if( !stricmp(str, "mouseb4") )
		lua_pushboolean(L, mouseb4);
	else
	if( !stricmp(str, "mouseb4d") )
		lua_pushboolean(L, mouseb4d);
	else
	if( !stricmp(str, "mouseb5") )
		lua_pushboolean(L, mouseb5);
	else
	if( !stricmp(str, "mouseb5d") )
		lua_pushboolean(L, mouseb5d);
	else
	if( !stricmp(str, "joyx") )
		lua_pushnumber(L, joyx);
	else
	if( !stricmp(str, "joyy") )
		lua_pushnumber(L, joyy);
	else
	if( !stricmp(str, "joyz") )
		lua_pushnumber(L, joyz);
	else
	if( !stricmp(str, "joyb0") )
		lua_pushboolean(L, joyb[0]);
	else
	if( !stricmp(str, "joyb1") )
		lua_pushboolean(L, joyb[1]);
	else
	if( !stricmp(str, "joyb2") )
		lua_pushboolean(L, joyb[2]);
	else
	if( !stricmp(str, "joyb3") )
		lua_pushboolean(L, joyb[3]);
	else
	if( !stricmp(str, "joyb4") )
		lua_pushboolean(L, joyb[4]);
	else
	if( !stricmp(str, "joyb5") )
		lua_pushboolean(L, joyb[5]);
	else
	if( !stricmp(str, "joyb6") )
		lua_pushboolean(L, joyb[6]);
	else
	if( !stricmp(str, "joyb7") )
		lua_pushboolean(L, joyb[7]);
	else
	if( !stricmp(str, "joyb8") )
		lua_pushboolean(L, joyb[8]);
	else
	if( !stricmp(str, "joyb9") )
		lua_pushboolean(L, joyb[9]);
	else
	if( !stricmp(str, "joybd0") )
		lua_pushboolean(L, joybd[0]);
	else
	if( !stricmp(str, "joybd1") )
		lua_pushboolean(L, joybd[1]);
	else
	if( !stricmp(str, "joybd2") )
		lua_pushboolean(L, joybd[2]);
	else
	if( !stricmp(str, "joybd3") )
		lua_pushboolean(L, joybd[3]);
	else
	if( !stricmp(str, "joybd4") )
		lua_pushboolean(L, joybd[4]);
	else
	if( !stricmp(str, "joybd5") )
		lua_pushboolean(L, joybd[5]);
	else
	if( !stricmp(str, "joybd6") )
		lua_pushboolean(L, joybd[6]);
	else
	if( !stricmp(str, "joybd7") )
		lua_pushboolean(L, joybd[7]);
	else
	if( !stricmp(str, "joybd8") )
		lua_pushboolean(L, joybd[8]);
	else
	if( !stricmp(str, "joybd9") )
		lua_pushboolean(L, joybd[9]);
	else
	if( !stricmp(str, "MAXVOL") )
		lua_pushnumber(L, MAXVOL);
	else
	// GetKey()
	if( !stricmp(str, "key") ) {
		char str[2];
		int key = key_None;
		if( !display_zprintf )
			key = GetKey();
		str[0] = ISEXTKEY(key)?0:key;
		str[1] = 0;
		lua_pushstring(L, str);
	}
	else
	if( !stricmp(str, "Up") )
		lua_pushboolean(L, keys[sc_Up]);
	else
	if( !stricmp(str, "Down") )
		lua_pushboolean(L, keys[sc_Down]);
	else
	if( !stricmp(str, "Left") )
		lua_pushboolean(L, keys[sc_Left]);
	else
	if( !stricmp(str, "Right") )
		lua_pushboolean(L, keys[sc_Right]);
	else
	if( !stricmp(str, "Enter") )
		lua_pushboolean(L, keys[sc_Enter]);
	else
	if( !stricmp(str, "Space") )
		lua_pushboolean(L, keys[sc_Space]);
	else
	if( !stricmp(str, "Backspace") )
		lua_pushboolean(L, keys[sc_BackSpace]);
	else
	if( !stricmp(str, "+") || !stricmp(str, "plus") )
		lua_pushboolean(L, keys[sc_Plus] || keys[sc_GreyPlus]);
	else
	if( !stricmp(str, "-") || !stricmp(str, "minus") )
		lua_pushboolean(L, keys[sc_Minus] || keys[sc_GreyMinus]);
	else
	if( !stricmp(str, "a") )
		lua_pushboolean(L, keys[sc_A]);
	else
	if( !stricmp(str, "b") )
		lua_pushboolean(L, keys[sc_B]);
	else
	if( !stricmp(str, "c") )
		lua_pushboolean(L, keys[sc_C]);
	else
	if( !stricmp(str, "d") )
		lua_pushboolean(L, keys[sc_D]);
	else {
		char err[512];
		sprintf(err,"GetValue_wrapper: no value for %s", str);
		lua_pushstring(L, err);
		lua_error(L);
		lua_pushnumber(L, 0);
	}

	return 1;
}




//
// Rand(400) = 1 .. 400
//
static int Rand_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	int m = 255;
	int r;

	if( n>=1 && lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		m = ftoi(val);
	}

	if( n==1 ) {
		for( int i=0; i<10; i++ )
			r = rand() % m;
		++r;
		lua_pushnumber(L, r);
		return 1;
	}

	if( n>=2 && lua_isnumber(L, 2) ) {
		double val = lua_tonumber(L, 2);
		int o = ftoi(val);
		for( int i=0; i<10; i++ )
			r = rand() % (o-m);
		r = r + m + 1;
		lua_pushnumber(L, r);
		return 1;
	}

	for( int i=0; i<10; i++ )
		r = rand() % m;

	++r;

	lua_pushnumber(L, r);

	return 1;
}


//
//
//
static int Delay_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	int d = 0;

	if( n>0 && lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		d = ftoi(val);
	}

	if( d > 0 ) {
		ULONG endtic = GetTic() + d;

		do {
			XLIB_MinimalMessagePump();
			// Sleep(1);
		} while( GetTic() < endtic );
	}

	return 0;
}





//
//
//
static int GetTic_wrapper( lua_State *L ) {

	lua_pushnumber(L, GetTic());

	return 1;
}


//
// id = PlaySound( "valami.mid", [vol: 1-100], [pan: 1-100], [loop: True-False] )
//
static int PlaySound_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "PlaySound_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	const char *str = lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "PlaySound_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int vol,pan;
	BOOL loop;

	// volume
	if( lua_isnumber(L, 2) ) {
		double val = lua_tonumber(L, 2);
		vol = ftoi(val);
	}
	else
		vol = MAXVOL;

	// pan
	if( lua_isnumber(L, 3) ) {
		double val = lua_tonumber(L, 3);
		pan = ftoi(val);
	}
	else
		pan = MIDPAN;

	// loop
	if( lua_isboolean(L, 4) )
		loop = lua_toboolean(L, 4);
	else
		loop = FALSE;

	int lib = 0;
	const char *libname = lua_tostring(L, 5);
	if( libname ) {
		if( !stricmp(libname,"BASS") ) lib = 0;
		if( !stricmp(libname,"OpenAL") ) lib = 1;
		if( !stricmp(libname,"Mikmod") ) lib = 2;
		if( !stricmp(libname,"DSound") ) lib = 3;
	}

	// xprintf("PlaySound: %s, %d, %d, %d\n", (char *)str, vol, pan, loop);

	int id = 0;

	if( lib == 1 ) {
		id = OAL_LoadWave( (char *)str );
		if( id < 0 ) {
			xprintf("PlaySound: OAL can't load %s\n",str);
			lua_pushnumber(L, 0);
			return 1;
		}
		OAL_SetWaveParam( id, -1, vol, pan, loop );
		OAL_PlayWave( id );
	}
	else
	if( lib == 2 ) {
		id = MIKLIB_LoadWave( (char *)str );
		if( id < 0 ) {
			xprintf("PlaySound: MIKLIB can't load %s\n",str);
			lua_pushnumber(L, 0);
			return 1;
		}
		MIKLIB_SetWaveParam( id, -1, vol, pan, loop );
		MIKLIB_PlayWave( id );
	}
	else
	if( lib == 3 ) {
		id = DS_LoadWave( (char *)str );
		if( id < 0 ) {
			xprintf("PlaySound: DS can't load %s\n",str);
			lua_pushnumber(L, 0);
			return 1;
		}
		DS_SetWaveParam( id, -1, vol, pan, loop );
		DS_PlayWave( id );
	}
	else {
		if( (id = SL_IsLoaded( (char *)str )) ) {
			if( SL_IsPlaying(id) == FALSE )
				id = SL_PlaySound( (char *)str, vol, pan, loop );
		}
		else
		if( (id = SL_PlaySound( (char *)str, vol, pan, loop )) <= 0 ) {
			xprintf("PlaySound: can't play %s\n",str);
			lua_pushnumber(L, 0);
			return 1;
		}
	}

	lua_pushnumber(L, id);

	return 1;
}



//
//
//
static int StopSound_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "StopSound_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int id = lua_tonumber(L, 1);

	OAL_StopWave(id);
	MIKLIB_StopWave( id );
	DS_StopWave( id );
	SL_StopSound(id);

	return 0;
}



//
// selector( "*.mid", "midi selector" )
//
static int selector_wrapper( lua_State *L ) {

	int n = lua_gettop(L);

	BOOL result;

	char *ext = (char *)lua_tostring(L, 1);
	char *title = (char *)lua_tostring(L, 2);
	char *init_dir = (char *)lua_tostring(L, 3);

	// xprintf("ext: %s, title: %s, dir: %s\n",ext,title,init_dir);

	if( ext ) {
		char str[XMAX_PATH];
		sprintf( str, "Files (%s)", ext );
		int i = strlen(str)+1;
		strcpy( &str[i], ext );
		i += strlen(&str[i])+1;
		strcpy( &str[i], "All Files (*.*)" );
		i += strlen(&str[i])+1;
		strcpy( &str[i], "*.*" );
		i += strlen(&str[i])+1;
		str[i] = 0;	// dupla bezáró
		// sprintf( str, "Files (%s)\0%s\0All Files (*.*)\0*.*\0", ext, ext );
		result = winOpenFile(str,title,init_dir);
	}
	else
		result = winOpenFile("All Files (*.*)\0*.*\0",title,init_dir);

	if( result == FALSE ) {
		lua_pushnil(L);
		return 1;
	}

	char *filename;
	if( (filename = XLIB_winGetNextFile()) == NULL ) {
		lua_pushnil(L);
		return 1;
	}

	// if( FileExist( filename ) );

	lua_pushstring(L, filename);

	return 1;
}




//
//
//
static int Console_wrapper( lua_State *L ) {

	int result_type = CON_NORESULT;

	int n = lua_gettop(L);
	if( n < 1 ) {
		// lua_pushstring(L, "Console_wrapper: not enough arguments");
		// lua_error(L);
		result_type = consoleExec();
	}
	else {
		const char *str = lua_tostring(L, 1);
		if( str == NULL ) {
			lua_pushstring(L, "Console_wrapper: can't convert to string");
			lua_error(L);
		}
		else
			result_type = consoleExec( -100, (char *)str );
	}

	switch( result_type ) {

		case CON_NUMBER:
			lua_pushnumber(L, consoleGetNumber());
			break;

		case CON_STRING:
			lua_pushstring(L, consoleGetString());
			break;

		case CON_BOOL:
			lua_pushboolean(L, consoleGetBool());
			break;

		default:
		case CON_NORESULT:
			lua_pushnil(L);
			break;
	}

	return 1;
}




//
//
//
void luaTypeToString( lua_State *L, int index, int type, char *out, int size ) {

	switch( type ) {
		case LUA_TSTRING: {
			size_t l = lua_strlen(L, index);
			snprintf( out, size, "%s", lua_tostring(L, index) );
			break;
		}
		case LUA_TBOOLEAN:
			snprintf( out, size, "%s", lua_toboolean(L, index) ? "True" : "False" );
			break;
		case LUA_TNUMBER: {
			double val = lua_tonumber(L, index);
			int ival = ftoi(val);
			if( val != (FLOAT)ival )
				snprintf( out, size, "%f", (FLOAT)val );
			else
				snprintf( out, size, "%d", ival );
			break;
		}
		default:
			snprintf( out, size, "%s", lua_typename(L, type) );
			/***
			char str[255];
			sprintf( str, "LUA_xprintf_wrapper: got %s type.", lua_typename(L, lua_type(L,i)) );
			lua_pushstring(L, str);
			lua_error(L);
			***/
			break;
        }

	return;
}


//
// local t1 = { a=1, b=2, c=3, "valami", 4 , 5 }
//
static void dumpLuaTable( lua_State *L, int tableIndex ) {

	BOOL first = TRUE;
	lua_pushnil(L);
	xprintf("{");

	while( lua_next(L, tableIndex) != 0 ) {

		if( first == FALSE )
			xprintf(", ");
		first = FALSE;

		int type = lua_type(L, -1);
		if( type == LUA_TTABLE )
			dumpLuaTable( L, lua_gettop(L) );
		else {
			if( lua_type(L, -2) == LUA_TSTRING )
				xprintf("%s=", lua_tostring(L, -2) );	// keyString
			char *valueString;
			ALLOCMEM(valueString, BUFSIZE);
			luaTypeToString( L, -1, type, valueString, BUFSIZE );
			xprintf(valueString);
			FREEMEM(valueString);
		}

		lua_pop(L, 1);
	}

	xprintf("}");

	return;
}



//
//
//
static int xprintf_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		// lua_pushstring(L, "xprintf_wrapper: not enough arguments");
		// lua_error(L);
	}

	for( int i=1; i<=n; i++ ) {

		int type = lua_type(L,i);

		if( lua_type(L,i) == LUA_TTABLE )
			dumpLuaTable(L, i);
		else {
			char *str;
			ALLOCMEM(str, BUFSIZE*2);
			luaTypeToString(L, i, type, str, BUFSIZE);
			// ChangeCharStr(str, '%%', "%%%" );
			xprintf("%s",str);
			FREEMEM(str);
		}
	}

	// új sort mindig ír a lua
	screen_activity = TRUE;
	xprintf("\n");

	return 0;
}




//
//
//
static void writeLuaType( lua_State *L, int index, int type, FILE *f ) {

	switch( type ) {
		case LUA_TSTRING: {
			size_t l = lua_strlen(L, index);
			fwrite( lua_tostring(L, index), l+1, 1, f );
			break;
		}
		case LUA_TBOOLEAN: {
			UCHAR c = (UCHAR)lua_toboolean(L, index);
			fwrite( &c, sizeof(UCHAR), 1, f );
			break;
		}
		case LUA_TNUMBER: {
			double val = lua_tonumber(L, index);
			int ival = ftoi(val);
			if( val <= 0xff && val >= 0 && val == (FLOAT)ival ) {
				UCHAR c = (UCHAR)val;
				fwrite( &c, sizeof(UCHAR), 1, f );
			}
			else
				fwrite( &val, sizeof(double), 1, f );
			break;
		}
		default:
			xprintf( "writeLuaType: got %s type.", lua_typename(L, lua_type(L,index)) );
			break;
        }

	return;
}




//
// local t1 = { a=1, b=2, c=3, "valami", 4 , 5 }
//
static void writeLuaTable( lua_State *L, int tableIndex, FILE *f ) {

	lua_pushnil(L);

	while( lua_next(L, tableIndex) != 0 ) {

		int type = lua_type(L, -1);
		if( type == LUA_TTABLE )
			writeLuaTable( L, lua_gettop(L), f );
		else {
			if( lua_type(L, -2) == LUA_TSTRING )
				const char *keyString = lua_tostring(L, -2);	// keyString

			writeLuaType( L, -1, type, f );
		}

		lua_pop(L, 1);
	}

	return;
}




//
// WriteFile( <table, string, number, boolean>, filename, [BOOL append | "append"] )
//
static int WriteFile_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "WriteFile_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *filename = (char *)lua_tostring(L, 2);
	if( filename == NULL ) {
		lua_pushstring(L, "WriteFile_wrapper: need a string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char open_flag[3] = "wb";
	if( lua_isboolean(L, 3) ) {
		if( lua_toboolean(L, 3) ) {
			open_flag[0] = 'a';
			if( FileExist( filename, TRUE ) == FALSE )
				open_flag[0] = 'w';
		}
	}
	else
	if( lua_isstring(L, 3) ) {
		char *append = (char *)lua_tostring(L, 3);
		if( append && !stricmp(append,"append") ) {
			open_flag[0] = 'a';
			if( FileExist( filename, TRUE ) == FALSE )
				open_flag[0] = 'w';
		}
	}

	if( lua_istable(L,1) ) {

		int len = lua_objlen(L,1);

		FILE *f = fopen( filename, open_flag );
		if( f == NULL ) {
			lua_pushstring(L, "WriteFile_wrapper: can't open file");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		writeLuaTable(L,1,f);
		fclose(f);
	}
	else
	if( lua_isnumber(L,1) ) {

		double num = (double)lua_tonumber(L, 1);

		FILE *f = fopen( filename, open_flag );
		if( f == NULL ) {
			lua_pushstring(L, "WriteFile_wrapper: can't open file");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		fwrite( &num, sizeof(double), 1, f );
		fclose(f);
	}
	else
	if( lua_isboolean(L,1) ) {

		UCHAR c = (UCHAR)lua_toboolean(L, 1);

		FILE *f = fopen( filename, open_flag );
		if( f == NULL ) {
			lua_pushstring(L, "WriteFile_wrapper: can't open file");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		fwrite( &c, sizeof(UCHAR), 1, f );
		fclose(f);
	}
	else
	if( lua_isstring(L,1) ) {

		char *str = (char *)lua_tostring(L, 1);

		FILE *f = fopen( filename, open_flag );
		if( f == NULL ) {
			lua_pushstring(L, "WriteFile_wrapper: can't open file");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		fwrite( str, strlen(str)+1, 1, f );
		fclose(f);
	}

	lua_pushnumber(L, 0);

	return 1;
}




//
// cmpr = Compress( <table, string>, [ "zip", "lza", ] )
//
static int Compress_wrapper( lua_State *L ) {

	UCHAR *buf = NULL;
	int size;
	int type = 0;

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "Compress_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// mivel compressel?
	if( n > 1 ) {
		char *type_string = (char *)lua_tostring(L, 2);
		if( type_string != NULL ) {
			     if( !stricmp(type_string,"zip") ) type = 0;
			else if( !stricmp(type_string,"lza") ) type = 1;
			else if( !stricmp(type_string,"lzh") ) type = 2;
			else if( !stricmp(type_string,"lzs") ) type = 3;
			else if( !stricmp(type_string,"rle") ) type = 4;
			else if( !stricmp(type_string,"uu") ) type = 5;
		}
	}

	if( lua_istable(L,1) ) {

		int len = lua_objlen(L,1);

		if( len > 0 ) {
			UCHAR *table;
			ALLOCMEM( table, len );
			UCHAR *p = table;

			lua_pushnil(L);

			while( lua_next(L, 1) != 0 ) {

				// if( lua_type(L, -2) == LUA_TSTRING )
				//	xprintf("%s=", lua_tostring(L, -2) );	// keyString

				int type = lua_type(L, -1);
				if( type == LUA_TTABLE )
					*p++ = 0;	// TODO: táblába ágyazott tábla
				else {
					double val = lua_tonumber(L, -1);
					int i = ftoi(val);
					*p++ = i;
				}

				lua_pop(L, 1);
			}

			     if( type == 0 ) size = ZIP_Compress( table, len, PTR(buf) );
			else if( type == 1 ) size = EncodeLZA( table, len, PTR(buf) );
			else if( type == 2 ) size = EncodeLZH( table, len, PTR(buf) );
			else if( type == 3 ) size = EncodeLZS( table, len, PTR(buf) );
			else if( type == 4 ) size = EncodeRLE( table, len, PTR(buf) );
			else if( type == 5 ) size = EncodeUU( "lua.dat", table, len, PTR(buf) );

			FREEMEM(table);
		}
	}
	else
	if( lua_isstring(L,1) ) {

		char *str = (char *)lua_tostring(L, 1);
		int len;

		if( str && (len=strlen(str)) > 0 ) {
			     if( type == 0 ) size = ZIP_Compress( (UCHAR *)str, len+1, PTR(buf) );
			else if( type == 1 ) size = EncodeLZA( (UCHAR *)str, len+1, PTR(buf) );
			else if( type == 2 ) size = EncodeLZH( (UCHAR *)str, len+1, PTR(buf) );
			else if( type == 3 ) size = EncodeLZS( (UCHAR *)str, len+1, PTR(buf) );
			else if( type == 4 ) size = EncodeRLE( (UCHAR *)str, len+1, PTR(buf) );
			else if( type == 5 ) size = EncodeUU( "lua.dat", (UCHAR *)str, len+1, PTR(buf) );
		}
	}

	// ha sikerült összenyomni
	if( buf ) {

		// return as table
		lua_newtable(L);
		int top = lua_gettop(L);
		int index = 1;

		for( int index = 1; index <= size; index++ )  {

			// key
			lua_pushinteger(L,index);

			// value
			lua_pushinteger(L,buf[index-1]);

			// set the table entry
			lua_settable(L, -3);
		}

		// push the new table
		lua_pushvalue(L,-1);

		FREEMEM(buf);
	}
	else
		lua_pushnil(L);

	return 1;
}




//
// ucmpr = Uncompress( table )
//
static int Uncompress_wrapper( lua_State *L ) {

	UCHAR *buf = NULL;
	int size;

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "Uncompress_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( lua_istable(L,1) ) {

		int len = lua_objlen(L,1);

		if( len > 0 ) {
			UCHAR *table;
			ALLOCMEM( table, len );
			UCHAR *p = table;

			lua_pushnil(L);

			while( lua_next(L, 1) != 0 ) {

				// if( lua_type(L, -2) == LUA_TSTRING )
				//	xprintf("%s=", lua_tostring(L, -2) );	// keyString

				int type = lua_type(L, -1);
				if( type == LUA_TTABLE )
					*p++ = 0;	// TODO: táblába ágyazott tábla
				else {
					double val = lua_tonumber(L, -1);
					int i = ftoi(val);
					*p++ = i;
				}

				lua_pop(L, 1);
			}

			     if( IsZIP( (char *)table, len ) ) size = ZIP_Uncompress( table, PTR(buf) );
			else if( IsLZA( (char *)table, len ) ) size = DecodeLZA( table, PTR(buf) );
			else if( IsLZH( (char *)table, len ) ) size = DecodeLZH( table, PTR(buf) );
			else if( IsLZS( (char *)table, len ) ) size = DecodeLZS( table, PTR(buf) );
			else if( IsRLE( (char *)table, len ) ) size = DecodeRLE( table, PTR(buf) );
			else if( IsUU( (char *)table, len ) ) size = DecodeUU( table, len, PTR(buf) );
			else { ALLOCMEM(buf,len); memcpy(buf,table,len); }

			FREEMEM(table);
		}
	}

	// ha sikerült összenyomni
	if( buf ) {

		// return as table
		lua_newtable(L);
		int top = lua_gettop(L);
		int index = 1;

		for( int index = 1; index <= size; index++ )  {

			// key
			lua_pushinteger(L,index);

			// value
			lua_pushinteger(L,buf[index-1]);

			// set the table entry
			lua_settable(L, -3);
		}

		// push the new table
		lua_pushvalue(L,-1);

		FREEMEM(buf);
	}
	else
		lua_pushnil(L);

	return 1;
}






//
// WriteMemory( <window name, pid, exe>, address, <number,string,table>, <"byte","word","int","float","double","unicode"> )
// Cheat Engine address
//
static int WriteMemory_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 3 ) {
		lua_pushstring(L, "WriteMemory_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	DWORD pid;
	char *name;

	if( lua_isnumber(L,1) )
		pid = (DWORD)lua_tonumber(L, 1);
	else
	if( lua_isstring(L,1) ) {

		name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "WriteMemory_wrapper: can't get window name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		HWND hwnd = (HWND)XLIB_FindWindow(name);
		if( hwnd == NULL ) {
			if( (pid = XLIB_GetPidFromExe(name)) == 0 ) {
				lua_pushstring(L, "WriteMemory_wrapper: can't find window or process or pid");
				lua_error(L);
				lua_pushnumber(L, 0);
				return 1;
			}
		}
		else
			GetWindowThreadProcessId(hwnd,&pid);
	}

	int offset = (int)lua_tonumber(L, 2);

	// Cheat Engine miatt nem kell base address
	DWORD baseAddr = 0; // XLIB_GetModuleBaseAddress(pid);

	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );

	if( hProcess == NULL ) {
		lua_pushstring(L, "WriteMemory_wrapper: can't open process");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int size = 0;		// default = az adat tipusa
	if( n>3 && lua_isnumber(L,4) ) {
		size = (int)lua_tonumber(L, 4);
		size = ABS(size);
	}
	else
	if( n>3 && lua_isstring(L,4) ) {
		char *name = (char *)lua_tostring(L, 4);
		if( name && !stricmp(name,"byte") )
			size = 1;
		if( name && !stricmp(name,"word") )
			size = -1;
		if( name && (!stricmp(name,"int") || !stricmp(name,"integer") || !stricmp(name,"unsigned")) )
			size = -2;
		if( name && !stricmp(name,"float") )
			size = -3;
		if( name && !stricmp(name,"double") )
			size = -4;
		if( name && !stricmp(name,"string") )
			size = -5;
		if( name && !stricmp(name,"unicode") )
			size = -6;
	}

	int type = 1;
	double number;
	char *string;
	UCHAR *table=NULL;
	int len;

	if( lua_isnumber(L,3) ) {
		number = lua_tonumber(L, 3);
		type = 1;
	}
	else
	if( lua_isboolean(L,3) ) {
		number = (double)lua_toboolean(L, 3);
		type = 1;
	}
	else
	if( lua_isstring(L,3) ) {

		string = (char *)lua_tostring(L, 3);
		if( string == NULL ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "WriteMemory_wrapper: can't get string");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}
		type = 2;
	}
	else
	if( lua_istable(L,3) ) {

		len = lua_objlen(L,3);

		if( len <= 0 ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "WriteMemory_wrapper: can't write NULL table");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		ALLOCMEM( table, len );
		UCHAR *p = table;

		lua_pushnil(L);

		while( lua_next(L, 3) != 0 ) {

			// if( lua_type(L, -2) == LUA_TSTRING )
			//	xprintf("%s=", lua_tostring(L, -2) );	// keyString

			int type = lua_type(L, -1);
			if( type == LUA_TTABLE )
				*p++ = 0;	// TODO: táblába ágyazott tábla
			else {
				double val = lua_tonumber(L, -1);
				int i = ftoi(val);
				*p++ = i;
			}

			lua_pop(L, 1);
		}

		type = 3;
	}
	else {
		CloseHandle(hProcess);
		lua_pushstring(L, "WriteMemory_wrapper: type not supported");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// csak az object van
	if( size == 0 ) {

		DWORD byteswritten;

		// byte
		if( type == 1 ) {
			UCHAR c = (UCHAR)ftoi(number);

			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&c,1,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string
		if( type == 2 )
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)string,strlen(string)+1,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());

		// tömb
		if( type == 3 )
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)table,len,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
	}

	// Byte
	if( size == 1 ) {

		DWORD byteswritten;

		// byte
		if( type == 1 ) {
			UCHAR c = (UCHAR)ftoi(number);

			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&c,1,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string[0]
		if( type == 2 )
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)string,1,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());

		// tömb[0]
		if( type == 3 )
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)table,1,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
	}

	// Byteok írása
	if( size > 1 ) {

		DWORD byteswritten;

		// byte
		if( type == 1 ) {
			UCHAR c = (UCHAR)ftoi(number);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&c,1,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string-ből valamennyit
		if( type == 2 ) {
			int len = MIN(size,strlen(string)+1);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)string,len,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// tömbből valamennyit
		if( type == 3 ) {
			int i = MIN(size,len);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)table,i,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}
	}

	// word 16 bit
	if( size == -1 ) {

		DWORD byteswritten;
		USHORT i;

		// byte
		if( type == 1 ) {
			i = (USHORT)ftoi(number);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&i,2,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string[0] + string[1]
		if( type == 2 ) {
			i = string[0];
			if( strlen(string) > 1 ) i |= (((int)string[1]) << 8);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&i,2,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// tömb[0] + tömb[1]
		if( type == 3 ) {
			i = table[0];
			if( len > 1 ) i |= (((int)table[1]) << 8);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&i,2,&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}
	}

	// int 32 bit
	if( size == -2 ) {

		DWORD byteswritten;
		UINT i;

		// byte
		if( type == 1 ) {
			i = (UINT)number;
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&i,sizeof(UINT),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string[0] + string[1] + string[2] + string[3]
		if( type == 2 ) {
			i = string[0];
			if( strlen(string) > 1 ) i |= (((int)string[1]) << 8);
			if( strlen(string) > 2 ) i |= (((int)string[2]) << 16);
			if( strlen(string) > 3 ) i |= (((int)string[3]) << 24);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&i,sizeof(UINT),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// tömb[0] + tömb[1] + tömb[2] + tömb[3]
		if( type == 3 ) {
			i = table[0];
			if( len > 1 ) i |= (((int)table[1]) << 8);
			if( len > 2 ) i |= (((int)table[2]) << 16);
			if( len > 3 ) i |= (((int)table[3]) << 24);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&i,sizeof(UINT),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}
	}

	// float 32 bit
	if( size == -3 ) {

		DWORD byteswritten;
		UINT i;
		FLOAT f;

		// byte
		if( type == 1 ) {
			f = (FLOAT)number;
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&f,sizeof(FLOAT),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string[0] + string[1] + string[2] + string[3]
		if( type == 2 ) {
			i = string[0];
			if( strlen(string) > 1 ) i |= (((int)string[1]) << 8);
			if( strlen(string) > 2 ) i |= (((int)string[2]) << 16);
			if( strlen(string) > 3 ) i |= (((int)string[3]) << 24);
			f = (FLOAT)i;
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&f,sizeof(FLOAT),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// tömb[0] + tömb[1] + tömb[2] + tömb[3]
		if( type == 3 ) {
			i = table[0];
			if( len > 1 ) i |= (((int)table[1]) << 8);
			if( len > 2 ) i |= (((int)table[2]) << 16);
			if( len > 3 ) i |= (((int)table[3]) << 24);
			f = (FLOAT)i;
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&f,sizeof(FLOAT),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}
	}

	// double 64 bit
	if( size == -3 ) {

		DWORD byteswritten;
		UINT i;
		double d;

		// byte
		if( type == 1 ) {
			d = (double)number;
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&d,sizeof(double),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// string[0] + string[1] + string[2] + string[3] + string[4] + string[5] + string[6] + string[7]
		if( type == 2 ) {
			i = string[0];
			if( strlen(string) > 1 ) i |= (((int)string[1]) << 8);
			if( strlen(string) > 2 ) i |= (((int)string[2]) << 16);
			if( strlen(string) > 3 ) i |= (((int)string[3]) << 24);
			d = (double)i;
			i = 0;
			if( strlen(string) > 4 ) i = (int)string[4];
			if( strlen(string) > 5 ) i |= (((int)string[5]) << 8);
			if( strlen(string) > 6 ) i |= (((int)string[6]) << 16);
			if( strlen(string) > 7 ) i |= (((int)string[7]) << 24);
			d += (double)i * (double)((__int64)1<<32);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&d,sizeof(double),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}

		// tömb[0] + tömb[1] + tömb[2] + tömb[3] + tömb[4] + tömb[5] + tömb[6] + tömb[7]
		if( type == 3 ) {
			i = table[0];
			if( len > 1 ) i |= (((int)table[1]) << 8);
			if( len > 2 ) i |= (((int)table[2]) << 16);
			if( len > 3 ) i |= (((int)table[3]) << 24);
			d = (double)i;
			i = 0;
			if( len > 4 ) i = (int)table[4];
			if( len > 5 ) i |= (((int)table[5]) << 8);
			if( len > 6 ) i |= (((int)table[6]) << 16);
			if( len > 7 ) i |= (((int)table[7]) << 24);
			d += (double)i * (double)((__int64)1<<32);
			if( !WriteProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&d,sizeof(double),&byteswritten) )
				xprintf("WriteProcessMemory: %s\n",XLIB_GetWinError());
		}
	}

	SAFE_FREEMEM(table);

	CloseHandle(hProcess);

	return 0;
}




//
// ReadMemory( <window name, pid>, address, [size,"string","unicode","float","double","unsigned","int","word"] )
// Cheat Engine compatible: Address (base address = 0)
//
static int ReadMemory_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "ReadMemory_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	DWORD pid;
	char *name;

	if( lua_isnumber(L,1) )
		pid = (DWORD)lua_tonumber(L, 1);
	else
	if( lua_isstring(L,1) ) {

		name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "ReadMemory_wrapper: can't get window name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		HWND hwnd = (HWND)XLIB_FindWindow(name);
		if( hwnd == NULL ) {
			if( (pid = XLIB_GetPidFromExe(name)) == 0 ) {
				lua_pushstring(L, "ReadMemory_wrapper: can't find window or process or pid");
				lua_error(L);
				lua_pushnumber(L, 0);
				return 1;
			}
		}
		else
			GetWindowThreadProcessId(hwnd,&pid);
	}

	int offset = 0;
	if( n>1 )
		offset = (int)lua_tonumber(L, 2);

	int size = 1;		// default = byte
	if( n>2 && lua_isnumber(L,3) )
		size = (int)lua_tonumber(L, 3);
	else
	if( n>2 && lua_isstring(L,3) ) {

		char *name = (char *)lua_tostring(L, 3);
		if( name && !stricmp(name,"string") )
			size = -1;
		if( name && !stricmp(name,"unicode") )
			size = -2;
		if( name && !stricmp(name,"float") )
			size = -3;
		if( name && !stricmp(name,"double") )
			size = -4;
		if( name && !stricmp(name,"unsigned") )
			size = -5;
		if( name && (!stricmp(name,"int") || !stricmp(name,"integer")) )
			size = -6;
		if( name && !stricmp(name,"word") )
			size = -7;
	}

	if( size == 0 ) {
		lua_pushstring(L, "ReadMemory_wrapper: size can't be zero");
		lua_error(L);
		lua_pushnumber(L, 0);
		// lua_pushnil(L);
		return 1;
	}

	// Cheat Engine miatt nem kell base address
	DWORD baseAddr = 0; // XLIB_GetModuleBaseAddress(pid);

	HANDLE hProcess = OpenProcess( PROCESS_VM_READ, FALSE, pid );

	if( hProcess == NULL ) {
		lua_pushstring(L, "ReadMemory_wrapper: can't open process");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// tömb
	if( size > 0 ) {

		UCHAR *buf;

		ALLOCMEM( buf, size );

		DWORD bytesread = 0;

		if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)buf,size,&bytesread) )
			xprintf("ReadProcessMemory: %s\n",XLIB_GetWinError());

		// return as table
		lua_newtable(L);
		int top = lua_gettop(L);

		for( int index = 1; index <= bytesread; index++ )  {

			// key
			lua_pushinteger(L,index);

			// value
			lua_pushinteger(L,buf[index-1]);

			// set the table entry
			lua_settable(L, -3);
		}

		// push the new table
		lua_pushvalue(L,-1);

		FREEMEM(buf);
	}

	// string
	if( size == -1 ) {

		UCHAR *buf;
		int maxsize = 1024*16;	// orosz rulett

		ALLOCMEM( buf, maxsize+1 );
		memset(buf,0L,maxsize+1);

		for( int i=0; i<maxsize; i++ ) {

			if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset+i),(void*)(buf+i),1,NULL) )
				break;

			if( buf[i] == 0 )
				break;
		}

		lua_pushstring(L, (const char*)buf);

		FREEMEM(buf);
	}


	// unicode string
	if( size == -2 ) {
		CloseHandle(hProcess);
		lua_pushstring(L, "ReadMemory_wrapper: TODO unicode");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// float
	if( size == -3 ) {

		float value = 0;

		if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&value,sizeof(float),NULL) ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "ReadMemory_wrapper: can't read process");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, value);
	}

	// double
	if( size == -4 ) {

		double value = 0;

		if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&value,sizeof(double),NULL) ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "ReadMemory_wrapper: can't read process");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, value);
	}

	// unsigned int
	if( size == -5 ) {

		UINT value = 0;

		if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&value,sizeof(UINT),NULL) ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "ReadMemory_wrapper: can't read process");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, value);
	}

	// integer
	if( size == -6 ) {

		int integer = 0;

		if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&integer,sizeof(int),NULL) ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "ReadMemory_wrapper: can't read process");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, integer);
	}

	// word
	if( size == -7 ) {

		short word = 0;

		if( !ReadProcessMemory(hProcess,(void*)(baseAddr+offset),(void*)&word,sizeof(short),NULL) ) {
			CloseHandle(hProcess);
			lua_pushstring(L, "ReadMemory_wrapper: can't read process");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, word);
	}

	CloseHandle(hProcess);

	return 1;
}




//
// DumpMemory( <window name, pid, exe>, [filename] )
//
static int DumpMemory_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "DumpMemory_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	DWORD pid;
	char *name = NULL;
	char *filename = NULL;

	if( lua_isnumber(L,1) )
		pid = (DWORD)lua_tonumber(L, 1);
	else
	if( lua_isstring(L,1) ) {

		name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "DumpMemory_wrapper: can't get window name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		HWND hwnd;
		if( !stricmp(name,"active") )
			hwnd = GetForegroundWindow();
		else
			hwnd = (HWND)XLIB_FindWindow(name);
		if( hwnd == NULL ) {
			if( (pid = XLIB_GetPidFromExe(name)) == 0 ) {
				lua_pushstring(L, "DumpMemory_wrapper: can't find window or process or pid");
				lua_error(L);
				lua_pushnumber(L, 0);
				return 1;
			}
		}
		else
			GetWindowThreadProcessId(hwnd,&pid);
	}

	// xprintf("pid: %d\n",pid);

	if( lua_isstring(L,2) )
		filename = (char *)lua_tostring(L, 2);

	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );

	if( hProcess == NULL ) {
		char str[512];
		sprintf(str,"DumpMemory_wrapper: can't open process %d", pid);
		lua_pushstring(L, str);
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( name )
		winShowPanel(name, "Dumping...");
	else {
		char str[100];
		sprintf(str, "process %d",pid);
		winShowPanel(str, "Dumping...");
	}

	// dump whole process memory
	// http://cpptutorials.freeforums.org/process-and-retriving-data-in-memory-t1564.html

	// XLIB_SetPrivileges();

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	ULONG MaxAppAdd = (ULONG)si.lpMaximumApplicationAddress;
	unsigned int StartAdd = (ULONG)si.lpMinimumApplicationAddress;

	winSetPanel(NULL,-1,MaxAppAdd-StartAdd);

	FILE *f = fopen(filename?filename:"dump.dat","wb");
	if( f == NULL ) {
		lua_pushstring(L, "DumpMemory_wrapper: can't open file");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	do {
		MEMORY_BASIC_INFORMATION mbi;

		if( VirtualQueryEx(hProcess, (LPCVOID)StartAdd, &mbi, sizeof(mbi)) == 0 )
			xprintf("VirtualQueryEx Error: %s\n",XLIB_GetWinError());

		DWORD Flags = PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
		if( mbi.State & MEM_COMMIT && mbi.Protect & Flags ) {

			DWORD start = (DWORD)mbi.BaseAddress;
			DWORD end = (DWORD)(start + mbi.RegionSize);

			// xprintf("read: from %d to %d (size: %d)\n", start, end, mbi.RegionSize);

			if( mbi.RegionSize > 0 ) {
				UCHAR *buf = NULL;
				int bufsize = mbi.RegionSize;
				DWORD readed = 0;

				ALLOCMEM( buf, bufsize );

				if( !ReadProcessMemory(hProcess,(void*)start,(void*)buf,bufsize,&readed) )
					; // xprintf("ReadProcessMemory error: %s\n",XLIB_GetWinError());

				if( readed > 0 )
					fwrite( buf, readed,1,f );

				FREEMEM(buf);
			}
		}

		StartAdd += mbi.RegionSize;

		winSetPanel(NULL,StartAdd-(ULONG)si.lpMinimumApplicationAddress);
	}
	while( StartAdd < MaxAppAdd );

	fclose(f);

	CloseHandle(hProcess);

	winHidePanel();

	return 0;
}


/*
typedef struct hwndcache_s {
	HWND hwnd;
	char name[256];
	BOOL flag;
} hwndcache_t, *hwndcache_ptr;

static hwndcache_t hwndcache[10] = {
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE,
	0, "", FALSE
};
*/


//
// SendKey( <(partial) window name | "lowlevel">, <key | "leftclick" | stb.>, [x | 100 ms], [y], [delay] )
//
static int SendKey_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "SendKey_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		lua_pushstring(L, "SendKey_wrapper: can't get window name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	HWND hwnd,activehwnd;
	int mode = 0;
	if( !strncmp(name,"exact:",6) ) {
		mode = 1;
		name += 6;
	}

	if( !strncmp(name,"other:",6) ) {
		mode = 2;
		name += 6;
		activehwnd = GetForegroundWindow();
	}

	if( !strncmp(name,"all:",4) ) {
		mode = 3;
		name += 4;
	}

	/*
	if( (strlen(name) == 8) && !stricmp(name,"keybd") ) {
		char *str;
		mode = 4;
		hwnd = (HWND)0xdeadbeef;
		// XLIB_HookKeyboardProc();
		if( lua_isstring(L, 2) && (str = (char *)lua_tostring(L, 2)) )
		for( int i=0; str && i<strlen(str); i++ ) {
			int c = VkKeyScan(str[i]);
			keybd_event( c, 0, 0,0 );
			Sleep(50);
			keybd_event( c, 0, KEYEVENTF_KEYUP, 0);
		}
		return 0;
	}
	*/

	if( (strlen(name) == 6) && !stricmp(name,"active") ) {
		// hwnd = GetForegroundWindow();
		hwnd = (HWND)XLIB_GetWindowUnderMouse();
	}
	else
		hwnd = (HWND)XLIB_FindWindow(name,mode);

	if( hwnd == NULL ) {
		// classra keres
		if( (hwnd = (HWND)XLIB_FindWindow(name,5)) == NULL ) {
			hwnd = (HWND)(int)lua_tonumber(L, 1);
			if( !IsWindow(hwnd) ) {
				lua_pushstring(L, "SendKey_wrapper: can't find window");
				lua_error(L);
				lua_pushnumber(L, 0);
				return 1;

			}
		}
	}

	/***
	if( mode == 2 || mode == 3 )
		hwnd == hwndcache[0].hwnd && hwndcache[0].flag == TRUE )
	***/

	do {	// minden ablaknak leküldi, csak az aktivnak nem
		if( (mode == 2) && (hwnd == activehwnd) )
			continue;

		// Notepad -> editbox
		/***
		HWND editx = FindWindowEx( hwnd, NULL, "edit", NULL );
		if( editx )
			hwnd = editx;
		***/

		char *str;

		/***
		// az "1" nála szám wtf
		if( lua_isnumber(L, 2) ) {

			int key = (int)lua_tonumber(L, 2);

			// PostMessage( hwnd, key, wparam, lparam );
			// SendMessage(hwnd, WM_CHAR, ' ', 1L);
		}
		else
		***/
		if( lua_isstring(L, 2) && (str = (char *)lua_tostring(L, 2)) ) {

			int x = 0;
			int y = 0;
			int keepdown = 50;
			POINT pt;
			int virtkey;

			GetCursorPos(&pt);
			ScreenToClient( hwnd, &pt );

			x = pt.x;
			y = pt.y;

			// SendKey( "WOW", "leftclick", 10,10 )
			if( n>=4 && lua_isnumber(L, 3) ) {
				int param_x = (int)lua_tonumber(L, 3);
				if( param_x > 0 )
					x = param_x;
			}

			if( n>=4 && lua_isnumber(L, 4) ) {
				int param_y = (int)lua_tonumber(L, 4);
				if( param_y > 0 )
					y = param_y;
			}

			// SendKey( "WOW", "F1", 50 )
			if( n==3 && lua_isnumber(L, 3) )
				keepdown = (int)lua_tonumber(L, 3);

			// SendKey( "WOW", "F1", 'down' || "up" )
			if( n==3 && lua_isstring(L, 3) ) {
				char *s = (char *)lua_tostring(L, 3);
				     if( !stricmp(s,"down") ) keepdown = -1;
				else if( !stricmp(s,"up") )   keepdown = -2;
			}

			// SendKey( "WOW", "leftclick", 10,10, 50 )
			if( n==5 && lua_isnumber(L, 5) )
				keepdown = (int)lua_tonumber(L, 5);

			// SendKey( "WOW", "leftclick", 10,10, 'down' || "up" )
			if( n==5 && lua_isstring(L, 5) ) {
				char *s = (char *)lua_tostring(L, 5);
				     if( !stricmp(s,"down") ) keepdown = -1;
				else if( !stricmp(s,"up") )   keepdown = -2;
			}

			if( !stricmp(str,"click") || !stricmp(str,"mouseclick") || !stricmp(str,"leftclick") || !stricmp(str,"mousebl") || !stricmp(str,"mouse1") || !stricmp(str,"lbutton") || !stricmp(str,"lclick") ) {
				// xprintf("left: %d, %d\n",x,y);
				if( keepdown != -2 ) PostMessage( hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x,y) );
				Delay( keepdown<=0?10:keepdown );
				if( keepdown >= 0 || keepdown == -2 ) { PostMessage( hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x,y) ); Delay(10); }
			}
			else
			if( !stricmp(str,"rightclick") || !stricmp(str,"mousebr") || !stricmp(str,"mouse2") || !stricmp(str,"rbutton") || !stricmp(str,"rclick") ) {
				// xprintf("right: %d, %d\n",x,y);
				if( keepdown != -2 ) PostMessage( hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x,y) );
				Delay( keepdown<=0?10:keepdown );
				if( keepdown >= 0 || keepdown == -2 ) { PostMessage( hwnd, WM_RBUTTONUP, 0, MAKELPARAM(x,y) ); Delay(10); }
			}
			else
			if( !stricmp(str,"middleclick") || !stricmp(str,"mousebm") || !stricmp(str,"mouse3") || !stricmp(str,"mbutton") || !stricmp(str,"mclick") ) {
				// xprintf("right: %d, %d\n",x,y);
				if( keepdown != -2 ) PostMessage( hwnd, WM_RBUTTONDOWN, MK_MBUTTON, MAKELPARAM(x,y) );
				Delay( keepdown<=0?10:keepdown );
				if( keepdown >= 0 || keepdown == -2 ) { PostMessage( hwnd, WM_MBUTTONUP, 0, MAKELPARAM(x,y) ); Delay(10); }
			}
			else
			if( !stricmp(str,"mousemove") ) {
				RECT rc;
				GetWindowRect(hwnd,&rc);
				SetCursorPos(rc.left+x,rc.top+y);
			}
			else
			if( (virtkey = StringToVirtkey(str)) != -1 ) {

#define KEYUP_FLAGS 0xC0000001

				if( keepdown != -2 ) PostMessage( hwnd, WM_KEYDOWN, virtkey, 1L );
				Delay( keepdown<=0?10:keepdown );
				if( keepdown >= 0 || keepdown == -2 ) { PostMessage( hwnd, WM_KEYUP, virtkey, KEYUP_FLAGS ); Delay(10); }
			}
			else
			if( !stricmp(str,"alt+enter") ) {
				// hackerance
				// PressKeyWithAlt
				int key = VK_RETURN;
				SendMessage(hwnd, WM_KEYDOWN,  VK_RETURN, 536870913 );
				Delay(100);
				SendMessage(hwnd, WM_KEYUP, VK_RETURN, 536870913 );
				xprintf("FIXME.\n");
			}
			else
			// string input: "blah-blah"
			for( int i=0; str && i<strlen(str); i++ ) {

				// ha megadta, hogy lennt kell tartani
				if( n>=3 && lua_isnumber(L, 3) ) {
					int c = VkKeyScan(str[i]);
					PostMessage( hwnd, WM_KEYDOWN, c, 1L );
					Delay( keepdown<=0?10:keepdown );
					PostMessage( hwnd, WM_KEYUP, c, KEYUP_FLAGS );
					Delay(10);
				}
				else {
					PostMessage( hwnd, WM_CHAR, str[i], 1 );
					Delay( keepdown<=0?10:keepdown );
				}

				XLIB_MinimalMessagePump();
			}
		}
	}
	while( (mode == 2 || mode == 3) && ((hwnd = (HWND)XLIB_FindNextWindow(name,mode)) != NULL) );

	return 0;
}





//
// SendInput( <key | "leftclick" | "mousemove" | stb.>, [x | 100 ms], [y], [delay] )
//
static int SendInput_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "SendInput_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *str;

	/***
	// az "1" nála szám wtf
	if( lua_isnumber(L, 1) ) {

		int key = (int)lua_tonumber(L, 1);

		// PostMessage( hwnd, key, wparam, lparam );
		// SendMessage(hwnd, WM_CHAR, ' ', 1L);
	}
	else
	***/
	if( lua_isstring(L, 1) && (str = (char *)lua_tostring(L, 1)) ) {

		int x = -1;
		int y = -1;
		int keepdown = 50;
		int virtkey;

		/***
		POINT pt;
		GetCursorPos(&pt);
		x = pt.x;
		y = pt.y;
		***/

		if( n>=3 && lua_isnumber(L, 2) ) {
			int param_x = (int)lua_tonumber(L, 2);
			if( param_x > 0 )
				x = param_x;
		}

		if( n>=3 && lua_isnumber(L, 3) ) {
			int param_y = (int)lua_tonumber(L, 3);
			if( param_y > 0 )
				y = param_y;
		}

		// SendInput( "leftclick", 100,100, 50 )
		if( n>=4 && lua_isnumber(L, 4) )
			keepdown = (int)lua_tonumber(L, 4);

		// SendInput( "leftclick", 50 )
		if( n==2 && lua_isnumber(L, 2) )
			keepdown = (int)lua_tonumber(L, 2);

		// SendInput( "leftclick", 'down' || "up" )
		if( n==2 && lua_isstring(L, 2) ) {
			char *s = (char *)lua_tostring(L, 2);
			     if( !stricmp(s,"down") ) keepdown = -1;
			else if( !stricmp(s,"up") )   keepdown = -2;
		}

		// xprintf("SendInput: \"%s\", %d, %d  delay: %d\n",str,x,y,keepdown);

		if( !stricmp(str,"click") || !stricmp(str,"mouseclick") || !stricmp(str,"leftclick") || !stricmp(str,"mousebl") || !stricmp(str,"mouse1") || !stricmp(str,"lbutton") || !stricmp(str,"lclick") ) {
			// xprintf("left: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 0, keepdown );
		}
		else
		if( !stricmp(str,"rightclick") || !stricmp(str,"mousebr") || !stricmp(str,"mouse2") || !stricmp(str,"rbutton") || !stricmp(str,"rclick") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 1, keepdown );
		else
		if( !stricmp(str,"middleclick") || !stricmp(str,"mousebm") || !stricmp(str,"mouse3") || !stricmp(str,"mbutton") || !stricmp(str,"mclick") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 2, keepdown );
		else
		if( !stricmp(str,"button4") || !stricmp(str,"mouseb4") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 3, keepdown );
		else
		if( !stricmp(str,"button5") || !stricmp(str,"mouseb5") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 4, keepdown );
		else
		if( !stricmp(str,"mousemove") )
			// xprintf("move: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, -1, keepdown );
		else
		if( (virtkey = StringToVirtkey(str)) != -1 ) {
			int scancode = StringToScancode(str);
			// xprintf("key: %s, %d, %d, %d\n",str,virtkey,scancode,keepdown);
			XLIB_SendKey(virtkey,scancode,keepdown);
		}
		else
		// string input: "blah-blah"
		for( int i=0; str && i<strlen(str); i++ ) {
			virtkey = VkKeyScan(str[i]);
			XLIB_SendKey(virtkey,-1,keepdown);
			// XLIB_MinimalMessagePump();
		}
	}

	return 0;
}




//
//
//
static int GetKeyState_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "GetKeyState_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		lua_pushstring(L, "GetKeyState_wrapper: can't get name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// joyslick
	     if( !stricmp(name, "joyb0") ) { lua_pushboolean(L, joyb[0]); return 1; }
	else if( !stricmp(name, "joyb1") ) { lua_pushboolean(L, joyb[1]); return 1; }
	else if( !stricmp(name, "joyb2") ) { lua_pushboolean(L, joyb[2]); return 1; }
	else if( !stricmp(name, "joyb3") ) { lua_pushboolean(L, joyb[3]); return 1; }
	else if( !stricmp(name, "joyb4") ) { lua_pushboolean(L, joyb[4]); return 1; }
	else if( !stricmp(name, "joyb5") ) { lua_pushboolean(L, joyb[5]); return 1; }
	else if( !stricmp(name, "joyb6") ) { lua_pushboolean(L, joyb[6]); return 1; }
	else if( !stricmp(name, "joyb7") ) { lua_pushboolean(L, joyb[7]); return 1; }
	else if( !stricmp(name, "joyb8") ) { lua_pushboolean(L, joyb[8]); return 1; }
	else if( !stricmp(name, "joyb9") ) { lua_pushboolean(L, joyb[9]); return 1; }
	else if( !stricmp(name, "joybd0") ) { lua_pushboolean(L, joybd[0]); return 1; }
	else if( !stricmp(name, "joybd1") ) { lua_pushboolean(L, joybd[1]); return 1; }
	else if( !stricmp(name, "joybd2") ) { lua_pushboolean(L, joybd[2]); return 1; }
	else if( !stricmp(name, "joybd3") ) { lua_pushboolean(L, joybd[3]); return 1; }
	else if( !stricmp(name, "joybd4") ) { lua_pushboolean(L, joybd[4]); return 1; }
	else if( !stricmp(name, "joybd5") ) { lua_pushboolean(L, joybd[5]); return 1; }
	else if( !stricmp(name, "joybd6") ) { lua_pushboolean(L, joybd[6]); return 1; }
	else if( !stricmp(name, "joybd7") ) { lua_pushboolean(L, joybd[7]); return 1; }
	else if( !stricmp(name, "joybd8") ) { lua_pushboolean(L, joybd[8]); return 1; }
	else if( !stricmp(name, "joybd9") ) { lua_pushboolean(L, joybd[9]); return 1; }

	int virtkey = StringToVirtkey(name);

	// igazi hardware key
	BOOL needhw = FALSE;
	if( n>=2 && lua_isstring(L, 2) ) {
		char *s = (char *)lua_tostring(L, 2);
		if( s && (!stricmp(s,"hw") || !stricmp(s,"hardware")) )
			needhw = TRUE;
	}

	if( virtkey == VK_CAPITAL ) {
		if( IsKeyToggledOn(VK_CAPITAL) )
			lua_pushboolean(L, TRUE );
		else
			lua_pushboolean(L, FALSE );
		return 1;
	}
	else
	if( virtkey == VK_SCROLL ) {
		if( IsKeyToggledOn(VK_SCROLL) )
			lua_pushboolean(L, TRUE );
		else
			lua_pushboolean(L, FALSE );
		return 1;
	}
	else
	if( virtkey == VK_NUMLOCK ) {
		if( IsKeyToggledOn(VK_NUMLOCK) )
			lua_pushboolean(L, TRUE );
		else
			lua_pushboolean(L, FALSE );
		return 1;
	}

	if( needhw == TRUE ) {

		XLIB_HookKeyboardProc();
		if( virtkey == VK_LBUTTON || virtkey == VK_MBUTTON || virtkey == VK_RBUTTON )
			XLIB_HookMouseProcLL();

		if( virtkey == -1 )
			virtkey = VkKeyScan(name[0]);

		if( hardware_keys )
			lua_pushboolean(L, (hardware_keys[virtkey]?TRUE:FALSE) );
		else
			lua_pushboolean(L, FALSE);

		return 1;
	}

	// GetSystemMetrics(SM_SWAPBUTTON)

	if( virtkey == -1 )
		virtkey = VkKeyScan(name[0]);

	if( IsKeyDownAsync(virtkey) )
		lua_pushboolean(L, TRUE );
	else
		lua_pushboolean(L, FALSE );

	return 1;
}



//
//
//
static int KeyWait_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "KeyWait_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		lua_pushstring(L, "KeyWait_wrapper: can't get name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int virtkey = 0;

	if( (virtkey = StringToVirtkey(name)) == -1 )
		virtkey = VkKeyScan(name[0]);

	while( IsKeyDownAsync(virtkey) )
		XLIB_MinimalMessagePump();

	return 0;
}



//
// rx,ry,window,x,y,hwnd,filename,classname = GetMouseInfo()
//
static int GetMouseInfo_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 0 ) {
		lua_pushstring(L, "GetMouseInfo_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	POINT pt;
	GetCursorPos(&pt);

	lua_pushnumber(L, pt.x);
	lua_pushnumber(L, pt.y);

	// hwnd = GetForegroundWindow();
	HWND hwnd = (HWND)XLIB_GetWindowUnderMouse();

	char String[512];
	if( !SendMessage(hwnd, WM_GETTEXT, sizeof(String), (LPARAM)String) )
		String[0] = 0;		// No window title

	/***
	// wchar_t *s = L"BUTTON";
	wchar_t *s = L"Explorer";
	XLIB_SetWindowTheme((__int64)hwnd,(char*)s,"");
	***/

	if( String[0] == 0 )
		lua_pushnil(L);
	else
		lua_pushstring(L, String);

	ScreenToClient( hwnd, &pt );

	lua_pushnumber(L, pt.x);
	lua_pushnumber(L, pt.y);

	lua_pushnumber(L, TOINT64(hwnd));

	char filename[XMAX_PATH];

	int ret = 6;

	if( XLIB_GetExeNameFromHwnd(TOINT64(hwnd),filename) ) {
		lua_pushstring(L, filename);
		++ret;
	}

	if( GetClassName( hwnd, String, sizeof(String)-1 ) ) {
		lua_pushstring(L, String);
		++ret;
	}

	return ret;
}



//
// SetMouseSpeed(2)
//
static int SetMouseSpeed_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 0 ) {
		lua_pushstring(L, "SetMouseSpeed_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( lua_isnumber(L, 1) ) {

		FLOAT f = (FLOAT)lua_tonumber(L, 1);

		XLIB_HookMouseProcLL();
		XLIB_SetMouseSpeed( f );
	}

	return 0;
}



//
//
//
static int MoveMouse_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "MoveMouse_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int x = lua_tonumber(L, 1);
	int y = lua_tonumber(L, 2);

	int anim = 0;
	if( n >= 4 )
		anim = lua_tonumber(L, 4) ? 1 : 0;

	BOOL relative = FALSE;
	BOOL needhw = FALSE;
	if( n>=3 && lua_isstring(L, 3) ) {
		char *s = (char *)lua_tostring(L, 3);
		if( s && (!stricmp(s,"hw") || !stricmp(s,"hardware")) ) {
			needhw = TRUE;
			XLIB_SendMouse( x, y, 0, -1, -3 );
			return 0;
		}
		if( s && (!stricmp(s,"window") || !stricmp(s,"client")) )
			relative = TRUE;
	}

	if( x != -1 && y != -1 ) {
		if( anim ) {
			/***
			POINT pt;
			GetCursorPos(&pt);

			lua_pushnumber(L, pt.x);
			lua_pushnumber(L, pt.y);
			***/
			SetCursorPos(x,y);
		}
		else {
			if( relative ) {
				HWND hwnd = (HWND)XLIB_GetWindowUnderMouse();

				POINT pt;
				pt.x = x;
				pt.y = y;

				ClientToScreen( hwnd, &pt );

				x = pt.x;
				y = pt.y;
			}
			SetCursorPos(x,y);
		}
	}

	if( n >= 3 && lua_isnumber(L, 3) ) {
		int z = lua_tonumber(L, 3);
		XLIB_SendMouse( -1, -1, z, -1, -3 );
	}

	return 0;
}




//
// hwnd = FindWindow("- GIMP")
//
static int FindWindow_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "FindWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		lua_pushstring(L, "FindWindow_wrapper: can't get name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int mode = 0;
	if( n>=2 && lua_isnumber(L, 2) )
		mode = (int)lua_tonumber(L, 2);

	if( !strncmp(name,"exact:",6) ) {
		mode = 1;
		name += 6;
	}

	HWND hwnd;

	// az aktív mindig active
	if( (strlen(name) == 6) && !stricmp(name,"active") ) {
		hwnd = GetForegroundWindow();
		lua_pushnumber(L, TOINT64(hwnd));
		return 1;
	}

	if( (hwnd = (HWND)XLIB_FindWindow(name,mode,TRUE)) != NULL ||
	    (hwnd = (HWND)XLIB_FindWindow(name,5,TRUE)) != NULL ) {		// classra
		// xprintf("FindWindow_wrapper: %d\n",TOINT64(hwnd));
		lua_pushnumber(L, TOINT64(hwnd));
	}
	else
		lua_pushnil(L);

	// xprintf("h2: %d\n",hwnd);

	return 1;
}



//
// hwnd = FindNextWindow(hwnd)
//
static int FindNextWindow_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "FindNextWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int hwnd = (int)lua_tonumber(L,1);

	if( (hwnd = XLIB_FindNextHWND(hwnd,TRUE)) != 0 )
		lua_pushnumber(L, TOINT64(hwnd));
	else
		lua_pushnil(L);

	return 1;
}




//
//
//
static int SetWindowTitle_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "SetWindowTitle_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	__int64 hwnd;

	if( lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		hwnd = (__int64)ftoi(val);
	}
	else {
		char *name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "SetWindowTitle_wrapper: can't get window name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		if( (strlen(name) == 6) && !stricmp(name,"active") )
			hwnd = (__int64)GetForegroundWindow();
		else
			hwnd = (__int64)XLIB_FindWindow(name);
		if( hwnd == 0 ) {
			lua_pushstring(L, "SetWindowTitle_wrapper: can't find window");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}
	}
	char *title = (char *)lua_tostring(L, 2);
	if( title == NULL ) {
		lua_pushstring(L, "SetWindowTitle_wrapper: can't get title");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	SendMessage( (HWND)hwnd, WM_SETTEXT, 0, (LPARAM)title );

	return 0;
}



//
//
//
static int CaptureWindow_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "CaptureWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	__int64 hwnd;

	if( lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		hwnd = (__int64)ftoi(val);
	}
	else
	if( lua_isstring(L, 1) ) {

		char *name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "CaptureWindow_wrapper: can't get window name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		if( (strlen(name) == 7) && !stricmp(name,"desktop") )
			hwnd = -1;
		else
		if( (strlen(name) == 9) && !stricmp(name,"clipboard") )
			hwnd = -2;
		else
		if( (strlen(name) == 6) && !stricmp(name,"active") )
			hwnd = (__int64)GetForegroundWindow();
		else {
			hwnd = (__int64)XLIB_FindWindow(name);
			if( hwnd == 0 ) {
				lua_pushstring(L, "CaptureWindow_wrapper: can't find window");
				lua_error(L);
				lua_pushnumber(L, 0);
				return 1;
			}
		}
	}
	else
		hwnd = (__int64)GetForegroundWindow();

	UCHAR *spr = CaptureWindow(hwnd);
	if( spr == NULL ) {
		lua_pushstring(L, "CaptureWindow_wrapper: can't get sprite");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int handle = LOADBIGSPRITEFROMMEM(spr);
	SAFE_FREEMEM(spr);

	lua_pushnumber(L, handle);

	return 1;
}




//
// ActiveWindow("Total Commander") = True,False
// ActiveWindow() = hwnd of active window
//
static int ActiveWindow_wrapper( lua_State *L ) {

	LUA_DEBUG_START(L);

	HWND hwnd;

	int n = lua_gettop(L);
	if( n < 1 ) {
		/***
		lua_pushstring(L, "ActiveWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		***/
		hwnd = GetForegroundWindow();
		lua_pushnumber(L, TOINT64(hwnd));
		char str[512];
		if( SendMessage(hwnd, WM_GETTEXT, sizeof(str), (LPARAM)str) ) {
			lua_pushstring(L, str);
			return 1;
		}
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		// lua_pushstring(L, "ActiveWindow_wrapper: can't get window name");
		// lua_error(L);
		// lua_pushnumber(L, 0);
		lua_pushboolean(L, FALSE );
		return 1;
	}

	int mode = 0;
	if( n>=2 && lua_isnumber(L, 2) )
		mode = (int)lua_tonumber(L, 2);

	if( !strncmp(name,"exact:",6) ) {
		mode = 1;
		name += 6;
	}

	// az aktív mindig active
	if( (strlen(name) == 6) && !stricmp(name,"active") ) {
		lua_pushboolean(L, TRUE );
		return 1;
	}

	/***
	hwnd = (HWND)XLIB_FindWindow(name,mode);
	if( hwnd == NULL ) {
		lua_pushboolean(L, FALSE );
		return 1;
	}

	if( GetForegroundWindow() == hwnd )
		lua_pushboolean(L, TRUE );
	else
	if( GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST )
		lua_pushboolean(L, TRUE );
	else
		lua_pushboolean(L, FALSE );
	***/

	hwnd = GetForegroundWindow();
	if( IsWindowVisible(hwnd) /*&& (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)*/ ) {
		char str[512];
		SendMessage(hwnd, WM_GETTEXT, sizeof(str), (LPARAM)str);

		// a végéről a spaceket
		for( int i=strlen(str)-1; i>=0; i-- )
			if( str[i] == ' ' )
				str[i] = 0;
			else
				break;

		if( mode == 1 )
			if( !strcmp( str, name ) && (strlen(str) == strlen(name)) ) {
				lua_pushboolean(L, TRUE );
				return 1;
			}

		if( mode == 0 )
			if( stristr( str, name ) ) {
				lua_pushboolean(L, TRUE );
				return 1;
			}

		if( GetClassName( hwnd, str, sizeof(str)-1 ) && !stricmp( str, name ) ) {
			lua_pushboolean(L, TRUE );
			return 1;
		}
	}
	// else
	//	xprintf("dafuq\n");

	lua_pushboolean(L, FALSE );

	LUA_DEBUG_END(L, 1);

	return 1;
}



//
// x,y,w,h,name,pid = GetWindowInfo("desktop")
//
static int GetWindowInfo_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		// lua_pushstring(L, "GetWindowRect_wrapper: not enough arguments");
		// lua_error(L);
		// lua_pushnumber(L, 0);
		// return 1;
		lua_pushnumber(L, 0 );
		lua_pushnumber(L, 0 );
		lua_pushnumber(L, GetSystemMetrics( SM_CXSCREEN ) );
		lua_pushnumber(L, GetSystemMetrics( SM_CYSCREEN ) );
		lua_pushstring(L, "desktop");
		lua_pushnumber(L, 1 );
		return 6;
	}

	HWND hwnd = NULL;
	char *name = NULL;

	if( lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		hwnd = (HWND)ftoi(val);
	}
	else
	if( lua_isstring(L, 1) ) {

		name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "GetWindowRect_wrapper: can't get window name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		int mode = 0;
		if( n>=2 && lua_isnumber(L, 2) )
			mode = (int)lua_tonumber(L, 2);

		if( !strncmp(name,"exact:",6) ) {
			mode = 1;
			name += 6;
		}

		// az aktív mindig active
		if( (strlen(name) == 6) && !stricmp(name,"active") )
			hwnd = GetForegroundWindow();
		else
			hwnd = (HWND)XLIB_FindWindow(name,mode);
	}

	if( hwnd == NULL ) {
		if( name && !stricmp(name,"desktop") ) {
			// desktop size ha nincs ilyen window
			lua_pushnumber(L, 0 );
			lua_pushnumber(L, 0 );
			lua_pushnumber(L, GetSystemMetrics( SM_CXSCREEN ) );
			lua_pushnumber(L, GetSystemMetrics( SM_CYSCREEN ) );
			lua_pushstring(L, "desktop");
			lua_pushnumber(L, 1 );
			return 6;
		}
		lua_pushstring(L, "GetWindowRect_wrapper: can't find window");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	RECT rc;
	if( !GetWindowRect(hwnd,&rc) ) {
		lua_pushstring(L, "GetWindowRect_wrapper: can't get size");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, rc.left );
	lua_pushnumber(L, rc.top );
	lua_pushnumber(L, (rc.right - rc.left) );
	lua_pushnumber(L, (rc.bottom - rc.top) );

	char string[512];
	int result = GetWindowText( hwnd, string, sizeof(string)-1 );
	lua_pushstring(L, string);

	DWORD pid;
	GetWindowThreadProcessId(hwnd,&pid);
	lua_pushnumber(L, pid );

	return 6;
}




//
//
//
static int WinWait_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "WinWait_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		lua_pushstring(L, "WinWait_wrapper: can't get name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int mode = 0;
	if( n>=2 && lua_isnumber(L, 2) )
		mode = (int)lua_tonumber(L, 2);

	if( !strncmp(name,"exact:",6) ) {
		mode = 1;
		name += 6;
	}

	// az aktív mindig active
	if( (strlen(name) == 6) && !stricmp(name,"active") )
		return 0;

	HWND hwnd;
	while( (hwnd = (HWND)XLIB_FindWindow(name,mode)) == NULL ) {
		Delay(10);
	}

	return 0;
}




//
//
//
static int ActivateWindow_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "ActivateWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	HWND target;

	// HWND
	if( lua_isnumber(L, 1) ) {
		if( (target = (HWND)(int)lua_tonumber(L, 1)) == NULL ) {
			lua_pushboolean(L, FALSE);
			return 1;
		}
	}
	else {
		char *name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "ActivateWindow_wrapper: can't get name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		int mode = 0;
		if( n>=2 && lua_isnumber(L, 2) )
			mode = (int)lua_tonumber(L, 2);

		if( !strncmp(name,"exact:",6) ) {
			mode = 1;
			name += 6;
		}

		// az aktív mindig active
		if( (strlen(name) == 6) && !stricmp(name,"active") )
			return 0;

		target = (HWND)XLIB_FindWindow(name,mode);
		if( target == NULL ) {
			/***
			lua_pushstring(L, "ActivateWindow_wrapper: can't get window");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
			***/
			// akkor classname-re
			if( (target = (HWND)XLIB_FindWindow(name,5)) == NULL ) {
				lua_pushboolean(L, FALSE);
				return 1;
			}
		}
	}

	while( GetForegroundWindow() != target ) {
		ShowWindow(target,SW_RESTORE);
		SetActiveWindow(target);
		SetForegroundWindow(target);
		Delay(10);
	}

	lua_pushboolean(L, TRUE);
	return 1;
}




//
// MoveWindow( <text,hwnd>, x,y, [w,h] )
//
static int MoveWindow_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "MoveWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	HWND hwnd;

	if( lua_isnumber(L, 1) ) {
		double val = lua_tonumber(L, 1);
		hwnd = (HWND)ftoi(val);
	}
	else {

		char *name = (char *)lua_tostring(L, 1);
		if( name == NULL ) {
			lua_pushstring(L, "MoveWindow_wrapper: can't get name");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}

		int mode = 0;
		if( !strncmp(name,"exact:",6) ) {
			mode = 1;
			name += 6;
		}

		// az aktív mindig active
		if( (strlen(name) == 6) && !stricmp(name,"active") )
			hwnd = GetForegroundWindow();
		else
			hwnd = (HWND)XLIB_FindWindow(name,mode);

		if( hwnd == NULL )
			if( (hwnd = (HWND)XLIB_FindWindow(name,5)) == NULL ) {
				lua_pushstring(L, "MoveWindow_wrapper: can't find window");
				lua_error(L);
				lua_pushnumber(L, 0);
				return 1;
			}
	}

	// xprintf("hwnd: %d\n", hwnd);

	RECT rc;
	if( !GetWindowRect(hwnd,&rc) )
		return 0;

	if( n>=2 && lua_isnumber(L, 2) )
		rc.left = (int)lua_tonumber(L, 2);

	if( n>=3 && lua_isnumber(L, 3) )
		rc.top = (int)lua_tonumber(L, 3);

	if( n>=4 && lua_isnumber(L, 4) )
		rc.right = (int)lua_tonumber(L, 4) + rc.left;

	if( n>=5 && lua_isnumber(L, 5) )
		rc.bottom = (int)lua_tonumber(L, 5) + rc.top;

	MoveWindow( hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE );
	Delay(10);

	/***
	lua_pushnumber(L, rc.left );
	lua_pushnumber(L, rc.top );
	lua_pushnumber(L, (rc.right - rc.left) );
	lua_pushnumber(L, (rc.bottom - rc.top) );
	***/

	return 0;
}



//
// KillWindow( "name" or HWND )
//
static int KillWindow_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "KillWindow_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *name = (char *)lua_tostring(L, 1);
	if( name == NULL ) {
		lua_pushstring(L, "KillWindow_wrapper: can't get name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int mode = 0;
	if( !strncmp(name,"exact:",6) ) {
		mode = 1;
		name += 6;
	}

	HWND hwnd;

	// az aktív mindig active
	if( (strlen(name) == 6) && !stricmp(name,"active") )
		hwnd = GetForegroundWindow();
	else
		hwnd = (HWND)XLIB_FindWindow(name,mode);

	if( hwnd == NULL ) {
		lua_pushstring(L, "KillWindow_wrapper: can't get window");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( XLIB_KillWindow(TOINT64(hwnd)) == FALSE ) {
		lua_pushstring(L, "KillWindow_wrapper: can't kill");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	return 0;
}



//
//
//
static int Quit_wrapper( lua_State *L ) {

	int n = lua_gettop(L);

	char *str = NULL;

	if( n >= 1 )
		str = (char *)lua_tostring(L, 1);

	Quit(str);

	return 0;
}




//
// DrawTexRect( x1,y1, w,h, 1, 170 )
//
static int DrawTexRect_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 4 ) {
		lua_pushstring(L, "DrawTexRect_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int x = ftoi( lua_tonumber(L, 1) );
	int y = ftoi( lua_tonumber(L, 2) );
	int w = ftoi( lua_tonumber(L, 3) );
	int h = ftoi( lua_tonumber(L, 4) );

	int color = 0;
	if( n > 4 )
		color = ftoi( lua_tonumber(L, 5) );

	int alpha = 255;
	if( n > 5 )
		alpha = ftoi( lua_tonumber(L, 6) );

	DrawTexRect( x,y, x+w,y+h, color, alpha );

	return 0;
}




/***
	if( SetClassLong( hwnd, GCL_HCURSOR, NULL ) == 0 ) {
		char str[512];
		sprintf(str,"SetWindowCursor_wrapper: %s",XLIB_GetWinError());
		lua_pushstring(L, str);
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}
***/


//
// Overlay( <"string" or "bernie ctrl text">, r,g,b,a, x,y,w,h )
//
static int Overlay_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 9 ) {
		lua_pushstring(L, "Overlay_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// stricmp(s,"bernie ctrl text")

	char *str = (char *)lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "Overlay_wrapper: can't get string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	int r = -1;
	int g = -1;
	int b = -1;
	int a = -1;

	if( n>=2 && lua_isnumber(L, 2) )
		r = (int)lua_tonumber(L, 2);
	if( n>=3 && lua_isnumber(L, 3) )
		g = (int)lua_tonumber(L, 3);
	if( n>=4 && lua_isnumber(L, 4) )
		b = (int)lua_tonumber(L, 4);
	if( n>=5 && lua_isnumber(L, 5) )
		a = (int)lua_tonumber(L, 5);

	int x = -1;
	int y = -1;
	int w = -1;
	int h = -1;

	if( n>=6 && lua_isnumber(L, 6) )
		x = (int)lua_tonumber(L, 6);
	if( n>=7 && lua_isnumber(L, 7) )
		y = (int)lua_tonumber(L, 7);
	if( n>=8 && lua_isnumber(L, 8) )
		w = (int)lua_tonumber(L, 8);
	if( n>=9 && lua_isnumber(L, 9) )
		h = (int)lua_tonumber(L, 9);

	SetOverlayParam( a,r,g,b, x,y,w,h );
	Overlay(str);

	// extern void AlphaWindow( __int64 in_hwnd, FLOAT gamma );
	// AlphaWindow( TOINT64(GetForegroundWindow()), 1.2 );

	return 0;
}




//
//
//
static int Browser_wrapper( lua_State *L ) {

	// ne zárja le ha lesz browser
	screen_activity = TRUE;

	int n = lua_gettop(L);
	if( n<1 ) {
		winBrowser();
		return 0;
	}

	char *str = (char *)lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "Browser_wrapper: can't get string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( FileExist(str,TRUE) ) {
		char filename[XMAX_PATH+1];
		if( Path2URL(str, filename) == FALSE ) {
			lua_pushstring(L, "Browser_wrapper: can't convert filename");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}
		winBrowser(filename);
	}
	else
		winBrowser(str);

	return 0;
}


//
//
//
static int GetBrowser_wrapper( lua_State *L ) {

	char *str;
	int maxlen = 1000*1000;

	ALLOCMEM( str, maxlen );

	if( winBrowserGetPage( str, maxlen ) ) {

		lua_pushstring(L, str );

		FREEMEM(str);

		return 1;
	}

	FREEMEM(str);

	return 0;
}




//
// BrowserReady(): blokkolja a futást
//
static int BrowserReady_wrapper( lua_State *L ) {

	lua_pushboolean(L, winBrowserReady() );

	return 1;
}


//
//
//
static int BrowserClick_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n<1 ) {
		lua_pushstring(L, "BrowserClick_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *str = (char *)lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "BrowserClick_wrapper: can't get string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	winBrowserClick( str );

	return 0;
}



//
//
//
static int BrowserSetValue_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n<2 ) {
		lua_pushstring(L, "BrowserSetValue_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *tag = (char *)lua_tostring(L, 1);
	if( tag == NULL ) {
		lua_pushstring(L, "BrowserSetValue_wrapper: can't get tag");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *str = (char *)lua_tostring(L, 2);
	if( str == NULL ) {
		lua_pushstring(L, "BrowserSetValue_wrapper: can't get string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	winBrowserSetValue( tag, str );

	return 0;
}


//
//
//
static int BrowserComboBox_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n<2 ) {
		lua_pushstring(L, "BrowserComboBox_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *tag = (char *)lua_tostring(L, 1);
	if( tag == NULL ) {
		lua_pushstring(L, "BrowserComboBox_wrapper: can't get tag");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 2);
	int ival = ftoi(val);

	winBrowserComboBox( tag, ival );

	return 0;
}



//
//
//
static int BrowserSendKey_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n<1 ) {
		lua_pushstring(L, "BrowserSendKey_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	HWND hwnd = (HWND)winBrowser();
	if( hwnd == NULL ) {
		lua_pushstring(L, "BrowserClick_wrapper: no browser");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *str;
	if( lua_isstring(L, 1) && (str = (char *)lua_tostring(L, 1)) ) {

		int x = 0;
		int y = 0;
		int keepdown = 50;
		POINT pt;
		int virtkey;

		GetCursorPos(&pt);
		ScreenToClient( hwnd, &pt );

		x = pt.x;
		y = pt.y;

		// SendKey( "WOW", "leftclick", 10,10 )
		if( n>=3 && lua_isnumber(L, 2) ) {
			int param_x = (int)lua_tonumber(L, 2);
			if( param_x > 0 )
				x = param_x;
		}

		if( n>=3 && lua_isnumber(L, 3) ) {
			int param_y = (int)lua_tonumber(L, 3);
			if( param_y > 0 )
				y = param_y;
		}

		// SendKey( "WOW", "F1", 50 )
		if( n==2 && lua_isnumber(L, 2) )
			keepdown = (int)lua_tonumber(L, 2);

		// SendKey( "WOW", "F1", 'down' || "up" )
		if( n==2 && lua_isstring(L, 2) ) {
			char *s = (char *)lua_tostring(L, 2);
			     if( !stricmp(s,"down") ) keepdown = -1;
			else if( !stricmp(s,"up") )   keepdown = -2;
		}

		// SendKey( "WOW", "leftclick", 10,10, 50 )
		if( n==4 && lua_isnumber(L, 4) )
			keepdown = (int)lua_tonumber(L, 4);

		// SendKey( "WOW", "leftclick", 10,10, 'down' || "up" )
		if( n==4 && lua_isstring(L, 4) ) {
			char *s = (char *)lua_tostring(L, 4);
			     if( !stricmp(s,"down") ) keepdown = -1;
			else if( !stricmp(s,"up") )   keepdown = -2;
		}

		if( !stricmp(str,"click") || !stricmp(str,"mouseclick") || !stricmp(str,"leftclick") || !stricmp(str,"mousebl") || !stricmp(str,"mouse1") || !stricmp(str,"lbutton") || !stricmp(str,"lclick") ) {
			// xprintf("left: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 0, keepdown );
		}
		else
		if( !stricmp(str,"rightclick") || !stricmp(str,"mousebr") || !stricmp(str,"mouse2") || !stricmp(str,"rbutton") || !stricmp(str,"rclick") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 1, keepdown );
		else
		if( !stricmp(str,"middleclick") || !stricmp(str,"mousebm") || !stricmp(str,"mouse3") || !stricmp(str,"mbutton") || !stricmp(str,"mclick") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 2, keepdown );
		else
		if( !stricmp(str,"button4") || !stricmp(str,"mouseb4") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 3, keepdown );
		else
		if( !stricmp(str,"button5") || !stricmp(str,"mouseb5") )
			// xprintf("right: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, 4, keepdown );
		else
		if( !stricmp(str,"mousemove") )
			// xprintf("move: %d, %d\n",x,y);
			XLIB_SendMouse( x, y, 0, -1, keepdown );
		else
		if( (virtkey = StringToVirtkey(str)) != -1 ) {
			int scancode = StringToScancode(str);
			// xprintf("key: %s, %d, %d, %d\n",str,virtkey,scancode,keepdown);
			XLIB_SendKey(virtkey,scancode,keepdown);
		}
		else
		// string input: "blah-blah"
		for( int i=0; str && i<strlen(str); i++ ) {
			virtkey = VkKeyScan(str[i]);
			XLIB_SendKey(virtkey,-1,keepdown);
			// XLIB_MinimalMessagePump();
		}
	}

	return 0;
}




//
// MessageBox(HWND,LPCSTR,LPCSTR,UINT);
// DllCall("user32/MessageBox", "int", nil, "char*", "valami", "char *", "text", "int", 0)
//
static int DllCall_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 1 ) {
		lua_pushstring(L, "DllCall_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *str = (char *)lua_tostring(L, 1);
	if( str == NULL ) {
		lua_pushstring(L, "DllCall_wrapper: can't get name");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char dllname[XMAX_PATH*2];
	char procname[XMAX_PATH];

	strcpy( dllname, str );

	char *p = strchr(dllname,'/');
	if( !p ) {
		strcpy(procname,dllname);
		dllname[0] = 0;
	}
	else {
		strcpy(procname,p+1);
		*p = 0;
	}

	HMODULE hDll;
	void *proc;
	static HMODULE sStdModule[] = { GetModuleHandle("user32"),
					GetModuleHandle("kernel32"),
					GetModuleHandle("comctl32"),
					GetModuleHandle("gdi32") }; // user32 is listed first for performance.
	static int sStdModule_count = sizeof(sStdModule) / sizeof(HMODULE);

	if( dllname[0] ) {
		hDll = GetModuleHandle(dllname);
		if( hDll == NULL ) {
			lua_pushstring(L, "DllCall_wrapper: can't load DLL");
			lua_error(L);
			lua_pushnumber(L, 0);
			return 1;
		}
		proc = (void *)GetProcAddress( hDll, procname );
	}
	else {
		int i;
		// Since no DLL was specified, search for the specified function among the standard modules.
		for( i=0; i<sStdModule_count; i++ )
			if( sStdModule[i] && (proc = (void *)GetProcAddress(sStdModule[i], procname)) )
				break;
		if( !proc ) {
			// Since the absence of the "A" suffix (e.g. MessageBoxA) is so common,
			// try it that way but only here with the standard libraries since the
			// risk of ambiguity (calling the wrong function) seems unacceptably
			// high in a custom DLL.  For example, a custom DLL might have
			// function called "AA" but not one called "A".
			strcat(procname, "A"); // 1 byte of memory was already reserved above for the 'A'.
			for( i=0; i<sStdModule_count; i++ )
				if( sStdModule[i] && (proc = (void *)GetProcAddress(sStdModule[i], procname)) )
					break;
		}
		if( proc )
			sprintf(dllname,"sStdModule[%d]",i);
	}

	if( proc == NULL ) {
		lua_pushstring(L, "DllCall_wrapper: can't get function");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	xprintf("DllCall: %s, %s\n",dllname,procname);

	// return_value = DynaCall(dll_call_mode, function, dyna_param, arg_count, exception_occurred, NULL, 0);

	lua_pushnumber(L, 0);

	return 1;
}



//
//
//
static int WriteText_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 3 ) {
		lua_pushstring(L, "WriteText_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// X
	if( !lua_isnumber(L, 1) ) {
		lua_pushstring(L, "WriteText_wrapper: X should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	double val = lua_tonumber(L, 1);
	int x = ftoi(val);

	// Y
	if( !lua_isnumber(L, 2) ) {
		lua_pushstring(L, "WriteText_wrapper: Y should be number");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	val = lua_tonumber(L, 2);
	int y = ftoi(val);

	// str
	const char *str = lua_tostring(L, 3);
	if( str == NULL ) {
		lua_pushstring(L, "WriteText_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	WriteString( x,y, (char *)str );
	screen_activity = TRUE;

	lua_pushnumber(L, 1);

	return 1;
}





static BOOL bElso = TRUE;

//
// BOOL CacheSubdir( char *filename = NULL, char *pattern = NULL, BOOL nosubdir = FALSE, BOOL scan_zips = FALSE );
//
static int CacheSubdir_wrapper( lua_State *L ) {

	int n = lua_gettop(L);

	char *filename = NULL;
	if( lua_isstring(L,1) )
		filename = (char *)lua_tostring(L, 1);

	char *pattern = NULL;
	if( lua_isstring(L,2) )
		pattern = (char *)lua_tostring(L, 2);

	BOOL nosubdir = TRUE;
	if( lua_isboolean(L,3) )
		nosubdir = (BOOL)lua_toboolean(L, 3);

	BOOL scan_zips = FALSE;
	if( lua_isboolean(L,4) )
		scan_zips = (BOOL)lua_toboolean(L, 4);

	BOOL result = CacheSubdir(filename,pattern,nosubdir,scan_zips);

	bElso = TRUE;

	lua_pushboolean(L, result);

	return 1;
}


//
// char *SearchNextFile( char *curr_filename, int merre=1, sort_e sort=SORT_NAME,
//				int *currEntry=NULL, int *maxEntry=NULL,
//				char *pattern=NULL );
//
static int SearchNextFile_wrapper( lua_State *L ) {

	int n = lua_gettop(L);

	char *curr_filename = NULL;
	if( lua_isstring(L,1) )
		curr_filename = (char *)lua_tostring(L, 1);

	int merre = 1;
	if( lua_isnumber(L,2) )
		merre = ftoi( (FLOAT)lua_tonumber(L, 2) );

	int sort = SORT_NAME;
	if( lua_isnumber(L,2) )
		sort = ftoi( (FLOAT)lua_tonumber(L, 2) );

	char *ret;

	if( bElso == TRUE )
		ret = SearchNextFile( NULL, SNF_BEGINDIR, (sort_e)sort );
	else {
		int currEntry,maxEntry;
		SearchNextFile( NULL, SNF_QUERY, (sort_e)sort, &currEntry, &maxEntry );

		int nextEntry;
		ret = SearchNextFile(curr_filename,merre,(sort_e)sort,&nextEntry);

		// ha visszaugrik az elejére az ne legyen jó
		if( nextEntry <= currEntry )
			ret = NULL;
	}

	if( ret )
		lua_pushstring(L,ret);
	else
		lua_pushnil(L);

	bElso = FALSE;

	return 1;
}



//
//
//
static int MoveFile_wrapper( lua_State *L ) {

	int n = lua_gettop(L);
	if( n < 2 ) {
		lua_pushstring(L, "MoveFile_wrapper: not enough arguments");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *filename = (char *)lua_tostring(L, 1);
	if( filename == NULL ) {
		lua_pushstring(L, "MoveFile_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	char *dst = (char *)lua_tostring(L, 2);
	if( dst == NULL ) {
		lua_pushstring(L, "MoveFile_wrapper: can't convert to string");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	if( XL_CopyFile( filename, dst, CF_OVERWRITE, FALSE ) == FALSE ) {
		lua_pushstring(L, "MoveFile_wrapper: can't copy file");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	// delete
	if( UnlinkFile( filename ) == FALSE ) {
		lua_pushstring(L, "MoveFile_wrapper: can't delete file");
		lua_error(L);
		lua_pushnumber(L, 0);
		return 1;
	}

	lua_pushnumber(L, 1);

	return 1;
}



//
//
//
BOOL LUA_HasFunc( char *funcname ) {

	if( Lua_VM == NULL )
		return FALSE;

	lua_getglobal(Lua_VM, funcname);
	if( !lua_isfunction(Lua_VM,-1) ) {
		lua_pop(Lua_VM,1);
		return FALSE;
	}

	lua_pop(Lua_VM,1);

	return TRUE;
}


//
//
//
BOOL LUA_RunFunc( char *funcname, int param1, int param2, int param3, int param4 ) {

	if( Lua_VM == NULL ) {
		// xprintf("LUA_Render: Lua not inited.\n" );
		return FALSE;
	}

	lua_getglobal(Lua_VM, funcname);
	if( !lua_isfunction(Lua_VM,-1) ) {
		// xprintf("LUA_Render: no Render function.\n" );
		lua_pop(Lua_VM,1);
		return FALSE;
	}

	lua_pushnumber(Lua_VM, param1);   /* push 1st argument */
	lua_pushnumber(Lua_VM, param2);   /* push 2nd argument */
	lua_pushnumber(Lua_VM, param3);
	lua_pushnumber(Lua_VM, param4);

	//                    2  1   do the call (2 arguments, 1 result)
	if( (last_return = lua_pcall(Lua_VM, 4, 0, 0)) != 0 ) {
		// xprintf("LUA_Render: error running Render function: %s\n", lua_tostring(Lua_VM, -1) );
		printError();
		return FALSE;
	}

	// lua_call(Lua_VM, 0, 0);

	/***
	// retrieve result
	if (!lua_isnumber(L, -1)) {
		printf("function `f' must return a number\n");
		return -1;
	}

	z = lua_tonumber(L, -1);
	printf("Result: %f\n",z);

	lua_pop(L, 1);
	***/

	return TRUE;
}



//
//
//
lua_State *LUA_GetVM( void ) {

	if( Lua_VM == NULL )
		xprintf("LUA_GetVM: Lua not inited.\n" );

	return Lua_VM;
}



//
// int func( lua_State *L );
//
BOOL LUA_AddFunc( char *name, void *func, lua_State *vm ) {

	if( Lua_VM == NULL && vm == NULL )
		return FALSE;

	if( name == NULL || func == NULL )
		return FALSE;

	lua_State *v = vm ? vm : Lua_VM;

	lua_register( v, name, (lua_CFunction)func );

	return TRUE;
}



//
//
//
static void *xlib_alloc( void *ud, void *ptr, size_t osize, size_t nsize ) {

	if( nsize == 0 ) {
		// xprintf("free: ======\n");
		// SAFE_FREEMEM(ptr);
		if( ptr ) free(ptr);
		return NULL;
	}

	/*
	xprintf("realloc: %d -> %d\n",osize,nsize );
	REALLOCMEM(ptr, nsize);
	return ptr;
	*/

	return realloc(ptr, nsize);
}


//
//
//
BOOL LUA_Init( void ) {

	if( hDll == NULL ) {

		char luadllname[XMAX_PATH];

		sprintf( luadllname, "%s%c%s", GetExeDir(), PATHDELIM, LUADLLNAME );

		if( (hDll = LoadLibrary( luadllname )) == NULL ) {
			Quit( "LUA_Init: can't load \"%s\" library. Please, reinstall application.\nThank You!", LUADLLNAME );
			return FALSE;
		}

		if( 0 ) {
		// crc check
		char *buf;
		int size = LoadFile(luadllname,PTR(buf));
		UINT crc = 0;
		CalcCrc( buf, size, &crc);
		// if( crc != 2704477673 ) {	// 5.1
		if( crc != 1206273931 ) {	// 5.2
			// Quit( "Install is damaged.\n\nPlease, reinstall application.\n\nThank You!" );
			VAPI( FreeLibrary( hDll ) );
			hDll = NULL;
 			return FALSE;
		}
		FREEMEM(buf);
		}

		if( !luaL_loadfunctions(hDll, &LuaFunctions, sizeof(LuaFunctions)) ) {
			Quit( "LUA_Init: error loading functions.\n");
			return FALSE;
		}

		xprintf("LUA_Init: Lua (%s) loaded.\n", luadllname );

		/***
		if( Lua_VM ) lua_close( Lua_VM );
		Lua_VM = NULL;

		if( (Lua_VM = lua_newstate(xlib_alloc,NULL)) != NULL ) {
			char *ver = (char *)lua_version(Lua_VM);
			xprintf("version: %s\n",ver);
		}
		***/
	}

	if( Lua_VM ) lua_close( Lua_VM );
	Lua_VM = NULL;

	if( (Lua_VM = lua_newstate(xlib_alloc,NULL)) == NULL ) {
		xprintf("LUA_Init: Error Initializing Lua.\n");
		return FALSE;
	}

	luaL_openlibs(Lua_VM);

	lua_register(Lua_VM, "PlaySound", PlaySound_wrapper);
	lua_register(Lua_VM, "StopSound", StopSound_wrapper);
	lua_register(Lua_VM, "SetFont", SetFont_wrapper);
	lua_register(Lua_VM, "WriteText", WriteText_wrapper);
	lua_register(Lua_VM, "GetValue", GetValue_wrapper);
	lua_register(Lua_VM, "PutSprite", PutSprite_wrapper);
	lua_register(Lua_VM, "PutSpritePart", PutSpritePart_wrapper);
	lua_register(Lua_VM, "LoadSprite", LoadSprite_wrapper);
	lua_register(Lua_VM, "WriteSprite", WriteSprite_wrapper);
	lua_register(Lua_VM, "FreeSprite", FreeSprite_wrapper);
	lua_register(Lua_VM, "CompositeSprite", CompositeSprite_wrapper);
	lua_register(Lua_VM, "SpriteSheet", SpriteSheet_wrapper);
	lua_register(Lua_VM, "GammaSprite", GammaSprite_wrapper);
	lua_register(Lua_VM, "SPRITEW", SPRITEW_wrapper);
	lua_register(Lua_VM, "SPRITEH", SPRITEH_wrapper);
	lua_register(Lua_VM, "SPR", SPR_wrapper);
	lua_register(Lua_VM, "GetTic", GetTic_wrapper);
	lua_register(Lua_VM, "Console", Console_wrapper);
	lua_register(Lua_VM, "console", Console_wrapper);
	lua_register(Lua_VM, "GetString", GetString_wrapper);
	lua_register(Lua_VM, "selector", selector_wrapper);
	lua_register(Lua_VM, "Wget", Wget_wrapper);
	lua_register(Lua_VM, "LoadFile", LoadFile_wrapper);
	lua_register(Lua_VM, "WriteFile", WriteFile_wrapper);
	lua_register(Lua_VM, "FileExist", FileExist_wrapper);
	lua_register(Lua_VM, "CacheSubdir", CacheSubdir_wrapper);
	lua_register(Lua_VM, "SearchNextFile", SearchNextFile_wrapper);
	lua_register(Lua_VM, "MoveFile", MoveFile_wrapper);
	lua_register(Lua_VM, "Compress", Compress_wrapper);
	lua_register(Lua_VM, "Uncompress", Uncompress_wrapper);
	lua_register(Lua_VM, "WriteMemory", WriteMemory_wrapper);
	lua_register(Lua_VM, "ReadMemory", ReadMemory_wrapper);
	lua_register(Lua_VM, "DumpMemory", DumpMemory_wrapper);
	lua_register(Lua_VM, "SendKey", SendKey_wrapper);
	lua_register(Lua_VM, "SendInput", SendInput_wrapper);
	lua_register(Lua_VM, "Rand", Rand_wrapper);
	lua_register(Lua_VM, "Delay", Delay_wrapper);
	lua_register(Lua_VM, "SetWindowTitle", SetWindowTitle_wrapper);
	lua_register(Lua_VM, "CaptureWindow", CaptureWindow_wrapper);
	lua_register(Lua_VM, "ActiveWindow", ActiveWindow_wrapper);
	lua_register(Lua_VM, "WinWait", WinWait_wrapper);
	lua_register(Lua_VM, "ActivateWindow", ActivateWindow_wrapper);
	lua_register(Lua_VM, "MoveWindow", MoveWindow_wrapper);
	lua_register(Lua_VM, "KillWindow", KillWindow_wrapper);
	lua_register(Lua_VM, "FindWindow", FindWindow_wrapper);
	lua_register(Lua_VM, "FindNextWindow", FindNextWindow_wrapper);
	lua_register(Lua_VM, "GetWindowInfo", GetWindowInfo_wrapper);
	lua_register(Lua_VM, "GetKeyState", GetKeyState_wrapper);
	lua_register(Lua_VM, "KeyWait", KeyWait_wrapper);
	lua_register(Lua_VM, "GetMouseInfo", GetMouseInfo_wrapper);
	lua_register(Lua_VM, "SetMouseSpeed", SetMouseSpeed_wrapper);
	lua_register(Lua_VM, "MoveMouse", MoveMouse_wrapper);
	lua_register(Lua_VM, "MsgBox", MsgBox_wrapper);
	lua_register(Lua_VM, "TextBox", MsgBox_wrapper);
	lua_register(Lua_VM, "ListBox", ListBox_wrapper);
	lua_register(Lua_VM, "AskYesNo", AskYesNo_wrapper);
	lua_register(Lua_VM, "DllCall", DllCall_wrapper);
	lua_register(Lua_VM, "Browser", Browser_wrapper);
	lua_register(Lua_VM, "GetBrowser", GetBrowser_wrapper);
	lua_register(Lua_VM, "BrowserReady", BrowserReady_wrapper);
	lua_register(Lua_VM, "BrowserClick", BrowserClick_wrapper);
	lua_register(Lua_VM, "BrowserSetValue", BrowserSetValue_wrapper);
	lua_register(Lua_VM, "BrowserComboBox", BrowserComboBox_wrapper);
	lua_register(Lua_VM, "BrowserSendKey", BrowserSendKey_wrapper);
	lua_register(Lua_VM, "Quit", Quit_wrapper);
	lua_register(Lua_VM, "Overlay", Overlay_wrapper);
	lua_register(Lua_VM, "DrawTexRect", DrawTexRect_wrapper);

	// override print
	static const struct luaL_Reg printlib[] = {
		{"print", xprintf_wrapper},
		{NULL, NULL} /* end of array */
	};

	lua_getglobal(Lua_VM, "_G");
	luaL_register(Lua_VM, NULL, printlib);
	lua_pop(Lua_VM, 1);

	luaL_loadstring(Lua_VM,"return (function (...) print(string.format(...)) end)(...) ");
	lua_setglobal(Lua_VM, "printf");

	// constansok
	lua_pushnumber(Lua_VM,TICKBASE);
	lua_setglobal(Lua_VM,"TICKBASE");

	lua_pushnumber(Lua_VM,SCREENW);
	lua_setglobal(Lua_VM,"SCREENW");
	lua_pushnumber(Lua_VM,SCREENH);
	lua_setglobal(Lua_VM,"SCREENH");

	lua_pushnumber(Lua_VM,MAXVOL);
	lua_setglobal(Lua_VM,"MAXVOL");
	lua_pushnumber(Lua_VM,MAXPAN);
	lua_setglobal(Lua_VM,"MAXPAN");
	lua_pushnumber(Lua_VM,MIDPAN);
	lua_setglobal(Lua_VM,"MIDPAN");

	return TRUE;
}



//
//
//
void LUA_Deinit( void ) {

	SAFEFREEMEM( current_lua_file );
	SAFEFREEMEM( current_lua_string );

	if( Lua_VM ) lua_close( Lua_VM );
	Lua_VM = NULL;

	VAPI( FreeLibrary( hDll ) );
	hDll = NULL;

	// XLIB_UnHookKeyboardProc();
	// XLIB_UnHookMouseProc();

	return;
}



//
//
//
void LUA_GetState( int *rc, BOOL *act ) {

	if( rc )
		*rc = last_return;

	if( act ) {
		*act = screen_activity;

		// van-e Render?
		if( Lua_VM ) {
			lua_getglobal(Lua_VM,"Render");
			if( lua_isfunction(Lua_VM,-1) )
				*act = TRUE;
			lua_pop(Lua_VM,1);
		}
	}

	return;
}




//
//
//
BOOL LUA_LoadFile( char *file ) {

	screen_activity = FALSE;
	last_return = 0;

	if( LUA_Init() == FALSE )
		return FALSE;

	if( FileExist( file ) == FALSE ) {
		xprintf("LUA_RunFile error: no \"%s\" file.\n", file );
		return FALSE;
	}

	UCHAR *buf;
	int size = LoadFile(file, PTR(buf));
	if( size <= 0 ) {
		xprintf("LUA_RunFile error: empty file.\n" );
		return FALSE;
	}

	REALLOCMEM( current_lua_string, size+1 );
	memcpy(current_lua_string, buf, size);
	current_lua_string[size] = 0;

	FREEMEM(buf);

	REALLOCMEM( current_lua_file, XMAX_PATH+6 );
	strcpy( current_lua_file, file );

	// 0 -> szóköz
	for( int i=0; i<size; i++ )
		if( current_lua_string[i] == 0 )
			current_lua_string[i] = 32;

	if( (last_return = luaL_loadstring(Lua_VM, current_lua_string)) == 0 ) {
		lua_newtable(Lua_VM); // arg
		int table_index = lua_gettop(Lua_VM);

		// lz valami.lua elso masodik
		if( myargc >= 3 )
			for( int i = 2, idx=1; i < myargc; i++,idx++ ) {
				// xprintf("pushing(%d as %d): %s\n",i,idx,myargv[i] );
				lua_pushstring(Lua_VM, myargv[i]);
				lua_rawseti(Lua_VM, table_index, idx);
			}
		else {
			// legyen benne valami
			lua_pushnil(Lua_VM);
			lua_rawseti(Lua_VM, table_index, 1);
		}

		lua_setglobal(Lua_VM, "arg");

		return TRUE;
	}

	return FALSE;
}



//
// LUA_RunFile( "test.lua" );
//
BOOL LUA_RunFile( char *file ) {

	screen_activity = FALSE;
	last_return = 0;

	if( LUA_Init() == FALSE )
		return FALSE;

	if( FileExist( file ) == FALSE ) {
		xprintf("LUA_RunFile error: no \"%s\" file.\n", file );
		return FALSE;
	}

	UCHAR *buf;
	int size = LoadFile(file, PTR(buf));
	if( size <= 0 ) {
		xprintf("LUA_RunFile error: empty file.\n" );
		return FALSE;
	}

	REALLOCMEM( current_lua_string, size+1 );
	memcpy(current_lua_string, buf, size);
	current_lua_string[size] = 0;

	FREEMEM(buf);

	REALLOCMEM( current_lua_file, XMAX_PATH+6 );
	strcpy( current_lua_file, file );

	// 0 -> szóköz
	for( int i=0; i<size; i++ )
		if( current_lua_string[i] == 0 )
			current_lua_string[i] = 32;

	if( (last_return = luaL_loadstring(Lua_VM, current_lua_string)) == 0 ) {
		lua_newtable(Lua_VM); // arg
		int table_index = lua_gettop(Lua_VM);

		// lz valami.lua elso masodik
		if( myargc >= 3 )
			for( int i = 2, idx=1; i < myargc; i++,idx++ ) {
				// xprintf("pushing(%d as %d): %s\n",i,idx,myargv[i] );
				lua_pushstring(Lua_VM, myargv[i]);
				lua_rawseti(Lua_VM, table_index, idx);
			}
		else {
			// legyen benne valami
			lua_pushnil(Lua_VM);
			lua_rawseti(Lua_VM, table_index, 1);
		}

		lua_setglobal(Lua_VM, "arg");

		// lua_pushnumber(L, 21);   /* push 1st argument */
		// lua_pushnumber(L, 31);   /* push 2nd argument */

		//                    		2  1   do the call (2 arguments, 1 result)
		last_return = lua_pcall(Lua_VM, 0, LUA_MULTRET, 0);	// execute Lua program
	}

	if( last_return ) {
		printError();
		return FALSE;
	}

	return TRUE;
}



//
//
//
void LUA_CloseFile( void ) {

	if( Lua_VM && lua_close ) lua_close( Lua_VM );
	Lua_VM = NULL;

	return;
}



//
//
//
BOOL LUA_RunString( char *str ) {

	screen_activity = FALSE;
	last_return = 0;

	if( LUA_Init() == FALSE )
		return FALSE;

	if( str == NULL )
		return FALSE;

	int size = strlen(str);

	REALLOCMEM( current_lua_string, size+1 );
	strcpy(current_lua_string, str);
	current_lua_string[size] = 0;

	// string run
	// char *strLuaInput = "a = 1 + 1;\nprint (\"1+1=\" .. a);\n";

	last_return = luaL_dostring(Lua_VM, current_lua_string);

	return TRUE;
}


/*
	lua_getglobal(L, "double");
	lua_pushnumber(L, 3);
	lua_call(L, 1, 1);
	_ASSERT(lua_popnumber(L) == 6);
*/


//
// Can be called from within a debugger to look at the current Lua
// call stack.  (Borrowed from ToME 3)
//
void LUA_PrintStack( void ) {

	struct lua_Debug dbg;
	int i = 0;
	lua_State *L = Lua_VM;

	xprintf( "LUA_PrintStack:\n" );

	while( lua_getstack(L, i++, &dbg) == 1 ) {

		lua_getinfo(L, "lnuS", &dbg);

		char *file = strrchr(dbg.short_src, '/');
		if( file == NULL )
			file = dbg.short_src;
		else
			file++;

		xprintf("%s, function %s, line %d\n", file, dbg.name, dbg.currentline);
	}

	return;
}



#define CL_RESETSTACK_RETURN( ls, oldtop, retval ) {	\
	if( oldtop != lua_gettop(ls) ) { 		\
		lua_settop(ls, oldtop); 		\
	} 						\
	return retval; 					\
}



//
// call_va("f", "dd>d", x, y, &z);
//
void call_va( const char *func, const char *sig, ... ) {

	lua_State *L = Lua_VM;
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	if( L == NULL )
		return;

	va_start(vl, sig);
	lua_getglobal(L, func);  /* get function */

	/* push arguments */
	narg = 0;
	while( *sig ) {  /* push arguments */

		switch (*sig++) {

			case 'd':  /* double argument */
				lua_pushnumber(L, va_arg(vl, double));
				break;

			case 'i':  /* int argument */
				lua_pushnumber(L, va_arg(vl, int));
				break;

			case 's':  /* string argument */
				lua_pushstring(L, va_arg(vl, char *));
				break;

			case '>':
				goto endwhile;

			default:
				xprintf("invalid option (%c)", *(sig - 1));
		}
		narg++;
		luaL_checkstack(L, 1, "too many arguments");
	} endwhile:

	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if( lua_pcall(L, narg, nres, 0) != 0 )  /* do the call */
		xprintf("error running function `%s': %s", func, lua_tostring(L, -1));

	/* retrieve results */
	nres = -nres;  /* stack index of first result */
	while( *sig ) {  /* get results */

		switch( *sig++ ) {
			case 'd':  /* double result */
				if( !lua_isnumber(L, nres) )
					xprintf("wrong result type");
				*va_arg(vl, double *) = lua_tonumber(L, nres);
				break;

			case 'i':  /* int result */
				if( !lua_isnumber(L, nres) )
					xprintf("wrong result type");
				*va_arg(vl, int *) = (int)lua_tonumber(L, nres);
				break;

			case 's':  /* string result */
				if( !lua_isstring(L, nres) )
					xprintf( "wrong result type");
				*va_arg(vl, const char **) = lua_tostring(L, nres);
				break;

			default:
				xprintf( "invalid option (%c)", *(sig - 1));
		}
		nres++;
	}

	va_end(vl);

	return;
}



