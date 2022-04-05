

#pragma warning(disable : 4996)


#include <memory.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <xlib.h>


#include "particle.h"


RCSID( "$Id: particle.cpp,v 1.1.1.1 2011/05/21 20:44:37 bernie Exp $" )



extern int CreateParticle( void );
extern int DrawParticle( void );
extern void ReshapeParticle( int w, int h );

static int particle_handle = -1, action_handle, maxParticles = 7000;
static int numSteps = 1, demoNum=10;


// system.cpp

float ParticleAction::dt;

_ParticleState _ps;

_ParticleState::_ParticleState()
{
	in_call_list = false;
	in_new_list = false;
	vertexB_tracks = true;

	dt = 1.0f;

	group_id = -1;
	list_id = -1;
	pgrp = NULL;
	pact = NULL;

	Vel = pDomain(PDPoint, 0.0f, 0.0f, 0.0f);
	VertexB = pDomain(PDPoint, 0.0f, 0.0f, 0.0f);
	Color = pDomain(PDPoint, 1.0f, 1.0f, 1.0f);
	Alpha = 1.0f;
	Size1 = 1.0f;
	Size2 = 1.0f;
	Age = 0.0f;

	// The list of groups, etc.
	group_list = new ParticleGroup *[16];
	group_count = 16;
	alist_list = new PAHeader *[16];
	alist_count = 16;
	for(int i=0; i<16; i++)
	{
		group_list[i] = NULL;
		alist_list[i] = NULL;
	}
}

ParticleGroup *_ParticleState::GetGroupPtr(int p_group_num)
{
	if(p_group_num < 0)
		return NULL; // IERROR

	if(p_group_num >= group_count)
		return NULL; // IERROR

	return group_list[p_group_num];
}

PAHeader *_ParticleState::GetListPtr(int a_list_num)
{
	if(a_list_num < 0)
		return NULL; // IERROR

	if(a_list_num >= alist_count)
		return NULL; // IERROR

	return alist_list[a_list_num];
}

// Return an index into the list of particle groups where
// p_group_count groups can be added.
int _ParticleState::GenerateGroups(int p_group_count)
{
	int num_empty = 0;
	int first_empty = -1;

	int i;

	for(i=0; i<group_count; i++)
	{
		if(group_list[i])
		{
			num_empty = 0;
			first_empty = -1;
		}
		else
		{
			if(first_empty < 0)
				first_empty = i;
			num_empty++;
			if(num_empty >= p_group_count)
				return first_empty;
		}
	}

	// Couldn't find a big enough gap. Reallocate.
	int new_count = 16 + group_count + p_group_count;
	ParticleGroup **glist = new ParticleGroup *[new_count];
	memcpy(glist, group_list, group_count * sizeof(void*));
	for(i=group_count; i<new_count; i++)
		glist[i] = NULL;
	delete [] group_list;
	group_list = glist;
	group_count = new_count;

	return GenerateGroups(p_group_count);
}

// Return an index into the list of action lists where
// list_count lists can be added.
int _ParticleState::GenerateLists(int list_count)
{
	int num_empty = 0;
	int first_empty = -1;
	int i;

	for(i=0; i<alist_count; i++)
	{
		if(alist_list[i])
		{
			num_empty = 0;
			first_empty = -1;
		}
		else
		{
			if(first_empty < 0)
				first_empty = i;
			num_empty++;
			if(num_empty >= list_count)
				return first_empty;
		}
	}

	// Couldn't find a big enough gap. Reallocate.
	int new_count = 16 + alist_count + list_count;
	PAHeader **new_list = new PAHeader *[new_count];
	memcpy(new_list, alist_list, alist_count * sizeof(void*));
	for(i=list_count; i<new_count; i++)
		new_list[i] = NULL;
	delete [] alist_list;
	alist_list = new_list;
	alist_count = new_count;

	return GenerateLists(list_count);
}

////////////////////////////////////////////////////////
// Auxiliary calls
void _pCallActionList(ParticleAction *apa, int num_actions,
					  ParticleGroup *pg)
{
	// All these require a particle group, so check for it.
	if(pg == NULL)
		return;

	PAHeader *pa = (PAHeader *)apa;

	// Step through all the actions in the action list.
	for(int action = 0; action < num_actions; action++, pa++)
	{
		//if(_ps.simTime < pa->tlow || _ps.simTime > pa->thigh)
		//	continue;

		switch(pa->type)
		{
		case PABounceID:
			((PABounce *)pa)->Execute(pg);
			break;
		case PACopyVertexBID:
			((PACopyVertexB *)pa)->Execute(pg);
			break;
		case PADampingID:
			((PADamping *)pa)->Execute(pg);
			break;
		case PAExplosionID:
			((PAExplosion *)pa)->Execute(pg);
			break;
		case PAFollowID:
			((PAFollow *)pa)->Execute(pg);
			break;
		case PAGravitateID:
			((PAGravitate *)pa)->Execute(pg);
			break;
		case PAGravityID:
			((PAGravity *)pa)->Execute(pg);
			break;
		case PAJetID:
			((PAJet *)pa)->Execute(pg);
			break;
		case PAKillOldID:
			((PAKillOld *)pa)->Execute(pg);
			break;
		case PAKillSlowID:
			((PAKillSlow *)pa)->Execute(pg);
			break;
		case PAMoveID:
			((PAMove *)pa)->Execute(pg);
			break;
		case PAOrbitLineID:
			((PAOrbitLine *)pa)->Execute(pg);
			break;
		case PAOrbitPointID:
			((PAOrbitPoint *)pa)->Execute(pg);
			break;
		case PARandomAccelID:
			((PARandomAccel *)pa)->Execute(pg);
			break;
		case PARandomDisplaceID:
			((PARandomDisplace *)pa)->Execute(pg);
			break;
		case PARandomVelocityID:
			((PARandomVelocity *)pa)->Execute(pg);
			break;
		case PARestoreID:
			((PARestore *)pa)->Execute(pg);
			break;
		case PASinkID:
			((PASink *)pa)->Execute(pg);
			break;
		case PASinkVelocityID:
			((PASinkVelocity *)pa)->Execute(pg);
			break;
		case PASourceID:
			((PASource *)pa)->Execute(pg);
			break;
		case PATargetColorID:
			((PATargetColor *)pa)->Execute(pg);
			break;
		case PATargetSizeID:
			((PATargetSize *)pa)->Execute(pg);
			break;
		case PAVortexID:
			((PAVortex *)pa)->Execute(pg);
			break;
		}
	}
}

// Add the incoming action to the end of the current action list.
void _pAddActionToList(ParticleAction *S, int size)
{
	if(!_ps.in_new_list)
		return; // ERROR

	if(_ps.pact == NULL)
		return; // ERROR

	if(_ps.list_id < 0)
		return; // ERROR

	PAHeader *alist = _ps.pact;

	if(alist->actions_allocated <= alist->count)
	{
		// Must reallocate.
		int new_alloc = 16 + alist->actions_allocated;
		PAHeader *new_alist = new PAHeader[new_alloc];
		memcpy(new_alist, alist, alist->count * sizeof(PAHeader));

		delete [] alist;
		_ps.alist_list[_ps.list_id] = _ps.pact = alist = new_alist;

		alist->actions_allocated = new_alloc;
	}

	// Now add it in.
	memcpy(&alist[alist->count], S, size);
	alist->count++;
}

////////////////////////////////////////////////////////
// State setting calls

void pColor(float red, float green, float blue, float alpha)
{
	_ps.Alpha = alpha;
	_ps.Color = pDomain(PDPoint, red, green, blue);
}

void pColorD(float alpha, PDomainEnum dtype,
			 float a0, float a1, float a2,
			 float a3, float a4, float a5,
			 float a6, float a7, float a8)
{
	_ps.Alpha = alpha;
	_ps.Color = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

void pVelocity(float x, float y, float z)
{
	_ps.Vel = pDomain(PDPoint, x, y, z);
}

void pVelocityD(PDomainEnum dtype,
				float a0, float a1, float a2,
				float a3, float a4, float a5,
				float a6, float a7, float a8)
{
	_ps.Vel = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

void pVertexB(float x, float y, float z)
{
	_ps.VertexB = pDomain(PDPoint, x, y, z);
}

void pVertexBD(PDomainEnum dtype,
			   float a0, float a1, float a2,
			   float a3, float a4, float a5,
			   float a6, float a7, float a8)
{
	_ps.VertexB = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


void pVertexBTracks(bool trackVertex)
{
	_ps.vertexB_tracks = trackVertex;
}

void pSize(float s1, float s2)
{
	if(s2 < 0.0f) s2 = s1;

	if(s1 < s2)
	{
		_ps.Size1 = s1;
		_ps.Size2 = s2;
	} else {
		_ps.Size1 = s2;
		_ps.Size2 = s1;
	}
}

void pStartingAge(float age)
{
	_ps.Age = age;
}

void pTimeStep(float newDT)
{
	_ps.dt = newDT;
}

////////////////////////////////////////////////////////
// Action List Calls

int pGenActionLists(int action_list_count)
{
	if(_ps.in_new_list)
		return -1; // ERROR

	int ind = _ps.GenerateLists(action_list_count);

	for(int i=ind; i<ind+action_list_count; i++)
	{
		_ps.alist_list[i] = new PAHeader[16];
		_ps.alist_list[i]->actions_allocated = 16;
		_ps.alist_list[i]->type = PAHeaderID;
		_ps.alist_list[i]->count = 1;
	}

	return ind;
}

void pNewActionList(int action_list_num)
{
	if(_ps.in_new_list)
		return; // ERROR

	_ps.pact = _ps.GetListPtr(action_list_num);

	if(_ps.pact == NULL)
		return; // ERROR

	_ps.list_id = action_list_num;
	_ps.in_new_list = true;
}

void pEndActionList()
{
	if(!_ps.in_new_list)
		return; // ERROR

	_ps.in_new_list = false;

	_ps.pact = NULL;
	_ps.list_id = -1;
}

void pDeleteActionLists(int action_list_num, int action_list_count)
{
	if(_ps.in_new_list)
		return; // ERROR

	if(action_list_num < 0)
		return; // ERROR

	if(action_list_num + action_list_count > _ps.alist_count)
		return; // ERROR

	for(int i = action_list_num; i < action_list_num + action_list_count; i++)
	{
		if(_ps.alist_list[i])
		{
			delete [] _ps.alist_list[i];
			_ps.alist_list[i] = NULL;
		}
		else
			return; // ERROR
	}
}

void pCallActionList(int action_list_num)
{
	if(_ps.in_new_list)
		return; // ERROR

	PAHeader *pa = _ps.GetListPtr(action_list_num);

	if(pa == NULL)
		return; // ERRROR

	// XXX A temporary hack.
	pa->dt = _ps.dt;

	_ps.in_call_list = true;

	_pCallActionList(pa+1, pa->count-1, _ps.pgrp);

	_ps.in_call_list = false;
}

////////////////////////////////////////////////////////
// Particle Group Calls

// Create particle groups, each with max_particles allocated.
int pGenParticleGroups(int p_group_count, int max_particles)
{
	if(_ps.in_new_list)
		return -1; // ERROR

	int ind = _ps.GenerateGroups(p_group_count);

	for(int i=ind; i<ind+p_group_count; i++)
	{
		_ps.group_list[i] = (ParticleGroup *)new
			Particle[max_particles + 2];
		_ps.group_list[i]->max_particles = max_particles;
		_ps.group_list[i]->particles_allocated = max_particles;
		_ps.group_list[i]->p_count = 0;
		_ps.group_list[i]->simTime = 0;
	}

	return ind;
}

void pDeleteParticleGroups(int p_group_num, int p_group_count)
{
	if(p_group_num < 0)
		return; // ERROR

	if(p_group_num + p_group_count > _ps.group_count)
		return; // ERROR

	for(int i = p_group_num; i < p_group_num + p_group_count; i++)
	{
		if(_ps.group_list[i])
		{
			delete [] _ps.group_list[i];
			_ps.group_list[i] = NULL;
		}
		else
			return; // ERROR
	}
}

// Change which group is current.
void pCurrentGroup(int p_group_num)
{
	if(_ps.in_new_list)
		return; // ERROR

	_ps.pgrp = _ps.GetGroupPtr(p_group_num);
	if(_ps.pgrp)
		_ps.group_id = p_group_num;
	else
		_ps.group_id = -1;
}

// Change the maximum number of particles in the current group.
int pSetMaxParticles(int max_count)
{
	if(_ps.in_new_list)
		return 0; // ERROR

	ParticleGroup *pg = _ps.pgrp;
	if(pg == NULL)
		return 0; // ERROR

	if(max_count < 0)
		return 0; // ERROR

	// Reducing max.
	if(pg->particles_allocated >= max_count)
	{
		pg->max_particles = max_count;

		// May have to kill particles.
		if(pg->p_count > pg->max_particles)
			pg->p_count = pg->max_particles;

		return max_count;
	}

	// Allocate particles.
	ParticleGroup *pg2 =(ParticleGroup *)new Particle[max_count + 2];
	if(pg2 == NULL)
	{
		// Not enough memory. Just give all we've got.
		// ERROR
		pg->max_particles = pg->particles_allocated;

		return pg->max_particles;
	}

	memcpy(pg2, pg, (pg->p_count + 2) * sizeof(Particle));

	delete [] pg;

	_ps.group_list[_ps.group_id] = _ps.pgrp = pg2;
	pg2->max_particles = max_count;
	pg2->particles_allocated = max_count;

	return max_count;
}

// Copy from the specified group to the current group.
void pCopyGroup(int p_group_num, int index, int copy_count)
{
	if(_ps.in_new_list)
		return; // ERROR

	ParticleGroup *srcgrp = _ps.GetGroupPtr(p_group_num);
	if(srcgrp == NULL)
		return; // ERROR

	ParticleGroup *destgrp = _ps.pgrp;
	if(destgrp == NULL)
		return; // ERROR

	// Find out exactly how many to copy.
	int ccount = copy_count;
	if(ccount > srcgrp->p_count - index)
		ccount = srcgrp->p_count - index;
	if(ccount > destgrp->max_particles - destgrp->p_count)
		ccount = destgrp->max_particles - destgrp->p_count;

	// Directly copy the particles to the current list.
	for(int i=0; i<ccount; i++)
	{
		destgrp->list[destgrp->p_count+i] =
			srcgrp->list[index+i];
	}
	destgrp->p_count += ccount;
}

// Copy from the current group to application memory.
void pGetParticles(int index, int count, float *verts,
				   float *color, float *vel, float *size)
{
	// XXX I should think about whether color means color3, color4, or what.
	// For now, it means color4.

	if(_ps.in_new_list)
		return; // ERROR

	ParticleGroup *pg = _ps.pgrp;
	if(pg == NULL)
		return; // ERROR

	if(index + count > pg->p_count)
		return; // ERROR can't ask for more than there are.

	int vi = 0, ci = 0, li = 0, si = 0;

	// This could be optimized.
	for(int i=0; i<count; i++)
	{
		Particle &m = pg->list[index + i];

		if(verts)
		{
			verts[vi++] = m.pos.x;
			verts[vi++] = m.pos.y;
			verts[vi++] = m.pos.z;
		}

		if(color)
		{
			color[ci++] = m.color.x;
			color[ci++] = m.color.y;
			color[ci++] = m.color.z;
			color[ci++] = m.alpha;
		}

		if(vel)
		{
			vel[li++] = m.vel.x;
			vel[li++] = m.vel.y;
			vel[li++] = m.vel.z;
		}

		if(size)
			size[si++] = m.size;
	}
}

// Returns the number of particles currently in the group.
int pGetGroupCount()
{
	if(_ps.in_new_list)
		return 0; // ERROR

	if(_ps.pgrp == NULL)
		return 0; // ERROR

	return _ps.pgrp->p_count;
}





// actions.cpp
//
// Copyright 1997-1998 by David K. McAllister
// Based in part on code Copyright 1997 by Jonathan P. Leech
//
// This file implements the dynamics of particle actions.


void PABounce::Execute(ParticleGroup *group)
{
	if(position.type == PDTriangle)
	{
		// Compute the inverse matrix of the plane basis.
		pVector &u = position.u;
		pVector &v = position.v;

		// w = u cross v
		float wx = u.y*v.z-u.z*v.y;
		float wy = u.z*v.x-u.x*v.z;
		float wz = u.x*v.y-u.y*v.x;

		float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);

		pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
		s1 *= det;
		pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
		s2 *= -det;

		// See which particles bounce.
		for (int i = 0; i < group->p_count; i++) {
			Particle &m = group->list[i];

			// See if particle's current and next positions cross plane.
			// If not, couldn't bounce, so keep going.
			pVector pnext(m.pos + m.vel * dt);

			// p2 stores the plane normal (the a,b,c of the plane eqn).
			// Old and new distances: dist(p,plane) = n * p + d
			// radius1 stores -n*p, which is d.
			float distold = m.pos * position.p2 + position.radius1;
			float distnew = pnext * position.p2 + position.radius1;

			// Opposite signs if product < 0
			// Is there a faster way to do this?
			if (distold * distnew >= 0)
				continue;

			// Find position at the crossing point by parameterizing
			// p(t) = pos + vel * t
			// Solve dist(p(t),plane) = 0 e.g.
			// n * p(t) + D = 0 ->
			// n * p + t (n * v) + D = 0 ->
			// t = -(n * p + D) / (n * v)
			// Could factor n*v into distnew = distold + n*v and save a bit.
			// Safe since n*v != 0 assured by quick rejection test.
			// This calc is indep. of dt because we have established that it
			// will hit before dt. We just want to know when.
			float nv = position.p2 * m.vel;
			float t = -distold / nv;

			// Actual intersection point p(t) = pos + vel t
			pVector phit(m.pos + m.vel * t);

			// Offset from origin in plane, p - origin
			pVector offset(phit - position.p1);

			// Dot product with basis vectors of old frame
			// in terms of new frame gives position in uv frame.
			float upos = offset * s1;
			float vpos = offset * s2;

			// Did it cross plane outside triangle?
			if (upos < 0 || vpos < 0 || (upos + vpos) > 1)
				continue;

			// A hit! A most palatable hit!

			// Compute tangential and normal components of velocity
			pVector vn(position.p2 * nv); // Normal Vn = (V.N)N
			pVector vt(m.vel - vn); // Tangent Vt = V - Vn

			// Don't apply friction if tangential velocity < cutoff
			float appliedFriction = oneMinusFriction;
			if (cutoffSqr > 0) {
				float vtmag = vt.length2();

				if (vtmag <= cutoffSqr)
					appliedFriction = 1;
			}

			// Compute new velocity heading out:
			// V' = (1-mu) Vt - resilience Vn
			m.vel = vt * appliedFriction - vn * resilience;
		}
	}
	else if(position.type == PDPlane)
	{
		// Compute the inverse matrix of the plane basis.
		pVector &u = position.u;
		pVector &v = position.v;

		// w = u cross v
		float wx = u.y*v.z-u.z*v.y;
		float wy = u.z*v.x-u.x*v.z;
		float wz = u.x*v.y-u.y*v.x;

		float det = 1/(wz*u.x*v.y-wz*u.y*v.x-u.z*wx*v.y-u.x*v.z*wy+v.z*wx*u.y+u.z*v.x*wy);

		pVector s1((v.y*wz-v.z*wy), (v.z*wx-v.x*wz), (v.x*wy-v.y*wx));
		s1 *= det;
		pVector s2((u.y*wz-u.z*wy), (u.z*wx-u.x*wz), (u.x*wy-u.y*wx));
		s2 *= -det;

		// See which particles bounce.
		for (int i = 0; i < group->p_count; i++) {
			Particle &m = group->list[i];

			// See if particle's current and next positions cross plane.
			// If not, couldn't bounce, so keep going.
			pVector pnext(m.pos + m.vel * dt);

			// p2 stores the plane normal (the a,b,c of the plane eqn).
			// Old and new distances: dist(p,plane) = n * p + d
			// radius1 stores -n*p, which is d.
			float distold = m.pos * position.p2 + position.radius1;
			float distnew = pnext * position.p2 + position.radius1;

			// Opposite signs if product < 0
			if (distold * distnew >= 0)
				continue;

			// Find position at the crossing point by parameterizing
			// p(t) = pos + vel * t
			// Solve dist(p(t),plane) = 0 e.g.
			// n * p(t) + D = 0 ->
			// n * p + t (n * v) + D = 0 ->
			// t = -(n * p + D) / (n * v)
			// Could factor n*v into distnew = distold + n*v and save a bit.
			// Safe since n*v != 0 assured by quick rejection test.
			// This calc is indep. of dt because we have established that it
			// will hit before dt. We just want to know when.
			float t = -distold / (position.p2 * m.vel);

			// Actual intersection point p(t) = pos + vel t
			pVector phit(m.pos + m.vel * t);

			// Offset from origin in plane, p - origin
			pVector offset(phit - position.p1);

			// Dot product with basis vectors of old frame
			// in terms of new frame gives position in uv frame.
			float upos = offset * s1;
			float vpos = offset * s2;

			// Crossed plane outside bounce region if !(0<=[uv]pos<=1)
			if (upos < 0 || upos > 1 ||
				vpos < 0 || vpos > 1)
				continue;

			// A hit! A most palatable hit!

			// Compute tangential and normal components of velocity
			float scale = m.vel * position.p2;
			pVector vn(position.p2 * scale); // Normal Vn = (V.N)N
			pVector vt(m.vel - vn); // Tangent Vt = V - Vn

			// Don't apply friction if tangential velocity < cutoff
			float appliedFriction = oneMinusFriction;
			if (cutoffSqr > 0) {
				float vtmag = vt.length2();

				if (vtmag <= cutoffSqr)
					appliedFriction = 1;
			}

			// Compute new velocity heading out:
			// V' = (1-mu) Vt - resilience Vn
			m.vel = vt * appliedFriction - vn * resilience;
		}
	}
	else if(position.type == PDSphere)
	{
		// Sphere that particles bounce off
		// The particles are always forced out of the sphere.
		for (int i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];

			// See if particle's next position is inside domain. If so,
			// bounce it.
			pVector pnext(m.pos + m.vel * dt);

			if (position.Within(pnext)) {
				// See if we were inside on previous timestep.
				bool pinside = position.Within(m.pos);

				// Normal to surface. This works for a sphere. Isn't
				// computed quite right, should extrapolate particle
				// position to surface.
				pVector n(m.pos - position.p1);
				n.normalize();

				// Compute tangential and normal components of velocity
				float scale = m.vel * n;

				pVector vn(n * scale); // Normal Vn = (V.N)N
				pVector vt = m.vel - vn; // Tangent Vt = V - Vn

				if (pinside) {
					// Previous position was inside. If normal component of
					// velocity points in, reverse it. This effectively
					// repels particles which would otherwise be trapped
					// in the sphere.
					if (scale < 0)
						m.vel = vt - vn;
				} else {
					// Previous position was outside -> particle crossed
					// surface boundary. Reverse normal component of velocity,
					// and apply friction (if Vt >= cutoff) and resilience.

					// Don't apply friction if tangential velocity < cutoff
					float appliedFriction = oneMinusFriction;
					if (cutoffSqr > 0) {
						float vtmag = vt.length2();

						if (vtmag <= cutoffSqr)
							appliedFriction = 1;
					}

					// Compute new velocity heading out:
					// V' = (1-mu) Vt - resilience Vn
					m.vel = vt * appliedFriction - vn * resilience;
				}
			}
		}
	}
}

// Set the secondary position of each particle to be its position.
void PACopyVertexB::Execute(ParticleGroup *group)
{
	int i;

	if(copy_pos)
		for (i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			m.posB = m.pos;
		}

	if(copy_vel)
		for (i = 0; i < group->p_count; i++)
		{
			Particle &m = group->list[i];
			m.velB = m.vel;
		}
}

// Dampen velocities
void PADamping::Execute(ParticleGroup *group)
{
	// This is important if dt is != 1.
	pVector one(1,1,1);
	pVector scale(one - ((one - damping) * dt));

	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];
		float vSqr = m.vel.length2();

		if (vSqr >= vlowSqr && vSqr <= vhighSqr) {
			m.vel.x *= scale.x;
			m.vel.y *= scale.y;
			m.vel.z *= scale.z;
		}
	}
}

// Exert force on each particle away from explosion center, iff
// shock wave is still going on at that particle's location.
void PAExplosion::Execute(ParticleGroup *group)
{
	if(age < 0) {
		// This special case computes center of mass of the
		// ParticleGroup to be the center of the explosion.

		center = pVector(0,0,0);
		for (int i = 0; i < group->p_count; i++) {
			Particle &m = group->list[i];
			center += m.pos;
		}

		center /= float(group->p_count);
		age = -age;
	}

	float mdt = magnitude * dt;
	float oneOverVelSqr = 1.0f / (velocity * velocity);
	float ageSqr = age * age;
	float agemlSqr = fsqr(age - lifetime);
	if(age - lifetime < 0)
		agemlSqr = 0;

	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		// Figure direction to particle.
		pVector dir(m.pos - center);
		float distSqr = dir.length2();

		// Time lag from start of explosion to when the
		// blast front reaches a particle dist units away.
		float delaySqr = distSqr * oneOverVelSqr;

		// See if the particle is in the propagating
		// shock wave of the explosion.
		if (delaySqr <= agemlSqr || delaySqr >= ageSqr)
			continue;

		// Force should really be more complicated than
		// a square wave, but this will do for now.
		m.vel += dir * (mdt / (sqrtf(distSqr) * (distSqr + epsilon)));
	}

	// Age the explosion. This is irrelevant for immediate mode.
	age += dt;
}

// Follow the next particle in the list
void PAFollow::Execute(ParticleGroup *group)
{
	float mdt = grav * dt;

	for (int i = 0; i < group->p_count - 1; i++) {
		Particle &m = group->list[i];

		// Accelerate toward the particle after me in the list.
		pVector vec01(group->list[i+1].pos - m.pos); // vec01 = p1 - p0
		float vec01lenSqr = vec01.length2();

		// Compute force exerted between the two bodies
		//m.vel += vec01 * (mdt / (sqrtf(vec01lenSqr) * (vec01lenSqr + epsilon)));
		m.vel += vec01 * (mdt / (sqrtf(vec01lenSqr) * (vec01lenSqr + epsilon)));
	}
}

// Inter-particle gravitation
// XXX Not fully tested.
void PAGravitate::Execute(ParticleGroup *group)
{
	float mdt = grav * dt;

	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		// Add interactions with other particles
		for (int j = i + 1; j < group->p_count; j++) {
			Particle &mj = group->list[j];

			pVector vec01(mj.pos - m.pos); // vec01 = p1 - p0
			float vec01lenSqr = vec01.length2();

			// Compute force exerted between the two bodies
			pVector acc(vec01 * (mdt / (sqrtf(vec01lenSqr) * (vec01lenSqr + epsilon))));

			m.vel += acc;
			mj.vel -= acc;
		}
	}
}

// Acceleration in a constant direction
void PAGravity::Execute(ParticleGroup *group)
{
	pVector ddir(direction * dt);

	for (int i = 0; i < group->p_count; i++) {
		// Step velocity with acceleration
		group->list[i].vel += ddir;
	}
}

// Accelerate particles along a line
void PAJet::Execute(ParticleGroup *group)
{
	float mdt = grav * dt;
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		// Figure direction to particle.
		pVector dir(m.pos - center);

		// Distance to jet (force drops as 1/r^2)
		// Soften by epsilon to avoid tight encounters to infinity
		float rSqr = dir.length2();

		if(rSqr < maxRadiusSqr)
		{
			pVector accel;
			acc.Generate(accel);

			// Step velocity with acceleration
			m.vel += accel * (mdt / (rSqr + epsilon));
		}
	}
}

// Get rid of older particles
void PAKillOld::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for (int i = group->p_count-1; i >= 0; i--) {
		Particle &m = group->list[i];

		if ((m.age < ageLimit) != kill_less_than)
			continue;
		group->Remove(i);
	}
}

// Get rid of slower particles
void PAKillSlow::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for (int i = group->p_count-1; i >= 0; i--) {
		Particle &m = group->list[i];

		if ((m.vel.length2() < speedLimitSqr) == kill_less_than)
			group->Remove(i);
	}
}

void PAMove::Execute(ParticleGroup *group)
{
	// Step particle positions forward by dt, and age the particles.
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		m.age += dt;
		m.pos += m.vel * dt;
	}

	// And finally, tick this particle group's clock.
	group->simTime += dt;
}

// Accelerate particles towards a line
void PAOrbitLine::Execute(ParticleGroup *group)
{
	float mdt = grav * dt;
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		// Figure direction to particle from base of line.
		pVector f(m.pos - p);

		pVector w(axis * (f * axis));

		// Direction from particle to nearest point on line.
		pVector into = w - f;

		// Distance to line (force drops as 1/r^2, normalize by 1/r)
		// Soften by epsilon to avoid tight encounters to infinity
		float rSqr = into.length2();

		if(rSqr < maxRadiusSqr)
			// Step velocity with acceleration
			m.vel += into * (mdt / (sqrtf(rSqr) + (rSqr + epsilon)));
	}
}

// Accelerate particles towards a point
void PAOrbitPoint::Execute(ParticleGroup *group)
{
	float mdt = grav * dt;
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		// Figure direction to particle.
		pVector dir(center - m.pos);

		// Distance to gravity well (force drops as 1/r^2, normalize by 1/r)
		// Soften by epsilon to avoid tight encounters to infinity
		float rSqr = dir.length2();

		if(rSqr < maxRadiusSqr)
			// Step velocity with acceleration
			m.vel += dir * (mdt / (sqrtf(rSqr) + (rSqr + epsilon)));
	}
}

// Accelerate in random direction each time step
void PARandomAccel::Execute(ParticleGroup *group)
{
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		pVector acceleration;
		gen_acc.Generate(acceleration);

		// dt will affect this by making a higher probability of
		// being near the original velocity after unit time. Smaller
		// dt approach a normal distribution instead of a square wave.
		m.vel += acceleration * dt;
	}
}

// Immediately displace position randomly
void PARandomDisplace::Execute(ParticleGroup *group)
{
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		pVector displacement;
		gen_disp.Generate(displacement);

		// dt will affect this by making a higher probability of
		// being near the original position after unit time. Smaller
		// dt approach a normal distribution instead of a square wave.
		m.pos += displacement * dt;
	}
}

// Immediately assign a random velocity
void PARandomVelocity::Execute(ParticleGroup *group)
{
	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		pVector velocity;
		gen_vel.Generate(velocity);

		// Shouldn't multiply by dt because velocities are
		// invariant of dt. How should dt affect this?
		m.vel = velocity;
	}
}

#if 0
// Produce coefficients of a velocity function v(t)=at^2 + bt + c
// satisfying initial x(0)=x0,v(0)=v0 and desired x(t)=xf,v(t)=vf,
// where x = x(0) + integrate(v(T),0,t)
static inline void _pconstrain(float x0, float v0, float xf, float vf,
							   float t, float *a, float *b, float *c)
{
	*c = v0;
	*b = 2 * ( -t*vf - 2*t*v0 + 3*xf - 3*x0) / (t * t);
	*a = 3 * ( t*vf + t*v0 - 2*xf + 2*x0) / (t * t * t);
}
#endif

// Over time, restore particles to initial positions
// Put all particles on the surface of a statue, explode the statue,
// and then suck the particles back to the original position. Cool!
void PARestore::Execute(ParticleGroup *group)
{
	if (timeLeft <= 0) {
		for (int i = 0; i < group->p_count; i++) {
			Particle &m = group->list[i];

			// Already constrained, keep it there.
			m.pos = m.posB;
			m.vel = pVector(0,0,0);
		}
	} else {
		float t = timeLeft;
		float dtSqr = dt * dt;
		float tSqrInv2dt = dt * 2.0f / (t * t);
		float tCubInv3dtSqr = dtSqr * 3.0f / (t * t * t);

		for (int i = 0; i < group->p_count; i++) {
#if 1
			Particle &m = group->list[i];

			// Solve for a desired-behavior velocity function in each axis
			// _pconstrain(m.pos.x, m.vel.x, m.posB.x, 0., timeLeft, &a, &b, &c);

			// Figure new velocity at next timestep
			// m.vel.x = a * dtSqr + b * dt + c;

			float b = (-2*t*m.vel.x + 3*m.posB.x - 3*m.pos.x) * tSqrInv2dt;
			float a = (t*m.vel.x - m.posB.x - m.posB.x + m.pos.x + m.pos.x) * tCubInv3dtSqr;

			// Figure new velocity at next timestep
			m.vel.x += a + b;

			b = (-2*t*m.vel.y + 3*m.posB.y - 3*m.pos.y) * tSqrInv2dt;
			a = (t*m.vel.y - m.posB.y - m.posB.y + m.pos.y + m.pos.y) * tCubInv3dtSqr;

			// Figure new velocity at next timestep
			m.vel.y += a + b;

			b = (-2*t*m.vel.z + 3*m.posB.z - 3*m.pos.z) * tSqrInv2dt;
			a = (t*m.vel.z - m.posB.z - m.posB.z + m.pos.z + m.pos.z) * tCubInv3dtSqr;

			// Figure new velocity at next timestep
			m.vel.z += a + b;
#else
			Particle &m = group->list[i];

			// XXX Optimize this.
			// Solve for a desired-behavior velocity function in each axis
			float a, b, c; // Coefficients of velocity function needed

			_pconstrain(m.pos.x, m.vel.x, m.posB.x, 0.,
				timeLeft, &a, &b, &c);

			// Figure new velocity at next timestep
			m.vel.x = a * dtSqr + b * dt + c;

			_pconstrain(m.pos.y, m.vel.y, m.posB.y, 0.,
				timeLeft, &a, &b, &c);

			// Figure new velocity at next timestep
			m.vel.y = a * dtSqr + b * dt + c;

			_pconstrain(m.pos.z, m.vel.z, m.posB.z, 0.,
				timeLeft, &a, &b, &c);

			// Figure new velocity at next timestep
			m.vel.z = a * dtSqr + b * dt + c;

#endif
		}
	}

	timeLeft -= dt;
}

// Kill particles on wrong side of sphere
void PASink::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for (int i = group->p_count-1; i >= 0; i--) {
		Particle &m = group->list[i];

		// Remove if inside/outside flag matches object's flag
		if (position.Within(m.pos) == kill_inside) {
			group->Remove(i);
		}
	}
}

// Kill particles on wrong side of sphere
void PASinkVelocity::Execute(ParticleGroup *group)
{
	// Must traverse list in reverse order so Remove will work
	for (int i = group->p_count-1; i >= 0; i--) {
		Particle &m = group->list[i];

		// Remove if inside/outside flag matches object's flag
		if (velocity.Within(m.vel) == kill_inside) {
			group->Remove(i);
		}
	}
}

// Randomly add particles to the system
void PASource::Execute(ParticleGroup *group)
{
	int rate = int(floor(particleRate * dt));

	// Dither the fraction particle in time.
	if(drand48() < particleRate * dt - float(rate))
		rate++;

	// Don't emit more than it can hold.
	if(group->p_count + rate > group->max_particles)
		rate = group->max_particles - group->p_count;

	pVector pos, posB, vel, col;

	if(vertexB_tracks)
	{
		for (int i = 0; i < rate; i++)
		{
			position.Generate(pos);
			velocity.Generate(vel);
			color.Generate(col);

			float size;
			if(size1 == size2)
				size = size1;
			else
				size = drand48() * (size2 - size1) + size1;
			group->Add(pos, pos, vel, col, size, alpha, age);
		}
	}
	else
	{
		for (int i = 0; i < rate; i++)
		{
			position.Generate(pos);
			positionB.Generate(posB);
			velocity.Generate(vel);
			color.Generate(col);

			float size;
			if(size1 == size2)
				size = size1;
			else
				size = drand48() * (size2 - size1) + size1;
			group->Add(pos, posB, vel, col, size, alpha, age);
		}
	}
}

// Shift color towards a specified value
void PATargetColor::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;

	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];
		m.color += (color - m.color) * scaleFac;
		m.alpha += (alpha - m.alpha) * scaleFac;
	}
}

// Shift size towards a specified value
void PATargetSize::Execute(ParticleGroup *group)
{
	float scaleFac = scale * dt;

	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];
		m.size += (destSize - m.size) * scaleFac;
	}
}

// Immediately displace position using vortex
// Vortex tip at center, around axis, with magnitude
// and tightness exponent
void PAVortex::Execute(ParticleGroup *group)
{
	float magdt = magnitude * dt;

	for (int i = 0; i < group->p_count; i++) {
		Particle &m = group->list[i];

		// Vector from tip of vortex
		pVector offset(m.pos - center);

		// Compute distance from particle to tip of vortex.
		float r = offset.length();

		// Don't do anything to particle if too close or too far.
		if (r < P_EPS || r > maxRadius)
			continue;

		// Compute normalized offset vector.
		pVector offnorm(offset / r);

		// Construct orthogonal vector frame in which to rotate
		// transformed point around origin
		float axisProj = offnorm * axis; // offnorm . axis

		// Components of offset perpendicular and parallel to axis
		pVector w(axis * axisProj); // parallel component
		pVector u(offnorm - w); // perpendicular component

		// Perpendicular component completing frame:
		pVector v(axis ^ u);

		// Figure amount of rotation
		// Resultant is (cos theta) u + (sin theta) v
		float theta = magdt / powf(r, tightness);
		float s = sinf(theta);
		float c = cosf(theta);

		offset = (u * c + v * s + w) * r;

		// Translate back to object space
		m.pos = offset + center;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Stuff for the pDomain.

// To offset [0 .. 1] vectors to [-.5 .. .5]
static pVector vHalf(0.5, 0.5, 0.5);

static inline pVector RandVec()
{
	return pVector(drand48(), drand48(), drand48());
}

// Return a random number with a normal distribution.
static inline float NRand(float sigma = 1.0f)
{
#define ONE_OVER_SIGMA_EXP (1.0f / 0.7975f)

	float y;
	do
	{
		y = -logf(drand48());
	}
	while(drand48() > expf(-fsqr(y - 1.0f)*0.5f));

	if(rand() & 0x1)
		return y * sigma * ONE_OVER_SIGMA_EXP;
	else
		return -y * sigma * ONE_OVER_SIGMA_EXP;
}

pDomain::pDomain(PDomainEnum dtype, float a0, float a1,
				 float a2, float a3, float a4, float a5,
				 float a6, float a7, float a8)
{
	type = dtype;
	switch(type)
	{
	case PDPoint:
		p1 = pVector(a0, a1, a2);
		break;
	case PDLine:
		{
			p1 = pVector(a0, a1, a2);
			pVector tmp(a3, a4, a5);
			// p2 is vector from p1 to other endpoint.
			p2 = tmp - p1;
		}
		break;
	case PDBox:
		// p1 is the min corner. p2 is the max corner.
		if(a0 < a3) {
			p1.x = a0; p2.x = a3;
		} else {
			p1.x = a3; p2.x = a0;
		}
		if(a1 < a4) {
			p1.y = a1; p2.y = a4;
		} else {
			p1.y = a4; p2.y = a1;
		}
		if(a2 < a5) {
			p1.z = a2; p2.z = a5;
		} else {
			p1.z = a5; p2.z = a2;
		}
		break;
	case PDTriangle:
		{
			p1 = pVector(a0, a1, a2);
			pVector tp2 = pVector(a3, a4, a5);
			pVector tp3 = pVector(a6, a7, a8);

			u = tp2 - p1;
			v = tp3 - p1;

			// The rest of this is needed for bouncing.
			radius1Sqr = u.length();
			pVector tu = u / radius1Sqr;
			radius2Sqr = v.length();
			pVector tv = v / radius2Sqr;

			p2 = tu ^ tv; // This is the non-unit normal.
			p2.normalize(); // Must normalize it.

			// radius1 stores the d of the plane eqn.
			radius1 = -(p1 * p2);
		}
		break;
	case PDPlane:
		{
			p1 = pVector(a0, a1, a2);
			u = pVector(a3, a4, a5);
			v = pVector(a6, a7, a8);

			// The rest of this is needed for bouncing.
			radius1Sqr = u.length();
			pVector tu = u / radius1Sqr;
			radius2Sqr = v.length();
			pVector tv = v / radius2Sqr;

			p2 = tu ^ tv; // This is the non-unit normal.
			p2.normalize(); // Must normalize it.

			// radius1 stores the d of the plane eqn.
			radius1 = -(p1 * p2);
		}
		break;
	case PDSphere:
		p1 = pVector(a0, a1, a2);
		if(a3 > a4) {
			radius1 = a3; radius2 = a4;
		} else {
			radius1 = a4; radius2 = a3;
		}
		radius1Sqr = radius1 * radius1;
		radius2Sqr = radius2 * radius2;
		break;
	case PDCone:
	case PDCylinder:
		{
			// p2 is a vector from p1 to the other end of cylinder.
			// p1 is apex of cone.

			p1 = pVector(a0, a1, a2);
			pVector tmp(a3, a4, a5);
			p2 = tmp - p1;

			if(a6 > a7) {
				radius1 = a6; radius2 = a7;
			} else {
				radius1 = a7; radius2 = a6;
			}
			radius1Sqr = radius1 * radius1;

			// Given an arbitrary nonzero vector n, form two orthonormal
			// vectors u and v forming a frame [u,v,n.normalize()].
			pVector n = p2;
			float p2l2 = n.length2(); // Optimize this.
			n.normalize();

			// radius2Sqr stores 1 / (p2.p2)
			if(p2l2 != 0.0f)
				radius2Sqr = 1.0f / p2l2;
			else
				radius2Sqr = 0.0f;

			// Find a vector with an orthogonal component to n.
			pVector basis(1.0f, 0.0f, 0.0f);
			if (1.0f - fabs(basis * n) < 1e-5f)
				basis = pVector(0.0f, 1.0f, 0.0f);

			// Project away N component, normalize and cross to get
			// second orthonormal vector.
			u = basis - n * (basis * n);
			u.normalize();
			v = n ^ u;
		}
		break;
	case PDBlob:
		p1 = pVector(a0, a1, a2);
		radius1 = a3;
		radius1Sqr = fsqr(radius1);
		radius2Sqr = -0.5f/radius1Sqr;
		radius2 = 1.0f * sqrtf(2.0f * float(M_PI) * radius1Sqr);
		break;
	}
}

// Determines if 'pos' is inside the box domain d
bool pDomain::Within(const pVector &pos) const
{
	switch (type) {
	case PDPoint:
		return false; // Even less likely!
	case PDLine:
		return false; // It's very hard to lie on a line.
	case PDBox:
		if ((pos.x < p1.x) || (pos.x > p2.x) ||
			(pos.y < p1.y) || (pos.y > p2.y) ||
			(pos.z < p1.z) || (pos.z > p2.z))
			return false; // outside

		return true; // inside
	case PDTriangle:
		return false; // XXX Is there something better?
	case PDPlane:
		// Distance from plane = n * p + d
		// Inside is the positive half-space.
		return pos * p2 >= -radius1;
	case PDSphere:
		{
			pVector rvec(pos - p1);
			float rSqr = rvec.length2();
			return rSqr <= radius1Sqr && rSqr >= radius2Sqr;
		}
	case PDCylinder:
	case PDCone:
		{
			// This is painful and slow. Might be better to do quick
			// accept/reject tests.
			// Let p2 = vector from base to tip of the cylinder
			// x = vector from base to test point
			//        x . p2
			// dist = ------ = projected distance of x along the axis
			//        p2. p2   ranging from 0 (base) to 1 (tip)
			//
			// rad = x - dist * p2 = projected vector of x along the base
			// p1 is the apex of the cone.

			pVector x(pos - p1);

			// Check axial distance
			// radius2Sqr stores 1 / (p2.p2)
			float dist = (p2 * x) * radius2Sqr;
			if (dist < 0.0f || dist > 1.0f)
				return false;

			// Check radial distance; scale radius along axis for cones
			pVector xrad = x - p2 * dist; // Radial component of x
			float rSqr = xrad.length2();

			if (type == PDCone)
				return (rSqr <= fsqr(dist * radius1) &&
				rSqr >= fsqr(dist * radius2));
			else
				return (rSqr <= radius1Sqr && rSqr >= fsqr(radius2));
		}
	case PDBlob:
		{
			pVector x(pos - p1);
			float Gx = expf(x.length2() * radius2Sqr) * radius2;
			return (drand48() < Gx);
		}
	default:
		return false;
	}
}

// Generate a random point uniformly distrbuted within the domain
void pDomain::Generate(pVector &pos) const
{
	switch (type)
	{
	case PDPoint:
		pos = p1;
		break;
	case PDLine:
		pos = p1 + p2 * drand48();
		break;
	case PDBox:
		// Scale and translate [0,1] random to fit box
		pos.x = p1.x + (p2.x - p1.x) * drand48();
		pos.y = p1.y + (p2.y - p1.y) * drand48();
		pos.z = p1.z + (p2.z - p1.z) * drand48();
		break;
	case PDTriangle:
		{
			float r1 = drand48();
			float r2 = drand48();
			if(r1 + r2 < 1.0f)
				pos = p1 + u * r1 + v * r2;
			else
				pos = p1 + u * (1.0f-r1) + v * (1.0f-r2);
		}
		break;
	case PDPlane:
		pos = p1 + u * drand48() + v * drand48();
		break;
	case PDSphere:
		// Place on [-1..1] sphere
		pos = RandVec() - vHalf;
		pos.normalize();

		// Scale unit sphere pos by [0..r] and translate
		// (should distribute as r^2 law)
		if(radius1 == radius2)
			pos = p1 + pos * radius1;
		else
			pos = p1 + pos * (radius2 + drand48() * (radius1 - radius2));
		break;
	case PDCylinder:
	case PDCone:
		{
			// For a cone, p2 is the apex of the cone.
			float dist = drand48(); // Distance between base and tip
			float theta = drand48() * 2.0f * float(M_PI); // Angle around base sphere
			// Distance from axis
			float r = radius2 + drand48() * (radius1 - radius2);

			float x = r * cosf(theta); // Weighting of each frame vector
			float y = r * sinf(theta);

			// Scale radius along axis for cones
			if (type == PDCone) {
				x *= dist;
				y *= dist;
			}

			pos = p1 + p2 * dist + u * x + v * y;
		}
		break;
	case PDBlob:
		pos.x = p1.x + NRand(radius1);
		pos.y = p1.y + NRand(radius1);
		pos.z = p1.z + NRand(radius1);

		break;
	default:
		pos = pVector(0,0,0);
	}
}



//****************************************
//
// action_api.cpp
//
//****************************************


// Do not call this function.
static void _pSendAction(ParticleAction *S, PActionEnum type, int size)
{
	S->type = type;

	if(_ps.in_new_list)
	{
		_pAddActionToList(S, size);
	}
	else
	{
		// Immediate mode. Execute it.
		// This is a hack to give them local access to dt.
		S->dt = _ps.dt;
		_pCallActionList(S, 1, _ps.pgrp);
	}
}

void pBounce(float friction, float resilience, float cutoff,
			 PDomainEnum dtype,
			 float a0, float a1, float a2,
			 float a3, float a4, float a5,
			 float a6, float a7, float a8)
{
	PABounce S;

	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	S.oneMinusFriction = 1.0f - friction;
	S.resilience = resilience;
	S.cutoffSqr = fsqr(cutoff);

	_pSendAction(&S, PABounceID, sizeof(PABounce));
}

void pCopyVertexB(bool copy_pos, bool copy_vel)
{
	PACopyVertexB S;

	S.copy_pos = copy_pos;
	S.copy_vel = copy_vel;

	_pSendAction(&S, PACopyVertexBID, sizeof(PACopyVertexB));
}

void pDamping(float damping_x, float damping_y, float damping_z,
			  float vlow, float vhigh)
{
	PADamping S;

	S.damping = pVector(damping_x, damping_y, damping_z);
	S.vlowSqr = fsqr(vlow);
	S.vhighSqr = fsqr(vhigh);

	_pSendAction(&S, PADampingID, sizeof(PADamping));
}

void pExplosion(float center_x, float center_y, float center_z,
				float velocity, float magnitude, float lifetime,
				float epsilon, float age)
{
	PAExplosion S;

	S.center = pVector(center_x, center_y, center_z);
	S.velocity = velocity;
	S.magnitude = magnitude;
	S.lifetime = lifetime;
	S.age = age;
	S.epsilon = epsilon;

	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;

	_pSendAction(&S, PAExplosionID, sizeof(PAExplosion));
}

void pFollow(float grav, float epsilon)
{
	PAFollow S;

	S.grav = grav;
	S.epsilon = epsilon;

	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;

	_pSendAction(&S, PAFollowID, sizeof(PAFollow));
}

void pGravitate(float grav, float epsilon)
{
	PAGravitate S;

	S.grav = grav;
	S.epsilon = epsilon;

	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;

	_pSendAction(&S, PAGravitateID, sizeof(PAGravitate));
}

void pGravity(float dir_x, float dir_y, float dir_z)
{
	PAGravity S;

	S.direction = pVector(dir_x, dir_y, dir_z);

	_pSendAction(&S, PAGravityID, sizeof(PAGravity));
}

void pJet(float center_x, float center_y, float center_z,
		  float grav, float epsilon, float maxRadius)
{
	PAJet S;

	S.center = pVector(center_x, center_y, center_z);
	S.acc = _ps.Vel;
	S.grav = grav;
	S.epsilon = epsilon;
	S.maxRadiusSqr = fsqr(maxRadius);

	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;

	_pSendAction(&S, PAJetID, sizeof(PAJet));
}

void pKillOld(float ageLimit, bool kill_less_than)
{
	PAKillOld S;

	S.ageLimit = ageLimit;
	S.kill_less_than = kill_less_than;

	_pSendAction(&S, PAKillOldID, sizeof(PAKillOld));
}

void pKillSlow(float speedLimit, bool kill_less_than)
{
	PAKillSlow S;

	S.speedLimitSqr = fsqr(speedLimit);
	S.kill_less_than = kill_less_than;

	_pSendAction(&S, PAKillSlowID, sizeof(PAKillSlow));
}


void pMove()
{
	PAMove S;

	_pSendAction(&S, PAMoveID, sizeof(PAMove));
}

void pOrbitLine(float p_x, float p_y, float p_z,
				float axis_x, float axis_y, float axis_z,
				float grav, float epsilon, float maxRadius)
{
	PAOrbitLine S;

	S.p = pVector(p_x, p_y, p_z);
	S.axis = pVector(axis_x, axis_y, axis_z);
	S.axis.normalize();
	S.grav = grav;
	S.epsilon = epsilon;
	S.maxRadiusSqr = fsqr(maxRadius);

	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;

	_pSendAction(&S, PAOrbitLineID, sizeof(PAOrbitLine));
}

void pOrbitPoint(float center_x, float center_y, float center_z,
				 float grav, float epsilon, float maxRadius)
{
	PAOrbitPoint S;

	S.center = pVector(center_x, center_y, center_z);
	S.grav = grav;
	S.epsilon = epsilon;
	S.maxRadiusSqr = fsqr(maxRadius);

	if(S.epsilon < 0.0f)
		S.epsilon = P_EPS;

	_pSendAction(&S, PAOrbitPointID, sizeof(PAOrbitPoint));
}

void pRandomAccel(PDomainEnum dtype,
				  float a0, float a1, float a2,
				  float a3, float a4, float a5,
				  float a6, float a7, float a8)
{
	PARandomAccel S;

	S.gen_acc = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);

	_pSendAction(&S, PARandomAccelID, sizeof(PARandomAccel));
}

void pRandomDisplace(PDomainEnum dtype,
					 float a0, float a1, float a2,
					 float a3, float a4, float a5,
					 float a6, float a7, float a8)
{
	PARandomDisplace S;

	S.gen_disp = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);

	_pSendAction(&S, PARandomDisplaceID, sizeof(PARandomDisplace));
}

void pRandomVelocity(PDomainEnum dtype,
					 float a0, float a1, float a2,
					 float a3, float a4, float a5,
					 float a6, float a7, float a8)
{
	PARandomVelocity S;

	S.gen_vel = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);

	_pSendAction(&S, PARandomVelocityID, sizeof(PARandomVelocity));
}

void pRestore(float timeLeft)
{
	PARestore S;

	S.timeLeft = timeLeft;

	_pSendAction(&S, PARestoreID, sizeof(PARestore));
}

void pSink(bool kill_inside, PDomainEnum dtype,
		   float a0, float a1, float a2,
		   float a3, float a4, float a5,
		   float a6, float a7, float a8)
{
	PASink S;

	S.kill_inside = kill_inside;
	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);

	_pSendAction(&S, PASinkID, sizeof(PASink));
}

void pSinkVelocity(bool kill_inside, PDomainEnum dtype,
				   float a0, float a1, float a2,
				   float a3, float a4, float a5,
				   float a6, float a7, float a8)
{
	PASinkVelocity S;

	S.kill_inside = kill_inside;
	S.velocity = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);

	_pSendAction(&S, PASinkVelocityID, sizeof(PASinkVelocity));
}

void pSource(float particleRate, PDomainEnum dtype,
			 float a0, float a1, float a2,
			 float a3, float a4, float a5,
			 float a6, float a7, float a8)
{
	PASource S;

	S.particleRate = particleRate;
	S.position = pDomain(dtype, a0, a1, a2, a3, a4, a5, a6, a7, a8);
	S.positionB = _ps.VertexB;
	S.size1 = _ps.Size1;
	S.size2 = _ps.Size2;
	S.velocity = _ps.Vel;
	S.color = _ps.Color;
	S.alpha = _ps.Alpha;
	S.age = _ps.Age;
	S.vertexB_tracks = _ps.vertexB_tracks;

	_pSendAction(&S, PASourceID, sizeof(PASource));
}

void pTargetColor(float color_x, float color_y, float color_z,
				  float alpha, float scale)
{
	PATargetColor S;

	S.color = pVector(color_x, color_y, color_z);
	S.alpha = alpha;
	S.scale = scale;

	_pSendAction(&S, PATargetColorID, sizeof(PATargetColor));
}

void pTargetSize(float destSize, float scale)
{
	PATargetSize S;

	S.destSize = destSize;
	S.scale = scale;

	_pSendAction(&S, PATargetSizeID, sizeof(PATargetSize));
}

// If in immediate mode, quickly add a vertex.
// If building an action list, call pSource.
void pVertex(float x, float y, float z)
{
	if(_ps.in_new_list)
	{
		pSource(1, PDPoint, x, y, z);
		return;
	}

	// Immediate mode. Quickly add the vertex.
	if(_ps.pgrp == NULL)
		return;

	pVector pos(x, y, z);
	pVector vel, col, posB;
	if(_ps.vertexB_tracks)
		posB = pos;
	else
		_ps.VertexB.Generate(posB);
	_ps.Vel.Generate(vel);
	_ps.Color.Generate(col);

	float size;
	if(_ps.Size1 == _ps.Size2)
		size = _ps.Size1;
	else
		size = drand48() * (_ps.Size2 - _ps.Size1) + _ps.Size1;

	_ps.pgrp->Add(pos, posB, vel, col, size, _ps.Alpha, _ps.Age);
}

void pVortex(float center_x, float center_y, float center_z,
			 float axis_x, float axis_y, float axis_z,
			 float magnitude, float tightness, float maxRadius)
{
	PAVortex S;

	S.center = pVector(center_x, center_y, center_z);
	S.axis = pVector(axis_x, axis_y, axis_z);
	S.axis.normalize();
	S.magnitude = magnitude;
	S.tightness = tightness;
	S.maxRadius = maxRadius;

	_pSendAction(&S, PAVortexID, sizeof(PAVortex));
}


//
// Particle.cpp
//




// A fountain spraying up in the middle of the screen
void Fountain(BOOL do_list)
{
	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocityD(PDCylinder, 0.0, -0.01, 0.35, 0.0, -0.01, 0.37, 0.021, 0.019);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);

	if(do_list)
		pNewActionList(action_handle);

	pCopyVertexB(false, true);

	pSource(150, PDLine, 0.0, 0.0, 0.401, 0.0, 0.0, 0.405);

	pGravity(0.0, 0.0, -0.01);

	pKillSlow(0.01);

	pBounce(-0.05, 0.35, 0, PDPlane, -5, -5, 0.0, 10, 0, 0, 0, 10, 0);

	pSink(false, PDPlane, 0, 0, -3, 1, 0, 0, 0, 1, 0);

	pMove();

	if(do_list)
		pEndActionList();
}

// A waterfall pouring down from above
void Waterfall(BOOL do_list)
{
	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocityD(PDBlob, 0.03, -0.001, 0.01, 0.002);
	pColorD(1.0, PDLine, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0);
	pSize(1.5);

	if(do_list)
		pNewActionList(action_handle);

	pCopyVertexB(false, true);

	pSource(50, PDLine, -5, -1, 8, -5, -3, 8);

	pGravity(0.0, 0.0, -0.01);

	pKillOld(300);

	pBounce(0, 0.35, 0, PDPlane, -7, -4, 7, 3, 0, 0, 0, 3, 0);

	pBounce(0, 0.5, 0, PDSphere, -4, -2, 4, 0.2);

	pBounce(0, 0.5, 0, PDSphere, -3.5, 0, 2, 2);

	pBounce(0, 0.5, 0, PDSphere, 3.8, 0, 0, 2);
	pBounce(-0.01, 0.35, 0, PDPlane, -25, -25, 0.0, 50, 0, 0, 0, 50, 0);

	pSink(false, PDSphere, 0,0,0,20);

	pMove();

	if(do_list)
		pEndActionList();
}


// Restore particles to their positionB,
// which is usually set by Shape().
void Restore(BOOL do_list)
{
	static float i = 0;
	if(do_list)
	{
		action_handle = pGenActionLists(1);
		pNewActionList(action_handle);
		i = 200;
	}

	pCopyVertexB(false, true);

	pGravity(0.0, 0.0, -0.001);

	pRestore(i-= (1.0f / float(numSteps)));

	pMove();

	if(do_list)
		pEndActionList();
}

// Make a bunch of particles in a particular shape.
void Shape(BOOL do_list)
{
	if(do_list)
	{
		pKillOld(0);
		int dim = int(pow(float(maxParticles), 0.33333333));
#define XX 8
#define YY 12
#define ZZ 8

		float dx = 2*XX / float(dim);
		float dy = 2*YY / float(dim);
		float dz = 2*ZZ / float(dim);

		float z = -ZZ;
		for(int k=0; k<dim; k++, z += dz)
		{
			float y = -YY;
			for(int l=0; l<dim; l++, y += dy)
			{
				float x = -XX;
				for(int j=0; j<dim; j++, x += dx)
				{
					// Make the particles.
					pVelocityD(PDBlob, 0, 0, 0, 0.001);
					pColor(0.5 + z*0.05, 0.5 + x*0.05, 0.5 + x*0.05);
					pVertex(x, y, z);

				}
			}
		}

		action_handle = pGenActionLists(1);
		pNewActionList(action_handle);
		pEndActionList();
	}
}


// A fountain spraying up in the middle of the screen
void Atom(BOOL do_list)
{
	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocityD(PDSphere, 0, 0, 0, 0.2);
	pColorD(1.0, PDBox, 0, 0, 0, 1.0, 0.5, 0.5);
	pSize(1.5);

	if(do_list)
		pNewActionList(action_handle);

	pCopyVertexB(false, true);

	pSource(300, PDSphere, 0, 0, 0, 6);

	// Orbit about the origin.
	pOrbitPoint(0, 0, 0, 0.05);

	// Keep orbits from being too eccentric.
	pSink(true, PDSphere, 0.0, 0.0, 0.0, 1.0);
	pSink(false, PDSphere, 0.0, 0.0, 0.0, 8.0);

	pMove();

	if(do_list)
		pEndActionList();
}

void JetSpray(BOOL do_list)
{
	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocity(0, 0, 0);
	pSize(1.5);

	if(do_list)
		pNewActionList(action_handle);

	pCopyVertexB(false, true);

	pColorD(1.0, PDSphere, .8, .4, .1, .1);
	pSource(1, PDPlane, -1, -1, 0.1, 2, 0, 0, 0, 2, 0);

	pColorD(1.0, PDSphere, .5, .4, .1, .1);
	pSource(300, PDPlane, -10, -10, 0.1, 20, 0, 0, 0, 20, 0);

	pGravity(0, 0, -0.01);

	static float jetx=0, jety=0, jetz=0;
	static float djx = drand48() * 0.5;
	static float djy = drand48() * 0.5;

	if(do_list)
	{
		jetx = 0;
		jety = 0;
		djx = drand48() * 0.5;
		djy = drand48() * 0.5;
	}

	jetx += djx;
	jety += djy;

	if(jetx > 10 || jetx < -10) {djx = -djx; djy += drand48() * 0.005;}
	if(jety > 10 || jety < -10) {djy = -djy; djx += drand48() * 0.005;}

	pVelocityD(PDBlob, 0,0,.05, 0.01);
	pJet(jetx, jety, jetz, 1, 0.01, 1.5);

	pBounce(0.3, 0.3, 0, PDPlane, -10, -10, 0.0, 20, 0, 0, 0, 20, 0);

	pSink(false, PDPlane, -10, -10, -20.0, 20, 0, 0, 0, 20, 0);

	pMove();

	if(do_list)
		pEndActionList();
}


void Explosion(BOOL do_list)
{
	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocityD(PDSphere, 0,0,0,0.01,0.01);
	pColorD(1.0, PDSphere, 0.5, 0.7, 0.5, .3);
	pSize(1.0);

	if(do_list)
		pNewActionList(action_handle);

	pCopyVertexB(false, true);

	pDamping(0.999, 0.999, 0.999);

	static float i=0;

	if(do_list)
		i = 0;

	pOrbitPoint(0, 0, 0, .02, 0.1);

	pExplosion(0, 0, 0, 1, 2, 3, 0.1, i+= (1.0f / float(numSteps)));

	pSink(false, PDSphere, 0, 0, 0, 30);

	pMove();

	if(do_list)
		pEndActionList();
}

void Swirl( BOOL do_list ) {

	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);

	if(do_list)
		pNewActionList(action_handle);

	static float jetx=-4, jety=0, jetz=-2.4;

	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;

	jetx += djx;
	jety += djy;
	jetz += djz;

	if(jetx > 10 || jetx < -10) djx = -djx;
	if(jety > 10 || jety < -10) djy = -djy;
	if(jetz > 10 || jetz < -10) djz = -djz;

	pCopyVertexB(false, true);

	int LifeTime = 300;

	pKillOld(LifeTime);

	pColorD(1.0, PDSphere, 0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1), 0.1);
	pSource(maxParticles / LifeTime, PDPoint, jetx, jety, jetz);

	pOrbitLine(0, 0, 1, 1, 0, 0.0, 0.1, 0.1, 99);

	pDamping(1, 0.994, 0.994);

	pSink(false, PDSphere, 0, 0, 0, 15);

	pMove();

	if(do_list)
		pEndActionList();
}



void Snake(BOOL do_list)
{
	if(do_list)
	{
		action_handle = pGenActionLists(1);

		pVelocity(0, 0, 0);
		pSize(1.0);
		pStartingAge(0);

		pKillOld(-300);
		pColorD(1.0, PDSphere, 0.93, 0.93, 0, 0.05);
		for(float x=-10.0; x<2.0; x+=0.05)
			pVertex(x, 0, 0);
		pNewActionList(action_handle);
	}
	static float jetx=0, jety=0, jetz=0;

	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;

	jetx += djx;
	jety += djy;
	jetz += djz;

	if(jetx > 6 || jetx < -6) djx = -djx;
	if(jety > 6 || jety < -6) djy = -djy;
	if(jetz > 6 || jetz < -6) djz = -djz;

	pCopyVertexB(false, true);

	pKillOld(-1.0, true);
	pStartingAge(-10.0);
	pColor(1, 0, 0);
	pVertex(jetx, jety, jetz);
	pStartingAge(0);

	// Either of these gives an interesting effect.
	pFollow(0.01, 1.0);
	// pGravitate(0.01, 1.0);

	pDamping(0.9, 0.9, 0.9);

	pMove();

	if(do_list)
		pEndActionList();
}

// A fountain spraying up in the middle of the screen
void FireFlies(BOOL do_list)
{
	if(do_list)
		action_handle = pGenActionLists(1);

	pSize(1.0);
	pVelocityD(PDPoint, 0,0,0);
	pColorD(1.0, PDSphere, .5, .4, .1, .1);

	if(do_list)
		pNewActionList(action_handle);

	pCopyVertexB(false, true);

	pSource(1, PDBlob, 0, 0, 2, 2);

	pRandomAccel(PDSphere, 0, 0, 0, 0.02);

	pKillOld(20);

	pMove();

	if(do_list)
		pEndActionList();
}

//
//
//
void Chaos( BOOL do_list ) {

	if(do_list)
		action_handle = pGenActionLists(1);

	pVelocityD(PDBlob, 0.02, -0.2, 0, 0.015);
	pSize(1.0);

	if(do_list)
		pNewActionList(action_handle);

	static float jetx=-4, jety=0, jetz=-2.4;

	static float djx = drand48() * 0.05;
	static float djy = drand48() * 0.05;
	static float djz = drand48() * 0.05;

	jetx += djx;
	jety += djy;
	jetz += djz;

	if(jetx > 10 || jetx < -10) djx = -djx;
	if(jety > 10 || jety < -10) djy = -djy;
	if(jetz > 10 || jetz < -10) djz = -djz;

	pCopyVertexB(false, true);

	int LifeTime = 350;

	pKillOld(LifeTime);

	pColorD(1.0, PDSphere, 0.4+fabs(jetx*0.1), 0.4+fabs(jety*0.1), 0.4+fabs(jetz*0.1), 0.1);
	pSource(maxParticles / LifeTime, PDPoint, jetx, jety, jetz);

	pOrbitPoint(2, 0, 3, 0.1, 0.1, 99);

	pOrbitPoint(-2, 0, -3, 0.1, 0.1, 99);

	pDamping(0.994, 0.994, 0.994);

	//pSink(false, PDSphere, 0, 0, 0, 15);

	pMove();

	if(do_list)
		pEndActionList();

	return;
}



static char emitterName[256] = "Prepare your anus!";


//
//
//
char *GetEmitterName( void ) {

	return emitterName;
}


//
// CallDemo
//
void DoEmitter( int demoNum, BOOL initial ) {

	/***
	if(!immediate && !initial)
		pCallActionList(action_handle);
		return;
	}
	***/

	switch(demoNum)
	{
	case 0:
		Fountain(initial);
		strcpy( emitterName, "Fountain" );
		break;
	case 1:
		Atom(initial);
		strcpy( emitterName, "Atom" );
		break;
	case 2:
		JetSpray(initial);
		strcpy( emitterName, "JetSpray" );
		break;
	case 3:
		Explosion(initial);
		strcpy( emitterName, "Explosion" );
		break;
	case 4:
		Swirl(initial);
		strcpy( emitterName, "Swirl" );
		break;
	case 5:
		FireFlies(initial);
		strcpy( emitterName, "FireFlies" );
		break;
	case 6:
		Waterfall(initial);
		strcpy( emitterName, "Waterfall" );
		break;
	case 7:
		Restore(initial);
		strcpy( emitterName, "Restore" );
		break;
	case 8:
		Shape(initial);
		strcpy( emitterName, "Shape" );
		break;
	case 9:
		Snake(initial);
		strcpy( emitterName, "Snake" );
		break;
	case 10:
		Chaos(initial);
		strcpy( emitterName, "Chaos" );
		break;
	default:
		strcpy( emitterName, "Null" );
		break;
	}

	if( initial == TRUE )
		xprintf("DoEmitter: initing \"%s\" emitter.\n", GetEmitterName() );

	return;
}


static int particleNum = 0;

//
//
//
int GetNumParticle( void ) {

	return particleNum;
}


//
// Emit OpenGL calls to draw the particles. These are drawn with
// whatever primitive type the user specified(GL_POINTS, for
// example). The color and radius are set per primitive, by default.
// For GL_LINES, the other vertex of the line is the velocity vector.
//
int DrawParticle( void ) {

	int i;

	for( i=0; i<numSteps; i++ )
		DoEmitter( demoNum, FALSE );

	//int count = pGetGroupCount();
	//cerr << count << endl;

	ParticleGroup *pg = _ps.pgrp;
	if( pg == NULL )
		return -1; // ERROR

	particleNum = pg->p_count;

	if( pg->p_count < 1 )
		return -2;

	rgb_t color;
	point3_t p[2];
	point2_t t[2];

	// static int frame_cnt = 0;
	// ++frame_cnt;

	for( i=0; i<particleNum; i++ ) {

		Particle &m = pg->list[i];

		// MY_cam_pos = { 10, 10, 10 };
		// gluLookAt(0, t, 10, 0, 0, 0, 0, 0, 1);
		// cam_pos = { 0, -23, 10 };

		color.r = (int)(m.color.x * 255.0f);
		color.g = (int)(m.color.y * 255.0f);
		color.b = (int)(m.color.z * 255.0f);
		color.a = (int)(m.alpha * 220.0f);

		p[0][0] = m.pos.x;
		p[0][1] = m.pos.y;
		p[0][2] = m.pos.z;

		// For lines, make a tail with the velocity vector's direction and
		// a length of radius.
		pVector tail(-m.vel.x, -m.vel.y, -m.vel.z);
		tail *= (m.size*4);
		tail += m.pos;

		p[1][0] = tail.x;
		p[1][1] = tail.y;
		p[1][2] = tail.z;

	#if 0
		TransformPoint( p[0], t[0] );
		TransformPoint( p[1], t[1] );

		PutPixel(t[0][0],t[0][1], color );
		PutPixel(t[1][0],t[1][1], color );
		// Line( t[0][0], t[0][1], t[1][0], t[1][1], color );
	#else

		if( TransformLine( p, t ) )
			Line( t[0][0], t[0][1], t[1][0], t[1][1], color );
	#endif

		/*
		static int frame_cnt = 0;
		if( !(frame_cnt % 100) )
			xprintf("pxl: [%.2f, %.2f, %0.2f] -> [%d,%d ; %d,%d]  (%d,%d,%d,%d)\n",
					p[0][0],p[0][1],p[0][2],
					t[0][0],t[0][1],
					t[1][0],t[1][1],
					color.r, color.g, color.b, color.a );
		++frame_cnt;
		*/


	}

	return particleNum;
}



//
//
//
void ReshapeParticle( int w, int h ) {

	/***
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, w / double(h), 1, 100);
	glMatrixMode(GL_MODELVIEW);

	// Useful for motion blur so BG doesn't get ugly.
#ifdef DEPTH_TEST
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
		glClear(GL_COLOR_BUFFER_BIT);
#endif
	***/

	return;
}



//
//
//
void SetParticle(  int dnum, int amount ) {

	if( amount ) {
		maxParticles += amount;
		xprintf("SetParticle: max particle == %d\n", maxParticles );
	}

	if( dnum >= 0 && dnum <= 10 ) {
		demoNum = dnum;
		DoEmitter( demoNum, TRUE );
	}

	return;
}



//
//
//
int CreateParticle( void ) {

	if( particle_handle != -1 )
		return particle_handle;

	particle_handle = pGenParticleGroups( 1, maxParticles );

	pCurrentGroup( particle_handle );

	DoEmitter( demoNum, TRUE );

	return particle_handle;
}






//
//
//
void DoParticle( BOOL init, point3_t cam_pos, point3_t cam_view, BOOL fps_cnt ) {

	static ULONG tics = 0;
	static int index = 0, flip = 1;
	int table[] = { 10,3, 1,3, 4,3, 0,3 };
	int gamestate_tic = GetTic();

#define EXPLO ( table[index] == 3 )

	if( init == TRUE ) {
		CreateParticle();
		index = 0;
		SetParticle( table[index], 0 );	// Chaos
		tics = gamestate_tic;
		MAKEVECTOR( cam_view, 0.0, 0.0, 0.0 );
		MAKEVECTOR( cam_pos, 10.0, 10.0, 10.0 );
		xprintf( "DoParticle: cam coords arrre derrrty.\n");
	}
	else

	// klikk = explo
	if( mousebl && !EXPLO ) {
		tics = gamestate_tic;
		++index;
		index %= dimof(table);
		SetParticle( table[index], 0 );	// Explosion lesz tuti
	}
	else


	// ha má vége az explonak
	if( EXPLO && (tics+(TICKBASE*2) < gamestate_tic) ) {
		tics = gamestate_tic;
		++index;
		index %= dimof(table);
		SetParticle( table[index], 0 );
		if( flip > 0 ) flip = -1; else flip = 1;
	}
	else

	// normálisan léptet ha itt az ideje
	if( !EXPLO && (tics+(TICKBASE*30) < gamestate_tic)) {
		tics = gamestate_tic;
		++index;
		index %= dimof(table);
		SetParticle( table[index], 0 );
		if( flip > 0 ) flip = -1; else flip = 1;
	}


	// pörgés
	static int lastTime = 0;
	FLOAT elapsedTime = FLOAT_ZERO;
	FLOAT rotdz = FLOAT_ZERO;

	if( gamestate_tic > lastTime && !EXPLO ) {
		elapsedTime = ((FLOAT)(gamestate_tic - lastTime) / TICKBASE);
		rotdz = 30.0f * (FLOAT)flip * elapsedTime;
	}

	lastTime = gamestate_tic;

/***
	// emelkedés
#define ELETIME (TICKBASE * 20)
#define ELEMNT (10.0f)

	static BOOL felfele = TRUE;
	static FLOAT lastele = -1.0f;
	FLOAT nowele = (FLOAT)(gamestate_tic % ELETIME) / (FLOAT)ELETIME;

	if( nowele < lastele )
		felfele ^= 1;

	if( felfele == TRUE ) {
		for(int i=0;i<3;i++) cam_pos[i] = 10.0f - ELEMNT/2 + ELEMNT * nowele;
	}
	else {
		for(int i=0;i<3;i++) cam_pos[i] = 10.0f + ELEMNT/2 - ELEMNT * nowele;
	}

	lastele = nowele;


	// prepare cam_
	matrix_t T;
	point3_t v,cam_up,cam_right;

	MAKEVECTOR( cam_up, 0.0f, 0.0f, 1.0f );

	CrossProduct( cam_up, cam_view, cam_right );
	ScaleVector( cam_right, -1.0f );
	NormalizeVector( cam_right );

	Rotate2Matrix( cam_right, deg2rad( 40.0f ), T );
	MulMatrixVector( T, cam_up, v );
	CopyVector( cam_up, v );

	SubVector( cam_view, cam_pos, v );
	NormalizeVector( v );
	AddVector( cam_pos, v, cam_view );
***/



	if( rotdz != FLOAT_ZERO ) {

		point3_t end,normal,center,dir,cam_right,cam_up;
		FLOAT linelength,percentage;

		MAKEVECTOR( cam_up, 0.0, 0.0, 1.0 );
		MAKEVECTOR( cam_right, 0.0, 1.0, 0.0 );

		CopyVector( dir, cam_view );

		dir[2] = cam_pos[2];

		SubVector( cam_view, cam_pos, dir );
		NormalizeVector( dir );

		end[0] = cam_pos[0] + dir[0] * 10.0f;
		end[1] = cam_pos[1] + dir[1] * 10.0f;
		end[2] = cam_pos[2] + dir[2] * 10.0f;

		SubVector( end, cam_pos, dir );

		MAKEVECTOR( normal, 0.0f,0.0f,1.0f );

		linelength = DotProduct( dir, normal );

		percentage = (cam_pos[2] - 0.0f) / linelength;

		center[0] = cam_pos[0] - dir[0] * percentage;
		center[1] = cam_pos[1] - dir[1] * percentage;
		center[2] = 0.0f;

		RotateVectorVector( cam_up, origo3, 0.0f, 0.0f, deg2rad(rotdz) );
		RotateVectorVector( cam_right, origo3, 0.0f, 0.0f, deg2rad(rotdz) );
		CopyVector( cam_view, center );
		RotateVectorVector( cam_pos, center, 0.0f, 0.0f, deg2rad(rotdz) );
	}


	SetupCulling( cam_pos, cam_view );
	GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

	// WriteString( 10, SCREENH-FontHeight(), "flip: %d, tics: %d, cam_pos[2]: %.2f, nowele: %.2f %d", flip,(gamestate_tic - tics) / TICKBASE, cam_pos[2], nowele, felfele );

	DrawParticle();

	// FPS + tooltip
	if( fps_cnt == TRUE ) {
		static ULONG frame=0L,framecnt=0,tics=0L;

		if(tics+TICKBASE<gamestate_tic) {
			tics = gamestate_tic;
			frame = framecnt;
			framecnt=0;
			// másodpercenként csak 1-szer korrigál
			//     if( frame < 25 ) SetParticle( -1, -100 );
			// else if( frame > 25 ) SetParticle( -1, 100 );
		}
		else
			++framecnt;

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );
		WriteString( 0,0 /*SCREENH-FontHeight()*/, "%d", frame );

		tooltip_t mytooltip[] = { { 0,0,FontWidth()*3,FontHeight()*2, "Frames Per Secundum counter" } };
		DrawTooltip( mytooltip, dimof(mytooltip) );
	}

	return;
}

