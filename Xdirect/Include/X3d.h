/* Copyright (C) 1997-99 Kirschner, Bernát. All Rights Reserved Worldwide. */
/* mailto: bernie@freemail.hu											   */
/* tel: +36 20 333 9517													   */

#ifndef _3D_H_INCLUDED
#define _3D_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#define USE_BULLET
// #define USE_NEWTON

#define XLIB_NUM_POLYS 100
#define XLIB_NUM_POLY_VERTEX 6

#define MAXOBJNAMCHAR 80
#define MAXVERTNUM 6


#define ANGLE2RAD  0.01745329251994329577f
#define LOG2E	   1.4426950408889634074f
#define LOG2(x)    (LOG2E * log(x))			// nat. log to log 2
#define rad2deg(x) ((x)*57.29577951f)			// times 180.0/PI
#define deg2rad(x) ((x)*ANGLE2RAD)			// times 180.0/PI

#define sqr(x)	( (x) * (x) )

#define FOK2RAD(fi) ( ( (fi) * (3.14159265) ) / (180.0) )
#define RAD2FOK(fi) ( ( (fi) * (180.0) ) / (3.14159265) )


XLIBDEF cvar_t zclipnear;
XLIBDEF cvar_t zclipfar;
XLIBDEF cvar_t gamma_corr;


#define MAKEVECTOR(u,x,y,z) {	\
		(u)[0] = (x);	\
		(u)[1] = (y);	\
		(u)[2] = (z); }

#define ZEROVECTOR(u) memset(&(u),0L,sizeof(point3_t))
#define ZEROQUAT(u)   memset(&(u),0L,sizeof(quat_t))
#define ZEROMATRIX(u) memset(&(u),0L,sizeof(matrix_t))


#define MAKEMATRIX(M,m00,m01,m02,m03,					    \
		     m10,m11,m12,m13,					    \
		     m20,m21,m22,m23 ) {				    \
	(M)[0][0] = m00; (M)[0][1] = m01; (M)[0][2] = m02; (M)[0][3] = m03; \
	(M)[1][0] = m10; (M)[1][1] = m11; (M)[1][2] = m12; (M)[1][3] = m13; \
	(M)[2][0] = m20; (M)[2][1] = m21; (M)[2][2] = m22; (M)[2][3] = m23; }

#define CLEARXFORM(obj) memset( (obj)->xform_flag, FALSE, (obj)->npoints*sizeof( (obj)->xform_flag[0] ) );


// newton.cpp

#ifdef __cplusplus

#include "c:/myproj~1/newton/include/newton.h"

typedef enum {
	PHYS_CUBE = 0,
	PHYS_SPHERE,
	PHYS_CONE,
	PHYS_CYLINDER,
	PHYS_POLYGON,
	PHYS_ENVIRONMENT,    // No motion for those objects
} phystype_t;


XLIBDEF NewtonWorld *nWorld;
XLIBDEF BOOL InitPhys( void );
XLIBDEF void DeinitPhys( void );
XLIBDEF NewtonWorld *PhysGetWorld( void );

#endif

// Bullet Phys

#ifdef USE_BULLET
#include "c:/MYPROJ~1/bullet-2.79/src/BULLET~1.H"

XLIBDEF BOOL InitBullet( void );
XLIBDEF void DeinitBullet( void );



#endif

// trans.cpp
#define EF_NOLITE	(1)
#define EF_TEXTURE	(2)
#define EF_HILITE	(3)
#define EF_BBOX		(4)				// kirajzolja-e a bounding boxokat

XLIBDEF ULONG engine_flag;


#pragma pack(push, 1)

typedef struct bound_s {

	FLOAT x1,x2;
	FLOAT y1,y2;
	FLOAT z1,z2;

} bound_t, *bound_ptr;

typedef struct bbox_s {

	point3_t min,max;		// bounding box

	point3_t pos;			// bounding sphere
	FLOAT r;

	BOOL dirty;

} bbox_t, *bbox_ptr;



typedef struct polygon_s {

	int		npoints;
	point_ptr	point;

} polygon_t,*polygon_ptr;





typedef struct polygon3_s {

	int		npoints;
	point3_ptr	point;

} polygon3_t,*polygon3_ptr;




#define XLIB_MAX_POLY_VERTS    10   	// assumes polygons have no more than
					//	four sides and are clipped a
					//	maximum of four times by frustum.
					//	Must be increased for more sides
					//	or more clip planes


typedef struct clippoly_s {

	int nverts;
	point3_t vert[XLIB_MAX_POLY_VERTS];

} clippoly_t, *clippoly_ptr;




typedef struct plane_s {

	FLOAT distance;
	point3_t normal;

} plane_t, *plane_ptr;






typedef struct triangle_s {

	point_t point[3];

} triangle_t, *triangle_ptr, tri_t, *tri_ptr;




typedef struct triangle3_s {

	point3_t point[3];

} triangle3_t, *triangle3_ptr, tri3_t, *tri3_ptr;



#define MAX_MIPMAP			3

#define TEXMAPW				64
#define TEXMAPH				64
#define TEXMAPMASKW			0x3f
#define TEXMAPMASKH			0x3f

// #define MAXTEXMAPNAMELEN PATH_MAX
// #define MAXTEXMAPNAMELEN (1024*2)

typedef struct texmap_s {

	struct texmap_s *prev;
	struct texmap_s *next;

 	UCHAR	*sprite1,*sprite2,*sprite3;
	UCHAR	*bump1,*bump2,*bump3;

	int	width1,height1;
	int	width2,height2;
	int	width3,height3;
	int	origw, origh;			// a bitmap eredeti méretei Original Width Height

	int	wmask1,hmask1;
	int	wmask2,hmask2;
	int	wmask3,hmask3;

	char	name[XMAX_PATH];

	BOOL	isChromaKey;
	ULONG	chromaColor;

	BOOL	isAlpha;

	BOOL	scaled;

	int	hwtexmapid;
	int	texmapid;

} texmap_t,*texmap_ptr;

#pragma pack(pop)

XLIBDEF texmap_t *texmap;
XLIBDEF int ntexmaps;

#define ERASETEXMAP( i ) {		\
	texmap[(i)].next = NULL;	\
	texmap[(i)].prev = NULL;	\
	texmap[(i)].name[0] = 0;	\
	texmap[(i)].sprite1 = NULL;	\
	texmap[(i)].sprite2 = NULL;	\
	texmap[(i)].sprite3 = NULL;	\
	texmap[(i)].bump1 = NULL;	\
	texmap[(i)].bump2 = NULL;	\
	texmap[(i)].bump3 = NULL;	\
	texmap[(i)].width1 = 0; 	\
	texmap[(i)].width2 = 0; 	\
	texmap[(i)].width3 = 0; 	\
	texmap[(i)].height1 = 0;	\
	texmap[(i)].height2 = 0;	\
	texmap[(i)].height3 = 0;	\
	texmap[(i)].wmask1 = 0; 	\
	texmap[(i)].wmask2 = 0; 	\
	texmap[(i)].wmask3 = 0; 	\
	texmap[(i)].hmask1 = 0; 	\
	texmap[(i)].hmask2 = 0; 	\
	texmap[(i)].hmask3 = 0; 	\
	texmap[(i)].isChromaKey = FALSE;\
	texmap[(i)].chromaColor = 0L;	\
	texmap[(i)].isAlpha = FALSE;	\
	texmap[(i)].scaled = FALSE;	\
	texmap[(i)].hwtexmapid = -1;	\
	texmap[(i)].texmapid = -1;	}


#define FF_COLOR		1
#define FF_TEXTURE		2
#define FF_RASTER		3
#define FF_LINE 		4
#define FF_SPRITE		5
#define FF_DBLSIDE		6
#define FF_NOLITE		7
#define FF_DISABLE		8
#define FF_FARTHEST		9
#define FF_FORCEFLAG	10
#define FF_ENVMAP		11

#define FS_COLOR	"color"
#define FS_TEXTURE	"texture"
#define FS_RASTER	"raster"
#define FS_LINE 	"line"
#define FS_SPRITE	"sprite"
#define FS_DOUBLE	"double"
#define FS_CHROME	"chrome"
#define FS_ALPHA	"alpha"

#define BADTEX		"BADTEX"
#define NULLTEX 	"NULLTEX"


#define FACE_MAXNVERTS	4

#pragma pack(push, 1)

typedef struct face_s {

	// void		*memory;

	int		nverts;
	int		vert[FACE_MAXNVERTS];		// [FACE_MAXNVERTS];

	point3_t	normal,tnormal;

	point3_t	center,tcenter;

	ULONG		flag;				// FF_RASTER,...
	ULONG		color;				// flat shade

	int		texmapid,texmapid1,texmapid2;	// a texmap[]-ben hanyadik
	point_t 	texvert[FACE_MAXNVERTS],texvert1[FACE_MAXNVERTS],texvert2[FACE_MAXNVERTS]; /* [FACE_MAXNVERTS]*/
	FLOAT		s[FACE_MAXNVERTS],s1[FACE_MAXNVERTS],s2[FACE_MAXNVERTS]; 	/* [FACE_MAXNVERTS]; */	  // s = [ 0...256 ] ld Glide
	FLOAT		t[FACE_MAXNVERTS],t1[FACE_MAXNVERTS],t2[FACE_MAXNVERTS]; 	/* [FACE_MAXNVERTS]; */	  // t = [ 0...256 ]

	rgb_t		rgb[FACE_MAXNVERTS],rgb1[FACE_MAXNVERTS],rgb2[FACE_MAXNVERTS]; /* [FACE_MAXNVERTS];	*/	// nverts db szin

	bbox_t		bbox;

} face_t,*face_ptr;

#pragma pack(pop)


#define ALLOCFACE( face, num ) {			\
	(face).nverts = (num);				\
}

//
// Blender Bmesh miatt
//
#define bmesh_ALLOCFACE( face, num ) {			\
							\
	(face).nverts = (num);				\
							\
	int size = (num) * sizeof(int) +		\
		   (num) * sizeof(point_t) * 3 +	\
		   (num) * sizeof(FLOAT) * 6 +		\
		   (num) * sizeof(rgb_t) * 3;		\
							\
	ALLOCMEM( (face).memory, size )			\
							\
	void *ptr = (face).memory;			\
							\
	(face).vert = (int *)ptr;			\
							\
	ptr += (num) * sizeof(int);			\
	(face).texvert = (point_t *)ptr;		\
	ptr += (num) * sizeof(point_t);			\
	(face).texvert1 = (point_t *)ptr;		\
	ptr += (num) * sizeof(point_t);			\
	(face).texvert2 = (point_t *)ptr;		\
							\
	ptr += (num) * sizeof(point_t);			\
	(face).s = (FLOAT *)ptr;			\
	ptr += (num) * sizeof(FLOAT);			\
	(face).s1 = (FLOAT *)ptr;			\
	ptr += (num) * sizeof(FLOAT);			\
	(face).s2 = (FLOAT *)ptr;			\
							\
	ptr += (num) * sizeof(FLOAT);			\
	(face).t = (FLOAT *)ptr;			\
	ptr += (num) * sizeof(FLOAT);			\
	(face).t1 = (FLOAT *)ptr;			\
	ptr += (num) * sizeof(FLOAT);			\
	(face).t2 = (FLOAT *)ptr;			\
							\
	ptr += (num) * sizeof(FLOAT);			\
	(face).rgb = (rgb_t *)ptr;			\
	ptr += (num) * sizeof(rgb_t);			\
	(face).rgb1 = (rgb_t *)ptr;			\
	ptr += (num) * sizeof(rgb_t);			\
	(face).rgb2 = (rgb_t *)ptr;			\
}





#pragma pack(push, 1)

//
// Object type
//
typedef struct object_s {

	int		npoints;		// mennyi pont
	int		nfaces;

	point3_t	*point, *pointnormal;	// pontok terben
	point3_t	*tpoint; //*tpointnormal;  // transzformalt pont
	point_t 	*ppoint; //*ppointnormal;  // a screen coord

	ULONG		*xform_flag;		// TRUE ha mar igen

	point3_t	center,tcenter;

	face_t		*face;
	char		*name;

	bbox_t		bbox;

#ifdef __cplusplus
#ifdef INCCOLDET
	CollisionModel3D *model;
#endif
#else
	char *dummy_model;
#endif


#ifdef USE_NEWTON
#ifdef __cplusplus
	// Newton

	phystype_t	type;

	point3_t	velocity;
	point3_t	scale;
	point3_t	direction;
	point3_t	position;
	point3_t	rotation;
	FLOAT		mass;

	NewtonCollision *nCollision;
        NewtonBody	*nBody;
	// End
#endif
#endif


#ifdef USE_BULLET

	// Bullet Phys

	plPhysicsSdkHandle	*btPhysicsSdk;
	plDynamicsWorldHandle	*btDynamicsWorld;

	plCollisionShapeHandle 	btCollisionShape;
	plRigidBodyHandle 	btRigidBody;

	FLOAT 			btRadius;
	FLOAT 			btMass;
#endif

} object_t,*object_ptr;

#pragma pack(pop)



//
// AnimObject type
//
typedef struct animobject_s {

	int		numobj;

	int 		delay;		// between frames in millisec
	int 		tic;

	object_t 	**objs;

	point3_t	pos;

	char		filename[XMAX_PATH];

} animobject_t, *animobject_ptr;



#define MANIPULATE_VECTORS1( vector1, action )  		{ vector1[0] action; vector1[1] action; vector1[2] action; }
#define MANIPULATE_VECTORS2( vector1, action, vector2 ) 	{ vector1[0] action vector2[0]; vector1[1] action vector2[1]; vector1[2] action vector2[2]; }
#define MANIPULATE_VECTORS3( vector1, action, vector2, result ) { result[0] = vector1[0] action vector2[0]; result[1] = vector1[1] action vector2[1]; result[2] = vector1[2] action vector2[2]; }

#define NORMALIZE_VECTOR( vector1 ) 	{ FLOAT vecNull[MATROW] = {0.0f,0.0f,0.0f}, dist = Utility::GetDistance(vector1,vecNull); MANIPULATE_VECTORS1( vector1, /=dist ); }


#define COLORDEPTH 16			// see shading

XLIBDEF UCHAR *shadetable;		// [ 256 * COLORDEPTH ];
XLIBDEF int shadeoffset;

#define STABLE(color,offs) ( shadetable[ ( (color) * COLORDEPTH ) + (offs) ] )
//#define GETSHADEDCOLOR(color) ((UCHAR)shadetable[(color)][shadeoffset])


// LIGHT


typedef enum {

	LT_NONE = 0,

	LT_POINT,
	LT_SPOT,
	LT_NORMAL,
	LT_AMBIENT,

	LT_NUMTYPES

} lighttype_t;



#pragma pack(push, 1)

typedef struct light_s {

	struct light_s *prev;
	struct light_s *next;

	point3_t pos,dir;
	point3_t vdir;

	lighttype_t type;

	int r,g,b,a;

	FLOAT dist;

} light_t, *light_ptr;

#pragma pack(pop)

#ifdef __cplusplus
XLIBDEF light_t *AddLight( lighttype_t type, int r,int g,int b,int a, FLOAT dist=0.0f, point3_t pos=NULL, point3_t dir=NULL );
#endif
XLIBDEF BOOL RemoveLight( light_t *light );
XLIBDEF void GetLightRgb( point3_t, point3_t, rgb_t* );


// trig.c

XLIBDEF FLOAT GetSin( FLOAT x );
XLIBDEF FLOAT GetCos( FLOAT x );
XLIBDEF FLOAT GetAsin( FLOAT x );
XLIBDEF FLOAT GetAcos( FLOAT x );
XLIBDEF FLOAT GetAtan( FLOAT x );

XLIBDEF FLOAT ffsqrt( FLOAT n );

XLIBDEF void GenTrig( void );


// vector.c

XLIBDEF point_t origo;
XLIBDEF point3_t origo3;

XLIBDEF int ilog2( int );
XLIBDEF int ihypot( int dx, int dy );
XLIBDEF int lsqrt( int value );
XLIBDEF char *vtos( point3_t v );
XLIBDEF FLOAT randf( FLOAT min, FLOAT max );
XLIBDEF void RandVector( point3_t v );
XLIBDEF void DumpVector( point3_t, char* );
XLIBDEF void GFXDLLCALLCONV CopyVector( point3_t dst, point3_t src );
XLIBDEF void CopyPoint( point_t dst, point_t src );
XLIBDEF void SwapVector( point3_t, point3_t );
XLIBDEF void AddVector( point3_t, point3_t, point3_t);
XLIBDEF void SubVector( point3_t, point3_t, point3_t);
XLIBDEF FLOAT GFXDLLCALLCONV DotProduct( point3_t v1, point3_t v2 );
#define VectorLength AbsoluteVector
XLIBDEF FLOAT AbsoluteVector( point3_t v );
XLIBDEF FLOAT AngleVector( point3_t v1, point3_t v2 );
XLIBDEF void VectorAngles( point3_t, FLOAT*, FLOAT*, FLOAT* );
XLIBDEF FLOAT DistanceVector( point3_t v1, point3_t v2 );
XLIBDEF void ScaleVector( point3_t, FLOAT );
XLIBDEF void CrossProduct( point3_t v1, point3_t v2, point3_t );
XLIBDEF void NormalVectorPlane( point3_t v1, point3_t v2, point3_t, point3_t );
XLIBDEF void NormalVector( point3_t v1, point3_t v2, point3_t );
XLIBDEF void NormalizeVector( point3_t );
XLIBDEF void RotateVectorAxis( point3_t, FLOAT, FLOAT, FLOAT );
XLIBDEF void RotateVectorVector( point3_t, point3_t, FLOAT, FLOAT, FLOAT );
XLIBDEF void RotateAround( point3_t, point3_t, FLOAT, point3_t );
XLIBDEF void RotateVectorAroundAxis( point3_t r, point3_t p, point3_t axis, FLOAT angle );
XLIBDEF void ReflectVector( point3_t v1, point3_t v2, point3_t out );
#ifdef __cplusplus
XLIBDEF void RotatePoint( FLOAT x0, FLOAT y0, FLOAT *x1, FLOAT *y1, FLOAT fok, FLOAT centerx=0.0f, FLOAT centery=0.0f );
#endif
XLIBDEF void Azimut( point3_t, point3_t, FLOAT );
XLIBDEF void Nutacio( point3_t, point3_t, FLOAT );
XLIBDEF BOOL EqualVector( point3_t a, point3_t b );
XLIBDEF FLOAT DistanceLinePoint( point3_t line1, point3_t line2, point3_t point, point3_t *cross );
XLIBDEF UCHAR WriteDir( point3_t dir );
XLIBDEF void ReadDir( UCHAR b, point3_t *dir );
XLIBDEF BOOL VertexInTriangle( point3_t v0, point3_t v1, point3_t v2, point3_t p );
XLIBDEF FLOAT DistanceRayPlane( point3_t vRayOrigin, point3_t vnRayVector, point3_t vnPlaneNormal, FLOAT planeD );
XLIBDEF void Dampen( FLOAT *v, FLOAT amt );
XLIBDEF void DampenTo( FLOAT *v, FLOAT amt, FLOAT center );
XLIBDEF void MatrixFromAxisAngle( point3_t axis, FLOAT theta, matrix_t m );
XLIBDEF void RandomUnitVector( point3_t v );
XLIBDEF void RandomUnitPoint( point_t v );

XLIBDEF void planeABCD( point3_t *p, point3_t a, point3_t b, point3_t c );
XLIBDEF FLOAT planeDistance( point3_t plane, point3_t p );
XLIBDEF FLOAT planePolyDistance( point3_t point, point3_t polynorm, point3_t polypoint );
XLIBDEF BOOL getPlaneIntersection( point3_t lstart, point3_t lend, point3_t plane,  point3_t planepoint, point3_t *ipoint, FLOAT *percentage );
XLIBDEF void planeNormalize( point3_t *plane );
XLIBDEF void planeNormalizeb( point3_t plane, point3_t *planeb );


// matrix.c

XLIBDEF void DumpMatrix( matrix_t );
XLIBDEF void TransposeMatrix( matrix_t );
XLIBDEF void MulMatrix( matrix_t a, matrix_t b, matrix_t c);
XLIBDEF FLOAT DiagonalMulMatrix( matrix_t a );
XLIBDEF void ScaleMatrix( matrix_t a, FLOAT s );
XLIBDEF void AddMatrix( matrix_t a, matrix_t b, matrix_t c);
XLIBDEF void SubMatrix( matrix_t a, matrix_t b, matrix_t c);
XLIBDEF void NormalizeMatrix( matrix_t );
XLIBDEF void OrthoNormalizeMatrix( matrix_t );
XLIBDEF void CopyMatrix( matrix_t dst, matrix_t src );
XLIBDEF void IdentityMatrix( matrix_t m );
XLIBDEF void MakeMatrix( matrix_t,
		FLOAT, FLOAT, FLOAT, FLOAT,
		FLOAT, FLOAT, FLOAT, FLOAT,
		FLOAT, FLOAT, FLOAT, FLOAT );
XLIBDEF FLOAT DetMatrix( matrix_t );
XLIBDEF void InverseMatrix( matrix_t );
XLIBDEF void MatrixRotateAxis( point3_t axis, FLOAT angleDeg, matrix_t m );

XLIBDEF void RotateMatrixX( matrix_t, FLOAT );
XLIBDEF void RotateMatrixY( matrix_t, FLOAT );
XLIBDEF void RotateMatrixZ( matrix_t, FLOAT );
XLIBDEF void TransMatrixX( matrix_t, FLOAT );
XLIBDEF void TransMatrixY( matrix_t, FLOAT );
XLIBDEF void TransMatrixZ( matrix_t, FLOAT );
XLIBDEF void RotateMatrix( matrix_t T, FLOAT Xan, FLOAT Yan, FLOAT Zan );

XLIBDEF void Rotate2Matrix( point3_t, FLOAT, matrix_t );

XLIBDEF void MulMatrixVector( matrix_t m, point3_t a, point3_t b );
XLIBDEF void MulMatrixVectorNoDisplace( matrix_t m, point3_t a, point3_t b );
XLIBDEF void GenTransMatrix( point3_t, point3_t, matrix_t m );


// quat.c


XLIBDEF void DumpQuat( quat_t a, char *name );
XLIBDEF void MulQuat( quat_t a, quat_t b, quat_t c );
XLIBDEF void ConjugateQuat( quat_t a );
XLIBDEF void InvertQuat( quat_t a );
XLIBDEF void NormalizeQuat( quat_t a );
XLIBDEF void AngleAxis2Quat( FLOAT Angle, FLOAT x, FLOAT y, FLOAT z, quat_t a );
XLIBDEF void Quat2Matrix( quat_t a, matrix_t matrix );
XLIBDEF void SlerpQuat( quat_t a, quat_t b, float t, quat_t c );
XLIBDEF void ExpQuat( quat_t a );
XLIBDEF void LogQuat( quat_t a );
XLIBDEF void Euler2Quat( FLOAT rx, FLOAT ry, FLOAT rz, quat_t a );
XLIBDEF void Euler2Quat2( FLOAT rx, FLOAT ry, FLOAT rz, quat_t a );



// object.c

XLIBDEF BOOL CldTest( BOOL flag );
XLIBDEF BOOL LoadObject( char*, object_ptr* );
XLIBDEF BOOL SaveObject( object_ptr );
XLIBDEF BOOL LoadBin( char*, object_ptr* );
XLIBDEF BOOL SaveBin( object_ptr );
XLIBDEF BOOL LoadAsc( char*, object_ptr* );
XLIBDEF BOOL SaveAsc( object_ptr );
XLIBDEF BOOL DupObject( object_t *obj, object_ptr *obj_ptr );
XLIBDEF void OptObject( object_ptr *obj_ptr );
XLIBDEF void InterpolateObject( object_t *o1, object_t *o2, object_t *o3, FLOAT scale );

XLIBDEF BOOL LoadAnimObject( char *filename, animobject_ptr *animobj );
XLIBDEF void DiscardAnimObject( animobject_ptr *animobj );

XLIBDEF BOOL DiscardObject( object_ptr* );


#ifdef __cplusplus
XLIBDEF BOOL CreateGridObj( object_ptr *obj_ptr, int width, int height = -1, int texmapid = -1 );
#endif


// face.c

XLIBDEF void NormalizeObject( object_ptr );
XLIBDEF void BoundObject( object_t *obj, bound_t box );

#define PHONGNAME "env.pcx"
XLIBDEF UCHAR *environment_map;			 // ez lehet barmi

XLIBDEF void DeinitPhongMap( void );
XLIBDEF void MakePhongMap( int depth );
XLIBDEF void StretchPhongMap( int clip );
XLIBDEF void MakeColorFace( face_ptr, UCHAR );
XLIBDEF void MakeRasterFace( face_ptr f, UCHAR color );
XLIBDEF void MakeTextureFace( face_ptr, UCHAR* );
XLIBDEF void MakeLensTexture( void );
#ifdef __cplusplus
XLIBDEF BOOL EnvmapObject( object_t *obj, int texmapid=-1, int alpha=-1 );
#endif

XLIBDEF void ListTexmap( void );
XLIBDEF void DumpTexmap( void );
XLIBDEF int LoadTexmap( char *name, int flag, ULONG color );
XLIBDEF int AddTexMap( UCHAR *spr, const char *name, const char *filename, int flag, ULONG chromaColor );
XLIBDEF int ReloadTexMapHigh( int texmapid, UCHAR *orig_spr, UCHAR *pal, int flag, ULONG chromaColor );
XLIBDEF BOOL ReloadTexMap( int texmapid );
#ifdef __cplusplus
XLIBDEF BOOL ReloadAllTexMap( BOOL flag=FALSE );
#endif

#define TF_ALPHA	1
#define TF_CHROMA	2
#define TF_BUMP 	3
XLIBDEF int AddTexMapHigh( UCHAR *spr, UCHAR *pal, const char *name, const char *filename, int flag, ULONG chromaColor );
XLIBDEF BOOL ModifyTexMap( int texmapid, UCHAR *spr, UCHAR *pal, int flag );

XLIBDEF void DiscardAllTexmap( void );
XLIBDEF void DiscardTexmap( int handler );
XLIBDEF int TexNumForName( const char * );
XLIBDEF int GetFirstTexNum( void );
XLIBDEF char *TexNameForNum( int );
XLIBDEF texmap_t* GFXDLLCALLCONV TexForNum( int texmapid );

XLIBDEF void RotateObject( object_ptr, FLOAT, FLOAT, FLOAT );
XLIBDEF void MoveObject( object_ptr, FLOAT, FLOAT, FLOAT );
XLIBDEF void MulMatrixObject( matrix_t T, object_ptr o );

#define GETBBOXCNT	( getBboxFaceNum( (-1) ) )

#ifdef __cplusplus
XLIBDEF int getBboxFaceNum( int new_value=-1 );
#endif
XLIBDEF BOOL UpdateObjectFaceBbox( object_t *obj );

// pipe.c

typedef struct renderinfo_s {

	int objs, rejectedobjs;
	int faces, rejectedfaces;
	int verts, xformedverts;
	int bboxfacenum, rejectedbboxfacenum;

	int xformframe_cnt;

} renderinfo_t, *renderinfo_ptr;

XLIBDEF renderinfo_t renderinfo;

		      //   cam_pos	  cam_view    znear  zfar
XLIBDEF void SetupCulling( point3_t pos, point3_t dir );  // reset pipeline
XLIBDEF void CullToPipeLine( object_ptr o );
XLIBDEF void ForceToPipeLine( object_ptr o );
XLIBDEF void SortPipeLine( void );
XLIBDEF void FlushPipeLine( void );
XLIBDEF void FreePipeLine( void );
XLIBDEF BOOL PolyFacesViewer( object_t *obj, int num, clippoly_t unclipped_poly, point3_t fnormal, point3_t fcenter );
XLIBDEF void AddToPipeline( polygon_t ppoly,
				point3_t tcenter,
				polygon3_t poly,
				polygon3_t tpoly,
				point3_t *pointnormal,
				point_t *outtex,
				point3_t *inst,
				face_t *f );

XLIBDEF void SetupShading( FLOAT, FLOAT );
XLIBDEF void AddShadeColor( int, int );

XLIBDEF void PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb );
XLIBDEF void PutPoly( polygon_t ppoly, rgb_t *rgb );
XLIBDEF void BeginScene( void );
XLIBDEF void EndScene( void );
XLIBDEF void FlushScene( void );


// trans.c

#pragma pack(push, 1)

typedef struct trans2pipeline_s {

	face_t *face;
	int faceid;

	polygon3_t opoly,tpoly;
	polygon_t ppoly;

	// point_t outtex[XLIB_MAX_POLY_VERTS];
	point3_t outst[XLIB_MAX_POLY_VERTS];
	point3_t outst1[XLIB_MAX_POLY_VERTS];

	point3_t outpointnormal[XLIB_MAX_POLY_VERTS];

	rgb_t outrgb[XLIB_MAX_POLY_VERTS];
	rgb_t outrgb1[XLIB_MAX_POLY_VERTS];

} trans2pipeline_t, *trans2pipeline_ptr;

#pragma pack(pop)

#define XC_FOV	( 1.2f )

XLIBDEF FLOAT fov;


XLIBDEF point3_t *GetCamDir( void );
XLIBDEF point3_t *GetCamPos( void );

XLIBDEF void GetCameraVector( point3_t out_pos, point3_t out_up, point3_t out_right );
XLIBDEF void GenMatrix( point3_t pos, point3_t dir, point3_t right, point3_t cup, FLOAT roll );
XLIBDEF void AppendRotation( FLOAT,FLOAT,FLOAT );
XLIBDEF int TransformObject( object_ptr );
XLIBDEF int TransformObjectMatrix( object_ptr, matrix_t );
XLIBDEF int TransformFace( object_ptr, int num );
#ifdef __cplusplus
XLIBDEF int TransformAnimObject( animobject_t *animobj, matrix_t m=NULL  );
#endif
		    //	cam_pos      cam_view	dir  amount
XLIBDEF void MoveCamera( point3_t, point3_t, int, FLOAT );
#ifdef __cplusplus
XLIBDEF void RotateCamera( point3_t cam_pos, point3_t cam_view, point3_t cam_right,
					point3_t cam_up, FLOAT ang_lefel, FLOAT ang_oldal, BOOL reset = FALSE );
#endif
XLIBDEF BOOL TransformPoint( point3_t inp, point2_t outp );
XLIBDEF BOOL TransformPoints( point3_t *inp, point2_t *outp, int numpoint );
XLIBDEF BOOL TransformLine( point3_t *inp, point2_t *outp );

			 //  soffsx,soffsy,projw, projh, magn
XLIBDEF void SetupProjection( int x, int y, int w, int h );
XLIBDEF BOOL InClipVolumeFace( object_ptr, int );
XLIBDEF BOOL InClipVolumeObject( object_t *obj, matrix_t m );
XLIBDEF void ResetXform( object_t *obj );
XLIBDEF void BackRotateVector( point3_t *pin, point3_t *pout );

XLIBDEF BOOL SetHilightRgb( int r, int g, int b, int alpha );
XLIBDEF void EraseHilight( void );


// texmap.c

XLIBDEF void ___DrawTexturedFace( object_ptr, face_ptr, FIXED *intensity );
XLIBDEF void DrawTexturedFace( polygon_ptr poly, point_ptr texvert, int TexMapId );
XLIBDEF void DrawTexturedFaceHigh( polygon_ptr poly, point_ptr texvert, int TexMapId, polygon3_ptr tpoly );

XLIBDEF void DrawTexturedPoly( polygon_ptr poly, point_ptr texvert, UCHAR *texmap );


XLIBDEF void DrawShadedTexturedPolygon( polygon_ptr Polygon, point_ptr texvert, int texmapid );


// Fog

XLIBDEF FLOAT fognear,fogfar;
XLIBDEF FIXED ffognear,ffogfar;

XLIBDEF void SetupFog( FLOAT fn, FLOAT ff, ULONG fogcolor );


// seg.c

XLIBDEF void FillSeg(polygon_t *PL,UCHAR C,unsigned char *seg,int w,int h);


// gfx.cpp

XLIBDEF void RasterFill( polygon_t *PL, rgb_t C );
XLIBDEF void Fill( polygon_t *PL, rgb_t C );

XLIBDEF void DrawPolygon( polygon_t poly, rgb_t color );
XLIBDEF void DrawPolyline( polygon_t poly, rgb_t color );



// fx.cpp

XLIBDEF FLOAT PerlinGet( FLOAT x );
XLIBDEF FLOAT PerlinGet2D( FLOAT x, FLOAT y );
XLIBDEF FLOAT PerlinGet3D( FLOAT x, FLOAT y, FLOAT t );
XLIBDEF UCHAR PerlinGetI3D( FLOAT x, FLOAT y, FLOAT t );
XLIBDEF UCHAR PerlinGetI3DI( UINT xfp, UINT yfp, UINT tfp );


// particle.cpp

XLIBDEF int CreateParticle( void );
XLIBDEF int DrawParticle( void );
XLIBDEF void ReshapeParticle( int w, int h );
XLIBDEF void SetParticle(  int dnum, int amount );
XLIBDEF char *GetEmitterName( void );
XLIBDEF int GetNumParticle( void );
#ifdef __cplusplus
XLIBDEF void DoParticle( BOOL init, point3_t cam_pos, point3_t cam_view, BOOL fps_cnt=FALSE );
#endif



#ifdef __cplusplus
}
#endif


#endif
