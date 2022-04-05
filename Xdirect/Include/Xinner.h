/* Copyright (C) 1997-99 Kirschner, Bernát. All Rights Reserved Worldwide.	*/
/* mailto: bernie@freemail.c3.hu											*/
/* tel: +3620 333 9517														*/

#ifndef _XINNER_H_INCLUDED
#define _XINNER_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif


typedef struct gfxdlldesc_s {

	void *hwnd;
	void *hInst;

	int width,height,bpp,x,y;
	int flag;			// amit a SetX kapott

	BOOL bZBuffer;

	int rmask,gmask,bmask;
	int rsize,gsize,bsize;
	int rshift,gshift,bshift;

	void *xprintf;
	void *Quit;
	void *GetBackHwnd;
	void *CheckParm;
	void *TexForNum;
	void *CopyVector;
	void *DotProduct;
	void *winFullscreen;
	void *winReadProfileString;
	void *winWriteProfileString;
	void *winReadProfileInt;
	void *winWriteProfileInt;
	void *winOpengl;
	void *GetHwnd;
	void *GetHinstance;
	void *malloc;			// AllocMemNoPtr
	void *free;			// FreeMemNoPtr
	void *realloc;			// reallocNoPtr

	int temp1,temp2;		// kamu adatok átvitelére
	int temp3,temp4;

} gfxdlldesc_t, *gfxdlldesc_ptr;


#define GFXDLL_ISMSGOK		1
#define GFXDLL_24BIT		2
#define GFXDLL_GETERROR		3
#define GFXDLL_MINTEXSIZE	4
#define GFXDLL_MAXTEXSIZE	5
#define GFXDLL_RESIZE		6
#define GFXDLL_TEX24BIT		7
#define GFXDLL_ISDEBUG		8
#define GFXDLL_ENABLED		9
#define GFXDLL_THREADSAFE	10


typedef struct GFXDLLinfo_s {

	void *GFXDLL_SetupCulling;
	void *GFXDLL_AddToPipeline;
	void *GFXDLL_AddTexMapHigh;
	void *GFXDLL_ReloadTexMapHigh;
	void *GFXDLL_ModifyTexMapHigh;
	void *GFXDLL_PutSpritePoly;
	void *GFXDLL_PutPoly;
	void *GFXDLL_DiscardAllTexture;
	void *GFXDLL_DiscardTexture;
	void *GFXDLL_Line;
	void *GFXDLL_PutPixel;
	void *GFXDLL_BeginScene;
	void *GFXDLL_EndScene;
	void *GFXDLL_FlushScene;
	void *GFXDLL_LockLfb;
	void *GFXDLL_UnlockLfb;
	void *GFXDLL_Init;
	void *GFXDLL_Deinit;
	void *GFXDLL_GetDescription;
	void *GFXDLL_GetData;
	void *GFXDLL_GetPitch;
	void *GFXDLL_Activate;
	void *GFXDLL_FlipPage;
	void *GFXDLL_Clear;
	void *GFXDLL_Fog;
	void *GFXDLL_InitPalette;
	void *GFXDLL_SetRgb;
	void *GFXDLL_GetRgb;
	void *GFXDLL_Fullscreen;

} GFXDLLinfo_t, *GFXDLLinfo_ptr;


typedef struct XLIBDLLinfo_s {
/*
extern int XLIB_winCopyTextToClipboard( char *str );
extern BOOL XLIB_winGetSysDirectory( char *szPath );
extern void *XLIB_winFindFirstFile( char *mask, findfile_ptr find );
extern BOOL XLIB_winFindNextFile( void *handle, findfile_ptr find );
extern BOOL XLIB_winFindClose( void *handle );
extern BOOL XLIB_winDeleteFile( char *name );
extern int XLIB_winGetCurrentDirectory( char *dir, int num );
extern BOOL XLIB_winSetCurrentDirectory( char *dir );
extern BOOL XLIB_winGetFontDir( char *szPath );
extern BOOL XLIB_winBrowseForFolder( char *szPath );
extern BOOL XLIB_winIsConnected( void );
extern int XLIB_winLoadBitmap( void );
extern int XLIB_winPlayWave( char *str );
extern BOOL XLIB_winMagyar( void );
extern int XLIB_winReadProfileInt( char *key, int default_value );
extern void XLIB_winWriteProfileInt( char *key, int value );
extern double XLIB_winReadProfileFloat( char *key, double default_value );
extern void XLIB_winWriteProfileFloat( char *key, double value );
extern void XLIB_winReadProfileString( char *key, char *default_string, char *string, int size );
extern void XLIB_winWriteProfileString( char *key, char *string );
extern BOOL XLIB_winDeleteProfileKey( char *key );
extern void XLIB_GetUptime( void );
extern int XLIB_GetCPUSpeed( void );
extern void XLIB_xsystem( char * );
extern void *XLIB_winAlloc( int amount );
extern void XLIB_winFree( void *buf );
extern BOOL XLIB_IsSofticeLoaded( void );
extern void XLIB_EnumerateProcessorType( void );
extern BOOL XLIB_CheckBernie( void );
extern const char *XLIB_winGetMachineName( void );
extern const char *XLIB_winGetUserName( void );
extern void XLIB_winSetCaption( char *str, ... );
extern int XLIB_winLogLevel( void );
extern char *XLIB_winGetNextFile( void );
extern int XLIB_winGetUrlDocument( char *url, UCHAR *buf );
extern BOOL XLIB_getNetVersion( char *ident, int *major, int *minor );
extern BOOL XLIB_CheckVersion( char *name, char *version );
extern void XLIB_DumpProcessInfo( void );
extern ULONG XLIB_winFindTargetWindow( char *windowName );
extern BOOL XLIB_winNoSplash( void );
extern BOOL XLIB_winRegNoLog( void );
extern int XLIB_winTimeZone( void );
extern BOOL XLIB_winGetGammaRamp( void );
extern BOOL XLIB_winSetGammaRamp( FLOAT f );
extern void XLIB_winSetWindowTrans( ULONG hwnd, BOOL bTransparent, int nTransparency );
extern UINT XLIB_winGetMouseScrollLines( void );
char *XLIB_winSetProgramDesc( char *str );
extern BOOL XLIB_winLoadFonts( void );
extern void XLIB_winRemoveFonts( void );
*/
	void *winCopyTextToClipboard;
	void *winPasteTextFromClipboard;
	void *winGetSysDirectory;
	void *winFindFirstFile;
	void *winFindNextFile;
	void *winFindClose;
	void *winDeleteFile;
	void *winGetCurrentDirectory;
	void *winSetCurrentDirectory;
	void *winGetFontDir;
	void *winGetProgramFilesDir;
	void *winGetMyDocumentsDir;
	void *winBrowseForFolder;
	void *winIsConnected;
	void *winLoadBitmap;
	void *winPlayWave;
	void *winMagyar;
	void *winReadProfileInt;
	void *winWriteProfileInt;
	void *winReadProfileFloat;
	void *winWriteProfileFloat;
	void *winReadProfileString;
	void *winWriteProfileString;
	void *winDeleteProfileKey;
	void *GetUptime;
	void *GetCPUSpeed;
	void *xsystem;
	void *winAlloc;
	void *winFree;
	void *IsSofticeLoaded;
	void *EnumerateProcessorType;
	void *CheckBernie;
	void *winGetMachineName;
	void *winGetUserName;
	void *winSetCaption;
	void *winLogLevel;
	void *winGetNextFile;
	void *winGetUrlDocument;
	void *getNetVersion;
	void *CheckVersion;
	void *DumpProcessInfo;
	void *winFindTargetWindow;
	void *winNoSplash;
	void *winRegNoLog;
	void *winTimeZone;
	void *winGetGammaRamp;
	void *winSetGammaRamp;
	void *winSetWindowTrans;
	void *winGetMouseScrollLines;
	void *winLoadFonts;
	void *winRemoveFonts;

	void *XLIB_WinMain;

	void *GetWinError;
	void *winTemp;
	void *GetWinMemInfo;

	void *SetX;
	void *ResetX;

	void *IsIjl;
	void *LoadIjl;
	void *SaveIjl;
	void *SetRgbIjl;
	void *GetRgbIjl;
	void *GetIjlDim;

	void *AVI_Open;
	void *AVI_Close;
	void *AVI_DrawFrame;
	void *AVI_GetFrame;

	void *winShareware;
	void *ProgramDesc;

	void *winGetMaxWidth;
	void *winGetMaxHeight;
	void *winBeep;

} XLIBDLLinfo_t, *XLIBDLLinfo_ptr;



#undef DLLIMPORT

#ifndef XDLL_EXPORTS			// visual c++
// #ifdef _EXPORTING				// gcc

#define DLLIMPORT __declspec(dllexport)

DLLIMPORT extern void GFXDLLCALLCONV GFXDRV_GetInfo( GFXDLLinfo_t * );

/*
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_SetupCulling( point3_t pos, point3_t dir, point3_t normal, FLOAT zn, FLOAT zf );
DLLIMPORT extern BOOL GFXDLLCALLCONV GFXDLL_AddToPipeline( trans2pipeline_t );
DLLIMPORT extern int GFXDLLCALLCONV GFXDLL_AddTexMapHigh( texmap_t *texmap );
DLLIMPORT extern int GFXDLLCALLCONV GFXDLL_ReloadTexMapHigh( texmap_t *texmap );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_PutPoly( polygon_t ppoly, rgb_t *rgb );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_DiscardAllTexture( void );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_DiscardTexture( int handler );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_Line( int x0, int y0, int x1, int y1, ULONG color, int thick );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_PutPixel( int x, int y, ULONG color );

DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_BeginScene( void );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_EndScene( void );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_FlushScene( void );

DLLIMPORT extern BOOL GFXDLLCALLCONV GFXDLL_LockLfb( __int64 *, int flag );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_UnlockLfb( void );

DLLIMPORT extern BOOL GFXDLLCALLCONV GFXDLL_Init( gfxdlldesc_t *desc );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_Deinit( void );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_GetDescription( char * );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_GetData( void *, int );
DLLIMPORT extern ULONG GFXDLLCALLCONV GFXDLL_GetPitch( void );
DLLIMPORT extern BOOL GFXDLLCALLCONV GFXDLL_Activate( int flag );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_FlipPage( void );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_Clear( ULONG color, ULONG depth, int flag );

DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_Fog( FLOAT fognear, FLOAT fogfar, ULONG fogcolor );

DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_InitPalette( UCHAR *pal );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_SetRgb( int color, int red, int green, int blue );
DLLIMPORT extern void GFXDLLCALLCONV GFXDLL_GetRgb( int color, int *red, int *green, int *blue );
*/

#else

extern void (GFXDLLCALLCONV *GFXDRV_GetInfo)( GFXDLLinfo_t * );

extern void (GFXDLLCALLCONV *GFXDLL_SetupCulling)( point3_t pos, point3_t dir, point3_t normal, FLOAT zn, FLOAT zf );
extern BOOL (GFXDLLCALLCONV *GFXDLL_AddToPipeline)( trans2pipeline_t );
extern int (GFXDLLCALLCONV *GFXDLL_AddTexMapHigh)( texmap_t *texmap );
extern int (GFXDLLCALLCONV *GFXDLL_ReloadTexMapHigh)( texmap_t *texmap );
extern int (GFXDLLCALLCONV *GFXDLL_ModifyTexMapHigh)( texmap_t *texmap );
extern void (GFXDLLCALLCONV *GFXDLL_PutSpritePoly)( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb );
extern void (GFXDLLCALLCONV *GFXDLL_PutPoly)( polygon_t ppoly, rgb_t *rgb );
extern void (GFXDLLCALLCONV *GFXDLL_DiscardAllTexture)( void );
extern void (GFXDLLCALLCONV *GFXDLL_DiscardTexture)( int handler );
extern void (GFXDLLCALLCONV *GFXDLL_Line)( int x0, int y0, int x1, int y1, rgb_t color );
extern void (GFXDLLCALLCONV *GFXDLL_PutPixel)( int x, int y, rgb_t color );

extern void (GFXDLLCALLCONV *GFXDLL_BeginScene)( void );
extern void (GFXDLLCALLCONV *GFXDLL_EndScene)( void );
extern void (GFXDLLCALLCONV *GFXDLL_FlushScene)( void );

extern BOOL (GFXDLLCALLCONV *GFXDLL_LockLfb)( __int64 *, int flag );
extern void (GFXDLLCALLCONV *GFXDLL_UnlockLfb)( void );

extern BOOL (GFXDLLCALLCONV *GFXDLL_Init)( gfxdlldesc_t *desc );
extern void (GFXDLLCALLCONV *GFXDLL_Deinit)( void );
extern BOOL (GFXDLLCALLCONV *GFXDLL_Fullscreen)( int flag );
extern void (GFXDLLCALLCONV *GFXDLL_GetDescription)( char * );
extern void (GFXDLLCALLCONV *GFXDLL_GetData)( void *, int );
extern ULONG (GFXDLLCALLCONV *GFXDLL_GetPitch)( void );
extern BOOL (GFXDLLCALLCONV *GFXDLL_Activate)( int flag );
extern void (GFXDLLCALLCONV *GFXDLL_FlipPage)( void );
extern void (GFXDLLCALLCONV *GFXDLL_Clear)( ULONG color, ULONG depth, int flag );

extern void (GFXDLLCALLCONV *GFXDLL_Fog)( FLOAT fognear, FLOAT fogfar, ULONG fogcolor );

extern void (GFXDLLCALLCONV *GFXDLL_InitPalette)( UCHAR *pal );
extern void (GFXDLLCALLCONV *GFXDLL_SetRgb)( int color, int red, int green, int blue );
extern void (GFXDLLCALLCONV *GFXDLL_GetRgb)( int color, int *red, int *green, int *blue );


#endif


__declspec(dllexport) extern void GFXDLLCALLCONV XLIBDLL_GetInfo( XLIBDLLinfo_t * );

extern "C" char *getXlibVersion( void );


#ifdef __cplusplus
}
#endif


#endif
