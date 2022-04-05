/* Copyright (C) 1997 Kirschner, Bernát. All Rights Reserved Worldwide. */

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: newtonshell.cpp,v 1.0 2011/04/21 16:52:15 bernie Exp $" )

// http://newtondynamics.com

#define NEWTONDLLNAME  "NEWTON.DLL"

static HINSTANCE hNewton = NULL;		// bass handle

#define LOADNEWTONFUNCTION(f,g) { 						\
	*((void**)&g)=(void*)GetProcAddress(hNewton,#f); 			\
	if( (g) == NULL ) { 							\
		if( hNewton ) FreeLibrary( hNewton ); 				\
		hNewton = NULL; 						\
		xprintf("LoadNewton: can't load \"%s\" function.\n", #f ); 	\
		return FALSE; 							\
	} };



NEWTON_API NewtonWorld* (*_NewtonCreate)() = NULL;
NEWTON_API void (*_NewtonDestroy)(const NewtonWorld* newtonWorld);
NEWTON_API int (*_NewtonMaterialGetDefaultGroupID)(const NewtonWorld* newtonWorld);
NEWTON_API void (*_NewtonMaterialSetDefaultFriction)(const NewtonWorld* newtonWorld, int id0, int id1,dFloat staticFriction, dFloat kineticFriction);
NEWTON_API void (*_NewtonMaterialSetDefaultElasticity)(const NewtonWorld* newtonWorld, int id0, int id1, dFloat elasticCoef);
NEWTON_API void (*_NewtonMaterialSetDefaultSoftness)(const NewtonWorld* newtonWorld, int id0, int id1, dFloat value);
NEWTON_API void* (*_NewtonBodyGetUserData)(const NewtonBody* body);
NEWTON_API void  (*_NewtonBodyGetMassMatrix)(const NewtonBody* body, dFloat* mass, dFloat* Ixx, dFloat* Iyy, dFloat* Izz);
NEWTON_API void  (*_NewtonBodySetForce)(const NewtonBody* body, const dFloat* force);
NEWTON_API NewtonCollision* (*_NewtonCreateSphere)(const NewtonWorld* newtonWorld, dFloat radiusX, dFloat radiusY, dFloat radiusZ, int shapeID, const dFloat *offsetMatrix);
NEWTON_API NewtonCollision* (*_NewtonCreateBox)(const NewtonWorld* newtonWorld, dFloat dx, dFloat dy, dFloat dz, int shapeID, const dFloat *offsetMatrix);
NEWTON_API NewtonCollision* (*_NewtonCreateCone)(const NewtonWorld* newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat *offsetMatrix);
NEWTON_API NewtonCollision* (*_NewtonCreateCapsule)(const NewtonWorld* newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat *offsetMatrix);
NEWTON_API NewtonCollision* (*_NewtonCreateCylinder)(const NewtonWorld* newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat *offsetMatrix);
NEWTON_API NewtonBody* (*_NewtonCreateBody)(const NewtonWorld* newtonWorld, const NewtonCollision* collision, const dFloat* matrix);
NEWTON_API void  (*_NewtonDestroyBody)(const NewtonWorld* newtonWorld, const NewtonBody* body);
NEWTON_API void  (*_NewtonBodySetUserData)(const NewtonBody* body, void* userData);
NEWTON_API void  (*_NewtonBodySetMassMatrix)(const NewtonBody* body, dFloat mass, dFloat Ixx, dFloat Iyy, dFloat Izz);
NEWTON_API int  (*_NewtonBodyGetFreezeState)(const NewtonBody* body);
NEWTON_API void (*_NewtonBodySetFreezeState)(const NewtonBody* body, int state);
NEWTON_API void (*_NewtonBodySetAutoSleep)(const NewtonBody* body, int state);
NEWTON_API void  (*_NewtonBodySetTransformCallback)(const NewtonBody* body, NewtonSetTransform callback);
NEWTON_API void  (*_NewtonBodySetForceAndTorqueCallback)(const NewtonBody* body, NewtonApplyForceAndTorque callback);
NEWTON_API void (*_NewtonBodyAddImpulse)(const NewtonBody* body, const dFloat* pointDeltaVeloc, const dFloat* pointPosit);



// #pragma comment(lib, "c:/myproj~1/newton/Release/newton.lib" )
// #pragma comment(lib, "c:/myproj~1/newton/Release/dJointLibrary.lib" )


point3_t InGameGravity = { 0, -25, 0 };


NewtonWorld *nWorld = NULL;



//
//
//
static BOOL LoadNewton( void ) {

	char newtondllname[XMAX_PATH];

	sprintf( newtondllname, "%s%c%s", GetExeDir(), PATHDELIM, NEWTONDLLNAME );

	// load the temporary BASS.DLL library
	if( (hNewton = LoadLibrary(newtondllname)) == NULL ) {
		xprintf("LoadNewton: couldn't load \"%s\" library.\n",newtondllname);
		return FALSE;
	}

	LOADNEWTONFUNCTION( NewtonCreate, _NewtonCreate );
	LOADNEWTONFUNCTION( NewtonDestroy, _NewtonDestroy );
	LOADNEWTONFUNCTION( NewtonMaterialGetDefaultGroupID, _NewtonMaterialGetDefaultGroupID );
	LOADNEWTONFUNCTION( NewtonMaterialSetDefaultFriction, _NewtonMaterialSetDefaultFriction );
	LOADNEWTONFUNCTION( NewtonMaterialSetDefaultElasticity, _NewtonMaterialSetDefaultElasticity );
	LOADNEWTONFUNCTION( NewtonMaterialSetDefaultSoftness, _NewtonMaterialSetDefaultSoftness );
	LOADNEWTONFUNCTION( NewtonBodyGetUserData, _NewtonBodyGetUserData );
	LOADNEWTONFUNCTION( NewtonBodyGetMassMatrix, _NewtonBodyGetMassMatrix );
	LOADNEWTONFUNCTION( NewtonBodySetForce, _NewtonBodySetForce );
	LOADNEWTONFUNCTION( NewtonCreateSphere, _NewtonCreateSphere );
	LOADNEWTONFUNCTION( NewtonCreateBox, _NewtonCreateBox );
	LOADNEWTONFUNCTION( NewtonCreateCone, _NewtonCreateCone );
	LOADNEWTONFUNCTION( NewtonCreateCapsule, _NewtonCreateCapsule );
	LOADNEWTONFUNCTION( NewtonCreateCylinder, _NewtonCreateCylinder );
	LOADNEWTONFUNCTION( NewtonCreateBody, _NewtonCreateBody );
	LOADNEWTONFUNCTION( NewtonDestroyBody, _NewtonDestroyBody );
	LOADNEWTONFUNCTION( NewtonBodySetUserData, _NewtonBodySetUserData );
	LOADNEWTONFUNCTION( NewtonBodyGetFreezeState, _NewtonBodyGetFreezeState );
	LOADNEWTONFUNCTION( NewtonBodySetFreezeState, _NewtonBodySetFreezeState );
	LOADNEWTONFUNCTION( NewtonBodySetAutoSleep, _NewtonBodySetFreezeState );
	LOADNEWTONFUNCTION( NewtonBodySetTransformCallback, _NewtonBodySetTransformCallback );
	LOADNEWTONFUNCTION( NewtonBodySetForceAndTorqueCallback, _NewtonBodySetForceAndTorqueCallback );
	LOADNEWTONFUNCTION( NewtonBodyAddImpulse, _NewtonBodyAddImpulse );

	xprintf("LoadNewton: %s loaded.\n", newtondllname);

	return TRUE;
}



//
//
//
BOOL InitPhys( void ) {

	if( LoadNewton() == FALSE )
		return FALSE;

	if( nWorld )
		return TRUE;

	xprintf("InitPhys: creating World.\n" );

	// Create the physics environment
	nWorld = _NewtonCreate();

	// Create a physics material (in this example only one)
	int MaterialID = _NewtonMaterialGetDefaultGroupID(nWorld);

	// Configure material (friction, elasticity, softness)
	_NewtonMaterialSetDefaultFriction(nWorld, MaterialID, MaterialID, 0.8f, 0.4f);
	_NewtonMaterialSetDefaultElasticity(nWorld, MaterialID, MaterialID, 0.3f);
	_NewtonMaterialSetDefaultSoftness(nWorld, MaterialID, MaterialID, 0.05f);


	return TRUE;
}



//
//
//
void DeinitPhys( void ) {

    	// Destroy physics environment
	_NewtonDestroy( nWorld );

	nWorld = NULL;

	return;
}


//
//
//
NewtonWorld *PhysGetWorld( void ) {

	if( nWorld == NULL )
		InitPhys();

	return nWorld;
}




//
//
//
static void _cdecl SetMeshTransformEvent( const NewtonBody* nBody, const FLOAT *Matrix, int ThreadIndex ) {

	object_t *obj = (object_t *)_NewtonBodyGetUserData(nBody);

	if( obj ) {

		// Set the object's transformation matrix computed by the physics engine
		matrix4_t Mat;
		memcpy( &Mat[0], Matrix, sizeof(FLOAT)*16 );

		// Mat.scale(Obj->GetMesh()->getScale());
		// Obj->GetMesh()->setMatrix(Mat);
	}

	return;
}


//
//
//
static void _cdecl ApplyForceAndTorqueEvent( const NewtonBody* nBody, FLOAT TimeStep, int ThreadIndex ) {

	object_t *obj = (object_t *)_NewtonBodyGetUserData(nBody);

	if( !obj )
		return;

#ifdef USE_NEWTON

	// Get the mass information
	FLOAT Mass;
	FLOAT Ixx, Iyy, Izz;

	_NewtonBodyGetMassMatrix( obj->nBody, &Mass, &Ixx, &Iyy, &Izz );

	// Set the gravity force (can point in any direction)
	point3_t Dir;

	Dir[0] = InGameGravity[0] * Mass;
	Dir[1] = InGameGravity[1] * Mass;
	Dir[2] = InGameGravity[2] * Mass;

	_NewtonBodySetForce(nBody, Dir);

#endif

	return;
}



//
//
//
BOOL PhysCreateStatic( object_t *obj ) {


	return TRUE;
}



//
//
//
BOOL PhysCreateDynamic( object_t *obj ) {

	if( !obj )
		return FALSE;

#ifdef USE_NEWTON

	// Create the special physics object type
	switch( obj->type ) {

		case PHYS_CUBE: {
			obj->nCollision = _NewtonCreateBox(nWorld, obj->scale[0], obj->scale[1], obj->scale[2], 0, 0);
			// Mesh_       = spSmngr->createModel(scene::ENTITY_CUBE);
		}
		break;

		case PHYS_CONE: {
			obj->nCollision = _NewtonCreateCone(nWorld, obj->scale[1]/2, obj->scale[0], 0, 0);
			// Mesh_       = spSmngr->createModel(scene::ENTITY_CONE);
			// Mesh_->meshTurn(dim::vector3df(0, 0, -90));
		}
		break;

		case PHYS_CYLINDER: {
			obj->nCollision = _NewtonCreateCylinder(nWorld, obj->scale[1]/2, obj->scale[0]*1.5, 0, 0);
			// Mesh_       = spSmngr->createModel(scene::ENTITY_CYLINDER);
			// Mesh_->meshTurn(dim::vector3df(0, 0, 90));
			// Mesh_->meshTransform(dim::vector3df(1.5, 1, 1));
		}
		break;
	}


	matrix4_t mat = { 1, 0, 0, 0,
			  0, 1, 0, 0,
			  0, 0, 1, 0,
			  0, 0, 0, 1 };

	// Create and configure the physics body
	obj->nBody = _NewtonCreateBody( nWorld, obj->nCollision, mat );

	// Mass and user data
	obj->mass = 35.0f;

	_NewtonBodySetUserData(obj->nBody, obj);
	_NewtonBodySetMassMatrix(obj->nBody, obj->mass, 15.0f, 15.0f, 15.0f);

	// No auto freeze/sleep
	_NewtonBodySetFreezeState(obj->nBody, 0);
	_NewtonBodySetAutoSleep(obj->nBody, 0);

	// Newton callback procedures
	_NewtonBodySetTransformCallback(obj->nBody, SetMeshTransformEvent);
	_NewtonBodySetForceAndTorqueCallback(obj->nBody, ApplyForceAndTorqueEvent);

#endif

	return TRUE;
}



//
//
//
BOOL PhysAddImpulse( object_t *obj ) {

	if( !obj )
		return FALSE;

#ifdef USE_NEWTON

	_NewtonBodyAddImpulse( obj->nBody, obj->direction, obj->position );

#endif

	return TRUE;
}

