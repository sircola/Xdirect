
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <stdio.h>
#include <windows.h>

#include <xrcs.h>

#include "dllmain.h"

RCSID( "$Id: dllmain.cpp,v 1.1.1.1 2003/08/19 17:44:49 bernie Exp $" )


BOOL (*dwinQuit)( char *s, ... ) = NULL;
void *(*drealloc)( void *, int ) = NULL;
void *(*dmalloc)( int ) = NULL;


//
//
//
void dllQuit( char *s, ... ) {

	static BOOL sema = FALSE;
	char *quit_funcname = (char *)"Quit";

	if( sema == FALSE ) {

		sema = TRUE;

		char text[8*1024];
		va_list args;

		if( s ) {
			char buf[2*1024];
			va_start(args, s);
			_vsnprintf(buf,2*1024, s, args);
			va_end(args);
			sprintf( text, "Error from \"%s\":\n\n%s", GFXDLL_NAME, buf );
		}
		else
			sprintf( text, "Error from \"%s\" with null string.", GFXDLL_NAME );

		if( dprintf ) dprintf( text );

		dllDeinit();

		ShowCursor( TRUE );

		if( s ) {
			if( dwinQuit || dwinQuit( text ) == FALSE )
				MessageBox( dGetBackHwnd?(HWND)dGetBackHwnd():NULL, text, GFXDLL_NAME, MB_OK | MB_ICONERROR );
		}
	}

	exit(21);

	return;
}




//
// Change the resolution & bitdepth:
//  -Change back by calling ChangeDisplaySettings(0,0);
//  -Note the use of the undocumented CDS_FULLSCREEN to leave their icons alone
//  -Note the funny fallback to handle both Win95 and WinNT
//
BOOL ChangeResolution( int w, int h, int bitdepth ) {

	DEVMODE devMode;
	INT32   modeExist;
	INT32   modeSwitch;
	INT32   closeMode = 0;
	INT32   i;
	int bpp = bitdepth;

	if( dprintf )
		dprintf("ChangeResolution: entering.\n");

	memset( &devMode, 0, sizeof(DEVMODE) );	// Makes Sure Memory's Cleared
	devMode.dmSize = sizeof(DEVMODE);

	for( i=0; ;i++ ) {

		modeExist = EnumDisplaySettings(NULL, i, &devMode);

		/* Check if we ran out of modes */
		if( !modeExist ) {

			// ListDisplaySettings();

			/* Pony up an error message */
			dprintf("No matching mode to %ldx%ldx%ld-bit color available.\n", w, h, bpp);

			break;
		}

		// ha nem kell a bpp
		if( bitdepth == -1 )
			bpp = (int)devMode.dmBitsPerPel;

		/* Check for a matching mode */
		if( ((int)devMode.dmBitsPerPel == bpp) &&
		    ((int)devMode.dmPelsWidth == w) &&
		    ((int)devMode.dmPelsHeight == h) ) {

			dprintf("Trying %ldx%ldx%ld-bit color (%ld Hertz).\n", devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel, devMode.dmDisplayFrequency);

			modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

			if( modeSwitch == DISP_CHANGE_SUCCESSFUL )
				return TRUE;

			dprintf("Nope, didnt work.\n");

			if( !closeMode )
				closeMode = i;
		}
	}

  	//Might be running in Windows95, let's try without the hertz change
	EnumDisplaySettings(NULL, closeMode, &devMode);

	if( bitdepth != -1 )
		devMode.dmBitsPerPel = bitdepth;
	devMode.dmPelsWidth  = w;
	devMode.dmPelsHeight = h;
	devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	dprintf("Trying %ldx%ldx%ld-bit color (Win95 Hack).\n", devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel);

	modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
	if(modeSwitch == DISP_CHANGE_SUCCESSFUL) return TRUE;
	dprintf("Nope, didnt work.\n");

	/* try with separate changes to bitdepth and resolution */
	devMode.dmFields = DM_BITSPERPEL;
	dprintf("ChangeResolution: Trying %ldx%ldx%ld-bit color (Separate BPP switch).\n", devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel);
	modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

	if( modeSwitch == DISP_CHANGE_SUCCESSFUL ) {
		devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		dprintf("Trying %ldx%ldx%ld-bit color (Separate resolution switch).\n", devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel);

		modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);

		if(modeSwitch == DISP_CHANGE_SUCCESSFUL)
			return TRUE;

		ChangeDisplaySettings(NULL, 0);
	}

	dprintf("Nope, didnt work.\n");

	/* Well nothing works now does it */
	if(modeSwitch == DISP_CHANGE_RESTART) {
		dprintf("You must change your video mode to %ldx%ldx%ld-bit color and then restart.\n", w, h, bitdepth);
	}
	else
	if(modeSwitch == DISP_CHANGE_BADMODE) {
		dprintf("The video mode %ldx%ldx%ld-bit color is not supported.\n", w, h, bitdepth);
	}
	else
	if(modeSwitch == DISP_CHANGE_FAILED) {
		dprintf("Hardware failed to change to %ldx%ldx%ld-bit color.\n", w, h, bitdepth);
	}
	else {
		dprintf("Failed to change to %ldx%ldx%ld-bit color - unknown reason.\n", w, h, bitdepth);
	}

	return FALSE;
}





//
//
//
void RestoreResoltion( void ) {

	ChangeDisplaySettings( NULL, 0 );

	return;
}





typedef struct {
	int			xres,yres,bits;
} mEntry;

static int numModes = 0;		/* Number of available modes */
static mEntry *modeList = NULL;		/* List of available modes */

//
//
//
void dibdetect( void ) {

	int modenum,stat,cdsStat,i;
	DEVMODE dv;

	// Enumerate all the available modes, filtering out those that require a
	// reboot of the system.

	modenum = 0;
	numModes = 0;

	do {

		memset( &dv, 0, sizeof(DEVMODE) );
		dv.dmSize = sizeof(DEVMODE);

		if( (stat = EnumDisplaySettings(NULL, modenum++, &dv)) != 0 ) {

			dv.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			cdsStat = ChangeDisplaySettings( &dv, CDS_TEST | CDS_FULLSCREEN );

			if( cdsStat == DISP_CHANGE_SUCCESSFUL ) {

				// Add the mode to the list. Note that some drivers report the
				// same mode more than once, so we check for this to ensure
				// we dont get duplicate modes.

				for( i=0; i<numModes; i++ ) {
					if( modeList[i].xres == (int)dv.dmPelsWidth &&
					    modeList[i].yres == (int)dv.dmPelsHeight &&
					    modeList[i].bits == (int)dv.dmBitsPerPel )
						break;
				}

				if( i >= numModes ) {

					modeList = (mEntry *)drealloc( modeList, (numModes+1) * sizeof(mEntry) );

					modeList[numModes].xres = dv.dmPelsWidth;
					modeList[numModes].yres = dv.dmPelsHeight;
					modeList[numModes].bits = dv.dmBitsPerPel;

					++numModes;
				}
			}
		}

	} while( stat );

	for( i=0; i<numModes; i++ )
		dprintf("res: %d x %d %dbpp\n", modeList[i].xres, modeList[i].yres, modeList[i].bits );

	if( modeList )
		dfree( modeList );

	return;
}





//
// DllEntryPoint
//
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {


	switch( fdwReason ) {

		case DLL_PROCESS_ATTACH:

			return dll_init();

			break;

		case DLL_PROCESS_DETACH:

			dll_deinit();

			break;
	}

	return TRUE;
}


/***

// ha debug van akkor lesz ez a func, ezzel lehet csekkelni
#ifdef _DEBUG

extern "C" __declspec(dllexport) BOOL __cdecl GFXDRV_DebugVersion( void );


//
//
//
BOOL __cdecl GFXDRV_DebugVersion( void ) {


	return TRUE;
}

#endif
***/