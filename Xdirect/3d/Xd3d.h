
#ifndef _XD3D_H_INCLUDED
#define _XD3D_H_INCLUDED


#define DDX( x )  dprintf("%s: %d\n",#x, x );


#define HM_UNINIT	0
#define HM_TEXTURE	1
#define HM_COLOR	2

//
//
//
typedef struct hardwarestate_s {

	int texmapid;
	int mode;
	D3DCULL cullmode;

	int SCREENW,SCREENH,bpp,pitch,real_bpp;
	int orig_SCREENW,orig_SCREENH,orig_bpp;
	HWND hwnd;
	HINSTANCE hInst;

	int fogenable;
	BOOL bIsColorKey;
	BOOL bIsGouraud;
	BOOL zbuffermode;

	BOOL tex24bit;

	int mintexw,mintexh;
	int maxtexw,maxtexh;

	int x,y;
	int flag;

	BOOL bSceneBegun;
	BOOL bMipmap;

} hardwarestate_t;

extern hardwarestate_t hw_state;


//
//
//
typedef struct dd_info_s {

	DDDEVICEIDENTIFIER  DeviceInfo;
	DDDEVICEIDENTIFIER  DeviceInfoHost;

} dd_info_t, *dd_info_ptr;



//
//
//
typedef struct d3dtexture_s {

	LPDIRECT3DMATERIAL3 	lpMat;
	D3DMATERIALHANDLE 	hMat;

	int texmapid;			// Xlib texmapid

	// d3dtextr.cpp
	HBITMAP			hbmBitmap;	// Bitmap containing texture image
	LPDIRECTDRAWSURFACE4	pddsSurface;	// Surface of the texture
	LPDIRECT3DTEXTURE2	ptexTexture;	// Direct3D texture for the texture
	DWORD			dwStage;	// Texture stage (for multi-textures)
	BOOL			bHasAlpha;
	DWORD			dwFlags;
	UCHAR 			*dib;

	// BYTE			*pSurfaceBits[10];
	DDSURFACEDESC2		ddsd;

	texmap_t 		*texmap;

} d3dtexture_t, *d3dtexture_ptr;


extern "C" int drmask,dgmask,dbmask;
extern "C" int drsize,dgsize,dbsize;
extern "C" int drshift,dgshift,dbshift;


// d3dtextr.cpp

#define SAFE_RELEASE(p) { if( p ){ (p)->Release(); } (p) = NULL; }


struct TEXTURESEARCHINFO {

	BOOL dump_info;

	DWORD dwDesiredBPP; // Input for texture format search
	BOOL  bUseAlpha;
	BOOL  bUsePalette;
	BOOL  bUseFourCC;
	BOOL  bFoundGoodFormat;

	DDPIXELFORMAT* pddpf; // Result of texture format search
};



/**************************
//
//
//
class D3DTexture {

	private:
		IDirectDrawSurface *MemorySurface;  // system memory surface
		IDirectDrawSurface *DeviceSurface;  // video memory texture
		IDirectDrawPalette *Palette;

	public:
		D3DTexture() {
			MemorySurface = 0;
			DeviceSurface = 0;
			Palette = 0;
			TextureHandle = 0;
			hMat = 0;
			lpMat = NULL;
			texmapid = -1;
		}

		D3DTEXTUREHANDLE TextureHandle;
		D3DMATERIALHANDLE hMat;
		LPDIRECT3DMATERIAL3 lpMat;
		int texmapid;

		D3DTEXTUREHANDLE    GetHandle()  { return TextureHandle; }
		IDirectDrawSurface *GetSurface() { return MemorySurface; }
		IDirectDrawSurface *GetDeviceSurface() { return DeviceSurface; }
		IDirectDrawPalette *GetPalette() { return Palette; }

		BOOL Load(IDirect3DDevice3 *Device, UCHAR *spr );
		BOOL Copy(HBITMAP Bitmap);
		void Release(void);
		BOOL Restore(void);
};
***********************/

/*
extern int rmask,gmask,bmask;
extern int rsize,gsize,bsize;
extern int rshift,gshift,bshift;
*/
extern char d3dName[256];


extern char *D3D_Error( HRESULT rval );

extern "C" void dllQuit( char *s, ... );
extern "C" void (*dprintf)( char *s, ... );
extern "C" texmap_t * (*dTexForNum)( int texmapid );
extern "C" ULONG (*dGetBackHwnd)( void );
extern "C" BOOL (*dwinQuit)( char *s, ... );
extern "C" void *(*dmalloc)( int );
extern "C" int (*dfree)( void * );

extern "C" int dll_init( void );
extern "C" void dll_deinit( void );


static BOOL CheckCaps( GUID *lpGUID );
static void D3D_CreateDevice( void );


extern DWORD DDColorMatch( IDirectDrawSurface *pdds, COLORREF rgb );
// extern HRESULT DDSetColorKey( LPDIRECTDRAWSURFACE4 pdds, COLORREF rgb );




extern int D3D_winComboBox( combobox_t *combobox_data );
extern void D3D_ClearLogWindow( void );
extern void D3D_AddLogWindow( char *str );

#define MAX_DESC 20
extern DDCAPS *D3D_GetCaps( void );
extern D3DDEVICEDESC *D3D_GetDesc( void );

extern DDCAPS *global_ddcaps;
extern D3DDEVICEDESC *global_mydesc;

extern DDCAPS *global_dd_info;
extern dd_info_t *D3D_GetDdinfo( void );

extern BOOL TextrRestore( int num );


#include "d3dtextr.h"

#endif