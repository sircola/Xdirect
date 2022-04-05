/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif

#define FLOAT_ONE ( 1.0f )

extern BOOL LoadSdl( void );
extern void FreeSdl( void );


extern void FillConvex( polygon_ptr VertexList, polygon3_t *tpoly, ULONG Color );
extern void SW_DrawTexturedFaceHigh( polygon_ptr poly, point_ptr texvert, texmap_t *texmap, polygon3_ptr tpoly, int face_flag, rgb_t *rgb );
extern void SW_DrawTexturedFace( polygon_ptr poly, point_ptr texvert, texmap_t *texmap );
extern void SW_FillFaceHigh( polygon_ptr poly, USHORT color, polygon3_ptr tpoly, int face_flag, rgb_t *rgb  );
extern void *SW_GetVideoPtr( int x, int y );

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

	int texmap;
	int mode;
	int SCREENW,SCREENH,bpp,pixel_len;
	int CLIPMINX,CLIPMINY;
	int CLIPMAXX,CLIPMAXY;
	HWND hwnd;
	
	BOOL bZbuffer;
	FLOAT *zbuffer;

	BOOL bBlend;
	HINSTANCE hInst;

} hardwarestate_t;

#define HM_UNINIT	0
#define HM_TEXTURE	1
#define HM_COLOR	2


extern hardwarestate_t hw_state;

extern FLOAT sf_zclipnear;
extern FLOAT sf_zclipnearP2;
extern FLOAT sf_zclipfar;
extern FLOAT sf_zclipfarP2;
extern FLOAT fzclipfar;
extern int sf_perdiv;
// extern FLOAT fognear,fogfar,invfog;




#ifdef __cplusplus
}
#endif


