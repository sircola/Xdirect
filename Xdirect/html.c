
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <windows.h>
#include <xlib.h>

#include <html.h>



static HINSTANCE hHtml = NULL;


static int __cdecl (*pEmbedBrowserObject)(HWND h) = NULL;
static void __cdecl (*pUnEmbedBrowserObject)(HWND h) = NULL;
static int __cdecl (*pDisplayHTMLPage)(HWND h, LPCTSTR s) = NULL;
static int __cdecl (*pDisplayHTMLStr)(HWND h, LPCTSTR s) = NULL;
static int __cdecl (*pIsHTMLReady)(HWND h) = NULL;
static void __cdecl (*pDoPageAction)(HWND h, DWORD i) = NULL;
static void __cdecl (*pResizeBrowser)(HWND h, DWORD i, DWORD j) = NULL;
static char __cdecl *(*pGetUrlString)( void ) = NULL;
static char __cdecl *(*pHtmlGetInfo)( void ) = NULL;
static int __cdecl (*pGetHTMLPage)(HWND h, char *s, int l) = NULL;
static int __cdecl (*pSetValue)(HWND h, char *s, char *f) = NULL;
static int __cdecl (*pClickButton)(HWND h, char *s) = NULL;
static int __cdecl (*pComboBox)(HWND h, char *s, int l) = NULL;


#define XHTMLDLLNAME  XWINGYOGYDLLNAME





//
//
//
static int LoadHtml( void ) {

	if( hHtml )
		return 0;

	char bassdllname[XMAX_PATH];
	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, XHTMLDLLNAME );

	if( (hHtml = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadHtml: couldn't load \"%s\" library.\n",bassdllname);
		return 1;
	}

	if( (pEmbedBrowserObject = (int (__cdecl*)(HWND))GetProcAddress( hHtml, "EmbedBrowserObject" )) == NULL ) {
		xprintf("no EmbedBrowserObject\n" );
		// nem lehet kitölteni merta speech is ott van
		// FreeLibrary( hHtml );
		hHtml = NULL;
		return 2;
	}

	if( (pUnEmbedBrowserObject = (void (__cdecl*)(HWND))GetProcAddress( hHtml, "UnEmbedBrowserObject" )) == NULL ) {
		xprintf("no UnEmbedBrowserObject\n" );
		hHtml = NULL;
		return 3;
	}

	if( (pDisplayHTMLPage = (int (__cdecl*)(HWND, LPCTSTR))GetProcAddress( hHtml, "DisplayHTMLPage" )) == NULL ) {
		xprintf("no DisplayHTMLPage\n" );
		hHtml = NULL;
		return 4;
	}

	if( (pDisplayHTMLStr = (int (__cdecl*)(HWND, LPCTSTR))GetProcAddress( hHtml, "DisplayHTMLStr" )) == NULL ) {
		xprintf("no DisplayHTMLStr\n" );
		hHtml = NULL;
		return 5;
	}

	if( (pGetHTMLPage = (int (__cdecl*)(HWND, char*, int l))GetProcAddress( hHtml, "GetHTMLPage" )) == NULL ) {
		xprintf("no GetHTMLPage\n" );
		hHtml = NULL;
		return 4;
	}

	if( (pComboBox = (int (__cdecl*)(HWND, char*, int l))GetProcAddress( hHtml, "ComboBox" )) == NULL ) {
		xprintf("no ComboBox\n" );
		hHtml = NULL;
		return 4;
	}

	if( (pClickButton = (int (__cdecl*)(HWND, char*))GetProcAddress( hHtml, "ClickButton" )) == NULL ) {
		xprintf("no ClickButton\n" );
		hHtml = NULL;
		return 4;
	}

	if( (pSetValue = (int (__cdecl*)(HWND, char*, char*))GetProcAddress( hHtml, "SetValue" )) == NULL ) {
		xprintf("no SetValue\n" );
		hHtml = NULL;
		return 4;
	}

	if( (pIsHTMLReady = (int (__cdecl*)(HWND))GetProcAddress( hHtml, "IsHTMLReady" )) == NULL ) {
		xprintf("no IsHTMLReady\n" );
		hHtml = NULL;
		return 6;
	}

	if( (pDoPageAction = (void (__cdecl*)(HWND,DWORD))GetProcAddress( hHtml, "DoPageAction" )) == NULL ) {
		xprintf("no DoPageAction\n" );
		hHtml = NULL;
		return 7;
	}

	if( (pResizeBrowser = (void (__cdecl*)(HWND,DWORD,DWORD))GetProcAddress( hHtml, "ResizeBrowser" )) == NULL ) {
		xprintf("no ResizeBrowser\n" );
		hHtml = NULL;
		return 8;
	}

	if( (pGetUrlString = (char * (__cdecl*)(void))GetProcAddress( hHtml, "GetUrlString" )) == NULL ) {
		xprintf("no GetUrlString\n" );
		hHtml = NULL;
		return 9;
	}

	if( (pHtmlGetInfo = (char * (__cdecl*)(void))GetProcAddress( hHtml, "GetInfo" )) == NULL ) {
		xprintf("no GetInfo\n" );
		hHtml = NULL;
		return 9;
	}

	xprintf("LoadHtml: %s (%s) loaded.\n", pHtmlGetInfo(), bassdllname );

	return 0;
}



//
//
//
int HTML_EmbedBrowserObject(HWND h) {

	if( LoadHtml() )
		return FALSE;

	return pEmbedBrowserObject(h);
}

//
//
//
void HTML_UnEmbedBrowserObject(HWND h) {

	if( pUnEmbedBrowserObject )
		pUnEmbedBrowserObject(h);

	return;
}


//
//
//
int HTML_DisplayPage( HWND h, LPCTSTR s ) {

	if( LoadHtml() )
		return 0;

	return pDisplayHTMLPage(h, s);
}


//
//
//
int HTML_GetPage( HWND h, char *s, int len ) {

	if( LoadHtml() )
		return 0;

	return pGetHTMLPage(h, s, len);
}



//
//
//
int HTML_DisplayStr( HWND h, LPCTSTR s ){

	if( LoadHtml() )
		return 0;

	return pDisplayHTMLStr(h, s);
}


//
//
//
int HTML_IsReady( HWND h ){

	if( pIsHTMLReady )
		return pIsHTMLReady(h);

	return 1;
}


//
//
//
void HTML_DoPageAction(HWND h, DWORD i) {

	if( pDoPageAction )
		return pDoPageAction(h,i);

	return;
}


//
//
//
void HTML_ResizeBrowser(HWND h, DWORD i, DWORD j) {

	if( pResizeBrowser )
		return pResizeBrowser(h,i,j);

	return;
}


//
//
//
char *HTML_GetUrlString( void ){

	if( pGetUrlString )
		return pGetUrlString();

	return NULL;
}


//
//
//
int HTML_ClickButton( HWND h, char *tag ) {

	if( pClickButton )
		return pClickButton( h, tag );

	return 0;
}

//
//
//
int HTML_SetValue( HWND h, char *tag, char *str ) {

	if( pSetValue )
		return pSetValue( h, tag, str );

	return 0;
}


//
//
//
int HTML_ComboBox( HWND h, char *tag, int index ) {

	if( pComboBox )
		return pComboBox( h, tag, index );

	return 0;
}

