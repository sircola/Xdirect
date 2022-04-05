
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <mmsystem.h>
#include <stddef.h>

#include <xlib.h>
#include <xinner.h>

#include "resource.h"

// #include <DebugHlp.h>


#include <html.h>

RCSID( "$Id: Xmain.cpp,v 1.2 2003/09/22 13:59:57 bernie Exp $" )


#ifdef _MSC_VER
// #pragma comment(lib, "mmtimer")
#endif


#ifdef _DEBUG
#define XLIBDLLNAME "DEBUG_xlib.dll"
#else
#define XLIBDLLNAME "xlib.dll"
#endif

// #pragma optimize("", off)
// #pragma init_seg(lib)
// #pragma optimize("", off)


#define __XLIB_WIN32


extern "C" int usermain( int argc, char *argv[], char *envp[] );


static int (GFXDLLCALLCONV *XLIB_WinMain)( HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow, ULONG proc ) = NULL;

char *(GFXDLLCALLCONV *GetWinError)( void ) = NULL;
char *(GFXDLLCALLCONV *winTemp)( void ) = NULL;
void (GFXDLLCALLCONV *GetWinMemInfo)( winmeminfo_ptr ) = NULL;
BOOL (GFXDLLCALLCONV *isArmadillo)( void ) = NULL;

BOOL (GFXDLLCALLCONV *IsIjl)( char *name ) = NULL;
BOOL (GFXDLLCALLCONV *LoadIjl)( char *fname, memptr_ptr ptr ) = NULL;
BOOL (GFXDLLCALLCONV *SaveIjl)( char *name, UCHAR *s, UCHAR *p ) = NULL;
void (GFXDLLCALLCONV *SetRgbIjl)(void) = NULL;
void (GFXDLLCALLCONV *GetRgbIjl)(UCHAR *pal) = NULL;
BOOL (GFXDLLCALLCONV *GetIjlDim)(char*,int*,int*) = NULL;

BOOL (GFXDLLCALLCONV *SetX)( int width, int height, int depth, int flag ) = NULL;
void (GFXDLLCALLCONV *ResetX)( void ) = NULL;

BOOL (GFXDLLCALLCONV *AVI_Open)( char* szFilename ) = NULL;
void (GFXDLLCALLCONV *AVI_Close)( void ) = NULL;
void (GFXDLLCALLCONV *AVI_DrawFrame)( int Frame, int x, int y, int w, int h ) = NULL;
UCHAR* (GFXDLLCALLCONV *AVI_GetFrame)( int Frame, int flag ) = NULL;


int (GFXDLLCALLCONV *winCopyTextToClipboard)( char *str ) = NULL;
char *(GFXDLLCALLCONV *winPasteTextFromClipboard)( void ) = NULL;
BOOL (GFXDLLCALLCONV *winGetSysDirectory)( char *szPath ) = NULL;
void *(GFXDLLCALLCONV *winFindFirstFile)( char *mask, findfile_ptr find ) = NULL;
BOOL (GFXDLLCALLCONV *winFindNextFile)( void *handle, findfile_ptr find ) = NULL;
BOOL (GFXDLLCALLCONV *winFindClose)( void *handle ) = NULL;
BOOL (GFXDLLCALLCONV *winDeleteFile)( char *name ) = NULL;
int (GFXDLLCALLCONV *winGetCurrentDirectory)( char *dir, int num ) = NULL;
BOOL (GFXDLLCALLCONV *winSetCurrentDirectory)( char *dir ) = NULL;
BOOL (GFXDLLCALLCONV *winGetFontDir)( char *szPath ) = NULL;
BOOL (GFXDLLCALLCONV *winGetProgramFilesDir)( char *szPath ) = NULL;
BOOL (GFXDLLCALLCONV *winGetMyDocumentsDir)( char *szPath ) = NULL;
BOOL (GFXDLLCALLCONV *winBrowseForFolder)( char *szPath ) = NULL;
BOOL (GFXDLLCALLCONV *winIsConnected)( void ) = NULL;
int (GFXDLLCALLCONV *winLoadBitmap)( void ) = NULL;
int (GFXDLLCALLCONV *winPlayWave)( char *str ) = NULL;
BOOL (GFXDLLCALLCONV *winMagyar)( void ) = NULL;
int (GFXDLLCALLCONV *winReadProfileInt)( char *key, int default_value ) = NULL;
void (GFXDLLCALLCONV *winWriteProfileInt)( char *key, int value ) = NULL;
FLOAT (GFXDLLCALLCONV *winReadProfileFloat)( char *key, FLOAT default_value ) = NULL;
void (GFXDLLCALLCONV *winWriteProfileFloat)( char *key, FLOAT value ) = NULL;
void (GFXDLLCALLCONV *winReadProfileString)( char *key, char *default_string, char *string, int size ) = NULL;
void (GFXDLLCALLCONV *winWriteProfileString)( char *key, char *string ) = NULL;
BOOL (GFXDLLCALLCONV *winDeleteProfileKey)( char *key ) = NULL;
void (GFXDLLCALLCONV *GetUptime)( void ) = NULL;
int (GFXDLLCALLCONV *GetCPUSpeed)( void ) = NULL;
void (GFXDLLCALLCONV *xsystem)( char * ) = NULL;
void *(GFXDLLCALLCONV *winAlloc)( int amount ) = NULL;
void (GFXDLLCALLCONV *winFree)( void *buf ) = NULL;
BOOL (GFXDLLCALLCONV *IsSofticeLoaded)( void ) = NULL;
void (GFXDLLCALLCONV *EnumerateProcessorType)( void ) = NULL;
BOOL (GFXDLLCALLCONV *CheckBernie)( void ) = NULL;
const char *(GFXDLLCALLCONV *winGetMachineName)( void ) = NULL;
const char *(GFXDLLCALLCONV *winGetUserName)( void ) = NULL;
void (GFXDLLCALLCONV *winSetCaption)( char *str, ... ) = NULL;
int (GFXDLLCALLCONV *winLogLevel)( void ) = NULL;
char *(GFXDLLCALLCONV *winGetNextFile)( void ) = NULL;
int (GFXDLLCALLCONV *winGetUrlDocument)( char *url, UCHAR **buf ) = NULL;
BOOL (GFXDLLCALLCONV *getNetVersion)( char *url, int *major, int *minor, char *download ) = NULL;
BOOL (GFXDLLCALLCONV *CheckVersion)( char *name, char *version ) = NULL;
void (GFXDLLCALLCONV *DumpProcessInfo)( void ) = NULL;
ULONG (GFXDLLCALLCONV *winFindTargetWindow)( char *windowName ) = NULL;
BOOL (GFXDLLCALLCONV *winNoSplash)( void ) = NULL;
BOOL (GFXDLLCALLCONV *winRegNoLog)( void ) = NULL;
int (GFXDLLCALLCONV *winTimeZone)( void ) = NULL;
BOOL (GFXDLLCALLCONV *winGetGammaRamp)( void ) = NULL;
BOOL (GFXDLLCALLCONV *winSetGammaRamp)( FLOAT f ) = NULL;
void (GFXDLLCALLCONV *winSetWindowTrans)( __int64 hwnd, BOOL bTransparent, int nTransparency ) = NULL;
UINT (GFXDLLCALLCONV *winGetMouseScrollLines)( void ) = NULL;
BOOL (GFXDLLCALLCONV *winLoadFonts)( void ) = NULL;
void (GFXDLLCALLCONV *winRemoveFonts)( void ) = NULL;
int (GFXDLLCALLCONV *winGetMaxWidth)( void ) = NULL;
int (GFXDLLCALLCONV *winGetMaxHeight)( void ) = NULL;
void (GFXDLLCALLCONV *winBeep)( void ) = NULL;



static HINSTANCE hDll = NULL;


//
//
//
static BOOL LoadXLIBDLL( char *dllname ) {


	XLIBDLLinfo_t info;
	void (GFXDLLCALLCONV *XLIBDLL_GetInfo)( XLIBDLLinfo_t * ) = NULL;

	if( (hDll = LoadLibraryEx( dllname, NULL, 0L )) == NULL )
		return FALSE;

	if( (XLIBDLL_GetInfo = (void (GFXDLLCALLCONV *)(XLIBDLLinfo_t*))GetProcAddress( hDll, "XLIBDLL_GetInfo" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		return FALSE;
	}

	// input
	info.winShareware = (void *)winShareware;
	info.ProgramDesc = (void *)getProgramDesc();

	XLIBDLL_GetInfo( &info );

	// output
	XLIB_WinMain = (int (GFXDLLCALLCONV *)(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow, ULONG proc))info.XLIB_WinMain;

	GetWinError = (char *(GFXDLLCALLCONV *)(void))info.GetWinError;
	winTemp = (char *(GFXDLLCALLCONV *)(void))info.winTemp;
	GetWinMemInfo = (void (GFXDLLCALLCONV *)(winmeminfo_ptr))info.GetWinMemInfo;

	IsIjl = (BOOL (GFXDLLCALLCONV *)(char *))info.IsIjl;
	LoadIjl = (BOOL (GFXDLLCALLCONV *)(char *, memptr_ptr))info.LoadIjl;
	SaveIjl = (BOOL (GFXDLLCALLCONV *)(char *name, UCHAR *s, UCHAR *p))info.SaveIjl;
	SetRgbIjl = (void (GFXDLLCALLCONV *)(void))info.SetRgbIjl;
	GetRgbIjl = (void (GFXDLLCALLCONV *)(UCHAR *))info.GetRgbIjl;
	GetIjlDim = (BOOL (GFXDLLCALLCONV *)(char*,int*,int*))info.GetIjlDim;

	SetX = (BOOL (GFXDLLCALLCONV *)(int width, int height, int depth, int flag))info.SetX;
	ResetX = (void (GFXDLLCALLCONV *)(void))info.ResetX;

	AVI_Open = (BOOL (GFXDLLCALLCONV *)(char* szFilename))info.AVI_Open;
	AVI_Close = (void (GFXDLLCALLCONV *)(void))info.AVI_Close;
	AVI_DrawFrame = (void (GFXDLLCALLCONV *)(int,int,int,int,int))info.AVI_DrawFrame;
	AVI_GetFrame = (UCHAR* (GFXDLLCALLCONV *)(int,int))info.AVI_GetFrame;

	winCopyTextToClipboard = (int (GFXDLLCALLCONV *)( char *str ))info.winCopyTextToClipboard;
	winPasteTextFromClipboard = (char *(GFXDLLCALLCONV *)( void ))info.winPasteTextFromClipboard;
	winGetSysDirectory = (BOOL (GFXDLLCALLCONV *)( char *szPath ))info.winGetSysDirectory;

	winFindFirstFile = (void *(GFXDLLCALLCONV *)( char *mask, findfile_ptr find ))info.winFindFirstFile;
	winFindNextFile = (BOOL (GFXDLLCALLCONV *)( void *handle, findfile_ptr find ))info.winFindNextFile;
	winFindClose = (BOOL (GFXDLLCALLCONV *)( void *handle ))info.winFindClose;
	winDeleteFile = (BOOL (GFXDLLCALLCONV *)( char *name ))info.winDeleteFile;

	winGetCurrentDirectory = (int (GFXDLLCALLCONV *)( char *dir, int num ))info.winGetCurrentDirectory;
	winSetCurrentDirectory = (BOOL (GFXDLLCALLCONV *)( char *dir ))info.winSetCurrentDirectory;
	winGetFontDir = (BOOL (GFXDLLCALLCONV *)( char *szPath ))info.winGetFontDir;
	winGetProgramFilesDir = (BOOL (GFXDLLCALLCONV *)( char *szPath ))info.winGetProgramFilesDir;
	winGetMyDocumentsDir = (BOOL (GFXDLLCALLCONV *)( char *szPath ))info.winGetMyDocumentsDir;
	winBrowseForFolder = (BOOL (GFXDLLCALLCONV *)( char *szPath ))info.winBrowseForFolder;
	winIsConnected = (BOOL (GFXDLLCALLCONV *)( void ))info.winIsConnected;
	winLoadBitmap = (int (GFXDLLCALLCONV *)( void ))info.winLoadBitmap;
	winPlayWave = (int (GFXDLLCALLCONV *)( char *str ))info.winPlayWave;
	winMagyar = (BOOL (GFXDLLCALLCONV *)( void ))info.winMagyar;
	winReadProfileInt = (int (GFXDLLCALLCONV *)( char *key, int default_value ))info.winReadProfileInt;
	winWriteProfileInt = (void (GFXDLLCALLCONV *)( char *key, int value ))info.winWriteProfileInt;
	winReadProfileFloat = (FLOAT (GFXDLLCALLCONV *)( char *key, FLOAT default_value ))info.winReadProfileFloat;
	winWriteProfileFloat = (void (GFXDLLCALLCONV *)( char *key, FLOAT value ))info.winWriteProfileFloat;
	winReadProfileString = (void (GFXDLLCALLCONV *)( char *key, char *default_string, char *string, int size ))info.winReadProfileString;
	winWriteProfileString = (void (GFXDLLCALLCONV *)( char *key, char *string ))info.winWriteProfileString;
	winDeleteProfileKey = (BOOL (GFXDLLCALLCONV *)( char *key ))info.winDeleteProfileKey;
	GetUptime = (void (GFXDLLCALLCONV *)( void ))info.GetUptime;
	GetCPUSpeed = (int (GFXDLLCALLCONV *)( void ))info.GetCPUSpeed;
	xsystem = (void (GFXDLLCALLCONV *)( char * ))info.xsystem;
	winAlloc = (void *(GFXDLLCALLCONV *)( int amount ))info.winAlloc;
	winFree = (void (GFXDLLCALLCONV *)( void *buf ))info.winFree;
	IsSofticeLoaded = (BOOL (GFXDLLCALLCONV *)( void ))info.IsSofticeLoaded;
	EnumerateProcessorType = (void (GFXDLLCALLCONV *)( void ))info.EnumerateProcessorType;
	CheckBernie = (BOOL (GFXDLLCALLCONV *)( void ))info.CheckBernie;
	winGetMachineName = (const char *(GFXDLLCALLCONV *)( void ))info.winGetMachineName;
	winGetUserName = (const char *(GFXDLLCALLCONV *)( void ))info.winGetUserName;
	winSetCaption = (void (GFXDLLCALLCONV *)( char *str, ... ))info.winSetCaption;
	winLogLevel = (int (GFXDLLCALLCONV *)( void ))info.winLogLevel;
	winGetNextFile = (char *(GFXDLLCALLCONV *)( void ))info.winGetNextFile;
	winGetUrlDocument = (int (GFXDLLCALLCONV *)( char *url, UCHAR **buf ))info.winGetUrlDocument;
	getNetVersion = (BOOL (GFXDLLCALLCONV *)( char *url, int *major, int *minor, char *download ))info.getNetVersion;
	CheckVersion = (BOOL (GFXDLLCALLCONV *)( char *name, char *version ))info.CheckVersion;
	DumpProcessInfo = (void (GFXDLLCALLCONV *)( void ))info.DumpProcessInfo;
	winFindTargetWindow = (ULONG (GFXDLLCALLCONV *)( char *windowName ))info.winFindTargetWindow;
	winNoSplash = (BOOL (GFXDLLCALLCONV *)( void ))info.winNoSplash;
	winRegNoLog = (BOOL (GFXDLLCALLCONV *)( void ))info.winRegNoLog;
	winTimeZone = (int (GFXDLLCALLCONV *)( void ))info.winTimeZone;
	winGetGammaRamp = (BOOL (GFXDLLCALLCONV *)( void ))info.winGetGammaRamp;
	winSetGammaRamp = (BOOL (GFXDLLCALLCONV *)( FLOAT f ))info.winSetGammaRamp;
	winSetWindowTrans = (void (GFXDLLCALLCONV *)( __int64 hwnd, BOOL bTransparent, int nTransparency ))info.winSetWindowTrans;
	winGetMouseScrollLines = (UINT (GFXDLLCALLCONV *)( void ))info.winGetMouseScrollLines;
	winLoadFonts = (BOOL (GFXDLLCALLCONV *)( void ))info.winLoadFonts;
	winRemoveFonts = (void (GFXDLLCALLCONV *)( void ))info.winRemoveFonts;
	winGetMaxWidth = (int (GFXDLLCALLCONV *)( void ))info.winGetMaxWidth;
	winGetMaxHeight = (int (GFXDLLCALLCONV *)( void ))info.winGetMaxHeight;
	winBeep = (void (GFXDLLCALLCONV *)( void ))info.winBeep;

	return TRUE;
}




//
//
//
#ifndef __XLIB_WIN32
int main( int argc, char *argv[], char *envp[] ) {
#else
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow ) {
#endif
	char *dllname = XLIBDLLNAME;

	if( LoadXLIBDLL( dllname ) == FALSE ) {
		char str[MAX_PATH];
		sprintf( str, "Could not load %s library.", dllname );
		MessageBox( NULL, str, "::Fatal Error::", MB_OK );
		return FALSE;
	}

	return XLIB_WinMain( hInstance, hPrevInst, lpCmdLine, nCmdShow, (ULONG)usermain );
}




//////////////////////////////////

#define HOMEPAGE "website.url"		// "http://gamesfrom21.webs.com"


static HWND hwndshareware = NULL;
static int sharewarebutton = 1;
static BOOL sharewareok = FALSE;

#define MAXREGNAME 255

static char registered_name[MAXREGNAME] = "";
static char registered_key[MAXREGNAME] = "";


// These typedefs show the compiler the formats of the functions
typedef bool (__stdcall *CheckCodeFn)(const char *name, const char *code);
typedef bool (__stdcall *VerifyKeyFn)(const char *name, const char *code);
typedef bool (__stdcall *InstallKeyFn)(const char *name, const char *code);
typedef bool (__stdcall *InstallKeyLaterFn)(const char *name, const char *code);
typedef bool (__stdcall *UninstallKeyFn)(void);
typedef bool (__stdcall *SetDefaultKeyFn)(void);
typedef bool (__stdcall *UpdateEnvironmentFn)(void);
typedef bool (__stdcall *IncrementCounterFn)(void);
typedef int  (__stdcall *CopiesRunningFn)(void);
typedef bool (__stdcall *ChangeHardwareLockFn)(void);
typedef DWORD (__stdcall *GetShellProcessIDFn)(void);
typedef bool (__stdcall *FixClockFn)(const char *fixclockkey);

//
//
//
static BOOL InstallRegCode( const char *name, const char *codestring ) {

	BOOL returnvalue = FALSE;

	HINSTANCE libInst=LoadLibrary("ArmAccess.DLL");
	if( !libInst )
		return FALSE;

	CheckCodeFn CheckCode=(CheckCodeFn)GetProcAddress(libInst, "CheckCode");
	if( CheckCode==0 )
		return FALSE;

	returnvalue = CheckCode(name, codestring);

	FreeLibrary( libInst );

	return BOOLEAN(returnvalue);
}


//
//
//
BOOL CALLBACK RegDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch( msg ) {

		case WM_KEYDOWN:

			// xprintf("key: %d\n",wParam);

			if( wParam == VK_ESCAPE ) {

				DestroyWindow( hwnd );

				return TRUE;
			}

			break;

		case WM_HELP:

			// XLIB_Help( hwnd );

			break;

		case WM_INITDIALOG: {

			RECT rc;
			GetWindowRect( (HWND)hwnd, &rc );
			int x = (GetSystemMetrics( SM_CXSCREEN ) - (rc.right - rc.left) ) /2;
			int y = (GetSystemMetrics( SM_CYSCREEN ) - (rc.bottom - rc.top) ) /2;
			MoveWindow( (HWND)hwnd,x,y,(rc.right - rc.left),(rc.bottom - rc.top), TRUE );

			/*
			winReadProfileString( "xlib_regname", "(null)", registered_name, MAXREGNAME );
			winReadProfileString( "xlib_regkey", "(null)", registered_key, MAXREGNAME );
			*/

			registered_name[0] = 0;
			registered_key[0] = 0;

			if( stricmp(registered_name, "(null)" ) )
				Edit_SetText( GetDlgItem( hwnd, IDC_EDIT1 ), registered_name );

			if( stricmp(registered_key, "(null)" ) )
				Edit_SetText( GetDlgItem( hwnd, IDC_EDIT2 ), registered_key );

			return TRUE;
		}

		case WM_COMMAND:

			switch( wParam ) {

				case IDOK: {

					Edit_GetText( GetDlgItem( hwnd, IDC_EDIT1 ), registered_name, MAXREGNAME );
					winWriteProfileString( "xlib_regname", registered_name );

					Edit_GetText( GetDlgItem( hwnd, IDC_EDIT2 ), registered_key, MAXREGNAME );
					winWriteProfileString( "xlib_regkey", registered_key );

					// nem adott meg nevet
					if( !strlen(registered_name) ) {
						MessageBox( hwnd, "Please enter the exact name, that you have\nspecified at the regsitration site.", "Name missing", MB_OK | MB_ICONERROR );
						return TRUE;
					}

					// nem adott meg kulcsot
					if( !strlen(registered_key) ) {
						MessageBox( hwnd, "Please enter the Registration Key!\nYou can use Copy and Paste from the email.", "Registration Key missing", MB_OK | MB_ICONERROR );
						return TRUE;
					}

					/***
					The registration name and code you entered are incorrect.
					Please retype them and try again.

					If you are sure they are correct
					and you still get this message, contact us.


					Thanks to all those who have registered so far.
					Your registration fees have helped speed the
					development of this version. I hope to make
					many further improvements in the future.
					***/

					char *str = "xlib_registered";

					if( InstallRegCode(registered_name,registered_key) ) {
						winWriteProfileInt( str, 1 );
						MessageBox( hwnd, "Congratulation, registration is successful!\nPlease restart the application!", "Thank you for your support!", MB_OK | MB_ICONINFORMATION );
						Quit(NULL);
					}
					else {
						winWriteProfileInt( str, 0 );
						MessageBox( hwnd, "Registration failed! Please use the exact name, that you have\nspecified at the regsitration site.\n\nIf the problem does not solve, please email us at gamesfrom21@gmail.com\nwith subject: Registration failed. Thank you!\n\nWe are sorry for the inconvenience.", "Invalid Registration Key", MB_OK | MB_ICONERROR );
					}


					return TRUE;

					break;
				}

				case IDCANCEL:

					DestroyWindow( hwnd );

					return TRUE;

					break;

			}

	}

	return FALSE;
}







/***

#define SCROLLAMOUNT		-1
#define DISPLAY_SLOW		70
#define DISPLAY_MEDIUM		40
#define DISPLAY_FAST		10
#define DISPLAY_SPEED		DISPLAY_MEDIUM

#define RGB_RED			RGB(255,0,0)
#define RGB_GREEN		RGB(0,255,0)
#define RGB_BLUE		RGB(0,0,255)
#define RGB_WHITE   		RGB(255,255,255)
#define RGB_YELLOW  		RGB(255,255,0)
#define RGB_TURQUOISE 		RGB(0,255,255)
#define RGB_PURPLE  		RGB(255,0,255)
#define RGB_BLACK       	RGB(0,0,0)

#define BACKGROUND_COLOR        RGB_BLACK
#define TOP_LEVEL_TITLE_COLOR	RGB_RED
#define TOP_LEVEL_GROUP_COLOR   RGB_YELLOW
#define GROUP_TITLE_COLOR       RGB_TURQUOISE
#define NORMAL_TEXT_COLOR	RGB_WHITE

// You can set font heights here to suit your taste
#define TOP_LEVEL_TITLE_HEIGHT	21
#define TOP_LEVEL_GROUP_HEIGHT  19
#define GROUP_TITLE_HEIGHT    	17
#define	NORMAL_TEXT_HEIGHT	15



// these define the escape sequences to suffix each array line with. They
// are listed in order of descending font size.
#define TOP_LEVEL_TITLE		'\n'
#define TOP_LEVEL_GROUP         '\r'
#define GROUP_TITLE           	'\t'
#define NORMAL_TEXT		'\f'
#define DISPLAY_BITMAP		'\b'

// Enter the text and bitmap resource string identifiers which are to
// appear in the scrolling list. Append each non-empty string entry with
// an escape character corresponding to the desired font effect. Empty
// strings do not get a font assignment.
// (see #defines above)
//
// Array count. Make sure this keeps up with how many lines there are
// in the array!
#define ARRAYCOUNT		93
char *pArrCredit[] = { 	"MAIN APPLICATION TITLE \n",
						"",
						"Copyright (c) 1998 \f",
						"Company Name \f",
						"All Rights Reserved \f",
						"",
						"BITMAP1\b",    // MYBITMAP is a quoted bitmap resource
						"",
						"",
						"Project Lead \t",
						"",
						"Joe Schmoe \f",
						"",
						"",
						"Technical Lead \t",
						"",
						"John Smith \f",
						"",
						"",
						"Engineering Lead \t",
						"",
						"Stan the Man \f",
						"",
						"",
						"Product Lead \t",
						"",
						"Mary Jones \f",
						"",
						"",
						"Engineering \t",
						"",
						"Hank Smith,  Joe Paterno,  Frank Eiler,  John Madden \f",
						"Harry Houdini,  Mary Marcus, Jane Lane, Sue Shoe,  Joe Gogo \f",
						"John Endling, Bob Herston, Rick Mayer \f",
						"",
						"",
						"BITMAP1\b",  // BARGRAPH1 is a quoted bitmap resource
						"",
						"",
						"QA \t",
						"",
						"Mary Hech,  Sam Bamnm,  Ron Fonn,  Steve Waeve \f",
						"Igor Borisnoff,  FellaB \f",
						"",
						"",
						"Documentation \t",
						"",
						"Marge Farge,  Joanne Hone,  Annette Fune \f",
						"",
						"",
						"Technical Program Office \t",
						"",
						"Burf Murphy, Foll Roller \f",
						"",
						"",
						"",
						"Systems Support \t",
						"",
						"Bendy Land \f",
						"",
						"",
						"Administrative Support \t",
						"",
						"Donna Fonna \f",
						"",
						"",
						"* * * * * * * * * \t",
						"",
						"",
						"BITMAP2\b",
						"",
						"Project Manager \t",
						"",
						"Fred Foghorn \f",
						"",
						"",
						"Engineering \t",
						"",
						"Hank Bank,  Ray Fay,  Bill Sill,  Mark Dark,  Peter Leter \f",
						"Lev Bef \f",
						"",
						"",
						"Quality Assurance \t",
						"",
						"Biff Bin \f",
						"",
						"",
						"",
						"BITMAP3\b",
						"",
						"",
						"",
						""
						};


#define     DISPLAY_TIMER_ID		150		// timer id

RECT    m_ScrollRect,r;		   // rect of Static Text frame
int     nArrIndex,nCounter;		   // work ints
char	m_szWork[1024];			   // holds display line
BOOL        m_bFirstTime;
BOOL        m_bDrawText;
int         nClip;
int         nCurrentFontHeight;

HWND	m_pDisplayFrame;

HBITMAP     m_bmpWork;		           // bitmap holder
HBITMAP 	pBmpOld;                    // other bitmap work members
HBITMAP    m_bmpCurrent;
HBITMAP 	m_hBmpOld;

int 		m_size_x;                     // drawing helpers
int 		m_size_y;                     // drawing helpers
point2_t	m_pt;
HBITMAP 	m_bmpInfo;
HDC 		m_dcMem;
BOOL 		m_bProcessingBitmap;


//
//	 InitDialog
//
//	 	Setup the display rect and start the timer.
//
BOOL ScrollOnInitDialog( HWND hwnd ) {

	BOOL bRet;
    	UINT nRet;

	nCurrentFontHeight = NORMAL_TEXT_HEIGHT;

	HDC dc = GetDC(hwnd);
	m_dcMem = CreateCompatibleDC(dc);

	m_bProcessingBitmap = FALSE;

	nArrIndex=0;
	nCounter=1;
	nClip=0;
	m_bFirstTime=TRUE;
	m_bDrawText=FALSE;
	m_hBmpOld = 0;

	m_pDisplayFrame= (HWND)GetDlgItem(hwnd, IDC_DISPLAY_STATIC);

	// If you assert here, you did not assign your static display control
	// the IDC_ value that was used in the GetDlgItem(...). This is the
	// control that will display the credits.

	// ASSERT(m_pDisplayFrame);

	m_pDisplayFrame->GetClientRect(&m_ScrollRect);


	nRet = SetTimer(DISPLAY_TIMER_ID,DISPLAY_SPEED,NULL);
	_ASSERTE(nRet != 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//
//	 OnTimer
//
//	 	On each of the display timers, scroll the window 1 unit. Each 20
//      units, fetch the next array element and load into work string. Call
//      Invalidate and UpdateWindow to invoke the OnPaint which will paint
//      the contents of the newly updated work string.
//
void ScrollOnTimer( UINT nIDEvent ) {

	if (nIDEvent != DISPLAY_TIMER_ID)
		{
		// CDialog::OnTimer(nIDEvent);
		return;
		}

	if (!m_bProcessingBitmap)
	if (nCounter++ % nCurrentFontHeight == 0)	 // every x timer events, show new line
		{
		nCounter=1;
		m_szWork = pArrCredit[nArrIndex++];

		if (nArrIndex > ARRAYCOUNT-1)
			nArrIndex=0;
		nClip = 0;
		m_bDrawText=TRUE;
		}

	ScrollWindow(0,SCROLLAMOUNT,&m_ScrollRect,&m_ScrollRect);
	nClip = nClip + abs(SCROLLAMOUNT);

	RECT r;
	HWND* pWnd = GetDlgItem(IDC_DISPLAY_STATIC);
	ASSERT_VALID(pWnd);
	GetClientRect(pWnd, &r);
	ClientToScreen(pWnd, r);
	ScreenToClient(&r);
	InvalidateRect(r,FALSE); // FALSE does not erase background

	// CDialog::OnTimer(nIDEvent);

	return;
}




//
//	 OnPaint
//
//	 	Send the newly updated work string to the display rect.
//
void ScrollPaint( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	HDC dc; // device context for painting

	PAINTSTRUCT ps;
	HDC* pDc = BeginPaint(hwnd, &ps);

	SetBkMode(pDc, TRANSPARENT);

	//	FONT SELECTION
    	HFONT m_fntArial;
	HFONT pOldFont;
	BOOL bSuccess;

	BOOL bUnderline;
	BOOL bItalic;


	if (!m_szWork.IsEmpty())
	switch (m_szWork[m_szWork.GetLength()-1] )
		{
		case NORMAL_TEXT:
		default:
			bItalic = FALSE;
			bUnderline = FALSE;
			nCurrentFontHeight = NORMAL_TEXT_HEIGHT;
   			bSuccess = m_fntArial.CreateFont(NORMAL_TEXT_HEIGHT, 0, 0, 0,
   								FW_THIN, bItalic, bUnderline, 0,
   								ANSI_CHARSET,
                               	OUT_DEFAULT_PRECIS,
                               	CLIP_DEFAULT_PRECIS,
                               	PROOF_QUALITY,
                               	VARIABLE_PITCH | 0x04 | FF_DONTCARE,
                               	(LPSTR)"Arial");
			pDc->SetTextColor(NORMAL_TEXT_COLOR);
			pOldFont  = pDc->SelectObject(&m_fntArial);
			break;

		case TOP_LEVEL_GROUP:
			bItalic = FALSE;
			bUnderline = FALSE;
			nCurrentFontHeight = TOP_LEVEL_GROUP_HEIGHT;
   			bSuccess = m_fntArial.CreateFont(TOP_LEVEL_GROUP_HEIGHT, 0, 0, 0,
   								FW_BOLD, bItalic, bUnderline, 0,
   								ANSI_CHARSET,
                               	OUT_DEFAULT_PRECIS,
                               	CLIP_DEFAULT_PRECIS,
                               	PROOF_QUALITY,
                               	VARIABLE_PITCH | 0x04 | FF_DONTCARE,
                               	(LPSTR)"Arial");
			pDc->SetTextColor(TOP_LEVEL_GROUP_COLOR);
			pOldFont  = pDc->SelectObject(&m_fntArial);
			break;



		case GROUP_TITLE:
			bItalic = FALSE;
			bUnderline = FALSE;
			nCurrentFontHeight = GROUP_TITLE_HEIGHT;
   			bSuccess = m_fntArial.CreateFont(GROUP_TITLE_HEIGHT, 0, 0, 0,
   								FW_BOLD, bItalic, bUnderline, 0,
   								ANSI_CHARSET,
                               	OUT_DEFAULT_PRECIS,
                               	CLIP_DEFAULT_PRECIS,
                               	PROOF_QUALITY,
                               	VARIABLE_PITCH | 0x04 | FF_DONTCARE,
                               	(LPSTR)"Arial");
			pDc->SetTextColor(GROUP_TITLE_COLOR);
			pOldFont  = pDc->SelectObject(&m_fntArial);
			break;


		case TOP_LEVEL_TITLE:
			bItalic = FALSE;
			bUnderline = TRUE;
			nCurrentFontHeight = TOP_LEVEL_TITLE_HEIGHT;
			bSuccess = m_fntArial.CreateFont(TOP_LEVEL_TITLE_HEIGHT, 0, 0, 0,
								FW_BOLD, bItalic, bUnderline, 0,
								ANSI_CHARSET,
	                           	OUT_DEFAULT_PRECIS,
	                           	CLIP_DEFAULT_PRECIS,
	                           	PROOF_QUALITY,
	                           	VARIABLE_PITCH | 0x04 | FF_DONTCARE,
	                           	(LPSTR)"Arial");
			pDc->SetTextColor(TOP_LEVEL_TITLE_COLOR);
			pOldFont  = pDc->SelectObject(&m_fntArial);
			break;

		case DISPLAY_BITMAP:
			if (!m_bProcessingBitmap)
				{
				CString szBitmap = m_szWork.Left(m_szWork.GetLength()-1);
	   			if (!m_bmpWork.LoadBitmap((const char *)szBitmap))
					{
					CString str;
					str.Format("Could not find bitmap resource \"%s\". "
                               "Be sure to assign the bitmap a QUOTED resource name", szBitmap);
					KillTimer(DISPLAY_TIMER_ID);
					MessageBox(str);
					return;
					}
				m_bmpCurrent = &m_bmpWork;
	   			m_bmpCurrent->GetObject(sizeof(BITMAP), &m_bmpInfo);

				m_size.cx = m_bmpInfo.bmWidth;	// width  of dest rect
				RECT workRect;
				m_pDisplayFrame->GetClientRect(&workRect);
				m_pDisplayFrame->ClientToScreen(&workRect);
				ScreenToClient(&workRect);
				// upper left point of dest
				m_pt.x = (workRect.right -
							((workRect.right-workRect.left)/2) - (m_bmpInfo.bmWidth/2));
				m_pt.y = workRect.bottom;


				pBmpOld = m_dcMem.SelectObject(m_bmpCurrent);
				if (m_hBmpOld == 0)
					m_hBmpOld = (HBITMAP) pBmpOld->GetSafeHandle();
				m_bProcessingBitmap = TRUE;
				}
			break;

		}




	CBrush bBrush(BLACK);
	CBrush* pOldBrush;
	pOldBrush  = pDc->SelectObject(&bBrush);
	// Only fill rect comprised of gap left by bottom of scrolling window
	r=m_ScrollRect;
	r.top = r.bottom-abs(SCROLLAMOUNT);
	pDc->DPtoLP(&r);

	if (m_bFirstTime)
		{
		m_bFirstTime=FALSE;
		pDc->FillRect(&m_ScrollRect,&bBrush);
		}
	else
		pDc->FillRect(&r,&bBrush);

	r=m_ScrollRect;
	r.top = r.bottom-nClip;


	if (!m_bProcessingBitmap)
		{
		int x = pDc->DrawText((const char *)m_szWork,m_szWork.GetLength()-1,&r,DT_TOP|DT_CENTER|
					DT_NOPREFIX | DT_SINGLELINE);
		m_bDrawText=FALSE;
		}
	else
		{
    	dc.StretchBlt( m_pt.x, m_pt.y-nClip, m_size.cx, nClip,
                   		&m_dcMem, 0, 0, m_bmpInfo.bmWidth-1, nClip,
                   		SRCCOPY );
		if (nClip > m_bmpInfo.bmHeight)
			{
			m_bmpWork.DeleteObject();
			m_bProcessingBitmap = FALSE;
			nClip=0;
			m_szWork.Empty();
			nCounter=1;
			}
		pDc->SelectObject(pOldBrush);
		bBrush.DeleteObject();
		m_pDisplayFrame->EndPaint(&ps);
		return;
		}


	pDc->SelectObject(pOldBrush);
	bBrush.DeleteObject();

	if (!m_szWork.IsEmpty())
		{
		pDc->SelectObject(pOldFont);
		m_fntArial.DeleteObject();
		}

	m_pDisplayFrame->EndPaint(&ps);

	// Do not call CDialog::OnPaint() for painting messages
}

***/




static int kocka_tic = 0;
#define KOCKA_DELAY (TICKBASE*30)	// a HTML-nek ennyi ideje van megjelenítõdni
					// utána megy rá a kocka effekt
static int kocka_inited = FALSE;	// kockaeffekt meg volt-e már hivva

//
// a nag.html be van-e töltve?
//
static BOOL IsReady( void ) {

	if( hwndshareware == NULL )
		return FALSE;

	// ez még sajnos nem jó
	if( HTML_IsReady( GetDlgItem( hwndshareware, IDC_EDIT1 ) ) )
		return TRUE;

	return FALSE;
}


//
//
//
static void RandomWrong( void ) {

	char *str[] = {
		"The start button dude.",
		"The other one.",
		"Do you want to play or not?",
		"Almost there.",
		"Try another one.",
		"Waiting.",
		"Choices, choices.",
		"Buttons, buttons everywhere.",
	};

	int j;
	for( int i=0; i<10; i++ )
		j = ( rand() % dimof(str) );

	SpeakText( str[ j ] );

	return;
}


//
// IDD_NAG dialog
//
BOOL CALLBACK SharewareDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch( msg ) {

		case WM_PAINT:

			// ScrollPaint( hwnd, msg, wParam, lParam );

			break;

		case WM_KEYDOWN:

			// xprintf("key: %d\n",wParam);

			if( wParam == VK_ESCAPE ) {

				DestroyWindow( hwnd );
				hwndshareware = NULL;
				sharewareok = FALSE;

				return TRUE;
			}

			break;


		case WM_COMMAND:

			switch( wParam ) {

				case IDC_BUTTON1:

					if( sharewarebutton == 1 ) {
						DestroyWindow( hwnd );
						hwndshareware = NULL;
						sharewareok = TRUE;
					}
					else
						RandomWrong();

					return TRUE;

					break;

				case IDC_BUTTON2:

					if( sharewarebutton == 2 ) {
						DestroyWindow( hwnd );
						hwndshareware = NULL;
						sharewareok = TRUE;
					}
					else
						RandomWrong();

					return TRUE;

					break;

				case IDC_BUTTON3:

					if( sharewarebutton == 3 ) {
						DestroyWindow( hwnd );
						hwndshareware = NULL;
						sharewareok = TRUE;
					}
					else
						RandomWrong();

					return TRUE;

					break;

				case IDC_BUTTON4:

					DestroyWindow( hwnd );
					hwndshareware = NULL;
					sharewareok = FALSE;

					char str[512];
					// sprintf( str, "%s?referrer=%s", HOMEPAGE, winSetProgramDesc(NULL) );
					strcpy( str, HOMEPAGE );
					winGotoUrl(str);

					return TRUE;

					break;


				case IDC_BUTTON5:

					DestroyWindow( hwnd );
					hwndshareware = NULL;
					sharewareok = FALSE;

					xsystem("order.url");

					return TRUE;

					break;


				case IDC_BUTTON6:

					ShowWindow( hwnd, SW_HIDE );

					DialogBox( (HINSTANCE)GetHinstance(), MAKEINTRESOURCE(IDD_KEYIN), GetTopWindow(NULL), (DLGPROC)RegDlgProc);

					ShowWindow( hwnd, SW_NORMAL );

					break;

				// case IDOK:
				case IDCANCEL:

					DestroyWindow( hwnd );
					hwndshareware = NULL;
					sharewareok = FALSE;

					return TRUE;
			}

			break;

		case WM_HELP:

			// myHelp( hwnd );

			break;


		case WM_SIZE:

			// Resize the browser object to fit the window
			HTML_ResizeBrowser( GetDlgItem( hwnd, IDC_EDIT1 ), LOWORD(lParam), HIWORD(lParam));

			// return 0;

			break;

		case WM_INITDIALOG: {

			RECT rc;
			GetWindowRect( (HWND)hwnd, &rc );
			int x = (GetSystemMetrics( SM_CXSCREEN ) - (rc.right - rc.left) ) /2;
			int y = (GetSystemMetrics( SM_CYSCREEN ) - (rc.bottom - rc.top) ) /2;
			MoveWindow( (HWND)hwnd,x,y,(rc.right - rc.left),(rc.bottom - rc.top), TRUE );

			HWND myhwnd = GetDlgItem( hwnd, IDC_EDIT1 );

			// char *page = "http://google.hu";
			// char *page = "http://index.hu";
			// char *page = "Help-particle-api.htm";
			char page[XMAX_PATH];
			sprintf(page,"%s%cnag.htm",GetExeDir(),PATHDELIM);

			if( !HTML_EmbedBrowserObject(myhwnd) ) {
				char url[XMAX_PATH];
				Path2URL( page, url );
				HTML_DisplayPage( myhwnd, url );
			}
			else
				xprintf( "SharewareDlgProc: can't embed HTML page.\n" );

			// ScrollOnInitDialog( hwnd );

			// Showtime!  Invalidate the entire window, force an update, and show the window.

			ShowWindow(hwnd, SW_SHOW);
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);

			break;
		}

		case WM_DESTROY:

			HTML_UnEmbedBrowserObject( GetDlgItem( hwnd, IDC_EDIT1 ) );

			hwndshareware = NULL;

			break;

		// default: return DefWindowProc( hwnd, msg, wParam, lParam);
	}


	// a KOCKA_DELAY után mindenképpen lesz effekt ha kész ha nem
	if( (kocka_tic + KOCKA_DELAY) < GetTic() ) {

		if( kocka_inited == FALSE ) {
			kocka_inited = TRUE;
			msg = WM_INITDIALOG;
			if( KockaEffekt( (ULONG)hwnd, (ULONG)msg, (ULONG)wParam, (ULONG)lParam ) == TRUE )
				return TRUE;
		}

		// egyébb üzik
		if( KockaEffekt( (ULONG)hwnd, (ULONG)msg, (ULONG)wParam, (ULONG)lParam ) == TRUE )
			return TRUE;
	}

	return FALSE;
}



//
//
//
char *FixNewLines( char *lpcszText ) {

	char *lpszScratch;
	char *lpszNewLine;

	if( !lpcszText )
		return NULL;

	// allocate a buffer twice as big in case we need to
	// add any \r's to solo \n's

	if( (lpszScratch = (char*)malloc( (strlen(lpcszText) * 2) + 1 )) == NULL )
		return NULL;

	/* make a copy of the passed text */
	strcpy(lpszScratch, lpcszText);

	/* replace \n\r with \r\n */
	lpszNewLine = strstr(lpszScratch, "\n\r");
	while( lpszNewLine ) {

		*lpszNewLine = '\r';
		*(lpszNewLine + 1) = '\n';

		// skip our changes
		lpszNewLine += 2;

		lpszNewLine = strstr(lpszNewLine, "\n\r");
	}

	/* replace solo \n's with \r\n */
	lpszNewLine = lpszScratch;
	while( *lpszNewLine ) {

		if( *lpszNewLine == '\n' ) {
			/* handle the case of \n being the 1st char */
			if( (lpszNewLine == lpszScratch) ) {
				MoveMemory(lpszScratch + 1, lpszScratch, strlen(lpszScratch) + 1);
				*lpszScratch = '\r';
				lpszNewLine++;
			}
			else
			if( *(lpszNewLine - 1) != '\r' ) {
				MoveMemory(lpszNewLine + 1, lpszNewLine, strlen(lpszNewLine) + 1);
				*lpszNewLine = '\r';
				lpszNewLine++;
			}
		}

		lpszNewLine++;
	}

	return lpszScratch;
}



//
// 444-memory-333
//
BOOL checkShareware( void ) {

	int result = FALSE;

	// Message("Armadillo goes here");
	result = TRUE;

	return result;
}


//
// TODO: amradillot le kell cserélni
//
BOOL winShareware( BOOL force_dlg ) {

	char string[512];
	int buttid = IDC_BUTTON1;
	FILE *f = NULL;

	GetEnvironmentVariable("USERNAME", registered_name, MAXREGNAME);
	GetEnvironmentVariable("USERKEY", registered_key, MAXREGNAME);

	// Message( "username: \"%s\"\nuserkey: \"%s\"", registered_name, registered_key );

	if( force_dlg == FALSE )
		if( checkShareware() == TRUE )
			return TRUE;

	registered_name[0] = 0;
	registered_key[0] = 0;

	sharewareok = FALSE;

	winHideLog();

	sharewarebutton = (rand()%3) + 1;
	CLAMPMINMAX( sharewarebutton, 1, 3 );

	SetKockaEffektParam( sharewarebutton );
	kocka_inited = FALSE;
	kocka_tic = GetTic();

	// TODO: sharewaret game közben is meg lehessen hívni
	HWND hwnd = (HWND)GetHwnd();
	// if( hwnd == NULL )
		hwnd = GetDesktopWindow();

	if( (hwndshareware = CreateDialog( (HINSTANCE)GetHinstance(), MAKEINTRESOURCE(IDD_NAG), hwnd, (DLGPROC)SharewareDlgProc)) == NULL ) {
		Quit("winShareware: CreateDialog failed.");
		return FALSE;
	}

	ShowWindow( hwndshareware, SW_NORMAL );

	// And force the window to get attention
	BringWindowToTop(hwndshareware);
	SetForegroundWindow(hwndshareware);
	SetActiveWindow(hwndshareware);


/***
	ptr = (char *)malloc( 1024 );
	// sprintf( ptr, "Thank you very much for your interest!\nThis program is shareware. We encourage you to register it online or via mail.\nFor more information goto %s.\nUse the buttons below if you have online connection to the Internet.\n\nCopyright (C) 2003 BKGames.\nAll Rights Reserved Worldwide.\nmailto:support@bkgames.com\n\nGood Luck.\n\nNow press the PLAY button!", HOMEPAGE );
	strcpy( ptr, nag_str );
#define INFOFILE "nag.txt"

	if( (f = fopen(INFOFILE, "rt")) != NULL ) {

		int len = filelength( fileno(f) );

		if( (ptr = (char *)realloc( ptr, len+1 ) ) != NULL ) {

			memset( ptr, 0L, len+1 );

			fread( ptr, len, 1, f );
			ptr[len] = 0;
		}
		else {
			sprintf( ptr, "Could not find \"%s\" file!\nNot enough memory!", INFOFILE);
			SetWindowFont( GetDlgItem( hwndshareware, IDC_EDIT1 ),GetStockFont(OEM_FIXED_FONT), TRUE);
		}

		fclose( f );
	}
	else
		SetWindowFont( GetDlgItem( hwndshareware, IDC_EDIT1 ),GetStockFont(OEM_FIXED_FONT), TRUE);
	text = FixNewLines( ptr );
	free( ptr );
	ptr = text;

	Edit_SetText( GetDlgItem( hwndshareware, IDC_EDIT1 ), ptr );

	free(ptr);
***/

	// sprintf( string, "Press %d button to play.", sharewarebutton );
	// Static_SetText( GetDlgItem( hwndshareware, IDC_STATIC2 ), string );

#define PLAYSTR "START!"

	switch( sharewarebutton ) {
		default:
		case 1: buttid = IDC_BUTTON1; break;
		case 2: buttid = IDC_BUTTON2; break;
		case 3: buttid = IDC_BUTTON3; break;
	}

	Button_SetText( GetDlgItem( hwndshareware, buttid ), PLAYSTR );
	SetWindowFont( GetDlgItem( hwndshareware, buttid ),GetStockFont(OEM_FIXED_FONT), TRUE);

	sprintf( string, "&Visit %s", "Gamesfrom21.webs.com" );
	Button_SetText( GetDlgItem( hwndshareware, IDC_BUTTON4 ), string );

	SpeakText( "Hello, press start button please." );

	while( hwndshareware != NULL ) {

		// if( IsReady() ) kocka_tic = -KOCKA_DELAY;

		// force the FIRST message
		if( (kocka_inited == FALSE) && ((kocka_tic + KOCKA_DELAY) < GetTic()) )
			PostMessage( hwndshareware, WM_USER + 2, 0,0 );


		// animálja a kockát
		KockaEffekt( (ULONG)hwndshareware, WM_APP, 0L, 0L );

		MSG msg;

		if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
			if( !IsDialogMessage( hwndshareware, &msg ) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Sleep( 1 );
	}

	if( hwndshareware ) {
		DestroyWindow( hwndshareware );
		hwndshareware = NULL;
	}

	kocka_inited = FALSE;

	winShowLog();

	return sharewareok;
}






// cheat
// int usermain( int argc, char *argv[], char *envp[] ) { return 0; }
// char *getProgramDesc( void ) { return NULL; }

