/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif


#define FLOAT_ONE ( 1.0f )

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
extern ULONG (*dGetBackHwnd)( void );
extern "C" BOOL (*dwinQuit)( char *s, ... );


extern FIXED dffognear,dffogfar;


typedef struct edgescan_s {

	int	Direction;
	int	RemainingScans;
	int	CurrentEnd;

	FLOAT	DestX;
	FLOAT	DestXStep;

	FLOAT	SourceX;
	FLOAT	SourceY;
	FLOAT	SourceStepX;
	FLOAT	SourceStepY;

	FLOAT	DestZ;			// camera <-> pont
	FLOAT	DestZStep;

	FLOAT	ooz;
	FLOAT	oozstep;

	FLOAT	red,green,blue;
	FLOAT	redstep,greenstep,bluestep;

	FLOAT	alpha;
	FLOAT	alphastep;

} edgescan_t,*edgescan_ptr;



typedef struct hardwarestate_s {

	BOOL tex24bit;

	int texmap;
	int mode;
	int SCREENW,SCREENH,bpp,pixel_len;
	int CLIPMINX,CLIPMINY;
	int CLIPMAXX,CLIPMAXY;
	__int64 hwnd;

	BOOL bZbuffer;
	FLOAT *zbuffer;

	BOOL bBlend;
	__int64 hInst;

	int x,y;
	int flag;
	int orig_SCREENW,orig_SCREENH,orig_bpp;

} hardwarestate_t;

#define HM_UNINIT	0
#define HM_TEXTURE	1
#define HM_COLOR	2

extern int drmask,dgmask,dbmask;
extern int drsize,dgsize,dbsize;
extern int drshift,dgshift,dbshift;


extern hardwarestate_t hw_state;

extern FLOAT sf_zclipnear;
extern FLOAT sf_zclipnearP2;
extern FLOAT sf_zclipfar;
extern FLOAT sf_zclipfarP2;
extern FLOAT fzclipfar;
extern int sf_perdiv;
extern FLOAT dfognear,dfogfar,dinvfog;

typedef struct pipeline_s {

	point3_t tcenter;
	triangle_t triangle;		// screen poly
	triangle3_t ttriangle;		// transformed poly

	int face_flag;

	ULONG color;

	texmap_t *texmap;
	point_t texvert[3];
	rgb_t rgb[3];

} pipeline_t,*pipeline_ptr;


/*
//
// __inline
//
static __inline FIXED fmul( FIXED f1, FIXED f2 ) {

	FIXED f3;

#ifdef __GNUC__

	f3 = f1 * f2;

#else
	__asm {

		mov	eax, f1
		mov	edx, f2

		imul	edx
		shrd	eax, edx, 16

		mov	f3, eax
	}
#endif

	return f3;
}




//
// __inline
//
static __inline FIXED fdiv( FIXED f1, FIXED f2 ) {

	FIXED f3;

	if( f2 == FIXED_ZERO )
		Quit("fdiv: devide by zero.");

#ifdef __GNUC__

	f3 = f1 / f2;

#else
	__asm {

		mov	eax, f1
		mov	ecx, f2

		cdq
		shld	edx,eax,16
		shl	eax,16
		idiv	ecx

		mov	f3, eax
	}
#endif

	return f3;
}



//
// 8.8 __inline
//
static __inline FIXED fsqrt( FIXED f1 ) {

	FIXED f2;

#ifdef __GNUC__

	dllQuit("fsqrt: not ready.");

	f1 = (int)sqrt( (double)f1 );

#else
	__asm {

		push	eax
		push	ebx
		push	ecx
		push	edx

		mov	ecx,f1

		xor	eax,eax
		mov	ebx,40000000h
	sqrtLP1:
		mov	edx,ecx
		sub	edx,ebx
		jl	sqrtLP2
		sub	edx,eax
		jl	sqrtLP2
		mov	ecx,edx
		shr	eax,1
		or	eax,ebx
		shr	ebx,2
		jnz	sqrtLP1
		shl	eax,8
		jmp	sqrtLP3
	sqrtLP2:
		shr	eax,1
		shr	ebx,2
		jnz	sqrtLP1
		shl	eax,8
	sqrtLP3:

		mov	f2,eax

		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
	}
#endif

	return f2;
}

*/


/*
//
// ket pont tavolsaga
//
static __inline FLOAT DistanceVector( point3_t v1, point3_t v2 ) {

	FLOAT dst;

	dst = (FLOAT)sqrt( ( (v1[0]-v2[0]) * (v1[0]-v2[0]) ) +
			   ( (v1[1]-v2[1]) * (v1[1]-v2[1]) ) +
			   ( (v1[2]-v2[2]) * (v1[2]-v2[2]) )  );

	return dst;
}
*/


#undef RGBINT
#define RGBINT(r,g,b)	(USHORT)( ( (ULONG)r << drshift ) | ( (ULONG)g << dgshift ) | ( (ULONG)b << dbshift ) )

#undef INTRED16
#undef INTGREEN16
#undef INTBLUE16

#define INTRED16(c)		( (ULONG) ( ((c)>>drshift) & drmask ) )
#define INTGREEN16(c)	( (ULONG) ( ((c)>>dgshift) & dgmask ) )
#define INTBLUE16(c)	( (ULONG) ( ((c)>>dbshift) & dbmask ) )

#undef RGB24
#define RGB24(c)	( (ULONG)( ( ( c >> (16+(8-drsize)) ) & drmask ) << drshift ) | (ULONG)( ( ( c >> (8+(8-dgsize)) ) & dgmask ) << dgshift ) | (ULONG)( ( ( c >> (0+(8-dbsize)) ) & dbmask ) << dbshift ) )




#ifdef __cplusplus
}
#endif


