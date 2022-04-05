
#define WIN32_MEAN_AND_LEAN

#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <sdl.h>

#define _EXPORTING

#include <xlib.h>
#include <xinner.h>

#include "dllmain.h"

RCSID( "$Id: sdl.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )

// http://wiki.libsdl.org/MigrationGuide


#define GFXDLL_VERSION "0.9"
char *GFXDLL_NAME = "xsdl.drv";

#pragma comment( exestr , "SDL v2 driver v" GFXDLL_VERSION " (" __DATE__ " " __TIME__ ")")

#include "xsdl.h"


static char *sdldllname = "SDL2.DLL";
static char sdl_error_str[512] = "SDL: uninitialized error string";



// http://www.unitedbytes.net


void (*dprintf)( char *s, ... ) = NULL;
texmap_t *(*dTexForNum)( int texmapid ) = NULL;
void (*dCopyVector)( point3_t dst, point3_t src ) = NULL;
int (*SD_winReadProfileInt)( char *, int ) = NULL;
void (*SD_winWriteProfileInt)( char *, int ) = NULL;
int (*dfree)( void* ) = NULL;


int dactive_flag = 1;

hardwarestate_t hw_state;

BOOL (*dwinFullscreen)( void ) = NULL;
ULONG (*dGetBackHwnd)( void ) = NULL;

// #define SDLCALL __cdecl

static void (SDLCALL *pSDL_FreeSurface)( SDL_Surface *surface ) = NULL;
static int (SDLCALL *pSDL_Flip)( SDL_Surface *screen ) = NULL;
static int (SDLCALL *pSDL_Init)( Uint32 flags ) = NULL;
static int (SDLCALL *pSDL_LockSurface)( SDL_Surface *surface ) = NULL;
static void (SDLCALL *pSDL_UnlockSurface)( SDL_Surface *surface ) = NULL;
static void (SDLCALL *pSDL_Quit)( void ) = NULL;
static SDL_Surface *(SDLCALL *pSDL_SetVideoMode)( int width, int height, int bpp, Uint32 flags ) = NULL;
static char *(SDLCALL *pSDL_VideoDriverName)(char *namebuf, int maxlen) = NULL;
static int (SDLCALL *pSDL_FillRect)( SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color ) = NULL;
static char * (SDLCALL *pSDL_GetError)(void);
static const SDL_version * (SDLCALL *pSDL_Linked_Version)(void);
static SDL_Window * (SDLCALL *pSDL_CreateWindow)(const char *title,int x, int y, int w,int h, Uint32 flags);
extern void (SDLCALL *pSDL_RenderPresent)(SDL_Renderer * renderer);


static SDL_Surface *screen = NULL;   // The screen surface

static SDL_Color white = {255, 255, 255};
static SDL_Color cyan = {0, 255, 255};
static SDL_Color orange = {255, 128, 0};



//
//
//
static BOOL SD_LockLfb( __int64 *mem, int flag ) {

	USEPARAM( flag );

	mem = NULL;

	return TRUE;
}





//
//
//
static void SD_UnlockLfb( void ) {

	return;
}






//
//
//
static void SD_SetupCulling( point3_t pos, point3_t dir, point3_t normal, FLOAT zn, FLOAT zf ) {


	return;
}





static BOOL SD_AddToPipeline( trans2pipeline_t trans2pipeline ) {

	return TRUE;
}




//
//
//
static void SD_PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) {


	return;
}


//
//
//
static void SD_PutPoly( polygon_t ppoly, rgb_t *rgb ) {

	return;
}






//
// only HIGH sprite
//
static int SD_AddTexMapHigh( texmap_t *texmap ) {

	return texmap->texmapid;
}




//
//
//
static int SD_ReloadTexMapHigh( texmap_t *texmap ) {


	return texmap->texmapid;
}


//
//
//
static int SD_ModifyTexMapHigh( texmap_t *texmap ) {


	return texmap->texmapid;
}




//
//
//
static void SD_DiscardAllTexture( void ) {

	return;
}


//
//
//
static void SD_DiscardTexture( int handler ) {

	return;
}


//
//
//
static void SD_Line( int x0, int y0, int x1, int y1, rgb_t color ) {

	return;
}


//
//
//
static void SD_PutPixel( int x, int y, rgb_t color ) {

	Uint32 *pixmem32;
	Uint32 color32 = color.r + color.g + color.b;

	pixmem32 = (Uint32*) screen->pixels + (y * ((screen->pitch)/4)) + x;
	*pixmem32 = color32;

	return;
}



//
//
//
void *SD_GetVideoPtr( int x, int y ) {

	//dprintf("%d %d\n",x,y);

	return NULL;
}




//
//
//
static void SD_BeginScene( void ) {

	return;
}





//
//
//
static void SD_EndScene( void ) {

	return;
}




//
//
//
static void SD_FlushScene( void ) {

	SD_EndScene();
	SD_BeginScene();

	return;
}




//
//
//
static void SD_Fog( FLOAT fn, FLOAT ff, ULONG fogcolor ) {

	return;
}







//
//
//
static BOOL SD_Activate( int flag ) {

	if( (dactive_flag = flag) ) {

	}

	return FALSE;
}





//
//
//
static void SD_InitPalette( UCHAR *pal ) {

	return;
}






//
//
//
static void SD_SetRgb( int color, int red, int green, int blue ) {

	return;
}






//
//
//
static void SD_GetRgb( int color, int *red, int *green, int *blue ) {

	return;
}







//
//
//
static ULONG SD_GetPitch( void ) {

	return (ULONG)( hw_state.SCREENW * hw_state.pixel_len );
}









//
//
//
static void SD_FlipPage( void ) {

	pSDL_Flip( screen );

	return;
}






//
//
//
static void SD_Clear( ULONG color, ULONG depth, int flag ) {

	pSDL_FillRect( screen, NULL, color );   // Clear the entire screen with black

	return;
}








//
//
//
static void SD_SetClip( int cmx, int cmy, int cxx, int cxy ) {

	hw_state.CLIPMINX = cmx;
	hw_state.CLIPMINY = cmy;
	hw_state.CLIPMAXX = cxx;
	hw_state.CLIPMAXY = cxy;

	return;
}




//
//
//
static BOOL SD_Init( gfxdlldesc_t *desc ) {

	char buf[MAX_PATH];

	desc->bpp = 16;

	hw_state.hwnd = (HWND)desc->hwnd;
	hw_state.SCREENW = desc->width;
	hw_state.SCREENH = desc->height;
	hw_state.bpp = desc->bpp;
	hw_state.pixel_len = desc->bpp/8;
	hw_state.texmap = -1;
	hw_state.mode = HM_TEXTURE;
	hw_state.hInst = (HINSTANCE)desc->hInst;
	hw_state.tex24bit = FALSE;


	dprintf = (void (GFXDLLCALLCONV *)(char *s,...))desc->xprintf;
	dfree = (int (GFXDLLCALLCONV *)(void *))desc->free;
	dmalloc = (void *(GFXDLLCALLCONV *)(int))desc->malloc;
	dwinQuit = (BOOL (GFXDLLCALLCONV *)(char *s,...))desc->Quit;
	dGetBackHwnd = (ULONG (GFXDLLCALLCONV *)(void))desc->GetBackHwnd;
	dTexForNum = (texmap_t *(GFXDLLCALLCONV *)(int texmapid))desc->TexForNum;
	dCopyVector = (void (GFXDLLCALLCONV *)(point3_t dst, point3_t src))desc->CopyVector;
	dwinFullscreen = (BOOL (GFXDLLCALLCONV *)(void))desc->winFullscreen;
	SD_winReadProfileInt = (int (GFXDLLCALLCONV *)(char*,int))desc->winReadProfileInt;
	SD_winWriteProfileInt = (void (GFXDLLCALLCONV *)(char*,int))desc->winWriteProfileInt;

	dprintf("init: %s v%s (c) Copyright 2002 Bernie Kirschner (%s %s)\n",GFXDLL_NAME,GFXDLL_VERSION,__DATE__,__TIME__);

	if( LoadSdl() == FALSE )
		return FALSE;

	// FORCE quitting FIXME
	const SDL_version *version = pSDL_Linked_Version();
	sprintf(sdl_error_str, "SDL driver is currently disabled!\n\nCheck for updates at Gamesfrom21.webs.com.\n\nPlease choose another driver. Thank you!\n\n(good news: found SDL v%d.%d.%d)",version->major, version->minor, version->patch);
	return FALSE;

	if( hw_state.bpp != 16 )
		return FALSE;

	if( pSDL_Init(SDL_INIT_VIDEO) < 0 ) {
		sprintf(sdl_error_str, "SDL_Init returned: %s\n", pSDL_GetError());
		return FALSE;
	}

	if( dwinFullscreen() == FALSE )
		// Initialize the display in fullscreen
		// screen = pSDL_SetVideoMode( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp, SDL_SWSURFACE|SDL_DOUBLEBUF );
		SDL_Window *screen = SDL_CreateWindow("SDL window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, hw_state.SCREENW, hw_state.SCREENH, SDL_WINDOW_OPENGL);
	else
		// Initialize the display windowed
		// screen = pSDL_SetVideoMode( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN );
		SDL_Window *screen = SDL_CreateWindow("SDL window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, hw_state.SCREENW, hw_state.SCREENH, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);

	if( screen == NULL ) {
		sprintf(sdl_error_str, "SDL_SetVideoMode returned: %s\n", pSDL_GetError());
		return FALSE;
	}

	pSDL_VideoDriverName( buf, sizeof(buf) );
	dprintf("log: SDL driver: %s\n", buf );

	return TRUE;
}






//
//
//
static void SD_Deinit( void ) {

	SD_DiscardAllTexture();

	if( screen ) {
		pSDL_FreeSurface( screen );
		screen = NULL;
	}

	if( pSDL_Quit )
		pSDL_Quit();

	FreeSdl();

	if( dprintf ) dprintf("deinit: %s success.\n",GFXDLL_NAME);

	return;
}



//
//
//
static BOOL GFXDLLCALLCONV SD_Fullscreen( int flag ) {

	dprintf("SD_Fullscreen: Switched unsupported.\n");

	return -1;
}



//
//
//
static void SD_GetDescription( char *str ) {

#ifdef _DEBUG
	sprintf(str,"SDL v1.12 driver v%s (need libSDL v%d.%d.%d)",GFXDLL_VERSION,SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
#else
	sprintf(str,"SDL v1.12 driver v%s",GFXDLL_VERSION);
#endif

	return;
}



//
//
//
static void SD_GetData( void *buf, int len ) {

	if( *(ULONG *)buf == GFXDLL_ISMSGOK )
		*(ULONG *)buf = TRUE;

	if( *(ULONG *)buf == GFXDLL_24BIT )
		*(ULONG *)buf = FALSE;

	if( *(ULONG *)buf == GFXDLL_MAXTEXSIZE )
		*(ULONG *)buf = 256;

	if( *(ULONG *)buf == GFXDLL_MINTEXSIZE )
		*(ULONG *)buf = 64;

	if( *(ULONG *)buf == GFXDLL_GETERROR )
		strncpy( (char *)buf, sdl_error_str, len );
		// *(ULONG *)buf = 0;

	if( *(ULONG *)buf == GFXDLL_TEX24BIT )
		*(ULONG *)buf = hw_state.tex24bit;

	if( *(ULONG *)buf == GFXDLL_THREADSAFE )
		*(ULONG *)buf = FALSE;

	if( *(ULONG *)buf == GFXDLL_ISDEBUG )

#ifdef _DEBUG
		*(ULONG *)buf = TRUE;
#else
		*(ULONG *)buf = FALSE;
#endif

	if( *(ULONG *)buf == GFXDLL_ENABLED ) {

		HINSTANCE sdl = 0;			// bass handle

		if( (sdl = LoadLibrary(sdldllname)) == NULL ) {
			*(ULONG *)buf = FALSE;
			sprintf( sdl_error_str, "SD_GetData: need at least SDL v%d.%d.%d (no %s library).",SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, sdldllname );
		}
		else {
			FreeLibrary(sdl);
			*(ULONG *)buf = TRUE;
		}
	}

	return;
}





//
//
//
void GFXDLLCALLCONV GFXDRV_GetInfo( GFXDLLinfo_t *info ) {

	info->GFXDLL_SetupCulling	= (void *)SD_SetupCulling;
	info->GFXDLL_AddToPipeline	= (void *)SD_AddToPipeline;
	info->GFXDLL_AddTexMapHigh	= (void *)SD_AddTexMapHigh;
	info->GFXDLL_ReloadTexMapHigh	= (void *)SD_ReloadTexMapHigh;
	info->GFXDLL_ModifyTexMapHigh	= (void *)SD_ModifyTexMapHigh;
	info->GFXDLL_PutSpritePoly	= (void *)SD_PutSpritePoly;
	info->GFXDLL_PutSpritePoly	= (void *)SD_PutPoly;
	info->GFXDLL_DiscardAllTexture	= (void *)SD_DiscardAllTexture;
	info->GFXDLL_DiscardTexture	= (void *)SD_DiscardTexture;
	info->GFXDLL_Line		= (void *)SD_Line;
	info->GFXDLL_PutPixel		= (void *)SD_PutPixel;
	info->GFXDLL_BeginScene		= (void *)SD_BeginScene;
	info->GFXDLL_EndScene		= (void *)SD_EndScene;
	info->GFXDLL_FlushScene		= (void *)SD_FlushScene;
	info->GFXDLL_LockLfb		= (void *)SD_LockLfb;
	info->GFXDLL_UnlockLfb		= (void *)SD_UnlockLfb;
	info->GFXDLL_Init		= (void *)SD_Init;
	info->GFXDLL_Deinit		= (void *)SD_Deinit;
	info->GFXDLL_GetDescription	= (void *)SD_GetDescription;
	info->GFXDLL_GetData		= (void *)SD_GetData;
	info->GFXDLL_GetPitch		= (void *)SD_GetPitch;
	info->GFXDLL_Activate		= (void *)SD_Activate;
	info->GFXDLL_FlipPage		= (void *)SD_FlipPage;
	info->GFXDLL_Clear		= (void *)SD_Clear;
	info->GFXDLL_Fog		= (void *)SD_Fog;
	info->GFXDLL_InitPalette	= (void *)SD_InitPalette;
	info->GFXDLL_SetRgb		= (void *)SD_SetRgb;
	info->GFXDLL_GetRgb		= (void *)SD_GetRgb;
	info->GFXDLL_Fullscreen		= (void *)SD_Fullscreen;

	return;
}



#define SDLDEF(f) (WINAPI *(f))		// define the functions as pointers

static HINSTANCE sdl_lib = 0;			// bass handle


//
//
//
static BOOL errstr( char *funcName ) {

	dprintf( "LoadSdl: no \"%s\" function.\n", funcName );

	sprintf( sdl_error_str, "LoadSDL: can't find \"%s\" function in \"%s\" library.\n\nPlease upgrade your SDL library to at least %d.%d.%d version from www.libsdl.org.", funcName, sdldllname, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL );

	return FALSE;
}


//
// load BASS and the required functions
//
BOOL LoadSdl( void ) {

	if( (sdl_lib = LoadLibrary(sdldllname)) == NULL ) {
		dprintf("LoadSdl: couldn't load %s library.\n",sdldllname);
		sprintf( sdl_error_str, "You don't have SDL library correctly installed.\n\nCan't load \"%s\" library. Please install Simple DirectMedia Layer library at least %d.%d.%d version from www.libsdl.org.\n\nThen put \"%s\" file into your PATH or into the directory where you installed this program.", sdldllname,SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, sdldllname );
		return FALSE;
	}

	if( (pSDL_FreeSurface = (void (SDLCALL *)(SDL_Surface *surface))GetProcAddress( sdl_lib, "SDL_FreeSurface" )) == NULL ) return errstr( "SDL_FreeSurface" );
	if( (pSDL_VideoDriverName = (char *(SDLCALL *)(char *namebuf, int maxlen))GetProcAddress( sdl_lib, "SDL_VideoDriverName" )) == NULL ) return errstr( "SDL_VideoDriverName" );
	if( (pSDL_Flip = (int (SDLCALL *)(SDL_Surface *surface))GetProcAddress( sdl_lib, "SDL_Flip" )) == NULL ) return errstr( "SDL_Flip" );
	if( (pSDL_Init = (int (SDLCALL *)(Uint32 flags))GetProcAddress( sdl_lib, "SDL_Init" )) == NULL ) return errstr( "SDL_Init" );
	if( (pSDL_LockSurface = (int (SDLCALL *)(SDL_Surface *surface))GetProcAddress( sdl_lib, "SDL_LockSurface" )) == NULL ) return errstr( "SDL_LockSurface" );
	if( (pSDL_UnlockSurface = (void (SDLCALL *)(SDL_Surface *surface))GetProcAddress( sdl_lib, "SDL_UnlockSurface" )) == NULL ) return errstr( "SDL_UnlockSurface" );
	if( (pSDL_Quit = (void (SDLCALL *)(void))GetProcAddress( sdl_lib, "SDL_Quit" )) == NULL ) return errstr( "SDL_Quit" );
	if( (pSDL_SetVideoMode = (SDL_Surface *(SDLCALL *)( int width, int height, int bpp, Uint32 flags))GetProcAddress( sdl_lib, "SDL_SetVideoMode" )) == NULL ) return errstr( "SDL_SetVideoMode" );
	if( (pSDL_FillRect = (int (SDLCALL *)(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color))GetProcAddress( sdl_lib, "SDL_FillRect" )) == NULL ) return errstr( "SDL_FillRect" );
	if( (pSDL_GetError = (char *(SDLCALL *)(void))GetProcAddress( sdl_lib, "SDL_GetError" )) == NULL ) return errstr( "SDL_GetError" );
	if( (pSDL_Linked_Version = (const SDL_version *(SDLCALL *)(void))GetProcAddress( sdl_lib, "SDL_Linked_Version" )) == NULL ) return errstr( "SDL_Linked_Version" );

	const SDL_version *version = pSDL_Linked_Version();

	dprintf("init: SDL v%d.%d.%d (%s) loaded (compiled with SDL v%d.%d.%d).\n", version->major, version->minor, version->patch,sdldllname, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

	return TRUE;
}



//
//
//
void FreeSdl( void ) {

	if( sdl_lib )
		FreeLibrary( sdl_lib );
	sdl_lib = NULL;

	// DeleteFile(tempfile);

	return;
}





//
//
//
extern "C" void dllDeinit( void ) {

	SD_Deinit();

	return;
}



//
//
//
extern "C" unsigned long dllGetHinst( void ) {

	return (unsigned long)hw_state.hInst;
}




//
//
//
int dll_init( void ) {


	return TRUE;
}



//
//
//
void dll_deinit( void ) {

	return;
}




void CopyPixels( SDL_Surface *src, SDL_Surface *dst ) {

	Uint8 *srcp, *dstp;
	int w, h;

	if( (src->w != dst->w) || (src->h != dst->h) ||
	    (src->format->BytesPerPixel != dst->format->BytesPerPixel) ) {
		/* Different sized surfaces... */
		return;
	}

	srcp = (Uint8 *)src->pixels;
	dstp = (Uint8 *)dst->pixels;
	h = src->h;
	w = src->w*src->format->BytesPerPixel;	/* Width in bytes */

	while( h-- ) {
		memcpy(dstp, srcp, w);
		srcp += src->pitch;  /* Not necessarily 'w' */
		dstp += dst->pitch;  /* Not necessarily src->pitch */
	}

	return;
}




/***

CUB_GraphicServer::CUB_GraphicServer ()
{
	m_Config.m_iScreenSizeX = 0;
	m_Config.m_iScreenSizeY = 0;
	m_Config.m_bIsRunning   = false;

	m_pScreenSurface = NULL;
	m_pBackSurface   = NULL;

	memset (m_Config.m_cErrorText, 0, sizeof(m_Config.m_cErrorText));
}

CUB_GraphicServer::~CUB_GraphicServer ()
{
	Shutdown ();
}

bool CUB_GraphicServer::IsRunning ()
{
	return m_Config.m_bIsRunning;
}

bool CUB_GraphicServer::FlipScreen ()
{
	#ifdef _DEBUG
		// Check for valid pointers
		sAssert (m_pScreenSurface);
	#endif

	SDL_Flip (m_pScreenSurface);

	return true;
}

SDL_Surface* CUB_GraphicServer::GetBackBuffer ()
{
	return m_pBackSurface;
}

SDL_Surface* CUB_GraphicServer::GetScreenBuffer ()
{
	return m_pScreenSurface;
}

bool CUB_GraphicServer::BeginScene ()
{
	return true;
}

bool CUB_GraphicServer::EndScene ()
{
	return true;
}

HWND CUB_GraphicServer::GetWindowHandle ()
{
	return NULL;
}

bool CUB_GraphicServer::ClearScreen ()
{
	return true;
}

bool CUB_GraphicServer::BeginSpriteRendering ()
{
	return true;
}

bool CUB_GraphicServer::EndSpriteRendering ()
{
	return true;
}

bool CUB_GraphicServer::GetEngineConfig (char *cConfigFile)
{
	//////////////////////////////////////////////////////////////////////
	// Gets the engine configuration from an .ini file
	//////////////////////////////////////////////////////////////////////

	char  cBuffer [1024];
	int   iError = 0;
	FILE *pFh    = 0;

	// Get the current path and add it to the config file
	GetCurrentDirectory (MAX_PATH, cBuffer);
	strcat (cBuffer, "\\");
	strcat (cBuffer, cConfigFile);

	// Try to open the config file and check if it´s present
	pFh = fopen (cConfigFile, "r");

	if (!pFh) {
		// Error
		sprintf (m_Config.m_cErrorText, "CUB_GraphicServerD3D->GetEngineConfig: Could not open engine configuration!");
		iError = 1;
	}
	else {
		// Close the config file
		fclose (pFh);

		// Get width and height of the render screen
		m_Config.m_iScreenSizeX = GetInt ("DeviceOptions", "ScreenWidth" , 640, cBuffer);
		m_Config.m_iScreenSizeY = GetInt ("DeviceOptions", "ScreenHeight", 480, cBuffer);

		// Get bits per pixel
		m_Config.m_iBitsPerPixel = GetInt ("DeviceOptions", "BitsPerPixel" , 16, cBuffer);

		// Get bits per texel
		m_Config.m_iBitsPerTexel = GetInt ("DeviceOptions", "BitsPerTexel" , 0, cBuffer);

		// Get fullscreen flag
		if (GetInt ("DeviceOptions", "FullScreen", 0, cBuffer) == 0)
		{
			m_Config.m_bFullScreen = false;
		}
		else {
			m_Config.m_bFullScreen = true;
		}

		// Get hardware/software rendering flag
		if (GetInt ("DeviceOptions", "HardwareRendering", 0, cBuffer) == 0)
		{
			m_Config.m_bHardwareRendering = false;
		}
		else {
			m_Config.m_bHardwareRendering = true;
		}

		////////////////////////////////////////////////////////////////////////

		// Set logging text
		sprintf (m_Config.m_cErrorText, "CUB_GraphicServerD3D->GetEngineConfig ...[DONE]");
	}

	// Log this event
	if (LoggingActive()) {
		GetLogFile()->Print (m_Config.m_cErrorText);
	}

	if (iError == 1)
		return false;

	return true;
}

bool CUB_GraphicServer::Initialize (int iScreenSizeX, int iScreenSizeY, bool bFullScreen, bool bUseHardwareRendering, bool bUseConfigFile)
{
	int iError = 0;

	// Shutdown old data
	Shutdown ();

	if (!bUseConfigFile)
	{
		m_Config.m_iScreenSizeX       = iScreenSizeX;
		m_Config.m_iScreenSizeY       = iScreenSizeY;
		m_Config.m_bFullScreen        = bFullScreen;
		m_Config.m_bHardwareRendering = bUseHardwareRendering;
		m_Config.m_iBitsPerPixel      = 16;
	}

	if (SDL_Init (SDL_INIT_VIDEO) < 0) {
		sprintf (m_Config.m_cErrorText, "CUB_GraphicServer->Initialize: Could not initialize the video mode!");
		iError = 1;
	}
	else {
		// Hook at exit
		atexit (SDL_Quit);

		if (m_Config.m_bFullScreen)
		{
			// Initialize the display in fullscreen
			m_pScreenSurface = SDL_SetVideoMode (m_Config.m_iScreenSizeX, m_Config.m_iScreenSizeY, m_Config.m_iBitsPerPixel, SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
		}
		else
		{
			// Initialize the display windowed
			m_pScreenSurface = SDL_SetVideoMode (m_Config.m_iScreenSizeX, m_Config.m_iScreenSizeY, m_Config.m_iBitsPerPixel, SDL_HWSURFACE|SDL_DOUBLEBUF);
		}

		if (m_pScreenSurface == NULL) {
			sprintf (m_Config.m_cErrorText, "CUB_GraphicServer->Initialize: Could not set the video mode!");
			iError = 1;
		}
		else {
			// Set the window manager title bar
			SDL_WM_SetCaption (GAME_APP_NAME, 0);

			// Set logging text
			sprintf (m_Config.m_cErrorText, "CUB_GraphicServer->Initialize (SDL) ...[DONE]");
		}
	}

	// Log this event
	if (LoggingActive()) {
		GetLogFile()->Print (m_Config.m_cErrorText);
	}

	if (iError == 1)
		return false;

	return true;
}

void CUB_GraphicServer::Shutdown ()
{
	//////////////////////////////////////////////////////////////////////
	// Shuts the graphic server down
	//////////////////////////////////////////////////////////////////////

	// Log this event
	if (LoggingActive()) {
		GetLogFile()->Print ("CUB_GraphicServer->Shutdown ...[DONE]");
	}

	// Set flags
	m_Config.m_bIsRunning = false;

	// Delete dynamic objects
	if (m_pScreenSurface)
	{
		SDL_FreeSurface (m_pScreenSurface);
		m_pScreenSurface = NULL;
	}

	if (m_pBackSurface)
	{
		SDL_FreeSurface (m_pBackSurface);
		m_pBackSurface = NULL;
	}

	// Quit SDL
	SDL_Quit ();
}



//////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <SDL/SDL.h>

#include "font.h"

SDL_Surface *screen;   // The screen surface
SDLFont *font1;        // 2 fonts
SDLFont *font2;
int y=480;             // Position of the scrolling text

char *string="Cone3D GFX with SDL Lesson 4"; // The scrolling text

void DrawScene()
{
  SDL_FillRect(screen,NULL,0x000000);   // Clear the entire screen with black

  // Draw the string 'string' to the center of the screen, position: y
  drawString(screen,font1,320-stringWidth(font1,string)/2,y,string);

  // Draw a counter to the top-left of the screen
  drawString(screen,font2,1,1,"Scroll location: %d",y);

  // Draw the webspace url to the bottom-right of the screen
  drawString(screen,font2,639-stringWidth(font2,"http://cone3d.gamedev.net"),
                                         480-16,"http://cone3d.gamedev.net");
  SDL_Flip(screen);
}

int main(int argc, char *argv[])
{
  // Initalize SDL
  if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
  {
    printf("Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  // Load a bmp file as the icon of the program
  SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"),NULL);

  // Initalize the video mode
  screen=SDL_SetVideoMode(640,480,32,SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
  if ( screen == NULL )
  {
    printf("Unable to set 640x480 video: %s\n", SDL_GetError());
    exit(1);
  }

  // Load in the fonts
  font1 = initFont("data/font1");
  font2 = initFont("data/font2",1,1,0);

  // Loop a bit
  int done=0;
  while(done == 0)
  {
    SDL_Event event;

    while ( SDL_PollEvent(&event) )
    {
      // If someone closes the prorgam, then quit
      if ( event.type == SDL_QUIT )  {  done = 1;  }

      if ( event.type == SDL_KEYDOWN )
      {
        // If someone presses ESC, then quit
        if ( event.key.keysym.sym == SDLK_ESCAPE ) { done = 1; }
      }
    }

    // Draw the scene
    DrawScene();

    // Scroll the text
    y-=1;if(y<-32) y=480;
  }

  // Let's clean up...
  freeFont(font1);
  freeFont(font2);

  return 0;
}

***/

/***
   if(j->joystick && SDL_WasInit(SDL_INIT_JOYSTICK))
               SDL_JoystickClose(j->joystick);
***/
