/* Copyright (C) 1997 Kirschner, Bernát. All Rights Reserved Worldwide. */


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <xlib.h>
#include <xinner.h>

RCSID( "$Id: trans.c,v 1.0 97-03-10 17:38:49 bernie Exp $" )


// global

ULONG engine_flag = 0L;


static matrix_t T;			// transzformacios matrix

static int projw=0,projh=0;		// SCREENW,SCREENH + CLIPMINX,CLIPMAXX
static FLOAT proj_magn=1.0f;		// a nagyitas project

FLOAT fov = XC_FOV;
static FLOAT xscreenscale, yscreenscale, maxscale;

static point3_t vdir,right,up;
static point3_t cam_pos, cam_dir;

static BOOL clipped_flag = FALSE;

static rgb_t hilite = { 255,255,255, 255 };

static int bbox_num = (-1);		// hány face tartozik egy bounding box-hoz


//
//
//
void SetupProjection( int x, int y, int w, int h ) {

	// TODO: ez miért is kell?
	w += 2;
	h += 2;

	projw = x + (w>>1);
	projh = y + (h>>1);

	xscreenscale = w / fov;
	yscreenscale = h / fov;
	maxscale = MAX(xscreenscale, yscreenscale);

	if( bbox_num == (-1) ) { bbox_num = GETBBOXCNT; }

	return;
}


#define NUM_FRUSTUM_PLANES  ( 4 )
#define CLIP_PLANE_EPSILON  ( 0.0001f )

static plane_t frustumplanes[NUM_FRUSTUM_PLANES];



//
// Rotate a vector from viewspace to worldspace.
//
void BackRotateVector( point3_t *pin, point3_t *pout ) {

	int i;

	// Rotate into the world orientation
	for( i=0; i<3; i++ ) {

		(*pout)[i] = (*pin)[0] * right[i] +
			     (*pin)[1] * up[i] +
			     (*pin)[2] * vdir[i];
	}

	return;
}



//
// Set up a clip plane with the specified normal.
//
static void SetWorldspaceClipPlane( point3_t *normal, plane_t *plane ) {

	// Rotate the plane normal into worldspace
	BackRotateVector( normal, &plane->normal );

	plane->distance = DotProduct( cam_pos, plane->normal ) + CLIP_PLANE_EPSILON;

	return;
}



//
// Set up the planes of the frustum, in worldspace coordinates.
//
static void SetupFrustum( void ) {

	FLOAT angle, s, c;
	point3_t normal;

	angle = GetAtan( 2.0f / fov * maxscale / xscreenscale );
	s = GetSin(angle);
	c = GetCos(angle);

	// Left clip plane
	normal[0] = s;
	normal[1] = 0;
	normal[2] = c;
	SetWorldspaceClipPlane( &normal, &frustumplanes[0] );

	// Right clip plane
	normal[0] = -s;
	SetWorldspaceClipPlane( &normal, &frustumplanes[1] );

	angle = GetAtan( 2.0f / fov * maxscale / yscreenscale );
	s = GetSin(angle);
	c = GetCos(angle);

	// Bottom clip plane
	normal[0] = 0;
	normal[1] = s;
	normal[2] = c;
	SetWorldspaceClipPlane( &normal, &frustumplanes[2] );

	// Top clip plane
	normal[1] = -s;
	SetWorldspaceClipPlane( &normal, &frustumplanes[3] );

#if 0
	// Far plane
	v[0] = vdir[0];
	v[1] = vdir[1];
	v[2] = vdir[2];
	//BackRotateVector( &v, &frustumplanes[4].normal );
	CopyVector( frustumplanes[4].normal, v );
	frustumplanes[4].distance = zclipfar;

	{
	int i;

	for( i=0; i<NUM_FRUSTUM_PLANES; i++ ) {
		DumpVector( frustumplanes[i].normal, "frust[x].normal" );
		printf("dist: %.2f\n",frustumplanes[i].distance );
	}
	}
#endif

	return;
}



//
// Clip a polygon to a plane.
//
static BOOL ClipToPlane( plane_t *pplane,
			 clippoly_t *pin,
			 clippoly_t *pout,
			 point3_t *inst,
			 point3_t *outst,
			 point3_t *inst1,
			 point3_t *outst1,
			 point3_t *inpointnormal,
			 point3_t *outpointnormal,
			 rgb_t *inrgb,
			 rgb_t *outrgb,
			 rgb_t *inrgb1,
			 rgb_t *outrgb1,
			 ULONG engine_flag,
			 ULONG face_flag ) {

	int i,j,nextvert,curin,nextin;
	FLOAT curdot,nextdot,scale;
	point3_t *pinvert, *poutvert;
	point3_t *pinst, *poutst;
	point3_t *pinst1, *poutst1;
	point3_t *pinnorm, *poutnorm;
	rgb_t *pinrgb, *poutrgb;
	rgb_t *pinrgb1, *poutrgb1;

	pinst = inst;
	poutst = outst;

	pinst1 = inst1;
	poutst1 = outst1;

	pinnorm = inpointnormal;
	poutnorm = outpointnormal;

	pinrgb = inrgb;
	poutrgb = outrgb;

	pinrgb1 = inrgb1;
	poutrgb1 = outrgb1;

	pinvert  = pin->vert;
	poutvert = pout->vert;

	curdot = DotProduct( *pinvert, pplane->normal );
	curin = (curdot >= pplane->distance);

	for( i=0; i<pin->nverts; i++ ) {

		nextvert = (i + 1) % pin->nverts;

		// Keep the current vertex if it's inside the plane
		if( curin ) {
			CopyVector( (*poutvert), (*pinvert) );
			++poutvert;

			if( ISFLAG(engine_flag, EF_TEXTURE) ) {
				CopyVector( (*poutst), (*pinst) );
				++poutst;
			}

			if( ISFLAG(face_flag, FF_ENVMAP) ) {
				CopyVector( (*poutst1), (*pinst1) );
				++poutst1;
			}

			CopyVector( (*poutnorm), (*pinnorm) );
			++poutnorm;

			if( !ISFLAG(engine_flag, EF_NOLITE) ) {
				poutrgb->r = pinrgb->r;
				poutrgb->g = pinrgb->g;
				poutrgb->b = pinrgb->b;
				poutrgb->a = pinrgb->a;
				++poutrgb;
				if( ISFLAG(face_flag, FF_ENVMAP) ) {
					poutrgb1->a = pinrgb1->a;
					++poutrgb1;
				}
			}
		}

		nextdot = DotProduct( pin->vert[nextvert], pplane->normal );
		nextin = (nextdot >= pplane->distance);

		// Add a clipped vertex if one end of the current edge is
		// inside the plane and the other is outside
		if( curin != nextin ) {

			clipped_flag = TRUE;

			scale = (pplane->distance - curdot) / (nextdot - curdot);

			for( j=0; j<3; j++ ) {
				(*poutvert)[j] = (*pinvert)[j] + ((pin->vert[nextvert][j] - (*pinvert)[j]) * scale);
				(*poutnorm)[j] = (*pinnorm)[j] + ((inpointnormal[nextvert][j] - (*pinnorm)[j]) * scale);
			}

			// texture

			//scale = DistanceVector( *pinvert, *poutvert ) /
			//		DistanceVector( *pinvert, pin->vert[nextvert] );

			if( ISFLAG(engine_flag, EF_TEXTURE) )
				for( j=0; j<2; j++ )
					(*poutst)[j] = (*pinst)[j] + ( inst[nextvert][j] - (*pinst)[j] ) * scale;

			if( ISFLAG(face_flag, FF_ENVMAP) )
				for( j=0; j<2; j++ )
					(*poutst1)[j] = (*pinst1)[j] + ( inst1[nextvert][j] - (*pinst1)[j] ) * scale;

			if( !ISFLAG(engine_flag, EF_NOLITE) ) {
				poutrgb->r = pinrgb->r + ftoi( (FLOAT)( inrgb[nextvert].r - pinrgb->r ) * scale );
				poutrgb->g = pinrgb->g + ftoi( (FLOAT)( inrgb[nextvert].g - pinrgb->g ) * scale );
				poutrgb->b = pinrgb->b + ftoi( (FLOAT)( inrgb[nextvert].b - pinrgb->b ) * scale );
				poutrgb->a = pinrgb->a + ftoi( (FLOAT)( inrgb[nextvert].a - pinrgb->a ) * scale );
				if( ISFLAG(face_flag, FF_ENVMAP) )
					poutrgb1->a = pinrgb1->a + ftoi( (FLOAT)( inrgb1[nextvert].a - pinrgb1->a ) * scale );
			}

			++poutst;
			++poutst1;
			++poutvert;
			++poutnorm;
			++poutrgb;
			++poutrgb1;
		}

		curdot = nextdot;
		curin = nextin;
		pinvert++;
		pinst++;
		pinst1++;
		pinnorm++;
		pinrgb++;
		pinrgb1++;
	}

	pout->nverts = poutvert - pout->vert;

	if( pout->nverts < 3 )
		return FALSE;

	return TRUE;
}





//
// Clip a polygon to the frustum.
//
static int ClipToFrustum( clippoly_t *pin,
			  clippoly_t *pout,
			  point3_t *inst,
			  point3_t *outst,
			  point3_t *inst1,
			  point3_t *outst1,
			  point3_t *inpointnormal,
			  point3_t *outpointnormal,
			  rgb_t *inrgb,
			  rgb_t *outrgb,
			  rgb_t *inrgb1,
			  rgb_t *outrgb1,
			  ULONG engine_flag,
			  ULONG face_flag ) {

	int i, curpoly;
	clippoly_t *ppoly;
	point3_t *pst;
	point3_t *pst1;
	point3_t *pnorm;
	rgb_t *prgb;
	rgb_t *prgb1;

static clippoly_t tpoly[2];
static point3_t tst[2][XLIB_MAX_POLY_VERTS];
static point3_t tst1[2][XLIB_MAX_POLY_VERTS];
static point3_t tnorm[2][XLIB_MAX_POLY_VERTS];
static rgb_t trgb[2][XLIB_MAX_POLY_VERTS];
static rgb_t trgb1[2][XLIB_MAX_POLY_VERTS];

	curpoly = 0;

	ppoly = pin;
	pst = inst;
	pst1 = inst1;
	pnorm = inpointnormal;
	prgb = inrgb;
	prgb1 = inrgb1;

	for( i=0; i<(NUM_FRUSTUM_PLANES-1); i++ ) {

		if( !ClipToPlane( &frustumplanes[i],
				  ppoly, &tpoly[curpoly],
				  pst, tst[curpoly],
				  pst1, tst1[curpoly],
				  pnorm, tnorm[curpoly],
				  prgb, trgb[curpoly],
				  prgb1, trgb1[curpoly],
				  engine_flag,
				  face_flag ) )
			return 0;

		ppoly = &tpoly[curpoly];
		pst = tst[curpoly];
		pst1 = tst1[curpoly];
		pnorm = tnorm[curpoly];
		prgb = trgb[curpoly];
		prgb1 = trgb1[curpoly];

		curpoly ^= 1;
	}

	return ClipToPlane( &frustumplanes[NUM_FRUSTUM_PLANES-1],
				ppoly, pout,
				pst, outst,
				pst1, outst1,
				pnorm, outpointnormal,
				prgb, outrgb,
				prgb1, outrgb1,
				engine_flag,
				face_flag );
}






//
// Clip a BBOX polygon to a plane.
//
static BOOL bbox_ClipToPlane( plane_t *pplane, clippoly_t *pin, clippoly_t *pout ) {

	int i,j,nextvert,curin,nextin;
	FLOAT curdot,nextdot,scale;
	point3_t *pinvert, *poutvert;

	pinvert  = pin->vert;
	poutvert = pout->vert;

	curdot = DotProduct( *pinvert, pplane->normal );
	curin = (curdot >= pplane->distance);

	for( i=0; i<pin->nverts; i++ ) {

		nextvert = (i + 1) % pin->nverts;

		// Keep the current vertex if it's inside the plane
		if( curin ) {

			CopyVector( (*poutvert), (*pinvert) );
			++poutvert;
		}

		nextdot = DotProduct( pin->vert[nextvert], pplane->normal );
		nextin = (nextdot >= pplane->distance);

		// Add a clipped vertex if one end of the current edge is
		// inside the plane and the other is outside
		if( curin != nextin ) {

			clipped_flag = TRUE;

			scale = (pplane->distance - curdot) / (nextdot - curdot);

			for( j=0; j<MATROW; j++ )
				(*poutvert)[j] = (*pinvert)[j] + ((pin->vert[nextvert][j] - (*pinvert)[j]) * scale);

			++poutvert;
		}

		curdot = nextdot;
		curin = nextin;
		pinvert++;
	}

	pout->nverts = poutvert - pout->vert;

	if( pout->nverts < 3 )
		return FALSE;

	return TRUE;
}





//
// Clip a BBOX polygon to the frustum.
//
static int bbox_ClipToFrustum( clippoly_t *pin,	clippoly_t *pout ) {

	int i, curpoly;
	clippoly_t *ppoly;

	static clippoly_t tpoly[2];

	curpoly = 0;

	ppoly = pin;

	for( i=0; i<(NUM_FRUSTUM_PLANES-1); i++ ) {

		if( !bbox_ClipToPlane( &frustumplanes[i], ppoly, &tpoly[curpoly] ) )
			return 0;

		ppoly = &tpoly[curpoly];

		curpoly ^= 1;
	}

	return bbox_ClipToPlane( &frustumplanes[NUM_FRUSTUM_PLANES-1], ppoly, pout );
}





//
//
//
void GetCameraVector( point3_t out_pos, point3_t out_up, point3_t out_right ) {

	if( out_pos != NULL )
		CopyVector( out_pos, cam_pos );

	if( out_up != NULL )
		CopyVector( out_up, up );

	if( out_right != NULL )
		CopyVector( out_right, right );

	return;
}




//
// a = cam_pos, b = cam_view (a pont amire nez)
//
void GenMatrix( point3_t pos, point3_t dir, point3_t cright, point3_t cup, FLOAT roll ) {

	CopyVector( cam_pos, pos );
	CopyVector( cam_dir, dir );

	SubVector( dir, pos, vdir );
	NormalizeVector( vdir );

	if( cup == NULL ) {

		if( vdir[0] == FLOAT_ZERO && vdir[1] == FLOAT_ZERO )
			{ MAKEVECTOR( up, 0.0f, -vdir[2], 0.0f ); }
		else
			{ MAKEVECTOR( up, 0.0f, 0.0f, 1.0f ); }

		// CopyVector( up, vdir );
		// Nutacio( up, origo3, (FLOAT)(-M_PI_4) );

	}
	else
		CopyVector( up, cup );

	if( cright == NULL ) {
		CrossProduct( vdir, up, right );
		CrossProduct( right, vdir, up );
	}
	else
		CopyVector( right, cright );


	CopyVector( T[0], right );
	CopyVector( T[1], up );
	CopyVector( T[2], vdir );

	// roll
	if( roll != FLOAT_ZERO )
		RotateMatrixZ( T, roll );

	CopyVector( right, T[0] );
	CopyVector( up,    T[1] );
	CopyVector( vdir,  T[2] );

	// a nezopont helyvektora

	T[0][3] = - DotProduct( pos, right );
	T[1][3] = - DotProduct( pos, up );
	T[2][3] = - DotProduct( pos, vdir );

	SetupFrustum();

	return;
}





//
//
//
void AppendRotation( FLOAT Xan, FLOAT Yan, FLOAT Zan ) {

	RotateMatrixX( T, Xan );
	RotateMatrixY( T, Yan );
	RotateMatrixZ( T, Zan );

	return;
}




//
// Clip a polygon to a plane.
//
static BOOL ClipToPlane2( object_t *obj, face_t *f, plane_t *pplane ) {

	int	i,nextvert,curin,nextin;
	FLOAT curdot,nextdot;

	curdot = DotProduct( obj->point[ f->vert[0] ], pplane->normal );
	curin = (curdot >= pplane->distance);

	for( i=0; i<f->nverts; i++ ) {

		nextvert = (i + 1) % f->nverts;

		nextdot = DotProduct( obj->point[ f->vert[nextvert] ], pplane->normal );
		nextin = (nextdot >= pplane->distance);

		if( curin != nextin )
			return TRUE;

		curdot = nextdot;
		curin = nextin;
	}

	return FALSE;
}




//
//
//
static BOOL ClipToPlane3( point3_t *vert, int nverts, plane_t *pplane ) {

	int i,nextvert,curin,nextin;
	FLOAT curdot,nextdot;

	if( nverts < 2 ) {
		xprintf( "ClipToPlane3: only %d verts.\n", nverts );
		return FALSE;
	}

	curdot = DotProduct( vert[0], pplane->normal );
	curin = (curdot >= pplane->distance);

	for( i=0; i<nverts; i++ ) {

		nextvert = (i + 1) % nverts;

		nextdot = DotProduct( vert[ nextvert ], pplane->normal );
		nextin = (nextdot >= pplane->distance);

		if( curin != nextin )
			return TRUE;

		curdot = nextdot;
		curin = nextin;
	}

	return FALSE;
}






//
//
//
BOOL InClipVolumeFace( object_t *obj, int num ) {

	int i;
	BOOL result = FALSE;
	point3_t v;

	for( i=0; i<NUM_FRUSTUM_PLANES; i++ ) {

		// ha barmelyik kivul van
		if( (DotProduct( obj->face[num].center, frustumplanes[i].normal ) < frustumplanes[i].distance) ) {
			result = TRUE;
			break;
		}
	}

	if( result ) {

		SubVector( obj->face[num].center, cam_pos, v );

		if( DotProduct( v, vdir ) < deg2rad(-45.0f) )
			return FALSE;

		for( i=0; i<NUM_FRUSTUM_PLANES; i++ ) {

			if( ClipToPlane2( obj, &obj->face[num], &frustumplanes[i] ) ) {
				result = FALSE;
				break;
			}
		}

	}

	return !result;
}




//
// Clip a point3 set to a plane.
//
static BOOL ClipPointsToPlane( plane_t *pplane,
				point3_t *pin,
				point3_t *pout,
				int innverts,
				int *outnverts ) {

	int i,j,nextvert,curin,nextin;
	FLOAT curdot,nextdot,scale;
	point3_t *pinvert = pin;
	point3_t *poutvert = pout;

	if( innverts < 2 ) {
		xprintf( "ClipPointsToPlane: only %d verts.\n", innverts );
		return FALSE;
	}

	curdot = DotProduct( *pinvert, pplane->normal );
	curin = (curdot >= pplane->distance);

	for( i=0; i<innverts; i++ ) {

		nextvert = (i + 1) % innverts;

		// Keep the current vertex if it's inside the plane
		if( curin ) {
			CopyVector( (*poutvert), (*pinvert) );
			++poutvert;
		}

		nextdot = DotProduct( pin[nextvert], pplane->normal );
		nextin = (nextdot >= pplane->distance);

		// Add a clipped vertex if one end of the current edge is
		// inside the plane and the other is outside
		if( curin != nextin ) {

			clipped_flag = TRUE;

			scale = (pplane->distance - curdot) / (nextdot - curdot);

			for( j=0; j<3; j++ )
				(*poutvert)[j] = (*pinvert)[j] + ((pin[nextvert][j] - (*pinvert)[j]) * scale);

			++poutvert;
		}

		curdot = nextdot;
		curin = nextin;
		pinvert++;
	}

	*outnverts = poutvert - pout;

	if( (*outnverts) < 2 )
		return FALSE;

	return TRUE;
}




//
// Clip a point3 set to the frustum.
//
static int ClipPointsToFrustum( point3_t *pin,
				point3_t *pout,
				int nverts,
				int *outverts ) {

	point3_t tpoly[2][XLIB_MAX_POLY_VERTS];

	// ALLOCMEM( tpoly[0], 2*nverts * sizeof(point3) );
	// ALLOCMEM( tpoly[1], 2*nverts * sizeof(point3) );

	if( nverts > XLIB_MAX_POLY_VERTS ){
		xprintf( "ClipPointsToFrustum: nverts (%d) > %.\n", nverts, XLIB_MAX_POLY_VERTS );
		return 0;
	}

	int i, curpoly=0;
	point3_t *ppoly = pin;

	for( i=0; i<(NUM_FRUSTUM_PLANES-1); i++ ) {

		if( !ClipPointsToPlane( &frustumplanes[i],
					ppoly, tpoly[curpoly],
					nverts, outverts ) ) {
			return 0;
		}

		ppoly = tpoly[curpoly];

		curpoly ^= 1;
	}

	int result = ClipPointsToPlane( &frustumplanes[NUM_FRUSTUM_PLANES-1],
					ppoly, pout,
					nverts, outverts );
	// FREEMEM( tpoly[0] );
	// FREEMEM( tpoly[1] );

	return result;
}








//
//
//
BOOL InClipVolumeObject( object_t *obj, matrix_t m ) {

	BOOL result = FALSE;
	point3_t v,c;

	if( m != NULL )
		MulMatrixVector( m, obj->center, c );
	else
		CopyVector( c, obj->center );
/***
	for( i=0; i<NUM_FRUSTUM_PLANES; i++ ) {

		// ha barmelyik kivul van
		if( (DotProduct( c, frustumplanes[i].normal ) < frustumplanes[i].distance) ) {
			result = TRUE;
			break;
		}
	}
***/
//	if( result ) {

		SubVector( c, cam_pos, v );

		if( DotProduct( v, vdir ) < 0.0f )
			return FALSE;
//	}

	return !result;
}



static matrix_ptr object2world; // object to WorldSpace
static BOOL o2w_flag = FALSE;

#define OBJECT2WORLD (*object2world)

static point3_t inst[XLIB_MAX_POLY_VERTS];
static point3_t inst1[XLIB_MAX_POLY_VERTS];
static point3_t inpointnormal[XLIB_MAX_POLY_VERTS];
static rgb_t inrgb[XLIB_MAX_POLY_VERTS];
static rgb_t inrgb1[XLIB_MAX_POLY_VERTS];
static point3_t tpoint[XLIB_MAX_POLY_VERTS];
static point_t ppoint[XLIB_MAX_POLY_VERTS];
static clippoly_t unclipped_poly,clipped_poly;

static trans2pipeline_t trans2pipeline;

// TODO: ezt is be kell építeni
#define REALLOCCLIPPOLY( num ) {						\
	if( unclipped_poly.nverts < (num) ) {					\
		unclipped_poly.nverts = (num); 					\
		REALLOCMEM( unclipped_poly.vert, (num) * sizeof(point3_t) );	\
		REALLOCMEM( inst, (num) * sizeof(point3_t) );			\
		REALLOCMEM( inst1, (num) * sizeof(point3_t) );			\
		REALLOCMEM( inpointnormal, (num) * sizeof(point3_t) );		\
		REALLOCMEM( inrgb, (num) * sizeof(rgb_t) );			\
		REALLOCMEM( inrgb1, (num) * sizeof(rgb_t) );			\
		REALLOCMEM( tpoint, (num) * sizeof(point3_t) );			\
		REALLOCMEM( ppoint, (num) * sizeof(point_t) );			\
	} 									\
	if( clipped_poly.nverts < (num) ) {					\
		clipped_poly.nverts = (num); 					\
		REALLOCMEM( clipped_poly.vert, (num) * sizeof(point3_t) );	\
	} 									\
	if( (num) == 0 ) {							\
		SAFE_FREEMEM( unclipped_poly.vert );				\
		unclipped_poly.nverts = (num); 					\
		SAFE_FREEMEM( clipped_poly.vert );				\
		clipped_poly.nverts = (num); 					\
		SAFE_FREEMEM( inst );						\
		SAFE_FREEMEM( inst1 );						\
		SAFE_FREEMEM( inpointnormal );					\
		SAFE_FREEMEM( inrgb );						\
		SAFE_FREEMEM( inrgb1 );						\
		SAFE_FREEMEM( tpoint );						\
		SAFE_FREEMEM( ppoint );						\
	}									\
}


//
//
//
static void drawLinePoly( clippoly_t clipped_poly, rgb_t color ) {

	int i;
	point3_t tpoint;
	point_t ppoint[2];
	FLOAT zrecip;

	for( i=0; i<(clipped_poly.nverts-1); i++ ) {

		// elso pont
		// transform

		MulMatrixVector( T, clipped_poly.vert[i], tpoint );

		// projection

		zrecip = maxscale / tpoint[2];

		ppoint[0][0] = ftoi( (FLOAT)projw - (tpoint[0] * zrecip) );
		ppoint[0][1] = ftoi( (FLOAT)projh - (tpoint[1] * zrecip) );

		// második pont
		// transform

		MulMatrixVector( T, clipped_poly.vert[i+1], tpoint );

		// projection

		zrecip = maxscale / tpoint[2];

		ppoint[1][0] = ftoi( (FLOAT)projw - (tpoint[0] * zrecip) );
		ppoint[1][1] = ftoi( (FLOAT)projh - (tpoint[1] * zrecip) );

		Line( ppoint[0][0], ppoint[0][1], ppoint[1][0], ppoint[1][1], color );
	}

	return;
}




//
//
//
BOOL InClipVolumeBbox( object_t *obj, int num, bbox_t *bbox ) {

	int i,j;
	BOOL belul;
	BOOL result = TRUE;
	point3_t v[8];

	if( !o2w_flag ) {
		MAKEVECTOR( v[0], bbox->min[0], bbox->min[1], bbox->min[2] );
		MAKEVECTOR( v[1], bbox->min[0], bbox->max[1], bbox->min[2] );
		MAKEVECTOR( v[2], bbox->max[0], bbox->max[1], bbox->min[2] );
		MAKEVECTOR( v[3], bbox->max[0], bbox->min[1], bbox->min[2] );
		MAKEVECTOR( v[4], bbox->min[0], bbox->min[1], bbox->max[2] );
		MAKEVECTOR( v[5], bbox->min[0], bbox->max[1], bbox->max[2] );
		MAKEVECTOR( v[6], bbox->max[0], bbox->max[1], bbox->max[2] );
		MAKEVECTOR( v[7], bbox->max[0], bbox->min[1], bbox->max[2] );
	}
	else {
		bbox_t temp;
		MulMatrixVector( OBJECT2WORLD, bbox->min, temp.min );
		MulMatrixVector( OBJECT2WORLD, bbox->max, temp.max );
		MAKEVECTOR( v[0], temp.min[0], temp.min[1], temp.min[2] );
		MAKEVECTOR( v[1], temp.min[0], temp.max[1], temp.min[2] );
		MAKEVECTOR( v[2], temp.max[0], temp.max[1], temp.min[2] );
		MAKEVECTOR( v[3], temp.max[0], temp.min[1], temp.min[2] );
		MAKEVECTOR( v[4], temp.min[0], temp.min[1], temp.max[2] );
		MAKEVECTOR( v[5], temp.min[0], temp.max[1], temp.max[2] );
		MAKEVECTOR( v[6], temp.max[0], temp.max[1], temp.max[2] );
		MAKEVECTOR( v[7], temp.max[0], temp.min[1], temp.max[2] );
	}

	for( i=0; i<NUM_FRUSTUM_PLANES; i++ ) {

		belul = FALSE;

		for( j=0; j<dimof(v); j++ ) {

			// curin = (curdot >= pplane->distance);

			// ha barmelyik belul van
			if( (DotProduct( v[j], frustumplanes[i].normal ) >= frustumplanes[i].distance) ) {
				belul = TRUE;
				break;
			}
		}

		if( belul == FALSE ) {
			result = FALSE;
			break;
		}
	}

	// bounding box kirajzolás
	if( (result == TRUE) && ISFLAG(engine_flag, EF_BBOX) ) {

		rgb_t color;
		BOOL partial = FALSE;

		// van-e kívül szög?
		for( i=0; (i<NUM_FRUSTUM_PLANES)&&(partial==FALSE); i++ )
			for( j=0; j<dimof(v); j++ )
				// ha barmelyik KIVÜL van
				if( (DotProduct( v[j], frustumplanes[i].normal ) < frustumplanes[i].distance) ) {
					partial = TRUE;
					break;
				}

		// color = (partial ? (RGBINT24(250,50,50)) : (RGBINT24(50,250,50)));

		color.a = 255;

		if( partial ) {
			color.r = 250;
			color.r = 50;
			color.r = 50;
		}
		else {
			color.r = 50;
			color.r = 250;
			color.r = 50;
		}

		// alja
		unclipped_poly.nverts = 5;

		CopyVector( unclipped_poly.vert[0], v[0] );
		CopyVector( unclipped_poly.vert[1], v[1] );
		CopyVector( unclipped_poly.vert[2], v[2] );
		CopyVector( unclipped_poly.vert[3], v[3] );
		CopyVector( unclipped_poly.vert[4], v[0] );

		clipped_flag = FALSE;
		if( bbox_ClipToFrustum( &unclipped_poly, &clipped_poly ) ) {

			if( clipped_flag == FALSE ) {

				drawLinePoly( unclipped_poly, color );

			}
			else {

				drawLinePoly( clipped_poly, color );

			}
		}

		// tetje
		unclipped_poly.nverts = 5;

		CopyVector( unclipped_poly.vert[0], v[4] );
		CopyVector( unclipped_poly.vert[1], v[5] );
		CopyVector( unclipped_poly.vert[2], v[6] );
		CopyVector( unclipped_poly.vert[3], v[7] );
		CopyVector( unclipped_poly.vert[4], v[4] );

		clipped_flag = FALSE;
		if( bbox_ClipToFrustum( &unclipped_poly, &clipped_poly ) ) {

			if( clipped_flag == FALSE ) {

				drawLinePoly( unclipped_poly, color );

			}
			else {

				drawLinePoly( clipped_poly, color );

			}
		}

		// baloldal
		unclipped_poly.nverts = 5;

		CopyVector( unclipped_poly.vert[0], v[0] );
		CopyVector( unclipped_poly.vert[1], v[1] );
		CopyVector( unclipped_poly.vert[2], v[5] );
		CopyVector( unclipped_poly.vert[3], v[4] );
		CopyVector( unclipped_poly.vert[4], v[0] );

		clipped_flag = FALSE;
		if( bbox_ClipToFrustum( &unclipped_poly, &clipped_poly ) ) {

			if( clipped_flag == FALSE ) {

				drawLinePoly( unclipped_poly, color );

			}
			else {

				drawLinePoly( clipped_poly, color );

			}
		}

		// jobboldal
		unclipped_poly.nverts = 5;

		CopyVector( unclipped_poly.vert[0], v[2] );
		CopyVector( unclipped_poly.vert[1], v[3] );
		CopyVector( unclipped_poly.vert[2], v[7] );
		CopyVector( unclipped_poly.vert[3], v[6] );
		CopyVector( unclipped_poly.vert[4], v[2] );

		clipped_flag = FALSE;
		if( bbox_ClipToFrustum( &unclipped_poly, &clipped_poly ) ) {

			if( clipped_flag == FALSE ) {

				drawLinePoly( unclipped_poly, color );

			}
			else {

				drawLinePoly( clipped_poly, color );

			}
		}

		/***
		point3_t tpoint[8];
		point_t ppoint[8];
		FLOAT zrecip;

		for( i=0; i<dimof(v); i++ ) {

			// transform

			MulMatrixVector( T, v[i], tpoint[i] );

			// projection

			zrecip = maxscale / tpoint[i][2];

			ppoint[i][0] = ftoi( (FLOAT)projw - (tpoint[i][0] * zrecip) );
			ppoint[i][1] = ftoi( (FLOAT)projh - (tpoint[i][1] * zrecip) );
		}

		// alja
		Line( ppoint[0][0], ppoint[0][1], ppoint[1][0], ppoint[1][1], color );
		Line( ppoint[1][0], ppoint[1][1], ppoint[2][0], ppoint[2][1], color );
		Line( ppoint[2][0], ppoint[2][1], ppoint[3][0], ppoint[3][1], color );
		Line( ppoint[0][0], ppoint[0][1], ppoint[3][0], ppoint[3][1], color );

		// teteje
		Line( ppoint[4][0], ppoint[4][1], ppoint[5][0], ppoint[5][1], color );
		Line( ppoint[5][0], ppoint[5][1], ppoint[6][0], ppoint[6][1], color );
		Line( ppoint[6][0], ppoint[6][1], ppoint[7][0], ppoint[7][1], color );
		Line( ppoint[4][0], ppoint[4][1], ppoint[7][0], ppoint[7][1], color );

		// oldal élek
		Line( ppoint[0][0], ppoint[0][1], ppoint[4][0], ppoint[4][1], color );
		Line( ppoint[1][0], ppoint[1][1], ppoint[5][0], ppoint[5][1], color );
		Line( ppoint[2][0], ppoint[2][1], ppoint[6][0], ppoint[6][1], color );
		Line( ppoint[3][0], ppoint[3][1], ppoint[7][0], ppoint[7][1], color );
		***/
	}

	return result;
}




//
//
//
int TransformFace( object_ptr obj, int num ) {

	int i,u,v;
	FLOAT zrecip;
	point3_t fnormal,fcenter;
	face_t *face;

#define P(X) trans2pipeline.X

	P(tpoly).point = tpoint;
	P(ppoly).point = ppoint;

	P(face) = face = &(obj->face[num]);

	renderinfo.faces += 1;

	if( ISFLAG( face->flag, FF_TEXTURE ) )
		SETFLAG( engine_flag, EF_TEXTURE);
	else
		ERASEFLAG( engine_flag, EF_TEXTURE);

	if( ISFLAG( face->flag, FF_NOLITE ) )
		SETFLAG( engine_flag, EF_NOLITE );
	else
		ERASEFLAG( engine_flag, EF_NOLITE );

	if( ISFLAG( engine_flag, EF_HILITE ) )
		SETFLAG( engine_flag, EF_NOLITE );

	if( !o2w_flag ) {
		CopyVector( fnormal, face->normal );
		CopyVector( fcenter, face->center );
	}
	else {
		MulMatrixVectorNoDisplace( OBJECT2WORLD, face->normal, fnormal );
		MulMatrixVector( OBJECT2WORLD, face->center, fcenter );
	}

	// MulMatrixVector( T, f->normal, f->tnormal );
	// NormalizeVector( f->tnormal );

	unclipped_poly.nverts = face->nverts;

	for( i=0; i<face->nverts; i++ ) {

		if( !o2w_flag )
			CopyVector( unclipped_poly.vert[i], obj->point[ face->vert[i] ] );
		else
			MulMatrixVector( OBJECT2WORLD, obj->point[ face->vert[i] ], unclipped_poly.vert[i] );
	}

	if( !ISFLAG( face->flag, FF_FORCEFLAG ) ) {
		if( !PolyFacesViewer( obj, num, unclipped_poly, fnormal, fcenter ) ) {
			renderinfo.rejectedfaces += 1;
			return FALSE;
		}
	}

	for( i=0; i<face->nverts; i++ ) {

		if( !o2w_flag ) {
			// CopyVector( unclipped_poly.vert[i], obj->point[ face->vert[i] ] );
			CopyVector( inpointnormal[i], obj->pointnormal[ face->vert[i] ] );
		}
		else {
			// MulMatrixVector( OBJECT2WORLD, obj->point[ face->vert[i] ], unclipped_poly.vert[i] );
			MulMatrixVectorNoDisplace( OBJECT2WORLD, obj->pointnormal[ face->vert[i] ], inpointnormal[i] );
		}

		if( !ISFLAG(engine_flag, EF_NOLITE) ) {
			inrgb[i].r = face->rgb[i].r;
			inrgb[i].g = face->rgb[i].g;
			inrgb[i].b = face->rgb[i].b;
			inrgb[i].a = face->rgb[i].a;

			if( ISFLAG(face->flag, FF_ENVMAP) )
				inrgb1[i].a = face->rgb1[i].a;
		}

		if( ISFLAG(engine_flag, EF_TEXTURE) ) {
			inst[i][0] = face->s[i];
			inst[i][1] = face->t[i];
		}

		if( ISFLAG(face->flag, FF_ENVMAP) ) {

			// http://www.reindelsoftware.com/Documents/Mapping/Mapping.html
			// Spherical Mapping

			point3_t out;

			// GenMatrix normalizálja
			ReflectVector( vdir, inpointnormal[i], out );
			NormalizeVector( out );

			// Y tengely felöl feszül rá a textúra
			SWAPF( out[1], out[2] );
			out[1] = -out[1];

			// opengl
			FLOAT m = 2 * sqrt( out[0]*out[0] + out[1]*out[1] + (out[2]+1)*(out[2]+1) );

			u = ftoi( (out[0] / m + 0.5f) * 255 );
			v = ftoi( (out[1] / m + 0.5f) * 255 );

			/* old
			u = ftoi( inpointnormal[i][0] * 255.0f );
			v = ftoi( inpointnormal[i][1] * 255.0f );
			*/

			/* Blinn/Newell Latitude Mapping
			u = ftoi(((atan(out[0] / out[2]) + M_PI) / 2 * M_PI) * 255.0f);
			v = ftoi(((asin(out[1]) + (M_PI/2)) / M_PI) * 255.0f );
			*/

			if( u < 0 ) u = -u;
			if( v < 0 ) v = -v;

			if( u > 255 ) u %= 255;
			if( v > 255 ) v %= 255;

			inst1[i][0] = (FLOAT)u + 0.5f;
			inst1[i][1] = (FLOAT)v + 0.5f;
		}
	}

	clipped_flag = FALSE;
	if( !ClipToFrustum( &unclipped_poly, &clipped_poly, inst, P(outst), inst1, P(outst1), inpointnormal, P(outpointnormal), inrgb, P(outrgb), inrgb1, P(outrgb1), engine_flag, face->flag ) )
		return FALSE;

	// ha nem lett clippelve
	if( clipped_flag == FALSE ) {

		for( i=0; i<face->nverts; i++ ) {

			if( obj->xform_flag[ face->vert[i] ] != renderinfo.xformframe_cnt ) {

				obj->xform_flag[ face->vert[i] ] = renderinfo.xformframe_cnt;

				/* transform */

				MulMatrixVector( T, unclipped_poly.vert[i], obj->tpoint[ face->vert[i] ] );

				CopyVector( tpoint[i], obj->tpoint[ face->vert[i] ] );

				/* projection */

				zrecip = maxscale / tpoint[i][2];

				ppoint[i][0] = obj->ppoint[ face->vert[i] ][0] = ftoi( (FLOAT)projw - (tpoint[i][0] * zrecip) );
				ppoint[i][1] = obj->ppoint[ face->vert[i] ][1] = ftoi( (FLOAT)projh - (tpoint[i][1] * zrecip) );
			}
			else {

				renderinfo.xformedverts += 1;

				CopyVector( tpoint[i], obj->tpoint[ face->vert[i] ] );

				ppoint[i][0] = obj->ppoint[ face->vert[i] ][0];
				ppoint[i][1] = obj->ppoint[ face->vert[i] ][1];
			}

		}
	}
	else {

		for( i=0; i<clipped_poly.nverts; i++ ) {

			/* transform */

			MulMatrixVector( T, clipped_poly.vert[i], tpoint[i] );

			/* projection */

			zrecip = maxscale / tpoint[i][2];

			ppoint[i][0] = ftoi( (FLOAT)projw - (tpoint[i][0] * zrecip) );
			ppoint[i][1] = ftoi( (FLOAT)projh - (tpoint[i][1] * zrecip) );
		}
	}

	if( !bZBuffer ) MulMatrixVector( T, fcenter, face->tcenter );

	P(opoly).npoints = P(tpoly).npoints = P(ppoly).npoints = clipped_poly.nverts;
	P(opoly).point = clipped_poly.vert;

	renderinfo.verts += clipped_poly.nverts;

	if( !ISFLAG(engine_flag, EF_NOLITE) ) {

		if( ISFLAG(engine_flag, EF_TEXTURE) ) {

			// texturázott face -> per vertex lighting
			rgb_t l;
			for( i=0; i<P(ppoly).npoints; i++ ) {
				GetLightRgb( P(opoly).point[i], P(outpointnormal)[i], &l );
				P(outrgb)[i].r += l.r;
				P(outrgb)[i].g += l.g;
				P(outrgb)[i].b += l.b;
				// P(outrgb)[i].a += l.a;
			}
/*
			// FIXME: additive a blend tehát hogy lehet sötétíteni?
			// 127< textura világosít
			// 127> sőtétítenie kellene!!!
			// 127= full textúra color
			// FIXME: ezt hogyan oldják meg mások?
			for( i=0; i<P(ppoly).npoints; i++ ) {
				P(outrgb)[i].r -= 255/2;
				P(outrgb)[i].g -= 255/2;
				P(outrgb)[i].b -= 255/2;
				// P(outrgb)[i].a += l.a;
			}
*/
		}
		else {
			// modulate mint a textúrák
			rgb_t l;
			// GetLightRgb( face->tcenter, face->normal, &l );	// per face - Gouroud
			for( i=0; i<P(ppoly).npoints; i++ ) {
				GetLightRgb( P(opoly).point[i], P(outpointnormal)[i], &l );	// per vertex - Phong
				P(outrgb)[i].r = ftoi( (FLOAT)P(outrgb)[i].r * (FLOAT)l.r / 255.0f);
				P(outrgb)[i].g = ftoi( (FLOAT)P(outrgb)[i].g * (FLOAT)l.g / 255.0f);
				P(outrgb)[i].b = ftoi( (FLOAT)P(outrgb)[i].b * (FLOAT)l.b / 255.0f);
			}
		}
	}
	else
	if( ISFLAG(engine_flag, EF_HILITE) ) {
		for( i=0; i<P(ppoly).npoints; i++ ) {
			P(outrgb)[i].r = hilite.r;
			P(outrgb)[i].g = hilite.g;
			P(outrgb)[i].b = hilite.b;
			P(outrgb)[i].a = hilite.a;
		}
	}
	else
		for( i=0; i<P(ppoly).npoints; i++ ) {
			P(outrgb)[i].r = 255;
			P(outrgb)[i].g = 255;
			P(outrgb)[i].b = 255;
			// P(outrgb)[i].a = 255;
		}


#if 0
	// dumpyness

	xprintf("face %d.\n",num);
	for( i=0; i<P(ppoly).npoints; i++ )
		xprintf("(%d): scr: %d, %d    tex: %.2f, %.2f    st: %.2f, %.2f\n",
			i,
			P(ppoly).point[i][0], P(ppoly).point[i][1],
			P(outst1)[i][0], P(outst1)[i][1] );
#endif

	if( GFXDLL_AddToPipeline ) GFXDLL_AddToPipeline( trans2pipeline );

	//AddToPipeline( ppoly, f->tcenter, opoly, tpoly, outpointnormal,
	//		outtex, outst, f );

#undef P

	return TRUE;
}





//
//
//
int TransformObject( object_ptr obj ) {

	int i,j,result = FALSE;
	face_t *face = NULL;
	bbox_t *bbox = NULL;

	renderinfo.objs += 1;

	o2w_flag = FALSE;

	for( i=0; i<obj->nfaces; i+=bbox_num ) {

		if( i >= obj->nfaces )
			break;

		bbox = &(obj->face[ i ].bbox);

		if( InClipVolumeBbox( obj, i, bbox ) == FALSE ) {
			renderinfo.rejectedbboxfacenum += bbox_num;
			continue;
		}

		renderinfo.bboxfacenum += bbox_num;

		for( j=i; j<(i+bbox_num); j++ ) {

			if( j >= obj->nfaces )
				break;

			face = &obj->face[ j ];

			if( ISFLAG( face->flag, FF_DISABLE ) )
				continue;

			if( TransformFace( obj, j ) == TRUE )
				result = TRUE;
		}
	}

	return result;
}



//
//
//
int TransformAnimObject( animobject_t *animobj, matrix_t m ) {

	int result = TRUE;

	int tic = GetTic();

	if( animobj->tic == -1 )
		animobj->tic = tic;

	int frame = ((tic - animobj->tic) % (animobj->numobj * animobj->delay)) / animobj->delay;

	CLAMPMINMAX( frame, 0, animobj->numobj-1 );

	if( m == NULL )
		TransformObject( animobj->objs[frame] );
	else
		TransformObjectMatrix( animobj->objs[frame], m );

	return result;
}



//
//
//
int TransformObjectMatrix( object_ptr obj, matrix_t m ) {

	int i,j,result = FALSE;
	face_t *face = NULL;
	bbox_t *bbox = NULL;

	renderinfo.objs += 1;

	o2w_flag = TRUE;
	object2world = (matrix_t *)m;

	for( i=0; i<obj->nfaces; i+=bbox_num ) {

		if( i >= obj->nfaces )
			break;

		bbox = &(obj->face[ i ].bbox);

		if( InClipVolumeBbox( obj, i, bbox ) == FALSE ) {
			renderinfo.rejectedbboxfacenum += bbox_num;
			continue;
		}

		renderinfo.bboxfacenum += bbox_num;

		for( j=i; j<(i+bbox_num); j++ ) {

			if( j >= obj->nfaces )
				break;

			face = &obj->face[ j ];

			if( ISFLAG( face->flag, FF_DISABLE ) )
				continue;

			if( TransformFace( obj, j ) == TRUE )
				result = TRUE;
		}
	}

	return result;
}



//
//
//
BOOL TransformPoint( point3_t inp, point2_t outp ) {

	point3_t tpoint;
	FLOAT zrecip;

	/* transform */

	MulMatrixVector( T, inp, tpoint );

	/* projection */

	zrecip = maxscale / tpoint[2];

	outp[0] = ftoi( (FLOAT)projw - (tpoint[0] * zrecip) );
	outp[1] = ftoi( (FLOAT)projh - (tpoint[1] * zrecip) );

	return TRUE;
}


//
//
//
BOOL TransformPoints( point3_t *inp, point2_t *outp, int numpoint ) {

	Quit("TransformPoints: Not Yet Implemented!" );

	return FALSE;
}



//
// FIXME: hogy vághat 4 sík 1 vonalat 2-nél több darabba?
//
BOOL TransformLine( point3_t *inp, point2_t *outp ) {

	int numpoint = 2;

	point3_t clipped_points[NUM_FRUSTUM_PLANES*2];
	point3_t tpoint[NUM_FRUSTUM_PLANES*2]; // 2
	int outverts = 0,i;
	FLOAT zrecip;

	clipped_flag = FALSE;

	if( !ClipPointsToFrustum( inp, clipped_points, numpoint, &outverts ) )
		return FALSE;

	// ha nem lett clippelve
	if( clipped_flag == FALSE ) {

		for( i=0; i<numpoint; i++ ) {

			/* transform */

			MulMatrixVector( T, inp[i], tpoint[i] );

			/* projection */

			zrecip = maxscale / tpoint[i][2];

			outp[i][0] = ftoi( (FLOAT)projw - (tpoint[i][0] * zrecip) );
			outp[i][1] = ftoi( (FLOAT)projh - (tpoint[i][1] * zrecip) );
		}
	}
	else {
		// xprintf( "TransformLine: clipped in %d.\n",outverts);

		for( i=0; i<outverts; i++ ) {

			/* transform */

			MulMatrixVector( T, clipped_points[i], tpoint[i] );

			/* projection */

			zrecip = maxscale / tpoint[i][2];

			outp[i][0] = ftoi( (FLOAT)projw - (tpoint[i][0] * zrecip) );
			outp[i][1] = ftoi( (FLOAT)projh - (tpoint[i][1] * zrecip) );
		}
	}


	return TRUE;
}






//
//
//
int ___TransformObject( object_ptr obj ) {

	int i,result = FALSE;

	o2w_flag = FALSE;

	for( i=0; i<obj->nfaces; i++ ) {

		if( ISFLAG( obj->face[i].flag, FF_DISABLE ) )
			continue;

		if( TransformFace( obj, i ) == TRUE )
			result = TRUE;
	}

	return result;
}


//
//
//
int ___TransformObjectMatrix( object_ptr obj, matrix_t m ) {

	int i, result = FALSE;

	renderinfo.objs += 1;

	o2w_flag = TRUE;
	object2world = (matrix_t *)m;

	for( i=0; i<obj->nfaces; i++ ) {

		if( ISFLAG( obj->face[i].flag, FF_DISABLE ) )
			continue;

		if( TransformFace( obj, i ) == TRUE )
			result = TRUE;
	}

	o2w_flag = FALSE;

	return result;
}





//
//
//
void ResetXform( object_t *obj ) {

	if( obj && obj->npoints && obj->xform_flag )
		memset( obj->xform_flag, 0L, obj->npoints * sizeof(ULONG) );

	return;
}



//
//
//
BOOL SetHilightRgb( int r, int g, int b, int alpha ) {

	hilite.r = r;
	hilite.g = g;
	hilite.b = b;

	hilite.a = alpha;

	SETFLAG( engine_flag, EF_HILITE );

	return FALSE;
}




//
//
//
void EraseHilight( void ) {

	ERASEFLAG( engine_flag, EF_HILITE );

	return;
}






//
//
//
void MoveCamera( point3_t cam_pos, point3_t cam_view, int dir, FLOAT amount ) {

	FLOAT dx=0.0f,dy=0.0f,dz=0.0f;
	point3_t normal;

	SubVector( cam_view, cam_pos, normal );
	NormalizeVector( normal );

	switch( dir ) {

		case MC_LEFT:
			dx = normal[1];
			dy = -normal[0];
			dz = 0.0;
			break;

		case MC_RIGHT:
			dx = -normal[1];
			dy = normal[0];
			dz = 0.0;
			break;

		case MC_JUSTFORWARD:
		case MC_FORWARD:
			dx = normal[0];
			dy = normal[1];
			dz = normal[2];
			break;

		case MC_BACKWARD:
			dx = -normal[0];
			dy = -normal[1];
			dz = -normal[2];
			break;

		case MC_UP:
			dx = 0.0;
			dy = 0.0;
			dz = 1.0;
			break;

		case MC_DOWN:
			dx = 0.0;
			dy = 0.0;
			dz = -1.0;
			break;
	}

	cam_pos[0] += (dx*amount);
	cam_pos[1] += (dy*amount);
	cam_pos[2] += (dz*amount);

	if( dir != MC_JUSTFORWARD ) {
		cam_view[0] += (dx*amount);
		cam_view[1] += (dy*amount);
		cam_view[2] += (dz*amount);
	}

	return;
}




//
//
//
void RotateCamera( point3_t cam_pos,
		   point3_t cam_view,
		   point3_t cam_right,
		   point3_t cam_up,
		   FLOAT ang_lefel, FLOAT ang_oldal, BOOL reset ) {

	matrix_t T;
	point3_t v,dir;

	// bázis vektorok kiszámítása elsõ alkalommal
	if( reset == TRUE ) {

		SubVector( cam_view, cam_pos, v );
		NormalizeVector( v );
		AddVector( cam_pos, v, cam_view );

		MAKEVECTOR( cam_up, 0.0f, 0.0f, 1.0f );

		CrossProduct( cam_up, v, cam_right );
		ScaleVector( cam_right, -1.0f );
		NormalizeVector( cam_right );

		CrossProduct( cam_right, v, cam_up );
		NormalizeVector( cam_up );
	}

	// le-fel
	if( ang_lefel != FLOAT_ZERO ) {

		MatrixRotateAxis( cam_right, -ang_lefel, T );

		SubVector( cam_view, cam_pos, dir );
		NormalizeVector( dir );

		MulMatrixVector( T, dir, v );
		AddVector( cam_pos, v, cam_view );

		MulMatrixVector( T, cam_up, v );
		CopyVector( cam_up, v );
	}


	// vizszintes irány: jobbra-balra
	if( ang_oldal != FLOAT_ZERO ) {

		MatrixRotateAxis( cam_up, ang_oldal, T );

		SubVector( cam_view, cam_pos, dir );
		NormalizeVector( dir );

		MulMatrixVector( T, dir, v );
		AddVector( cam_pos, v, cam_view );

		MulMatrixVector( T, cam_right, v );
		CopyVector( cam_right, v );
	}

	return;
}




//
// a = sin/(1 - zn/zf)
// b = -a*zn
// FOV = field of view
// zn = near clip plane
// zf = far clip plane
//
// [ cos( FOV/2 )  0			      0 	     0 ]
// [ 0		   cos( FOV/2 ) 	      0 	     0 ]
// [ 0		   0   sin( FOV/2 )/(1 - zn/zf)   sin( FOV/2 ) ]
// [ 0		   0			  -a*zn 	     0 ]
//
//


void GenMatrix2( void ) {

}





#if 0

/*
 * Author: Filippo Tampieri
 */

#define FALSE 0
#define TRUE  1
#define DOT(A,B) (A[0] * B[0] + A[1] * B[1] + A[2] * B[2])

/*
    vertexIsBehindPlane returns TRUE if point P is behind the
    plane of normal N and coefficient d, FALSE otherwise.
*/
vertexIsBehindPlane(P, N, d)
float P[3], N[3], d;
{
    return(DOT(N, P) + d <= 0. ? TRUE : FALSE);
}

/*
    boxIsBehindPlane returns TRUE if the axis-aligned box of
    minimum corner Cmin and maximum corner Cmax is behind the
    plane of normal N and coefficient d, FALSE otherwise.
*/
boxIsBehindPlane(Cmin, Cmax, N, d)
float Cmin[3], Cmax[3], N[3], d;
{
    register int i;
    float P[3];

    /*
        assign to P the corner further away
        along the direction of normal N
    */
    for(i = 0; i < 3; i++)
        P[i] = N[i] >= 0. ? Cmax[i] : Cmin[i];

    /* test P against the input plane */
    return(vertexIsBehindPlane(P, N, d));
}



#endif
