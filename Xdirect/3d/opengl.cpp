
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <windowsx.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "opengl.h"

#define _EXPORTING

#include <xlib.h>
#include <xinner.h>

#include "dllmain.h"

RCSID( "$Id: opengl.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )

// GLee valami extension
// http://elf-stone.com/glee.php

#define GFXDLL_VERSION "1.0"
char *GFXDLL_NAME = "xopengl.dll";

#pragma comment( exestr , "OpenGL driver v" GFXDLL_VERSION " (" __DATE__ " " __TIME__ ")")

static char opengldllname[XMAX_PATH] = "opengl32.dll";
static char opengl_error_str[512] = "OpenGL: uninitialized OpenGL error string.";


#include "xopengl.h"

#define FOVX (90.0f)

void (*dprintf)( char *s, ... ) = NULL;
texmap_t *(*dTexForNum)( int texmapid ) = NULL;
void (*dCopyVector)( point3_t dst, point3_t src ) = NULL;
FLOAT (*dDotProduct)( point3_t v1, point3_t v2 ) = NULL;
BOOL (*dwinFullscreen)( void ) = NULL;
BOOL (*dwinOpengl)( void ) = NULL;
int (*GL_winReadProfileInt)( char *, int ) = NULL;
void (*GL_winWriteProfileInt)( char *, int ) = NULL;
void (*GL_winReadProfileString)( char *key, char *default_string, char *string, int size ) = NULL;
void (*GL_winWriteProfileString)( char *key, char *string ) = NULL;
ULONG (*dGetHwnd)( void ) = NULL;
ULONG (*dGetHinstance)( void ) = NULL;
ULONG (*dGetBackHwnd)( void ) = NULL;
int (*dfree)( void * ) = NULL;

static BOOL SurfaceOn = FALSE;
static UCHAR *videoptr = NULL;

static BOOL bInGraphic = FALSE;

// options

static opengltexture_t *opengltexture = NULL;
static int nopengltextures = 0;
static int alloced_texmap = 0;

static int texmapid_cnt = 1000;

static BOOL isGouraud = FALSE;

hardwarestate_t hw_state;

int drmask,dgmask,dbmask;
int drsize,dgsize,dbsize;
int drshift,dgshift,dbshift;



static point3_t vpos,		// viewer's position
		vdir,		//  Ä´ÃÄ    direction
		vnormal,	//  Ä´ÃÄ    normal = vpos - vdir;
		mvnormal;	// mvnormal = dir - pos;


static FLOAT gl_zclipnear = FLOAT_EPSILON;
static FLOAT gl_zclipnearP2 = FLOAT_EPSILON;
static FLOAT gl_zclipfar = FLOAT_MAX;
static FLOAT gl_zclipfarP2 = FLOAT_MAX;

FLOAT glfognear,glfogfar,glinvfog;
// point3_t origo3 = { 0.0f, 0.0f, 0.0f };


// trilinear min: GL_LINEAR_MIPMAP_LINEAR
// bilinear min: GL_LINEAR_MIPMAP_NEAREST
static GLfloat magfilter = GL_LINEAR; // GL_NEAREST_MIPMAP_LINEAR;
static GLfloat minfilter = GL_LINEAR; // GL_LINEAR GL_NEAREST GL_NEAREST_MIPMAP_LINEAR

#define GL_VISALPHA		(192)

// #define MAGIC_NUMBER (256.0f)


static int ogl_active_flag = 1;

#define CHECKACTIVE	do { if( ogl_active_flag < 0 ) return; } while(0);
#define CHECKACTIVEN	do { if( ogl_active_flag < 0 ) return FALSE; } while(0);

//
//
//
static BOOL GFXDLLCALLCONV GL_LockLfb( __int64 *mem, int flag ) {

	SurfaceOn = TRUE;

	// glReadPixels( 0,0, hw_state.SCREENW/2, hw_state.SCREENH/2, GL_RGB, GL_UNSIGNED_BYTE, videoptr );

	*mem = (__int64)videoptr;

	return TRUE;
}





//
//
//
static void GFXDLLCALLCONV GL_UnlockLfb( void ) {

	SurfaceOn = FALSE;

	// glDrawPixels( hw_state.SCREENW, hw_state.SCREENH, GL_RGB,GL_UNSIGNED_BYTE, videoptr );

	return;
}






//
//
//
static void GFXDLLCALLCONV GL_SetupCulling( point3_t pos, point3_t dir, point3_t normal, FLOAT zn, FLOAT zf ) {

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

	gl_zclipnear = zn;
	gl_zclipnearP2 = zn*zn;
	gl_zclipfar = zf;
	gl_zclipfarP2 = zf*zf;

	return;
}






//
//
//
static BOOL GFXDLLCALLCONV GL_Rasterize( trans2pipeline_t trans2pipeline ) {

#define P(tag) trans2pipeline.tag

	int r,g,b,a,k;
	int texmapid = -1;
	texmap_t *texmap = NULL;
	opengltexture_t *opengltexmap = NULL;
	FLOAT x,y,z,u,v,dot;
	BOOL alpha = FALSE;
	rgb_t *outrgb = NULL;
	point3_t *outst;

	CHECKACTIVEN;

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

		opengltexmap = NULL;

		for( k=0; k<nopengltextures; k++ )
			if( opengltexture[k].texmapid == (GLuint)texmap->hwtexmapid ) {
				opengltexmap = &opengltexture[k];
				break;
			}

		if( opengltexmap == NULL )
			return TRUE;

		if( hw_state.mode != HM_TEXTURE ) {

			glEnable( GL_TEXTURE_2D );
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );

							// GL_ADD GL_MODULATE
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			hw_state.texmapid = -9;
			hw_state.mode = HM_TEXTURE;
		}

		if( hw_state.texmapid != texmapid ) {
			glBindTexture( GL_TEXTURE_2D, opengltexmap->texmapid );
			hw_state.texmapid = texmapid;
		}

		if( texmap->isChromaKey ) {
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0 );
		}

		if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {

			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.1f);

			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		if( texmap->isAlpha ) {
			glDepthMask( GL_FALSE );
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE );
		}
	}
	else
	if( ISFLAG( P(face)->flag, FF_COLOR ) ) {

		// fillezett

		if( hw_state.mode != HM_COLOR ) {

			glDisable( GL_TEXTURE_2D );

			hw_state.texmapid = -10;
			hw_state.mode = HM_COLOR;
		}
	}
	else
		return TRUE;

	// fog
	for( k=0; k<P(ppoly).npoints; k++ ) {
		if( P(tpoly).point[k][2] > glfognear && !ISFLAG( P(face)->flag, FF_FARTHEST ) ) {
			alpha = TRUE;
			if( P(tpoly).point[k][2] > glfogfar )
				outrgb[k].a = 0;
			else
				outrgb[k].a -= (int)((P(tpoly).point[k][2] - glfognear) * (FLOAT)outrgb[k].a / (glfogfar - glfognear));
		}
	}

	// a szinekben meagadott alpha

	if( alpha == FALSE )
	for( k=0; k<P(ppoly).npoints; k++ )
		if( outrgb[k].a < 255 ) {
			alpha = TRUE;
			// dprintf("true\n");
			break;
		}

	if( alpha == TRUE ) {
		glEnable( GL_BLEND );
		if( (texmap != NULL) && (texmap->isAlpha == TRUE) )
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		else
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	// szinek vege


	if( ISFLAG( P(face)->flag, FF_DBLSIDE ) || ISFLAG( P(face)->flag, FF_FORCEFLAG ) ) {
		if( hw_state.cullmode != 0 ) {
			glDisable( GL_CULL_FACE );
			hw_state.cullmode = 0;
		}
	}
	else {
		if( hw_state.cullmode != 1 ) {
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
			hw_state.cullmode = 1;
		}
	}


	if( ISFLAG( P(face)->flag, FF_FARTHEST ) ) {
		if( hw_state.zbuffermode == TRUE ) {
			glDisable( GL_DEPTH_TEST );
			glDepthMask( GL_FALSE );
			hw_state.zbuffermode = FALSE;
		}
	}
	else {
		if( hw_state.zbuffermode == FALSE ) {
			glEnable( GL_DEPTH_TEST );
			glDepthFunc( GL_LEQUAL );  /* Superimpose smaller Z values over larger ones */
			if( (texmap == NULL) || (texmap->isAlpha == FALSE) )
				glDepthMask( GL_TRUE );
			hw_state.zbuffermode = TRUE;
		}
	}

	/*** multitexture
	glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x][y]);
	glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x][y]);
	glVertex3fv(_sommet[x][y]);
	***/

	FLOAT inv255 = 1.0f / 256.0f;
	FLOAT invZfar = 1.0f / gl_zclipfar;
	FLOAT equ255 = 256.0f;
	FLOAT ooz;
	FLOAT invU;
	FLOAT invV;

	if( texmap ) {
	if( texmap->width1 == texmap->height1 ) {
		invU = inv255;
		invV = inv255;
	}
	if( texmap->width1 > texmap->height1 ) {
		invU = 1.0f / (FLOAT)texmap->width1;
		invV = 1.0f / (((FLOAT)texmap->height1 * equ255) / (FLOAT)texmap->width1);
	}
	else
	if( texmap->width1 < texmap->height1 ) {
		invU = 1.0f / (((FLOAT)texmap->width1 * equ255) / (FLOAT)texmap->height1);
		invV = 1.0f / (FLOAT)texmap->height1;
	}}

	// TODO: torzulnak a texturák
	glBegin(GL_TRIANGLE_FAN); // GL_POLYGON GL_QUADS GL_TRIANGLE_FAN

	for( k=0; k<P(ppoly).npoints; k++ ) {

		x = (FLOAT)( P(ppoly).point[k][0] );
		y = (FLOAT)( P(ppoly).point[k][1] );
		z = P(tpoly).point[k][2] * invZfar;

		r = outrgb[k].r;
		g = outrgb[k].g;
		b = outrgb[k].b;
		a = outrgb[k].a;

		if( FABS(z) < FLOAT_EPSILON )
			ooz = (1.0f / FLOAT_EPSILON);
		else
			ooz = (1.0f / z);

		if( texmapid != -1 ) { // ISFLAG( P(face)->flag, FF_TEXTURE ) ) {

			u = outst[k][0] * invU;
			v = 1.0f - ( outst[k][1] * invV );

			// CLAMPMINMAX( u, 0.0f, 1.0f );
			// CLAMPMINMAX( v, 0.0f, 1.0f );

			// glTexCoord2f( u, v );
			glTexCoord4f( u*ooz, v*ooz, 0.0f, ooz );
		}
		else
		if( isGouraud == TRUE ) {

			#define ADDCOLOR ( 100 )
			#define MAXCOLOR ( 256 - ADDCOLOR )

			dot = -dDotProduct( vnormal, trans2pipeline.outpointnormal[k] );
			if( ISFLAG( P(face)->flag, FF_DBLSIDE ) )
				dot = ABS(dot);
			r = ADDCOLOR + ftoi((FLOAT)MAXCOLOR * ((dot<0.0f)?0.0f:dot));
			g = r;
			b = r;
			a = outrgb[k].a;
		}

		CLAMPMINMAX( r, 0, 255 );
		CLAMPMINMAX( g, 0, 255 );
		CLAMPMINMAX( b, 0, 255 );
		CLAMPMINMAX( a, 0, 255 );

		glColor4f( (FLOAT)r * inv255, (FLOAT)g * inv255, (FLOAT)b * inv255, (FLOAT)a * inv255 );

		glVertex3f( x, y, z );
		// glVertex4f( x*ooz, y*ooz, z*ooz, ooz );
	}

	glEnd();

	if( texmap ) {

		if( texmap->isAlpha ) {

			glDisable( GL_BLEND );

			if( hw_state.zbuffermode == TRUE )
				glDepthMask( GL_TRUE );
		}

		if( texmap->isChromaKey ) {

			glDisable( GL_ALPHA_TEST );
			// glDisable( GL_BLEND );
			// glBlendFunc( GL_ONE, GL_ZERO );
		}

		if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {
			glDisable( GL_BLEND );
			glDisable( GL_ALPHA_TEST );
		}
	}

	if( alpha == TRUE ) {
		glDisable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ZERO );
	}

#undef P

	return TRUE;
}



//
//
//
static BOOL GFXDLLCALLCONV GL_AddToPipeline( trans2pipeline_t trans2pipeline ) {

#define P(tag) trans2pipeline.tag

	P(faceid) = 0;
	GL_Rasterize( trans2pipeline );

	if( ISFLAG( P(face)->flag, FF_ENVMAP) ) {
		int i;
		P(faceid) = 1;
		for( i=0; i<P(ppoly).npoints; i++ ) {
			P(outrgb1)[i].r = P(outrgb)[i].r;
			P(outrgb1)[i].g = P(outrgb)[i].g;
			P(outrgb1)[i].b = P(outrgb)[i].b;
		}
		GL_Rasterize( trans2pipeline );
	}

#undef P

	return TRUE;
}



//
//
//
static void GFXDLLCALLCONV GL_PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) {

	int k;
	FLOAT r,g,b,a;
	texmap_t *texmap = NULL;
	opengltexture_t *opengltexmap = NULL;
	FLOAT x,y,z,u,v;
	BOOL alpha = FALSE;

	CHECKACTIVE;

	// texturazott

	if( (texmap = dTexForNum(texmapid)) == NULL )
		return;

	opengltexmap = NULL;

	for( k=0; k<nopengltextures; k++ )
		if( opengltexture[k].texmapid == (GLuint)texmap->hwtexmapid ) {
			opengltexmap = &opengltexture[k];
			break;
		}

	if( opengltexmap == NULL )
		return;

	if( hw_state.mode != HM_TEXTURE ) {

		glEnable( GL_TEXTURE_2D );
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );
		glDisable( GL_TEXTURE_GEN_R );
		glDisable( GL_TEXTURE_GEN_Q );

		hw_state.texmapid = -9;
		hw_state.mode = HM_TEXTURE;
	}

	if( hw_state.texmapid != texmapid ) {
		glBindTexture( GL_TEXTURE_2D, opengltexmap->texmapid );
		hw_state.texmapid = texmapid;
	}

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter );

	if( texmap->isChromaKey ) {

		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_GREATER, 0 );
		// glEnable( GL_BLEND );
		// glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.1f);

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	if( texmap->isAlpha ) {

		glDepthMask( GL_FALSE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
	}

	if( hw_state.cullmode != 0 ) {
		glDisable( GL_CULL_FACE );
		hw_state.cullmode = 0;
	}

	// a szinekben meagadott alpha

	for( k=0; k<ppoly.npoints; k++ )
		if( rgb[k].a < 255 ) {
			alpha = TRUE;
			break;
		}

	if( alpha == TRUE ) {
		glEnable( GL_BLEND );
		if( (texmap != NULL) && (texmap->isAlpha == TRUE) )
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		else
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	/***
	if( hw_state.zbuffermode == TRUE ) {
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );
		hw_state.zbuffermode = FALSE;
	}
	***/

glDepthMask( GL_FALSE );

	FLOAT inv255 = 1.0f / 256.0f;
	FLOAT equ255 = 256.0f;
	FLOAT invU;
	FLOAT invV;

	if( texmap->width1 == texmap->height1 ) {
		invU = inv255;
		invV = inv255;
	}
	else
	if( texmap->width1 > texmap->height1 ) {
		invU = 1.0f / (FLOAT)texmap->width1;
		invV = 1.0f / (((FLOAT)texmap->height1 * equ255) / (FLOAT)texmap->width1);
	}
	else
	if( texmap->width1 < texmap->height1 ) {
		invU = 1.0f / (((FLOAT)texmap->width1 * equ255) / (FLOAT)texmap->height1);
		invV = 1.0f / (FLOAT)texmap->height1;
	}

	glBegin(GL_POLYGON); // GL_POLYGON GL_QUADS GL_TRIANGLE_FAN

	for( k=0; k<ppoly.npoints; k++ ) {

		r = (FLOAT)rgb[k].r * inv255;
		g = (FLOAT)rgb[k].g * inv255;
		b = (FLOAT)rgb[k].b * inv255;
		a = (FLOAT)rgb[k].a * inv255;

		CLAMPMINMAX( r, 0.0f, 1.0f );
		CLAMPMINMAX( g, 0.0f, 1.0f );
		CLAMPMINMAX( b, 0.0f, 1.0f );
		CLAMPMINMAX( a, 0.0f, 1.0f );

		glColor4f( r,g,b,a );

		x = (FLOAT)( ppoly.point[k][0] );
		y = (FLOAT)( ppoly.point[k][1] );
		z = FLOAT_EPSILON;

		u = inst[k][0] * invU;
		v = 1.0f - (inst[k][1] * invV);

		// CLAMPMINMAX( u, 0.0f, 1.0f );
		// CLAMPMINMAX( v, 0.0f, 1.0f );

		glTexCoord4f( u, v, 0.0f, 1.0f );

		glVertex3f( x, y, z );
	}

	glEnd();

glDepthMask( GL_TRUE );

	if( texmap->isAlpha ) {

		glDisable( GL_BLEND );

		if( hw_state.zbuffermode == TRUE )
			glDepthMask( GL_TRUE );
	}

	if( texmap->isChromaKey ) {

		glDisable( GL_ALPHA_TEST );
		// glDisable( GL_BLEND );
		// glBlendFunc( GL_ONE, GL_ZERO );
	}

	if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {
		glDisable( GL_BLEND );
		glDisable( GL_ALPHA_TEST );
	}


	if( alpha == TRUE ) {
		glDisable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ZERO );
	}

	return;
}



//
//
//
static void GFXDLLCALLCONV GL_PutPoly( polygon_t ppoly, rgb_t *rgb ) {

	int k;
	FLOAT r,g,b,a;
	FLOAT x,y,z;
	BOOL alpha = FALSE;

	CHECKACTIVE;

	if( hw_state.mode != HM_COLOR ) {

		glDisable( GL_TEXTURE_2D );

		hw_state.texmapid = -10;
		hw_state.mode = HM_COLOR;
	}

	if( hw_state.cullmode != 0 ) {
		glDisable( GL_CULL_FACE );
		hw_state.cullmode = 0;
	}

	// a szinekben meagadott alpha

	for( k=0; k<ppoly.npoints; k++ )
		if( rgb[k].a < 255 ) {
			alpha = TRUE;
			break;
		}

	if( alpha == TRUE ) {
		glEnable( GL_BLEND );
		// glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	/***
	if( hw_state.zbuffermode == TRUE ) {
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );
		hw_state.zbuffermode = FALSE;
	}
	***/

glDepthMask( GL_FALSE );

	FLOAT inv255 = 1.0f / 256.0f;

	glBegin(GL_POLYGON); // GL_POLYGON GL_QUADS GL_TRIANGLE_FAN

	for( k=0; k<ppoly.npoints; k++ ) {

		r = (FLOAT)rgb[k].r * inv255;
		g = (FLOAT)rgb[k].g * inv255;
		b = (FLOAT)rgb[k].b * inv255;
		a = (FLOAT)rgb[k].a * inv255;

		CLAMPMINMAX( r, 0.0f, 1.0f );
		CLAMPMINMAX( g, 0.0f, 1.0f );
		CLAMPMINMAX( b, 0.0f, 1.0f );
		CLAMPMINMAX( a, 0.0f, 1.0f );

		glColor4f( r,g,b,a );

		x = (FLOAT)( ppoly.point[k][0] );
		y = (FLOAT)( ppoly.point[k][1] );
		z = FLOAT_EPSILON;

		glVertex3f( x, y, z );
	}

	glEnd();

glDepthMask( GL_TRUE );

	if( alpha == TRUE ) {
		glDisable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ZERO );
	}

	return;
}





//
//  only HIGH sprite
//
static int GFXDLLCALLCONV GL_AddTexMapHigh( texmap_t *texmap ) {

	int num;
	UCHAR *ptr,*sptr;
	int x,y;
	GLenum error;

#define ORIG_SPR texmap->sprite1

	if( alloced_texmap < (nopengltextures + 1) ) {

		alloced_texmap += MAXTEXTURE;
		if( (opengltexture = (opengltexture_t*)drealloc( opengltexture, (alloced_texmap*sizeof(opengltexture_t)) )) == NULL )
			dllQuit("GL_AddTexMapHigh: can't realloc opengltexture (%d).",alloced_texmap);
	}

	num = nopengltextures;

	opengltexture[ num ].texmapid = texmapid_cnt++;

	glGenTextures( 1, &opengltexture[ num ].texmapid );
	dprintf("(bind: %d) ", opengltexture[ num ].texmapid );

	glBindTexture( GL_TEXTURE_2D, opengltexture[ num ].texmapid );
	if( (error = glGetError()) != GL_NO_ERROR )
		dprintf("GL_AddTexMapHigh: glBindTexture failed %d.", error );

	if( (opengltexture[ num ].sprite = (UCHAR *)dmalloc( texmap->width1*texmap->height1*4)) == NULL )
			dllQuit("GL_AddTexMapHigh: can't malloc sprite.");

	ptr = opengltexture[ num ].sprite;

	for( y=0; y<SPRITEH(ORIG_SPR); y++ ) {

		sptr = SPRLINE(ORIG_SPR,SPRITEH(ORIG_SPR)-(y+1));

		for( x=0; x<SPRITEW(ORIG_SPR); x++ ) {

			ptr[0] = *sptr++;
			ptr[1] = *sptr++;
			ptr[2] = *sptr++;

			if( SPRPIXELLEN( ORIG_SPR ) == 4 ) {
				// RGBA
				ptr[3] = *sptr++;
				ptr += 1;
			}
			else
			if( texmap->isChromaKey == TRUE ) {

				if( (ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 0) )
					ptr[3] = 0x00;
				else
					ptr[3] = GL_VISALPHA;

				ptr += 1;

				sptr += (SPRPIXELLEN( ORIG_SPR ) - 3);
			}

			ptr += 3;
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	if( hw_state.texture_edge_clamp == TRUE ) {
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); // GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}
	else {
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); // GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	}

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter );

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// FIXME: ez vajon jó-e?
	// #define GL_GENERATE_MIPMAP 0x8191
	// glTexParameterf( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

	if( texmap->isChromaKey == TRUE || SPRPIXELLEN( ORIG_SPR ) == 4 )
		glTexImage2D( GL_TEXTURE_2D, 0, 4, texmap->width1,texmap->height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, opengltexture[ num ].sprite );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, 3, texmap->width1,texmap->height1, 0, GL_RGB, GL_UNSIGNED_BYTE, opengltexture[ num ].sprite );

	// gluBuild2DMipmaps(GL_TEXTURE_2D, type, width, height, type, GL_UNSIGNED_BYTE, data);

	if( (error = glGetError()) != GL_NO_ERROR )
		dprintf("GL_AddTexMapHigh: glTexImage2D failed %d.", error );

	hw_state.texmapid = -9;

	++nopengltextures;

#undef ORIG_SPR

	return opengltexture[ num ].texmapid;
}




//
// FIXME: a sprite azonos méret legyen
//
static int GFXDLLCALLCONV GL_ReloadTexMapHigh( texmap_t *texmap ) {

	int num;
	UCHAR *ptr,*sptr;
	int x,y;
	GLenum error;

#define ORIG_SPR texmap->sprite1

	for( num=0; num<nopengltextures; num++ )
		if( opengltexture[num].texmapid == (GLuint)texmap->hwtexmapid )
			break;

	if( num >= nopengltextures ) {
		dprintf("reload: texture \"%s\" not found.\n", texmap->name );
		return GL_AddTexMapHigh( texmap );
	}

	glBindTexture( GL_TEXTURE_2D, opengltexture[ num ].texmapid );
	if( (error = glGetError()) != GL_NO_ERROR )
		dprintf("GL_ReloadTexMapHigh: glBindTexture failed %d.", error );

//	if( (sprite = (UCHAR *)dmalloc( texmap->width1*texmap->height1*4)) == NULL )
//			dllQuit("GL_ReloadTexMapHigh: can't malloc sprite.");

	ptr = opengltexture[num].sprite;

	for( y=0; y<SPRITEH(ORIG_SPR); y++ ) {

		sptr = SPRLINE(ORIG_SPR,SPRITEH(ORIG_SPR)-(y+1));

		for( x=0; x<SPRITEW(ORIG_SPR); x++ ) {

			ptr[0] = *sptr++;
			ptr[1] = *sptr++;
			ptr[2] = *sptr++;

			if( SPRPIXELLEN( ORIG_SPR ) == 4 ) {
				// RGBA
				ptr[3] = *sptr++;
				ptr += 1;
			}
			else
			if( texmap->isChromaKey == TRUE ) {

				if( (ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 0) )
					ptr[3] = 0x00;
				else
					ptr[3] = GL_VISALPHA;

				ptr += 1;

				sptr += (SPRPIXELLEN( ORIG_SPR ) - 3);
			}

			ptr += 3;
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); // GL_REPEAT, GL_CLAMP
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter );	// GL_NEAREST
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter ); // GL_NEAREST

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	if( texmap->isChromaKey == TRUE || SPRPIXELLEN( ORIG_SPR ) == 4 )
		glTexImage2D( GL_TEXTURE_2D, 0, 4, texmap->width1,texmap->height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, opengltexture[num].sprite );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, 3, texmap->width1,texmap->height1, 0, GL_RGB, GL_UNSIGNED_BYTE, opengltexture[num].sprite );

	if( (error = glGetError()) != GL_NO_ERROR )
		dprintf("GL_AddTexMapHigh: glTexImage2D failed %d.", error );

	hw_state.texmapid = -9;

#undef ORIG_SPR

	return opengltexture[ num ].texmapid;
}




//
// FIXME: a sprite azonos méret legyen
//
static int GFXDLLCALLCONV GL_ModifyTexMapHigh( texmap_t *texmap ) {

	return GL_ReloadTexMapHigh( texmap );

	/***
	int num;

#define ORIG_SPR texmap->sprite1

	for( num=0; num<nopengltextures; num++ )
		if( opengltexture[num].texmapid == (GLuint)texmap->hwtexmapid )
			break;

	if( num >= nopengltextures ) {
		// dprintf("reload: texture \"%s\" not found.\n", texmap->name );
		return 0;
	}

	UCHAR *ptr = opengltexture[num].sprite;

	for( int y=0; y<SPRITEH(ORIG_SPR); y++ ) {

		UCHAR *sptr = SPRLINE(ORIG_SPR,SPRITEH(ORIG_SPR)-(y+1));

		for( int x=0; x<SPRITEW(ORIG_SPR); x++ ) {

			ptr[0] = *sptr++;
			ptr[1] = *sptr++;
			ptr[2] = *sptr++;

			if( SPRPIXELLEN( ORIG_SPR ) == 4 ) {
				// RGBA
				ptr[3] = *sptr++;
				ptr += 1;
			}
			else
			if( texmap->isChromaKey == TRUE ) {

				if( (ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 0) )
					ptr[3] = 0x00;
				else
					ptr[3] = GL_VISALPHA;

				ptr += 1;

				sptr += (SPRPIXELLEN( ORIG_SPR ) - 3);
			}

			ptr += 3;
		}
	}

	if( texmap->isChromaKey == TRUE || SPRPIXELLEN( ORIG_SPR ) == 4 )
		glTexImage2D( GL_TEXTURE_2D, 0, 4, texmap->width1,texmap->height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, opengltexture[num].sprite );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, 3, texmap->width1,texmap->height1, 0, GL_RGB, GL_UNSIGNED_BYTE, opengltexture[num].sprite );

	return opengltexture[ num ].texmapid;
#undef ORIG_SPR
	***/
}



//
//
//
static void GFXDLLCALLCONV GL_DiscardTexture( int texmapid ) {

	int i;

	for( i=0; i<nopengltextures; i++ )
		if( (int)opengltexture[i].texmapid == texmapid )
			break;

	if( i >= nopengltextures ) {
		dprintf("GL_DiscardTexture: no %d texture.\n",texmapid);
		return;
	}

	glDeleteTextures( 1, &opengltexture[i].texmapid );

	dfree( opengltexture[i].sprite );

	dprintf("(hwtexmapid: %d) ",opengltexture[i].texmapid);

	if( ((nopengltextures-i)-1) > 0 )
		memmove( &opengltexture[i], &opengltexture[i+1], ((nopengltextures-i)-1) * sizeof(opengltexture_t) );

	memset( &opengltexture[nopengltextures-1], 0L, sizeof(opengltexture_t) );

	--nopengltextures;

	return;
}




//
//
//
static void GFXDLLCALLCONV GL_DiscardAllTexture( void ) {

	while( nopengltextures > 0 )
		GL_DiscardTexture( opengltexture[nopengltextures-1].texmapid );

	if( opengltexture ) dfree( opengltexture );
	opengltexture = NULL;

	nopengltextures = 0;
	alloced_texmap = 0;

	return;
}




//
//
//
static void GFXDLLCALLCONV GL_Line( int x0, int y0, int x1, int y1, rgb_t color ) {

	FLOAT r,g,b,a;
	FLOAT inv255 = 1.0f / 255.0f;
	FLOAT x,y;

	CHECKACTIVE;

	r = (FLOAT)color.r * inv255;
	g = (FLOAT)color.g * inv255;
	b = (FLOAT)color.b * inv255;
	a = (FLOAT)color.a * inv255;

	CLAMPMINMAX( r, 0.0f, 1.0f );
	CLAMPMINMAX( g, 0.0f, 1.0f );
	CLAMPMINMAX( b, 0.0f, 1.0f );

	glColor4f( r,g,b,a );

	// dprintf( "color: %0.2f; %0.2f; %0.2f; %0.2f\n", r,g,b,a );

	// Draw aliased lines
	// glDisable(GL_LINE_SMOOTH);

	// Disable line stippling
	// glDisable(GL_LINE_STIPPLE);

	// Set the line width
	// glLineWidth(10.0);

	if( hw_state.mode != HM_COLOR ) {

		glDisable( GL_TEXTURE_2D );

		hw_state.texmapid = -10;
		hw_state.mode = HM_COLOR;
	}

	glDisable(GL_DEPTH_TEST);
	// glDisable(GL_LIGHTING);

	glBegin(GL_LINE_STRIP);

	x = (FLOAT)( x0 );
	y = (FLOAT)( y0 );

	glVertex2f( x, y );

	x = (FLOAT)( x1 );
	y = (FLOAT)( y1 );

	glVertex2f( x, y );

	glEnd();

	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_LIGHTING);

	return;
}


//
//
//
static void GFXDLLCALLCONV GL_PutPixel( int x, int y, rgb_t color ) {

	FLOAT r,g,b,a;
	FLOAT inv255 = 1.0f / 255.0f;

	CHECKACTIVE;

	r = (FLOAT)color.r * inv255;
	g = (FLOAT)color.g * inv255;
	b = (FLOAT)color.b * inv255;
	a = (FLOAT)color.a * inv255;

	CLAMPMINMAX( r, 0.0f, 1.0f );
	CLAMPMINMAX( g, 0.0f, 1.0f );
	CLAMPMINMAX( b, 0.0f, 1.0f );

	glColor4f( r,g,b,a );

	glDisable(GL_DEPTH_TEST);
	// glDisable(GL_LIGHTING);

	if( hw_state.mode != HM_COLOR ) {

		glDisable( GL_TEXTURE_2D );

		hw_state.texmapid = -10;
		hw_state.mode = HM_COLOR;
	}

	glBegin(GL_POINTS);

	glVertex2f( (FLOAT)x, (FLOAT)y );

	glEnd();

	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_LIGHTING);

	return;
}



//
//
//
static void GFXDLLCALLCONV GL_BeginScene( void ) {


	return;
}



//
//
//
static void GFXDLLCALLCONV GL_EndScene( void ) {

	glFinish();

	return;
}




//
//
//
static void GFXDLLCALLCONV GL_FlushScene( void ) {

	glFlush();

	return;
}






//
// TODO: mi a fasz Opengl?
//
static void GFXDLLCALLCONV GL_Fog( FLOAT fn, FLOAT ff, ULONG fogcolor ) {

	GLfloat fog_color[3]; //  = { 1.0f, 1.0f, 1.0f, 1.0f };   /* white */

	fog_color[0] = (FLOAT)((( fogcolor >> 16 ) & 0xff)) / 256.0f;
	fog_color[1] = (FLOAT)((( fogcolor >>  8 ) & 0xff)) / 256.0f;
	fog_color[2] = (FLOAT)((( fogcolor >>  0 ) & 0xff)) / 256.0f;

	// dprintf("fog: %.2f %.2f %.2f\n", fog_color[0], fog_color[1], fog_color[2] );

	glfognear = fn;
	glfogfar = ff;

	glEnable( GL_FOG );
	glFogi( GL_FOG_MODE, GL_LINEAR ); // GL_LINEAR, GL_EXP
	// glHint(GL_FOG_HINT, GL_NICEST);			// Fog Hint Value

	glFogfv( GL_FOG_COLOR, fog_color );

	// FLOAT fDensity = 0.35f;
	FLOAT fDensity = 0.6f;
	// glFogf( GL_FOG_DENSITY, fDensity );		// How Dense Will The Fog Be

	glFogf( GL_FOG_START, fn / gl_zclipfar );
	glFogf( GL_FOG_END, ff / gl_zclipfar );

	// Disable opengl fog
	// glDisable( GL_FOG );

	return;
}







//
//
//
static BOOL GFXDLLCALLCONV GL_Activate( int flag ) {

	if( flag ) {

	}

	ogl_active_flag = flag;

	return FALSE;
}





//
//
//
static void GFXDLLCALLCONV GL_InitPalette( UCHAR *pal ) {


	return;
}






//
//
//
static void GFXDLLCALLCONV GL_SetRgb( int color, int red, int green, int blue ) {


	return;
}






//
//
//
static void GFXDLLCALLCONV GL_GetRgb( int color, int *red, int *green, int *blue ) {


	return;
}







//
//
//
static ULONG GFXDLLCALLCONV GL_GetPitch( void ) {

	return (ULONG)hw_state.SCREENW * (hw_state.bpp / 8);
}









//
//
//
static void GFXDLLCALLCONV GL_FlipPage( void ) {

	/* Swap Buffers once rendering is complete */

	OpenGLSwapBuffers( hw_state.hdc );

	return;
}







//
//
//
static void GFXDLLCALLCONV GL_Clear( ULONG color, ULONG depth, int flag ) {

#define INV256 (1.0f / 256.0f)

	if( flag & (CF_TARGET | CF_ZBUFFER) ) {

		glClearColor( (FLOAT)((color>>16)&0xff) * INV256, (FLOAT)((color>>8)&0xff) * INV256, (FLOAT)((color>>0)&0xff) * INV256, 0.0f );
		glClearDepth( 1.0f );

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}
	else
	if( flag & CF_ZBUFFER ) {

		glClearDepth( 1.0f );

		glClear( GL_DEPTH_BUFFER_BIT );
	}
	else
	if( flag & CF_TARGET ) {

		glClearColor( (FLOAT)((color>>16)&0xff) * INV256, (FLOAT)((color>>8)&0xff) * INV256, (FLOAT)((color>>0)&0xff) * INV256, 0.0f );

		glClear( GL_COLOR_BUFFER_BIT );
	}

	return;
}







//
//
//
void Init( void ) {

	int w,h;

	glClearColor( 0.2f, 0.2f, 0.6f, 1.0f );

    // glEnable( GL_TEXTURE_2D );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST ); // GL_NICEST GL_FASTEST
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );

	glShadeModel( GL_SMOOTH ); /* Smooth shading */

//	glEnable( GL_POLYGON_SMOOTH );
//	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	w = hw_state.SCREENW; // GetSystemMetrics( SM_CXSCREEN );
	h = hw_state.SCREENH; // GetSystemMetrics( SM_CYSCREEN );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
//	glOrtho( 0.0f, (FLOAT)w, 0.0f, (FLOAT)h, 0.0f, 1.0f );
 	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glScalef(2.0f / (float)w, -2.0f / (float)h, 1.0f);
	glTranslatef(-((float)w / 2.0f), -((float)h / 2.0f), 0.0f);
	glViewport( 0, 0, w, h );
	glScissor( 0, 0, w, h );

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );  /* Superimpose smaller Z values over larger ones */
	glDepthRange( 0.0f, 1.0f );
	glDepthMask( GL_TRUE );
	hw_state.zbuffermode = TRUE;

	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	hw_state.cullmode = 1;

	glDisable( GL_ALPHA_TEST );
	// glAlphaFunc( GL_GREATER, 0.5 );

	glDisable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	glReadBuffer( GL_BACK );

	return;
}


static char opengl_desc_str[128] = "unidentified";


//
//
//
BOOL SetupPixelFormat( HDC hdc ) {

	PIXELFORMATDESCRIPTOR pfd;
	int pixelformat;

	memset( &pfd, 0, sizeof(pfd) );

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 16;
	pfd.cAccumBits = 0;
	pfd.cStencilBits = 0;

	/* Search for best matching pixel format */
	if( (pixelformat = OpenGLChoosePixelFormat(hdc, &pfd)) == 0 ) {
		dprintf("ChoosePixelFormat failed.\n");
		return FALSE;
	}

	/* what did we get I wonder - populate pfd so we can peruse in the debugger */
	memset( &pfd, 0, sizeof(pfd) );

	OpenGLDescribePixelFormat( hdc, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd );

	int generic_format = pfd.dwFlags & PFD_GENERIC_FORMAT;
	int generic_accelerated = pfd.dwFlags & PFD_GENERIC_ACCELERATED;

	if( generic_format && ! generic_accelerated ) {

		// software

		strcpy( opengl_desc_str, "software" );
	}
	else
	if( generic_format && generic_accelerated ) {

		// hardware - MCD

		strcpy( opengl_desc_str, "hardware - MCD" );
	}
	else
	if( !generic_format && !generic_accelerated ) {

		// hardware - ICD

		strcpy( opengl_desc_str, "hardware - ICD" );
	}
	else
		strcpy( opengl_desc_str, "unidentified" );


	/* set the format to closest match */

	if( OpenGLSetPixelFormat(hdc, pixelformat, &pfd) == FALSE ) {
		dprintf("SetPixelFormat failed.\n");
		return FALSE;
	}

	return TRUE;
}





//
//
//
BOOL GL_CheckExtension( char *extensionName ) {

	// get the list of supported extensions
	char *extensionList = (char *)glGetString( GL_EXTENSIONS );

	if( !extensionName || !extensionList )
		return FALSE;

	while( extensionList ) {

		// find the length of the first extension substring
		unsigned int firstExtensionLength = strcspn(extensionList, " ");


		if( strlen(extensionName) == firstExtensionLength &&
			strncmp(extensionName, extensionList, firstExtensionLength) == 0 ) {
			return TRUE;
		}

		// move to the next substring
		extensionList += firstExtensionLength + 1;
	}

	return FALSE;
}




//
//
//
static BOOL GFXDLLCALLCONV GL_Init( gfxdlldesc_t *desc ) {

	char *gfx_ext = NULL;

	hw_state.hwnd = (HWND)desc->hwnd;
	hw_state.hdc = GetDC( (HWND)desc->hwnd );
	hw_state.SCREENW = desc->width;
	hw_state.SCREENH = desc->height;
	hw_state.bpp = desc->bpp;
	hw_state.pixel_len = ABS(desc->bpp)/8;
	hw_state.texmapid = -1;
	hw_state.mode = HM_UNINIT;
	hw_state.multitex = FALSE;
	hw_state.hInst = (HINSTANCE)desc->hInst;
	hw_state.tex24bit = TRUE;
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
	dDotProduct = (FLOAT (GFXDLLCALLCONV *)(point3_t v1, point3_t v2 ))desc->DotProduct;
	dwinFullscreen = (BOOL (GFXDLLCALLCONV *)(void))desc->winFullscreen;
	GL_winReadProfileString = (void (GFXDLLCALLCONV *)(char*,char*,char*,int))desc->winReadProfileString;
	GL_winWriteProfileString = (void (GFXDLLCALLCONV *)(char*,char*))desc->winWriteProfileString;
	GL_winReadProfileInt = (int (GFXDLLCALLCONV *)(char*,int))desc->winReadProfileInt;
	GL_winWriteProfileInt = (void (GFXDLLCALLCONV *)(char*,int))desc->winWriteProfileInt;

	dwinOpengl = (BOOL (GFXDLLCALLCONV *)(void))desc->winOpengl;
	dGetHwnd = (ULONG (GFXDLLCALLCONV *)(void))desc->GetHwnd;
	dGetHinstance = (ULONG (GFXDLLCALLCONV *)(void))desc->GetHinstance;

	dprintf("init: %s v%s (c) Copyright 1998 bernie (%s %s)\n",GFXDLL_NAME,GFXDLL_VERSION,__DATE__,__TIME__);

	if( OpenGLInit( opengldllname ) == -1 ) {
		sprintf( opengl_error_str, "You don't have a properly installed OpenGL driver or can't load \"%s\" library.\n\nInstall or upgrade your OpenGL drivers or choose another driver.",opengldllname);
		return FALSE;
	}

	dprintf("log: \"%s\" library loaded.\n",opengldllname);

//	if( hw_state.bpp != 16 )
//		return FALSE;

	bInGraphic = TRUE;

	// Change resolution

	// temp = BOOLEAN(GL_winReadProfileInt("gl_fullscreen", 1));
	// GL_winWriteProfileInt( "gl_fullscreen", temp );

	if( dwinFullscreen() ) {
		if( hw_state.bpp > 0 )
			ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp );
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

		if( desc->flag != SETX_NOBORDER )
			AdjustWindowRect( &rc, dwStyle, FALSE );

		// dprintf( "glscreen: %d,%d, %d, %d\n",rc.left ,rc.top,rc.right,rc.bottom );

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE );

		// dprintf( "movewindow: %d,%d, %d,%d [%d,%d]\n", rc.left, rc.top, rc.right, rc.bottom, rc.right-rc.left, rc.bottom-rc.top );
	}

	// if( hw_state.bpp < 0 )
	//	hw_state.bpp *= -1;

	hw_state.bpp = ABS( hw_state.bpp );

	if( SetupPixelFormat( hw_state.hdc ) != TRUE ) {
		sprintf( opengl_error_str, "GL_Init: SetupPixelFormat failed.");
		return FALSE;
	}

	if( (hw_state.hRC = OpenGLCreateContext(hw_state.hdc)) == NULL ) {
		sprintf( opengl_error_str, "GL_Init: wglCreateContext failed.");
		return FALSE;
	}

	if( OpenGLMakeCurrent(hw_state.hdc, hw_state.hRC) != TRUE ) {
		sprintf( opengl_error_str, "GL_Init: wglMakeCurrent failed.");
		return FALSE;
	}

	Init();

	char *s	= (char *)glGetString(GL_EXTENSIONS);
	gfx_ext = (char *)dmalloc( strlen(s) + 1 );
	strcpy( gfx_ext, s );

	dprintf("init: OpenGL (%s) using: \"%s\" Version \"%s\" by \"%s\" (%s).\n",opengldllname, glGetString(GL_RENDERER),glGetString( GL_VERSION ),glGetString(GL_VENDOR),opengl_desc_str);

	int len = strlen( gfx_ext );
	for( int i = 0; i<len; i++ ) {
		if( gfx_ext[i] == ' ' )
			gfx_ext[i] = '\n';
	}

	dprintf("log: GL extensions:\n%s",gfx_ext);

	if( hw_state.multitex = BOOLEAN( strstr(gfx_ext, "GL_ARB_multitexture") ) ) {

		glGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &hw_state.multitex );

		if( (hw_state.multitex < 0) || (hw_state.multitex > 16) ) {
			printf("Unreasonable amount of multitexture layers found therefore it's not truely supported, setting to 1.\n");
			hw_state.multitex = 1;
		}

		dprintf("log: multitex with %d textures.\n", hw_state.multitex );
	}
	else
		dprintf("log: no multitex.\n");

	// envcombine4
	hw_state.envcombine4 = BOOLEAN( strstr(gfx_ext, "GL_NV_texture_env_combine4") );

	// texture_edge_clamp
	hw_state.texture_edge_clamp = BOOLEAN( strstr(gfx_ext, "GL_EXT_texture_edge_clamp") );

	if( gfx_ext ) dfree( gfx_ext );


	hw_state.tex_size = 1024;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &hw_state.tex_size );

	hw_state.mintex_size = 64;

	dprintf("log: texture size min: %d, max: %d.\n", hw_state.mintex_size, hw_state.tex_size );

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

	drmask = desc->rmask;
	dgmask = desc->gmask;
	dbmask = desc->bmask;

	drsize = desc->rsize;
	dgsize = desc->gsize;
	dbsize = desc->bsize;

	drshift = desc->rshift;
	dgshift = desc->gshift;
	dbshift = desc->bshift;

	nopengltextures = 0;
	alloced_texmap = 0;
	opengltexture = NULL;

	if( (videoptr = (UCHAR *)dmalloc( hw_state.SCREENW * hw_state.SCREENH * 4 )) == NULL )
		return FALSE;

	hw_state.orig_SCREENW = hw_state.SCREENW;
	hw_state.orig_SCREENH = hw_state.SCREENH;
	hw_state.orig_bpp = hw_state.bpp;

	return TRUE;
}






//
//
//
static void GFXDLLCALLCONV GL_Deinit( void ) {

	GL_DiscardAllTexture();

	if( videoptr ) dfree( videoptr );
	videoptr = NULL;

	OpenGLMakeCurrent( NULL, NULL );

	if( hw_state.hRC ) { OpenGLDeleteContext( hw_state.hRC ); hw_state.hRC = 0; }
	if( hw_state.hdc ) { ReleaseDC( hw_state.hwnd, hw_state.hdc);  hw_state.hdc = 0; }

	if( dwinFullscreen() )
		RestoreResoltion();

	bInGraphic = FALSE;

	OpenGLUnInit();

	if( dprintf ) dprintf("deinit: %s success.\n",GFXDLL_NAME);

	return;
}



//
//
//
static BOOL GFXDLLCALLCONV GL_Fullscreen( int flag ) {

	DWORD dwWndStyle = GetWindowStyle((HWND)hw_state.hwnd);

	if( videoptr ) dfree( videoptr );
	videoptr = NULL;

	if( flag == TRUE ) {

		// determine the new window style
		dwWndStyle = WS_POPUP | WS_VISIBLE;

		// change the window style
		SetWindowLong((HWND)hw_state.hwnd, GWL_STYLE, dwWndStyle);

		if( hw_state.bpp > 0 )
			if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
				dprintf( "GL_Fullscreen: ChangeResolution(%d, %d, %d) the first phail.\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp);
				hw_state.SCREENW = 640;
				hw_state.SCREENH = 480;
				hw_state.bpp = 32;
				if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
					dprintf( "GL_Fullscreen: ChangeResolution(%d, %d, %d) the second fail.\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp);
						hw_state.SCREENW = 800;
						hw_state.SCREENH = 600;
						hw_state.bpp = 32;
						if( ChangeResolution( hw_state.SCREENW, hw_state.SCREENH, hw_state.bpp ) == FALSE ) {
							dprintf( "GL_Fullscreen: ChangeResolution(%d, %d, %d) final fail.\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp);
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

	if( (videoptr = (UCHAR *)dmalloc( hw_state.SCREENW * hw_state.SCREENH * 4 )) == NULL )
		return FALSE;

	dprintf("GL_Fullscreen: Switched to %s.\n", flag?"Fullscreen":"Windowed" );

	return TRUE;
}



//
//
//
static void GFXDLLCALLCONV GL_GetDescription( char *str ) {

	sprintf(str,"OpenGL v1 driver v%s",GFXDLL_VERSION);

	return;
}



//
//
//
static void GFXDLLCALLCONV GL_GetData( void *buf, int len ) {

	if( *(ULONG *)buf == GFXDLL_ISMSGOK )
		*(ULONG *)buf = TRUE; // !bInGraphic;

	if( *(ULONG *)buf == GFXDLL_24BIT )
		*(ULONG *)buf = TRUE; // !bInGraphic;

	if( *(ULONG *)buf == GFXDLL_MAXTEXSIZE )
		*(ULONG *)buf = hw_state.tex_size;

	if( *(ULONG *)buf == GFXDLL_MINTEXSIZE )
		*(ULONG *)buf = hw_state.mintex_size;

	if( *(ULONG *)buf == GFXDLL_GETERROR )
		strncpy( (char *)buf, opengl_error_str, len );
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

		HINSTANCE hOpengl = NULL;			// bass handle

		if( (hOpengl = LoadLibrary(opengldllname)) == NULL ) {
			*(ULONG *)buf = FALSE;
			sprintf( opengl_error_str, "GL_GetData: you don't have a properly installed OpenGL driver or can't load \"%s\" library.", opengldllname );
		}
		else {
			FreeLibrary(hOpengl);
			*(ULONG *)buf = TRUE;
		}
	}

	return;
}



//
//
//
void GFXDLLCALLCONV GFXDRV_GetInfo( GFXDLLinfo_t *info ) {


	info->GFXDLL_SetupCulling		= (void *)GL_SetupCulling;
	info->GFXDLL_AddToPipeline		= (void *)GL_AddToPipeline;
	info->GFXDLL_AddTexMapHigh		= (void *)GL_AddTexMapHigh;
	info->GFXDLL_ReloadTexMapHigh		= (void *)GL_ReloadTexMapHigh;
	info->GFXDLL_ModifyTexMapHigh		= (void *)GL_ModifyTexMapHigh;
	info->GFXDLL_PutSpritePoly		= (void *)GL_PutSpritePoly;
	info->GFXDLL_PutPoly			= (void *)GL_PutPoly;
	info->GFXDLL_DiscardAllTexture		= (void *)GL_DiscardAllTexture;
	info->GFXDLL_DiscardTexture		= (void *)GL_DiscardTexture;
	info->GFXDLL_Line			= (void *)GL_Line;
	info->GFXDLL_PutPixel			= (void *)GL_PutPixel;
	info->GFXDLL_BeginScene			= (void *)GL_BeginScene;
	info->GFXDLL_EndScene			= (void *)GL_EndScene;
	info->GFXDLL_FlushScene			= (void *)GL_FlushScene;
	info->GFXDLL_LockLfb			= (void *)GL_LockLfb;
	info->GFXDLL_UnlockLfb			= (void *)GL_UnlockLfb;
	info->GFXDLL_Init			= (void *)GL_Init;
	info->GFXDLL_Deinit			= (void *)GL_Deinit;
	info->GFXDLL_GetDescription		= (void *)GL_GetDescription;
	info->GFXDLL_GetData			= (void *)GL_GetData;
	info->GFXDLL_GetPitch			= (void *)GL_GetPitch;
	info->GFXDLL_Activate			= (void *)GL_Activate;
	info->GFXDLL_FlipPage			= (void *)GL_FlipPage;
	info->GFXDLL_Clear			= (void *)GL_Clear;
	info->GFXDLL_Fog			= (void *)GL_Fog;
	info->GFXDLL_InitPalette		= (void *)GL_InitPalette;
	info->GFXDLL_SetRgb			= (void *)GL_SetRgb;
	info->GFXDLL_GetRgb			= (void *)GL_GetRgb;
	info->GFXDLL_Fullscreen			= (void *)GL_Fullscreen;

	return;
}



//
//
//
void screenShot( char *fName, int width, int height ) {

	UCHAR *fBuffer;

	if( (fBuffer = (UCHAR *)dmalloc( width * height * 3 )) == NULL )
		return;

	glPixelStorei(GL_PACK_ALIGNMENT,1);

	// glReadBuffer(GL_BACK_LEFT);

	// read our image data from the frame buffer
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, fBuffer );

	// write the image data to a .tga file
	// writeTGA (fName, fBuffer, winW, winH);

	dfree( fBuffer );

	return;
}



//
//
//
extern "C" void dllDeinit( void ) {

	GL_Deinit();

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




/***

//   Write the current view to a file
//   The multiple fputc()s can be replaced with
//      fwrite(image,width*height*3,1,fptr);
//   If the memory pixel order is the same as the destination file format.


int WindowDump(void)
{
   int i,j;
   FILE *fptr;
   static int counter = 0; // This supports animation sequences
   char fname[32];
   unsigned char *image;

   // Allocate our buffer for the image
   if ((image = dmalloc(3*width*height*sizeof(char))) == NULL) {
      fprintf(stderr,"Failed to allocate memory for image\n");
      return(FALSE);
   }

   glPixelStorei(GL_PACK_ALIGNMENT,1);

   // Open the file
   if (stereo)
      sprintf(fname,"L_%04d.raw",counter);
   else
      sprintf(fname,"C_%04d.raw",counter);
   if ((fptr = fopen(fname,"w")) == NULL) {
      fprintf(stderr,"Failed to open file for window dump\n");
      return(FALSE);
   }

   // Copy the image into our buffer
   glReadBuffer(GL_BACK_LEFT);
   glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

   // Write the raw file
   / fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm
   for (j=height-1;j>=0;j--) {
      for (i=0;i<width;i++) {
         fputc(image[3*j*width+3*i+0],fptr);
         fputc(image[3*j*width+3*i+1],fptr);
         fputc(image[3*j*width+3*i+2],fptr);
      }
   }
   fclose(fptr);

   if (stereo) {
      // Open the file
      sprintf(fname,"R_%04d.raw",counter);
      if ((fptr = fopen(fname,"w")) == NULL) {
         fprintf(stderr,"Failed to open file for window dump\n");
         return(FALSE);
      }

      // Copy the image into our buffer
      glReadBuffer(GL_BACK_RIGHT);
      glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

      // Write the raw file
      // fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm
      for (j=height-1;j>=0;j--) {
         for (i=0;i<width;i++) {
            fputc(image[3*j*width+3*i+0],fptr);
            fputc(image[3*j*width+3*i+1],fptr);
            fputc(image[3*j*width+3*i+2],fptr);
         }
      }
      fclose(fptr);
   }

   // Clean up
   counter++;
   dfree(image);
   return(TRUE);
}

***/






/***
From: akin@tuolumne.engr.sgi.com (Allen Akin)
Newsgroups: comp.graphics.api.opengl,comp.sys.sgi.graphics
Subject: Re: Texture distortion problem
Date: Fri, 11 Jul 97 14:13:03 1997
Organization: Silicon Graphics, Inc., Mountain View, CA

In article <868594016.1232@dejanews.com>,  <echan@harris.com> wrote:
| I'm trying to map an oblique photo texture onto a rectangle, that is,
| the rectangular region in the photo has some perspective foreshortening.
| When I map the corners of the distorted texture rectangle to the
| rectangular polygon I get this angular artifact along the diagonal.
| My understanding is that this has to do with a linear interpolation
| optimization in the rendering pipeline.  (Is that right?)

Mostly.  You've specified texture coordinates that vary nonlinearly
across the surface of the rectangle.  When the renderer decomposes the
rectangle into two triangles, and then interpolates the texture
coordinates linearly inside each of the triangles, you'll see a seam at
the edge between the two triangles.

This has nothing to do with perspective-correct interpolation of
texture coordinates, which is why changing the perspective correction
hint has no effect.  It's more a problem of approximating a nonlinear
model with a linear one.  It would be a lot easier to explain this if I
could draw you a picture on a whiteboard, but maybe a schematic
presentation will do the trick.

Imagine your rectangle projected onto the viewing plane, with texture
coordinates attached to each vertex.  I'll pick texture coordinates
that shorten up one side of the square in texture space, just as you
did in your test case:

	(x,y;s,t) = (0,1; 0.25,1) +-----------+ (1,1; 0.75,1)
				  |           |
				  |           |
				  |           |
				  |           |
				  |           |
		       (0,0; 0,0) +-----------+ (1,0; 1,0)

For the texture to be mapped onto the rectangle without distortion
artifacts, the mapping must be linear; that is, each of ds/dx, ds/dy,
dt/dx, and dt/dy must not vary across the rectangle.

However, you can see that ds/dx is 1.0 at the bottom edge of the
rectangle, and ds/dx is 0.5 at the top edge of the rectangle, so the
mapping isn't linear.  This causes artifacts when the rectangle is
triangulated:

	(x,y;s,t) = (0,1; 0.25,1) +-----------+ (1,1; 0.75,1)
				  | +         |
				  |   +       |
				  |     +     |
				  |       +   |
				  |         + |
		       (0,0; 0,0) +-----------+ (1,0; 1,0)

The lower-left triangle has ds/dx = 1.0 and ds/dy = 0.25.  The
upper-right triangle has ds/dx = 0.5 and ds/dy = -0.25.  If you imagine
rendering pixels horizontally across the rectangle, picking up
appropriate texels as you go, you can see that you'll take a sudden
``turn'' in texture space just as you cross the edge between triangles,
because ds/dx and ds/dy change abruptly.  This is what creates the seam
you're seeing.

Note that none of this discussion involves perspective correction;
that's a completely independent issue.  This is all a matter of trying
to model a nonlinear change in texture coordinates using two triangles,
within which the texture coordinates vary linearly.  This is very much
like trying to model a nonplanar quadrilateral using two triangles; as
stated, it's not a well-constrained problem, and simple approximations
are subject to many kinds of artifacts.

By the way, I like to transform this sort of problem into a purely
geometric one, by thinking of quadrilaterals in (x,y,s) and (x,y,t)
spaces.  If you observe that one of those quadrilaterals is nonplanar,
then you know right away you'll run into rendering artifacts.  The same
technique also gives you insight into the behavior of Gouraud shading
on quadrilaterals with arbitrary colors at the vertices.

|                             ... I can't make the artifact go away
| when using explicit texture coordinates.  Suggestions?

There are two things you can do.  One is to subdivide your
quadrilateral into much smaller pieces, essentially creating a
piecewise-planar approximation of your (x,y,s) surface.  This is quite
straightforward, though it might be slow if you need a large number of
pieces to obtain adequate quality.

Since you're using OpenGL (rather than other APIs which shall remain
nameless :-)), you have a second option:  you could take advantage of
projective texturing.  Here's a note on the subject from Mark Segal:

	From: segal (Mark Segal)
	Subject: Re: Textures on Quads?
	Date: 15 May 1996 18:55:56 GMT

	| The effect I'm hoping to achieve is essentially an image warp
	| where the top of the texture is scaled down, but all the
	| vertical lines stay straight (sort of emitting radially from
	| the top).  Is there a way to achive this without having lots
	| and lots of geometry?  Thanx for any help.

	You can do this with 'projective' textures (it only works with
	a quadrilateral, though).  I won't give you all the details,
	but here's the idea:

	There's a (2D) projective map that carries a square (your texture)
	onto the quadrilateral.  If you can figure out what this map is,
	you can load the texture matrix with it and OpenGL will figure
	out the coordinates for you.  Alternatively, you could apply the
	map yourself and supply the texture 'q' coordinate (using
	glTexCoord4).

	What you have to do is look at a diagram like this
		 /
		/ |
	       /  |
	       |  |
	       |  |       *
	       \  |
		\ |
		 \

	This is supposed to look like what you'd see if you put your
	eye at the star and looked at a square in perspective in 2D
	(think about what a wireframe cube looks like in 3D in perspective).
	What you have to do is figure out the near, far, and other
	parameters to get the projection matrix that converts your
	square into your quadrilateral.  (Ignore the y coordinate;
	you're working in 2D so all you care about is x and z.)
	Once you figure out this transformation you're done.

	If you don't want to figure out the viewing parameters, you
	can do this directly: look at

	(s')     ( a b c )( s )
	(t')  =  ( d e f )( t )
	(q')     ( g h i )( q )

	Applied to (s,t,q) = (0,0,1), (1,0,1), (0,1,1), and (1,1,1) (these
	are the coordinates of the corners of your texture).  You want
	(s'/q', t'/q') for each of these four (s,t,q) values to be the
	corners of your quadrilateral.  This will give you 8 equations
	in the 9 unknowns a-i.  You get one more equation because you
	can scale the matrix by anything you want and get the same
	result (equivalently, you're free to set any entry of the
	matrix that doesn't turn out to be zero to anything you want).
	Once you have the matrix, set the OpenGL texture matrix to it
	(you have to fill in a row and column of zeros for the third
	coordinate, which isn't being used) and supply (s,t) at the
	quadrilateral vertices.  Or, leave the texture matrix at the
	identity and supply s', t', and q'.

	Hope this helps.

		Mark

Allen
***/

/*** multitexture

<code>
glActiveTextureARB(GL_TEXTURE0_ARB);
glMultiTexCoord2fARB(GL_TEXTURE0_ARB,U,V);
<end>



(*glActiveTextureARB)(GL_TEXTURE0_ARB);
 glEnable(GL_TEXTURE_2D);
 glBindTexture(GL_TEXTURE_2D, Game::get().skydome.textureBack);
 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 glBlendFunc(GL_SRC_ALPHA,GL_ONE);
 (*glActiveTextureARB)(GL_TEXTURE1_ARB);
 glEnable(GL_TEXTURE_2D);
 glBindTexture(GL_TEXTURE_2D, Game::get().skydome.textureFront);
 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
 glEnable(GL_BLEND);

  TriVec *tris = mesh->triangles;
  TexVec *tvecs = mesh->textureCoord;

  glBegin( GL_TRIANGLES );

  for (int t=0 ; t<mesh->triangleNum ; t++)



// i see here writing this email glMultiTexCoord2fvARB is better
   glMultiTexCoord2fARB(GL_TEXTURE0_ARB,  tvecs[tris[t][0]][0],
tvecs[tris[t][0]][1]);
   glMultiTexCoord2fARB(GL_TEXTURE1_ARB,  tvecs[tris[t][0]][0],
tvecs[tris[t][0]][1]);
   glVertex3fv( vecs[tris[t][0]] );

   glMultiTexCoord2fARB(GL_TEXTURE0_ARB,  tvecs[tris[t][1]][0],
tvecs[tris[t][1]][1]);
   glMultiTexCoord2fARB(GL_TEXTURE1_ARB,  tvecs[tris[t][1]][0],
tvecs[tris[t][1]][1]);
   glVertex3fv( vecs[tris[t][1]]);

   glMultiTexCoord2fARB(GL_TEXTURE0_ARB,  tvecs[tris[t][2]][0],
tvecs[tris[t][2]][1]);
   glMultiTexCoord2fARB(GL_TEXTURE1_ARB,  tvecs[tris[t][2]][0],
tvecs[tris[t][2]][1]);
   glVertex3fv( vecs[tris[t][2]]);
  }
  glEnd();

 glDisable(GL_BLEND);

(*glActiveTextureARB)(GL_TEXTURE1_ARB);
 glDisable(GL_TEXTURE_2D);
(*glActiveTextureARB)(GL_TEXTURE0_ARB);
 glDisable(GL_TEXTURE_2D);

 glEnable(GL_TEXTURE_2D);



  static void ReadColor(Bitmap& p, int x, int y, int width, int height)
{
	// setup buffer
	Bitmap buf( width, height, PixelFormat(32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000) );

	// read pixels
	glReadPixels( x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buf.image );

	// flip the image about y-axis
	int* a = (int*)buf.image;
	int* b = a + width*(height-1);
	for ( int iy=0; iy<height/2; iy++ )
	{
		for ( int ix=0; ix<width; ix++ )
		{
			Swap( *a++, *b++ );
		}
		b -= width*2;
	}

	// color conversion
	p.Blit( buf, false, false );
}


void DeviceGL::SetTexCoordMode(int stage, TexCoordMode mode)
{
	switch ( mode )
	{
		case TEXCOORD_VERTEX:
		{
			// disable all planes
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
			break;
		}

		case TEXCOORD_POINT:
		{
			// set vertex->texcoord matrix to identity
			GLfloat EyePlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
			GLfloat EyePlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
			GLfloat EyePlaneR[] = { 0.0, 0.0, 1.0, 0.0 };
			GLfloat EyePlaneQ[] = { 0.0, 0.0, 0.0, 1.0 };

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_S, GL_OBJECT_PLANE, EyePlaneS);

			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_T, GL_OBJECT_PLANE, EyePlaneT);

			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_R, GL_OBJECT_PLANE, EyePlaneR);

			glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_Q, GL_OBJECT_PLANE, EyePlaneQ);

			// enable all planes
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
			glEnable( GL_TEXTURE_GEN_Q );
			break;
		}

		case TEXCOORD_NORMAL:
		case TEXCOORD_REFLECT:
			break;

		default:
			break;
	}
}

***/

/*** stencil buffer

bool MainGame(void)											// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();										// Reset The Current Modelview Matrix

	timer.Update();
	rinput.Update();
	input.Update();
	player.Update();
	camera.Update();
	psystem.Update();

	glRotatef(-camera.rotX, 1.0f, 0.0f, 0.0f);
	glRotatef(-camera.rotY, 0.0f, 1.0f, 0.0f);
	glTranslatef(-camera.posX, -camera.posY, -camera.posZ);

	// Don't update color or depth.
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Draw 1 into the stencil buffer.
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 1);

	// Now drawing the floor just tags the floor pixels
    // as stencil value 1.
	glBegin(GL_TRIANGLE_STRIP);
//		glTexCoord2f(0.f, 5.f);
		glVertex3f(-10.f, 0, -10.f);
//		glTexCoord2f(5.f, 5.f);
		glVertex3f(10.f, 0, -10.f);
//		glTexCoord2f(0.f, 0.f);
		glVertex3f(-10.f, 0, 10.f);
//		glTexCoord2f(5.f, 0.f);
		glVertex3f(10.f, 0, 10.f);
	glEnd();

	// Re-enable update of color and depth.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// Now, only render where stencil is set to 1.
	glStencilFunc(GL_EQUAL, 1, 1);  // draw if stencil ==1
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glPushMatrix();							// alien tükörkép rendering
	glScalef(1.0f, -1.0f, 1.0f);
	glCullFace(GL_BACK);
	player.Render();
	psystem.Render();
	glCullFace(GL_FRONT);
	glPopMatrix();

	glDisable(GL_STENCIL_TEST);

	glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	floortex.Bind();
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.f, 5.f);
		glVertex3f(-10.f, 0, -10.f);
		glTexCoord2f(5.f, 5.f);
		glVertex3f(10.f, 0, -10.f);
		glTexCoord2f(0.f, 0.f);
		glVertex3f(-10.f, 0, 10.f);
		glTexCoord2f(5.f, 0.f);
		glVertex3f(10.f, 0, 10.f);
	glEnd();
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	player.Render();
	psystem.Render();

	SwapBuffers(hDC);
	return true;
}

***/

/***


Vector4 glToScreen(const Vector4& v) {

    // Get the matrices and viewport

    double modelView[16];
    double projection[16];
    double viewport[4];
    double depthRange[2];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetDoublev(GL_VIEWPORT, viewport);
    glGetDoublev(GL_DEPTH_RANGE, depthRange);

    // Compose the matrices into a single row-major transformation


    Vector4 T[4];
    int r, c, i;
    for (r = 0; r < 4; ++r) {
        for (c = 0; c < 4; ++c) {
            T[r][c] = 0;
            for (i = 0; i < 4; ++i) {
                // OpenGL matrices are column major


                T[r][c] += projection[r + i * 4] * modelView[i + c * 4];
            }
        }
    }

    // Transform the vertex

    Vector4 result;
    for (r = 0; r < 4; ++r) {
        result[r] = T[r].dot(v);
    }

    // Homogeneous divide

    const double rhw = 1 / result.w;

    return Vector4(
        (1 + result.x * rhw) * viewport[2] / 2 + viewport[0],
        (1 - result.y * rhw) * viewport[3] / 2 + viewport[1],
        (result.z * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0],
        rhw);
}



Here's my favorite formula to project on screen:

xScreen = ( xInViewSpace / zInViewSpace * focusDist ) + screenWithHalf;
yScreen = ( yInViewSpace / zInViewSpace * focusDist ) + screenHeightHalf

where focusDist = screeenHeightHalf / Tan( fov/2 );


Works the same way for sizes. For example, to find an approximate size of a 3D object on screen (in pixels)

sizeInPixels = size3D / zInViewSpace * focusDist;

***/