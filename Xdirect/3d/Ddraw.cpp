
#ifndef XLIB
#define XLIB
#endif

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_MEAN_AND_LEAN
#define INITGUID
#include <windows.h>
#include <windowsx.h>
#include <float.h>
#include <limits.h>
#include <string.h>

#include <ddraw.h>
#include <d3d.h>

#define FOG_ENABLE

#include <xlib.h>
#include <xinner.h>
#include "xd3d.h"
#include "D3DUtil.h"
#include "dllmain.h"

RCSID( "$Id: Ddraw.cpp,v 1.2 2003/09/22 13:59:58 bernie Exp $" )

#ifdef _MSC_VER
#pragma comment(lib, "ddraw")
#endif


#define GFXDLL_VERSION "1.0"
char *GFXDLL_NAME = "xd3d.drv";

#pragma comment( exestr , "Direct3D driver v" GFXDLL_VERSION " (" __DATE__ " " __TIME__ ")")

// NT4 Sp3	Diretcx v3.0
// Win98SE	v6.1
// W2K		v7.0

// eredetileg 16 bit volt
// nem tudom 32 mindehol mûködik-e
#define FIRSTTEXBPP	32 // 32
#define SECONDTEXBPP	16

// HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Class\Display\001\DriverDesc

#include "..\dderr.h"

#define DXMSG(str) if( 1 ) MessageBox( hw_state.hwnd, str, "Message", MB_OK );

void (*dprintf)( char *s, ... ) = NULL;
int (*dCheckParm)( char *s ) = NULL;
BOOL (*dwinFullscreen)( void ) = NULL;
texmap_t* (*dTexForNum)( int texmapid ) = NULL;
void (*dCopyVector)( point3_t dst, point3_t src ) = NULL;
FLOAT (*dDotProduct)( point3_t v1, point3_t v2 ) = NULL;
ULONG (*dGetBackHwnd)( void ) = NULL;
int (*dfree)( void * ) = NULL;


typedef HRESULT (WINAPI *LPDIRECTDRAWCREATE)(GUID *, LPDIRECTDRAW *, IUnknown *);
LPDIRECTDRAWCREATE pDirectDrawCreate = NULL;


static LPDIRECTDRAW DirectDraw1 = NULL;
static LPDIRECTDRAW4 DirectDraw4 = NULL;
static LPDIRECTDRAWSURFACE4 PrimarySurface = NULL;
static LPDIRECTDRAWSURFACE4 SecondarySurface = NULL;
static LPDIRECTDRAWPALETTE DirectDrawPalette = NULL;
static LPDIRECTDRAWCLIPPER lpDDClipper = NULL;
static const GUID *pDeviceGUID = NULL;


static IDirect3D3 *Direct3D = NULL;
static IDirect3DDevice3 *d3dDevice = NULL;
static IDirect3DViewport3 *d3dViewport = NULL;
static LPDIRECTDRAWSURFACE4 lpZBuffer = NULL;
static char d3dDName[512] = "";		// device name
char d3dName[256] = "HAL";

static char d3d_error_str[1024] = "no error";

// static int myFlag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;


// options

static BOOL TexturePerspective = TRUE;
static BOOL TextureDither = TRUE;
// static D3DTEXTUREOP TextureBlend = D3DTOP_MODULATE; // MODULATE, ADD

static D3DTEXTUREMAGFILTER magfilter = D3DTFG_LINEAR; // D3DTFG_POINT D3DTFG_LINEAR D3DTFG_FLATCUBIC
static D3DTEXTUREMINFILTER minfilter = D3DTFN_LINEAR; // D3DTFN_POINT
static D3DTEXTUREMIPFILTER mipfilter = D3DTFP_NONE; // D3DTFP_POINT D3DTFP_LINEAR D3DTFP_NONE

static BOOL isGouraud = FALSE;

static d3dtexture_t *d3dtexture = NULL;
static int nd3dtextures = 0;
static int alloced_texmap = 0;

#define D3DALLOCBLOCK  20000	// TODO: vajon a CreateMaterial miatt kell statikus pointer?


static int texmapid_cnt = 1000;


static FLOAT d3dzclipnear = 0.0f;
static FLOAT d3dzclipfar = 1.0f;


hardwarestate_t hw_state;
static BOOL bInGraphic = FALSE;

int drmask,dgmask,dbmask;
int drsize,dgsize,dbsize;
int drshift,dgshift,dbshift;

static FLOAT d3dfognear = 0.5f, d3dfogfar = 0.8f, d3dinvfog = 1.0f;
static ULONG d3dfogcolor = 0L;

static point3_t vpos,		// viewer's position
		vdir,		//  Ä´ÃÄ    direction
		vnormal,	//  Ä´ÃÄ    normal = vpos - vdir;
		mvnormal;	// mvnormal = dir - pos;

static RECT rcScreenRect;
static RECT rcViewportRect;

static int d3d_active_flag = 1;

#define CHECKACTIVE	{ if( d3d_active_flag < 0 ) return; }
#define CHECKACTIVEN	{ if( d3d_active_flag < 0 ) return FALSE; }

#define HM_NOTEX	(-99)


#define UPDATEBLTRECT {									\
	GetClientRect( (HWND)hw_state.hwnd, &rcScreenRect);				\
	GetClientRect( (HWND)hw_state.hwnd, &rcViewportRect );				\
	ClientToScreen( (HWND)hw_state.hwnd, (POINT*)&rcScreenRect.left );		\
	ClientToScreen( (HWND)hw_state.hwnd, (POINT*)&rcScreenRect.right );		\
	/* dprintf("rcScreenRect: %d,%d %d,%d.\n", rcScreenRect.left, rcScreenRect.top, rcScreenRect.right, rcScreenRect.bottom ); */ \
	/* dprintf("rcViewportRect: %d,%d %d,%d.\n", rcViewportRect.left, rcViewportRect.top, rcViewportRect.right, rcViewportRect.bottom ); */ \
}


//
//
//
static void GFXDLLCALLCONV D3D_SetupCulling( point3_t pos, point3_t dir, point3_t normal, FLOAT zclipnear, FLOAT zclipfar ) {

	D3DVIEWPORT2 viewData;
	HRESULT error;

	if( pos ) dCopyVector( vpos, pos );
	if( dir ) dCopyVector( vdir, dir );
	if( normal ) dCopyVector( vnormal, normal );

	d3dzclipnear = zclipnear;
	d3dzclipfar = zclipfar;

	// dprintf("%.2f %.2f\n",d3dzclipnear,d3dzclipfar);

	memset( &viewData, 0, sizeof(D3DVIEWPORT2) );

	FLOAT aspect = (FLOAT)hw_state.SCREENW / (FLOAT)hw_state.SCREENH;

	viewData.dwSize = sizeof(D3DVIEWPORT2);
	viewData.dwX = 0;
	viewData.dwY = 0;
	viewData.dwWidth  = hw_state.SCREENW;
	viewData.dwHeight = hw_state.SCREENH;

	viewData.dvClipX = -1;
	viewData.dvClipWidth = 2;

	viewData.dvClipY = aspect;
	viewData.dvClipHeight = 2 * aspect;

	viewData.dvMinZ = -1.0f;
	viewData.dvMaxZ = 1.0f;

	if( (error=d3dViewport->SetViewport2( &viewData )) != DD_OK ) {
		sprintf( d3d_error_str, "D3D_SetupCulling: SetViewport2 failed.\n\nError: %s",D3D_Error(error));
		dllQuit( d3d_error_str );
	}

	if( (error=d3dDevice->SetCurrentViewport( d3dViewport )) != DD_OK ) {
		sprintf( d3d_error_str, "D3D_SetupCulling: SetCurrentViewport failed.\n\nError: %s",D3D_Error(error));
		dllQuit( d3d_error_str );
	}

	return;
}

/***
>We are using the following formula to calculate the uv coordinate
>(pixelXY+1/256)

   You're going to want to try some combinations of adding a
half-texel shifts on the UV coords above. Thanks to there not being a
hard standard on all of this (blame MS), you'll need to do a bit of
tweaking on this.

   One thing that was done on a previous game was to do a test as the
game started up: some colored blocks were drawn to a corner of the
screen, and they were read back out of the framebuffer. That info was
used to dynamically determine what the texelshift should be.
***/



//
//
//
static BOOL GFXDLLCALLCONV D3D_Rasterize( trans2pipeline_t trans2pipeline ) {
// polygon_t ppoly, polygon3_t poly, polygon3_t tpoly, point3_t *pointnormal,
// point_t *outtex, point3_t *inst, face_t *f, int *rgb ) {

#define P(tag) trans2pipeline.tag

	int k,idx;
	int r=0,g=0,b=0,a=255;
	D3DTLVERTEX vert;
	texmap_t *texmap = NULL;
	d3dtexture_t *d3dtexmap = NULL;
	BOOL alpha = FALSE;
	int texmapid = -1;
	rgb_t *outrgb = NULL;
	point3_t *outst;
	FLOAT dot;

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

		d3dtexmap = NULL;

		for( idx=0; idx<nd3dtextures; idx++ )
			if( d3dtexture[idx].texmapid == texmap->hwtexmapid ) {
				d3dtexmap = &d3dtexture[idx];
				break;
			}

		if( d3dtexmap == NULL )
			return TRUE;

		if( hw_state.mode != HM_TEXTURE ) {

			//d3dDevice->SetLightState( D3DLIGHTSTATE_MATERIAL, d3dtexmap->hMat );
			//d3dDevice->SetLightState( D3DLIGHTSTATE_AMBIENT, 0x40404040 );

							// D3DTOP_MODULATE D3DTOP_ADD
			d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
			d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
			d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

			// bilinear filtering + mipmap = trilinear

			d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, magfilter );
			d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, minfilter );
			d3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, mipfilter );

			// d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

			d3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
			d3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

			hw_state.texmapid = HM_NOTEX;
			hw_state.mode = HM_TEXTURE;
		}

		if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {

			d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
			d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x08);
			d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

			d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
			d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

			d3dDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, TRUE);
			d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
			d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
		}

		if( texmap->isAlpha ) {

			d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
			d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );

			d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
			d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );

			// az alatta levõ látszódjon
			d3dDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		}

		if( texmap->isChromaKey ) {

			d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
		}

		if( hw_state.texmapid != texmapid ) {

			// if( d3dtexmap->pddsSurface->IsLost() )
			//	d3dtexmap->pddsSurface->Restore();

			// d3dDevice->SetTexture( 0, D3DTextr_GetTexture( d3dtexmap->d3dtexmapid ) );

			if( d3dtexmap->pddsSurface->IsLost() || d3dtexmap->ptexTexture == NULL )
				TextrRestore( idx );

			if( d3dDevice->SetTexture( 0, d3dtexmap->ptexTexture ) != D3D_OK )
				; // dprintf("D3D_Rasterize: SetTexture wtf?\n");

			hw_state.texmapid = texmapid;
		}
	}
	else
	if( ISFLAG( P(face)->flag, FF_COLOR ) ) {

		// fillezett

		if( hw_state.mode != HM_COLOR ) {

			d3dDevice->SetTexture( 0, NULL );

			d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
			d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
			d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

			d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
			d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

			hw_state.texmapid = HM_NOTEX;
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
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
		if( (texmap != NULL) && (texmap->isAlpha == TRUE) )
			d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		else
			d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}


	if( ISFLAG( P(face)->flag, FF_DBLSIDE ) || ISFLAG( P(face)->flag, FF_FORCEFLAG ) ) {
		if( hw_state.cullmode != D3DCULL_NONE ) {
			d3dDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
			hw_state.cullmode = D3DCULL_NONE;
		}
	}
	else {
		if( hw_state.cullmode != D3DCULL_CW ) {
			d3dDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_CW );
			hw_state.cullmode = D3DCULL_CW;
		}
	}

	if( ISFLAG( P(face)->flag, FF_FARTHEST ) ) {
		if( hw_state.zbuffermode == TRUE ) {
			d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
			d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE );
			d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE );
			hw_state.zbuffermode = FALSE;
		}
	}
	else
	if( hw_state.zbuffermode == FALSE ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL );
		if( (texmap == NULL) || (texmap->isAlpha == FALSE) )
			d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE );
		hw_state.zbuffermode = TRUE;
	}


	FLOAT inv255 = 1.0f / 256.0f;
	FLOAT invZfar = 1.0f / d3dzclipfar;
	FLOAT equ255 = 256.0f;
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

	d3dDevice->Begin( D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS );

	for( k=0; k<P(ppoly).npoints; k++ ) {

		vert.dvSX  = D3DVAL( P(ppoly).point[k][0] );
		vert.dvSY  = D3DVAL( P(ppoly).point[k][1] );

		vert.dvSZ  = D3DVAL( P(tpoly).point[k][2] * invZfar );

		// vert.dvRHW = P(tpoly).point[k][2];
		vert.dvRHW = D3DVAL( 1.0f / vert.dvSZ );

		r = outrgb[k].r;
		g = outrgb[k].g;
		b = outrgb[k].b;
		a = outrgb[k].a;

		if( texmapid != -1 ) { // ISFLAG( P(face)->flag, FF_TEXTURE ) ) {

			vert.dvTU = D3DVAL( (outst[k][0] + 0.5f) * invU );
			vert.dvTV = D3DVAL( (outst[k][1] + 0.5f) * invV );

			// CLAMPMINMAX( vert.dvTU, 0.0f, 1.0f );
			// CLAMPMINMAX( vert.dvTV, 0.0f, 1.0f );
		}
		else
		if( isGouraud == TRUE ) {

			#define ADDCOLOR ( 100 )
			#define MAXCOLOR ( 255 - ADDCOLOR )

			dot = -dDotProduct( vnormal, trans2pipeline.outpointnormal[k]  );
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

		// vert.dcColor = RGBA_MAKE( r, g, b, a );

#ifdef FOG_ENABLE

		if( (P(tpoly).point[k][2] > d3dfognear) && !ISFLAG( P(face)->flag, FF_FARTHEST ) ) {

			// vert.dcSpecular

			if( P(tpoly).point[k][2] > d3dfogfar )
				a = 0; // vert.dcSpecular = 0x00000000 | vert.dcColor;
			else {

				// vert.dcSpecular = (255 - (DWORD)((P(tpoly).point[k][2] - d3dfognear) * d3dinvfog)) << 24 | vert.dcColor;
				a -= (DWORD)((P(tpoly).point[k][2] - d3dfognear) * d3dinvfog);

				// köd kell, majd kikapcsolja a logika
				if( alpha == FALSE && a < 255 ) {
					alpha = TRUE;
					d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
					d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
					d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
					if( (texmap != NULL) && (texmap->isAlpha == TRUE) )
						d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
				}

				/***
				FLOAT scale = (P(tpoly).point[k][2] - d3dfognear) * d3dinvfog;

				vert.dcSpecular.dvA = (D3DVALUE)scale;
					((ftoi(scale * (FLOAT)((d3dfogcolor>>16)&0xff)) << 16)&0xff0000) |
					((ftoi(scale * (FLOAT)((d3dfogcolor>>8)&0xff)) << 8)&0x00ff00) |
					((ftoi(scale * (FLOAT)((d3dfogcolor>>0)&0xff)) << 0)&0x0000ff);
				***/
			}
		}
		else
			; // vert.dcSpecular = 0xff000000 | vert.dcColor;
#endif

		CLAMPMINMAX( a, 0, 255 );

		vert.dcColor = RGBA_MAKE( r, g, b, a );
		vert.dcSpecular = vert.dcColor;

		d3dDevice->Vertex( &vert );
	}

	d3dDevice->End(0);

	if( (alpha == TRUE) || (texmap && (texmap->isAlpha)) ) {

		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO );
		d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );

		if( hw_state.zbuffermode == TRUE )
			d3dDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
	}

	if( texmap ) {

		if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {
			d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
			d3dDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, FALSE);
		}

		if( texmap->isChromaKey )
			d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
	}

	return TRUE;
}



//
//
//
static BOOL GFXDLLCALLCONV D3D_AddToPipeline( trans2pipeline_t trans2pipeline ) {

#define P(tag) trans2pipeline.tag

	P(faceid) = 0;
	D3D_Rasterize( trans2pipeline );

	if( ISFLAG( P(face)->flag, FF_ENVMAP) ) {
		P(faceid) = 1;
		for( int i=0; i<P(ppoly).npoints; i++ ) {
			P(outrgb1)[i].r = P(outrgb)[i].r;
			P(outrgb1)[i].g = P(outrgb)[i].g;
			P(outrgb1)[i].b = P(outrgb)[i].b;
		}
		D3D_Rasterize( trans2pipeline );
	}

#undef P

	return TRUE;
}




//
//
//
static void GFXDLLCALLCONV D3D_PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) {

	int k,r,g,b,a,idx;
	D3DTLVERTEX vert;
	texmap_t *texmap = NULL;
	d3dtexture_t *d3dtexmap = NULL;
	BOOL alpha = FALSE;

	CHECKACTIVE;

	if( (texmap = dTexForNum(texmapid)) == NULL )
		return;

	d3dtexmap = NULL;

	for( idx=0; idx<nd3dtextures; idx++ )
		if( d3dtexture[idx].texmapid == texmap->hwtexmapid ) {
			d3dtexmap = &d3dtexture[idx];
			break;
		}

	if( d3dtexmap == NULL )
		return;

	if( (hw_state.texmapid != texmapid) || (hw_state.mode != HM_TEXTURE) ) {

		// d3dDevice->SetTexture( 0, D3DTextr_GetTexture( d3dtexmap->d3dtexmapid ) );

		if( d3dtexmap->pddsSurface->IsLost() || d3dtexmap->ptexTexture == NULL )
			TextrRestore( idx );

		if( d3dDevice->SetTexture( 0, d3dtexmap->ptexTexture ) != D3D_OK )
			; // dprintf("D3D_PutSpritePoly: SetTexture wtf?\n");

		//d3dDevice->SetLightState( D3DLIGHTSTATE_MATERIAL, d3dtexture->hMat );
		//d3dDevice->SetLightState( D3DLIGHTSTATE_AMBIENT, 0x40404040 );

		d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

		d3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

		hw_state.texmapid = texmapid;
		hw_state.mode = HM_TEXTURE;
	}

	// FIXME: MAGFILTER csak ezzel jó D3DTFG_POINT
	d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, /*magfilter*/ D3DTFG_POINT );
	d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, minfilter );
	d3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, mipfilter );

	if( texmap->isChromaKey ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
	}
	else
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

	if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {

		d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
		d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x08);
		d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

		d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		d3dDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, TRUE);
		d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
	}
	else
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );

	if( texmap->isAlpha ) {

		d3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );

		d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );
	}

	if( hw_state.zbuffermode == TRUE ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE );
		hw_state.zbuffermode = FALSE;
	}

	// a szinekben meagadott alpha

	for( k=0; k<ppoly.npoints; k++ )
		if( rgb[k].a < 255 ) {
			alpha = TRUE;
			break;
		}

	if( alpha == TRUE ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
		if( (texmap != NULL) && (texmap->isAlpha == TRUE) )
			d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		else
			d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	if( hw_state.cullmode != D3DCULL_NONE ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
		hw_state.cullmode = D3DCULL_NONE;
	}

	if( hw_state.zbuffermode == TRUE ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, FALSE );
		hw_state.zbuffermode = FALSE;
	}

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

	d3dDevice->Begin( D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP );

	for( k=0; k<ppoly.npoints; k++ ) {

		vert.sx = (FLOAT)ppoly.point[k][0];
		vert.sy = (FLOAT)ppoly.point[k][1];
		vert.sz = FLOAT_EPSILON;
		vert.rhw = D3DVAL( 1.0f / vert.dvSZ );

		vert.dvTU = D3DVAL( (inst[k][0] + 0.5) * invU );
		vert.dvTV = D3DVAL( (inst[k][1] + 0.5) * invV );

		// CLAMPMINMAX( vert.dvTU, invU, 1.0f );
		// CLAMPMINMAX( vert.dvTV, invV, 1.0f );

		r = rgb[k].r;
		g = rgb[k].g;
		b = rgb[k].b;
		a = rgb[k].a;

		CLAMPMINMAX( r, 0, 255 );
		CLAMPMINMAX( g, 0, 255 );
		CLAMPMINMAX( b, 0, 255 );
		CLAMPMINMAX( a, 0, 255 );

		vert.dcColor = RGBA_MAKE( r, g, b, a );
		vert.dcSpecular = 0xff000000;

		d3dDevice->Vertex( &vert );
	}

	d3dDevice->End(0);

	if( SPRPIXELLEN( texmap->sprite1 ) == 4 ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		d3dDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, FALSE);
	}

	if( texmap->isAlpha || (alpha == TRUE) ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO );
		d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );
	}

	if( texmap->isChromaKey ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
	}

	return;
}



//
//
//
static void GFXDLLCALLCONV D3D_PutPoly( polygon_t ppoly, rgb_t *rgb ) {

	int k,r,g,b,a;
	D3DTLVERTEX vert;
	BOOL alpha = FALSE;

	CHECKACTIVE;

	if( hw_state.mode != HM_COLOR ) {

		d3dDevice->SetTexture( 0, NULL );

		d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

		hw_state.texmapid = HM_NOTEX;
		hw_state.mode = HM_COLOR;
	}

	// a szinekben meagadott alpha

	for( k=0; k<ppoly.npoints; k++ )
		if( rgb[k].a < 255 ) {
			alpha = TRUE;
			break;
		}

	if( alpha == TRUE ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
		// d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	if( hw_state.cullmode != D3DCULL_NONE ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
		hw_state.cullmode = D3DCULL_NONE;
	}

	if( hw_state.zbuffermode == TRUE ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, FALSE );
		hw_state.zbuffermode = FALSE;
	}

	d3dDevice->Begin( /*D3DPT_TRIANGLESTRIP*/ D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP );

	for( k=0; k<ppoly.npoints; k++ ) {

		vert.sx = (FLOAT)ppoly.point[k][0];
		vert.sy = (FLOAT)ppoly.point[k][1];
		vert.sz = FLOAT_EPSILON;
		vert.rhw = D3DVAL( 1.0f / vert.dvSZ );

		r = rgb[k].r;
		g = rgb[k].g;
		b = rgb[k].b;
		a = rgb[k].a;

		CLAMPMINMAX( r, 0, 255 );
		CLAMPMINMAX( g, 0, 255 );
		CLAMPMINMAX( b, 0, 255 );
		CLAMPMINMAX( a, 0, 255 );

		vert.dcColor = RGBA_MAKE( r, g, b, a );
		vert.dcSpecular = 0xff000000;

		d3dDevice->Vertex( &vert );
	}

	d3dDevice->End(0);

	d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, magfilter );
	d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, minfilter );
	d3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, mipfilter );

	if( alpha == TRUE ) {
		d3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO );
		d3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );
	}

	return;
}





//-----------------------------------------------------------------------------
// Name: TextureSearchCallback()
// Desc: Enumeration callback routine to find a best-matching texture format.
//	 The param data is the DDPIXELFORMAT of the best-so-far matching
//	 texture. Note: the desired BPP is passed in the dwSize field, and the
//	 default BPP is passed in the dwFlags field.
//-----------------------------------------------------------------------------
HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT *pddpf, VOID *param ) {

	if( pddpf == NULL || param == NULL )
		return DDENUMRET_OK;

	TEXTURESEARCHINFO *ptsi = (TEXTURESEARCHINFO*)param;

	if( ptsi->dump_info == TRUE ) {
		dprintf("=================================\n");
		dprintf("tex format: dwSize = %d\n", pddpf->dwSize );
		dprintf("tex format: dwFlags = %d\n",pddpf->dwFlags );
		dprintf("tex format: dwFourCC = %d\n",pddpf->dwFourCC );
		dprintf("tex format: dwRGBBitCount = %d\n",pddpf->dwRGBBitCount );
		dprintf("tex format: dwRBitMask = %d\n",pddpf->dwRBitMask );
		dprintf("tex format: dwGBitMask = %d\n",pddpf->dwGBitMask );
		dprintf("tex format: dwBBitMask = %d\n",pddpf->dwBBitMask );
		dprintf("tex format: dwRGBAlphaBitMask = %d\n",pddpf->dwRGBAlphaBitMask );
		dprintf("tex format: dwAlphaBitDepth = %d\n",pddpf->dwAlphaBitDepth );
		dprintf("=================================\n");
	}

	// Skip any funky modes
	if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
		return DDENUMRET_OK;

	// Check for palettized formats
	if( ptsi->bUsePalette ) {

		if( !( pddpf->dwFlags & DDPF_PALETTEINDEXED8 ) )
			return DDENUMRET_OK;

		// Accept the first 8-bit palettized format we get
		memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
		ptsi->bFoundGoodFormat = TRUE;

		return DDENUMRET_CANCEL;
	}

	// Else, skip any paletized formats (all modes under 16bpp)
	if( pddpf->dwRGBBitCount < 16 )
		return DDENUMRET_OK;

	// Check for FourCC formats
	if( ptsi->bUseFourCC ) {
		if( pddpf->dwFourCC == 0 )
			return DDENUMRET_OK;
		return DDENUMRET_CANCEL;
	}

	// Else, skip any FourCC formats
	if( pddpf->dwFourCC != 0 )
		return DDENUMRET_OK;

	// Make sure current alpha format agrees with requested format type
	if( (ptsi->bUseAlpha==TRUE) && !(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
		return DDENUMRET_OK;

	if( (ptsi->bUseAlpha==FALSE) && (pddpf->dwFlags&DDPF_ALPHAPIXELS) )
		return DDENUMRET_OK;

	// Check if we found a good match
	if( pddpf->dwRGBBitCount == ptsi->dwDesiredBPP ) {
		memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
		ptsi->bFoundGoodFormat = TRUE;
		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: CopyBitmapToSurface()
// Desc: Copies the image of a bitmap into a surface
//-----------------------------------------------------------------------------
static HRESULT CopyBitmapToSurface( int num ) {

	LPDIRECTDRAWSURFACE4 pddsTarget = d3dtexture[num].pddsSurface;
	DWORD dwFlags = d3dtexture[num].dwFlags;

	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW4 pDD = NULL;
	pddsTarget->GetDDInterface( (VOID**)&pDD );

	// Get the bitmap structure (to extract width, height, and bpp)
	BITMAP bm;
	GetObject( d3dtexture[num].hbmBitmap, sizeof(BITMAP), &bm );

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	D3DUtil_InitSurfaceDesc( &ddsd );
	pddsTarget->GetSurfaceDesc( &ddsd );
	ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;    // DDSCAPS_ALPHA
	ddsd.ddsCaps.dwCaps2 = 0L;
	ddsd.dwWidth = bm.bmWidth;
	ddsd.dwHeight = bm.bmHeight;

	if( d3dtexture[num].bHasAlpha == TRUE ) {
		ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
		ddsd.ddpfPixelFormat.dwRBitMask        = 0x00FF0000;
		ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000FF00;
		ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000FF;
	}

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE4 pddsTempSurface = NULL;
	HRESULT hr;
	if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsTempSurface, NULL ) ) ) {
		SAFE_RELEASE(pDD);
		return DDENUMRET_CANCEL;
	}

	// Get a DC for the bitmap
	HDC hdcBitmap = CreateCompatibleDC( NULL );
	if( hdcBitmap == NULL ) {
		SAFE_RELEASE(pddsTempSurface);
		SAFE_RELEASE(pDD);
		return DDENUMRET_CANCEL;
	}

	SelectObject( hdcBitmap, d3dtexture[num].hbmBitmap );

	// Handle palettized textures. Need to attach a palette
	if( ddsd.ddpfPixelFormat.dwRGBBitCount == 8 ) {

		// Create the color table and parse in the palette
		DWORD pe[256];
		WORD wNumColors = GetDIBColorTable( hdcBitmap, 0, 256, (RGBQUAD*)pe );

		for( WORD i=0; i<wNumColors; i++ ) {

			pe[i] = 0xff000000 + RGB( GetBValue(pe[i]), GetGValue(pe[i]), GetRValue(pe[i]) );

			// Set alpha for transparent pixels
			if( dwFlags & D3DTEXTR_TRANSPARENTBLACK ) {
				if( (pe[i]&0x00ffffff) == 0x00000000 )
					pe[i] &= 0x00ffffff;
			}
			else
			if( dwFlags & D3DTEXTR_TRANSPARENTWHITE ) {
				if( (pe[i]&0x00ffffff) == 0x00ffffff )
					pe[i] &= 0x00ffffff;
			}
		}

		// Create & attach a palette with the bitmap's colors
		LPDIRECTDRAWPALETTE  pPalette;
		if( dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) )
			pDD->CreatePalette( DDPCAPS_8BIT|DDPCAPS_ALPHA, (PALETTEENTRY*)pe, &pPalette, NULL );
		else
			pDD->CreatePalette( DDPCAPS_8BIT, (PALETTEENTRY*)pe, &pPalette, NULL );

		pddsTempSurface->SetPalette( pPalette );
		pddsTarget->SetPalette( pPalette );
		SAFE_RELEASE( pPalette );
	}

	// Copy the bitmap image to the surface.
	HDC hdcSurface;
	if( SUCCEEDED( pddsTempSurface->GetDC( &hdcSurface ) ) ) {
		BitBlt( hdcSurface, 0, 0, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0, SRCCOPY );
		pddsTempSurface->ReleaseDC( hdcSurface );
	}

	DeleteDC( hdcBitmap );
	hdcBitmap = NULL;

	// Copy the temp surface to the real texture surface
	pddsTarget->Blt( NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL );

	if( ddsd.ddpfPixelFormat.dwRGBBitCount == 8 ) {
		LPDIRECTDRAWPALETTE  pPalette;
		DWORD pe[256];

		pddsTempSurface->GetPalette( &pPalette );
		pPalette->GetEntries( 0, 0, 256, (PALETTEENTRY*)&pe );
		SAFE_RELEASE( pPalette );

		pddsTarget->GetPalette( &pPalette );
		pPalette->GetEntries( 0, 0, 256, (PALETTEENTRY*)&pe );
		SAFE_RELEASE( pPalette );
	}

	// Done with the temp surface
	SAFE_RELEASE(pddsTempSurface);

	// For textures with real alpha (not palettized), set transparent bits
	if( ddsd.ddpfPixelFormat.dwRGBAlphaBitMask ) {
		if( d3dtexture[num].bHasAlpha || dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) ) {

			// Lock the texture surface
			DDSURFACEDESC2 ddsd;
			D3DUtil_InitSurfaceDesc( &ddsd );
			while( pddsTarget->Lock( NULL, &ddsd, 0, NULL ) == DDERR_WASSTILLDRAWING )
				Sleep(1);

			DWORD dwAlphaMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
			DWORD dwRGBMask   = ( ddsd.ddpfPixelFormat.dwRBitMask |
					      ddsd.ddpfPixelFormat.dwGBitMask |
					      ddsd.ddpfPixelFormat.dwBBitMask );

			// dprintf("pitch: %d,  width: %d\n", ddsd.lPitch, ddsd.dwWidth );

			DWORD dwColorkey  = 0x00000000; // Colorkey on black
			if( dwFlags & D3DTEXTR_TRANSPARENTWHITE )
				dwColorkey = dwRGBMask;     // Colorkey on white

			// CopyRGBADataToSurface()
			UCHAR *alpha_bits = (SPRPIXELLEN(d3dtexture[num].texmap->sprite1) == 4)? &d3dtexture[num].texmap->sprite1[SPRITEHEADER+3] : NULL;
			DWORD dwAShiftL = 8, dwAShiftR = 0;
			DWORD dwMask;
			for( dwMask=dwAlphaMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwAShiftR++;
				for( ; dwMask; dwMask>>=1 ) dwAShiftL--;

			// dprintf("dsdd.dwRGBBitCount: %d  dsdd.width: %d  dsdd.pitch: %d\n", ddsd.ddpfPixelFormat.dwRGBBitCount, ddsd.dwWidth, ddsd.lPitch );

			// Add an opaque alpha value to each non-colorkeyed pixel
			for( DWORD y=0; y<ddsd.dwHeight; y++ ) {

				WORD  *p16 =  (WORD*)((BYTE*)ddsd.lpSurface + y*ddsd.lPitch);
				DWORD *p32 = (DWORD*)((BYTE*)ddsd.lpSurface + y*ddsd.lPitch);

				for( int x=0; x<ddsd.dwWidth; x++ ) {

					if( ddsd.ddpfPixelFormat.dwRGBBitCount == 16 ) {
						if( ( *p16 &= dwRGBMask ) != dwColorkey )
							*p16 |= dwAlphaMask;
						++p16;
					}

					if( ddsd.ddpfPixelFormat.dwRGBBitCount == 32 ) {

						if( alpha_bits ) {

							DWORD dwPixel = *p32;

							BYTE a = (BYTE)*alpha_bits;
							alpha_bits += 4;

							DWORD da = ((a>>(dwAShiftL))<<dwAShiftR)&dwAlphaMask;

							dwPixel &= ~dwAlphaMask;
							*p32 = dwPixel + da;
						}
						else
						if( ( *p32 &= dwRGBMask ) != dwColorkey )
							*p32 |= dwAlphaMask;

						++p32;
					}
				}
			}

			pddsTarget->Unlock( NULL );
		}
	}

	// TODO: itt kellene a Mipmapeket bemásolni a surfacere
	// http://www.gamedev.net/page/resources/_/technical/directx-and-xna/mip-mapping-in-direct3d-r1233
	if( hw_state.bMipmap == TRUE ) {



	}

	SAFE_RELEASE(pDD);

	return DDENUMRET_OK;
}





/*
 * DDSetColorKey
 * set a color key for a surface, given a RGB.
 * if you pass CLR_INVALID as the color key, the pixel
 * in the upper-left corner will be used.
 */
static HRESULT DDSetColorKey( LPDIRECTDRAWSURFACE4 pdds, COLORREF rgb ) {

	LPDIRECTDRAWSURFACE4 lpDDLevel=NULL, lpDDNextLevel=NULL;
	DDSCAPS2 ddsCaps;
	HRESULT ddres;
	DDCOLORKEY ddck;

	lpDDLevel = pdds;
	lpDDLevel->AddRef();
	ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	ddsCaps.dwCaps2 = 0;

	ddres = DD_OK;
	while( ddres == DD_OK ) {

		ddck.dwColorSpaceLowValue  = rgb; //DDColorMatch(pdds, rgb);
		ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;

		if( (ddres = lpDDLevel->SetColorKey( DDCKEY_SRCBLT /* | DDCKEY_COLORSPACE */ , &ddck )) != DD_OK ) {
			dprintf("DDSetColorKey: failed.\n");
			break;
		}

		ddres = lpDDLevel->GetAttachedSurface( &ddsCaps, &lpDDNextLevel );

		lpDDLevel->Release();
		lpDDLevel = lpDDNextLevel;
	}

	if( (ddres != DD_OK) && (ddres != DDERR_NOTFOUND) )
		return !DD_OK;

	return DD_OK;
}





//
//
//
BOOL TextrRestore( int num ) {

	// Release any previously created objects
	SAFE_RELEASE( d3dtexture[num].ptexTexture );
	SAFE_RELEASE( d3dtexture[num].pddsSurface );

	// Check params
	if( d3dDevice == NULL )
		return FALSE; // DDERR_INVALIDPARAMS;

	// Get the DirectDraw interface for creating surfaces
	LPDIRECTDRAW4 pDD;
	if( ( pDD = D3DUtil_GetDirectDrawFromDevice( d3dDevice ) ) == NULL )
		return FALSE;
	pDD->Release();

	// Get the device caps
	D3DDEVICEDESC ddHwDesc, ddSwDesc;
	DWORD dwDeviceCaps;
	ddHwDesc.dwSize = sizeof(D3DDEVICEDESC);
	ddSwDesc.dwSize = sizeof(D3DDEVICEDESC);
	if( FAILED( d3dDevice->GetCaps( &ddHwDesc, &ddSwDesc ) ) )
		return FALSE;

	if( BOOLEAN(ddHwDesc.dcmColorModel) == TRUE )
	/*if( ddHwDesc.dwFlags )*/
		dwDeviceCaps = ddHwDesc.dpcTriCaps.dwTextureCaps;
	else	dwDeviceCaps = ddSwDesc.dpcTriCaps.dwTextureCaps;

	// Get the bitmap structure (to extract width, height, and bpp)
	BITMAP bm;
	// HBITMAP hbmBitmap = d3dtexture[num].hbmBitmap;
	GetObject( d3dtexture[num].hbmBitmap, sizeof(BITMAP), &bm );
	DWORD dwWidth  = (DWORD)bm.bmWidth;
	DWORD dwHeight = (DWORD)bm.bmHeight;

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	D3DUtil_InitSurfaceDesc( &ddsd );
	ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	ddsd.dwTextureStage  = d3dtexture[num].dwStage;
	ddsd.dwWidth = dwWidth;
	ddsd.dwHeight = dwHeight;

	// Adjust width and height, if the driver requires it
	if( dwDeviceCaps & D3DPTEXTURECAPS_POW2 ) {
		for( ddsd.dwWidth=1;  dwWidth>ddsd.dwWidth;   ddsd.dwWidth<<=1 );
		for( ddsd.dwHeight=1; dwHeight>ddsd.dwHeight; ddsd.dwHeight<<=1 );
	}

	if( dwDeviceCaps & D3DPTEXTURECAPS_SQUAREONLY ) {
		if( ddsd.dwWidth > ddsd.dwHeight ) ddsd.dwHeight = ddsd.dwWidth;
		else				   ddsd.dwWidth  = ddsd.dwHeight;
	}

	if( hw_state.bMipmap == TRUE ) {

		/* count how many mip-map levels we need */
		int mipWidth = ddsd.dwWidth;
		int mipHeight = ddsd.dwHeight;
		int mipLevels = 1;

		/* smallest mip-map we want is 2 x 2 */
		while( (mipWidth > 2) && (mipHeight > 2) ) {
			mipLevels++;
			mipWidth  /= 2;
			mipHeight /= 2;
		}

		if( mipLevels > 1 ) {
			/* tell it we want mip-maps */
			ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
			ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
			ddsd.dwMipMapCount = mipLevels;
		}
	}

	BOOL bUsePalette = ( bm.bmBitsPixel <= 8 );
	BOOL bUseAlpha	 = FALSE;

	if( d3dtexture[num].bHasAlpha ) {
		bUseAlpha = TRUE;
		ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
		ddsd.ddpfPixelFormat.dwRBitMask        = 0x00FF0000;
		ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000FF00;
		ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000FF;
	}

	if( d3dtexture[num].dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) ) {
		if( bUsePalette ) {
			if( dwDeviceCaps & D3DPTEXTURECAPS_ALPHAPALETTE ) {
				bUseAlpha   = TRUE;
				bUsePalette = TRUE;
			}
			else {
				bUseAlpha   = TRUE;
				bUsePalette = FALSE;
			}
		}
	}

	// Setup the structure to be used for texture enumration.
	TEXTURESEARCHINFO tsi;
	tsi.dump_info		= FALSE;
	tsi.pddpf		= &ddsd.ddpfPixelFormat;
	tsi.bUseAlpha		= bUseAlpha;
	tsi.bUsePalette		= bUsePalette;
	tsi.bUseFourCC		= ( ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC );
	tsi.dwDesiredBPP	= FIRSTTEXBPP;
	tsi.bFoundGoodFormat	= FALSE;
	if( d3dtexture[num].dwFlags & D3DTEXTR_32BITSPERPIXEL || bUseAlpha )
		tsi.dwDesiredBPP = 32;

	hw_state.tex24bit = TRUE;

	// Enumerate the texture formats, and find the closest device-supported texture pixel format
	d3dDevice->EnumTextureFormats( TextureSearchCallback, &tsi );

	// nézzük van-e második
	if( tsi.bFoundGoodFormat == FALSE ) {

		tsi.dwDesiredBPP = SECONDTEXBPP;

		hw_state.tex24bit = FALSE;

		// dprintf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH mégegyszer\n");

		d3dDevice->EnumTextureFormats( TextureSearchCallback, &tsi );

		if( tsi.bFoundGoodFormat == FALSE ) {

			// If a palettized format was requested, but not found, default to a
			// 16-bit texture format
			if( bUsePalette ) {

				tsi.bUsePalette  = FALSE;
				tsi.dwDesiredBPP = 16;

				d3dDevice->EnumTextureFormats( TextureSearchCallback, &tsi );

				if( tsi.bFoundGoodFormat == FALSE )
					return FALSE;
			}
			else
				return FALSE;
		}
	}

	// Create a new surface for the texture
	HRESULT hr;
	if( FAILED( hr = pDD->CreateSurface(&ddsd, &d3dtexture[num].pddsSurface, NULL) ) )
		return FALSE;

	// Create the texture
	if( FAILED( d3dtexture[num].pddsSurface->QueryInterface( IID_IDirect3DTexture2, (VOID**)&d3dtexture[num].ptexTexture ) ) )
		return FALSE;

	HRESULT error;
	// Copy the bitmap to the texture surface
	error = CopyBitmapToSurface( num );

	if( d3dtexture[num].dwFlags & D3DTEXTR_CHROMAKEY )
		DDSetColorKey( d3dtexture[num].pddsSurface, RGB(0,0,0) );

	if( error != DDENUMRET_OK )
		return FALSE;

	return TRUE;
}




//
//
//
static int CreateTexture( int num, UCHAR *spr, int dwStage, int dwFlag ) {

	UCHAR *ptr,*sptr;
	int x,y;
	BITMAPINFO Info;
	HDC hdc = NULL;

	d3dtexture[num].dwStage = dwStage;
	d3dtexture[num].dwFlags = dwFlag;

	memset( &Info.bmiHeader, 0, sizeof(BITMAPINFOHEADER) );
	Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Info.bmiHeader.biWidth = SPRITEW(spr);
	Info.bmiHeader.biHeight = SPRITEH(spr);
	Info.bmiHeader.biPlanes = 1;
	Info.bmiHeader.biBitCount = 24;
	Info.bmiHeader.biCompression = BI_RGB;
	Info.bmiHeader.biSizeImage = 0;

	// FIXME: ez nem volt benne az origba
	hdc = GetDC( (HWND)hw_state.hwnd );

	if( (d3dtexture[num].hbmBitmap = CreateDIBSection(hdc,&Info,DIB_RGB_COLORS,(void**)&d3dtexture[num].dib,NULL,0)) == NULL ) {
		if( hdc ) ReleaseDC( NULL, hdc );
		dprintf("createbitmap failed.\n");
		return S_FALSE;
	}

	if( hdc ) ReleaseDC( NULL, hdc );

	// dprintf("\"%s\": %d, %d\n",ptcTexture->strName, SPRITEW(spr), SPRITEH(spr));

	ptr = d3dtexture[num].dib;

	for( y=0; y<SPRITEH(spr); y++ ) {

		sptr = &SPR(spr,0,SPRITEH(spr)-(y+1));

		for( x=0; x<SPRITEW(spr); x++ ) {

			ptr[2] = *sptr++;
			ptr[1] = *sptr++;
			ptr[0] = *sptr++;

			ptr += 3;

			// RGBA
			if( SPRPIXELLEN( spr ) == 4 )
				++sptr;
		}
	}

	return S_OK;
}







//
// only HIGH sprite
//
int D3D_AddTexMapHigh( texmap_t *texmap ) {

	D3DMATERIAL mat;
	HRESULT error;
	int num;
	DWORD dwFlag;

#define ORIG_SPR texmap->sprite1

	// dprintf("texmap: sizeof:%d, %d\n", sizeof(texmap_t), sizeof(texmap));

	if( alloced_texmap < (nd3dtextures + 1) ) {

		alloced_texmap += D3DALLOCBLOCK;
		if( (d3dtexture = (d3dtexture_t*)drealloc( d3dtexture, (alloced_texmap*sizeof(d3dtexture_t)) )) == NULL ) {
			sprintf( d3d_error_str, "D3D_AddTexMapHigh: can't realloc d3dtexture (%d).",alloced_texmap);
			dllQuit( d3d_error_str );
		}
	}

	num = nd3dtextures;
	// dprintf("num,nd3dtextures: %d, %d\n", num, nd3dtextures);

	memset( &d3dtexture[num], 0L, sizeof(d3dtexture_t) );

	d3dtexture[num].texmapid = texmapid_cnt++; // texmap->texmapid;
	d3dtexture[num].texmap = texmap;

	dwFlag = 0;

	if( (texmap->isChromaKey || texmap->isAlpha) && hw_state.bIsColorKey ) {
		// dprintf("chromatexture\n");
		dwFlag |= (D3DTEXTR_CHROMAKEY | D3DTEXTR_TRANSPARENTBLACK);
	}

	d3dtexture[num].bHasAlpha = (SPRPIXELLEN(ORIG_SPR) >= 4);

	if( (error = CreateTexture( num, ORIG_SPR, 0,dwFlag )) != S_OK ) {
		sprintf( d3d_error_str, "D3D_AddTexMapHigh: out of memory.\n");
		dprintf( d3d_error_str );
		return -1;
	}

	/***
	if( texmap->isChromaKey )
		if( (error=DDSetColorKey( D3DTextr_GetSurface( texmap->name ), texmap->chromaColor )) != DD_OK )
			dprintf("D3D_AddTexMapHigh: can't set chroma key.\nError:%s\n",D3D_Error(error));
	***/

	// d3dtexture[num].lpMat = NULL;

	if( (error = Direct3D->CreateMaterial(&(d3dtexture[num].lpMat), NULL)) != D3D_OK ) {
		sprintf( d3d_error_str, "D3D_AddTexMap: CreateMaterial failed.\n\nError: %s",D3D_Error(error) );
		dllQuit( d3d_error_str );
	}

	memset(&mat, 0, sizeof(D3DMATERIAL));
	mat.dwSize = sizeof(D3DMATERIAL);
	mat.diffuse.a = (D3DVALUE)0.0;
	mat.diffuse.r = (D3DVALUE)0.0;
	mat.diffuse.g = (D3DVALUE)0.0;
	mat.diffuse.b = (D3DVALUE)0.0;
	mat.ambient.a = (D3DVALUE)0.0;
	mat.ambient.r = (D3DVALUE)0.0;
	mat.ambient.g = (D3DVALUE)0.0;
	mat.ambient.b = (D3DVALUE)0.0;
	mat.specular.a = (D3DVALUE)0.0;
	mat.specular.r = (D3DVALUE)0.0;
	mat.specular.g = (D3DVALUE)0.0;
	mat.specular.b = (D3DVALUE)0.0;
	mat.power = (FLOAT)0.0;
	mat.dwRampSize = 16L;

	if( (error = d3dtexture[num].lpMat->SetMaterial( &mat )) != D3D_OK ) {
		sprintf( d3d_error_str, "D3D_AddTexMap: SetMaterial failed.\n\nError: %s",D3D_Error(error) );
		dllQuit( d3d_error_str );
	}
	if( (error = d3dtexture[num].lpMat->GetHandle( d3dDevice, &(d3dtexture[num].hMat) )) != D3D_OK ) {
		sprintf( d3d_error_str, "D3D_AddTexMap: GetHandle failed.\n\nError: %s",D3D_Error(error) );
		dllQuit( d3d_error_str );
	}

	++nd3dtextures;

	d3dtexture[num].pddsSurface = NULL;
	d3dtexture[num].ptexTexture = NULL;

	// D3DTextr_Restore( d3dtexture[num].d3dtexmapid, d3dDevice );

	if( TextrRestore( num ) == FALSE )
		dprintf("(TextrRestore failed.) ");

	DDSCAPS2 ddscaps;
	DWORD dwFree=0, dwTotal=0;

	memset( &ddscaps, 0L, sizeof(DDSCAPS2) );
	// ddscaps.dwCaps = DDSCAPS_TEXTURE;
	ddscaps.dwCaps = DDSCAPS_VIDEOMEMORY;

	if( (error=DirectDraw4->GetAvailableVidMem( &ddscaps, &dwTotal, &dwFree )) != DD_OK )
		dprintf("D3D_AddTexMapHigh: GetAvailableVidMem failed (%s).\n",DX_Error(error));

	dwFree /= (1024*1024);

	HDC aDC = GetDC(NULL);
	int aWidth = GetDeviceCaps(aDC, HORZRES);
	int aHeight = GetDeviceCaps(aDC, VERTRES);
	int aBPP = GetDeviceCaps( aDC, BITSPIXEL );
	ReleaseDC(NULL, aDC);

	dwTotal += (aBPP/8)*aWidth*aHeight;
	dwTotal /= (1024*1024);

	dprintf("(remains: %d of %d MB) ", dwFree, dwTotal );

#undef ORIG_SPR

	return d3dtexture[num].texmapid;
}



//
//
//
static int GFXDLLCALLCONV D3D_ReloadTexMapHigh( texmap_t *texmap ) {

	D3DMATERIAL mat;
	HRESULT error;
	int num;
	DWORD dwFlag;

#define ORIG_SPR texmap->sprite1

	for( num=0; num<nd3dtextures; num++ )
		if( d3dtexture[num].texmapid == texmap->hwtexmapid )
			break;

	if( num >= nd3dtextures ) {
		// dprintf("d3d: tex not found. (id: %d  nd3ds: %d)\n",texmap->hwtexmapid,nd3dtextures);
		return D3D_AddTexMapHigh( texmap );
	}

	// D3DTextr_DestroyTexture( d3dtexture[num].d3dtexmapid );

	SAFE_RELEASE( d3dtexture[num].lpMat );

	dwFlag = 0;

	if( (texmap->isChromaKey || texmap->isAlpha) && hw_state.bIsColorKey ) {
		// dprintf("chromatexture\n");
		dwFlag |= (D3DTEXTR_CHROMAKEY | D3DTEXTR_TRANSPARENTBLACK);
	}

	if( d3dtexture[num].hbmBitmap )
		DeleteObject( d3dtexture[num].hbmBitmap );
	d3dtexture[num].hbmBitmap = NULL;

	if( (error = CreateTexture( num, ORIG_SPR, 0,dwFlag )) != S_OK ) {
		dprintf("D3D_ReloadTexMapHigh: not enough memory.\n");
		return -1;
	}

	/***
	if( texmap->isChromaKey )
		if( (error=DDSetColorKey( D3DTextr_GetSurface( texmap->name ), texmap->chromaColor )) != DD_OK )
			dprintf("D3D_AddTexMapHigh: can't set chroma key.\nError:%s\n",D3D_Error(error));
	***/

	if( (error=Direct3D->CreateMaterial(&d3dtexture[num].lpMat, NULL)) != D3D_OK ) {
		sprintf( d3d_error_str, "D3D_ReloadTexMapHigh: CreateMaterial failed.\n\nError: %s",D3D_Error(error) );
		dllQuit( d3d_error_str );
	}

	memset(&mat, 0, sizeof(D3DMATERIAL));
	mat.dwSize = sizeof(D3DMATERIAL);
	mat.diffuse.a = (D3DVALUE)0.0;
	mat.diffuse.r = (D3DVALUE)0.0;
	mat.diffuse.g = (D3DVALUE)0.0;
	mat.diffuse.b = (D3DVALUE)0.0;
	mat.ambient.a = (D3DVALUE)0.0;
	mat.ambient.r = (D3DVALUE)0.0;
	mat.ambient.g = (D3DVALUE)0.0;
	mat.ambient.b = (D3DVALUE)0.0;
	mat.specular.a = (D3DVALUE)0.0;
	mat.specular.r = (D3DVALUE)0.0;
	mat.specular.g = (D3DVALUE)0.0;
	mat.specular.b = (D3DVALUE)0.0;
	mat.power = (FLOAT)0.0;
	mat.dwRampSize = 16L;

	error = d3dtexture[num].lpMat->SetMaterial( &mat ); // dprintf("setmat: %d\n",error);
	error = d3dtexture[num].lpMat->GetHandle( d3dDevice, &d3dtexture[num].hMat );	// dprintf("gethand: %d\n",error);

	// D3DTextr_Restore( d3dtexture[num].d3dtexmapid, d3dDevice );

	if( TextrRestore( num ) == FALSE )
		dprintf("(TextrRestore failed.)\n");

#undef ORIG_SPR

	return d3dtexture[num].texmapid;
}




//
// FIXME: a sprite azonos méret legyen
//
static int GFXDLLCALLCONV D3D_ModifyTexMapHigh( texmap_t *texmap ) {

	int num;
	DWORD dwFlag;
	HRESULT error;

#define ORIG_SPR texmap->sprite1

	for( num=0; num<nd3dtextures; num++ )
		if( d3dtexture[num].texmapid == texmap->hwtexmapid )
			break;

	if( num >= nd3dtextures ) {
		// dprintf("d3d: tex not found. (id: %d  nd3ds: %d)\n",texmap->hwtexmapid,nd3dtextures);
		return -1;
	}

	/***
	if( d3dtexture[num].hbmBitmap )
		DeleteObject( d3dtexture[num].hbmBitmap );
	d3dtexture[num].hbmBitmap = NULL;

	dwFlag = 0;

	if( (texmap->isChromaKey || texmap->isAlpha) && hw_state.bIsColorKey ) {
		// dprintf("chromatexture\n");
		dwFlag |= (D3DTEXTR_CHROMAKEY | D3DTEXTR_TRANSPARENTBLACK);
	}

	if( (error = CreateTexture( num, ORIG_SPR, 0,dwFlag )) != S_OK ) {
		dprintf("D3D_ModifyTexMapHigh: not enough memory.\n");
		return -1;
	}
	***/

	UCHAR *ptr = d3dtexture[num].dib;

	for( int y=0; y<SPRITEH(ORIG_SPR); y++ ) {

		UCHAR *sptr = &SPR(ORIG_SPR,0,SPRITEH(ORIG_SPR)-(y+1));

		for( int x=0; x<SPRITEW(ORIG_SPR); x++ ) {

			ptr[2] = *sptr++;
			ptr[1] = *sptr++;
			ptr[0] = *sptr++;

			ptr += 3;

			// RGBA
			if( SPRPIXELLEN( ORIG_SPR ) == 4 )
				++sptr;
		}
	}

	if( CopyBitmapToSurface( num ) != DDENUMRET_OK )
		return -1;

#undef ORIG_SPR

	return d3dtexture[num].texmapid;
}




//
// // hwtexmapid-vel hívják
//
static void GFXDLLCALLCONV D3D_DiscardTexture( int texmapid ) {

	int i;

	for( i=0; i<nd3dtextures; i++ )
		if( d3dtexture[i].texmapid == texmapid )
			break;

	if( i >= nd3dtextures ) {
		dprintf("D3D_DiscardTexture: no %d texture.\n",texmapid);
		return;
	}

	// D3DTextr_DestroyTexture( d3dtexture[i].d3dtexmapid );

	SAFE_RELEASE( d3dtexture[i].lpMat );

	SAFE_RELEASE( d3dtexture[i].ptexTexture );
	SAFE_RELEASE( d3dtexture[i].pddsSurface );

	if( d3dtexture[i].hbmBitmap )
		DeleteObject( d3dtexture[i].hbmBitmap );
	d3dtexture[i].hbmBitmap = NULL;

	dprintf("(hwtexmapid: %d) ",d3dtexture[i].texmapid);

	if( ((nd3dtextures-i)-1) > 0 )
		memmove( &d3dtexture[i], &d3dtexture[i+1], ((nd3dtextures-i)-1) * sizeof(d3dtexture_t) );

	memset( &d3dtexture[nd3dtextures-1], 0L, sizeof(d3dtexture_t) );

	--nd3dtextures;

	return;
}



//
//
//
static void GFXDLLCALLCONV D3D_DiscardAllTexture( void ) {

	while( nd3dtextures > 0 )
		D3D_DiscardTexture( d3dtexture[nd3dtextures-1].texmapid );

	if( d3dtexture ) dfree( d3dtexture );

	d3dtexture = NULL;

	nd3dtextures = 0;
	alloced_texmap = 0;

	return;
}





//
// ULONG color
//
static void GFXDLLCALLCONV D3D_Line( int x0, int y0, int x1, int y1, rgb_t color ) {

	int r=0,g=0,b=0,a=255;
	D3DTLVERTEX vert;

	CHECKACTIVE;

	if( hw_state.mode != HM_COLOR ) {

		d3dDevice->SetTexture( 0, NULL );

		d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

		hw_state.texmapid = HM_NOTEX;
		hw_state.mode = HM_COLOR;
	}

	/*
	DWORD _state_zenable;
	DWORD _state_zwriteenable;
	DWORD _state_zbias;
	DWORD _state_Zfunc;
	d3dDevice->GetRenderState(D3DRENDERSTATE_ZENABLE, &_state_zenable);
	d3dDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &_state_zwriteenable);
	d3dDevice->GetRenderState(D3DRENDERSTATE_ZBIAS, &_state_zbias);
	d3dDevice->GetRenderState( D3DRENDERSTATE_ZFUNC, &_state_Zfunc );
	*/

	if( hw_state.zbuffermode == TRUE ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
		hw_state.zbuffermode = FALSE;
	}

	// D3DPT_LINESTRIP  D3DPT_LINELIST
	d3dDevice->Begin( D3DPT_LINESTRIP, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS );

	// x0,y0
	vert.dvSX  = D3DVAL( x0 );
	vert.dvSY  = D3DVAL( y0 );

	vert.dvSZ  = D3DVAL( 0.0001f );
	vert.dvRHW = D3DVAL( 1.0f / vert.dvSZ );

	r = color.r; // INTRED24( color );
	g = color.g; // INTGREEN24( color );
	b = color.b; // INTBLUE24( color );
	a = color.a; // 255;

	CLAMPMINMAX( r, 0, 255 );
	CLAMPMINMAX( g, 0, 255 );
	CLAMPMINMAX( b, 0, 255 );
	CLAMPMINMAX( a, 0, 255 );

	vert.dcColor = RGBA_MAKE( r, g, b, a );
	vert.dcSpecular = vert.dcColor;

	d3dDevice->Vertex( &vert );

	// x1,y1
	vert.dvSX  = D3DVAL( x1 );
	vert.dvSY  = D3DVAL( y1 );

	d3dDevice->Vertex( &vert );

	d3dDevice->End(0);

	/*
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, _state_zenable);
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, _state_zwriteenable);
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, _state_zbias);
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, _state_Zfunc );
	*/

	return;
}


//
//
//
static void GFXDLLCALLCONV D3D_PutPixel( int x, int y, rgb_t color ) {

	int r=0,g=0,b=0,a=255;
	D3DTLVERTEX vert;

	CHECKACTIVE;

	if( hw_state.mode != HM_COLOR ) {

		d3dDevice->SetTexture( 0, NULL );

		d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

		hw_state.texmapid = HM_NOTEX;
		hw_state.mode = HM_COLOR;
	}

	// int i = d3dDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE);
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS );
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
	hw_state.zbuffermode = FALSE;

	d3dDevice->Begin( D3DPT_POINTLIST, D3DFVF_TLVERTEX, D3DDP_DONOTUPDATEEXTENTS );

	vert.dvSX  = D3DVAL( x );
	vert.dvSY  = D3DVAL( y );

	vert.dvSZ  = D3DVAL( 0.0001f );
	vert.dvRHW = D3DVAL( 1.0f / vert.dvSZ );

	r = color.r;
	g = color.g;
	b = color.b;
	a = color.a;

	CLAMPMINMAX( r, 0, 255 );
	CLAMPMINMAX( g, 0, 255 );
	CLAMPMINMAX( b, 0, 255 );
	CLAMPMINMAX( a, 0, 255 );

	vert.dcColor = RGBA_MAKE( r, g, b, a );
	vert.dcSpecular = vert.dcColor;

	d3dDevice->Vertex( &vert );

	d3dDevice->End(0);

	d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,1);
	d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,1);

	return;
}



//
//
//
static void GFXDLLCALLCONV D3D_BeginScene( void ) {

/***
	if ( !SUCCEEDED(mD3DDevice->SetRenderTarget(mDDSDrawSurface, 0))) { // this happens when there's been a mode switch (this caused the nvidia screensaver bluescreen)
		gD3DInterfacePreDrawError = true;
		return false;
	}
	else
		gD3DInterfacePreDrawError = false;
***/

	if( !SUCCEEDED(d3dDevice->SetRenderTarget(SecondarySurface, 0)))
		dprintf("setrendertarget\n");
		// return;

	d3dDevice->BeginScene();

	hw_state.bSceneBegun = TRUE;

	return;
}




//
//
//
static void GFXDLLCALLCONV D3D_EndScene( void ) {

	// FIXME: kell-e ez?
	hw_state.mode = HM_UNINIT;
	hw_state.texmapid = HM_NOTEX;
	d3dDevice->SetTexture( 0, NULL );

	if( hw_state.bSceneBegun == TRUE ) {
		d3dDevice->EndScene();
		hw_state.bSceneBegun = FALSE;
	}

	return;
}




//
//
//
static void GFXDLLCALLCONV D3D_FlushScene( void ) {

	// d3dDevice->Flush();

	return;
}






//
//
//
static void GFXDLLCALLCONV D3D_Fog( FLOAT fognear, FLOAT fogfar, ULONG fogcolor ) {

	int r,g,b;

	//if( !hw_state.fogenable )
	//	return;

	r = ( fogcolor >> 16 ) & 0xff;
	g = ( fogcolor >>  8 ) & 0xff;
	b = ( fogcolor >>  0 ) & 0xff;

	d3dfognear = fognear;
	d3dfogfar = fogfar;
	d3dfogcolor = RGB_MAKE( r,g,b );

	d3dinvfog = 255.0f / (d3dfogfar - d3dfognear);

	// dprintf("d3d_fog: %.2f %.2f\n",d3dfognear,d3dfogfar);

	d3dDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, TRUE );
	d3dDevice->SetRenderState( D3DRENDERSTATE_FOGCOLOR, d3dfogcolor );

	d3dDevice->SetLightState( D3DLIGHTSTATE_FOGMODE, D3DFOG_LINEAR );
	d3dDevice->SetLightState( D3DLIGHTSTATE_FOGEND, *((DWORD *)(&d3dfogfar)) );
	d3dDevice->SetLightState( D3DLIGHTSTATE_FOGSTART, *((DWORD *)(&d3dfognear)) );

	return;
}




static BOOL SurfaceOn = FALSE;

//
//
//
static BOOL GFXDLLCALLCONV D3D_LockLfb( __int64 *mem, int flag ) {

	HRESULT error = DD_OK;
	DDSURFACEDESC2 ddsd;

	USEPARAM( flag );

	SurfaceOn = TRUE;

	// Set up ddsd
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	// dprintf("---------------->Locking\n");

	// Repeat until it works
	while( 1 ) {

		// Try the lock
		if( SecondarySurface )
			error = SecondarySurface->Lock( NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR, NULL);

		// If OK, get out
		if( error == DD_OK )
			break;

		// Correctable errors
		if( error == DDERR_SURFACELOST ) {
			if( PrimarySurface ) PrimarySurface->Restore();
			if( SecondarySurface ) SecondarySurface->Restore();
			if( lpZBuffer ) lpZBuffer->Restore();
			Sleep(1);
			continue;
		}

		if( error == DDERR_WASSTILLDRAWING ) {
			Sleep(1);
			continue;
		}

		// Bad error
		sprintf( d3d_error_str, "D3D_LockLfb: Lock failed.\n\nError: %s",DX_Error(error));
		dllQuit( d3d_error_str );
	}

	*mem = (__int64)ddsd.lpSurface;
	// int pitch = ddsd.lPitch;
	// mem[y*pitch + x] = Pixel;

	/***
	http://archive.gamedev.net/archive/reference/articles/541/pixel.cpp
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_PIXELFORMAT;
	if( SecondarySurface->GetSurfaceDesc ( &ddsd ) != DD_OK )
		return FALSE;
	int rc=0, gc=0, bc=0;
	for( int i=1; i<65536; i<<=1 ) {
		if( ddsd.ddpfPixelFormat.dwRBitMask & i ) ++rc;
		if( ddsd.ddpfPixelFormat.dwGBitMask & i ) ++gc;
		if( ddsd.ddpfPixelFormat.dwBBitMask & i ) ++bc;
	}
	redmask = ddsd.ddpfPixelFormat.dwRBitMask;
	redshift = gc + bc;
	greenmask = ddsd.ddpfPixelFormat.dwGBitMask;
	greenshift = bc;
	bluemask = ddsd.ddpfPixelFormat.dwBBitMask;
	blueshift = 0;
	WORD Pixel = (r << rgb16->Position.rgbRed) |
		     (g << rgb16->Position.rgbGreen) |
                     (b);// << rgb16->Position.rgbBlue);
	WORD *pixels = (WORD *)ddsd->lpSurface;
	DWORD pitch = ddsd->dwWidth;
	pixels[y*pitch + x] = Pixel;
	***/

	return TRUE;
}





//
//
//
static void GFXDLLCALLCONV D3D_UnlockLfb( void ) {

	HRESULT error = DD_OK;

	SurfaceOn = FALSE;

	// dprintf("----------->UNLocking\n");

	// Repeat until it works
	while( 1 ) {

	        // Try the unlock
		if( SecondarySurface )
			error = SecondarySurface->Unlock(NULL);

		// If OK or not locked, get out
		if( error == DD_OK ) break;
		if( error == DDERR_NOTLOCKED ) break;

		// Correctable errors
		if( error == DDERR_SURFACELOST ) {
			if( PrimarySurface ) PrimarySurface->Restore();
			if( SecondarySurface ) SecondarySurface->Restore();
			if( lpZBuffer ) lpZBuffer->Restore();
			Sleep(1);
			continue;
		}

		// Bad error
		sprintf( d3d_error_str, "D3D_UnlockLfb: Unlock failed.\n\nError: %s",DX_Error(error));
		dllQuit( d3d_error_str );
	}

	return;
}





//
//
//
static BOOL GFXDLLCALLCONV D3D_Activate( int flag ) {

	if( flag == 1 ) {

		HRESULT error = DD_OK;

		if( PrimarySurface && PrimarySurface->IsLost() ) {
			if( (error = PrimarySurface->Restore()) != DD_OK ) {

				if( error == DDERR_WRONGMODE ) {
					if( dprintf ) dprintf( "D3D_Activate: PrimarySurface wrong mode (%s).\n", DX_Error(error) );
					// Free(); /* My Free DirectDraw */
					// return Create(); /* My Create DirectDraw */
				}
				else
					if( dprintf ) dprintf( "D3D_Activate: PrimarySurface error = %d (%s).\n", error, DX_Error(error) );
			}
		}

		if( SecondarySurface && SecondarySurface->IsLost() )
			error = SecondarySurface->Restore();

		if( lpZBuffer && lpZBuffer->IsLost() )
			error = lpZBuffer->Restore();

		/***
		for( int i=0; i<nd3dtextures; i++ )
			if( TextrRestore( i ) == FALSE )
				if( dprintf ) dprintf("D3D_Activate: texture restore failed (%d).\n", i );
		***/

		hw_state.texmapid = HM_NOTEX;
		hw_state.mode = HM_UNINIT;

		// if( dprintf ) dprintf( "D3D_Activate: surfaces are restored.\n" );
	}

	d3d_active_flag = flag;

	return FALSE;
}





//
//
//
static void GFXDLLCALLCONV D3D_InitPalette( UCHAR *pal ) {

	PALETTEENTRY lpColorTable[256];
	int i;
	HRESULT error;

	for( i=0; i<256; i++ ) {

		lpColorTable[i].peRed	= ( (pal[(i*3)+0]+1) << 2 ) - 1;
		lpColorTable[i].peGreen = ( (pal[(i*3)+1]+1) << 2 ) - 1;
		lpColorTable[i].peBlue	= ( (pal[(i*3)+2]+1) << 2 ) - 1;
		lpColorTable[i].peFlags = 0;
	}

	if( (error=DirectDraw4->CreatePalette( DDPCAPS_8BIT|DDPCAPS_ALLOW256, lpColorTable, &DirectDrawPalette,NULL )) ) {
		sprintf( d3d_error_str, "D3D_InitPalette: CreatePalette failed.\n\nError: %s",DX_Error(error) );
		dllQuit( d3d_error_str );
	}

	if( (error=PrimarySurface->SetPalette( DirectDrawPalette )) ) {
		sprintf( d3d_error_str, "D3D_InitPalette: SetPalette (Primary) failed.\n\nError: %s",DX_Error(error) );
		dllQuit( d3d_error_str );
	}

	if( (error=SecondarySurface->SetPalette( DirectDrawPalette )) ) {
		sprintf( d3d_error_str, "D3D_InitPalette: SetPalette (Secondary) failed.\n\nError: %s",DX_Error(error) );
		dllQuit( d3d_error_str );
	}

	return;
}






//
//
//
static void GFXDLLCALLCONV D3D_SetRgb( int color, int red, int green, int blue ) {

	PALETTEENTRY rgb;
	HRESULT error;

	if( !DirectDrawPalette ) return;

	rgb.peRed   = ( (red+1)   << 2 ) - 1;
	rgb.peGreen = ( (green+1) << 2 ) - 1;
	rgb.peBlue  = ( (blue+1)  << 2 ) - 1;
	rgb.peFlags = 0;

	if( (error=DirectDrawPalette->SetEntries( 0, color, 1, &rgb )) ) {
		sprintf( d3d_error_str, "D3D_SetRgb: SetEntries failed.\n\nError: %s",DX_Error(error));
		dllQuit( d3d_error_str );
	}

	return;
}






//
//
//
static void GFXDLLCALLCONV D3D_GetRgb( int color, int *red, int *green, int *blue ) {

	PALETTEENTRY rgb;
	HRESULT error;

	if( !DirectDrawPalette )
		return;

	if( (error=DirectDrawPalette->GetEntries( 0, color, 1, &rgb)) ) {
		sprintf( d3d_error_str, "D3D_GetRgb: GetEntries failed.\n\nError: %s",DX_Error(error));
		dllQuit( d3d_error_str );
	}

	*red	= ( ((short)rgb.peRed+1)   / 4) - 1;
	*green	= ( ((short)rgb.peGreen+1) / 4) - 1;
	*blue	= ( ((short)rgb.peBlue+1)  / 4) - 1;

	return;
}




//
//
//
static ULONG GFXDLLCALLCONV D3D_GetPitch( void ) {

	return (ULONG)hw_state.pitch;
}





//
//
//
static void GFXDLLCALLCONV D3D_FlipPage( void ) {

	HRESULT error = DD_OK;

	// WM_ACTIVETE nem mindig érkezik meg idõben
	CHECKACTIVE;

	/***
	int FPS = 10;
	char Buffer[ 256 ] = "";
	sprintf( Buffer , "FPS= %d" , FPS );
	HDC hDC;
	SecondarySurface->GetDC( &hDC );
	SetBkColor( hDC , 0 );
	SetTextColor( hDC , RGB( 0 , 255 , 0 ) );
	TextOut( hDC , 3 , 3 , Buffer , ( int )strlen( Buffer ) );
	SecondarySurface->ReleaseDC( hDC );
	***/

	if( PrimarySurface == NULL )
		return;

	int i = 0;
	while( i++ < 3 ) {

		// if( FAILED( error = d3dDevice->TestCooperativeLevel() ) )

		// Try the lock
		if( dwinFullscreen() )			// DDFLIP_NOVSYNC
			error = PrimarySurface->Flip( NULL, DDFLIP_WAIT );
		else
			error = PrimarySurface->Blt( &rcScreenRect, SecondarySurface, &rcViewportRect, DDBLT_ASYNC | DDBLT_WAIT, NULL );

		// If OK, get out
		if( error == DD_OK )
			break;

		if( error == DDERR_WRONGMODE ) {
			dprintf("D3D_FlipPage: wrong mode (%d).\n", error );
		}

		// Correctable errors
		if( error == DDERR_SURFACELOST ) {
			if( PrimarySurface->IsLost() ) PrimarySurface->Restore();
			if( SecondarySurface && SecondarySurface->IsLost() ) SecondarySurface->Restore();
			if( lpZBuffer && lpZBuffer->IsLost() ) lpZBuffer->Restore();
			dprintf("D3D_FlipPage: restoring surfaces (%d).\n", i );
			Sleep(1);
			continue;
		}

		if( error == DDERR_WASSTILLDRAWING ) {
			dprintf("D3D_FlipPage: was still drawing (%d).\n", i );
			Sleep(1);
			continue;
		}

		// Bad error
		if( dwinFullscreen() )
			sprintf( d3d_error_str, "D3D_FlipPage: Flip failed.\n\nError: %s",DX_Error(error) );
		else
			sprintf( d3d_error_str, "D3D_FlipPage: Blit failed.\n\nError: %s",DX_Error(error) );

		// ha bezátják az ablakot akkor is jöhet ide még
		// dllQuit( d3d_error_str );
	}

	return;
}





//
//
//
static void GFXDLLCALLCONV D3D_Clear( ULONG color, ULONG depth, int flag ) {

	ULONG d3dcolor = 0L;


	if( flag & (CF_TARGET | CF_ZBUFFER) ) {

		d3dcolor = RGBINT24( ( ((d3dfogcolor>>16)&0xff) + ((color>>16)&0xff) ),
				     ( ((d3dfogcolor>> 8)&0xff) + ((color>> 8)&0xff) ),
				     ( ((d3dfogcolor>> 0)&0xff) + ((color>> 0)&0xff) ) );

		d3dViewport->Clear2( 1UL, (D3DRECT*)&rcViewportRect,
					D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
					d3dcolor, 1.0f, 0L );
	}
	else
	if( flag & CF_ZBUFFER ) {

		d3dViewport->Clear2( 1UL, (D3DRECT*)&rcViewportRect,
					D3DCLEAR_ZBUFFER,
					d3dcolor, 1.0f, 0L );
	}
	else
	if( flag & CF_TARGET ) {

		d3dcolor = RGBINT24( ( ((d3dfogcolor>>16)&0xff) + ((color>>16)&0xff) ),
				     ( ((d3dfogcolor>> 8)&0xff) + ((color>> 8)&0xff) ),
				     ( ((d3dfogcolor>> 0)&0xff) + ((color>> 0)&0xff) ) );

		d3dViewport->Clear2( 1UL, (D3DRECT*)&rcViewportRect,
					D3DCLEAR_TARGET,
					d3dcolor, 1.0f, 0L );
	}

	return;
}



combobox_t combobox;


void (*D3D_winReadProfileString)( char *, char *, char *, int ) = NULL;
void (*D3D_winWriteProfileString)( char *, char * ) = NULL;
int (*D3D_winReadProfileInt)( char *, int ) = NULL;
void (*D3D_winWriteProfileInt)( char *, int ) = NULL;


//
// Finding if there is hardware
//
static void CheckHw( GUID *guid = NULL, DWORD *dwMemType = NULL ) {

	D3DFINDDEVICESEARCH d3dSearch;

	d3dSearch.dwSize = sizeof(D3DFINDDEVICESEARCH);
	d3dSearch.dwFlags = D3DFDS_HARDWARE;
	d3dSearch.bHardware = TRUE;

	D3DFINDDEVICERESULT d3dRes;

	d3dRes.dwSize = sizeof(D3DFINDDEVICERESULT);

	if( FAILED(Direct3D->FindDevice(&d3dSearch,&d3dRes)) ) {
		dprintf("CheckHw: no hardware.\n");
		if( dwMemType ) *dwMemType = DDSCAPS_SYSTEMMEMORY;
		if( guid ) *guid = IID_IDirect3DRGBDevice;
	}
	else {
		dprintf("CheckHw: hw accelerated.\n");
		if( dwMemType ) *dwMemType = DDSCAPS_VIDEOMEMORY;
		if( guid ) *guid = IID_IDirect3DHALDevice;
	}

	return;
}


//
//
//
static HRESULT WINAPI EnumZBufferCallback( DDPIXELFORMAT *pddpf, void *pddpfDesired ) {

	if( pddpf->dwFlags == DDPF_ZBUFFER ) {

		memcpy( pddpfDesired, pddpf, sizeof(DDPIXELFORMAT) );

		// dprintf("zbit: %d\n", pddpf->dwZBufferBitDepth );

		if( pddpf->dwZBufferBitDepth >= 16 )
			return D3DENUMRET_CANCEL;
	}

	return D3DENUMRET_OK;
}



//
//
//
static BOOL CheckCaps( GUID *lpGUID, DDDEVICEIDENTIFIER *DeviceInfo, DDDEVICEIDENTIFIER *DeviceInfoHost ) {

	HRESULT error;
	DDCAPS *ddcaps;
	DDSCAPS2 ddscaps;
	DDSURFACEDESC2 ddsd;

	ddcaps = D3D_GetCaps();

	if( !pDirectDrawCreate || (error = pDirectDrawCreate(lpGUID, &DirectDraw1, NULL)) != DD_OK ) {
		dprintf("CheckCaps: DirectDrawCreate failed (%s).\n",DX_Error(error));
		return FALSE;
	}

	if( (error = DirectDraw1->QueryInterface( IID_IDirectDraw4, (void**)&DirectDraw4) ) != DD_OK ) {
		SAFE_RELEASE(DirectDraw1);
		dprintf("CheckCaps: QueryInterface IID_IDirectDraw4 failed (%s).\n",DX_Error(error));
		return FALSE;
	}

	ddcaps->dwSize = sizeof(DDCAPS);
	ddcaps->dwCaps = 0x0;

	if( (error=DirectDraw4->GetCaps( ddcaps, NULL )) ) {
		SAFE_RELEASE(DirectDraw1);
		dprintf("CheckCaps: GetCaps failed (%s).\n",DX_Error(error));
		return FALSE;
	}

	error = DirectDraw4->GetDeviceIdentifier( DeviceInfo, 0 );
	error = DirectDraw4->GetDeviceIdentifier( DeviceInfoHost, DDGDI_GETHOSTIDENTIFIER );

	// texture memory check

	memset( &ddscaps, 0L, sizeof(DDSCAPS2) );
	ddscaps.dwCaps = DDSCAPS_TEXTURE;

	if( (error = DirectDraw4->GetAvailableVidMem( &ddscaps, &ddcaps->dwVidMemTotal, &ddcaps->dwVidMemFree )) )
		dprintf("CheckCaps: GetAvailableVidMem failed (%s).\n",DX_Error(error));

	if( (error = DirectDraw4->QueryInterface( IID_IDirect3D3, (void**)&Direct3D ) ) != DD_OK ) {
		SAFE_RELEASE(DirectDraw1);
		dprintf("CheckCaps: QueryInterface failed (%s).\n",DX_Error(error));
		return FALSE;
	}

	CheckHw();

	// desc

	D3DDEVICEDESC *mydesc = D3D_GetDesc();
	memset( mydesc, 0L, sizeof(D3DDEVICEDESC) );

	if( dwinFullscreen() == FALSE ) {

		dprintf("log: windowed mode detected.\n");

		if( (error = DirectDraw4->SetCooperativeLevel( hw_state.hwnd, DDSCL_NORMAL )) == DD_OK ) {

			ZeroMemory( &ddsd, sizeof(ddsd) );
			ddsd.dwSize = sizeof( ddsd );
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
						DDSCAPS_3DDEVICE |
						DDSCAPS_VIDEOMEMORY;

			if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK )
				ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;

			if( (error == DD_OK) || ((error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) == DD_OK) ) {

				if( (error=PrimarySurface->GetSurfaceDesc(&ddsd)) != DD_OK )
					dprintf( "CheckCaps: GetSurfaceDesc failed (windowed). Error: %s\n",DX_Error(error));

				dprintf("CheckCaps pixel format: %d bits, %x:%x:%x\n",ddsd.ddpfPixelFormat.dwRGBBitCount,ddsd.ddpfPixelFormat.dwRBitMask,ddsd.ddpfPixelFormat.dwGBitMask,ddsd.ddpfPixelFormat.dwBBitMask);

				DDSURFACEDESC2 ddsd2;
				ZeroMemory( &ddsd2, sizeof(ddsd2) );

				ddsd2.dwSize 		= sizeof( ddsd2 );
				ddsd2.dwFlags        	= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
				ddsd2.ddsCaps.dwCaps 	= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
				ddsd2.dwWidth		= hw_state.SCREENW;
				ddsd2.dwHeight		= hw_state.SCREENH;

				ddsd2.ddpfPixelFormat = ddsd.ddpfPixelFormat;

				if( (error=DirectDraw4->CreateSurface(&ddsd2,&SecondarySurface,NULL)) == DD_OK ) {

					D3DDEVICEDESC hwdesc,swdesc;

					memset( &hwdesc, 0L, sizeof(D3DDEVICEDESC) );
					memset( &swdesc, 0L, sizeof(D3DDEVICEDESC) );

					hwdesc.dwSize = sizeof(D3DDEVICEDESC);
					swdesc.dwSize = sizeof(D3DDEVICEDESC);

					strcpy( d3dName, "HAL" );
					D3D_CreateDevice();

					if( d3dDevice ) {

						if( (error = d3dDevice->GetCaps(&hwdesc, &swdesc)  ) == DD_OK ) {

							if( BOOLEAN(hwdesc.dcmColorModel) == TRUE )
								memmove( mydesc, &hwdesc, sizeof(D3DDEVICEDESC) );
							else {
								memmove( mydesc, &swdesc, sizeof(D3DDEVICEDESC) );
								dprintf("no hw device.\n");
							}
						}
						else
							dprintf("CheckCaps: GetCaps failed.\nError: %s",D3D_Error(error));

						if( d3dDevice->Release() ) dprintf("d3dDevice\n");
						d3dDevice = NULL;
					}
					else
						dprintf("CheckCaps: CreateDevice failed (%s).\n",DX_Error(error));

					SAFE_RELEASE(SecondarySurface); // dprintf("secondary\n");
				}
				else
					dprintf("CheckCaps: can't create secondary surface (%s).\n",DX_Error(error));

				if( PrimarySurface->Release() ) dprintf("primary\n");
				PrimarySurface = NULL;
			}
			else
				dprintf("CheckCaps: CreateSurface failed (%s).\n",DX_Error(error));

			if( (error = DirectDraw4->SetCooperativeLevel( hw_state.hwnd, DDSCL_NORMAL )) )
				dprintf("CheckCaps: SetCooperativeLevel failed (%s).\n",DX_Error(error));
		}
		else
			dprintf("CheckCaps: SetCooperativeLevel failed (%s).\n",DX_Error(error));

	}
	else {

		dprintf("log: fullscreen caps.\n");

		if( (error = DirectDraw4->SetCooperativeLevel( hw_state.hwnd, (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) )) == DD_OK ) {

			ddsd.dwSize = sizeof( ddsd );
			ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
						  DDSCAPS_COMPLEX |
						  DDSCAPS_FLIP |
						  DDSCAPS_3DDEVICE |
						  DDSCAPS_VIDEOMEMORY;
			ddsd.dwBackBufferCount = 1;

			if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK )
				ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;

			if( (error == DD_OK) || ((error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) == DD_OK) ) {

				DDSCAPS2 ddscaps;
				memset( &ddscaps, 0, sizeof(DDSCAPS2) );
				ddscaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_3DDEVICE;

				if( (error=PrimarySurface->GetAttachedSurface( &ddscaps, &SecondarySurface )) == DD_OK ) {

					D3DDEVICEDESC hwdesc,swdesc;

					memset( &hwdesc, 0L, sizeof(D3DDEVICEDESC) );
					memset( &swdesc, 0L, sizeof(D3DDEVICEDESC) );

					hwdesc.dwSize = sizeof(D3DDEVICEDESC);
					swdesc.dwSize = sizeof(D3DDEVICEDESC);

					strcpy( d3dName, "HAL" );
					D3D_CreateDevice();

					if( d3dDevice ) {

						if( (error=d3dDevice->GetCaps(&hwdesc, &swdesc)  ) == DD_OK ) {

							if( BOOLEAN(hwdesc.dcmColorModel) == TRUE )
								memmove( mydesc, &hwdesc, sizeof(D3DDEVICEDESC) );
							else {
								memmove( mydesc, &swdesc, sizeof(D3DDEVICEDESC) );
								dprintf("no hw device.\n");
							}
						}
						else
							dprintf("CheckCaps: GetCaps failed.\nError: %s",D3D_Error(error));

						if( d3dDevice->Release() ) dprintf("d3dDevice\n");
						d3dDevice = NULL;
					}
					else
						dprintf("CheckCaps: CreateDevice failed (%s).\n",DX_Error(error));

					SecondarySurface->Release(); // dprintf("secondary\n");
					SecondarySurface = NULL;
				}
				else
					dprintf("CheckCaps: can't get secondary surface (%s).\n",DX_Error(error));

				if( PrimarySurface->Release() ) dprintf("primary\n");
				PrimarySurface = NULL;
			}
			else
				dprintf("CheckCaps: CreateSurface failed (%s).\n",DX_Error(error));

			if( (error = DirectDraw4->SetCooperativeLevel( hw_state.hwnd, DDSCL_NORMAL )) )
				dprintf("CheckCaps: SetCooperativeLevel failed (%s).\n",DX_Error(error));
		}
		else
			dprintf("CheckCaps: SetCooperativeLevel failed (%s).\n",DX_Error(error));
	}

	SAFE_RELEASE(Direct3D);
	SAFE_RELEASE(DirectDraw4);
	SAFE_RELEASE(DirectDraw1);

	return TRUE;
}






//
//
//
BOOL CALLBACK FindDeviceCallbackEx1( GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam, HMONITOR hm ) {

	DDDEVICEIDENTIFIER DeviceInfo;
	DDDEVICEIDENTIFIER DeviceInfoHost;

	char *szFind = (char *)lParam;

	dprintf("FindDeviceCallbackEx1: checking %s (%s).\n",szName,szDevice);

	char primary_device_name[] = { '\\','\\','.','\\','D','I','S','P','L','A','Y','1',0 };

	// win7-nél ezt is beadja
	if( !stricmp( primary_device_name, szDevice ) ) {
		dprintf("FindDeviceCallbackEx1: %s was caught at %d%s\n", szDevice, combobox.nstrs, combobox.nstrs==0?".":" skipping." );
		// ha már volt akkor nem kell mégegyszer
		if( combobox.nstrs > 0 )
			return DDENUMRET_OK;
	}

	// GetMonitorInfo(hm,...)

	if( CheckCaps( lpGUID, &DeviceInfo, &DeviceInfoHost ) == FALSE ) {
		dprintf("FindDeviceCallbackEx1: CheckCaps failed.\n");
		return DDENUMRET_OK;
	}

	// dprintf("log: %s (%s) (%s).\n",DeviceInfo.szDescription,DeviceInfo.szDriver,szName);
	// dprintf("dih: %s, %s\n",DeviceInfoHost.szDescription,DeviceInfoHost.szDriver);

	sprintf( combobox.str[ combobox.nstrs ][0],"%s (%s)",DeviceInfo.szDescription,DeviceInfo.szDriver); // szName, szDevice );
	sprintf( combobox.str[ combobox.nstrs ][1],"%s", szDevice );

	if( !stricmp( szFind, (char *)szDevice) )
		combobox.cursel = combobox.nstrs;

	++combobox.nstrs;

	dprintf("FindDeviceCallbackEx1: added.\n");

	return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: DDEnumCallback()
// Desc: Old style callback retained for backwards compatibility
//-----------------------------------------------------------------------------
BOOL CALLBACK FindDeviceCallback1( GUID *pGUID, LPSTR pDescription, LPSTR strName, LPVOID pContext ) {

	return ( FindDeviceCallbackEx1( pGUID, pDescription, strName, pContext, NULL ) );
}





//
//
//
BOOL CALLBACK FindDeviceCallbackEx2( GUID* lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam, HMONITOR hm ) {

	char ach[512];
	char *szFind = (char *)lParam;

	wsprintf(ach,"%s (%s)", szDevice, szName );

	if( lstrcmpi(szFind, szDevice) == 0 || lstrcmpi(szFind, ach) == 0 ) {
		if( pDirectDrawCreate && pDirectDrawCreate(lpGUID, &DirectDraw1, NULL) == DD_OK ) {
			if( DirectDraw1 )
				strcpy(d3dDName,szName);
			return DDENUMRET_CANCEL;
		}
	}

	return DDENUMRET_OK;
}



//-----------------------------------------------------------------------------
// Name: DDEnumCallback()
// Desc: Old style callback retained for backwards compatibility
//-----------------------------------------------------------------------------
BOOL CALLBACK FindDeviceCallback2( GUID *pGUID, LPSTR pDescription, LPSTR strName, LPVOID pContext ) {

	return ( FindDeviceCallbackEx2( pGUID, pDescription, strName, pContext, NULL ) );
}



//
//
//
static void D3D_CreateDevice( void ) {

	HRESULT error = !DD_OK;

	SAFE_RELEASE(d3dDevice);

	if( !stricmp(d3dName, "HAL") ) {
		error = Direct3D->CreateDevice( IID_IDirect3DHALDevice, SecondarySurface, &d3dDevice, NULL );
		pDeviceGUID = &IID_IDirect3DHALDevice;
	}

	if( !stricmp(d3dName, "RGB") ) {
		error = Direct3D->CreateDevice( IID_IDirect3DRGBDevice, SecondarySurface, &d3dDevice, NULL );
		pDeviceGUID = &IID_IDirect3DRGBDevice;
	}

	if( !stricmp(d3dName, "REF") ) {
		error = Direct3D->CreateDevice( IID_IDirect3DRefDevice, SecondarySurface, &d3dDevice, NULL );
		pDeviceGUID = &IID_IDirect3DRefDevice;
	}

	if( error != DD_OK ) {
		dprintf( "D3D_CreateDevice: first pass with %s. (error: %s)\n",d3dName, D3D_Error(error));
		strcpy( d3dName, "HAL" );
		error = Direct3D->CreateDevice( IID_IDirect3DHALDevice, SecondarySurface, &d3dDevice, NULL );
		pDeviceGUID = &IID_IDirect3DHALDevice;
	}

	if( error != DD_OK ) {
		strcpy( d3dName, "RGB" );
		error = Direct3D->CreateDevice( IID_IDirect3DRGBDevice, SecondarySurface, &d3dDevice, NULL );
		pDeviceGUID = &IID_IDirect3DRGBDevice;
	}

	if( error != DD_OK ) {
		strcpy( d3dName, "REF" );
		if( (error = Direct3D->CreateDevice( IID_IDirect3DRefDevice, SecondarySurface, &d3dDevice, NULL)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_CreateDevice: CreateDevice failed.\n\nError: %s",D3D_Error(error));
			dllQuit( d3d_error_str );
		}
		pDeviceGUID = &IID_IDirect3DRefDevice;
	}

	char str[512];

	strcpy( str, "D3D_CreateDevice: " );

	if( strlen(d3dDName) > 1 ) {
		strcat( str, d3dDName );
		strcat( str, " (" );
		strcat( str, d3dName );
		strcat( str, ")" );
	}
	else
		strcat( str, d3dName );

	dprintf( "%s\n", str );

	return;
}



//
//
//
HRESULT CALLBACK EnumDisplayModesCallback( LPDDSURFACEDESC desc, LPVOID *bagoy ) {

	// check parmeters
	if( !desc ) return DDENUMRET_OK;

	// setup mode

	dprintf("mode: %dx%d %dbit\n",	desc->dwWidth,
					desc->dwHeight,
					desc->ddpfPixelFormat.dwRGBBitCount );

	return DDENUMRET_OK;
}




//
//
//
static HRESULT SW_ModeList( void ) {

	// enumerate directx modes
	if( DirectDraw1 )
		return DirectDraw1->EnumDisplayModes( 0, NULL, (LPVOID)NULL, (LPDDENUMMODESCALLBACK)EnumDisplayModesCallback );

	return DD_OK;
}





//
//
//
static BOOL GFXDLLCALLCONV D3D_Init( gfxdlldesc_t *desc ) {

	HRESULT error;
	DDCAPS ddcaps;
	DDSURFACEDESC2 ddsd;
	int i;
	BOOL bIsHardware = FALSE;
	int devDepth = 0;
	char szDevice[XMAX_PATH];
	BOOL mustinit, exist;

	hw_state.hwnd = (HWND)desc->hwnd;
	hw_state.hInst = (HINSTANCE)desc->hInst;
	hw_state.SCREENW = desc->width;
	hw_state.SCREENH = desc->height;
	hw_state.bpp = desc->bpp;
	hw_state.texmapid = HM_NOTEX;
	hw_state.mode = HM_COLOR;
	hw_state.cullmode = D3DCULL_CW;
	hw_state.x = desc->x;
	hw_state.y = desc->y;
	hw_state.flag = desc->flag;
	hw_state.zbuffermode = FALSE;
	hw_state.bSceneBegun = FALSE;
	hw_state.bMipmap = FALSE;
	desc->bZBuffer = FALSE;

	dprintf = (void (GFXDLLCALLCONV *)(char *s,...))desc->xprintf;
	dmalloc = (void *(GFXDLLCALLCONV *)(int))desc->malloc;
	drealloc = (void *(GFXDLLCALLCONV *)(void *,int))desc->realloc;
	dfree = (int (GFXDLLCALLCONV *)(void *))desc->free;
	dwinQuit = (BOOL (GFXDLLCALLCONV *)(char *s,...))desc->Quit;
	dGetBackHwnd = (ULONG (GFXDLLCALLCONV *)(void))desc->GetBackHwnd;
	dCheckParm = (int (GFXDLLCALLCONV *)(char *s))desc->CheckParm;
	dTexForNum = (texmap_t *(GFXDLLCALLCONV *)(int texmapid))desc->TexForNum;
	dCopyVector = (void (GFXDLLCALLCONV *)(point3_t dst, point3_t src))desc->CopyVector;
	dDotProduct = (FLOAT (GFXDLLCALLCONV *)(point3_t v1, point3_t v2 ))desc->DotProduct;
	dwinFullscreen = (BOOL (GFXDLLCALLCONV *)(void))desc->winFullscreen;
	D3D_winReadProfileString = (void (GFXDLLCALLCONV *)(char*,char*,char*,int))desc->winReadProfileString;
	D3D_winWriteProfileString = (void (GFXDLLCALLCONV *)(char*,char*))desc->winWriteProfileString;
	D3D_winReadProfileInt = (int (GFXDLLCALLCONV *)(char*,int))desc->winReadProfileInt;
	D3D_winWriteProfileInt = (void (GFXDLLCALLCONV *)(char*,int))desc->winWriteProfileInt;

	dprintf("init: %s v%s (c) Copyright 1998 bernie (%s %s)\n",GFXDLL_NAME,GFXDLL_VERSION,__DATE__,__TIME__);

	// if( hw_state.bpp != 16 )
	//	return FALSE;

	combobox.nstrs = 0;
	combobox.cursel = 0;

	// enumarate

	typedef HRESULT (WINAPI *LPDIRECTDRAWENUMERATE)( LPDDENUMCALLBACKA, LPVOID );

	LPDIRECTDRAWENUMERATEEX pDirectDrawEnumerateEx;
	LPDIRECTDRAWENUMERATE pDirectDrawEnumerate;
	HINSTANCE hDDrawDLL = NULL;

	if( (hDDrawDLL = LoadLibrary("DDRAW.DLL")) == NULL )
		return FALSE;

	if( (pDirectDrawCreate = (LPDIRECTDRAWCREATE)GetProcAddress(hDDrawDLL, "DirectDrawCreate")) == NULL )
		return FALSE;

	if( (pDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEX)GetProcAddress( hDDrawDLL, "DirectDrawEnumerateExA" )) != NULL ) {
		dprintf("log: calling DirectDrawEnumerateExA.\n");
		pDirectDrawEnumerateEx( FindDeviceCallbackEx1, (LPVOID)szDevice,
								DDENUM_ATTACHEDSECONDARYDEVICES |
								DDENUM_DETACHEDSECONDARYDEVICES |
								DDENUM_NONDISPLAYDEVICES );
	}
	else
	if( (pDirectDrawEnumerate = (LPDIRECTDRAWENUMERATE)GetProcAddress( hDDrawDLL, "DirectDrawEnumerateA" )) != NULL ) {
		pDirectDrawEnumerate( FindDeviceCallback1, (LPVOID)szDevice );
		dprintf("log: old enum.\n");
	}
	else
		return FALSE;

	// DirectDrawEnumerateEx( FindDeviceCallbackEx1, (LPVOID)szDevice );

	if( combobox.nstrs == 0 ) {
		// dllQuit("For Direct3D support you must have a card with the following capabilities:\n\n\t- 3D hardware acceleration\n\t- texture mapping\n\t- at least 2Mb texture memory\n\t- alpha blending");
		sprintf( d3d_error_str, "DirectX v6.0 or better required for Direct3D support.\n\nYou can download the latest DirectX drivers from www.microsoft.com/directx page.\n\nIf the problem persists, choose another driver. Thank you!");
		return FALSE;
	}

	D3D_winReadProfileString( "d3d_device", "fIRESTARTER", szDevice, XMAX_PATH );

	if( D3D_winReadProfileInt( "gfx_setup", 0 ) > 0 )
		mustinit = FALSE;
	else
		mustinit = TRUE;

	if( dCheckParm("setup") > 0 )
		mustinit = TRUE;

	combobox.cursel = combobox.nstrs - 1;

	if( !stricmp( szDevice, "fIRESTARTER" ) )
		mustinit = TRUE;

	exist = FALSE;

	for( i=0; i<combobox.nstrs; i++ )
		if( !stricmp( combobox.str[ i ][1], szDevice ) ) {
			combobox.cursel = i;
			exist = TRUE;
			break;
		}

	if( (exist == FALSE) || (mustinit == TRUE) )
		if( D3D_winComboBox( &combobox ) == -1 ) {
			sprintf( d3d_error_str, "D3D_Init: winComboBox returned -1.\n");
			dprintf( d3d_error_str );
			dllQuit(NULL);
		}

	strcpy( szDevice, combobox.str[ combobox.cursel ][1] );

	dprintf("using: %s (%s)\n",combobox.str[ combobox.cursel ][0], szDevice );

	D3D_winWriteProfileString( "d3d_device", szDevice );

	SAFE_RELEASE(DirectDraw1);

	// enumerate

	if( pDirectDrawEnumerateEx ) {
		dprintf("log: calling2 DirectDrawEnumerateExA.\n");
		pDirectDrawEnumerateEx( FindDeviceCallbackEx2, (LPVOID)szDevice,
								DDENUM_ATTACHEDSECONDARYDEVICES|
								DDENUM_DETACHEDSECONDARYDEVICES|
								DDENUM_NONDISPLAYDEVICES );
	}
	else {
		pDirectDrawEnumerate( FindDeviceCallback2, (LPVOID)szDevice );
		dprintf("log: old enum2.\n");
	}

	// DirectDrawEnumerate( FindDeviceCallback2, (LPVOID)szDevice );

	if( DirectDraw1 == NULL ) {
		sprintf( d3d_error_str, "D3D_Init: enumerate unsuccesfull.");
		dprintf( d3d_error_str );
		if( !pDirectDrawCreate || pDirectDrawCreate( NULL, &DirectDraw1, NULL ) != DD_OK )
			return FALSE;
	}
	else
		dprintf( "D3D_Init: created DirectDraw1 from %s.\n", d3dDName );

	// SW_ModeList();

	if( (error = DirectDraw1->QueryInterface( IID_IDirectDraw4, (void**)&DirectDraw4) ) != DD_OK ) {
		sprintf( d3d_error_str, "D3D_Init: QueryInterface failed.\n\nError: %s",DX_Error(error));
		return FALSE;
	}

	ddsd.dwSize = sizeof(DDSURFACEDESC2);

	ddcaps.dwSize = sizeof(DDCAPS);

	if( (error=DirectDraw4->GetCaps( &ddcaps, NULL )) ) {
		sprintf( d3d_error_str, "D3D_Init: GetCaps failed.\n\nError: %s",DX_Error(error));
		return FALSE;
	}

	if( dwinFullscreen() ) {

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, 0,0, hw_state.SCREENW, hw_state.SCREENH, TRUE );

		if( (error=DirectDraw4->SetCooperativeLevel(hw_state.hwnd,DDSCL_ALLOWREBOOT|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: SetCooperativeLevel failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		if( hw_state.bpp > 0 )
			if( (error=DirectDraw4->SetDisplayMode(hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,0,0)) != DD_OK ) {
				sprintf( d3d_error_str, "D3D_Init: SetDisplayMode failed.\n\nError: %s",DX_Error(error));
				return FALSE;
			}
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

		// dprintf( "dxscreen: %d,%d, %d, %d\n",rc.left ,rc.top,rc.right,rc.bottom );

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE );

		// dprintf( "movewindow: %d,%d, %d,%d [%d,%d]\n", rc.left, rc.top, rc.right, rc.bottom, rc.right-rc.left, rc.bottom-rc.top );

		if( (error=DirectDraw4->SetCooperativeLevel(hw_state.hwnd,DDSCL_NORMAL)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: SetCooperativeLevel failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}
	}

	// if( hw_state.bpp < 0 )
		//hw_state.bpp = -hw_state.bpp;

	hw_state.bpp = ABS( hw_state.bpp );

	UPDATEBLTRECT;

	if( (error = DirectDraw4->QueryInterface( IID_IDirect3D3, (void**)&Direct3D) ) != DD_OK ) {
		sprintf( d3d_error_str, "D3D_Init: QueryInterface failed.\n\nError: %s",DX_Error(error));
		return FALSE;
	}

	CheckHw();

	if( dwinFullscreen() ) {

		bInGraphic = TRUE;

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
					DDSCAPS_FLIP |
					DDSCAPS_COMPLEX |
					DDSCAPS_3DDEVICE |
					DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 1;

		if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
				sprintf( d3d_error_str, "D3D_Init: CreateSurface primary failed.\n\nError: %s",DX_Error(error));
				return FALSE;
			}
		}

		DDSCAPS2 ddscaps;
		ZeroMemory( &ddscaps, sizeof(DDSCAPS2) );
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_3DDEVICE;

		ddsd.dwSize = sizeof( ddsd );

		if( (error=PrimarySurface->GetAttachedSurface( &ddscaps, &SecondarySurface )) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: can't get secondary surface.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_PIXELFORMAT;

		if( (error=SecondarySurface->GetSurfaceDesc(&ddsd)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: GetSurfaceDesc failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		/***/
		i = desc->rshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwRBitMask) ) ++desc->rshift;
		i = desc->gshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwGBitMask) ) ++desc->gshift;
		i = desc->bshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwBBitMask) ) ++desc->bshift;

		desc->rmask = ddsd.ddpfPixelFormat.dwRBitMask;
		while( !(desc->rmask & 1) ) desc->rmask >>= 1;

		i = desc->rsize = 0;
		while( (1<<(i++)) & desc->rmask ) ++desc->rsize;

		desc->gmask = ddsd.ddpfPixelFormat.dwGBitMask;
		while( !(desc->gmask & 1) ) desc->gmask >>= 1;

		i = desc->gsize = 0;
		while( (1<<(i++)) & desc->gmask ) ++desc->gsize;

		desc->bmask = ddsd.ddpfPixelFormat.dwBBitMask;
		while( !(desc->bmask & 1) ) desc->bmask >>= 1;

		i = desc->bsize = 0;
		while( (1<<(i++)) & desc->bmask ) ++desc->bsize;

		drmask=desc->rmask; dgmask=desc->gmask; dbmask=desc->bmask;
		drsize=desc->rsize; dgsize=desc->gsize; dbsize=desc->bsize;
		drshift=desc->rshift; dgshift=desc->gshift; dbshift=desc->bshift;

		hw_state.pitch = ddsd.lPitch;
		hw_state.real_bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
	}
	else {
		// dwinFullscreen() == FALSE

		bInGraphic = FALSE;

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
					DDSCAPS_3DDEVICE |
					DDSCAPS_VIDEOMEMORY;

		if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
				sprintf( d3d_error_str, "D3D_Init: CreateSurface primary failed (windowed).\n\nError: %s",DX_Error(error));
				return FALSE;
			}
		}

		if( (error=PrimarySurface->GetSurfaceDesc(&ddsd)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: GetSurfaceDesc failed (windowed).\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		DDSURFACEDESC2 ddsd2;
		ZeroMemory( &ddsd2, sizeof(ddsd2) );

		ddsd2.dwSize 		= sizeof( ddsd2 );
		ddsd2.dwFlags        	= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		ddsd2.ddsCaps.dwCaps 	= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		ddsd2.dwWidth		= hw_state.SCREENW;
		ddsd2.dwHeight		= hw_state.SCREENH;

		ddsd2.ddpfPixelFormat = ddsd.ddpfPixelFormat;

		/*
		memset(&DDPixelFormat, 0, sizeof(DDPIXELFORMAT));
		DDPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
		hRes=lpDDPrimarySurface->GetPixelFormat(&DDPixelFormat);
		*/

		// ZeroMemory( &ddsd2.ddpfPixelFormat, sizeof(DDPIXELFORMAT) );
		// ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		// ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB;
		// ddsd2.ddpfPixelFormat.dwRGBBitCount = 16;

		/*
		desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC | DDPF_RGB;
		desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		desc.ddpfPixelFormat.dwFourCC = BI_RGB;
		desc.ddpfPixelFormat.dwRGBBitCount = 32;
		desc.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
		desc.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
		desc.ddpfPixelFormat.dwBBitMask = 0x000000ff;
		desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000;
		*/

		if( (error=DirectDraw4->CreateSurface(&ddsd2,&SecondarySurface,NULL)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: CreateSurface secondary failed (windowed).\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		/***
		if( ddsd.ddpfPixelFormat.dwRGBBitCount != 16 )
			dllQuit("D3D windowed mode requires a 16 bits desktop!\nInstead you should use fullscreen mode.\nThank you.");
		***/

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_PIXELFORMAT;

		if( (error=SecondarySurface->GetSurfaceDesc(&ddsd)) != DD_OK ) {
			sprintf( d3d_error_str, "D3D_Init: GetSurfaceDesc failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		hw_state.pitch = ddsd.lPitch;
		hw_state.real_bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
	}

	// ha default akkor hacked
	if( desc->bpp < 0 || (dwinFullscreen() == FALSE) ) {

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
	}

	hw_state.orig_SCREENW = hw_state.SCREENW;
	hw_state.orig_SCREENH = hw_state.SCREENH;
	hw_state.orig_bpp = hw_state.bpp;

	dprintf("log: %dx%d %d (%d) bits, pitch: %d, %d:%d:%d (%d:%d:%d)\n",
			hw_state.SCREENW,
			hw_state.SCREENH,
			hw_state.bpp, hw_state.real_bpp, hw_state.pitch,
			drsize,dgsize,dbsize,
			drshift,dgshift,dbshift );

	dprintf("pixel format: %d bits, %x:%x:%x\n",
			ddsd.ddpfPixelFormat.dwRGBBitCount,
			ddsd.ddpfPixelFormat.dwRBitMask,
			ddsd.ddpfPixelFormat.dwGBitMask,
			ddsd.ddpfPixelFormat.dwBBitMask);
	// Init3D

	hw_state.fogenable = FALSE;
	hw_state.bIsColorKey = FALSE;
	hw_state.bIsGouraud = FALSE;

	hw_state.bIsColorKey = BOOLEAN( global_ddcaps->dwCKeyCaps & DDCKEYCAPS_SRCBLT );

	if( global_mydesc->dwFlags & D3DDD_TRICAPS ) {

		if( global_mydesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE )
			hw_state.fogenable = TRUE;

		if( global_mydesc->dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )
			hw_state.bIsGouraud = TRUE;
	}

	strcpy( d3dName, "HAL" );
	D3D_CreateDevice();

	// FIXME
	// pDeviceGUID = &IID_IDirect3DHALDevice;

	// Enumerate the various z-buffer formats, finding a DDPIXELFORMAT
	// to use to create the z-buffer surface.

	DDPIXELFORMAT ddpfZBuffer;
	DDSURFACEDESC2 zddsd;

	memset( &zddsd, 0 ,sizeof(DDSURFACEDESC2) );
	zddsd.dwSize = sizeof(DDSURFACEDESC2);

	ZeroMemory( &ddpfZBuffer, sizeof(DDPIXELFORMAT) );

	if( (error = Direct3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferCallback, (void *)&ddpfZBuffer )) == DD_OK ) {

		// If we found a good zbuffer format, then the dwSize field will be
		// properly set during enumeration. Else, we have a problem and will exit.

		if( sizeof(DDPIXELFORMAT) == ddpfZBuffer.dwSize ) {

			// Get z-buffer dimensions from the render target
			// Setup the surface desc for the z-buffer.

			zddsd.dwFlags	     = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
			zddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
			zddsd.dwWidth	     = hw_state.SCREENW;
			zddsd.dwHeight	     = hw_state.SCREENH;

			memcpy( &zddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT) );

			// For hardware devices, the z-buffer should be in video memory. For
			// software devices, create the z-buffer in system memory

			if( IsEqualIID( *pDeviceGUID, IID_IDirect3DHALDevice ) )
				zddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
			else
				zddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

			if( (error = DirectDraw4->CreateSurface( &zddsd, &lpZBuffer, NULL) ) == DD_OK ) {

				if( (error = SecondarySurface->AddAttachedSurface( lpZBuffer ) ) == DD_OK ) {

					dprintf("init: %d bit Z-buffer.\n", ddpfZBuffer.dwZBufferBitDepth );

					desc->bZBuffer = TRUE;
				}
				else
					dprintf( "D3D_Init: AddAttachedSurface for Z-buffer failed. Error: %s\n", D3D_Error(error) );
			}
			else
				dprintf( "D3D_Init: CreateSurface for Z-buffer failed. Error: %s\n", D3D_Error(error) );
		}
		else
			dprintf( "D3D_Init: no Z-buffer!\n");
	}
	else
		dprintf("D3D_Init: EnumZBufferFormats phailed.\n" );

	if( d3dDevice->Release() ) dprintf("d3dDevice\n");
	d3dDevice = NULL;

	if( dwinFullscreen() == FALSE ) {

		SAFE_RELEASE(lpDDClipper);

		// Create a clipper and attach it to the primary surface.
		if( FAILED( DirectDraw4->CreateClipper( 0, &lpDDClipper, NULL ) ) ) {
			sprintf( d3d_error_str, "D3D_Init: couldn't create the clipper." );
			return FALSE;
		}

		// Associate our clipper with our hwnd so it will be updated by Windows.
		if ( FAILED( lpDDClipper->SetHWnd( 0, hw_state.hwnd ) ) ) {
			sprintf( d3d_error_str, "D3D_Init: couldn't set the hwnd." );
			return FALSE;
		}

		// Associate our clipper with the primary surface, so Blt will use it.
		if( FAILED( PrimarySurface->SetClipper( lpDDClipper ) ) ) {
			sprintf( d3d_error_str, "D3D_Init: couldn't set the clipper." );
			return FALSE;
		}
	}

	strcpy( d3dName, "HAL" );
	D3D_CreateDevice();

	if( (error=Direct3D->CreateViewport( &d3dViewport, NULL )) != DD_OK ) {
		sprintf( d3d_error_str, "D3D_Init: CreateViewport failed.\n\nError: %s",D3D_Error(error));
		return FALSE;
	}

	if( (error=d3dDevice->AddViewport(d3dViewport)) != DD_OK ) {
		sprintf( d3d_error_str, "D3D_Init: AddViewport failed.\n\nError: %s",D3D_Error(error));
		return FALSE;
	}

	D3D_SetupCulling( NULL, NULL, NULL, 0.0f, 1.0f );

	if( desc->bZBuffer ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		hw_state.zbuffermode = TRUE;
	}

	if( hw_state.bIsGouraud )
		d3dDevice->SetRenderState( D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD );
	else
		d3dDevice->SetRenderState( D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT );

	d3dDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, hw_state.cullmode );
	d3dDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TextureDither );
	d3dDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TexturePerspective );
	// d3dDevice->SetRenderState( D3DRENDERSTATE_WRAP0, D3DWRAP_U | D3DWRAP_V );

	// d3dDevice->SetLightState( D3DLIGHTSTATE_COLORVERTEX, TRUE );

	d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, magfilter );
	d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, minfilter );
	// d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	nd3dtextures = 0;
	alloced_texmap = 0;
	d3dtexture = NULL;

	// ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE); // SW_SHOWMAXIMIZED );
	// MoveWindow( (HWND)hw_state.hwnd, 0,0, hw_state.SCREENW, hw_state.SCREENH, TRUE );

	D3DDEVICEDESC ddHwDesc, ddSwDesc;
	ddHwDesc.dwSize = sizeof(D3DDEVICEDESC);
	ddSwDesc.dwSize = sizeof(D3DDEVICEDESC);

	if( d3dDevice->GetCaps( &ddHwDesc, &ddSwDesc ) == DD_OK ) {

		D3DDEVICEDESC mydesc;

		if( BOOLEAN(ddHwDesc.dcmColorModel) == TRUE )
			memmove( &mydesc, &ddHwDesc, sizeof(D3DDEVICEDESC) );
		else
			memmove( &mydesc, &ddSwDesc, sizeof(D3DDEVICEDESC) );

		hw_state.mintexw = mydesc.dwMinTextureWidth;
		hw_state.mintexh = mydesc.dwMinTextureHeight;
		hw_state.maxtexw = mydesc.dwMaxTextureWidth == 0 ? 256 : mydesc.dwMaxTextureWidth;
		hw_state.maxtexh = mydesc.dwMaxTextureHeight == 0 ? 256 : mydesc.dwMaxTextureHeight;
	}
	else {
		dprintf("log: guessing texture size.\n" );

		hw_state.mintexw = 64;
		hw_state.mintexh = 64;
		hw_state.maxtexw = 256;
		hw_state.maxtexh = 256;
	}

	dprintf("texture size min: %d x %d, max: %d x %d\n", hw_state.mintexw, hw_state.mintexh, hw_state.maxtexw, hw_state.maxtexh );

	dprintf("init: Direct3D inited (compiled with DirectX v%d.%d).\n",HIBYTE(DIRECTDRAW_VERSION),LOBYTE(DIRECTDRAW_VERSION) );

	return TRUE;
}





//
//
//
static void GFXDLLCALLCONV D3D_Deinit( void ) {

	HRESULT error;

	D3D_DiscardAllTexture();

	if( lpDDClipper ) {
		if( lpDDClipper->Release() && dprintf) dprintf("lpDDClipper\n");
		lpDDClipper = NULL;
	}

	if( SecondarySurface && SurfaceOn )
	if( (error=SecondarySurface->Unlock(NULL)) && dprintf )
		dprintf("D3D_Deinit: Unlock failed (%s).\n",DX_Error(error));

	if( d3dViewport ) {
		if( (error=d3dDevice->DeleteViewport(d3dViewport)) != DD_OK && dprintf )
			dprintf("D3D_Deinit: DeleteViewport failed (%s).\n",D3D_Error(error));
		if( d3dViewport->Release() && dprintf ) dprintf("viewport\n");
		d3dViewport = NULL;
	}

	if( lpZBuffer ) {
		if( lpZBuffer->Release() && dprintf) dprintf("lpZBuffer\n");
		lpZBuffer = NULL;
	}

	if( d3dDevice ) {
		if( d3dDevice->Release() && dprintf) dprintf("d3dDevice\n");
		d3dDevice = NULL;
	}

	if( SecondarySurface ) {
		if( SecondarySurface->Release() && dprintf) dprintf("secondary\n");
		SecondarySurface = NULL;
	}

	if( PrimarySurface ) {
		if( PrimarySurface->Release() && dprintf) dprintf("primary\n");
		PrimarySurface = NULL;
	}

	if( DirectDrawPalette ) {
		if( DirectDrawPalette->Release() && dprintf) dprintf("palette\n");
		DirectDrawPalette = NULL;
	}

	if( Direct3D ) {
		if( Direct3D->Release() && dprintf) dprintf("Direct3D\n");
		Direct3D = NULL;
	}

	if( DirectDraw4 && dwinFullscreen() )
	if( (error=DirectDraw4->RestoreDisplayMode()) && dprintf )
		dprintf("D3D_Deinit: RestoreDisplayMode failed (%s).\n",DX_Error(error));

	bInGraphic = FALSE;

	if( DirectDraw4 ) {
		if( DirectDraw4->Release() && dprintf ) dprintf("draw4\n");
		DirectDraw4 = NULL;
	}

	if( DirectDraw1 ) {
		if( DirectDraw1->Release() && dprintf ) dprintf("draw1\n");
		DirectDraw1 = NULL;
	}

	if( dprintf ) dprintf("deinit: %s success.\n",GFXDLL_NAME);

	return;
}



//
// toggle switch fullscreen
//
static BOOL GFXDLLCALLCONV D3D_Fullscreen( int flag ) {

	HRESULT error;
	DDSURFACEDESC2 ddsd;
	DWORD dwWndStyle = GetWindowStyle((HWND)hw_state.hwnd);
	int i;

	for( int num=0; num<nd3dtextures; num++ ) {
		// release texture
		SAFE_RELEASE( d3dtexture[num].ptexTexture );
		SAFE_RELEASE( d3dtexture[num].pddsSurface );
		SAFE_RELEASE( d3dtexture[num].lpMat );
	}

	if( (error=d3dDevice->DeleteViewport(d3dViewport)) != DD_OK )
		dprintf("D3D_Fullscreen: DeleteViewport failed (%s).\n",D3D_Error(error));

	SAFE_RELEASE(d3dViewport);
	SAFE_RELEASE(d3dDevice);
	SAFE_RELEASE(lpZBuffer);
	SAFE_RELEASE(SecondarySurface);
	SAFE_RELEASE(PrimarySurface);
	SAFE_RELEASE(lpDDClipper);

	if( flag == TRUE ) {

		// determine the new window style
		dwWndStyle = WS_POPUP | WS_VISIBLE;

		// change the window style
		SetWindowLong((HWND)hw_state.hwnd, GWL_STYLE, dwWndStyle);

		if( (error=DirectDraw4->SetCooperativeLevel(hw_state.hwnd,DDSCL_ALLOWREBOOT|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_FPUSETUP)) != DD_OK ) {
			dprintf( "D3D_Fullscreen: SetCooperativeLevel failed.\nError: %s\n",DX_Error(error));
			return FALSE;
		}

		if( hw_state.bpp > 0 )
			if( (error=DirectDraw4->SetDisplayMode(hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,0,0)) != DD_OK ) {
				dprintf( "D3D_Fullscreen: SetDisplayMode(%d, %d, %d) the BPP phail.\nError: %s\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,DX_Error(error));
				hw_state.bpp = 32;
				if( (error=DirectDraw4->SetDisplayMode(hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,0,0)) != DD_OK ) {
					dprintf( "D3D_Fullscreen: SetDisplayMode(%d, %d, %d) the first phail.\nError: %s\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,DX_Error(error));
					// bokica speckó
					hw_state.SCREENW = 640;
					hw_state.SCREENH = 480;
					hw_state.bpp = 16;
					if( (error=DirectDraw4->SetDisplayMode(hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,0,0)) != DD_OK ) {
						dprintf( "D3D_Fullscreen: SetDisplayMode(%d, %d, %d) the second fail.\nError: %s\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,DX_Error(error));
						// ilyen felbontás mindig van
						hw_state.SCREENW = 800;
						hw_state.SCREENH = 600;
						hw_state.bpp = 32;
						if( (error=DirectDraw4->SetDisplayMode(hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,0,0)) != DD_OK ) {
							dprintf( "D3D_Fullscreen: SetDisplayMode(%d, %d, %d) final fail.\nError: %s\n",hw_state.SCREENW,hw_state.SCREENH,hw_state.bpp,DX_Error(error));
							return FALSE;
						}
					}
				}
			}

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, 0,0, hw_state.SCREENW, hw_state.SCREENH, TRUE );
		Sleep(1);

		UPDATEBLTRECT;

		if( (error = DirectDraw4->QueryInterface( IID_IDirect3D3, (void**)&Direct3D) ) != DD_OK ) {
			dprintf( "D3D_Fullscreen: QueryInterface failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		bInGraphic = TRUE;

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
					DDSCAPS_FLIP |
					DDSCAPS_COMPLEX |
					DDSCAPS_3DDEVICE |
					DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 1;

		if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
				dprintf( "D3D_Fullscreen: CreateSurface primary failed.\n\nError: %s",DX_Error(error));
				return FALSE;
			}
		}

		DDSCAPS2 ddscaps;
		ZeroMemory( &ddscaps, sizeof(DDSCAPS2) );
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_3DDEVICE;

		ddsd.dwSize = sizeof( ddsd );

		if( (error=PrimarySurface->GetAttachedSurface( &ddscaps, &SecondarySurface )) != DD_OK ) {
			dprintf( "D3D_Fullscreen: can't get secondary surface.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_PIXELFORMAT;

		if( (error=SecondarySurface->GetSurfaceDesc(&ddsd)) != DD_OK ) {
			dprintf( "D3D_Fullscreen: GetSurfaceDesc failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		i = drshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwRBitMask) ) ++drshift;
		i = dgshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwGBitMask) ) ++dgshift;
		i = dbshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwBBitMask) ) ++dbshift;

		drmask = ddsd.ddpfPixelFormat.dwRBitMask;
		while( !(drmask & 1) ) drmask >>= 1;

		i = drsize = 0;
		while( (1<<(i++)) & drmask ) ++drsize;

		dgmask = ddsd.ddpfPixelFormat.dwGBitMask;
		while( !(dgmask & 1) ) dgmask >>= 1;

		i = dgsize = 0;
		while( (1<<(i++)) & dgmask ) ++dgsize;

		dbmask = ddsd.ddpfPixelFormat.dwBBitMask;
		while( !(dbmask & 1) ) dbmask >>= 1;

		i = dbsize = 0;
		while( (1<<(i++)) & dbmask ) ++dbsize;

		hw_state.pitch = ddsd.lPitch;
		hw_state.real_bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
	}
	else {
		hw_state.SCREENW = hw_state.orig_SCREENW;
		hw_state.SCREENH = hw_state.orig_SCREENH;
		hw_state.bpp = hw_state.orig_bpp;

		if( (error=DirectDraw4->RestoreDisplayMode())!= DD_OK ) {
			dprintf( "D3D_Fullscreen: RestoreDisplayMode failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

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

		// dprintf( "movewindow ORIG: %d,%d, %d,%d\n", rc.left, rc.top, rc.right, rc.bottom );

		if( hw_state.flag != SETX_NOBORDER )
			AdjustWindowRect( &rc, dwStyle, FALSE );

		// dprintf( "dxscreen: %d,%d, %d, %d\n",rc.left ,rc.top,rc.right,rc.bottom );

		ShowWindow( (HWND)hw_state.hwnd, SW_RESTORE);
		MoveWindow( (HWND)hw_state.hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE );
		Sleep(1);

		// dprintf( "movewindow: %d,%d, %d,%d [%d,%d]\n", rc.left, rc.top, rc.right, rc.bottom, rc.right-rc.left, rc.bottom-rc.top );

		UPDATEBLTRECT;

		Sleep(1);

		if( (error=DirectDraw4->SetCooperativeLevel(hw_state.hwnd,DDSCL_NORMAL)) != DD_OK ) {
			dprintf( "D3D_Fullscreen: SetCooperativeLevel failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		if( (error = DirectDraw4->QueryInterface( IID_IDirect3D3, (void**)&Direct3D) ) != DD_OK ) {
			dprintf( "D3D_Fullscreen: QueryInterface failed.\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		bInGraphic = FALSE;

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS; // | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
					DDSCAPS_3DDEVICE |
					DDSCAPS_VIDEOMEMORY;

		ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;

		if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			if( (error=DirectDraw4->CreateSurface(&ddsd,&PrimarySurface,NULL)) != DD_OK ) {
				dprintf( "D3D_Fullscreen: CreateSurface primary failed (fullscreen).\n\nError: %s",DX_Error(error));
				return FALSE;
			}
		}

		DDSCAPS2 ddscaps;
		ZeroMemory( &ddscaps, sizeof(DDSCAPS2) );
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_3DDEVICE;

		ddsd.dwSize 		= sizeof( ddsd );
		ddsd.dwFlags        	= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.ddsCaps.dwCaps 	= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		ddsd.dwWidth		= hw_state.SCREENW;
		ddsd.dwHeight		= hw_state.SCREENH;

		if( (error=DirectDraw4->CreateSurface(&ddsd,&SecondarySurface,NULL)) != DD_OK ) {
			dprintf( "D3D_Fullscreen: CreateSurface secondary failed (fullscreen).\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_PIXELFORMAT;

		if( (error=SecondarySurface->GetSurfaceDesc(&ddsd)) != DD_OK ) {
			dprintf( "D3D_Fullscreen: GetSurfaceDesc failed (fullscreen).\n\nError: %s",DX_Error(error));
			return FALSE;
		}

		i = drshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwRBitMask) ) ++drshift;
		i = dgshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwGBitMask) ) ++dgshift;
		i = dbshift = 0;
		while( !((1<<(i++)) & ddsd.ddpfPixelFormat.dwBBitMask) ) ++dbshift;

		drmask = ddsd.ddpfPixelFormat.dwRBitMask;
		while( !(drmask & 1) ) drmask >>= 1;

		i = drsize = 0;
		while( (1<<(i++)) & drmask ) ++drsize;

		dgmask = ddsd.ddpfPixelFormat.dwGBitMask;
		while( !(dgmask & 1) ) dgmask >>= 1;

		i = dgsize = 0;
		while( (1<<(i++)) & dgmask ) ++dgsize;

		dbmask = ddsd.ddpfPixelFormat.dwBBitMask;
		while( !(dbmask & 1) ) dbmask >>= 1;

		i = dbsize = 0;
		while( (1<<(i++)) & dbmask ) ++dbsize;

		hw_state.pitch = ddsd.lPitch;
		hw_state.real_bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;

		// Create a clipper and attach it to the primary surface.
		if( FAILED( DirectDraw4->CreateClipper( 0, &lpDDClipper, NULL ) ) ) {
			sprintf( d3d_error_str, "D3D_Init: couldn't create the clipper." );
			return FALSE;
		}

		// Associate our clipper with our hwnd so it will be updated by Windows.
		if ( FAILED( lpDDClipper->SetHWnd( 0, hw_state.hwnd ) ) ) {
			sprintf( d3d_error_str, "D3D_Init: couldn't set the hwnd." );
			return FALSE;
		}

		// Associate our clipper with the primary surface, so Blt will use it.
		if( FAILED( PrimarySurface->SetClipper( lpDDClipper ) ) ) {
			sprintf( d3d_error_str, "D3D_Init: couldn't set the clipper." );
			return FALSE;
		}
	}

	strcpy( d3dName, "HAL" );
	D3D_CreateDevice();
	// FIXME
	// pDeviceGUID = &IID_IDirect3DHALDevice;

	DDPIXELFORMAT ddpfZBuffer;
	DDSURFACEDESC2 zddsd;

	memset( &zddsd, 0 ,sizeof(DDSURFACEDESC2) );
	zddsd.dwSize = sizeof(DDSURFACEDESC2);

	if( (error = Direct3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferCallback, (VOID*)&ddpfZBuffer )) == DD_OK ) {

		// If we found a good zbuffer format, then the dwSize field will be
		// properly set during enumeration. Else, we have a problem and will exit.

		if( sizeof(DDPIXELFORMAT) != ddpfZBuffer.dwSize ) {
			dprintf( "D3D_Fullscreen: no zbuffer!");
			return FALSE;
		}

		// Get z-buffer dimensions from the render target
		// Setup the surface desc for the z-buffer.

		zddsd.dwFlags	     = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		zddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
		zddsd.dwWidth	     = hw_state.SCREENW;
		zddsd.dwHeight	     = hw_state.SCREENH;

		memcpy( &zddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT) );

		// For hardware devices, the z-buffer should be in video memory. For
		// software devices, create the z-buffer in system memory

		if( IsEqualIID( *pDeviceGUID, IID_IDirect3DHALDevice ) )
			zddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			zddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

		if( (error = DirectDraw4->CreateSurface( &zddsd, &lpZBuffer, NULL) ) != DD_OK ) {
			dprintf( "D3D_Fullscreen: CreateSurface for Z-buffer failed.\n\nError: %s",D3D_Error(error) );
			return FALSE;
		}

		if( (error = SecondarySurface->AddAttachedSurface( lpZBuffer ) ) != DD_OK ) {
			dprintf( "D3D_Fullscreen: AddAttachedSurface for Z-buffer failed.\n\nError: %s",D3D_Error(error) );
			return FALSE;
		}
	}
	else
		dprintf("D3D_Fullscreen: EnumZBufferFormats phailed.\n" );

	if( (error = Direct3D->CreateDevice( *pDeviceGUID, SecondarySurface, &d3dDevice, NULL ))!= DD_OK ) {
		dprintf( "D3D_Fullscreen: CreateDevice failed.\n\nError: %s",D3D_Error(error));
		return FALSE;
	}

	if( (error=Direct3D->CreateViewport( &d3dViewport, NULL )) != DD_OK ) {
		dprintf( "D3D_Fullscreen: CreateViewport failed.\n\nError: %s",D3D_Error(error));
		return FALSE;
	}

	if( (error=d3dDevice->AddViewport(d3dViewport)) != DD_OK ) {
		dprintf( "D3D_Fullscreen: AddViewport failed.\nError: %s",D3D_Error(error));
		return FALSE;
	}

	if( hw_state.zbuffermode == TRUE ) {
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL );
		d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE );
	}

	if( hw_state.bIsGouraud )
		d3dDevice->SetRenderState( D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD );
	else
		d3dDevice->SetRenderState( D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT );

	d3dDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, hw_state.cullmode );
	d3dDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TextureDither );
	d3dDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TexturePerspective );

	d3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, magfilter );
	d3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, minfilter );

	for( int num=0; num<nd3dtextures; num++ ) {

		SAFE_RELEASE(d3dtexture[num].lpMat);

		if( (error = Direct3D->CreateMaterial(&(d3dtexture[num].lpMat), NULL)) != D3D_OK )
			dprintf( "D3D_Fullscreen: CreateMaterial failed.\nError: %s",D3D_Error(error) );

		D3DMATERIAL mat;

		memset(&mat, 0, sizeof(D3DMATERIAL));
		mat.dwSize = sizeof(D3DMATERIAL);
		mat.diffuse.a = (D3DVALUE)0.0;
		mat.diffuse.r = (D3DVALUE)0.0;
		mat.diffuse.g = (D3DVALUE)0.0;
		mat.diffuse.b = (D3DVALUE)0.0;
		mat.ambient.a = (D3DVALUE)0.0;
		mat.ambient.r = (D3DVALUE)0.0;
		mat.ambient.g = (D3DVALUE)0.0;
		mat.ambient.b = (D3DVALUE)0.0;
		mat.specular.a = (D3DVALUE)0.0;
		mat.specular.r = (D3DVALUE)0.0;
		mat.specular.g = (D3DVALUE)0.0;
		mat.specular.b = (D3DVALUE)0.0;
		mat.power = (FLOAT)0.0;
		mat.dwRampSize = 16L;

		error = d3dtexture[num].lpMat->SetMaterial( &mat );
		if( error != D3D_OK ) {
			sprintf( d3d_error_str, "SetMaterial failed.\n\nError: %s",D3D_Error(error) );
			dllQuit( d3d_error_str );
		}
		error = d3dtexture[num].lpMat->GetHandle( d3dDevice, &(d3dtexture[num].hMat) );
		if( error != D3D_OK ) {
			sprintf( d3d_error_str, "GetHandle failed.\n\nError: %s",D3D_Error(error) );
			dllQuit( d3d_error_str );
		}

		if( TextrRestore( num ) == FALSE )
			dprintf("D3D_Fullscreen: TextrRestore (num = %d) phailed.\n", num );
	}

	hw_state.mode = HM_UNINIT;
	hw_state.texmapid = HM_NOTEX;

	dprintf("D3D_Fullscreen: Switched to %s.\n", flag?"Fullscreen":"Windowed" );

	return TRUE;
}



//
//
//
void GFXDLLCALLCONV D3D_GetDescription( char *str ) {

#ifdef _DEBUG
	sprintf(str,"Direct3D driver v%s (need DirectX v%d.%d)",GFXDLL_VERSION,HIBYTE(DIRECTDRAW_VERSION),LOBYTE(DIRECTDRAW_VERSION));
#else
	sprintf(str,"Direct3D driver v%s",GFXDLL_VERSION,HIBYTE(DIRECTDRAW_VERSION),LOBYTE(DIRECTDRAW_VERSION));
#endif

	return;
}



//
//
//
static void GFXDLLCALLCONV D3D_GetData( void *buf, int len ) {

	if( *(ULONG *)buf == GFXDLL_ISMSGOK )
		*(ULONG *)buf = !bInGraphic;

	if( *(ULONG *)buf == GFXDLL_24BIT )
		*(ULONG *)buf = TRUE;

	if( *(ULONG *)buf == GFXDLL_MAXTEXSIZE ) {
		if( hw_state.maxtexw < hw_state.maxtexh )
			*(ULONG *)buf = hw_state.maxtexw;
		else
			*(ULONG *)buf = hw_state.maxtexh;
	}

	if( *(ULONG *)buf == GFXDLL_MINTEXSIZE ) {
		if( hw_state.mintexw > hw_state.mintexh )
			*(ULONG *)buf = hw_state.mintexw;
		else
			*(ULONG *)buf = hw_state.mintexh;
	}

	if( *(ULONG *)buf == GFXDLL_RESIZE ) {
		UPDATEBLTRECT;
	}

	if( *(ULONG *)buf == GFXDLL_GETERROR )
		strncpy( (char *)buf, d3d_error_str, len );
		// *(ULONG *)buf = 0;

	if( *(ULONG *)buf == GFXDLL_TEX24BIT )
		*(ULONG *)buf = hw_state.tex24bit;

	// TODO: fullscreenbe is legyen threadsafe
	if( *(ULONG *)buf == GFXDLL_THREADSAFE ) {
		// *(ULONG *)buf = FALSE;
		*(ULONG *)buf = TRUE;
	}

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


	info->GFXDLL_SetupCulling		= (void *)D3D_SetupCulling;
	info->GFXDLL_AddToPipeline		= (void *)D3D_AddToPipeline;
	info->GFXDLL_AddTexMapHigh		= (void *)D3D_AddTexMapHigh;
	info->GFXDLL_ReloadTexMapHigh		= (void *)D3D_ReloadTexMapHigh;
	info->GFXDLL_ModifyTexMapHigh		= (void *)D3D_ModifyTexMapHigh;
	info->GFXDLL_PutSpritePoly		= (void *)D3D_PutSpritePoly;
	info->GFXDLL_PutPoly			= (void *)D3D_PutPoly;
	info->GFXDLL_DiscardAllTexture		= (void *)D3D_DiscardAllTexture;
	info->GFXDLL_DiscardTexture		= (void *)D3D_DiscardTexture;
	info->GFXDLL_Line			= (void *)D3D_Line;
	info->GFXDLL_PutPixel			= (void *)D3D_PutPixel;
	info->GFXDLL_BeginScene			= (void *)D3D_BeginScene;
	info->GFXDLL_EndScene			= (void *)D3D_EndScene;
	info->GFXDLL_FlushScene			= (void *)D3D_FlushScene;
	info->GFXDLL_LockLfb			= (void *)D3D_LockLfb;
	info->GFXDLL_UnlockLfb			= (void *)D3D_UnlockLfb;
	info->GFXDLL_Init			= (void *)D3D_Init;
	info->GFXDLL_Deinit			= (void *)D3D_Deinit;
	info->GFXDLL_GetDescription		= (void *)D3D_GetDescription;
	info->GFXDLL_GetData			= (void *)D3D_GetData;
	info->GFXDLL_GetPitch			= (void *)D3D_GetPitch;
	info->GFXDLL_Activate			= (void *)D3D_Activate;
	info->GFXDLL_FlipPage			= (void *)D3D_FlipPage;
	info->GFXDLL_Clear			= (void *)D3D_Clear;
	info->GFXDLL_Fog			= (void *)D3D_Fog;
	info->GFXDLL_InitPalette		= (void *)D3D_InitPalette;
	info->GFXDLL_SetRgb			= (void *)D3D_SetRgb;
	info->GFXDLL_GetRgb			= (void *)D3D_GetRgb;
	info->GFXDLL_Fullscreen			= (void *)D3D_Fullscreen;

	return;
}






//
//
//
extern "C" void dllDeinit( void ) {

	D3D_Deinit();

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






void WriteFps( void ) {

#define TIMESLOTS 30
	static float oldtime[TIMESLOTS];
	static int timepos = 0;

	HDC dc;

	LARGE_INTEGER clocks, freq;

	QueryPerformanceCounter(&clocks);
	QueryPerformanceFrequency(&freq);

	oldtime[timepos] = float(clocks.QuadPart) / freq.QuadPart;

	float frametime = oldtime[timepos] - oldtime[(timepos+1) % TIMESLOTS];

	timepos++;
	timepos = timepos % TIMESLOTS;

	SecondarySurface->GetDC(&dc);

	char str[256];
	sprintf(str, "%.2f", float(TIMESLOTS)/frametime);

	TextOut(dc,0,0,str,strlen(str));

	SecondarySurface->ReleaseDC(dc);

	return;
}





/***
	if( mydesc.dwDeviceZBufferBitDepth ) {

		DDSURFACEDESC2 ddsd;

		memset(&ddsd, 0 ,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS |
				DDSD_ZBUFFERBITDEPTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDCAPS_BLTDEPTHFILL;
		ddsd.dwWidth = hw_state.SCREENW;
		ddsd.dwHeight = hw_state.SCREENH;

		if( bIsHardware )
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

		devDepth = mydesc.dwDeviceZBufferBitDepth;
		if( devDepth & DDBD_32 )
			ddsd.dwZBufferBitDepth = 32;
		else
		if( devDepth & DDBD_24 )
			ddsd.dwZBufferBitDepth = 24;
		else
		if( devDepth & DDBD_16 )
			ddsd.dwZBufferBitDepth = 16;
		else
		if( devDepth & DDBD_8 )
			ddsd.dwZBufferBitDepth = 8;
		else {
			sprintf( d3d_error_str, "D3D_Init: Unsupported Z-buffer depth requested by device.");
			dllQuit( d3d_error_str );
		}

		int i;
		int ZBufferMaxValue = 0xffff;

		for( i=0; i<(int)ddsd.dwZBufferBitDepth; i++ )
			SETFLAG( ZBufferMaxValue, i );

		if( !(error = DirectDraw4->CreateSurface( &ddsd, &lpZBuffer, NULL) ) != DD_OK ) {

			if( (error = SecondarySurface->AddAttachedSurface( lpZBuffer )) != DD_OK )
				dllQuit("D3D_Init: AddAttachedBuffer failed for Z-Buffer.\nError: %s",D3D_Error(error) );

			memset(&ddsd, 0 ,sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);

			if( (error=lpZBuffer->GetSurfaceDesc( &ddsd ) ) != DD_OK )
				dllQuit("D3D_Init: Failed to get surface description of Z buffer.\nError: %s",D3D_Error(error) );

			if( bIsHardware && !((ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ? TRUE : FALSE) )
				dllQuit("D3D_Init: Could not fit the Z-buffer in video memory for this hardware device.");

			desc->bZBuffer = TRUE;
		}
		else
			dprintf("D3D_Init: CreateSurface for Z-buffer failed.\nError: %s\n",D3D_Error(error) );
	}
***/


/***

Blend modes:

Setting up blend modes for Alpha blended textures (i.e. textures with alpha
channels):

 SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
 SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);


Color keying via alpha testing:

 SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, true);
 SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_NOTEQUAL);
 SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x00);

 Will pass on values not equal to ALPHAREF.  Here alpha ref is 0, thus
anything
 with a texel color of 0 will not get rendered, whilst everything else will.

 There are other modes for ALPHAFUNC, D3DCMP_EQUAL will inverse the process
above.


Fading a polygon (for non alpha textures):

 Material.diffuse.a = alphavalue;
 SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
 SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

 This will fade a poly by the alphavalue specified above.

 This will not work with textures that have an alpha channel (at least not
 on the Banshee I had)

Fading a poly with alpha textures:
 Material.diffuse.a = alphavalue;
 SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
 SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
 SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
 SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);


To use with ALPHA mapped textures:
 SetTextureStageState(x,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
 SetTextureStageState(x,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
 SetTextureStageState(x,D3DTSS_ALPHAARG2,D3DTA_CURRENT);

Ignoring texture alpha:
 SetTextureStageState(x,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
 SetTextureStageState(x,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
 SetTextureStageState(x,D3DTSS_ALPHAARG2,D3DTA_CURRENT);

Ignore Material alpha:
 SetTextureStageState(x,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
 SetTextureStageState(x,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
 SetTextureStageState(x,D3DTSS_ALPHAARG2,D3DTA_CURRENT);


***/



/***
//-----------------------------------------------------------------------------
//
// SaveSurfaceToTGAFile
//
//-----------------------------------------------------------------------------
static  HRESULT SaveSurfaceToTGAFile(LPDIRECT3DDEVICE8 pD3DDevice, const char *szFileName, LPDIRECT3DSURFACE8 pSurface)
{
	HRESULT hr=S_OK;

	D3DSURFACE_DESC d3dsd;
	LPDIRECT3DSURFACE8 pSurfaceCopy;

	pSurface->GetDesc(&d3dsd);

	hr=pD3DDevice->CreateImageSurface(d3dsd.Width, d3dsd.Height, d3dsd.Format, &pSurfaceCopy);

	if(SUCCEEDED(hr))
	{
		hr=pD3DDevice->CopyRects(pSurface, NULL, 0, pSurfaceCopy, NULL);

		// save the copy to the file
		D3DLOCKED_RECT d3dLR;
		if(SUCCEEDED(hr = pSurfaceCopy->LockRect(&d3dLR, NULL, D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY)))
		{
			// open file
			FILE *pFile=fopen(szFileName, "wb");
			if(pFile!=NULL)
			{
				setvbuf(pFile, NULL, _IOFBF, 64*1024);

				// write header
				char targaheader[18];

				// Set unused fields of header to 0
				memset(targaheader, 0, sizeof(targaheader));

				targaheader[2] = 2;	// image type = uncompressed RGB
				targaheader[12] = (char) (d3dsd.Width & 0xFF);
				targaheader[13] = (char) (d3dsd.Width >> 8);
				targaheader[14] = (char) (d3dsd.Height & 0xFF);
				targaheader[15] = (char) (d3dsd.Height >> 8);
				targaheader[16] = (d3dsd.Format==D3DFMT_A8R8G8B8 || d3dsd.Format==D3DFMT_A1R5G5B5 ? 32 : 24);
				targaheader[17] = 0x20;	// Top-down, non-interlaced

				fwrite(targaheader, 18, 1, pFile);

				// write file
				const BYTE *pImage=(const BYTE *)d3dLR.pBits;
				for(DWORD y=0; y<d3dsd.Height; y++)
				{
					if(d3dsd.Format==D3DFMT_X8R8G8B8)
					{
						const BYTE *pImageAux=pImage;
						for(DWORD x=0; x<d3dsd.Width; x++)
						{
							putc(pImageAux[0], pFile);
							putc(pImageAux[1], pFile);
							putc(pImageAux[2], pFile);
							pImageAux+=4;
						}
					}
					else if(d3dsd.Format==D3DFMT_A8R8G8B8)
					{
						fwrite(pImage, d3dsd.Width, 4, pFile);
					}
					else if(d3dsd.Format==D3DFMT_R5G6B5)
					{
						const BYTE *pImageAux=pImage;
						for(DWORD x=0; x<d3dsd.Width; x++)
						{
							const DWORD dwColor=*((WORD *)pImageAux);

							BYTE color;

							color=BYTE((dwColor&0x001f)<<3);
							putc(color, pFile);

							color=BYTE(((dwColor&0x7e0)>>5)<<2);
							putc(color, pFile);

							color=BYTE(((dwColor&0xf800)>>11)<<3);
							putc(color, pFile);

							pImageAux+=2;
						}
					}
					else if(d3dsd.Format==D3DFMT_X1R5G5B5)
					{
						const BYTE *pImageAux=pImage;
						for(DWORD x=0; x<d3dsd.Width; x++)
						{
							const DWORD dwColor=*((WORD *)pImageAux);

							BYTE color;

							color=BYTE((dwColor&0x001f)<<3);
							putc(color, pFile);

							color=BYTE(((dwColor&0x3e0)>>5)<<3);
							putc(color, pFile);

							color=BYTE(((dwColor&0x7c00)>>10)<<3);
							putc(color, pFile);

							pImageAux+=2;
						}
					}
					else if(d3dsd.Format==D3DFMT_A1R5G5B5)
					{
						const BYTE *pImageAux=pImage;
						for(DWORD x=0; x<d3dsd.Width; x++)
						{
							const DWORD dwColor=*((WORD *)pImageAux);

							BYTE color;

							color=BYTE((dwColor&0x001f)<<3);
							putc(color, pFile);

							color=BYTE(((dwColor&0x3e0)>>5)<<3);
							putc(color, pFile);

							color=BYTE(((dwColor&0x7c00)>>10)<<3);
							putc(color, pFile);

							color=BYTE(((dwColor&0x8000)>>15)*255);
							putc(color, pFile);

							pImageAux+=2;
						}
					}

					pImage+=d3dLR.Pitch;
				}

				fclose(pFile);
			}

		    pSurfaceCopy->UnlockRect();
		}

		pSurfaceCopy->Release();
	}

	return hr;
}
***/