
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#include <glide.h>

#include <xlib.h>
#include <xinner.h>

#include "xglide.h"

#include "dllmain.h"

RCSID( "$Id: Glide.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )

// #pragma comment( lib "glide3x.lib fxmisc.lib fxpci.lib")


#define GFXDLL_VERSION "1.0"
char *GFXDLL_NAME = "xglide.drv";

#pragma comment( exestr , "3Dfx Glide v3 driver v" GFXDLL_VERSION " (" __DATE__ " " __TIME__ ")")


#define GLIDEDLLNAME "GLIDE3X.DLL"
static char glide_error_str[512] = "Glide: uninitialized error string";


// #define GOUROUD
#define MIPMAP_LEVEL 2

extern void dllQuit( char *s, ... );
void (*dprintf)( char *s, ... ) = NULL;
int (*dCheckParm)( char *s ) = NULL;
texmap_t *(*dTexForNum)( int texmapid ) = NULL;
void (*dCopyVector)( point3_t dst, point3_t src ) = NULL;
FLOAT (*dDotProduct)( point3_t v1, point3_t v2 ) = NULL;
ULONG (*dGetBackHwnd)( void ) = NULL;
int (*dfree)( void * ) = NULL;


static FLOAT gl_zclipnear = FLOAT_EPSILON;
static FLOAT gl_zclipnearP2 = FLOAT_EPSILON;
static FLOAT gl_zclipfar = FLOAT_MAX;
static FLOAT gl_zclipfarP2 = FLOAT_MAX;
static FLOAT gl_scale = 1.0f;


static glidetexture_t *glidetexture = NULL;
static int nglidetextures = 0;
static int alloced_texmap = 0;
static BOOL must_reset_cache = FALSE;


static GrLfbInfo_t lfbinfo;

#define BPL ( lfbinfo.strideInBytes )
// #define LFBMODE ( GR_LFB_READ_ONLY|GR_LFB_WRITE_ONLY|GR_LFB_IDLE )
#define LFBMODE ( GR_LFB_WRITE_ONLY )


static point3_t vpos,		// viewer's position
		vdir,		//  Ä´ÃÄ    direction
		vnormal,	//  Ä´ÃÄ    normal = vpos - vdir;
		mvnormal;	// mvnormal = dir - pos;


hardwarestate_t hw_state;
static BOOL bInGraphic = FALSE;
static BOOL isGouraud = FALSE;



//
//
//
static void GFXDLLCALLCONV Glide_SetupCulling( point3_t pos, point3_t dir, point3_t normal,  FLOAT zn, FLOAT zf ) {

	if( pos ) dCopyVector( vpos, pos );
	if( dir ) dCopyVector( vdir, dir );
	if( normal ) dCopyVector( vnormal, normal );

	gl_zclipnear = zn;
	gl_zclipnearP2 = zn*zn;
	gl_zclipfar = zf;
	gl_zclipfarP2 = zf*zf;

	gl_scale = ( hw_state.zmax / gl_zclipfar );

	return;
}








#if 0
	long start,align;

	grGet( GR_TEXTURE_ALIGN, sizeof(align), &align );

	// dprintf("align: %d\n",align);

	nextTexture += align;
	nextTexture &= ~(align-1);
#endif



//
//
//
static BOOL GFXDLLCALLCONV Glide_Rasterize( trans2pipeline_t trans2pipeline ) {
// polygon_t ppoly, polygon3_t poly, polygon3_t tpoly, point3_t *pointnormal,
// point_t *outtex, point3_t *inst, face_t *f, int *rgb ) {

#define P(tag) trans2pipeline.tag

	FLOAT dot,col=0.0f;
	int k, glide_address, glide_mem_required;
	GrVertex vert[8];
	glidetexture_t *glidetexmap;
	texmap_t *texmap = NULL;
	BOOL alpha = FALSE;
	int texmapid = -1;
	rgb_t *outrgb = NULL;
	point3_t *outst;

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

	if( texmapid != -1 ) { // ISFLAG( P(face)->flag, FF_TEXTURE ) ) {

		// texturazott

		if( (texmap = dTexForNum(texmapid)) == NULL )
			return TRUE;

		if( hw_state.mode != HM_TEXTURE ) {

			pGrColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
							GR_COMBINE_FACTOR_LOCAL,
							GR_COMBINE_LOCAL_ITERATED,
							GR_COMBINE_OTHER_TEXTURE,
							FXFALSE );

			hw_state.mode = HM_TEXTURE;
		}

		glidetexmap = NULL;

		for( k=0; k<nglidetextures; k++ )
			if( glidetexture[k].texmapid == texmap->texmapid ) {
				glidetexmap = &glidetexture[k];
				break;
			}

		if( glidetexmap == NULL )
			return TRUE;

		// See if it's in the cache
		if( (glide_address = glide_cache_item_check( texmap->name )) == (-1) ) {

			glide_mem_required = pGrTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );

			// Find (or create) some free space to download it to
			if( (glide_address = glide_cache_item_space( texmap->name, glide_mem_required )) == (-1) ) {
				must_reset_cache = TRUE;
				return TRUE;
			}

			// Get a pointer from the main-mem texture cache
			glidetexmap->info.data = glidetexmap->mipmap;

			// Download it to the TMU
			pGrTexDownloadMipMap( GR_TMU0, glide_address, GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );

			// Select the texture
			// grTexSource( GR_TMU0, glide_address, GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );
			// hw_state.texmap = texmapid;

			hw_state.texmap = (-1);
		}
//		else

		if( hw_state.texmap != texmapid ) {

			pGrTexSource( GR_TMU0, glide_address, GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );
			hw_state.texmap = texmapid;
		}


		if( texmap->isChromaKey ) {
			pGrChromakeyMode( GR_CHROMAKEY_ENABLE );
			pGrChromakeyValue( texmap->chromaColor );
		}

		if( texmap->isAlpha ) {
			pGrAlphaBlendFunction( GR_BLEND_ONE,
								  GR_BLEND_ONE,
								  GR_BLEND_ONE,
								  GR_BLEND_ZERO );
			pGrDepthMask( FXFALSE );
		}
	}
	else
	if( ISFLAG( P(face)->flag, FF_COLOR ) ) {

		// fillezett

		if( hw_state.mode != HM_COLOR ) {

			pGrColorCombine( GR_COMBINE_FUNCTION_LOCAL,
							GR_COMBINE_FACTOR_NONE,
							GR_COMBINE_LOCAL_ITERATED,
							GR_COMBINE_OTHER_NONE,
							FXFALSE );

			hw_state.mode = HM_COLOR;
		}
	}
	else
		return TRUE;

	// a szinekben meagadott alpha

	for( k=0; k<P(ppoly).npoints; k++ )
		if( outrgb[k].a < 255 ) {
			alpha = TRUE;
			break;
		}

	if( alpha == TRUE ) {
		pGrAlphaCombine( GR_COMBINE_FUNCTION_LOCAL,
	                    GR_COMBINE_FACTOR_NONE,
		                GR_COMBINE_LOCAL_ITERATED,
			            GR_COMBINE_OTHER_NONE,
				        FXFALSE );
		if( (texmap != NULL) && (texmap->isAlpha == TRUE) )
			pGrAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
								  GR_BLEND_ONE,
								  GR_BLEND_ONE,
								  GR_BLEND_ZERO );
		else
			pGrAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
								  GR_BLEND_ONE_MINUS_SRC_ALPHA,
								  GR_BLEND_ONE,
								  GR_BLEND_ZERO );
	}

	if( ISFLAG( P(face)->flag, FF_DBLSIDE ) || ISFLAG( P(face)->flag, FF_FORCEFLAG ) ) {
		if( hw_state.cullmode != 0 ) {
			pGrCullMode( GR_CULL_DISABLE );
			hw_state.cullmode = 0;
		}
	}
	else {
		if( hw_state.cullmode != 1 ) {
			pGrCullMode( GR_CULL_POSITIVE );
			hw_state.cullmode = 1;
		}
	}


	if( ISFLAG( P(face)->flag, FF_FARTHEST ) ) {
		if( hw_state.zbuffermode == TRUE ) {
			pGrDepthBufferFunction( GR_CMP_ALWAYS );
			pGrDepthMask( FXFALSE );
			hw_state.zbuffermode = FALSE;
		}
	}
	else {
		if( hw_state.zbuffermode == FALSE ) {
			pGrDepthBufferFunction( GR_CMP_LEQUAL );
			if( (texmap == NULL) || (texmap->isAlpha == FALSE) )
				pGrDepthMask( FXTRUE );
			hw_state.zbuffermode = TRUE;
		}
	}

	for( k=0; k<P(ppoly).npoints; k++ ) {

		vert[k].x = (FLOAT)( P(ppoly).point[k][0] );
		vert[k].y = (FLOAT)( P(ppoly).point[k][1] );
		// vert[k].z = P(tpoly).point[k][2];

		vert[k].oow = 1.0f / (P(tpoly).point[k][2] * gl_scale);

		vert[k].r = (FLOAT)outrgb[k].r;
		vert[k].g = (FLOAT)outrgb[k].g;
		vert[k].b = (FLOAT)outrgb[k].b;
		vert[k].a = (FLOAT)outrgb[k].a;

		if( texmapid != -1 ) { // ISFLAG( P(face)->flag, FF_TEXTURE ) ) {
			vert[k].tmuvtx[0].oow = vert[k].oow;
			vert[k].tmuvtx[0].sow = outst[k][0] * vert[k].tmuvtx[0].oow;
			vert[k].tmuvtx[0].tow = outst[k][1] * vert[k].tmuvtx[0].oow;
		}
		else
		if( isGouraud == TRUE ) {

			#define ADDCOLOR ( (FLOAT)100.0 )
			#define MAXCOLOR ( (FLOAT)255.0 - ADDCOLOR )

			dot = -dDotProduct( vnormal, trans2pipeline.outpointnormal[k]  );
			if( ISFLAG( P(face)->flag, FF_DBLSIDE ) )
				dot = ABS(dot);
			vert[k].r = ADDCOLOR + MAXCOLOR * ((dot<0.0f)?0.0f:dot);
			vert[k].g = vert[k].r;
			vert[k].b = vert[k].r;
			vert[k].a = (FLOAT)outrgb[k].a;
		}

/***
	#ifdef GOUROUD
		#define ADDCOLOR ( (FLOAT)50.0 )
		#define MAXCOLOR ( (FLOAT)256.0 - ADDCOLOR )

		dot = -DotProduct( vnormal, pointnormal[k] );
		if( ISFLAG( f->flag, FF_DBLSIDE ) )
			dot = ABS(dot);
		col = ADDCOLOR + MAXCOLOR * ((dot<0.0f)?0.0f:dot);
	#endif

		vert[k].r = P(outrgb)[k].r + col;
		vert[k].g = P(outrgb)[k].g + col;
		vert[k].b = P(outrgb)[k].b + col;
***/
		CLAMPMINMAX( vert[k].r, 0.0f, 255.0f );
		CLAMPMINMAX( vert[k].g, 0.0f, 255.0f );
		CLAMPMINMAX( vert[k].b, 0.0f, 255.0f );
		CLAMPMINMAX( vert[k].a, 0.0f, 255.0f );
	}

	pGrDrawVertexArrayContiguous( GR_POLYGON, P(ppoly).npoints, vert, sizeof(vert[0]) );


	if( texmapid != -1 ) { // ISFLAG( P(face)->flag, FF_TEXTURE ) ) {

		if( texmap->isChromaKey )
			pGrChromakeyMode( GR_CHROMAKEY_DISABLE );

		if( texmap->isAlpha ) {
			pGrAlphaBlendFunction( GR_BLEND_ONE,
								  GR_BLEND_ZERO,
								  GR_BLEND_ONE,
								  GR_BLEND_ZERO );
			if( hw_state.zbuffermode == TRUE )
				pGrDepthMask( FXTRUE );
		}
	}

	if( alpha == TRUE ) {
		pGrAlphaCombine( GR_COMBINE_FUNCTION_LOCAL,
						GR_COMBINE_FACTOR_NONE,
						GR_COMBINE_LOCAL_CONSTANT,
						GR_COMBINE_OTHER_NONE,
						FXFALSE );
		pGrAlphaBlendFunction( GR_BLEND_ONE,
							  GR_BLEND_ZERO,
							  GR_BLEND_ONE,
							  GR_BLEND_ZERO );
	}

	return TRUE;
}




//
//
//
static BOOL GFXDLLCALLCONV Glide_AddToPipeline( trans2pipeline_t trans2pipeline ) {

#define P(tag) trans2pipeline.tag

	P(faceid) = 0;
	Glide_Rasterize( trans2pipeline );

	if( ISFLAG( P(face)->flag, FF_ENVMAP) ) {
		int i;
		P(faceid) = 1;
		for( i=0; i<P(ppoly).npoints; i++ ) {
			P(outrgb1)[i].r = P(outrgb)[i].r;
			P(outrgb1)[i].g = P(outrgb)[i].g;
			P(outrgb1)[i].b = P(outrgb)[i].b;
		}
		Glide_Rasterize( trans2pipeline );
	}

#undef P

	return TRUE;
}





//
//
//
static void GFXDLLCALLCONV Glide_PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) {

	int k, glide_address, glide_mem_required;
	GrVertex vert[8];
	glidetexture_t *glidetexmap;
	texmap_t *texmap = NULL;
	BOOL alpha = FALSE;

	if( (texmap = dTexForNum(texmapid)) == NULL )
		return;

	if( hw_state.mode != HM_TEXTURE ) {

		pGrColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
				GR_COMBINE_FACTOR_LOCAL,
				GR_COMBINE_LOCAL_ITERATED,
				GR_COMBINE_OTHER_TEXTURE,
				FXFALSE );

		hw_state.mode = HM_TEXTURE;
	}

	glidetexmap = NULL;

	for( k=0; k<nglidetextures; k++ )
		if( glidetexture[k].texmapid == texmap->texmapid ) {
			glidetexmap = &glidetexture[k];
			break;
		}

	if( glidetexmap == NULL )
		return;

	// See if it's in the cache
	if( (glide_address = glide_cache_item_check( texmap->name )) == (-1) ) {

		glide_mem_required = pGrTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );

		// Find (or create) some free space to download it to
		if( (glide_address = glide_cache_item_space( texmap->name, glide_mem_required )) == (-1) ) {
			must_reset_cache = TRUE;
			return;
		}

		// Get a pointer from the main-mem texture cache
		glidetexmap->info.data = glidetexmap->mipmap;

		// Download it to the TMU
		pGrTexDownloadMipMap( GR_TMU0, glide_address, GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );

		// Select the texture
//		grTexSource( GR_TMU0, glide_address, GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );
//		hw_state.texmap = texmapid;

		hw_state.texmap = (-1);
	}
//	else

	if( hw_state.texmap != texmapid ) {

		pGrTexSource( GR_TMU0, glide_address, GR_MIPMAPLEVELMASK_BOTH, &glidetexmap->info );
		hw_state.texmap = texmapid;
	}


	if( texmap->isChromaKey ) {
		pGrChromakeyMode( GR_CHROMAKEY_ENABLE );
		pGrChromakeyValue( texmap->chromaColor );
	}

	if( texmap->isAlpha ) {
		pGrAlphaBlendFunction( GR_BLEND_ONE,
							  GR_BLEND_ONE,
							  GR_BLEND_ONE,
							  GR_BLEND_ZERO );
	}


	// a szinekben meagadott alpha

	for( k=0; k<ppoly.npoints; k++ )
		if( rgb[k].a < 255 ) {
			alpha = TRUE;
			break;
		}


	if( alpha == TRUE ) {
		pGrAlphaCombine( GR_COMBINE_FUNCTION_LOCAL,
	                    GR_COMBINE_FACTOR_NONE,
		                GR_COMBINE_LOCAL_ITERATED,
			            GR_COMBINE_OTHER_NONE,
				        FXFALSE );
		if( texmap->isAlpha == TRUE )
			pGrAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
								  GR_BLEND_ONE,
								  GR_BLEND_ONE,
								  GR_BLEND_ZERO );
		else
			pGrAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
								  GR_BLEND_ONE_MINUS_SRC_ALPHA,
								  GR_BLEND_ONE,
								  GR_BLEND_ZERO );
	}

	if( hw_state.cullmode != 0 ) {
		pGrCullMode( GR_CULL_DISABLE );
		hw_state.cullmode = 0;
	}

	/***
	if( hw_state.zbuffermode == TRUE ) {
		grDepthBufferFunction( GR_CMP_ALWAYS );
		grDepthMask( FXFALSE );
		hw_state.zbuffermode = FALSE;
	}
	***/

pGrDepthMask( FXFALSE );

	for( k=0; k<ppoly.npoints; k++ ) {

		vert[k].x = (FLOAT)ppoly.point[k][0];
		vert[k].y = (FLOAT)ppoly.point[k][1];
		vert[k].oow = 1.0f;

		vert[k].tmuvtx[0].oow = vert[k].oow;
		vert[k].tmuvtx[0].sow = inst[k][0] * vert[k].tmuvtx[0].oow;
		vert[k].tmuvtx[0].tow = inst[k][1] * vert[k].tmuvtx[0].oow;

		vert[k].r = (FLOAT)rgb[k].r;
		vert[k].g = (FLOAT)rgb[k].g;
		vert[k].b = (FLOAT)rgb[k].b;
		vert[k].a = (FLOAT)rgb[k].a;
	}

	pGrDrawVertexArrayContiguous( GR_POLYGON, ppoly.npoints, vert, sizeof(vert[0]) );

pGrDepthMask( FXTRUE );

	if( texmap->isChromaKey )
		pGrChromakeyMode( GR_CHROMAKEY_DISABLE );

	if( texmap->isAlpha ) {
		pGrAlphaBlendFunction( GR_BLEND_ONE,
							  GR_BLEND_ZERO,
							  GR_BLEND_ONE,
							  GR_BLEND_ZERO );
	}

	if( alpha == TRUE ) {
		pGrAlphaCombine( GR_COMBINE_FUNCTION_LOCAL,
						GR_COMBINE_FACTOR_NONE,
						GR_COMBINE_LOCAL_CONSTANT,
						GR_COMBINE_OTHER_NONE,
						FXFALSE );
		pGrAlphaBlendFunction( GR_BLEND_ONE,
							  GR_BLEND_ZERO,
							  GR_BLEND_ONE,
							  GR_BLEND_ZERO );
	}


	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_PutPoly( polygon_t ppoly, rgb_t *rgb ) {

	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_ScaleSprite( int dest_x, int dest_y, UCHAR *buf1, UCHAR *buf2 ) {

	ULONG ErrorAccX, ErrorAccY, ErrorAdjX, ErrorAdjY;
	int oldx, oldy, newx, newy;
	int i, j, count;
	USHORT *src_base;
	USHORT *src_ptr;
	USHORT *dest_ptr;
	USHORT *newbuf;

	oldx = SPRITEW(buf1);
	oldy = SPRITEH(buf1);
	newx = dest_x;
	newy = dest_y;

	newbuf = (USHORT*)(&buf2[SPRITEHEADER]);

	src_base = (USHORT*)(&buf1[SPRITEHEADER]);
	dest_ptr = newbuf;

	// My bitmap scaling routine.  As you probably noticed, it's
	// pretty Bresenhammy!

	ErrorAccY = 0x8000;

	if( newx>oldx ) {

		// Biggering

		ErrorAdjX = ((((ULONG)newx) << 16) / (((ULONG)oldx)));
		ErrorAdjY = ((((ULONG)newy) << 16) / (((ULONG)oldy)));

		i=oldy;
		while( i-- ) {

			ErrorAccX = 0x8000;
			src_ptr = src_base;

			j=oldx;
			while( j-- ) {

				ErrorAccX += ErrorAdjX;
				if( (count = (ErrorAccX >> 16)) ) {

					ErrorAccX &= 0xFFFFL;
					while( count-- ) {
						*dest_ptr++ = *src_ptr;
					}
				}

				++src_ptr;
			}

			ErrorAccY += ErrorAdjY;
			count = (ErrorAccY >> 16) - 1;

			while( count-- ) {
				memmove( dest_ptr, dest_ptr - newx, newx*2 );
				dest_ptr += newx;
			}

			ErrorAccY &= 0xFFFFL;
			src_base += oldx;
		}
	}
	else {

		// Smallering

		ErrorAdjX = ((((ULONG)oldx) << 16) / (((ULONG)newx)));
		ErrorAdjY = ((((ULONG)oldy) << 16) / (((ULONG)newy)));

		i=newy;
		while( i-- ) {

			ErrorAccX = 0x8000;
			src_ptr = src_base;

			j=newx;
			while( j-- ) {

				*dest_ptr++ = *src_ptr;
				ErrorAccX += ErrorAdjX;
				src_ptr += (ErrorAccX >> 16);
				ErrorAccX &= 0xFFFFL;
			}

			ErrorAccY += ErrorAdjY;
			src_base += (oldx * (ErrorAccY >> 16));
			ErrorAccY &= 0xFFFFL;
		}
	}

	return;
}






//
//
//
static BOOL isok( int num ) {

	     if( num == 256 ) return TRUE;
	else if( num == 128 ) return TRUE;
	else if( num == 64 ) return TRUE;
	else if( num == 32 ) return TRUE;
	else if( num == 16 ) return TRUE;
	else if( num == 8 ) return TRUE;
	else if( num == 4 ) return TRUE;
	else if( num == 2 ) return TRUE;
	else if( num == 1 ) return TRUE;

	return FALSE;
}



//
//
//
static int getaspect( int width, int height ) {

	int aspect;

	if( width >= height ) {

		aspect = width / height;

		if( aspect == 1 ) return GR_ASPECT_LOG2_1x1;
		if( aspect == 2 ) return GR_ASPECT_LOG2_2x1;
		if( aspect == 4 ) return GR_ASPECT_LOG2_4x1;
		if( aspect == 8 ) return GR_ASPECT_LOG2_8x1;

		return GR_ASPECT_LOG2_1x1;
	}
	else {

		aspect = height / width;

		if( aspect == 1 ) return GR_ASPECT_LOG2_1x1;
		if( aspect == 2 ) return GR_ASPECT_LOG2_1x2;
		if( aspect == 4 ) return GR_ASPECT_LOG2_1x4;
		if( aspect == 8 ) return GR_ASPECT_LOG2_1x8;

		return GR_ASPECT_LOG2_1x1;
	}

	return GR_ASPECT_LOG2_1x1;
}





//
//  only HIGH sprite
//
static int GFXDLLCALLCONV Glide_AddTexMapHigh( texmap_t *texmap ) {

	UCHAR *pmipmap,*spr;
	FxU32 *startAddress;
	GrTexInfo *info;
	int width,height,larger,i;
	int num;

#define ORIG_SPR texmap->sprite1

	if( alloced_texmap < (nglidetextures + 1) ) {

		alloced_texmap += MAXTEXTURE;
		if( (glidetexture = (glidetexture_t*)drealloc( glidetexture, (alloced_texmap*sizeof(glidetexture_t)) )) == NULL )
			dllQuit("Glide_AddTexMapHigh: can't realloc glidetexture (%d).",alloced_texmap);
	}

	num = nglidetextures;

	//if( (num = nglidetextures) >= MAXTEXTURE )
	//	dllQuit("Glide_AddTexMapHigh: out of texture slots (MAXTEXTURE=%d).",MAXTEXTURE);

	memset( &glidetexture[num], 0, sizeof(glidetexture_t) );

	if( SPRPIXELLEN(ORIG_SPR) != 2 )
		dllQuit("Glide_AddTexMapHigh: only HIGH sprite.");

	width = SPRITEW(ORIG_SPR);
	height = SPRITEH(ORIG_SPR);

	if( !isok(width) || !isok(height) )
		dllQuit("Glide_AddTexMapHigh: (%dx%d) is not power of 2.",width,height);

	glidetexture[num].texmapid = texmap->texmapid;

	info = &glidetexture[num].info;
	startAddress = &glidetexture[num].startAddress;

	larger = MAX(width,height);

	     if( larger == 256 ) info->largeLodLog2 = GR_LOD_LOG2_256;
	else if( larger == 128 ) info->largeLodLog2 = GR_LOD_LOG2_128;
	else if( larger == 64 ) info->largeLodLog2 = GR_LOD_LOG2_64;
	else if( larger == 32 ) info->largeLodLog2 = GR_LOD_LOG2_32;
	else if( larger == 16 ) info->largeLodLog2 = GR_LOD_LOG2_16;
	else if( larger == 8 ) info->largeLodLog2 = GR_LOD_LOG2_8;
	else if( larger == 4 ) info->largeLodLog2 = GR_LOD_LOG2_4;
	else if( larger == 2 ) info->largeLodLog2 = GR_LOD_LOG2_2;
	else if( larger == 1 ) info->largeLodLog2 = GR_LOD_LOG2_1;
	else dllQuit("Glide_AddTexMapHigh: texture too large (%dx%d).",width,height);

	info->aspectRatioLog2 = getaspect( width, height );

	if( (glidetexture[num].mipmap = (UCHAR*)dmalloc( ((256*256)+(128*128)+(64*64)+(32*32)+(16*16)+(8*8)+(4*4)+(2*2)+(1*1))*2 ) ) == NULL )
		dllQuit("Glide_AddTexMapHigh: can't alloc mipmap.");

	pmipmap = glidetexture[num].mipmap;

	memcpy( pmipmap, &ORIG_SPR[SPRITEHEADER], width*height*2 );
	pmipmap += width*height*2;

	for( i=0; i<MIPMAP_LEVEL; i++ ) {

		if( width == 1 || height == 1 )
			break;

		width /= 2;
		height /= 2;

		if( (spr = (UCHAR*)dmalloc( SPRITEHEADER + (width*height*2) ) ) == NULL )
			dllQuit("Glide_AddTexMapHigh: can't alloc sprite.");

		MKSPRW( spr, width );
		MKSPRH( spr, height );
		MKSPR16( spr );

		Glide_ScaleSprite( width,height, ORIG_SPR, spr );
		memmove( pmipmap, &spr[SPRITEHEADER], width*height*2 );
		pmipmap += width*height*2;

		dfree( spr );
	}

	info->smallLodLog2 = info->largeLodLog2 - i;
	info->format = GR_TEXFMT_RGB_565;
	info->data = glidetexture[num].mipmap;

	hw_state.texmap = -1;

	++nglidetextures;

#undef ORIG_SPR

	return glidetexture[num].texmapid;
}




//
//
//
static int GFXDLLCALLCONV Glide_ReloadTexMapHigh( texmap_t *texmap ) {

	UCHAR *pmipmap,*spr;
	FxU32 *startAddress;
	GrTexInfo *info;
	int width,height,larger,i;
	int num;

#define ORIG_SPR texmap->sprite1

	for( num=0; num<nglidetextures; num++ )
		if( glidetexture[num].texmapid == texmap->texmapid )
			break;

	if( num >= nglidetextures )
		return Glide_AddTexMapHigh( texmap );

	if( SPRPIXELLEN(ORIG_SPR) != 2 )
		dllQuit("Glide_ReloadTexMapHigh: only HIGH sprite.");

	width = SPRITEW(ORIG_SPR);
	height = SPRITEH(ORIG_SPR);

	if( !isok(width) || !isok(height) )
		dllQuit("Glide_ReloadTexMapHigh: (%dx%d) is not power of 2.",width,height);

	glidetexture[num].texmapid = texmap->texmapid;

	info = &glidetexture[num].info;
	startAddress = &glidetexture[num].startAddress;

	larger = MAX(width,height);

	     if( larger == 256 ) info->largeLodLog2 = GR_LOD_LOG2_256;
	else if( larger == 128 ) info->largeLodLog2 = GR_LOD_LOG2_128;
	else if( larger == 64 ) info->largeLodLog2 = GR_LOD_LOG2_64;
	else if( larger == 32 ) info->largeLodLog2 = GR_LOD_LOG2_32;
	else if( larger == 16 ) info->largeLodLog2 = GR_LOD_LOG2_16;
	else if( larger == 8 ) info->largeLodLog2 = GR_LOD_LOG2_8;
	else if( larger == 4 ) info->largeLodLog2 = GR_LOD_LOG2_4;
	else if( larger == 2 ) info->largeLodLog2 = GR_LOD_LOG2_2;
	else if( larger == 1 ) info->largeLodLog2 = GR_LOD_LOG2_1;
	else dllQuit("Glide_ReloadTexMapHigh: texture too large (%dx%d).",width,height);

	info->aspectRatioLog2 = getaspect( width, height );

	if( (glidetexture[num].mipmap = (UCHAR*)dmalloc( ((256*256)+(128*128)+(64*64)+(32*32)+(16*16)+(8*8)+(4*4)+(2*2)+(1*1))*2 ) ) == NULL )
		dllQuit("Glide_AddTexMapHigh: can't alloc mimmap.");

	pmipmap = glidetexture[num].mipmap;

	memcpy( pmipmap, &ORIG_SPR[SPRITEHEADER], width*height*2 );
	pmipmap += width*height*2;

	for( i=0; i<MIPMAP_LEVEL; i++ ) {

		if( width == 1 || height == 1 )
			break;

		width /= 2;
		height /= 2;

		if( (spr = (UCHAR*)dmalloc( SPRITEHEADER + (width*height*2) ) ) == NULL )
			dllQuit("Glide_ReloadTexMapHigh: can't alloc sprite.");

		MKSPRW( spr, width );
		MKSPRH( spr, height );
		MKSPR16( spr );

		Glide_ScaleSprite( width,height, ORIG_SPR, spr );
		memmove( pmipmap, &spr[SPRITEHEADER], width*height*2 );
		pmipmap += width*height*2;

		dfree( spr );
	}

	info->smallLodLog2 = info->largeLodLog2 - i;
	info->format = GR_TEXFMT_RGB_565;
	info->data = glidetexture[num].mipmap;

	hw_state.texmap = -1;
	must_reset_cache = TRUE;

#undef ORIG_SPR

	return glidetexture[num].texmapid;
}


//
// TODO:
//
static int GFXDLLCALLCONV Glide_ModifyTexMapHigh( texmap_t *texmap ) {

	int num;

#define ORIG_SPR texmap->sprite1

	for( num=0; num<nglidetextures; num++ )
		if( glidetexture[num].texmapid == texmap->texmapid )
			break;

	if( num >= nglidetextures )
		return 0;






#undef ORIG_SPR

	return glidetexture[num].texmapid;
}



//
//
//
static void GFXDLLCALLCONV Glide_DiscardAllTexture( void ) {

	int i;

	for( i=0; i<nglidetextures; i++ ) {
		if( glidetexture[i].mipmap ) dfree( glidetexture[i].mipmap );
	}

	if( glidetexture ) dfree( glidetexture );

	glidetexture = NULL;

	nglidetextures = 0;
	alloced_texmap = 0;

	return;
}


//
//
//
static void GFXDLLCALLCONV Glide_DiscardTexture( int handler ) {

	int i;

	for( i=0; i<nglidetextures; i++ )
		if( glidetexture[i].texmapid == handler )
			break;

	if( i >= nglidetextures ) {
		dprintf("Glide_DiscardTexture: no %d texture.\n",handler);
		return;
	}

	if( glidetexture[i].mipmap ) dfree( glidetexture[i].mipmap );

	dprintf("(hwtexmapid: %d) ",glidetexture[i].texmapid);

	if( ((nglidetextures-i)-1) > 0 )
		memmove( &glidetexture[i], &glidetexture[i+1], ((nglidetextures-i)-1) * sizeof(glidetexture_t) );

	--nglidetextures;

	return;
}




//
//
//
static void GFXDLLCALLCONV Glide_Line( int x0, int y0, int x1, int y1, rgb_t color ) {

	return;
}


//
//
//
static void GFXDLLCALLCONV Glide_PutPixel( int x, int y, rgb_t color ) {

	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_BeginScene( void ) {

	// init_fpu();

	if( must_reset_cache == TRUE ) {
		glide_cache_init();
		must_reset_cache = FALSE;
	}

	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_EndScene( void ) {

	pGrFlush();

	return;
}




//
//
//
static void GFXDLLCALLCONV Glide_FlushScene( void ) {

	pGrFlush();

	return;
}




static int lfb_flag;

//
//
//
static BOOL GFXDLLCALLCONV Glide_LockLfb( __int64 *mem, int flag ) {

	lfb_flag = flag;

	if( flag == LS_WRITE ) {
		if( pGrLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT, FXFALSE, &lfbinfo ) == FXFALSE )
			return FALSE;
	}
	else
	if( flag == LS_READ )
		if( pGrLfbLock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXTRUE, &lfbinfo ) == FXFALSE )
			return FALSE;

	*mem = (__int64)lfbinfo.lfbPtr;

	hw_state.bSurfaceOn = TRUE;

	return TRUE;
}



//
//
//
static void GFXDLLCALLCONV Glide_UnlockLfb( void ) {

	if( hw_state.bSurfaceOn == FALSE )
		return;

	if( lfb_flag == LS_WRITE ) {
		pGrLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER );
	}
	else
	if( lfb_flag == LS_READ )
		pGrLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER );

	hw_state.bSurfaceOn = FALSE;

	return;
}





//
//
//
static int GetRes( int SCREENW, int SCREENH, int bpp ) {

	     if( SCREENW==320 && SCREENH==200 ) return GR_RESOLUTION_320x200;
	else if( SCREENW==320 && SCREENH==240 ) return GR_RESOLUTION_320x240;
	else if( SCREENW==400 && SCREENH==256 ) return GR_RESOLUTION_400x256;
	else if( SCREENW==400 && SCREENH==300 ) return GR_RESOLUTION_400x300;
	else if( SCREENW==512 && SCREENH==384 ) return GR_RESOLUTION_512x384;
	else if( SCREENW==640 && SCREENH==200 ) return GR_RESOLUTION_640x200;
	else if( SCREENW==640 && SCREENH==350 ) return GR_RESOLUTION_640x350;
	else if( SCREENW==640 && SCREENH==400 ) return GR_RESOLUTION_640x400;
	else if( SCREENW==640 && SCREENH==480 ) return GR_RESOLUTION_640x480;
	else if( SCREENW==800 && SCREENH==600 ) return GR_RESOLUTION_800x600;
	else if( SCREENW==960 && SCREENH==720 ) return GR_RESOLUTION_960x720;
	else if( SCREENW==856 && SCREENH==480 ) return GR_RESOLUTION_856x480;
	else if( SCREENW==512 && SCREENH==256 ) return GR_RESOLUTION_512x256;
	else if( SCREENW==1024 && SCREENH==768 ) return GR_RESOLUTION_1024x768;
	else if( SCREENW==1280 && SCREENH==1024 ) return GR_RESOLUTION_1280x1024;
	else if( SCREENW==1600 && SCREENH==1200 ) return GR_RESOLUTION_1600x1200;

	return GR_RESOLUTION_NONE;
}


int ( GFXDLLCALLCONV *Glide_winReadProfileInt)( char *, int ) = NULL;
void ( GFXDLLCALLCONV *Glide_winWriteProfileInt)( char *, int ) = NULL;

static combobox_t combobox_data;


//
//
//
static BOOL GFXDLLCALLCONV Glide_Init( gfxdlldesc_t *desc ) {

	int i,n,res;
	char *str;
	BOOL mustinit, exist;

	hw_state.hwnd = (__int64)desc->hwnd;
	hw_state.SCREENW = desc->width;
	hw_state.SCREENH = desc->height;
	hw_state.bpp = 16;	// desc->bpp;
	hw_state.bSurfaceOn = FALSE;
	hw_state.hInst = (__int64)desc->hInst;
	hw_state.tex24bit = FALSE;


	dprintf = (void (GFXDLLCALLCONV *)(char *s,...))desc->xprintf;
	dmalloc = (void *(GFXDLLCALLCONV *)(int))desc->malloc;
	drealloc = (void *(GFXDLLCALLCONV *)(void *,int))desc->realloc;
	dfree = (int (GFXDLLCALLCONV *)( void *))desc->free;
	dwinQuit = (BOOL (GFXDLLCALLCONV *)(char *s,...))desc->Quit;
	dGetBackHwnd = (ULONG (GFXDLLCALLCONV *)(void))desc->GetBackHwnd;
	dCheckParm = (int (GFXDLLCALLCONV *)(char *s))desc->CheckParm;
	dTexForNum = (texmap_t *(GFXDLLCALLCONV *)(int texmapid))desc->TexForNum;
	dCopyVector = (void (GFXDLLCALLCONV *)(point3_t dst, point3_t src))desc->CopyVector;
	dDotProduct = (FLOAT (GFXDLLCALLCONV *)(point3_t v1, point3_t v2 ))desc->DotProduct;
	Glide_winReadProfileInt = (int (GFXDLLCALLCONV *)(char*,int))desc->winReadProfileInt;
	Glide_winWriteProfileInt = (void (GFXDLLCALLCONV *)(char*,int))desc->winWriteProfileInt;

	dprintf("init: %s v%s (c) Copyright 1998 bernie (%s %s)\n",GFXDLL_NAME,GFXDLL_VERSION,__DATE__,__TIME__);

	if( LoadGlide() == FALSE )
		return FALSE;

	pGrGlideInit();

	pGrGet( GR_NUM_BOARDS, sizeof(n), (long*)&n );
	if( n == 0 ) {
		sprintf( glide_error_str, "No 3Dfx Voodoo card present." );
		return FALSE;
	}

	dprintf("log: using %s v%s by %s.\n",pGrGetString(GR_RENDERER),pGrGetString( GR_VERSION ),pGrGetString(GR_VENDOR));

	str = (char *)pGrGetString( GR_VERSION );
	if( (int)(str[0]) < (int)('3') )
		dllQuit("Make sure you have at least Glide v3.0 installed (v%s).",str);

//	if( hw_state.bpp != 16 )
//		return FALSE;

	combobox_data.nstrs = 0;
	combobox_data.cursel = 0;

	for( i=0; i<n; i++ ) {

		pGrSstSelect( i );

		pGrConstantColorValue( ~0 );
		pGrLfbConstantDepth( ~0 );

		sprintf( combobox_data.str[ combobox_data.nstrs ][0],"%s",pGrGetString(GR_HARDWARE));
		sprintf( combobox_data.str[ combobox_data.nstrs ][1],"%d",i);

		++combobox_data.nstrs;
	}

	n = Glide_winReadProfileInt( "glide_SST", -1 );

	if( Glide_winReadProfileInt( "gfx_setup", 0 ) > 0 )
		mustinit = FALSE;
	else
		mustinit = TRUE;

	if( dCheckParm("setup") > 0 )
		mustinit = TRUE;

	if( n == -1 )
		mustinit = TRUE;

	exist = FALSE;

	for( i=0; i<combobox_data.nstrs; i++ )
		if( atoi(combobox_data.str[ i ][1]) == n ) {
			combobox_data.cursel = i;
			exist = TRUE;
			break;
		}

	if( (exist == FALSE) || (mustinit == TRUE) )
		if( Glide_winComboBox( &combobox_data ) == -1 )
			dllQuit(NULL);

	n = atoi( combobox_data.str[ combobox_data.cursel ][1] );

	Glide_winWriteProfileInt( "glide_SST", n );

	pGrSstSelect( n );

	/* resolution
	GrResolution resTemplate;
	GrResolution *presSupported;
	FxI32 size,

	resTemplate.resolution = GR_QUERY_ANY;
	resTemplate.refresh = GR_QUERY_ANY;
	resTemplate.numColorBuffers = GR_QUERY_ANY;
	resTemplate.numAuxBuffers = GR_QUERY_ANY;

	size = grQueryResolutions ( &resTemplate, NULL );
	presSupported = ( GrResolution * ) dmalloc ( size );

	size = size / sizeof ( GrResolution );
	grQueryResolutions ( &resTemplate, presSupported );

	dfree( presSupported );
	*/

	pGrConstantColorValue( ~0 );
	pGrLfbConstantDepth( ~0 );

	dprintf("using: %s\n",pGrGetString(GR_HARDWARE));

	if( (res = GetRes( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp )) == GR_RESOLUTION_NONE ) {
		sprintf( glide_error_str, "No requested %d x %d %d bit resolution.", hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp );
		return FALSE;
	}

	hw_state.bZBuffer = 1; // ( (hw_state.SCREENW>640) || (hw_state.SCREENH>480) ) ? 0 : 1;

	if( (hw_state.context = pGrSstWinOpen( (ULONG)hw_state.hwnd, res, GR_REFRESH_60Hz,
						GR_COLORFORMAT_ABGR, GR_ORIGIN_UPPER_LEFT,
						2,	/* double or tripple buffering */
						hw_state.bZBuffer	/* num_aux_buff: 0,1 */ )) == FXFALSE ) {
		sprintf( glide_error_str, "init: grSstWinOpen phailed for %d x %d %d bit resolution.", hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp );
		return FALSE;
	}


	if( hw_state.bZBuffer ) {

		int depth;

		pGrGet( GR_BITS_DEPTH, sizeof(int), (long*)&depth );
		hw_state.zmax = (FLOAT)pow(2.0f,depth);
		dprintf("init: %d bit Z buffer.\n",depth);
	}
	else
		hw_state.zmax = (FLOAT)0xffff;

	hw_state.zmax *= 0.90f;


	pGrRenderBuffer( GR_BUFFER_BACKBUFFER );

	pGrCoordinateSpace( GR_WINDOW_COORDS );
	pGrClipWindow( 0,0, hw_state.SCREENW, hw_state.SCREENH );

	//grCoordinateSpace( GR_CLIP_COORDS );
	//grViewport( 0,0, hw_state.SCREENW, hw_state.SCREENH );
	//grDepthRange( 0.0f, 1.0f );


	pGrVertexLayout(GR_PARAM_XY,  GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
	pGrVertexLayout(GR_PARAM_RGB, GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);
	pGrVertexLayout(GR_PARAM_A,   GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);
	// pGrVertexLayout(GR_PARAM_Z,   GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);

	pGrVertexLayout(GR_PARAM_Q,   GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
	//pGrVertexLayout(GR_PARAM_W,   GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);

	pGrVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
	pGrVertexLayout(GR_PARAM_Q0,  GR_VERTEX_OOW_TMU0_OFFSET << 2, GR_PARAM_DISABLE);
	pGrVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
	pGrVertexLayout(GR_PARAM_Q1,  GR_VERTEX_OOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);


	lfbinfo.size = sizeof( GrLfbInfo_t );
	if( pGrLfbLock( LFBMODE, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT, FXFALSE, &lfbinfo ) == FXFALSE )
		dllQuit("Glide_Init: grLfbLock failed." );

	pGrLfbUnlock( LFBMODE, GR_BUFFER_BACKBUFFER );

	//nextTexture = grTexMinAddress(GR_TMU0);
	//lastTexture = grTexMaxAddress(GR_TMU0);

	nglidetextures = 0;
	alloced_texmap = 0;
	glidetexture = NULL;

	glide_cache_init();

	hw_state.cullmode = 1;
	pGrCullMode( GR_CULL_POSITIVE );

	pGrDitherMode( GR_DITHER_4x4 );

	pGrTexMipMapMode( GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE );
	pGrTexFilterMode( GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR );
	pGrTexClampMode( GR_TMU0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP );	// GR_TEXTURECLAMP_WRAP GR_TEXTURECLAMP_CLAMP
	pGrColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
	pGrTexCombine( GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
	pGrAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
	pGrAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );

	hw_state.mode = HM_TEXTURE;

	if( hw_state.bZBuffer ) {

		/* enable oow depth buffering */

		pGrDepthBufferMode( GR_DEPTHBUFFER_WBUFFER );
		pGrDepthBufferFunction( GR_CMP_LEQUAL );
		pGrDepthMask( FXTRUE );
	}

	hw_state.zbuffermode = TRUE;

	/* Set hint to separate w for depth and tex */
	//grHints( GR_HINT_STWHINT, GR_STWHINT_W_DIFF_TMU0 );
	/* oow for the texture coords will be set to one
	   in effect disabling perspective correction */

	hw_state.texmap = -1;
	hw_state.mode = HM_TEXTURE;

	desc->bZBuffer = TRUE;

	desc->rshift = 11;
	desc->gshift = 5;
	desc->bshift = 0;

	desc->rmask = 0x1f;
	desc->gmask = 0x3f;
	desc->bmask = 0x1f;

	desc->rsize = 5;
	desc->gsize = 6;
	desc->bsize = 5;

	pGrEnable( GR_PASSTHRU );

	bInGraphic = FALSE;

	// MoveWindow( (HWND)hw_state.hwnd, GetSystemMetrics( SM_CXSCREEN ) - 100,GetSystemMetrics( SM_CYSCREEN )-100, 100, 100, TRUE );

	ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
	MoveWindow( (HWND)hw_state.hwnd, 0,0, hw_state.SCREENW, hw_state.SCREENH, TRUE );

	return TRUE;
}






//
//
//
static void GFXDLLCALLCONV Glide_Deinit( void ) {

	Glide_DiscardAllTexture();

	if( pGrFinish ) pGrFinish();

	if( pGrDisable ) pGrDisable( GR_PASSTHRU );

	if( pGrSstWinClose ) pGrSstWinClose( hw_state.context );

	glide_cache_uninit();

	if( pGrGlideShutdown ) pGrGlideShutdown();

	FreeGlide();

	bInGraphic = FALSE;

	if( dprintf ) dprintf("deinit: %s success.\n",GFXDLL_NAME);

	return;
}





//
//
//
static ULONG GFXDLLCALLCONV Glide_GetPitch( void ) {

	return lfbinfo.strideInBytes;
}





//
//
//
static BOOL GFXDLLCALLCONV Glide_Activate( int flag ) {

	if( flag <= 0 )
		if( pGrDisable ) pGrDisable( GR_PASSTHRU );
	else
		if( pGrEnable ) pGrEnable( GR_PASSTHRU );

	return TRUE;
}







//
//
//
static void GFXDLLCALLCONV Glide_FlipPage( void ) {

	#if 0
	{
	    static int frame = 1;
	    pGrSplash( (FLOAT)0, (FLOAT)hw_state.SCREENH/2, (FLOAT)hw_state.SCREENW/2, (FLOAT)hw_state.SCREENH/2, frame++ );
	}
	#endif

	// while( grBufferNumPending() > 3);

	pGrBufferSwap( 1 );	// 0 == no VB

	return;
}







//
//
//
static void GFXDLLCALLCONV Glide_Clear( ULONG color, ULONG depth, int flag ) {

	ULONG hw_color;

	if( flag & (CF_TARGET | CF_ZBUFFER) ) {

		hw_color  = (( color >> 16 ) & 0xff) <<  0;
		hw_color += (( color >>  8 ) & 0xff) <<  8;
		hw_color += (( color >>  0 ) & 0xff) << 16;

		pGrBufferClear( hw_color, 0, (USHORT)depth );
	}
	else
	if( flag & CF_ZBUFFER ) {

		pGrColorMask( FALSE, FALSE );

		pGrBufferClear( 0, 0, (USHORT)depth );

		pGrColorMask( TRUE, TRUE );
	}
	else
	if( flag & CF_TARGET ) {

		hw_color  = (( color >> 16 ) & 0xff) <<  0;
		hw_color += (( color >>  8 ) & 0xff) <<  8;
		hw_color += (( color >>  0 ) & 0xff) << 16;

		pGrDepthMask( FALSE );

		pGrBufferClear( hw_color, 0, (USHORT)depth );

		pGrDepthMask( TRUE );
	}

	return;
}








//
//
//
static void GFXDLLCALLCONV Glide_Fog( FLOAT fognear, FLOAT fogfar, ULONG fogcolor ) {

#if 0
	#define FOG_DENSITY .5

	GrFog_t fog[GR_FOG_TABLE_SIZE];
	int i,scale;
	int r,g,b;

	/***
	scale = (int)(fognear*(FLOAT)GR_FOG_TABLE_SIZE/fogfar);

	for( i=0; i<scale; i++ )
		fog[i] = 0;

	for( i=scale; i<GR_FOG_TABLE_SIZE; i++ )
		fog[i] = ((i-scale)*255)/(GR_FOG_TABLE_SIZE-scale);

	fog[GR_FOG_TABLE_SIZE-1] = 255;
	***/

	#define DENS 63 		// 63
	#define VEG 59			// 59

	for( i=0; i<VEG; i++ )
		fog[i] = 0;

	for( i=VEG; i<GR_FOG_TABLE_SIZE; i++ ) {
		fog[i] = (i-VEG)*DENS;
		CLAMPMINMAX( fog[i], 0, 255 );
	}

	pGrFogMode( GR_FOG_WITH_TABLE );

	pGrFogTable( fog );

	//grFogColorValue( fogcolor );

	r = ( fogcolor >> 16 ) & 0xff;
	g = ( fogcolor >>  8 ) & 0xff;
	b = ( fogcolor >>  0 ) & 0xff;

	pGrFogColorValue( b * 65536 + g * 256 + r );

#endif

	int nFog,i;
	GrFog_t *fog;
	int r,g,b;

	pGrGet( GR_FOG_TABLE_ENTRIES, 4, (long*)&nFog );
	if( (fog = (GrFog_t*)dmalloc( nFog * sizeof(GrFog_t) )) == NULL )
		return;

	#define DENS 48 		// 63
	#define VEG 59			// 59

	for( i=0; i<VEG; i++ )
		fog[i] = 0;

	for( i=VEG; i<nFog; i++ ) {
		fog[i] = (i-VEG)*DENS;
		CLAMPMINMAX( fog[i], 0, 255 );
	}

	pGrFogTable(fog);

	r = ( fogcolor >> 16 ) & 0xff;
	g = ( fogcolor >>  8 ) & 0xff;
	b = ( fogcolor >>  0 ) & 0xff;

	pGrFogColorValue( b * 65536 + g * 256 + r );

	pGrFogMode(GR_FOG_WITH_TABLE_ON_Q);

	dfree( fog );

	return;
}




FxU32 ( FX_CALL *pGrTexMinAddress)( GrChipID_t tmu ) = NULL;
FxU32 ( FX_CALL *pGrTexMaxAddress)( GrChipID_t tmu ) = NULL;
void ( FX_CALL *pGrColorCombine)( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert ) = NULL;
FxU32 ( FX_CALL *pGrTexTextureMemRequired)( FxU32 evenOdd, GrTexInfo *info ) = NULL;
void ( FX_CALL *pGrTexDownloadMipMap)( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info ) = NULL;
void ( FX_CALL *pGrTexSource)( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info ) = NULL;
void ( FX_CALL *pGrTexCombine)( GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert ) = NULL;
void ( FX_CALL *pGrChromakeyMode)( GrChromakeyMode_t mode ) = NULL;
void ( FX_CALL *pGrChromakeyValue)( GrColor_t value ) = NULL;
void ( FX_CALL *pGrAlphaBlendFunction)( GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df ) = NULL;
void ( FX_CALL *pGrDepthMask)( FxBool mask ) = NULL;
void ( FX_CALL *pGrAlphaCombine)( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert ) = NULL;
void ( FX_CALL *pGrCullMode)( GrCullMode_t mode ) = NULL;
void ( FX_CALL *pGrDepthBufferFunction)( GrCmpFnc_t function ) = NULL;
void ( FX_CALL *pGrDepthBufferMode)( GrDepthBufferMode_t mode ) = NULL;
void ( FX_CALL *pGrDrawVertexArrayContiguous)( FxU32 mode, FxU32 Count, void *pointers, FxU32 stride ) = NULL;
void ( FX_CALL *pGrFinish)(void) = NULL;
void ( FX_CALL *pGrFlush)(void) = NULL;
FxBool ( FX_CALL *pGrLfbLock)( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t *info ) = NULL;
FxBool ( FX_CALL *pGrLfbUnlock)( GrLock_t type, GrBuffer_t buffer ) = NULL;
void ( FX_CALL *pGrGlideInit)( void ) = NULL;
void ( FX_CALL *pGrGlideShutdown)( void ) = NULL;
FxU32 ( FX_CALL *pGrGet)( FxU32 pname, FxU32 plength, FxI32 *params ) = NULL;
const char * ( FX_CALL *pGrGetString)( FxU32 pname ) = NULL;
void ( FX_CALL *pGrSstSelect)( int which_sst ) = NULL;
void ( FX_CALL *pGrConstantColorValue)( GrColor_t value ) = NULL;
void ( FX_CALL *pGrLfbConstantDepth)( FxU32 depth ) = NULL;
GrContext_t ( FX_CALL *pGrSstWinOpen)( FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers ) = NULL;
FxBool ( FX_CALL *pGrSstWinClose)( GrContext_t context ) = NULL;
void ( FX_CALL *pGrRenderBuffer)( GrBuffer_t buffer ) = NULL;
void ( FX_CALL *pGrCoordinateSpace)( GrCoordinateSpaceMode_t mode ) = NULL;
void ( FX_CALL *pGrClipWindow)( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy ) = NULL;
void ( FX_CALL *pGrVertexLayout)(FxU32 param, FxI32 offset, FxU32 mode) = NULL;
void ( FX_CALL *pGrDitherMode)( GrDitherMode_t mode ) = NULL;
void ( FX_CALL *pGrTexMipMapMode)( GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend ) = NULL;
void ( FX_CALL *pGrTexFilterMode)( GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode ) = NULL;
void ( FX_CALL *pGrTexClampMode)( GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode ) = NULL;
void ( FX_CALL *pGrEnable)( GrEnableMode_t mode ) = NULL;
void ( FX_CALL *pGrDisable)( GrEnableMode_t mode ) = NULL;
void ( FX_CALL *pGrSplash)(float x, float y, float width, float height, FxU32 frame) = NULL;
void ( FX_CALL *pGrBufferSwap)( FxU32 swap_interval ) = NULL;
void ( FX_CALL *pGrBufferClear)( GrColor_t color, GrAlpha_t alpha, FxU32 depth ) = NULL;
void ( FX_CALL *pGrColorMask)( FxBool rgb, FxBool a ) = NULL;
void ( FX_CALL *pGrFogColorValue)( GrColor_t fogcolor ) = NULL;
void ( FX_CALL *pGrFogMode)( GrFogMode_t mode ) = NULL;
void ( FX_CALL *pGrFogTable)( const GrFog_t ft[] ) = NULL;



#define GLIDEDEF(f) (WINAPI *(f))		// define the functions as pointers

static HINSTANCE glide = 0;			// bass handle


//
//
//
static BOOL errstr( char *funcName ) {

	dprintf( "LoadGlide: no \"%s\" function.\n", funcName );

	sprintf( glide_error_str, "LoadGlide: can't find \"%s\" function in \"%s\" library.\n\nPlease upgrade your 3Dfx Glide drivers to at least 3.x version.", funcName, GLIDEDLLNAME );

	return FALSE;
}


//
// load BASS and the required functions
//
BOOL LoadGlide( void ) {

	if( (glide = LoadLibrary(GLIDEDLLNAME)) == NULL ) {
		dprintf("LoadGlide: couldn't load %s library.\n",GLIDEDLLNAME);
		sprintf( glide_error_str, "You don't have a 3Dfx Voodoo card installed. Or don't have Glide drivers correctly installed.\n\nCan't load \"%s\" library. Please upgrade your 3Dfx Glide dirvers to at least 3.x version.", GLIDEDLLNAME );
		return FALSE;
	}

	if( (pGrColorCombine = (void (FX_CALL *)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert))GetProcAddress( glide, "_grColorCombine@20" )) == NULL ) return errstr( "grColorCombine" );
	if( (pGrTexMinAddress = (FxU32 (FX_CALL *)(GrChipID_t tmu))GetProcAddress( glide, "_grTexMinAddress@4" )) == NULL ) return errstr( "grTexMinAddress" );
	if( (pGrTexMaxAddress = (FxU32 (FX_CALL *)(GrChipID_t tmu))GetProcAddress( glide, "_grTexMaxAddress@4" )) == NULL ) return errstr( "grTexMaxAddress" );
	if( (pGrTexTextureMemRequired = (FxU32 (FX_CALL *)(FxU32 evenOdd, GrTexInfo *info))GetProcAddress( glide, "_grTexTextureMemRequired@8" )) == NULL ) return errstr( "grTexTextureMemRequired" );
	if( (pGrTexDownloadMipMap = (void (FX_CALL *)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info ))GetProcAddress( glide, "_grTexDownloadMipMap@16" )) == NULL ) return errstr( "grTexDownloadMipMap" );
	if( (pGrTexSource = (void (FX_CALL *)( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info  ))GetProcAddress( glide, "_grTexSource@16" )) == NULL ) return errstr( "grTexSource" );
	if( (pGrTexCombine = (void (FX_CALL *)( GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert ))GetProcAddress( glide, "_grTexCombine@28" )) == NULL ) return errstr( "grTexCombine" );
	if( (pGrChromakeyMode = (void (FX_CALL *)( GrChromakeyMode_t mode ))GetProcAddress( glide, "_grChromakeyMode@4" )) == NULL ) return errstr( "grChromakeyMode" );
	if( (pGrChromakeyValue = (void (FX_CALL *)( GrColor_t value ))GetProcAddress( glide, "_grChromakeyValue@4" )) == NULL ) return errstr( "grChromakeyValue" );
	if( (pGrAlphaBlendFunction = (void (FX_CALL *)( GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df ))GetProcAddress( glide, "_grAlphaBlendFunction@16" )) == NULL ) return errstr( "grAlphaBlendFunction" );
	if( (pGrDepthMask = (void (FX_CALL *)( FxBool mask ))GetProcAddress( glide, "_grDepthMask@4" )) == NULL ) return errstr( "grDepthMask" );
	if( (pGrAlphaCombine = (void (FX_CALL *)( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert ))GetProcAddress( glide, "_grAlphaCombine@20" )) == NULL ) return errstr( "grAlphaCombine" );
	if( (pGrCullMode = (void (FX_CALL *)( GrCullMode_t mode ))GetProcAddress( glide, "_grCullMode@4" )) == NULL ) return errstr( "grCullMode" );
	if( (pGrDepthBufferFunction = (void (FX_CALL *)( GrCmpFnc_t function ))GetProcAddress( glide, "_grDepthBufferFunction@4" )) == NULL ) return errstr( "grDepthBufferFunction" );
	if( (pGrDepthBufferMode = (void (FX_CALL *)( GrDepthBufferMode_t mode ))GetProcAddress( glide, "_grDepthBufferMode@4" )) == NULL ) return errstr( "grDepthBufferMode" );
	if( (pGrDrawVertexArrayContiguous = (void (FX_CALL *)( FxU32 mode, FxU32 Count, void *pointers, FxU32 stride ))GetProcAddress( glide, "_grDrawVertexArrayContiguous@16" )) == NULL ) return errstr( "grDrawVertexArrayContiguous" );
	if( (pGrFinish = (void (FX_CALL *)( void ))GetProcAddress( glide, "_grFinish@0" )) == NULL ) return errstr( "grFinish" );
	if( (pGrFlush = (void (FX_CALL *)( void  ))GetProcAddress( glide, "_grFlush@0" )) == NULL ) return errstr( "grFlush" );
	if( (pGrLfbLock = (FxBool (FX_CALL *)( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t *info ))GetProcAddress( glide, "_grLfbLock@24" )) == NULL ) return errstr( "grLfbLock" );
	if( (pGrLfbUnlock = (FxBool (FX_CALL *)( GrLock_t type, GrBuffer_t buffer ))GetProcAddress( glide, "_grLfbUnlock@8" )) == NULL ) return errstr( "grLfbUnlock" );
	if( (pGrGlideInit = (void (FX_CALL *)( void ))GetProcAddress( glide, "_grGlideInit@0" )) == NULL ) return errstr( "grGlideInit" );
	if( (pGrGlideShutdown = (void (FX_CALL *)( void ))GetProcAddress( glide, "_grGlideShutdown@0" )) == NULL ) return errstr( "grGlideShutdown" );
	if( (pGrGet = (FxU32 (FX_CALL *)( FxU32 pname, FxU32 plength, FxI32 *params ))GetProcAddress( glide, "_grGet@12" )) == NULL ) return errstr( "grGet" );
	if( (pGrGetString = (const char *(FX_CALL *)( FxU32 pname ))GetProcAddress( glide, "_grGetString@4" )) == NULL ) return errstr( "grGetString" );
	if( (pGrSstSelect = (void (FX_CALL *)( int ))GetProcAddress( glide, "_grSstSelect@4" )) == NULL ) return errstr( "grSstSelect" );
	if( (pGrConstantColorValue = (void (FX_CALL *)( GrColor_t value ))GetProcAddress( glide, "_grConstantColorValue@4" )) == NULL ) return errstr( "grConstantColorValue" );
	if( (pGrLfbConstantDepth = (void (FX_CALL *)( FxU32 depth ))GetProcAddress( glide, "_grLfbConstantDepth@4" )) == NULL ) return errstr( "grLfbConstantDepth" );
	if( (pGrSstWinOpen = (GrContext_t (FX_CALL *)( FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers ))GetProcAddress( glide, "_grSstWinOpen@28" )) == NULL ) return errstr( "grSstWinOpen" );
	if( (pGrSstWinClose = (FxBool (FX_CALL *)( GrContext_t context ))GetProcAddress( glide, "_grSstWinClose@4" )) == NULL ) return errstr( "grSstWinClose" );
	if( (pGrRenderBuffer = (void (FX_CALL *)( GrBuffer_t buffer ))GetProcAddress( glide, "_grRenderBuffer@4" )) == NULL ) return errstr( "grRenderBuffer" );
	if( (pGrCoordinateSpace = (void (FX_CALL *)( GrCoordinateSpaceMode_t mode ))GetProcAddress( glide, "_grCoordinateSpace@4" )) == NULL ) return errstr( "grCoordinateSpace" );
	if( (pGrClipWindow = (void (FX_CALL *)( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy ))GetProcAddress( glide, "_grClipWindow@16" )) == NULL ) return errstr( "grClipWindow" );
	if( (pGrVertexLayout = (void (FX_CALL *)( FxU32 param, FxI32 offset, FxU32 mode ))GetProcAddress( glide, "_grVertexLayout@12" )) == NULL ) return errstr( "grVertexLayout" );
	if( (pGrDitherMode = (void (FX_CALL *)( GrDitherMode_t mode ))GetProcAddress( glide, "_grDitherMode@4" )) == NULL ) return errstr( "grDitherMode" );
	if( (pGrTexMipMapMode = (void (FX_CALL *)( GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend ))GetProcAddress( glide, "_grTexMipMapMode@12" )) == NULL ) return errstr( "grTexMipMapMode" );
	if( (pGrTexFilterMode = (void (FX_CALL *)( GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode ))GetProcAddress( glide, "_grTexFilterMode@12" )) == NULL ) return errstr( "grTexFilterMode" );
	if( (pGrTexClampMode = (void (FX_CALL *)( GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode ))GetProcAddress( glide, "_grTexClampMode@12" )) == NULL ) return errstr( "grTexClampMode" );
	if( (pGrEnable = (void (FX_CALL *)( GrEnableMode_t mode ))GetProcAddress( glide, "_grEnable@4" )) == NULL ) return errstr( "grEnable" );
	if( (pGrDisable = (void (FX_CALL *)( GrEnableMode_t mode ))GetProcAddress( glide, "_grDisable@4" )) == NULL ) return errstr( "grDisable" );
	if( (pGrSplash = (void (FX_CALL *)( float x, float y, float width, float height, FxU32 frame ))GetProcAddress( glide, "_grSplash@20" )) == NULL ) return errstr( "grSplash" );
	if( (pGrBufferSwap = (void (FX_CALL *)( FxU32 swap_interval ))GetProcAddress( glide, "_grBufferSwap@4" )) == NULL ) return errstr( "grBufferSwap" );
	if( (pGrBufferClear = (void (FX_CALL *)( GrColor_t color, GrAlpha_t alpha, FxU32 depth ))GetProcAddress( glide, "_grBufferClear@12" )) == NULL ) return errstr( "grBufferClear" );
	if( (pGrColorMask = (void (FX_CALL *)( FxBool rgb, FxBool a ))GetProcAddress( glide, "_grColorMask@8" )) == NULL ) return errstr( "grColorMask" );
	if( (pGrFogColorValue = (void (FX_CALL *)( GrColor_t fogcolor ))GetProcAddress( glide, "_grFogColorValue@4" )) == NULL ) return errstr( "grFogColorValue" );
	if( (pGrFogMode = (void (FX_CALL *)( GrFogMode_t mode ))GetProcAddress( glide, "_grFogMode@4" )) == NULL ) return errstr( "grFogMode" );
	if( (pGrFogTable = (void (FX_CALL *)( const GrFog_t ft[] ))GetProcAddress( glide, "_grFogTable@4" )) == NULL ) return errstr( "grFogTable" );

	dprintf("init: Glide (%s) loaded.\n", GLIDEDLLNAME );

	return TRUE;
}



//
//
//
void FreeGlide( void ) {

	if( !glide )
		return;

	pGrEnable = NULL;
	pGrDisable = NULL;

	FreeLibrary(glide);
	glide = NULL;

	// DeleteFile(tempfile);

	return;
}





//
//
//
static void GFXDLLCALLCONV Glide_SetRgb( int color, int red, int green, int blue ) {

	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_InitPalette( UCHAR *pal ) {

	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_GetRgb( int color, int *red, int *green, int *blue ) {


	return;
}


//
//
//
static BOOL GFXDLLCALLCONV Glide_Fullscreen( int flag ) {

	dprintf("Glide_Fullscreen: Glide is fullscreen only.\n");

	return -1;
}



//
//
//
static void GFXDLLCALLCONV Glide_GetDescription( char *str ) {

	sprintf(str,"3Dfx Glide v3 driver v%s",GFXDLL_VERSION);

	return;
}



//
//
//
static void GFXDLLCALLCONV Glide_GetData( void *buf, int len ) {

	if( *(ULONG *)buf == GFXDLL_ISMSGOK )
		*(ULONG *)buf = !bInGraphic;

	if( *(ULONG *)buf == GFXDLL_24BIT )
		*(ULONG *)buf = FALSE;

	if( *(ULONG *)buf == GFXDLL_MAXTEXSIZE )
		*(ULONG *)buf = 256;

	if( *(ULONG *)buf == GFXDLL_MINTEXSIZE )
		*(ULONG *)buf = 64;

	if( *(ULONG *)buf == GFXDLL_GETERROR )
		strncpy( (char *)buf, glide_error_str, len );
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

		HINSTANCE glide = 0;			// bass handle

		if( (glide = LoadLibrary(GLIDEDLLNAME)) == NULL ) {
			*(ULONG *)buf = FALSE;
			sprintf( glide_error_str, "Glide_GetData: didn't detect 3Dfx card or need at least version 3 of Glide (no %s library).", GLIDEDLLNAME );
		}
		else {
			FreeLibrary(glide);
			*(ULONG *)buf = TRUE;
		}
	}

	return;
}





//
//
//
void GFXDLLCALLCONV GFXDRV_GetInfo( GFXDLLinfo_t *info ) {

	info->GFXDLL_SetupCulling	= (void *)Glide_SetupCulling;
	info->GFXDLL_AddToPipeline	= (void *)Glide_AddToPipeline;
	info->GFXDLL_AddTexMapHigh	= (void *)Glide_AddTexMapHigh;
	info->GFXDLL_ReloadTexMapHigh	= (void *)Glide_ReloadTexMapHigh;
	info->GFXDLL_ModifyTexMapHigh	= (void *)Glide_ModifyTexMapHigh;
	info->GFXDLL_PutSpritePoly	= (void *)Glide_PutSpritePoly;
	info->GFXDLL_PutPoly		= (void *)Glide_PutPoly;
	info->GFXDLL_Line		= (void *)Glide_Line;
	info->GFXDLL_PutPixel		= (void *)Glide_PutPixel;
	info->GFXDLL_DiscardAllTexture	= (void *)Glide_DiscardAllTexture;
	info->GFXDLL_DiscardTexture	= (void *)Glide_DiscardTexture;
	info->GFXDLL_BeginScene		= (void *)Glide_BeginScene;
	info->GFXDLL_EndScene		= (void *)Glide_EndScene;
	info->GFXDLL_FlushScene		= (void *)Glide_FlushScene;
	info->GFXDLL_LockLfb		= (void *)Glide_LockLfb;
	info->GFXDLL_UnlockLfb		= (void *)Glide_UnlockLfb;
	info->GFXDLL_Init		= (void *)Glide_Init;
	info->GFXDLL_Deinit		= (void *)Glide_Deinit;
	info->GFXDLL_GetDescription	= (void *)Glide_GetDescription;
	info->GFXDLL_GetData		= (void *)Glide_GetData;
	info->GFXDLL_GetPitch		= (void *)Glide_GetPitch;
	info->GFXDLL_Activate		= (void *)Glide_Activate;
	info->GFXDLL_FlipPage		= (void *)Glide_FlipPage;
	info->GFXDLL_Clear		= (void *)Glide_Clear;
	info->GFXDLL_Fog		= (void *)Glide_Fog;
	info->GFXDLL_InitPalette	= (void *)Glide_InitPalette;
	info->GFXDLL_SetRgb		= (void *)Glide_SetRgb;
	info->GFXDLL_GetRgb		= (void *)Glide_GetRgb;
	info->GFXDLL_Fullscreen		= (void *)Glide_Fullscreen;

	return;
}






//
//
//
extern "C" void dllDeinit( void ) {

	Glide_Deinit();

	return;
}






//
//
//
int dll_init( void ) {

	// SST_RGAMMA,SST_GGAMMA,SST_BGAMMA = 1.0
	// FX_GLIDE_NO_SPLASH = 1

	// putenv("SST_RGAMMA=1.0");
	// putenv("SST_GGAMMA=1.0");
	// putenv("SST_BGAMMA=1.0");
	// putenv("FX_GLIDE_SWAPINTERVAL=0");
	// putenv("FX_GLIDE_NO_SPLASH=1");

	return TRUE;
}


//
//
//
void dll_deinit( void ) {

	return;
}


//
//
//
extern "C" unsigned long dllGetHinst( void ) {

	return (unsigned long)hw_state.hInst;
}

