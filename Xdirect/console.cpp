
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

#include <xlib.h>
#include <xinner.h>
#include <xlua.h>

RCSID( "$Id: console.cpp,v 1.2 2003/09/22 13:59:57 bernie Exp $" )


#define CV_VEL					(1.0f*(FLOAT)TICKBASE)  // velocity to animate the console with
#define CV_HISBUFF              10              // number of items in history
#define CV_HOME                 1               // console buffer "home" position

#define CV_BUFFSIZE		512		// size of the buffers for text
#define CV_BUFFMOVE             5

#define CV_SCREENBUFF           512            	// number of lines in screen buffer, 1 line is taken up by command line
#define CV_CARETCHAR	        '_'            	// caret character
#define CV_SEPCHAR              ']'            	// separator char
#define CV_CONSOLECHAR          '\'' 		// char to bring down console, can be int, use with care!



#define CV_TIMEOUT		( TICKBASE * 3 )
#define CV_MAXSCR		4
#define CV_MAXWOWROW		15

#define CNAME			"console.bmp"

#define CV_HEIGHT		( SCREENH * 1/2 )

// Internal console calls
void consoleLoadBack( char *name, int texId );
void consoleLoadFont( char *name, int texId );
void consoleDrawChar( int c, FLOAT x, FLOAT y );
int consoleDrawString( int level, int *s, FLOAT color[3] );
void consoleRenderBuffers( void );
void consoleCommand( void );
void consoleHistoryAdd( int *s );
void consoleCommandParse( int *s );
void consoleBuffCopy( char *d, char *s, int start, int end );
void consoleSetGLModes( int state );
static char *consoleCompleteCommand( void );
int consoleMeasureString( int *s );




// implement some sort of coloring system, this is temp

static rgb_t white = {255,255,255, 255 };
static rgb_t grey = {178,178,178, 255};
static rgb_t red = {255,0,0, 255};
static rgb_t green = {50,240,50, 255};
static rgb_t blue = {0,0,255, 255};
static rgb_t purple = {127,0,127, 255};


extern void genericParse( char *commandName, char *s );
extern void setVarValue( char *commandName, char *s );
extern void toggleVarState( char *commandName, char *s );
extern void commandDump( char *commandName, char *s );
static void playCD( char *commandName, char *s );
static void checkMail( char *commandName, char *s );



/*
	command_t Details
	-----------------
	comm		- Name of command or variable
	params		- Does this command or variable take paramaters?
	id			- Optional id, used when one function parses many commands or variables
	function	- Function to parse command, commandIndex is optional.
*/

typedef struct command_s {

	char name[128];
	void (*function)( char *commandName, char *params );

} command_t, *command_ptr;

static command_t *table = NULL;
static int ncomms = 0;


// Used for conState and buffer scrolling in the console_t structure
enum {
	UP = 0,
	DOWN,
	HOME,
	END,
	ONEUP,
	ONEDOWN,
};

#define MAX_RESSTR (XMAX_PATH*4)

typedef struct {

	int conW;		// - Width of the console
	int conH;		// - Height of the console
	FLOAT posY;		// - Vertical position of the console, used for animating
	int conAni;		// - Used for animation, 1 = animating, 0 = not animating
	int conState;		// - State of the console, UP or DOWN
	int conFont;		// - OpenGL texture id for the console font texture
	int conBackTex;		// - OpenGL texture id for the console background texture
	int bPos;		// - Command line buffer position
	int maxChar;		// - max characters to draw per row
	int conHisPos;		// - Position in the console history
	int conBufPos;		// - Position to start rendering the console from
	int conBufferCnt;
	int conChar;					// - Character that will bring down the console
	int conCmdHis[ CV_HISBUFF ][ CV_BUFFSIZE ];	// - Buffer for command history
	int conBuffer[ CV_SCREENBUFF ][ CV_BUFFSIZE ];	// - Buffers for the console
	int conTime[ CV_SCREENBUFF ];			// - Time
	int sprW,sprH;

	// char app[256];

	int result_type;
	double result_number;
	char result_string[MAX_RESSTR];		// nem tudom hogy ennyi elég lesz-e
	double result_bool;

} console_t, *console_ptr;

static console_t console;

cvar_t *cvar_vars = NULL;
static char *cvar_null_string = "";


//
//	This function will copy chars from a source string to a destination string
//	starting from position "start" and ending at "end."  All positions assumed
//	to be valid in the array.  Just a utility function.
//
void consoleBuffCopy( char *d, char *s, int start, int end ) {

	int t = start;

	s += start;

	while( *s && t != end && *s != '\0' ) {
		*d = *s;
		*d ++;
		*s ++;
		t ++;
	}

	*d = '\0';

	return;
}







//
//	Sets up the console and all of its values.  "w" and "h" are the width and
//	height of the application window. "function" is a pointer to the
//	applications redraw function. "fn" and "bn" are font file and background
//	file names respectively.  "fid" and "bid" are texture ids/
//
BOOL consoleInit( void ) {

	console.conW = SCREENW;
	console.conH = CV_HEIGHT;
	console.posY = 0;
	console.conAni = FALSE;
	console.conState = UP;
	console.bPos = 0;
	console.maxChar = CV_BUFFSIZE-1; // (SCREENW / FontWidth());
	console.conHisPos = 0;
	console.conBufPos = 1;
	console.conBufferCnt = 1;
	console.conChar = CV_CONSOLECHAR;	// ez nem igaz
	memset( console.conBuffer, '\0', sizeof(console.conBuffer) );
	memset( console.conCmdHis, '\0', sizeof(console.conCmdHis) );
	memset( console.conTime, 0, sizeof(console.conTime) );
	// strcpy( console.app, XLIB_winSetProgramDesc(NULL) );
	console.conFont = -1;
	console.conBackTex = -1;
	console.sprW = 0;
	console.sprH = 0;
	console.result_type = CON_NORESULT;

	int x;
	char (*bubble)[8][512] = (char (*)[8][512])BuildBubbleString( "Nunsztapu!" );
	for( x=0; x<8; x++ ) consolePrintf( "%s\n", (*bubble)[x] );

	consoleAddCommand( "ver", genericParse );
	consoleAddCommand( "quit", genericParse );
	consoleAddCommand( "exit", genericParse );
	consoleAddCommand( "echo", genericParse );
	consoleAddCommand( "browser", genericParse );
	consoleAddCommand( "clear", genericParse );
	consoleAddCommand( "memstat", genericParse );
	consoleAddCommand( "memcheck", genericParse );
	consoleAddCommand( "datetime", genericParse );
	consoleAddCommand( "caption", genericParse );
	consoleAddCommand( "videomode", genericParse );
	consoleAddCommand( "aboutfull", genericParse );
	consoleAddCommand( "togglefullscreen", genericParse );
	consoleAddCommand( "lua", genericParse );
	consoleAddCommand( "tic", genericParse );
	consoleAddCommand( "beep", genericParse );
	consoleAddCommand( "cmdlist", commandDump );
	consoleAddCommand( "playsound", genericParse );
	consoleAddCommand( "soundfont", genericParse );
	consoleAddCommand( "status", genericParse );
	consoleAddCommand( "p64", genericParse );
	consoleAddCommand( "cdplay", playCD );
	consoleAddCommand( "cdeject", genericParse );
	consoleAddCommand( "chkmail", checkMail );
	consoleAddCommand( "listtexmap", genericParse );
	consoleAddCommand( "dumptexmap", genericParse );
	consoleAddCommand( "reloadtexmap", genericParse );
	consoleAddCommand( "dirlist", genericParse );
	consoleAddCommand( "dirdump", genericParse );
	consoleAddCommand( "say", genericParse );
	consoleAddCommand( "espeak", genericParse );
	consoleAddCommand( "msgbox", genericParse );
	consoleAddCommand( "toclipboard", genericParse );
	consoleAddCommand( "fromclipboard", genericParse );
	consoleAddCommand( "overlay", genericParse );
	consoleAddCommand( "exec", genericParse );
	consoleAddCommand( "wget", genericParse );
	consoleAddCommand( "curl", genericParse );
	consoleAddCommand( "baszki", genericParse );
	consoleAddCommand( "log", genericParse );
	consoleAddCommand( "crc", genericParse );
	consoleAddCommand( "encstr", genericParse );
	consoleAddCommand( "decstr", genericParse );
	consoleAddCommand( "startavi", genericParse );
	consoleAddCommand( "stopavi", genericParse );
	consoleAddCommand( "screenshot", genericParse );
	consoleAddCommand( "path2url", genericParse );
	consoleAddCommand( "askyesno", genericParse );
	consoleAddCommand( "gettemp", genericParse );
	consoleAddCommand( "getfilenamenopath", genericParse );
	consoleAddCommand( "shortpath", genericParse );
	consoleAddCommand( "getfileext", genericParse );
	consoleAddCommand( "im", genericParse );
	consoleAddCommand( "del", genericParse );
	consoleAddCommand( "ispic", genericParse );
	consoleAddCommand( "getcolorname", genericParse );
	consoleAddCommand( "getcolorrgb", genericParse );
	consoleAddCommand( "findrandomwindow", genericParse );
	consoleAddCommand( "findrandompid", genericParse );
	consoleAddCommand( "getinput", genericParse );
	consoleAddCommand( "printwindow", genericParse );
	consoleAddCommand( "splash", genericParse );
	consoleAddCommand( "shareware", genericParse );
	consoleAddCommand( "wingamma", genericParse );
	consoleAddCommand( "setcore", genericParse );
	consoleAddCommand( "refreshjoystick", genericParse );
	consoleAddCommand( "shutdowncomputer", genericParse );
	consoleAddCommand( "rebootcomputer", genericParse );
	consoleAddCommand( "clock", genericParse );
	consoleAddCommand( "xproc1", genericParse );
	consoleAddCommand( "checkkbdhook", genericParse );

	return TRUE;
}



//
//
//
static void loadTex( void ) {

	// background
	if( (console.conBackTex = TexNumForName( CNAME )) == (-1) ) {

		int flag,color;
		UCHAR pal[768],*spr;
		texmap_t *tex;
		char filename[XMAX_PATH];

		sprintf( filename, "%s%c%s", GetDataDir(), PATHDELIM, CNAME );

		if( FileExist(filename) ) {
			LoadPicture( filename, PTR(spr) );
			GetRgbPicture(pal);
		}
		else
			CheckerTexture( &spr, 256,256 );

		flag = 0L;
		color = 0L;

		AddTexMapHigh( spr, pal, CNAME, CNAME, flag, color );

		if( (console.conBackTex = TexNumForName( CNAME )) == (-1) ) {
			xprintf("loadTex: no \"%s\" texture.", CNAME );
			FREEMEM( spr );
			return;
		}

		tex = TexForNum( console.conBackTex );

		console.sprW = tex->width1;
		console.sprH = tex->height1;

		FREEMEM( spr );
	}

	return;
}



//
//
//
static void loadFont( void ) {

	// font
	if( console.conFont == (-1) ) {

		console.conFont = LoadFont3D( "8x12xb.fon", TRUE, FALSE, NULL, 0 );

		/***
		{
		char font_name[XMAX_PATH];
		char *key_name = "xlib_console_font";

		XLIB_winReadProfileString( key_name, "GentiumPlus-R.ttf", font_name, MAX_PATH  );
		XLIB_winWriteProfileString( key_name, font_name );

		fontloader_t fontloader;
		fontloader.x = 10;
		fontloader.y = 10;
		console.conFont = LoadFont3D( font_name, TRUE, FALSE, &fontloader, 1 );
		}
		***/

		PushFont();
		SetFont( console.conFont );
		console.maxChar = (SCREENW / FontWidth());
		PopFont();
	}

	return;
}





//
//	Called when the application window is resized.  Reconfigures the console
//	for the new window dimensions.  Needs to be called from inside the
//	application to make sure that the console functions properly.
//
void consoleResize( void ) {

	console.conW = SCREENW;
	console.conH = CV_HEIGHT;
	console.posY = 0;
	console.maxChar = CV_BUFFSIZE-1; // loadFont: (console.conW / FontWidth()) / 1;

	return;
}








//
//	Renders the background of the console.
//
void consoleRenderBack( void ) {

	polygon_t poly;
	point_t point[4];
	rgb_t rgb[4] = { {255,255,255,255}, {255,255,255,255}, {255,255,255,255}, {255,255,255,255} };
	point3_t st[4] = { {0.0f,0.0f,0.0f}, {0.0f,255.0f,0.0f}, {255.0f,255.0f,0.0f}, {255.0f,0.0f,0.0f} };
	int alpha,tx,ty;

	// ha nincs lenn a console
	if( console.posY <= 0 )
		return;

	loadTex();

	poly.npoints = 4;
	poly.point = point;

	alpha = ftoi( 255 * console.posY / console.conH );

	rgb[0].a = alpha;
	rgb[1].a = alpha;
	rgb[2].a = alpha;
	rgb[3].a = alpha;

	for( ty=console.posY; ty>=0; ty-=console.sprH )
	for( tx=0; tx<=console.conW; tx+=console.sprW ) {

		point[0][0] = tx;		point[0][1] = ty - console.sprH;
		point[1][0] = tx;		point[1][1] = ty;
		point[2][0] = tx+console.sprW;	point[2][1] = ty;
		point[3][0] = tx+console.sprW;	point[3][1] = ty - console.sprH;

		PutSpritePoly( poly, st, console.conBackTex, rgb );
	}

	return;
}



//
//	Draws a texture mapped polygon with the requested char at position (x, y).
//
void consoleDrawChar( int c, FLOAT x, FLOAT y ) {

	// if( (x + FontWidth()) >= console.conW )
	//	return;

	WriteChar( (int)x, (int)y, c );


	return;
}



//
//	Draws a string onto the console at a certain (x,y) coord.  This does not
//	check for other strings and will write over everything.  It also does NO
//	bounds checking.  Use with caution.
//
void consoleDrawRawString( int *s, FLOAT x, FLOAT y, rgb_t color ) {

	int i;
	FLOAT tx = x;

	PushFont();

	SetFont( console.conFont );
	SetFontAlpha( FA_ALL, color.r,color.g,color.b, color.a );

	for( i=0; i<istrlen(s); i++ ) {
		consoleDrawChar( s[i], tx, y );
		tx += XL_GetCharWidth(s[i]);
	}

	// glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

	PopFont();

	return;
}


//
//
//
int consoleMeasureString( int *s ) {

	int i;

	if( s == NULL )
		return 0;

	PushFont();

	SetFont( console.conFont );

	char *p = istrtostr( s );

	i = MeasureString( p );

	FREEMEM(p);

	PopFont();

	return i;
}


#define CARETFLASH	(TICKBASE/2)




//
//	Draws a string onto the console at a certain level.  Why does this function
//	return an int?  It returns int so that we can know if we had to change
//	levels on the console to draw a long string.  This will allow for repeated
//	calls and have all output be on the correct level.
//
int consoleDrawString( int level, int *s, rgb_t color ) {

	int i, pos, t, y;
	int len = istrlen(s);
	int lev = level;
	FLOAT x = 0.0f;

	PushFont();

	SetFont( console.conFont );
	SetFontAlpha( FA_ALL, color.r,color.g,color.b, color.a );

	y = console.posY - ((level+1) * FontHeight() + 5);

	// makes sure we don't draw something that isn't on the screen
	if( y < 0 ) {
		PopFont();
		return level + 1;
	}

	// draw out command line separater thingy, colorize this?
	if( level == 0 ) {
		consoleDrawChar( CV_SEPCHAR, x, y );
		x += XL_GetCharWidth(CV_SEPCHAR);
	}

	// nothing to render leave after drawing caret
	if( s[0] == '\0' ) {

		if( level == 0 )
			if( (GetTic() % CARETFLASH) < (CARETFLASH/2) )
				consoleDrawChar( CV_CARETCHAR, x, y );

		PopFont();

		return level + 1;
	}

	// figure out how many lines this string is going to span
	if( len > console.maxChar ) {
/*
		t = len;
		while( t > console.maxChar ) {
			t -= console.maxChar;
			y += FontHeight();
		}
		x = 0;
*/	}

	// draw!
	for( i=0, pos=0; i<len; i++, pos++ ) {

		if( s[i] == 10 || s[i] == 13 ) continue;

		// Tab
		if( s[i] == 9 ) {
			x += (8 * XL_GetCharWidth('a'));
			continue;
		}

		if( pos > console.maxChar - 1 ) {
/*			pos = 0;
			x = FontWidth();
			y -= FontHeight();
			lev++;
*/		}
		else
			consoleDrawChar( s[i], x, y );

		x += XL_GetCharWidth(s[i]);
	}

	// draw current cursor pos
	if( level == 0 )
		if( (GetTic() % CARETFLASH) < (CARETFLASH/2) )
			consoleDrawChar( CV_CARETCHAR, x, y );

	PopFont();

	return lev + 1;
}




//
//	Render all of the consoles buffers, command line and non.
//
void consoleRenderBuffers( void ) {

	int level = 0;
	int i;

	// draw the command line
	level = consoleDrawString( level, console.conBuffer[0], green );

	// draw the buffers
	for( i=console.conBufPos; i<CV_SCREENBUFF; i++ )
		level = consoleDrawString( level, console.conBuffer[i], green );

	return;
}



//
//
//
void consoleFlush( void ) {

	int i;

	for( i=CV_SCREENBUFF-1; i>0; i-- ) {

		if( console.conBuffer[i][0] == 0 )
			continue;

		console.conTime[i] = -10*TICKBASE;
	}

	return;
}




//
// Main console drawing system.  This should not modify any of the
// programs existing OpenGL settings.  All states should be returned
// to their original values at exit.
//
void consoleDraw( void ) {

	int y,h,i;

	// if we are up and not animating get outta here
	if( console.conState == UP && console.conAni == FALSE ) {

		// mindig kiírja az elsõ 4 sort ha nicns lehuzva?
		/***
		y = 3;

		PushFont();
		SetFont( console.conFont );
		h = FontHeight();
		PopFont();

		for( i=CV_MAXSCR; i>0; i-- ) {

			if( (GetTic() - console.conTime[i]) > CV_TIMEOUT )
				continue;

			if( consoleMeasureString(console.conBuffer[i]) <= 0 )
				continue;

			consoleDrawRawString( console.conBuffer[i], 2, y, green );

			y += (int)((FLOAT)h*1.2f);
		}
		***/

		return;
	}

	loadFont();

	consoleRenderBack();
	consoleRenderBuffers();

	if( console.conBufPos != CV_HOME ) {

		// if user is scrolling into console buffer let them know

		PushFont();
		SetFont( console.conFont );

		int *str = toistr( "^^^^  ^^^^  ^^^^" );

		consoleDrawRawString( str, console.conW - FontWidth()*(istrlen(str)+3), console.posY-(FontHeight()+5), green );

		FREEMEM( str );

		PopFont();
	}

	return;
}



static int console_tic = 0;
extern int translateWin32Text( int kc );


//
//
//
int consoleExec( BOOL state, char *cmd ) {

	if( state == FALSE ) {
		console.conState = UP;
		console.posY = 0;
		display_zprintf = FALSE;
		console.conAni = FALSE;
		console_tic = 0;
	}
	else
	if( state == TRUE ) {
		console.conState = DOWN;
		console.posY = console.conH;
		display_zprintf = TRUE;
		console.conAni = FALSE;
		console_tic = 0;
		translateWin32Text(0);	// kamu, mert úgyis kell a keyboard
	}

	if( cmd && cmd[0] ) {
		if( state == -100 ) {
			// ne legyen benne a historyban
			int *i = toistr( cmd );
			if( i ) {
				consoleCommandParse( i );
				SAFE_FREEMEM( i );
			}
		}
		else {
			istrcpystr( console.conBuffer[0], cmd );
			consoleCommand();
			console.bPos = 0;
			console.conHisPos = 0;
			consoleBufferMove(HOME);
		}
	}

	return console.result_type;
}


//
// result string from consoleExec()
//
char *consoleGetString( void ) {

	if( console.result_type == CON_STRING )
		return console.result_string;

	return NULL;
}


//
//
//
double consoleGetNumber( void ) {

	if( console.result_type == CON_NUMBER )
		return console.result_number;

	return 0.0;
}


//
//
//
BOOL consoleGetBool( void ) {

	if( console.result_type == CON_BOOL )
		return console.result_bool;

	return 0.0;
}




//
//	Animates the console, either up or down
//
int consoleAnimate( void ) {

	int key;
	int tic = GetTic();

	if( display_zprintf ) {

		if( console.conState == UP )
			console_tic = tic - CV_VEL * console.posY / console.conH;

		console.conState = DOWN;
		key = GetKey();
	}
	else {

		if( console.conState == DOWN )
			console_tic = tic - (CV_VEL - (CV_VEL * console.posY / console.conH));

		console.conState = UP;
		key = key_None;
	}

	// animate

	if( console.conState == UP ) {

		console.posY = console.conH - ((tic - console_tic) * console.conH / CV_VEL);

		if( console.posY < 0 ) {
			console.posY = 0;
			console.conAni = FALSE;
		}
	}
	else
	if( console.conState == DOWN ) {

		console.posY = (tic - console_tic) * console.conH / CV_VEL;

		if( console.posY > console.conH ) {
			console.posY = console.conH;
			console.conAni = FALSE;
		}
	}

	// keys

	if( display_zprintf ) {

		if( /* (key == console.conChar) || */ (key == 48 /*'0'*/ && (keys[sc_Control] || keys[sc_Alt])) || (key == key_Escape) ) {

			display_zprintf = 0;
			console_tic = tic;

			if( console.conAni == FALSE )
				console.conAni = TRUE;
		}
		else {

			switch( key ) {
				case EXT(sc_Up): consoleHistoryRecall(); break;
				case EXT(sc_Down): consoleCommandLineClear(); break;
				//case EXT(sc_PageUp): consoleBufferMove(UP); break;
				//case EXT(sc_PageDown): consoleBufferMove(DOWN); break;
				case EXT(sc_Home): consoleBufferMove(END); break;
				case EXT(sc_End): consoleBufferMove(HOME); break;
				case key_Tab: consoleCompleteCommand(); break;
				case 'c':
					if( keys[sc_Control] ) {
						char *s = istrtostr( console.conBuffer[0] );
						XLIB_winCopyTextToClipboard( s );
						SAFE_FREEMEM(s);
						break;
					}
				case 'v':
					if( keys[sc_Control] ) {
						int *str = toistr( XLIB_winPasteTextFromClipboard() );
						istrcat( console.conBuffer[0], str );
						console.bPos += istrlen(str);
						CLAMPMAX( console.bPos, CV_BUFFSIZE - 2 );
						SAFE_FREEMEM( str );
						break;
					}
				case EXT(sc_Insert):
					if( keys[sc_Control] ) {
						char *s = istrtostr( console.conBuffer[0] );
						XLIB_winCopyTextToClipboard( s );
						SAFE_FREEMEM(s);
						break;
					}
					else if( keys[sc_LeftShift] || keys[sc_RightShift] ) {
						int *str = toistr( XLIB_winPasteTextFromClipboard() );
						istrcat( console.conBuffer[0], str );
						console.bPos += istrlen(str);
						CLAMPMAX( console.bPos, CV_BUFFSIZE - 2 );
						SAFE_FREEMEM( str );
						break;
					}
				default:
					if( (key != key_None) && !ISEXTKEY(key) )
						consoleAddChar( key );
					break;
			}

			if( keys[sc_PageUp] || mousedz > 0 ) consoleBufferMove(UP);
			if( keys[sc_PageDown] || mousedz < 0 ) consoleBufferMove(DOWN);
		}
	}

	// chkmail
	if( 0 && winEmail() ) {

		#define MAILTIC	(TICKBASE*60*5)

		static int mailtic = MAILTIC;

		if( mailtic < GetTic() ) {

			checkMail( NULL, NULL );

			mailtic = GetTic() + MAILTIC;
		}
	}

	// World of Warcraft szerü kisablakos kiírás WOW
	int myfont = GetFontId( "8x12xb.fon" );
	if( 0 && myfont != -1 ) {

		// csak ha font be lett töltve
		PushFont();
		SetFont( myfont );
		SetFontAlpha( FA_ALL, 230,207,197, 255 );

		int y = SCREENH * 4/5;
		int h = FontHeight();
		int fw = FontWidth();
		int i,w,margo = 5;

#define MAXWOWCHAR CV_BUFFSIZE

		int str[MAXWOWCHAR+1] = {0};

		for( i=0; i<CV_MAXWOWROW; i++ ) {

			if( (tic - console.conTime[i]) > CV_TIMEOUT )
				continue;

			istrncpy( str, console.conBuffer[i], MAXWOWCHAR );
			str[MAXWOWCHAR] = 0;

			if( (w = MeasureIString(str)) <= 0 )
				continue;

			DrawTexRect(margo-2, y, margo+w+2, y+(int)((FLOAT)h*1.2f), 0, 150 );

			WriteIString( margo, y, str );

			y -= (int)((FLOAT)h*1.2f);
		}

		PopFont();
	}

	// draw

	DrawStatus();	// státusz text ala WOW

	P64();		// Project64

	consoleDraw();

	return key_None;
}





//
//	Adds a character to the current command line buffer.
//
void consoleAddChar( int c ) {

	// don't bother trying to add chars if the console is UP
	if( console.conState == UP )
		return;

	switch( c ) {

		case CV_CONSOLECHAR:
			return;
			break;

		case '\r':
		// case '\n':

			consoleCommand();
			console.bPos = 0;
			console.conHisPos = 0;

			// ha enter van akkor leugrik az aljára
			consoleBufferMove(HOME);

			break;

		case '\b':

			console.bPos--;
			console.conBuffer[0][console.bPos] = '\0';

			CLAMPMIN( console.bPos, 0 );

			break;

		default :

			console.conBuffer[0][console.bPos] = c;
			console.bPos++;

			CLAMPMAX( console.bPos, CV_BUFFSIZE - 2 );

			break;
	}

	return;
}


//
//
//
static void removecrlf( void ) {

	int len = istrlen(console.conBuffer[1]);

	if( (len>0) && console.conBuffer[1][len-1] == 10 )
		console.conBuffer[1][(len--)-1] = 0;

	if( (len>0) && console.conBuffer[1][len-1] == 13 )
		console.conBuffer[1][(len--)-1] = 0;

	return;
}



//
// This function is called when a user hits enter.  It then
// proceedes to add the command to history, parse the command
// for variables, clear the command line and redraw the console.
// All command line data is gone after a call to this function.
//
void consoleCommand( void ) {

	if( console.conBuffer[0][0] == '\0' )
		return;

	consoleHistoryAdd( console.conBuffer[0] );

	consoleCommandParse( console.conBuffer[0] );

	memset( console.conBuffer[0], '\0', sizeof(console.conBuffer[0]) );

	return;
}



//
// Looks through the command/variable table for a match to the
// command just typed and if a match is found call the apropriate
// parsing function.
//
void consoleCommandParse( int *s ) {

	int i;
	int name[1024],*p;

	console.result_type = CON_NORESULT;

	istrncpy( name, s, 1024 );

	if( (p = istrchr(name, ' ' )) != NULL )
		*p = 0;

	if( s[istrlen(name)] == ' ' )
		p = &s[istrlen(name)+1];
	else
		p = NULL;

	if( p ) {
		while( ((*p == ' ') || (*p == 9)) && (*p != 0) )
			++p;

		if( *p == 0 ) p = NULL;
	}

	for( i=0; i<ncomms; i++ ) {
		if( !istricmpstr( name, table[i].name ) ) {
			char *str = istrtostr( p );
			table[i].function( table[i].name, str );
			SAFE_FREEMEM( str );
			return;
		}
	}

	char *sname = istrtostr( name );
	char *sp = istrtostr( p );

	if( CV_Command( sname, sp ) == FALSE ) {
		char *str = istrtostr( s );
		xprintf( "Invalid command: \"%s\".\n", str );
		xprintf( "Type \"cmdlist\" for all commands.\n");
		SAFE_FREEMEM( str );
	}

	SAFE_FREEMEM( sname );
	SAFE_FREEMEM( sp );

	return;
}



//
//
//
static char *consoleCompleteCommand( void ) {

	int i,len;
	int *partial = NULL;
	char *name = NULL;

	partial = console.conBuffer[0];

	len = istrlen( partial );

	if( !len )
		return NULL;

	// check functions

	for( i=0; i<ncomms; i++ )
		if( !istrnicmpstr( partial, table[i].name, len) ) {
			istrcpystr( console.conBuffer[0], table[i].name );
			console.bPos = istrlen( console.conBuffer[0] );
			return table[i].name;
		}

	char *sp = istrtostr( partial );

	if( (name = CV_CompleteVariable( sp )) != NULL ) {
		istrcpystr( console.conBuffer[0], name );
		console.bPos = istrlen( console.conBuffer[0] );
		SAFE_FREEMEM(sp);
		return name;
	}

	SAFE_FREEMEM(sp);

	return NULL;
}



//
//
//
BOOL consoleIsCommand( char *cmd ) {

	int i,len;

	len = strlen( cmd );

	if( !len )
		return FALSE;

	// check functions

	for( i=0; i<ncomms; i++ )
		if( !strnicmp( cmd, table[i].name, len) )
			return TRUE;

	return FALSE;
}





//
// Add a string to the console buffer.  Never touches the command line.
// If data is longer than C_BUFFSIZE the extra characters will be cut off.
// If data is longer than 255 then it breaks :(
//
void consolePrintf( char *s, ... ) {

#define TEMPLEN (16*1024)

	int i,len;
	char *temp;
	va_list	msg;

	if( s == NULL )
		return;

	CALLOCMEM( temp, TEMPLEN );

	va_start(msg, s);
	_vsnprintf(temp,TEMPLEN-1, s, msg);
	va_end(msg);

	if( strlen(temp) < 1 ) {
		FREEMEM( temp );
		return;
	}

	len = istrlen(console.conBuffer[1]);

	if( temp[0] == '\r' ) {
		istrncpystr( &console.conBuffer[1][0], &temp[1], CV_BUFFSIZE - 1 );
		console.conTime[1] = GetTic();

		// removecrlf();
	}
	else {

		if( (len > 0) && ((console.conBuffer[1][len-1] == 10) || (console.conBuffer[1][len-1] == 13)) ) { //  || (temp[0] == 10) || (temp[0] == 13) ) {

			// removecrlf();

			len = 0;
			for( i=CV_SCREENBUFF-1; i>1; i-- ) {
				istrncpy( console.conBuffer[i], console.conBuffer[i - 1], CV_BUFFSIZE - 1 );
				console.conTime[i] = console.conTime[i - 1];
			}
			++console.conBufferCnt;
		}

		istrcpystr( &console.conBuffer[1][len], temp );
		console.conTime[1] = GetTic();
	}

	FREEMEM( temp );

	// if( winNoLog() ) console.conTime[1] = -2 * CV_TIMEOUT;

	return;
}





//
//	Clears all of the consoles buffers, history is NOT cleared.
//
void consoleClearBuffers( void ) {

	memset( console.conBuffer, '\0', sizeof(console.conBuffer) );
	console.conBufPos = CV_HOME;
	console.conBufferCnt = 1;

	return;
}





//
//	Recalls a recent command for the consoles history.
//
void consoleHistoryRecall( void ) {

	if( console.conHisPos >= CV_HISBUFF )
		console.conHisPos = 0;

	istrncpy( console.conBuffer[0], console.conCmdHis[console.conHisPos],CV_BUFFSIZE - 1 );

	console.bPos = istrlen( console.conCmdHis[console.conHisPos] );
	console.conHisPos++;

	if( console.conCmdHis[console.conHisPos][0] == '\0' )
		console.conHisPos = 0;

	return;
}




//
//	Add a command into the history.
//
void consoleHistoryAdd( int *s ) {

	int i = CV_HISBUFF - 1;

	if( !istrcmp( console.conCmdHis[0], s ) )
		return;

	for( ; i>0; i-- )
		istrncpy( console.conCmdHis[i], console.conCmdHis[i - 1], CV_BUFFSIZE - 1 );

	istrncpy( console.conCmdHis[0], s, CV_BUFFSIZE - 1 );

	return;
}




//
//	Clears all data from the command line buffer
//
void consoleCommandLineClear( void ) {

	memset( console.conBuffer[0], '\0', sizeof(console.conBuffer[0]) );
	console.bPos = 0;

	return;
}




//
//	Moves the console buffer up or down, allows you to scroll through the buffer.
//
void consoleBufferMove( int dir ) {

	switch( dir ) {

		case UP:

			console.conBufPos += CV_BUFFMOVE;
			if( console.conBufPos > console.conBufferCnt)
				console.conBufPos = console.conBufferCnt;

			break;

		case DOWN:

			console.conBufPos -= CV_BUFFMOVE;
			if( console.conBufPos < CV_HOME )
				console.conBufPos = CV_HOME;

			break;

		case ONEUP:

			console.conBufPos += 1;
			if( console.conBufPos > console.conBufferCnt)
				console.conBufPos = console.conBufferCnt;

			break;

		case ONEDOWN:

			console.conBufPos -= 1;
			if( console.conBufPos < CV_HOME )
				console.conBufPos = CV_HOME;

			break;


		case HOME:

			console.conBufPos = CV_HOME;

			break;

		case END:

			PushFont();
			SetFont( console.conFont );

			console.conBufPos = console.conBufferCnt - ((console.conH / FontHeight()) - 2);
			if( console.conBufPos < CV_HOME )
				console.conBufPos = CV_HOME;

			PopFont();

			break;
	}

	return;
}




//
//
//
BOOL consoleAddCommand( char *name, void (*function)(char*,char*) ) {

	if( (name == NULL) || (function == NULL) ) {
		// xprintf("consoleAddCommand: cannot add \"%s\" command.\n", name );
		return FALSE;
	}

	REALLOCMEM( table, (ncomms+1) * sizeof(command_t) );

	strcpy( table[ncomms].name, name );
	table[ncomms].function = function;

	++ncomms;

	return TRUE;
}


////////////// cvar.cpp /////////////


//
//
//
cvar_t *CV_FindVar( char *var_name ) {

	cvar_t *var;

	for( var=cvar_vars; var; var=var->next )
		if( !stricmp( var_name, var->name ) )
			return var;

	return NULL;
}




//
//
//
FLOAT CV_VariableValue( char *var_name ) {

	cvar_t *var;

	if( (var = CV_FindVar( var_name )) == NULL )
		return 0;

	return atof( var->string );
}



//
//
//
char *CV_VariableString( char *var_name ) {

	cvar_t *var;

	if( (var = CV_FindVar( var_name )) == NULL )
		return cvar_null_string;

	return var->string;
}



//
//
//
char *CV_CompleteVariable( char *partial ) {

	cvar_t *cvar;
	int	len;

	if( partial == NULL )
		return NULL;

	len = strlen( partial );

	if( !len )
		return NULL;

	// check functions

	for( cvar=cvar_vars; cvar; cvar=cvar->next )
		if( !strnicmp( partial,cvar->name, len) )
			return cvar->name;

	return NULL;
}





//
//
//
void CV_Set( char *var_name, char *value ) {

	cvar_t *var;
	BOOL changed;

	if( (var = CV_FindVar( var_name )) == NULL ) {
		// there is an error in C code if this happens
		xprintf("CV_Set: variable \"%s\" not found.\n", var_name);
		return;
	}

	changed = stricmp( var->string, value );

	// if( var->string ) FREEMEM( var->string );	// free the old value string

	// ALLOCMEM( var->string, strlen(value)+1 );
	strcpy( var->string, value );

	var->value = atof( var->string );

	xprintf("%s is \"%s\"\n", var->name, var->string);


	// if( var->server && changed ) {
		// if( sv.active )
		//	SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
	// }

	return;
}





//
//
//
void CV_SetValue( char *var_name, FLOAT value ) {

	char str[256];

	// string_t<256> str("%f", value);

	sprintf( str, "%f", value );

	CV_Set( var_name, str );

	return;
}




//
// Adds a freestanding variable to the variable list.
//
void CV_RegisterVariable( cvar_t *variable ) {

	char *oldstr = NULL;

	// first check to see if it has allready been defined

	if( CV_FindVar( variable->name ) ) {
		xprintf("Can't register variable \"%s\", allready defined.\n", variable->name);
		return;
	}

	// check for overlap with a command
	if( consoleIsCommand( variable->name ) ) {
		xprintf("CV_RegisterVariable: \"%s\" is a command.\n", variable->name);
		return;
	}

	// copy the value off, because future sets will Z_Free it
	// oldstr = variable->string;
	// ALLOCMEM(variable->string, strlen(variable->string)+1 );
	// strcpy( variable->string, oldstr );
	variable->value = atof(variable->string);
	variable->func = NULL;

	// link the variable in
	variable->next = cvar_vars;
	cvar_vars = variable;

	return;
}



//
//
//
void CV_SetFunc( char *var_name, void (*func)(void) ) {

	cvar_t *var;

	if( (var = CV_FindVar( var_name )) == NULL ) {
		// there is an error in C code if this happens
		xprintf("CV_SetFunc: variable \"%s\" not found.\n", var_name);
		return;
	}

	var->func = func;

	return;
}




//
// Handles variable inspection and changing from the console
//
BOOL CV_Command( char *cmd, char *params ) {

	cvar_t *v;

	if( cmd == NULL )
		return FALSE;

	// check variables
	if( (v = CV_FindVar( cmd )) == NULL )
		return FALSE;

	// perform a variable print or set
	if( params == NULL ) {
		xprintf("%s is \"%s\"\n", v->name, v->string);
		return TRUE;
	}

	CV_Set( v->name, params );

	if( v->func )
		v->func();

	return TRUE;
}




//
// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.
//
void CV_WriteVariables( FILE *f ) {

	cvar_t	*var;

	for( var=cvar_vars; var; var = var->next )
//		if( var->archive )
			fprintf( f, "%s \"%s\"\n", var->name, var->string );

	return;
}





/////////////// USER FUNCTIONS //////////////////////


extern "C" char *getXlibVersion( void );

//
// this is just a REALLY REALLY simple parser
//
void genericParse( char *commandName, char *s ) {


	if( !stricmp( commandName, "ver" ) ) {

		xprintf("%s\n", XLIB_winSetProgramDesc(NULL) );
		xprintf("%s\n", getXlibVersion() );

		return;
	}

	if( !stricmp( commandName, "quit" ) || !stricmp( commandName, "exit" ) ) {

		Quit(NULL);

		return;
	}

	if( !stricmp( commandName, "echo" ) ) {

		if( s != NULL ) {
			if( !stricmp( s, "params" ) ) {

				int i;

				for( i=0; i<myargc; i++ )
					xprintf( "%d: \"%s\"", i, myargv[i] );
			}
			else {
				xprintf( "%s\n", s );
				console.result_type = CON_STRING;
				strncpy( console.result_string, s, XMAX_PATH );
			}
		}
		else
			xprintf( "%s\n", commandName );

		return;
	}

	if( !stricmp( commandName, "status" ) ) {

		if( s != NULL ) {
			StatusText( ST_BW, s );
		}
		else
			xprintf("Usage: %s [text].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "p64" ) ) {

		P64( s );

		return;
	}

	if( !stricmp( commandName, "browser" ) ) {

		winBrowser( s );

		return;
	}

	if( !stricmp( commandName, "videomode" ) ) {

		char str[XMAX_PATH] = "Unidentified";
		if( GFXDLL_GetDescription ) GFXDLL_GetDescription(str);

		xprintf("Video mode: %d x %d %dbit%s(Driver: %s)\n", SCREENW, SCREENH, bpp, winFullscreen()?" (Fullscreen) ":" ", str );

		return;
	}

	if( !stricmp( commandName, "aboutfull" ) ) {
		char str[XMAX_PATH] = "Unidentified";
		char str2[XMAX_PATH*2];
		if( GFXDLL_GetDescription ) GFXDLL_GetDescription(str);

		sprintf(str2,"%s\n%s\n\nVideo mode: %d x %d %dbit%s(Driver: %s)", XLIB_winSetProgramDesc(NULL), getXlibVersion(), SCREENW, SCREENH, bpp, winFullscreen()?" (Fullscreen) ":" ", str );

		if( s != NULL ) {
			strcat(str2,"\n\n");
			strcat(str2,s);
		}

		MessageTitle("About",str2);

		return;
	}


	if( !stricmp( commandName, "togglefullscreen" ) ) {

		ToggleFullscreen();

		return;
	}

	if( !stricmp( commandName, "lua" ) ) {

		if( s != NULL ) {

			if( FileExist(s) )
				LUA_RunFile( s );
			else
				LUA_RunString( s );
		}
		else
			xprintf( "Usage: %s [text or filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "playsound" ) ) {

		if( s != NULL && FileExist(s) ) {
			PLAYSOUND( s );
		}
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "crc" ) ) {

		if( s != NULL && FileExist(s,TRUE) ) {

#define BUFFSIZE (16*1024)

			char *buf = NULL;
			size_t size;
			FILE *f;

			ALLOCMEM(buf,BUFFSIZE);

			if( (f=fopen(s,"rb")) != NULL ) {

				UINT crc = 0;
				UINT sum = 0;

				while( (size = fread(buf, 1, BUFFSIZE, f) ) > 0)
					CalcCrc( buf, size, &crc, &sum );

				fclose( f );

				// xprintf("%s: %u\n",s,crc);
				// char str[XMAX_PATH];
				// sprintf(str,"%s: %u",s,crc);
				// XLIB_winCopyTextToClipboard( str );

				console.result_type = CON_STRING;
				sprintf(console.result_string,"%u-%u",crc,sum);
			}

			FREEMEM(buf);
		}
		else
		if( s != NULL ) {
			// csak string

			UINT crc = 0;
			UINT sum = 0;

			CalcCrc( s, strlen(s), &crc, &sum );

			console.result_type = CON_STRING;
			sprintf(console.result_string,"%u-%u",crc,sum);
		}
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "encstr" ) ) {

		if( s != NULL ) {

			if( strlen(s) > 255 ) {
				xprintf( "String too long.\n" );
				return;
			}

			UCHAR *buf = NULL;

			int len = EncodeLZA( (UCHAR *)s, strlen(s), PTR(buf) );
			// int len = ZIP_Compress( (UCHAR *)s, strlen(s), PTR(buf) );

			if( len >= (MAX_RESSTR - 3) ) {
				FREEMEM( buf );
				xprintf( "Array too long.\n" );
				return;
			}

			// TomFlag(NOFLAG);
			// XL_WriteFile("be", buf, len );

			char *p = &console.result_string[0];
			char *letter = "0123456789ABCDEF";

			int n = strlen(s);
			*p++ = letter[(n>>4) % 16];
			*p++ = letter[n % 16];

			for( int i=8; i<len; i++ ) {

				n = (int)buf[i];

				*p++ = letter[(n>>4) % 16];
				*p++ = letter[n % 16];
			}

			*p++ = 0;

			FREEMEM( buf );

			console.result_type = CON_STRING;
		}
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "decstr" ) ) {

		if( s != NULL ) {

#define h2i( ch ) (						\
	('a' <= (ch) && (ch) <= 'f') ? ((ch) - 'a' + 10) : 	\
	('A' <= (ch) && (ch) <= 'F') ? ((ch) - 'A' + 10) : 	\
	('0' <= (ch) && (ch) <= '9') ? ((ch) - '0') : 0 )

			UCHAR *buf;
			UCHAR *cmpr,*q;

			ALLOCMEM( cmpr, 4 + 4 + (strlen(s)-2)/2 );

			memcpy( cmpr, FIS_LZA, 4 );
			q = &cmpr[4];

			char *p = s;

			int i = h2i( *p ) * 16;
			++p;
			i |= h2i( *p );
			++p;
			*q++ = i;
			*q++ = 0;
			*q++ = 0;
			*q++ = 0;

			for( i=2; i<strlen(s); i+=2 ) {
				char a = *p++;
				char b = *p++;
				*q++ = ((char) ((h2i(a) << 4) | h2i(b)));
			}

			// TomFlag(NOFLAG);
			// XL_WriteFile("val", cmpr, 4 + 4 + (strlen(s)-2)/2 );

			int len = DecodeLZA( cmpr, PTR(buf) );

			memcpy( console.result_string, buf, len );
			console.result_string[len] = 0;

			FREEMEM( cmpr );
			FREEMEM( buf );

			console.result_type = CON_STRING;
		}
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}


	if( !stricmp( commandName, "soundfont" ) ) {

		if( s != NULL && FileExist(s) )
			SL_LoadSoundFont( s );
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "del" ) ) {

		if( s != NULL )
			UnlinkFile( s );
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "shortpath" ) ) {

		if( winShortPath(s,console.result_string) )
			console.result_type = CON_STRING;
		else
			xprintf( "Usage: %s [filename].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "im" ) ) {

		if( s != NULL && ImageMagickCommand(s) )
			; // console.result_type = CON_STRING;
		else
			xprintf( "Usage: %s [ImageMagick command string].\n", commandName );

		return;
	}

	if( !stricmp( commandName, "caption" ) ) {

		if( s != NULL ) {
			XLIB_winSetCaption( s );
		}
		else
			xprintf("Usage: %s [text].\n",commandName );

		return;
	}


	if( !stricmp( commandName, "clear" ) ) {

		consoleClearBuffers();

		return;
	}

	if( !stricmp( commandName, "memstat" ) ) {

		MemStats();

		return;
	}

	if( !stricmp( commandName, "memcheck" ) ) {

		MemCheck();

		return;
	}

	if( !stricmp( commandName, "cdeject" ) ) {

		CD_Eject();

		return;
	}

	if( !stricmp( commandName, "baszki" ) ) {

		xprintf( "Kicsinaltak Kennyt?\n" );

		return;
	}

	if( !stricmp( commandName, "log" ) ) {

		winSetNoLog( FALSE );
		winToggleLog();

		return;
	}

	if( !stricmp( commandName, "datetime" ) ) {

		// int days, hours, minutes, seconds;
		int year,day,hour,min,mp;

		GetDate( &year,NULL,&day);
		GetTime( &hour,&min,&mp );

		char str[XMAX_PATH];
		sprintf(str,"The current date is \"%d %s %d, %d:%02d:%02d\".\n",year,GetMonth(),day,hour,min,mp);
		xprintf(str);

		console.result_type = CON_STRING;
		strncpy( console.result_string, str, XMAX_PATH );

		return;
	}

	if( !stricmp( commandName, "dumptexmap" ) ) {

		if( XLIB_CheckBernie() )
			DumpTexmap();
		else
			xprintf( "Nothing happens.\n" );

		return;
	}

	if( !stricmp( commandName, "listtexmap" ) ) {

		ListTexmap();

		return;
	}


	if( !stricmp( commandName, "reloadtexmap" ) ) {

		ReloadAllTexMap();

		return;
	}

	if( !stricmp( commandName, "dirlist" ) ) {

		CacheSubdir( "Power Word: List", NULL );
		return;
	}

	if( !stricmp( commandName, "dirdump" ) ) {

		CacheSubdir( "Power Word: Dump", NULL );
		return;
	}

	if( !stricmp( commandName, "say" ) ) {

		if( s != NULL ) {
			char *txt;
			if( LoadFile(s, PTR(txt)) > 0 ) {
				SpeakText( txt );
				FREEMEM(txt);
			}
			else
				SpeakText( s );
		}
		else
			xprintf("Usage: %s [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "path2url" ) ) {

		if( s != NULL ) {
			if( Path2URL(s, console.result_string) ) {
				console.result_type = CON_STRING;
				// xprintf( "%s\n", console.result_string );
			}
		}
		else
			xprintf("Usage: %s [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "getfilenamenopath" ) ) {

		if( s != NULL ) {
			console.result_type = CON_STRING;
			strncpy( console.result_string, GetFilenameNoPath( s ), XMAX_PATH );
			// xprintf( "%s\n", console.result_string );
		}
		else
			xprintf("Usage: %s [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "getfileext" ) ) {

		if( s != NULL ) {
			char *ext = GetExtension( s );
			if( ext ) {
				console.result_type = CON_STRING;
				strcpy( console.result_string, ext );
				// xprintf( "%s\n", console.result_string );
			}
		}
		else
			xprintf("Usage: %s [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "gettemp" ) ) {

		console.result_type = CON_STRING;
		sprintf( console.result_string, "%s%c", (char *)GetRealTemp(), PATHDELIM );
		// xprintf( "%s\n", console.result_string );

		return;
	}

	if( !stricmp( commandName, "beep" ) ) {

		XLIB_winBeep();

		return;
	}

	if( !stricmp( commandName, "findrandomwindow" ) ) {

		console.result_type = CON_STRING;
		XLIB_FindRandomWindow(console.result_string);

		return;
	}

	if( !stricmp( commandName, "findrandompid" ) ) {

		console.result_type = CON_NUMBER;
		console.result_number = XLIB_FindRandomPid();

		return;
	}

	if( !stricmp( commandName, "printwindow" ) ) {

		XLIB_PrintWindow();

		return;
	}

	if( !stricmp( commandName, "espeak" ) ) {

		if( s != NULL ) {

			char *str = s;

			InitEspeak();

			// nyelv lehet
			if( str[0] == '-' ) {
				int i = 1;
				char lang[200];
				char *p = lang;
				while( str[i] != ' ' && i < 200 ) {
					*p++ = str[i];
					if( str[i] == 0 ) {
						i = 9001;	// hackerance: nem nyelv lesz
						break;
					}
					++i;
				}
				if( i < 200 ) {
					*p++ = 0;
					++i;
					int gender = 0;
					if( !strnicmp( "-female ",&str[i],8) ) {
						gender = 1;
						i += 8;
					}
					// xprintf("lang: \"%s\"\n",lang);
					// xprintf("string: \"%s\"\n",&str[i]);
					// xprintf("gender: %d\n",gender);
					if( EspeakSetVoice(lang,gender) == TRUE ) {
						// van ilyen nyelv
						str = &str[i];
					}
					else
					if( !strnicmp( "-female ",str,8) ) {
						str = &str[8];
						EspeakSetVoice("default",1);
					}
				}
			}
			else
				EspeakSetVoice("default");

			char *txt;
			if( LoadFile(str, PTR(txt)) > 0 ) {
				EspeakText( txt );
				FREEMEM(txt);
			}
			else
				EspeakText( str );
		}
		else
			xprintf("Usage: %s [-language -female] [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "overlay" ) ) {

		if( s != NULL ) {
			SetOverlayParam( 50 );
			Overlay( s );
		}
		else
			xprintf("Usage: %s [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "msgbox" ) ) {

		if( s != NULL ) {

			char *p = NULL;
			char *start;

			// első "
			if( (start = strchr(s,'\"')) && ++start != 0 ) {
				char *end;
				// van-e második "
				if( (end = strchr(start,'\"')) ) {
					*end = 0;
					s = start;
					if( ++end ) {
						// title van-e első "
						if( (start = strchr(end,'\"')) && ++start != 0 ) {
							// title van-e második "
							if( (end = strchr(start,'\"')) ) {
								*end = 0;
								p = start;
							}
							else {
								--start; 	// benne legyen a " is
								p = start;
							}
						}
						else
							p = end;
					}
				}
			}

			// xprintf("s: %s\n",s);
			// xprintf("p: %s\n",p);

			XLIB_Message( s, p );
		}
		else
			xprintf("Usage: %s [title] [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "askyesno" ) ) {

		if( s != NULL ) {

			char *p = NULL;
			char *start;

			// első "
			if( (start = strchr(s,'\"')) && ++start != 0 ) {
				char *end;
				// van-e második "
				if( (end = strchr(start,'\"')) ) {
					*end = 0;
					s = start;
					if( ++end ) {
						// title van-e első "
						if( (start = strchr(end,'\"')) && ++start != 0 ) {
							// title van-e második "
							if( (end = strchr(start,'\"')) ) {
								*end = 0;
								p = start;
							}
							else {
								--start; 	// benne legyen a " is
								p = start;
							}
						}
						else
							p = end;
					}
				}
			}

			// xprintf("s: %s\n",s);
			// xprintf("p: %s\n",p);

			console.result_type = CON_BOOL;
			console.result_bool = winAskYesNo( s, p );
		}
		else
			xprintf("Usage: %s [title] [text or filename].\n",commandName );

		return;
	}



	if( !stricmp( commandName, "ispic" ) ) {

		if( s != NULL ) {
			console.result_type = CON_BOOL;
			console.result_bool = IsFree( s );
			// xprintf("bool: %s (%s)\n", console.result_bool?"True":"False",s);
		}
		else {
			console.result_type = CON_BOOL;
			console.result_bool = FALSE;
			xprintf("Usage: %s [title] [text or filename].\n",commandName );
		}

		return;
	}

	if( !stricmp( commandName, "toclipboard" ) ) {

		if( s != NULL )
			XLIB_winCopyTextToClipboard( s );
		else
			xprintf("Usage: %s [text or filename].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "fromclipboard" ) ) {

		char *str;
		if( (str = XLIB_winPasteTextFromClipboard()) != NULL ) {
			strcpy( console.result_string, str );
			console.result_type = CON_STRING;
		}

		return;
	}

	if( !stricmp( commandName, "tic" ) ) {

		int tic = GetTic()/TICKBASE;

		int days = (int)(tic/(24*60*60));
		int hours = (tic/(60*60))%24;
		int mins = (tic/60)%60;
		int secs = tic%60;

		char time[100];
		char tmp[100];
		BOOL volt = FALSE;

		time[0] = 0;

		if( days ) { volt = TRUE; sprintf( tmp, "%d days ", days ); strcpy( time, tmp ); }
		if( hours ) { sprintf( tmp, volt?"%02d:":"%d:", hours ); volt = TRUE; strcat( time, tmp ); }
		sprintf( tmp, volt?"%02d:%02d":"%d:%02d", mins, secs ); strcat( time, tmp );

		xprintf( "Running time is %s.\n",time);

		console.result_type = CON_NUMBER;
		console.result_number = tic;

		return;
	}

	if( !stricmp( commandName, "exec" ) ) {

		if( s != NULL && XLIB_CheckBernie() )
			XLIB_winExec( s );
		else
			xprintf("Usage: %s [command].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "getinput" ) ) {

		char *p = NULL;

		if( s != NULL ) {

			char *start;

			// első "
			if( (start = strchr(s,'\"')) && ++start != 0 ) {
				char *end;
				// van-e második "
				if( (end = strchr(start,'\"')) ) {
					*end = 0;
					s = start;
					if( ++end ) {
						// title van-e első "
						if( (start = strchr(end,'\"')) && ++start != 0 ) {
							// title van-e második "
							if( (end = strchr(start,'\"')) ) {
								*end = 0;
								p = start;
							}
							else {
								--start; 	// benne legyen a " is
								p = start;
							}
						}
						else
							p = end;
					}
				}
			}

			// xprintf("s: %s\n",s);
			// xprintf("p: %s\n",p);

			// console('GetInput "(null)" "Encode:"')
			if( !stricmp( s, "(null)" ) )
				console.result_string[0] = 0;
			else
				strcpy(console.result_string,s);
		}

		if( winGetInput(console.result_string,p) )
			console.result_type = CON_STRING;

		return;
	}


	if( !stricmp( commandName, "startavi" ) ) {

		char aviname[XMAX_PATH];

		if( s != NULL ) {
			char *e = GetExtension(s);
			if( !e || stricmp( e, "avi" ) )
				sprintf(aviname,"%s.avi",s);
			else
				strcpy(aviname,s);
			if( FileExist(aviname,TRUE) ) {
				if( winAskYesNo("Record Video", "Overwrite %s?",aviname) )
					AVI_Create( aviname );
			}
			else
				AVI_Create( aviname );
		}
		else {
			int i;
			for( i=1; i<=99; i++ ) {
				sprintf(aviname,"video%02d.avi",i);
				if( FileExist(aviname,TRUE) == FALSE )
					break;
			}
			if( i>=100 )
				xprintf( "Usage: %s [text or filename].\n", commandName );
			else
				AVI_Create( aviname );
		}

		return;
	}

	if( !stricmp( commandName, "stopavi" ) ) {

		AVI_Finalize();

		return;
	}

	if( !stricmp( commandName, "screenshot" ) ) {

		if( s != NULL ) {
			char jpgname[XMAX_PATH];
			char *e = GetExtension(s);
			if( !e || stricmp( e, "jpg" ) )
				sprintf(jpgname,"%s.jpg",s);
			else
				strcpy(jpgname,s);
			ScreenShot( jpgname );
		}
		else
			ScreenShot();

		return;
	}


	if( !stricmp( commandName, "wget" ) || !stricmp( commandName, "curl" ) ) {

		if( s == NULL ) {
			xprintf( "Usage: %s [webaddress] [filename].\n", commandName );
			return;
		}

		char web[XMAX_PATH],name[XMAX_PATH];
		int i,j;

		for( i=0; i<strlen(s); i++ ) {
			if( i >= XMAX_PATH )		// túl hosszú a név
				return;
			web[i] = s[i];
			if( web[i] == ' ' )
				break;
		}
		web[i] = 0;

		if( i == 0 || i >= strlen(s) )
			return;

		if( strnicmp( web, "http:///", 7) ) {
			char temp[XMAX_PATH+7];
			strcpy( temp, web );
			strcpy( web, "http://" );
			strcat( web, temp );
		}

		// ha csak http:// -ből áll
		if( strlen(web) <= 7 )
			return;

		// skip spaces
		while( s[i] == ' ' )
			if( ++i >= strlen(s) )
				return;

		for( j=0; j<strlen(s)-i; j++ ) {
			if( j >= XMAX_PATH )		// túl hosszú a név
				return;
			name[j] = s[i+j];
			if( name[j] == ' ' )
				break;
		}
		name[j] = 0;

		winShowPanel( name, web );

		if( !stricmp( commandName, "wget" ) )
			XLIB_winGetUrlDocument( web, NULL, name );

		if( !stricmp( commandName, "curl" ) )
			XLIB_GetCurlURL( web, NULL, name  );

		winHidePanel();

		// xprintf("web: \"%s\"\n",web);
		// xprintf("name: \"%s\"\n",name);

		return;
	}

	if( !stricmp( commandName, "splash" ) ) {

		if( XLIB_winNoSplash() )
			xprintf("can't display splash.\n");

		winShowSplash();
		winSetSplashText( s );

		return;
	}

	if( !stricmp( commandName, "shareware" ) ) {

		console.result_bool = XLIB_winShareware(TRUE);
		console.result_type == CON_BOOL;

		return;
	}

	if( !stricmp( commandName, "wingamma" ) ) {

		if( s != NULL ) {
			int temp = atoi(s);
			// SetGamma( 128 = normal [0..255] )
			XLIB_SetBrightness( temp );
		}
		else
			xprintf("Usage: %s [command].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "setcore" ) ) {

		if( s != NULL ) {
			int temp = atoi(s);
			SetCore( temp );
		}
		else
			xprintf("Usage: %s [command].\n",commandName );

		return;
	}

	if( !stricmp( commandName, "refreshjoystick" ) ) {

		XLIB_RefreshJoystick();

		return;
	}

	if( !stricmp( commandName, "shutdowncomputer" ) ) {

		XLIB_Shutdown();

		return;
	}

	if( !stricmp( commandName, "rebootcomputer" ) ) {

		XLIB_Shutdown( TRUE );

		return;
	}

	if( !stricmp( commandName, "clock" ) ) {

		int temp = -1;

		if( s != NULL )
			temp = atoi(s);

		winClock( temp );

		return;
	}

	if( !stricmp( commandName, "xproc1" ) ) {
		extern void lockFore( void );
		lockFore();
		return;
	}

	if( !stricmp( commandName, "checkkbdhook" ) ) {

		XLIB_CheckKeyboardHook();

		return;
	}

	return;
}



//
//
//
int isChars( char *s ) {

	// can only have 0-9
	for( ; *s; *s ++ )
		if (*s != '0' && *s != '1' && *s != '2' &&
			*s != '3' && *s != '4' && *s != '5' &&
			*s != '6' && *s != '7' && *s != '8' && *s != '9')
				return 1;

	return 0;
}




//
//
//
void toggleVarState( char *commandName, char *s ) {

	int temp;

	int ani = 0;
	int fill = 1;

	if( (s == NULL) || isChars(s) ) {
/*
		if( commandId == 0 )
			consolePrintf( "\"ani\" is \"%d\"", ani );
		else
		if( commandId == 1 )
			consolePrintf( "\"fill\" is \"%d\"", fill );
*/
		return;
	}

	temp = atoi(s);

	if( temp == 0 || temp == 1 ) {
/*
		if( commandId == 0 ) {
			ani = temp;
			consolePrintf( "\"ani\" is \"%d\"", ani );
		}
		else
		if( commandId == 1 ) {

			fill = temp;

			if( fill )
				glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

			consolePrintf( "\"fill\" is \"%d\"", fill );
		}
*/
	}

	return;
}



//
//
//
void setVarValue( char *commandName, char *s ) {

	return;
}



//
//
//
void commandDump( char *commandName, char *s ) {

	int i;

	consolePrintf( "Commands:\n" );

	for( i=0; i<ncomms; i++ )
		consolePrintf( "%s\n", table[i].name );

	return;
}




//
//
//
static void playCD( char *commandName, char *s ) {

	int i = 1;

	if( s != NULL )
		sscanf( s, "%d", &i );

	xprintf("playing track %d...\n", i );
	CD_PlayTrack(i);

	return;
}



//
//
//
static void checkMail( char *commandName, char *s ) {

	BOOL email = FALSE;

	if( MA_Init() == TRUE ) {

		char felado[256], subj[256], date[256];
		BOOL unread;

		MA_ReadFirst();

		while( MA_ReadNext() ) {
			MA_ReadMail( felado, NULL, date, subj, NULL, &unread );
			// xprintf("%s%-30s %-50s %-20s\n", unread?"New: ":"",felado, subj, date );
			if( unread ) {
				xprintf("New mail from \"%s\", \"%s\".\n", felado, subj );
				email = TRUE;
			}
		}

		MA_Deinit();
	}

	return; // email;
}




