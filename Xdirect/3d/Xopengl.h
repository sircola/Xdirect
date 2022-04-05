/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif


extern void dllQuit( char *s, ... );

extern int dll_init( void );
extern void dll_deinit( void );

extern void (*dprintf)( char *s, ... );
extern texmap_t *(*dTexForNum)( int texmapid );
extern "C" ULONG (*dGetBackHwnd)( void );
extern "C" BOOL (*dwinQuit)( char *s, ... );

// extern FIXED ffognear,ffogfar;


typedef struct hardwarestate_s {

	BOOL tex24bit;

	int texmapid, mode;
	int zbuffermode;
	int cullmode;
	int SCREENW,SCREENH,bpp,pixel_len;
	int CLIPMINX,CLIPMINY;
	int CLIPMAXX,CLIPMAXY;
	HWND hwnd;
	HDC hdc;
	HGLRC hRC;

	BOOL envcombine4;
	BOOL texture_edge_clamp;
	int multitex;
	int tex_size;
	int mintex_size;
	HINSTANCE hInst;

	int x,y;
	int flag;
	int orig_SCREENW,orig_SCREENH,orig_bpp;

} hardwarestate_t;

#define HM_UNINIT	0
#define HM_TEXTURE	1
#define HM_COLOR	2


extern hardwarestate_t hw_state;


#define MAXTEXTURE 1000

typedef struct opengltexture_s {

	GLuint texmapid;

	UCHAR *sprite;

} opengltexture_t, *opengltexture_ptr;



extern int drmask,dgmask,dbmask;
extern int drsize,dgsize,dbsize;
extern int drshift,dgshift,dbshift;



#ifdef __cplusplus
}
#endif


