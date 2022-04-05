
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)


#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <windowsx.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <xlib.h>
#include <xinner.h>

#include "dllmain.h"

RCSID( "$Id: Soft.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )


#define GFXDLL_VERSION "1.0"
char *GFXDLL_NAME = "xsoft.drv";

#pragma comment( exestr , "Software-Only driver v" GFXDLL_VERSION " (" __DATE__ " " __TIME__ ")")


#include "xsoft.h"

void (*dprintf)( char *s, ... ) = NULL;
texmap_t *(*dTexForNum)( int texmapid ) = NULL;
void (*dCopyVector)( point3_t dst, point3_t src ) = NULL;
int (*SW_winReadProfileInt)( char *, int ) = NULL;
void (*SW_winWriteProfileInt)( char *, int ) = NULL;
ULONG (*dGetBackHwnd)( void ) = NULL;
int (*dfree)( void * ) = NULL;


int dactive_flag = 1;

static LPBITMAPINFO pBMI;
static HBITMAP hBmp;
static void *pDIBMemory = NULL;


// options


static int nswtextures = 0;


hardwarestate_t hw_state;

int drmask,dgmask,dbmask;
int drsize,dgsize,dbsize;
int drshift,dgshift,dbshift;


static pipeline_t *pipeline = NULL;
static int pipelinelen = 0;			// hany face van most rajta
static int allocedpipelinelen = 0;		// hanynak van hely



#define REALLOCPIPELINE( i ) { \
	if( (pipelinelen + (i)) > allocedpipelinelen ) { \
		allocedpipelinelen = pipelinelen + (100); \
		pipeline = (pipeline_t*)drealloc( pipeline, allocedpipelinelen * sizeof(pipeline_t) ); } \
}



static point3_t vpos,		// viewer's position
		vdir,		//  Ä´ÃÄ    direction
		vnormal,	//  Ä´ÃÄ    normal = vpos - vdir;
		mvnormal;	// mvnormal = dir - pos;


FLOAT sf_zclipnear = FLOAT_EPSILON;
FLOAT sf_zclipnearP2 = FLOAT_EPSILON;
FLOAT sf_zclipfar = FLOAT_MAX;
FLOAT sf_zclipfarP2 = FLOAT_MAX;
FLOAT fzclipfar = FLOAT_MAX;
int sf_perdiv = 16;

FLOAT dfognear,dfogfar,dinvfog;
// point3_t origo3 = { 0.0f, 0.0f, 0.0f };


BOOL (*dwinFullscreen)( void ) = NULL;


//
//
//
static BOOL GFXDLLCALLCONV SW_LockLfb( __int64 *mem, int flag ) {

	USEPARAM( flag );

	*mem = (__int64)pDIBMemory;

	return TRUE;
}





//
//
//
static void GFXDLLCALLCONV SW_UnlockLfb( void ) {

	return;
}






//
//
//
static void GFXDLLCALLCONV SW_SetupCulling( point3_t pos, point3_t dir, point3_t normal, FLOAT zn, FLOAT zf ) {

	/***
	CopyVector( vpos, pos );
	CopyVector( vdir, dir );

	SubVector( pos, dir, vnormal );
	NormalizeVector( vnormal );

	SubVector( dir, pos, mvnormal );
	NormalizeVector( mvnormal );
	***/

	if( pos ) dCopyVector( vpos, pos );
	if( dir ) dCopyVector( vdir, dir );
	if( normal ) dCopyVector( vnormal, normal );

	sf_zclipnear = zn;
	sf_zclipnearP2 = zn*zn;
	sf_zclipfar = zf;
	sf_zclipfarP2 = zf*zf;
	fzclipfar = zf;

	return;
}






//
//
//
static BOOL GFXDLLCALLCONV SW_Rasterize( trans2pipeline_t trans2pipeline ) {
//polygon_t ppoly, polygon3_t poly, polygon3_t tpoly, point3_t *pointnormal,
//				point_t *outtex, point3_t *inst, face_t *f, int *rgb ) {

#define P(tag) trans2pipeline.tag

	int k;
	int texmapid = -1;
	texmap_t *texmap = NULL;
	rgb_t *outrgb = NULL;
	point3_t *outst = NULL;

	if( P(faceid) == 0 ) {
		texmapid = ISFLAG( P(face)->flag, FF_TEXTURE ) ? P(face)->texmapid : (-1);
		outrgb = P(outrgb);
		outst = P(outst);
	}
	else
	if( P(faceid) == 1 ) {
		texmapid = P(face)->texmapid1;
		outrgb = P(outrgb1);
		outst = P(outst1);
	}
	else
	if( P(faceid) == 2 ) {
		texmapid = P(face)->texmapid2;
	}

	for( k=1; k<P(ppoly).npoints-1; k++ ) {

		if( ( texmapid != (-1) ) && ((texmap = dTexForNum(texmapid)) == NULL) ) {
			dprintf("SW_Rasterize: can't find %d texmapid.\n",P(face)->texmapid);
			continue;
		}

		REALLOCPIPELINE( 1 );

		pipeline[ pipelinelen ].texmap = texmap;

		// transformed center

		pipeline[ pipelinelen ].tcenter[0] = P(face)->tcenter[0];
		pipeline[ pipelinelen ].tcenter[1] = P(face)->tcenter[1];
		pipeline[ pipelinelen ].tcenter[2] = P(face)->tcenter[2];

		// screen poly

		pipeline[ pipelinelen ].triangle.point[0][0] = P(ppoly).point[0][0];
		pipeline[ pipelinelen ].triangle.point[0][1] = P(ppoly).point[0][1];

		pipeline[ pipelinelen ].triangle.point[1][0] = P(ppoly).point[k][0];
		pipeline[ pipelinelen ].triangle.point[1][1] = P(ppoly).point[k][1];

		pipeline[ pipelinelen ].triangle.point[2][0] = P(ppoly).point[k+1][0];
		pipeline[ pipelinelen ].triangle.point[2][1] = P(ppoly).point[k+1][1];


		// transformed poly

		pipeline[ pipelinelen ].ttriangle.point[0][0] = P(tpoly).point[0][0];
		pipeline[ pipelinelen ].ttriangle.point[0][1] = P(tpoly).point[0][1];
		pipeline[ pipelinelen ].ttriangle.point[0][2] = P(tpoly).point[0][2];

		pipeline[ pipelinelen ].ttriangle.point[1][0] = P(tpoly).point[k][0];
		pipeline[ pipelinelen ].ttriangle.point[1][1] = P(tpoly).point[k][1];
		pipeline[ pipelinelen ].ttriangle.point[1][2] = P(tpoly).point[k][2];

		pipeline[ pipelinelen ].ttriangle.point[2][0] = P(tpoly).point[k+1][0];
		pipeline[ pipelinelen ].ttriangle.point[2][1] = P(tpoly).point[k+1][1];
		pipeline[ pipelinelen ].ttriangle.point[2][2] = P(tpoly).point[k+1][2];

		memcpy( &pipeline[ pipelinelen ].rgb[0], &outrgb[0], sizeof(rgb_t) );
		memcpy( &pipeline[ pipelinelen ].rgb[1], &outrgb[k], sizeof(rgb_t) );
		memcpy( &pipeline[ pipelinelen ].rgb[2], &outrgb[k+1], sizeof(rgb_t) );

		pipeline[ pipelinelen ].face_flag = P(face)->flag;

		// color
		if( ISFLAG( P(face)->flag, FF_COLOR ) )
			pipeline[ pipelinelen ].color = P(face)->color;


		// texture
		if( texmapid != -1 ) {

			FLOAT invW,invH;

			ERASEFLAG( pipeline[ pipelinelen ].face_flag, FF_COLOR );
			SETFLAG( pipeline[ pipelinelen ].face_flag, FF_TEXTURE );

			invW = texmap->width1 / 256.0f;
			invH = texmap->height1 / 256.0f;

			pipeline[ pipelinelen ].texvert[0][0] = (int)(outst[0][0] * invW);
			pipeline[ pipelinelen ].texvert[0][1] = (int)(outst[0][1] * invH);

			pipeline[ pipelinelen ].texvert[1][0] = (int)(outst[k][0] * invW);
			pipeline[ pipelinelen ].texvert[1][1] = (int)(outst[k][1] * invH);

			pipeline[ pipelinelen ].texvert[2][0] = (int)(outst[k+1][0] * invW);
			pipeline[ pipelinelen ].texvert[2][1] = (int)(outst[k+1][1] * invH);
		}

		++pipelinelen;
	}

#undef P

	return TRUE;
}



//
//
//
static BOOL GFXDLLCALLCONV SW_AddToPipeline( trans2pipeline_t trans2pipeline ) {

#define P(tag) trans2pipeline.tag

	P(faceid) = 0;
	SW_Rasterize( trans2pipeline );

	if( ISFLAG( P(face)->flag, FF_ENVMAP) ) {
		int i;
		P(faceid) = 1;
		for( i=0; i<P(ppoly).npoints; i++ ) {
			P(outrgb1)[i].r = P(outrgb)[i].r;
			P(outrgb1)[i].g = P(outrgb)[i].g;
			P(outrgb1)[i].b = P(outrgb)[i].b;
		}
		SW_Rasterize( trans2pipeline );
	}

#undef P

	return TRUE;
}






//
//
//
static void GFXDLLCALLCONV SW_PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) {

	int k;
	texmap_t *texmap;
	pipeline_t pipeline;
	FLOAT invW,invH;

	if( (texmap = dTexForNum(texmapid)) == NULL  ) {
		dprintf("SW_PutSpritePoly: can't find %d texmapid.\n",texmapid);
		return;
	}

	for( k=1; k<ppoly.npoints-1; k++ ) {

		invW = texmap->width1 / 256.0f;
		invH = texmap->height1 / 256.0f;

		// screen poly

		pipeline.triangle.point[0][0] = ppoly.point[0][0];
		pipeline.triangle.point[0][1] = ppoly.point[0][1];

		pipeline.triangle.point[1][0] = ppoly.point[k][0];
		pipeline.triangle.point[1][1] = ppoly.point[k][1];

		pipeline.triangle.point[2][0] = ppoly.point[k+1][0];
		pipeline.triangle.point[2][1] = ppoly.point[k+1][1];

		// transformed poly

		pipeline.ttriangle.point[0][0] = 0.0f;
		pipeline.ttriangle.point[0][1] = 0.0f;
		pipeline.ttriangle.point[0][2] = 0.0f;

		pipeline.ttriangle.point[1][0] = 0.0f;
		pipeline.ttriangle.point[1][1] = 0.0f;
		pipeline.ttriangle.point[1][2] = 0.0f;

		pipeline.ttriangle.point[2][0] = 0.0f;
		pipeline.ttriangle.point[2][1] = 0.0f;
		pipeline.ttriangle.point[2][2] = 0.0f;

		memcpy( &pipeline.rgb[0], &rgb[0], sizeof(rgb_t) );
		memcpy( &pipeline.rgb[1], &rgb[k], sizeof(rgb_t) );
		memcpy( &pipeline.rgb[2], &rgb[k+1], sizeof(rgb_t) );

		pipeline.face_flag = 0;
		SETFLAG( pipeline.face_flag, FF_TEXTURE );
		SETFLAG( pipeline.face_flag, FF_FARTHEST );

		pipeline.texvert[0][0] = (int)( inst[0][0] * invW );
		pipeline.texvert[0][1] = (int)( inst[0][1] * invH );

		pipeline.texvert[1][0] = (int)( inst[k][0] * invW );
		pipeline.texvert[1][1] = (int)( inst[k][1] * invH );

		pipeline.texvert[2][0] = (int)( inst[k+1][0] * invW );
		pipeline.texvert[2][1] = (int)( inst[k+1][1] * invH );

		polygon_t spoly;
		polygon3_t tpoly;

		spoly.npoints = 3;
		spoly.point = pipeline.triangle.point;

		tpoly.npoints = 3;
		tpoly.point = pipeline.ttriangle.point;

		if( hw_state.bpp > 8 )
			SW_DrawTexturedFaceHigh( &spoly, pipeline.texvert, texmap, &tpoly, pipeline.face_flag, pipeline.rgb );
		else {
			static BOOL sema = FALSE;
			if( sema == FALSE )
				dprintf( "SW_PutSpritePoly: works only if bigger than 8 bpp for now. (%d)\n", hw_state.bpp )
			; // SW_DrawTexturedFace( &spoly, P.texvert, P.texmap );
		}
	}

	return;
}



//
//
//
static void GFXDLLCALLCONV SW_PutPoly( polygon_t ppoly, rgb_t *rgb ) {

	int k;
	int cnt = MAX( ppoly.npoints-1, 3-1 );		// FIXME

	for( k=1; k<cnt; k++ ) {

		REALLOCPIPELINE( 1 );

		pipeline[ pipelinelen ].texmap = NULL;

		// transformed center

		pipeline[ pipelinelen ].tcenter[0] = 0.0f;
		pipeline[ pipelinelen ].tcenter[1] = 0.0f;
		pipeline[ pipelinelen ].tcenter[2] = 0.0f;

		// screen poly

		pipeline[ pipelinelen ].triangle.point[0][0] = ppoly.point[0][0];
		pipeline[ pipelinelen ].triangle.point[0][1] = ppoly.point[0][1];

		pipeline[ pipelinelen ].triangle.point[1][0] = ppoly.point[k][0];
		pipeline[ pipelinelen ].triangle.point[1][1] = ppoly.point[k][1];

		pipeline[ pipelinelen ].triangle.point[2][0] = ppoly.point[k+1][0];
		pipeline[ pipelinelen ].triangle.point[2][1] = ppoly.point[k+1][1];

		// transformed poly

		pipeline[ pipelinelen ].ttriangle.point[0][0] = 0.0f;
		pipeline[ pipelinelen ].ttriangle.point[0][1] = 0.0f;
		pipeline[ pipelinelen ].ttriangle.point[0][2] = 0.0f;

		pipeline[ pipelinelen ].ttriangle.point[1][0] = 0.0f;
		pipeline[ pipelinelen ].ttriangle.point[1][1] = 0.0f;
		pipeline[ pipelinelen ].ttriangle.point[1][2] = 0.0f;

		pipeline[ pipelinelen ].ttriangle.point[2][0] = 0.0f;
		pipeline[ pipelinelen ].ttriangle.point[2][1] = 0.0f;
		pipeline[ pipelinelen ].ttriangle.point[2][2] = 0.0f;

		memcpy( &pipeline[ pipelinelen ].rgb[0], &rgb[0], sizeof(rgb_t) );
		memcpy( &pipeline[ pipelinelen ].rgb[1], &rgb[k], sizeof(rgb_t) );
		memcpy( &pipeline[ pipelinelen ].rgb[2], &rgb[k+1], sizeof(rgb_t) );

		pipeline[ pipelinelen ].face_flag = 0;
		SETFLAG( pipeline[ pipelinelen ].face_flag, FF_COLOR );
		ERASEFLAG( pipeline[ pipelinelen ].face_flag, FF_TEXTURE );
		SETFLAG( pipeline[ pipelinelen ].face_flag, FF_FARTHEST );

		++pipelinelen;
	}

	return;
}





//
// only HIGH sprite
//
static int GFXDLLCALLCONV SW_AddTexMapHigh( texmap_t *texmap ) {

	++nswtextures;

	return texmap->texmapid;
}




//
//
//
static int GFXDLLCALLCONV SW_ReloadTexMapHigh( texmap_t *texmap ) {


	return texmap->texmapid;
}


//
//
//
static int GFXDLLCALLCONV SW_ModifyTexMapHigh( texmap_t *texmap ) {


	return texmap->texmapid;
}



//
//
//
static void GFXDLLCALLCONV SW_DiscardAllTexture( void ) {

	nswtextures = 0;

	return;
}


//
//
//
static void GFXDLLCALLCONV SW_DiscardTexture( int handler ) {

	--nswtextures;

	return;
}



//
//
//
static void GFXDLLCALLCONV SW_Line( int x0, int y0, int x1, int y1, rgb_t rgb ) {

	int udx, udy, dx, dy, error, loop, xadd, yadd;

	dx = x1 - x0;	   	//	Work out x delta
	dy = y1 - y0;	   	//	Work out y delta

	udx = abs( dx );	//	udx is the unsigned x delta
	udy = abs( dy );	//	udy is the unsigned y delta

	if ( dx < 0 )		//	Work out direction to step in the x direction.
		xadd = -1;
	else
		xadd = 1;

	if ( dy < 0 )		//	Work out direction to step in the y direction.
		yadd = -1;
	else
		yadd = 1;

	error = 0;
	loop = 0;
	USHORT color = 	RGBINT( rgb.r, rgb.g, rgb.b );
	USHORT *video;

	if( udx > udy ) {
		do {						//	Delta X > Delta Y

			error += udy;

			if( error >= udx ) {	//	Time to move up / down?

				error -= udx;

				y0 += yadd;
			}

			loop++;

			if( (video = (USHORT *)SW_GetVideoPtr( x0, y0 )) != NULL )
				*video = color;	// Plot pixel - d_buffer is my screen buffer.

			x0 += xadd;			// Move horizontally.

		} while( loop < udx );	// Repeat for x length of line.
	}
	else{
		do {						// Delta Y > Delta X

			error += udx;

			if( error >= udy ) {	// Time to move left / right?
				error -= udy;

				x0 += xadd;		// Move across.
			}

			loop++;

			if( (video = (USHORT *)SW_GetVideoPtr( x0, y0 )) != NULL )
				*video = color;	// Plot pixel - d_buffer is my screen buffer.

			y0 += yadd;	// Move up / down a row.

		} while( loop < udy );	// Repeat for y length of line.
	}

	return;
}



//
//
//
static void GFXDLLCALLCONV SW_PutPixel( int x, int y, rgb_t color ) {

	USHORT *video;

	if( (video = (USHORT *)SW_GetVideoPtr( x, y )) != NULL )
		*video = RGBINT( color.r, color.g, color.b );

	return;
}


//
//
//
void *SW_GetVideoPtr( int x, int y ) {

	//dprintf("%d %d\n",x,y);
	if( x < 0 || x >= hw_state.SCREENW || y < 0 || y >= hw_state.SCREENH )
		return NULL;

	return &((UCHAR*)pDIBMemory)[ y * (hw_state.SCREENW * hw_state.pixel_len) + (x * hw_state.pixel_len) ];
}




//
//
//
static void GFXDLLCALLCONV SW_BeginScene( void ) {

	pipelinelen = 0;

	return;
}




//
//
//
static int pipe_cmp( const void *a, const void *b ) {

	if( ISFLAG( ((pipeline_ptr)a)->face_flag, FF_FARTHEST ) )
		return -1;

	if( ISFLAG( ((pipeline_ptr)b)->face_flag, FF_FARTHEST ) )
		return 1;

	return (int)( ((pipeline_ptr)b)->tcenter[2] - ((pipeline_ptr)a)->tcenter[2] );
}




//
//
//
static void GFXDLLCALLCONV SW_EndScene( void ) {

	int i;
	polygon_t spoly;
	polygon3_t tpoly;
	static int c = 0;

	if( hw_state.bZbuffer == FALSE )
		qsort( pipeline, pipelinelen, sizeof(pipeline_t), pipe_cmp);

#define P pipeline[i]

	// dprintf("len: %d\n",pipelinelen);

	for( i=0; i<pipelinelen; i++ ) {

		spoly.npoints = 3;
		spoly.point = P.triangle.point;

		tpoly.npoints = 3;
		tpoly.point = P.ttriangle.point;

		if( ISFLAG( P.face_flag, FF_TEXTURE ) ) {
			if( hw_state.bpp > 8 )
				SW_DrawTexturedFaceHigh( &spoly, P.texvert, P.texmap, &tpoly, P.face_flag, P.rgb );
			else
				; // SW_DrawTexturedFace( &spoly, P.texvert, P.texmap );
		}

		if( ISFLAG( P.face_flag, FF_COLOR ) )
			SW_FillFaceHigh( &spoly, (USHORT)P.color, &tpoly, P.face_flag, P.rgb );
	}

#undef P

	pipelinelen = 0;

	return;
}




//
//
//
static void GFXDLLCALLCONV SW_FlushScene( void ) {

	SW_EndScene();
	SW_BeginScene();

	return;
}




//
//
//
static void GFXDLLCALLCONV SW_Fog( FLOAT fn, FLOAT ff, ULONG fogcolor ) {

	dfognear = fn;
	dfogfar = ff;

	dinvfog = 255.0f / (dfogfar - dfognear);

	return;
}







//
//
//
static BOOL GFXDLLCALLCONV SW_Activate( int flag ) {

	if( (dactive_flag = flag) ) {

	}

	return FALSE;
}





//
//
//
static void GFXDLLCALLCONV SW_InitPalette( UCHAR *pal ) {

	PALETTEENTRY lpColorTable[256];
	int i;

	for( i=0; i<256; i++ ) {

		lpColorTable[i].peRed	= ( (pal[(i*3)+0]+1) << 2 ) - 1;
		lpColorTable[i].peGreen = ( (pal[(i*3)+1]+1) << 2 ) - 1;
		lpColorTable[i].peBlue	= ( (pal[(i*3)+2]+1) << 2 ) - 1;
		lpColorTable[i].peFlags = 0;
	}

	return;
}






//
//
//
static void GFXDLLCALLCONV SW_SetRgb( int color, int red, int green, int blue ) {

	PALETTEENTRY rgb;

	rgb.peRed   = ( (red+1)   << 2 ) - 1;
	rgb.peGreen = ( (green+1) << 2 ) - 1;
	rgb.peBlue  = ( (blue+1)  << 2 ) - 1;
	rgb.peFlags = 0;

	return;
}






//
//
//
static void GFXDLLCALLCONV SW_GetRgb( int color, int *red, int *green, int *blue ) {

	PALETTEENTRY rgb = {0,0,0,0};

	*red	= ( ((short)rgb.peRed+1)   / 4) - 1;
	*green	= ( ((short)rgb.peGreen+1) / 4) - 1;
	*blue	= ( ((short)rgb.peBlue+1)  / 4) - 1;

	return;
}







//
//
//
static ULONG GFXDLLCALLCONV SW_GetPitch( void ) {

	return (ULONG)( hw_state.SCREENW * hw_state.pixel_len );
}









//
//
//
static void GFXDLLCALLCONV SW_FlipPage( void ) {

	HDC dstDC = GetDC( (HWND)hw_state.hwnd );
	HDC srcDC = CreateCompatibleDC( dstDC );
	HBITMAP hBitmap = (HBITMAP)SelectObject( srcDC, hBmp );

	if( dwinFullscreen() )
		BitBlt( dstDC, (GetDeviceCaps( dstDC, HORZRES ) - hw_state.SCREENW) / 2, (GetDeviceCaps( dstDC, VERTRES ) - hw_state.SCREENH) / 2, hw_state.SCREENW, hw_state.SCREENH, srcDC, 0, 0, SRCCOPY );
	else
		// BitBlt( dstDC, (GetSystemMetrics( SM_CXSCREEN ) - hw_state.SCREENW) / 2, (GetSystemMetrics( SM_CYSCREEN ) - hw_state.SCREENH) / 2, hw_state.SCREENW, hw_state.SCREENH, srcDC, 0, 0, SRCCOPY );
		BitBlt( dstDC, 0, 0, hw_state.SCREENW, hw_state.SCREENH, srcDC, 0, 0, SRCCOPY );

	DeleteObject( hBitmap );
	DeleteDC( srcDC );
	ReleaseDC( NULL, dstDC );

	return;
}






//
//
//
static void GFXDLLCALLCONV SW_Clear( ULONG color, ULONG depth, int flag ) {

	FLOAT *zbuffer;
	int len;

	zbuffer = hw_state.zbuffer;
	len = hw_state.SCREENW * hw_state.SCREENH;

	if( flag & (CF_TARGET | CF_ZBUFFER) ) {

		if( hw_state.pixel_len == 2 ) {

			USHORT *video = (USHORT *)pDIBMemory;
			USHORT clear_color = (USHORT)RGB24( color );

			if( hw_state.bZbuffer == TRUE )
				while( len-- ) {
					*zbuffer++ = FLOAT_MAX;
					*video++ = clear_color;
				}
			else
				while( len-- )
					*video++ = clear_color;
		}
		else
			memset( pDIBMemory, color, (len * hw_state.pixel_len) );
	}
	else
	if( flag & CF_ZBUFFER ) {

		if( hw_state.bZbuffer == TRUE )
			while( len-- ) *zbuffer++ = FLOAT_MAX;
	}
	else
	if( flag & CF_TARGET ) {

		if( hw_state.pixel_len == 2 ) {

			USHORT *video = (USHORT *)pDIBMemory;
			USHORT clear_color = (USHORT)RGB24( color );

			while( len-- )
				*video++ = clear_color;
		}
		else
			memset( pDIBMemory, color, (len * hw_state.pixel_len) );
	}

	// memset( hw_state.zbuffer, FIXED_MAX, (hw_state.SCREENW * hw_state.SCREENH * sizeof(FIXED) ) );

	return;
}








//
//
//
static void GFXDLLCALLCONV SW_SetClip( int cmx, int cmy, int cxx, int cxy ) {

	hw_state.CLIPMINX = cmx;
	hw_state.CLIPMINY = cmy;
	hw_state.CLIPMAXX = cxx;
	hw_state.CLIPMAXY = cxy;

	return;
}







//
//
//
static BOOL GFXDLLCALLCONV SW_Init( gfxdlldesc_t *desc ) {

	HDC hdc;
	PIXELFORMATDESCRIPTOR pfd;

	desc->bpp = 16;

	hw_state.hwnd = (__int64)desc->hwnd;
	hw_state.SCREENW = desc->width;
	hw_state.SCREENH = desc->height;
	hw_state.bpp = ABS(desc->bpp);
	hw_state.pixel_len = ABS(desc->bpp)/8;
	hw_state.texmap = -1;
	hw_state.mode = HM_TEXTURE;
	hw_state.hInst = (__int64)desc->hInst;
	hw_state.tex24bit = FALSE;
	hw_state.x = desc->x;
	hw_state.y = desc->y;
	hw_state.flag = desc->flag;

	dprintf = (void (GFXDLLCALLCONV *)(char *s,...))desc->xprintf;
	dmalloc = (void *(GFXDLLCALLCONV *)(int))desc->malloc;
	drealloc = (void *(GFXDLLCALLCONV *)(void *,int))desc->realloc;
	dfree = (int (GFXDLLCALLCONV *)(void *))desc->free;
	dwinQuit = (BOOL (GFXDLLCALLCONV *)(char *s,...))desc->Quit;
	dGetBackHwnd = (ULONG (GFXDLLCALLCONV *)(void))desc->GetBackHwnd;
	dTexForNum = (texmap_t *(GFXDLLCALLCONV *)(int texmapid))desc->TexForNum;
	dCopyVector = (void (GFXDLLCALLCONV *)(point3_t dst, point3_t src))desc->CopyVector;
	dwinFullscreen = (BOOL (GFXDLLCALLCONV *)(void))desc->winFullscreen;
	SW_winReadProfileInt = (int (GFXDLLCALLCONV *)(char*,int))desc->winReadProfileInt;
	SW_winWriteProfileInt = (void (GFXDLLCALLCONV *)(char*,int))desc->winWriteProfileInt;

	dprintf("init: %s v%s (c) Copyright 1998 bernie (%s %s)\n",GFXDLL_NAME,GFXDLL_VERSION,__DATE__,__TIME__);

	// if( MessageBox( (HWND)hw_state.hwnd, "The Software-Renderer is NOT supported.\nDo you still want to continue?", "Warning", MB_DEFBUTTON1 | MB_YESNO | MB_ICONQUESTION) == IDNO ) dllQuit(NULL);


	hdc = GetDC( (HWND)hw_state.hwnd );

	/***
	hw_state.bpp = GetDeviceCaps( hdc, BITSPIXEL );
	ReleaseDC( NULL, hdc );

	if( hw_state.bpp != desc->bpp ) {
		dprintf("bpp: %d != %d.\n",hw_state.bpp,desc->bpp);
		return FALSE;
	}
	***/

	memset( &pfd, 0L, sizeof(PIXELFORMATDESCRIPTOR) );
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	DescribePixelFormat( hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd );

	ReleaseDC( (HWND)hw_state.hwnd, hdc );

	if( (pfd.iPixelType == PFD_TYPE_RGBA) && (pfd.cColorBits == hw_state.bpp) ) {

		drshift = desc->rshift = pfd.cRedShift;
		dgshift = desc->gshift = pfd.cGreenShift;
		dbshift = desc->bshift = pfd.cBlueShift;

		drmask = desc->rmask = ( 1 << pfd.cRedBits ) - 1;
		dgmask = desc->gmask = ( 1 << pfd.cGreenBits ) - 1;
		dbmask = desc->bmask = ( 1 << pfd.cBlueBits ) - 1;

		drsize = desc->rsize = pfd.cRedBits;
		dgsize = desc->gsize = pfd.cGreenBits;
		dbsize = desc->bsize = pfd.cBlueBits;

		dprintf("log: %d bits desktop, %d:%d:%d (%d:%d:%d)\n",pfd.cColorBits,drsize,dgsize,dbsize,drshift,dgshift,dbshift);
	}
	else {

		drshift = desc->rshift = 11;
		dgshift = desc->gshift = 5;
		dbshift = desc->bshift = 0;

		drmask = desc->rmask = 0x1f;
		dgmask = desc->gmask = 0x3f;
		dbmask = desc->bmask = 0x1f;

		drsize = desc->rsize = 5;
		dgsize = desc->gsize = 6;
		dbsize = desc->bsize = 5;
	}

	if( (pBMI = (LPBITMAPINFO)dmalloc( sizeof(BITMAPINFOHEADER)+(256*sizeof(RGBQUAD)) )) == NULL ) {
		dprintf("SW_Init: allocation of bitmap header failed.\n");
		return FALSE;
	}

	pBMI->bmiHeader.biSizeImage	= 0;
	pBMI->bmiHeader.biXPelsPerMeter = 0;
	pBMI->bmiHeader.biYPelsPerMeter = 0;
	pBMI->bmiHeader.biClrUsed	= 0;
	pBMI->bmiHeader.biClrImportant	= 0;

	pBMI->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	pBMI->bmiHeader.biWidth 	= hw_state.SCREENW;
	pBMI->bmiHeader.biHeight	= -hw_state.SCREENH;
	pBMI->bmiHeader.biPlanes	= 1;

	pBMI->bmiHeader.biCompression	= BI_BITFIELDS;
	pBMI->bmiHeader.biBitCount	= 16;
	MEMCAST( DWORD, pBMI->bmiColors[0] ) = (drmask << drshift);
	MEMCAST( DWORD, pBMI->bmiColors[1] ) = (dgmask << dgshift);
	MEMCAST( DWORD, pBMI->bmiColors[2] ) = (dbmask << dbshift);

	hdc = GetDC( (HWND)hw_state.hwnd );
	hBmp = CreateDIBSection( hdc, pBMI, DIB_RGB_COLORS, &pDIBMemory, NULL, 0 );
	ReleaseDC( NULL, hdc );

	if( !hBmp || !pDIBMemory ) {
		dfree( pBMI );
		pBMI = NULL;
		dprintf("SW_Init: CreateDIBSection phailed.\n");
		return FALSE;
	}

	desc->bZBuffer = FALSE;

	hw_state.bZbuffer = BOOLEAN(SW_winReadProfileInt("sw_zbufferenable", 1));
	SW_winWriteProfileInt( "sw_zbufferenable", hw_state.bZbuffer );

	hw_state.bZbuffer = TRUE;
	desc->bZBuffer = hw_state.bZbuffer;

	if( hw_state.bZbuffer == TRUE ) {
		if( (hw_state.zbuffer = (FLOAT *)dmalloc(hw_state.SCREENW * hw_state.SCREENH * sizeof(FLOAT))) == NULL ) {
			hw_state.bZbuffer = FALSE;
			dprintf("SW_Init: Z Buffer allocation failed.\n");
		}
	}

	SW_SetClip( 0,0, desc->width-1,desc->height-1 );

	nswtextures = 0;

	hw_state.bBlend = BOOLEAN(SW_winReadProfileInt("sw_blendenable", 1));
	SW_winWriteProfileInt( "sw_blendenable", hw_state.bBlend );


	// dibdetect();

	if( dwinFullscreen() ) {
		if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
			dprintf("SW_Init: fullscreen failed, retrying whatever you have.\n");
			ChangeResolution( hw_state.SCREENW, hw_state.SCREENH );
		}
		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, 0,0, hw_state.SCREENW, hw_state.SCREENH, TRUE );
	}
	else {
		RECT rc;
		DWORD dwStyle;
		WINDOWINFO wi;

		GetWindowInfo( (HWND)hw_state.hwnd, &wi );

		dwStyle = wi.dwStyle;

		rc.left = desc->x;
		rc.top = desc->y;
		rc.right = rc.left + hw_state.SCREENW;
		rc.bottom = rc.top + hw_state.SCREENH;

		// dprintf( "movewindow ORIG: %d,%d, %d,%d\n", rc.left, rc.top, rc.right, rc.bottom );

		AdjustWindowRect( &rc, dwStyle, FALSE );

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
/*		MoveWindow( (HWND)hw_state.hwnd,
			(GetSystemMetrics( SM_CXSCREEN ) - hw_state.SCREENW) / 2,
			(GetSystemMetrics( SM_CYSCREEN ) - hw_state.SCREENH) / 2,
			hw_state.SCREENW,
			hw_state.SCREENH + GetSystemMetrics( SM_CYCAPTION ) + 2*GetSystemMetrics( SM_CYFRAME ) - 2,
			TRUE );
*/
		MoveWindow( (HWND)hw_state.hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE );

		// dprintf( "movewindow: %d,%d, %d,%d [%d,%d]\n", rc.left, rc.top, rc.right, rc.bottom, rc.right-rc.left, rc.bottom-rc.top );
	}

	hw_state.orig_SCREENW = hw_state.SCREENW;
	hw_state.orig_SCREENH = hw_state.SCREENH;
	hw_state.orig_bpp = hw_state.bpp;

	return TRUE;
}






//
//
//
static void GFXDLLCALLCONV SW_Deinit( void ) {

	SW_DiscardAllTexture();

	if( pipeline && dfree) dfree( pipeline );
	pipeline = NULL;

	pipelinelen = 0;		     // hany face van most rajta
	allocedpipelinelen = 0; 	     // hanynak van hely

	if( pBMI && dfree ) dfree( pBMI );
	pBMI = NULL;

	if( hBmp ) DeleteObject( hBmp );
	hBmp = 0;
	pDIBMemory = NULL;

	if( hw_state.zbuffer && dfree ) dfree( hw_state.zbuffer );
	hw_state.zbuffer = NULL;

	if( dwinFullscreen() )
		RestoreResoltion();

	if( dprintf ) dprintf("deinit: %s success.\n",GFXDLL_NAME);

	return;
}






//
//
//
static BOOL GFXDLLCALLCONV SW_Fullscreen( int flag ) {

	DWORD dwWndStyle = GetWindowStyle((HWND)hw_state.hwnd);

	if( hw_state.zbuffer && dfree ) dfree( hw_state.zbuffer );
	hw_state.zbuffer = NULL;

	if( flag == TRUE ) {

		// determine the new window style
		dwWndStyle = WS_POPUP | WS_VISIBLE;

		// change the window style
		SetWindowLong((HWND)hw_state.hwnd, GWL_STYLE, dwWndStyle);

		if( hw_state.bpp > 0 )
			if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
				dprintf( "SW_Fullscreen: ChangeResolution(%d, %d, %d) the first phail.\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp);
				hw_state.SCREENW = 640;
				hw_state.SCREENH = 480;
				hw_state.bpp = 32;
				if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
					dprintf( "SW_Fullscreen: ChangeResolution(%d, %d, %d) the second fail.\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp);
						hw_state.SCREENW = 800;
						hw_state.SCREENH = 600;
						hw_state.bpp = 32;
						if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
							dprintf( "SW_Fullscreen: ChangeResolution(%d, %d, %d) final fail.\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp);
							return FALSE;
						}
				}
			}

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE );
		MoveWindow( (HWND)hw_state.hwnd, 0,0, hw_state.SCREENW, hw_state.SCREENH, TRUE );

	}
	else {
		hw_state.SCREENW = hw_state.orig_SCREENW;
		hw_state.SCREENH = hw_state.orig_SCREENH;
		hw_state.bpp = hw_state.orig_bpp;

		RestoreResoltion();

		dwWndStyle &= ~WS_POPUP;
		dwWndStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

		// change the window style
		SetWindowLong((HWND)hw_state.hwnd, GWL_STYLE, dwWndStyle);

		RECT rc;
		DWORD dwStyle;
		WINDOWINFO wi;

		GetWindowInfo( (HWND)hw_state.hwnd, &wi );

		dwStyle = wi.dwStyle;

		rc.left = hw_state.x;
		rc.top = hw_state.y;
		rc.right = rc.left + hw_state.SCREENW;
		rc.bottom = rc.top + hw_state.SCREENH;

		if( hw_state.flag != SETX_NOBORDER )
			AdjustWindowRect( &rc, dwStyle, FALSE );

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE );
	}

	if( hBmp ) DeleteObject( hBmp );
	hBmp = 0;
	pDIBMemory = NULL;

	pBMI->bmiHeader.biSizeImage	= 0;
	pBMI->bmiHeader.biXPelsPerMeter = 0;
	pBMI->bmiHeader.biYPelsPerMeter = 0;
	pBMI->bmiHeader.biClrUsed	= 0;
	pBMI->bmiHeader.biClrImportant	= 0;

	pBMI->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	pBMI->bmiHeader.biWidth 	= hw_state.SCREENW;
	pBMI->bmiHeader.biHeight	= -hw_state.SCREENH;
	pBMI->bmiHeader.biPlanes	= 1;

	pBMI->bmiHeader.biCompression	= BI_BITFIELDS;
	pBMI->bmiHeader.biBitCount	= 16;
	MEMCAST( DWORD, pBMI->bmiColors[0] ) = (drmask << drshift);
	MEMCAST( DWORD, pBMI->bmiColors[1] ) = (dgmask << dgshift);
	MEMCAST( DWORD, pBMI->bmiColors[2] ) = (dbmask << dbshift);

	HDC hdc = GetDC( (HWND)hw_state.hwnd );
	hBmp = CreateDIBSection( hdc, pBMI, DIB_RGB_COLORS, &pDIBMemory, NULL, 0 );
	ReleaseDC( NULL, hdc );

	if( !hBmp || !pDIBMemory ) {
		dfree( pBMI );
		pBMI = NULL;
		dprintf("SW_Fullscreen: CreateDIBSection phailed.\n");
		return FALSE;
	}

	if( hw_state.bZbuffer == TRUE ) {
		if( (hw_state.zbuffer = (FLOAT *)dmalloc(hw_state.SCREENW * hw_state.SCREENH * sizeof(FLOAT))) == NULL ) {
			hw_state.bZbuffer = FALSE;
			dprintf("SW_Fullscreen: Z Buffer allocation failed.\n");
		}
	}

	dprintf("SW_Fullscreen: Switched to %s.\n", flag?"Fullscreen":"Windowed" );

	return TRUE;
}



//
//
//
static void GFXDLLCALLCONV SW_GetDescription( char *str ) {

	sprintf(str,"Software-Only driver v%s",GFXDLL_VERSION);

	return;
}



//
//
//
static void GFXDLLCALLCONV SW_GetData( void *buf, int len ) {

	if( *(ULONG *)buf == GFXDLL_ISMSGOK )
		*(ULONG *)buf = TRUE;

	if( *(ULONG *)buf == GFXDLL_24BIT )
		*(ULONG *)buf = FALSE;

	if( *(ULONG *)buf == GFXDLL_MAXTEXSIZE )
		*(ULONG *)buf = 8*1024;

	if( *(ULONG *)buf == GFXDLL_MINTEXSIZE )
		*(ULONG *)buf = 1;

	if( *(ULONG *)buf == GFXDLL_GETERROR )
		*(ULONG *)buf = 0;

	if( *(ULONG *)buf == GFXDLL_TEX24BIT )
		*(ULONG *)buf = hw_state.tex24bit;

	if( *(ULONG *)buf == GFXDLL_THREADSAFE )
		*(ULONG *)buf = TRUE;

	if( *(ULONG *)buf == GFXDLL_ISDEBUG )

#ifdef _DEBUG
		*(ULONG *)buf = TRUE;
#else
		*(ULONG *)buf = FALSE;
#endif

	if( *(ULONG *)buf == GFXDLL_ENABLED )
		*(ULONG *)buf = TRUE;

	return;
}





//
//
//
void GFXDLLCALLCONV GFXDRV_GetInfo( GFXDLLinfo_t *info ) {

	info->GFXDLL_SetupCulling		= (void *)SW_SetupCulling;
	info->GFXDLL_AddToPipeline		= (void *)SW_AddToPipeline;
	info->GFXDLL_AddTexMapHigh		= (void *)SW_AddTexMapHigh;
	info->GFXDLL_ReloadTexMapHigh		= (void *)SW_ReloadTexMapHigh;
	info->GFXDLL_ModifyTexMapHigh		= (void *)SW_ModifyTexMapHigh;
	info->GFXDLL_PutSpritePoly		= (void *)SW_PutSpritePoly;
	info->GFXDLL_PutPoly			= (void *)SW_PutPoly;
	info->GFXDLL_DiscardAllTexture		= (void *)SW_DiscardAllTexture;
	info->GFXDLL_DiscardTexture		= (void *)SW_DiscardTexture;
	info->GFXDLL_Line			= (void *)SW_Line;
	info->GFXDLL_PutPixel			= (void *)SW_PutPixel;
	info->GFXDLL_BeginScene			= (void *)SW_BeginScene;
	info->GFXDLL_EndScene			= (void *)SW_EndScene;
	info->GFXDLL_FlushScene			= (void *)SW_FlushScene;
	info->GFXDLL_LockLfb			= (void *)SW_LockLfb;
	info->GFXDLL_UnlockLfb			= (void *)SW_UnlockLfb;
	info->GFXDLL_Init			= (void *)SW_Init;
	info->GFXDLL_Deinit			= (void *)SW_Deinit;
	info->GFXDLL_GetDescription		= (void *)SW_GetDescription;
	info->GFXDLL_GetData			= (void *)SW_GetData;
	info->GFXDLL_GetPitch			= (void *)SW_GetPitch;
	info->GFXDLL_Activate			= (void *)SW_Activate;
	info->GFXDLL_FlipPage			= (void *)SW_FlipPage;
	info->GFXDLL_Clear			= (void *)SW_Clear;
	info->GFXDLL_Fog			= (void *)SW_Fog;
	info->GFXDLL_InitPalette		= (void *)SW_InitPalette;
	info->GFXDLL_SetRgb			= (void *)SW_SetRgb;
	info->GFXDLL_GetRgb			= (void *)SW_GetRgb;
	info->GFXDLL_Fullscreen			= (void *)SW_Fullscreen;

	return;
}






//
//
//
extern "C" void dllDeinit( void ) {

	SW_Deinit();

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

