
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <shlobj.h>

#include <xlib.h>
#include <xinner.h>

#include <xrcs.h>
#include "resource.h"


#include "build.h"

RCSID( "$Id: Xdllmain.cpp,v 1.2 2003/09/22 13:59:57 bernie Exp $" )


#define XDLL_VERSION "1.0"

#pragma comment( exestr , "Xlib for Windoze v" XDLL_VERSION " (Build: " BUILD_NUM_STR ") (" __DATE__ " " __TIME__ ") (c) Bernie Kirschner (mailto:gamesfrom21@gmail.com)")
#pragma message( "Compiling " __FILE__ )
#pragma message( "Last modified on " __TIMESTAMP__ )


#define __XLIB_WIN32
#define __XLIB_MUTEX
// #define __XLIB_EXCEPTION



//
//
//
char *getXlibVersion( void ) {

	static char xlib_version[256];

#ifdef _DEBUG
	char *s = "(Debug version)";
#else
	char *s = "(Release version)";
#endif

	sprintf(xlib_version, "Xlib v%s for Windows (Build: %s) (%s %s) %s",XDLL_VERSION, BUILD_NUM_STR, __DATE__ , __TIME__, s );

	return xlib_version;
}


XLIBDEF HINSTANCE hInst;
XLIBDEF HWND hwnd;


//
// Converts lpCmdLine to WinMain into argc, argv
//
void CommandLineToArgv( LPSTR lpCmdLine ) {

	char *p,*b,*pEnd;

	char realName[XMAX_PATH*2] = "";

	GetModuleFileName( hInst, realName, XMAX_PATH*2 );

	char dir[XMAX_PATH*2];

	strcpy( dir, realName );

	if( (p=strrchr(dir,'\\')) != NULL ) {
		*(++p) = 0;
		AddSearchDir( dir );
	}

	// ./Data
	strcat( dir, "\\Data\\" );
	AddSearchDir( dir );

	if( (myargv[0] = (char *)malloc( strlen(realName)+1 )) != NULL ) {
		strcpy( myargv[0], realName );
		myargc = 1;
	}
	else
		myargc = 0;

	if( lpCmdLine == NULL )
		return;

	if( (fullcommandline = (char *)malloc( strlen(lpCmdLine)+1 )) == NULL )
		return;

	strcpy( fullcommandline, lpCmdLine );

	char *str;
	if( (str = (char *)malloc( strlen(lpCmdLine)+1 )) == NULL )
		return;

	strcpy( str, lpCmdLine );

	p = str;

	pEnd = p + strlen(str);

	while( 1 ) {

		while( *p == ' ' ) p++;
		if( p >= pEnd ) break;

		b = p;

		if( *p == '\"' ) {
			++p;
			b = p;
			/* keresi a string végét */
			while( *p && (*p != '\"') ) p++;

			if( *p == '\"' ) *p++ = 0;
		}
		else {
			/* skip till there's a 0 or a white space */
			while( *p && (*p != ' ') ) p++;

			if( *p == ' ' ) *p++ = 0;
		}

		// ha nem egy üres string
		if( (p-b)>1 && (myargv[myargc] = (char *)malloc( p-b+1 )) != NULL )
			strcpy( myargv[myargc++], b );

		if( myargc >= MAX_ARGC )
			break;
	}

	free(str);

	return;
}



//
// az .EXE directorija
//
char *GetExeDir( void ) {

	char *p;
	char dir[PATH_MAX];
	char realName[PATH_MAX] = "";
	static char dirname[PATH_MAX] = "";

	GetModuleFileName( hInst, realName, dimof(realName) );

	strncpy( dir, realName, sizeof(dir) );

	if( (p=strrchr(dir,PATHDELIM)) != NULL ) {
		*p = 0;
		// AddSearchDir( dir );
		strncpy( dirname, dir, dimof(dirname) );
	}
	else
		strncpy( dirname, ".", dimof(dirname) );

	return dirname;
}




//
// az .EXE directorija
//
char *GetExeName( void ) {

	char *p;
	static char realName[XMAX_PATH] = "";

	GetModuleFileName( hInst, realName, dimof(realName) );

	if( (p=strrchr(realName,'\\')) != NULL )
		++p;
	else
	if( (p=strrchr(realName,'/')) != NULL )
		++p;
	else
		p = realName;

	return p;
}


//
//
//
char *GetDataDir( void ) {

	char *p;
	char dir[PATH_MAX];
	char realName[PATH_MAX] = "";
	static char dirname[PATH_MAX] = "";

	GetModuleFileName( hInst, realName, dimof(realName) );

	strncpy( dir, realName, sizeof(dir) );

	if( (p=strrchr(dir,PATHDELIM)) != NULL ) {
		*p = 0;
		// AddSearchDir( dir );
		strncpy( dirname, dir, dimof(dirname) );
	}
	else
		strncpy( dirname, ".", dimof(dirname) );

	strcat( dirname, PATHDELIMSTR );
	strcat( dirname, "Data" );

	return dirname;
}






#ifdef __XLIB_EXCEPTION

static BOOL lbExceptionHandler = TRUE;

//
//
//
static int EvaluateException( ULONG exceptnum ) {

	if( (exceptnum == STATUS_INTEGER_OVERFLOW) ||
	    (exceptnum == STATUS_FLOAT_UNDERFLOW) ||
	    (exceptnum == STATUS_FLOAT_OVERFLOW) )
		return EXCEPTION_CONTINUE_SEARCH;

	if( lbExceptionHandler ) {

		// exception handler is active,
		// try to shut everything down and let the error get caught

		return EXCEPTION_EXECUTE_HANDLER;
	}

	// no exception handler,
	// let it slide through (if the debugger is active,
	// it should catch it)

	return EXCEPTION_CONTINUE_SEARCH;
}




//
//
//
static void ExceptionHandler( ULONG e ) {

	switch( e ) {
		case EXCEPTION_ACCESS_VIOLATION:	Quit("Exception: Access Violation."); break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	Quit("Exception: Array Bounds Exceeded."); break;
		case EXCEPTION_BREAKPOINT:		Quit("Exception: Breakpoint."); break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:	Quit("Exception: Datatype Misalignment."); break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:	Quit("Exception: Float Denormal Operand."); break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:	Quit("Exception: Float Divide By Zero."); break;
		case EXCEPTION_FLT_INEXACT_RESULT:	Quit("Exception: Float Inexact Result."); break;
		case EXCEPTION_FLT_INVALID_OPERATION:	Quit("Exception: Float Invalid Operation."); break;
		case EXCEPTION_FLT_OVERFLOW:		Quit("Exception: Float Overflow."); break;
		case EXCEPTION_FLT_STACK_CHECK: 	Quit("Exception: Float Stack Check."); break;
		case EXCEPTION_FLT_UNDERFLOW:		Quit("Exception: Float Underflow."); break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:	Quit("Exception: Illegal Instruction."); break;
		case EXCEPTION_IN_PAGE_ERROR:		Quit("Exception: In-page Error."); break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:	Quit("Exception: Integer Divide By Zero."); break;
		case EXCEPTION_INT_OVERFLOW:		Quit("Exception: Integer Overflow."); break;
		case EXCEPTION_INVALID_DISPOSITION:	Quit("Exception: Invalid Disposition."); break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:Quit("Exception: Noncontinuable Exception."); break;
		case EXCEPTION_PRIV_INSTRUCTION:	Quit("Exception: Private Instruction."); break;
		case EXCEPTION_SINGLE_STEP:		Quit("Exception: Single-step."); break;
		case EXCEPTION_STACK_OVERFLOW:		Quit("Exception: Stack Overflow."); break;
		default:				Quit("Exception: Unknown: %x.", e); break;
	}

	return;
}
#endif





#ifdef __XLIB_MUTEX


static HANDLE mutex = NULL;
static HANDLE spec_mutex = NULL;

//
//
//
void freemutex( void ) {

	if( mutex ) ReleaseMutex( mutex );
	if( mutex ) CloseHandle( mutex );
	mutex = NULL;

	if( spec_mutex ) ReleaseMutex( spec_mutex );
	if( spec_mutex ) CloseHandle( spec_mutex );
	spec_mutex = NULL;

	return;
}



//
//
//
BOOL CheckMutex( char *lpCmdLine ) {

	mutex = CreateMutex( NULL, TRUE, XLIB_winSetProgramDesc(NULL) );

	if( GetLastError() == ERROR_ALREADY_EXISTS ) {

		HWND hwnd;

		if( (hwnd = FindWindow( XLIBWNDCLASSNAME, NULL )) != NULL ) {

			ShowWindow(hwnd,SW_RESTORE);
			ShowWindow( hwnd, SW_SHOW );
			SetForegroundWindow( hwnd );

			// szimulálja a drag and drop-ot a lpCmdLine-nal

			if( lpCmdLine == NULL || !strlen(lpCmdLine) )
				return FALSE;

			// MessageBox( hwnd , lpCmdLine, "Az ûj:", MB_OK );

			HGLOBAL hGlobal;
			DROPFILES *pDrop;

			hGlobal = GlobalAlloc( GHND, sizeof(DROPFILES) + strlen(lpCmdLine) + 1 );

			if( hGlobal == NULL )
				return FALSE;

			pDrop = (DROPFILES *)GlobalLock( hGlobal );

			if( pDrop == NULL) {
				GlobalFree( hGlobal );
				return FALSE;
			}

			pDrop->pFiles = sizeof(DROPFILES);
			pDrop->pt.x = 0;
			pDrop->pt.y = 0;
			pDrop->fNC  = 0;
			pDrop->fWide = 0;

			char *p = (char *)pDrop + sizeof(DROPFILES);

			strcpy( p, lpCmdLine );
			p[strlen(lpCmdLine)+0] = 0;
			p[strlen(lpCmdLine)+1] = 0;

			GlobalUnlock( hGlobal );

			if( !PostMessage( hwnd, WM_DROPFILES, (WPARAM)hGlobal, 0L ) )
				GlobalFree( hGlobal );
		}
		else
		if( (hwnd = FindWindow( NULL, "Setup" )) != NULL ) {
			ShowWindow(hwnd,SW_RESTORE);
			ShowWindow( hwnd, SW_SHOW );
			SetForegroundWindow( hwnd );
		}
		return FALSE;
	}

	atexit( freemutex );

	return TRUE;
}




//
//
//
BOOL CheckCreateMutex( char *name, char *filename ) {

	if( name == NULL )
		return FALSE;

	// elengedi a régit
	if( mutex ) ReleaseMutex( mutex );
	if( mutex ) CloseHandle( mutex );
	mutex = NULL;

	spec_mutex = CreateMutex( NULL, TRUE, name );

	if( GetLastError() == ERROR_ALREADY_EXISTS ) {

		HWND hwnd;

		if( (hwnd = FindWindow( XLIBWNDCLASSNAME, NULL )) != NULL ) {

			ShowWindow(hwnd,SW_RESTORE);
			ShowWindow( hwnd, SW_SHOW );
			SetForegroundWindow( hwnd );

			// szimulálja a drag and drop-ot a lpCmdLine-nal

			if( filename == NULL || !strlen(filename) )
				return FALSE;

			// MessageBox( hwnd , lpCmdLine, "Az ûj:", MB_OK );

			HGLOBAL hGlobal;
			DROPFILES *pDrop;

			hGlobal = GlobalAlloc( GHND, sizeof(DROPFILES) + strlen(filename) + 1 );

			if( hGlobal == NULL )
				return FALSE;

			pDrop = (DROPFILES *)GlobalLock( hGlobal );

			if( pDrop == NULL) {
				GlobalFree( hGlobal );
				return FALSE;
			}

			pDrop->pFiles = sizeof(DROPFILES);
			pDrop->pt.x = 0;
			pDrop->pt.y = 0;
			pDrop->fNC  = 0;
			pDrop->fWide = 0;

			char *p = (char *)pDrop + sizeof(DROPFILES);

			strcpy( p, filename );
			p[strlen(filename)+0] = 0;
			p[strlen(filename)+1] = 0;

			GlobalUnlock( hGlobal );

			if( !PostMessage( hwnd, WM_DROPFILES, (WPARAM)hGlobal, 0L ) )
				GlobalFree( hGlobal );
		}
		else
		if( (hwnd = FindWindow( NULL, "Setup" )) != NULL ) {
			ShowWindow(hwnd,SW_RESTORE);
			ShowWindow( hwnd, SW_SHOW );
			SetForegroundWindow( hwnd );
		}

		return TRUE;
	}

	atexit( freemutex );

	return FALSE;
}




#endif





//
//
//
static int GFXDLLCALLCONV XLIB_WinMain( HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow, ULONG proc_usermain ) {

	int result = 0;
	char *str;
	int (GFXDLLCALLCONV *usermain)( int argc, char *argv[], char *envp[] ) = NULL;

	winShowSplash();

#ifdef __XLIB_MUTEX

	winSetSplashText( "Mutex..." );

	if( CheckMutex( lpCmdLine ) == FALSE )
		return 0;

#endif

	/* Start up DDHELP.EXE before opening any files, so DDHELP doesn't
	   keep them open.  This is a hack.. hopefully it will be fixed
	   someday.  DDHELP.EXE starts up the first time DDRAW.DLL is loaded.
	 */
	if( 1 ) {
		winSetSplashText( "Ddhelp..." );
		HINSTANCE handle = LoadLibrary(TEXT("DDRAW.DLL"));
		if( handle != NULL )
			FreeLibrary(handle);
	}

	winSetSplashText( "Memory manager..." );

	InitMemory();

	winSetSplashText( "File shell..." );

	InitFfile();

#ifndef __XLIB_WIN32
	SetConsoleTitle( szAppName );
	myargc = argc;
	myargv = argv;
	myenvp = envp;
	hInst = GetModuleHandle( NULL );
#else
	winSetSplashText( "Cmdline..." );
	hInst = hInstance;
	CommandLineToArgv( lpCmdLine );
	myenvp = NULL;
#endif

	// winHideSplash();

	//hwnd = GetTopWindow( NULL );

	winSetSplashText( "Background window..." );
	winShowBackWindow();

	winSetSplashText( "Log window..." );
	InitLogWindow();

	winSetSplashText( "3D Cosnsole..." );
	consoleInit();

	// AfxEnableControlContainer();

	str = (char *)malloc( 1024 );
	if( str )
		LoadString( hInst, 102, str, 256 );

	xprintf( "startup: %s\n", getXlibVersion());

	if( str ) {
		LoadString( hInst, 103, str, 256 );
		xprintf("log: %s.\n",str);

		free(str);
	}

	winSetSplashText( "Locale settings..." );
	setlocale(LC_NUMERIC, "C");

	/***
	DWORD dwVersion;
	char  szVersion[40];

	dwVersion = GetVersion();

	if( dwVersion < 0x80000000 ) {
	    // Windows NT
	    wsprintf (szVersion, "Microsoft Windows NT %u.%u (Build: %u)",
	       (DWORD)(LOBYTE(LOWORD(dwVersion))),
	       (DWORD)(HIBYTE(LOWORD(dwVersion))),
		    (DWORD)(HIWORD(dwVersion)) );
	} else if (LOBYTE(LOWORD(dwVersion))<4) {
	    // Win32s
		wsprintf (szVersion, "Microsoft Win32s %u.%u (Build: %u)",
	       (DWORD)(LOBYTE(LOWORD(dwVersion))),
	       (DWORD)(HIBYTE(LOWORD(dwVersion))),
		    (DWORD)(HIWORD(dwVersion) & ~0x8000) );
	} else {
	    // Windows 95
		wsprintf (szVersion, "Microsoft Windows 95 %u.%u",
		    (DWORD)(LOBYTE(LOWORD(dwVersion))),
		    (DWORD)(HIBYTE(LOWORD(dwVersion))) );
	}

	xprintf("system: %s\n",szVersion);
	***/

	if( winSetSplashText( "OS version..." ) ) {

		OSVERSIONINFO info;

		info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		GetVersionEx( &info );

		xprintf("log: ");

		switch( info.dwPlatformId ) {
/***
			case VER_PLATFORM_WIN32_CE:

				xprintf("Microsoft Windows CE v%d.%d", info.dwMajorVersion, info.dwMinorVersion );

				break;
***/
			case VER_PLATFORM_WIN32s:

				xprintf("Microsoft Windows v%d.%d",/*info.szCSDVersion,*/info.dwMajorVersion,info.dwMinorVersion);

				break;

			case VER_PLATFORM_WIN32_WINDOWS:

				xprintf("Microsoft Windows %s v%d.%d (Build: %d)",
					(info.dwMinorVersion == 0? "95" : info.dwMinorVersion == 10 ? "98" : info.dwMinorVersion == 90 ? "Me" : "9x" ),
					/*info.szCSDVersion,*/info.dwMajorVersion,info.dwMinorVersion,LOWORD(info.dwBuildNumber));

				break;

			case VER_PLATFORM_WIN32_NT:

				if( info.dwMajorVersion==5 && info.dwMinorVersion==0 )
					xprintf("Microsoft Windows 2000 v%d.%d (Build: %d) With %s",/*info.szCSDVersion,*/info.dwMajorVersion,info.dwMinorVersion,info.dwBuildNumber, info.szCSDVersion);
				else
				if( info.dwMajorVersion==5 && info.dwMinorVersion==1 )
					xprintf("Microsoft Windows XP %s",info.szCSDVersion);
				else
				if( info.dwMajorVersion <= 4 )
					xprintf("Microsoft Windows NT v%d.%d (Build: %d) With %s",/*info.szCSDVersion,*/info.dwMajorVersion,info.dwMinorVersion,info.dwBuildNumber, info.szCSDVersion);
				else
					// for unknown windows/newest windows version
					xprintf("Windows v%d.%d (Build: %d) %s",/*info.szCSDVersion,*/info.dwMajorVersion,info.dwMinorVersion,info.dwBuildNumber, info.szCSDVersion);

				break;

			default:

				xprintf("it's Windows");
				break;
		}

		xprintf(".\n");
	}

	if( winSetSplashText( "CPU..." ) )
		XLIB_EnumerateProcessorType();

	if( winSetSplashText( "BIOS..." ) )
		XLIB_GetBios();

	if( winSetSplashText( "Uptime..." ) )
		XLIB_GetUptime();

	winSetSplashText( "Variables..." );

	XLIB_GetVariables();

	// LoadFont( DEFAULTFONT );

	winSetSplashText( "Timer..." );

	SetupTimer();

	// DestroyWindow( dlg );

	winSetSplashText( "LUT..." );
	GenTrig();

	winSetSplashText( "Random number generator..." );
	srand( time(NULL) );

	winSetSplashText( "loading background image..." );
	winSetBackImage( TRUE );

	winSetSplashText( "loading fonts..." );
	XLIB_winLoadFonts();

	if( CopyCheck() == FALSE )
		goto ende;

#ifdef __XLIB_EXCEPTION

	ULONG exceptval;

	__try {

#endif

//		winHideSplash();

		// AllocConsole();

		usermain = (int (GFXDLLCALLCONV *)(int argc, char *argv[], char *envp[]))proc_usermain;

		result = usermain( myargc, myargv, myenvp );

#ifdef __XLIB_EXCEPTION

	} __except( EvaluateException(exceptval = GetExceptionCode()) ) {

		// A Bad Thing happened, try to crash cleanly
		ExceptionHandler(exceptval);
	}
#endif

ende:;

	AVI_Finalize();		// FIXME: kell-e ez ide?

	DeinitString();

	FreeConsole();		// windoz

	CleanupTimer();

	DiscardFont();

	DeinitFfile();

	DeinitMemory();

	DeinitLogWindow();

	FreeFilenamString();

	return result;
}


BOOL (GFXDLLCALLCONV *XLIB_winShareware)( BOOL ) = NULL;


// cheat
// int usermain( int argc, char *argv[], char *envp[] ) { return 0; }
// char *getProgramDesc( void ) { return NULL; }

//
//
//
void GFXDLLCALLCONV XLIBDLL_GetInfo( XLIBDLLinfo_t *info ) {

	info->XLIB_WinMain =			(void *)XLIB_WinMain;

	info->GetWinError =			(void *)XLIB_GetWinError;
	info->winTemp =				(void *)XLIB_winTemp;
	info->GetWinMemInfo =			(void *)XLIB_GetWinMemInfo;

	info->SetX =				(void *)XLIB_SetX;
	info->ResetX =				(void *)XLIB_ResetX;

	info->IsIjl  =				(void *)XLIB_IsIjl;
	info->LoadIjl =				(void *)XLIB_LoadIjl;
	info->SaveIjl =				(void *)XLIB_SaveIjl;
	info->SetRgbIjl =			(void *)XLIB_SetRgbIjl;
	info->GetRgbIjl =			(void *)XLIB_GetRgbIjl;
	info->GetIjlDim =			(void *)XLIB_GetIjlDim;

	info->AVI_Open =			(void *)XLIB_AVI_Open;
	info->AVI_Close =			(void *)XLIB_AVI_Close;
	info->AVI_DrawFrame =			(void *)XLIB_AVI_DrawFrame;
	info->AVI_GetFrame =			(void *)XLIB_AVI_GetFrame;

	info->winCopyTextToClipboard =		(void *)XLIB_winCopyTextToClipboard;
	info->winPasteTextFromClipboard =	(void *)XLIB_winPasteTextFromClipboard;
	info->winGetSysDirectory =		(void *)XLIB_winGetSysDirectory;
	info->winFindFirstFile =		(void *)XLIB_winFindFirstFile;
	info->winFindNextFile =			(void *)XLIB_winFindNextFile;
	info->winFindClose =			(void *)XLIB_winFindClose;
	info->winDeleteFile =			(void *)XLIB_winDeleteFile;
	info->winGetCurrentDirectory =		(void *)XLIB_winGetCurrentDirectory;
	info->winSetCurrentDirectory =		(void *)XLIB_winSetCurrentDirectory;
	info->winGetFontDir =			(void *)XLIB_winGetFontDir;
	info->winGetMyDocumentsDir =		(void *)XLIB_winGetMyDocumentsDir;
	info->winGetProgramFilesDir =		(void *)XLIB_winGetProgramFilesDir;
	info->winBrowseForFolder =		(void *)XLIB_winBrowseForFolder;
	info->winIsConnected =			(void *)XLIB_winIsConnected;
	info->winLoadBitmap =			(void *)XLIB_winLoadBitmap;
	info->winPlayWave =			(void *)XLIB_winPlayWave;
	info->winMagyar =			(void *)XLIB_winMagyar;
	info->winReadProfileInt =		(void *)XLIB_winReadProfileInt;
	info->winWriteProfileInt =		(void *)XLIB_winWriteProfileInt;
	info->winReadProfileFloat =		(void *)XLIB_winReadProfileFloat;
	info->winWriteProfileFloat =		(void *)XLIB_winWriteProfileFloat;
	info->winReadProfileString =		(void *)XLIB_winReadProfileString;
	info->winWriteProfileString =		(void *)XLIB_winWriteProfileString;
	info->winDeleteProfileKey =		(void *)XLIB_winDeleteProfileKey;
	info->GetUptime =			(void *)XLIB_GetUptime;
	info->GetCPUSpeed =			(void *)XLIB_GetCPUSpeed;
	info->xsystem =				(void *)XLIB_xsystem;
	info->winAlloc =			(void *)XLIB_winAlloc;
	info->winFree =				(void *)XLIB_winFree;
	info->IsSofticeLoaded =			(void *)XLIB_IsSofticeLoaded;
	info->EnumerateProcessorType =		(void *)XLIB_EnumerateProcessorType;
	info->CheckBernie =			(void *)XLIB_CheckBernie;
	info->winGetMachineName =		(void *)XLIB_winGetMachineName;
	info->winGetUserName =			(void *)XLIB_winGetUserName;
	info->winSetCaption =			(void *)XLIB_winSetCaption;
	info->winLogLevel =			(void *)XLIB_winLogLevel;
	info->winGetNextFile =			(void *)XLIB_winGetNextFile;
	info->winGetUrlDocument =		(void *)XLIB_winGetUrlDocument; // (ULONG)XLIB_GetCurlURL;
	info->getNetVersion =			(void *)XLIB_getNetVersion;
	info->CheckVersion =			(void *)XLIB_CheckVersion;
	info->DumpProcessInfo =			(void *)XLIB_DumpProcessInfo;
	info->winFindTargetWindow =		(void *)XLIB_winFindTargetWindow;
	info->winNoSplash =			(void *)XLIB_winNoSplash;
	info->winRegNoLog =			(void *)XLIB_winRegNoLog;
	info->winTimeZone =			(void *)XLIB_winTimeZone;
	info->winSetWindowTrans =		(void *)XLIB_winSetWindowTrans;
	info->winGetMouseScrollLines =		(void *)XLIB_winGetMouseScrollLines;
	info->winLoadFonts =			(void *)XLIB_winLoadFonts;
	info->winRemoveFonts =			(void *)XLIB_winRemoveFonts;
	info->winGetMaxWidth =			(void *)XLIB_winGetMaxWidth;
	info->winGetMaxHeight =			(void *)XLIB_winGetMaxHeight;
	info->winBeep =				(void *)XLIB_winBeep;

	XLIB_winSetProgramDesc( (char *)info->ProgramDesc );
	XLIB_winShareware = (BOOL (GFXDLLCALLCONV *)( BOOL ))info->winShareware;

	return;
}


//
// DllEntryPoint
//
BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {

	switch( fdwReason ) {

		case DLL_PROCESS_ATTACH:

			// return dll_init();

			break;

		case DLL_PROCESS_DETACH:

			// dll_deinit();

			break;
	}

	return TRUE;
}



