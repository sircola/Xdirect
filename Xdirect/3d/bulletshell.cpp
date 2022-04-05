
/* Copyright (C) 1997 Kirschner, Bernát. All Rights Reserved Worldwide. */


#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: bulletshell.cpp,v 1.0 2011/04/21 16:52:15 bernie Exp $" )

// http://www.bulletphysics.com
// needed for source/blender/blenkernel/intern/collision.c
// c:\My Projects\bullet-2.79\Demos\BulletDinoDemo\BulletDino.c
// c:\My Projects\bullet-2.79\Demos\OpenPL_Demo\OpenPL_Demo.c

#ifdef _DEBUG
#define BULLETDLLNAME  "DEBUG_BULLET.DLL"
#else
#define BULLETDLLNAME  "BULLET.DLL"
#endif


static HINSTANCE hBullet = 0;		// bass handle

#define LOADBULLETFUNCTION(f,g) { 						\
	*((void**)&g)=(void*)GetProcAddress(hBullet,#f); 			\
	if( (g) == NULL ) { 							\
		if( hBullet ) FreeLibrary( hBullet ); 				\
		hBullet = NULL; 						\
		xprintf("LoadBullet: can't load \"%s\" function.\n", #f ); 	\
		return FALSE; 							\
	} };


static plPhysicsSdkHandle (*_plNewBulletSdk)(void) = NULL;
static void (*_plDeletePhysicsSdk)(plPhysicsSdkHandle physicsSdk) = NULL;
static plCollisionBroadphaseHandle (*_plCreateSapBroadphase)(btBroadphaseCallback beginCallback,btBroadphaseCallback endCallback) = NULL;
static void (*_plDestroyBroadphase)(plCollisionBroadphaseHandle bp) = NULL;
static plBroadphaseProxyHandle (*_plCreateProxy)(plCollisionBroadphaseHandle bp, void* clientData, plReal minX,plReal minY,plReal minZ, plReal maxX,plReal maxY, plReal maxZ) = NULL;
static void (*_plDestroyProxy)(plCollisionBroadphaseHandle bp, plBroadphaseProxyHandle proxyHandle) = NULL;
static void (*_plSetBoundingBox)(plBroadphaseProxyHandle proxyHandle, plReal minX,plReal minY,plReal minZ, plReal maxX,plReal maxY, plReal maxZ) = NULL;
static plCollisionWorldHandle (*_plCreateCollisionWorld)(plPhysicsSdkHandle physicsSdk) = NULL;
static plDynamicsWorldHandle (*_plCreateDynamicsWorld)(plPhysicsSdkHandle physicsSdk) = NULL;
static void (*_plDeleteDynamicsWorld)(plDynamicsWorldHandle world) = NULL;
static void (*_plStepSimulation)(plDynamicsWorldHandle, plReal timeStep) = NULL;
static void (*_plAddRigidBody)(plDynamicsWorldHandle world, plRigidBodyHandle object) = NULL;
static void (*_plRemoveRigidBody)(plDynamicsWorldHandle world, plRigidBodyHandle object) = NULL;
static plRigidBodyHandle (*_plCreateRigidBody)( void* user_data,  float mass, plCollisionShapeHandle cshape ) = NULL;
static void (*_plDeleteRigidBody)(plRigidBodyHandle body) = NULL;
static plCollisionShapeHandle (*_plNewSphereShape)(plReal radius) = NULL;
static plCollisionShapeHandle (*_plNewBoxShape)(plReal x, plReal y, plReal z) = NULL;
static plCollisionShapeHandle (*_plNewCapsuleShape)(plReal radius, plReal height) = NULL;
static plCollisionShapeHandle (*_plNewConeShape)(plReal radius, plReal height) = NULL;
static plCollisionShapeHandle (*_plNewCylinderShape)(plReal radius, plReal height) = NULL;
static plCollisionShapeHandle (*_plNewCompoundShape)(void) = NULL;
static void (*_plAddChildShape)(plCollisionShapeHandle compoundShape,plCollisionShapeHandle childShape, plVector3 childPos,plQuaternion childOrn) = NULL;
static void (*_plDeleteShape)(plCollisionShapeHandle shape) = NULL;
static plCollisionShapeHandle (*_plNewConvexHullShape)(void) = NULL;
static void (*_plAddVertex)(plCollisionShapeHandle convexHull, plReal x,plReal y,plReal z) = NULL;
static plMeshInterfaceHandle (*_plNewMeshInterface)(void) = NULL;
static void (*_plAddTriangle)(plMeshInterfaceHandle meshHandle, plVector3 v0,plVector3 v1,plVector3 v2) = NULL;
static plCollisionShapeHandle (*_plNewStaticTriangleMeshShape)(plMeshInterfaceHandle) = NULL;
static void (*_plSetScaling)(plCollisionShapeHandle shape, plVector3 scaling) = NULL;
static void (*_plGetOpenGLMatrix)(plRigidBodyHandle object, plReal* matrix) = NULL;
static void (*_plGetPosition)(plRigidBodyHandle object,plVector3 position) = NULL;
static void (*_plGetOrientation)(plRigidBodyHandle object,plQuaternion orientation) = NULL;
static void (*_plSetPosition)(plRigidBodyHandle object, const plVector3 position) = NULL;
static void (*_plSetOrientation)(plRigidBodyHandle object, const plQuaternion orientation) = NULL;
static void (*_plSetEuler)(plReal yaw,plReal pitch,plReal roll, plQuaternion orient) = NULL;
static void (*_plSetOpenGLMatrix)(plRigidBodyHandle object, plReal* matrix) = NULL;
static int (*_plRayCast)(plDynamicsWorldHandle world, const plVector3 rayStart, const plVector3 rayEnd, plRayCastResult res) = NULL;
static double (*_plNearestPoints)(float p1[3], float p2[3], float p3[3], float q1[3], float q2[3], float q3[3], float *pa, float *pb, float normal[3]) = NULL;



static plPhysicsSdkHandle physicsSdk = 0;
static plDynamicsWorldHandle dynamicsWorld = 0;



//
//
//
static BOOL LoadBullet( void ) {

	if( hBullet )
		return TRUE;

	char bulletdllname[XMAX_PATH];

	sprintf( bulletdllname, "%s%c%s", GetExeDir(), PATHDELIM, BULLETDLLNAME );

	// load the temporary BASS.DLL library
	if( (hBullet = LoadLibrary(bulletdllname)) == NULL ) {
		xprintf("LoadBullet: couldn't load \"%s\" library.\n",bulletdllname);
		return FALSE;
	}

	LOADBULLETFUNCTION( plNewBulletSdk, _plNewBulletSdk );
	LOADBULLETFUNCTION( plDeletePhysicsSdk, _plDeletePhysicsSdk );
	LOADBULLETFUNCTION( plCreateSapBroadphase, _plCreateSapBroadphase );
	LOADBULLETFUNCTION( plDestroyBroadphase, _plDestroyBroadphase );
	LOADBULLETFUNCTION( plCreateProxy, _plCreateProxy );
	LOADBULLETFUNCTION( plDestroyProxy, _plDestroyProxy );
	LOADBULLETFUNCTION( plSetBoundingBox, _plSetBoundingBox );
	LOADBULLETFUNCTION( plCreateCollisionWorld, _plCreateCollisionWorld );
	LOADBULLETFUNCTION( plCreateDynamicsWorld, _plCreateDynamicsWorld );
	LOADBULLETFUNCTION( plDeleteDynamicsWorld, _plDeleteDynamicsWorld );
	LOADBULLETFUNCTION( plStepSimulation, _plStepSimulation );
	LOADBULLETFUNCTION( plAddRigidBody, _plAddRigidBody );
	LOADBULLETFUNCTION( plRemoveRigidBody, _plRemoveRigidBody );
	LOADBULLETFUNCTION( plCreateRigidBody, _plCreateRigidBody );
	LOADBULLETFUNCTION( plDeleteRigidBody, _plDeleteRigidBody );
	LOADBULLETFUNCTION( plNewSphereShape, _plNewSphereShape );
	LOADBULLETFUNCTION( plNewBoxShape, _plNewBoxShape );
	LOADBULLETFUNCTION( plNewCapsuleShape, _plNewCapsuleShape );
	LOADBULLETFUNCTION( plNewConeShape, _plNewConeShape );
	LOADBULLETFUNCTION( plNewCylinderShape, _plNewCylinderShape );
	LOADBULLETFUNCTION( plNewCompoundShape, _plNewCompoundShape );
	LOADBULLETFUNCTION( plAddChildShape, _plAddChildShape );
	LOADBULLETFUNCTION( plDeleteShape, _plDeleteShape );
	LOADBULLETFUNCTION( plNewConvexHullShape, _plNewConvexHullShape );
	LOADBULLETFUNCTION( plAddVertex, _plAddVertex );
	LOADBULLETFUNCTION( plNewMeshInterface, _plNewMeshInterface );
	LOADBULLETFUNCTION( plAddTriangle, _plAddTriangle );
	LOADBULLETFUNCTION( plNewStaticTriangleMeshShape, _plNewStaticTriangleMeshShape );
	LOADBULLETFUNCTION( plSetScaling, _plSetScaling );
	LOADBULLETFUNCTION( plGetOpenGLMatrix, _plGetOpenGLMatrix );
	LOADBULLETFUNCTION( plGetPosition, _plGetPosition );
	LOADBULLETFUNCTION( plGetOrientation, _plGetOrientation );
	LOADBULLETFUNCTION( plSetPosition, _plSetPosition );
	LOADBULLETFUNCTION( plSetOrientation, _plSetOrientation );
	LOADBULLETFUNCTION( plSetEuler, _plSetEuler );
	LOADBULLETFUNCTION( plSetOpenGLMatrix, _plSetOpenGLMatrix );
	LOADBULLETFUNCTION( plRayCast, _plRayCast );
	LOADBULLETFUNCTION( plNearestPoints, _plNearestPoints );

	xprintf("LoadBullet: %s loaded.\n", bulletdllname);

	return TRUE;
}




//
//
//
BOOL InitBullet( void ) {

	if( LoadBullet() == FALSE )
		return FALSE;

	xprintf("InitBullet: creating World.\n" );

	physicsSdk = _plNewBulletSdk();
	dynamicsWorld = _plCreateDynamicsWorld(physicsSdk);

	return TRUE;
}



//
//
//
void DeinitBullet( void ) {

	if( hBullet == NULL )
		return;

	if( dynamicsWorld ) _plDeleteDynamicsWorld( dynamicsWorld );
	if( physicsSdk ) _plDeletePhysicsSdk( physicsSdk );

	dynamicsWorld = 0;
	physicsSdk = 0;

	return;
}

