/***

3.) How to use Quaternions in practice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Ok, now you know stuff like multiplying quaternions etc, but how to use them.
Well, let's start with rotations ...

You can convert the rotation you have in [Angle,Axis] representation easily
into a Quaternion ...
Let's say you want to rotate and object around the Axis (0.5, 1.0, 0.25);
you simply do

Quaternion Rotation;
float Angle = 90.0;
Rotation.FromAxis(Angle*PI/180, 0.5, 1.0, 0.25);

note thata the angle must be in radians, so you need to convert first, the
3 other parameters are the (x,y,z) of the axis.
Fine. Now you have a rotation Quaternion, but what to do with it ??
simply do

float RotationMatrix[3][3];
Rotation.ToMatrix(RotationMatrix);

now you can transform your vertices usingt this matrix ... easy huh? :)

4.) How to use this in .3ds ?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
First off, i want to send big thanks to MRi/DoomsDay and Kombat/Immortals for
helping me out with this subject.
In .3ds the rotation is described as an Angle,Axis representation. Note that
the Y,Z are swapped in .3ds
you just do

for(int i=0; i < NrOfKeys; i++)
{
  ReadAllStuffFromFile();

  RotationKeys[i].FromAxis(Angle, Axis.x, Axis.z, Axis.y);

  if (i > 0)
    RotationKeys[i] = RotationKeys[i-1] * RotationKeys[i];
}

you must multiply the current quaternion with the previous one, as the rotation
is relative to the rotation in the previous Key, expect for Key 0 (frame 0)
where the rotation is correct ...

5.) Interpolating Quaternions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Interpolating between Euler Angles is very ugly, you might get the "Glimbal
Lock", an error that occurs very often and gives bad results. That's a reason
to use Quaternions which give good results when interpolated.

Let's come back to our .3ds example
Let's say you have a Quaternion Rot1 at Frame 10, and a Quaternion Rot2 at
frame 30.
now you want to get the Rotation Quaternion at frame 17. How to do this?
very easy :)

float t = (17-10) / (30-10);
or if you like this more

float t = (ActualFrame - KeyFrame1) / (KeyFrame2 - KeyFrame1);

// t must be between 0 and 1 therefor this little hack

// now you do
Quaternion ThisFrameRot;
ThisFrameRot.Slerp(Rot1, Rot2, t);

and you have your rotation at frame 17. You just need to do a ToMatrix now and
you can transform your vertices ... Kinda easy, hu :))

***/


#include <math.h>
#include <string.h>

#include <xlib.h>


RCSID( "$Id: quat.cpp,v 1.1.1.1 2003/08/19 17:44:50 bernie Exp $" )







//
//
//
void DumpQuat( quat_t a, char *name ) {

	xprintf("%s = ( %.2f, %.2f, %.2f, %.2f )\n",name,a[0],a[1],a[2],a[3]);

	return;
}









// Purpose:	Computes the product of two quaternions
// Arguments:	pointer to quaternions and dest
//
void MulQuat( quat_t a, quat_t b, quat_t c ) {

	c[3] = a[3]*b[3] - ( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] );

	c[0] = a[3]*b[0] + b[3]*a[0] + a[1]*b[2] - a[2]*b[1];
	c[1] = a[3]*b[1] + b[3]*a[1] + a[2]*b[0] - a[0]*b[2];
	c[2] = a[3]*b[2] + b[3]*a[2] + a[0]*b[1] - a[1]*b[0];

	return;
}







//
//
//
void ConjugateQuat( quat_t a ) {

	a[0] = -a[0];
	a[1] = -a[1];
	a[2] = -a[2];

	return;
}








//
//
//
void InvertQuat( quat_t a ) {

	FLOAT normal,recip;

	if( (normal = (FLOAT)ffsqrt( a[3]*a[3] + a[0]*a[0] + a[1]*a[1] + a[2]*a[2] )) == 0.0f )
		normal = 1.0f;

	recip = 1.0f / normal;

	a[3] =	a[3] * recip;
	a[0] = -a[0] * recip;
	a[1] = -a[1] * recip;
	a[2] = -a[2] * recip;

	return;
}





// Purpose:	Normalize a Quaternion
// Arguments:	a quaternion to set
//
void NormalizeQuat( quat_t a ) {

	FLOAT normal,recip;

	normal = (FLOAT)ffsqrt( a[3]*a[3] + a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );

	if( normal == 0.0f ) {
		a[3] = 1.0f;
		a[0] = a[1] = a[2] = 0.0f;
	}
	else {
		recip = 1.0f / normal;

		a[3] *= recip;
		a[0] *= recip;
		a[1] *= recip;
		a[2] *= recip;
	}

	return;
}







// Purpose:	Convert a set of Euler angles to a Quaternion
// Arguments:	A rotation set of 3 angles (radian), a quaternion to set
// Discussion:	As the order of rotations is important I am
//		using the Quantum Mechanics convention of (X,Y,Z)
//		a Yaw-Pitch-Roll (Y,X,Z) system would have to be
//		adjusted.  It is more efficient this way though.
//
void Euler2Quat( FLOAT rx, FLOAT ry, FLOAT rz, quat_t a ) {

	FLOAT tx,ty,tz,cx,cy,cz,sx,sy,sz,cc,cs,sc,ss;

	// get the half angles

	tx = rx * (FLOAT)0.5;
	ty = ry * (FLOAT)0.5;
	tz = rz * (FLOAT)0.5;
	cx = (FLOAT)cos(tx);
	cy = (FLOAT)cos(ty);
	cz = (FLOAT)cos(tz);
	sx = (FLOAT)sin(tx);
	sy = (FLOAT)sin(ty);
	sz = (FLOAT)sin(tz);

	cc = cx * cz;
	cs = cx * sz;
	sc = sx * cz;
	ss = sx * sz;

	a[0] = (cy * sc) - (sy * cs);
	a[1] = (cy * ss) + (sy * cc);
	a[2] = (cy * cs) - (sy * sc);
	a[3] = (cy * cc) + (sy * ss);

	// insure the quaternion is normalized
	// probably not necessary in most cases

	NormalizeQuat( a );

	return;
}





// Purpose:	Convert a set of Euler angles to a Quaternion
// Arguments:	A rotation set of 3 angles (radian), a quaternion to set
// Discussion:	This is a second variation.  It creates a
//		Series of quaternions and multiplies them together
//		It would be easier to extend this for other rotation orders
//
void Euler2Quat2( FLOAT rx, FLOAT ry, FLOAT rz, quat_t a ) {

	FLOAT ti,tj,tk;
	quat_t qx,qy,qz,qf;


	// get the half angles

	ti = rx * (FLOAT)0.5;
	tj = ry * (FLOAT)0.5;
	tk = rz * (FLOAT)0.5;

	qx[0] = (FLOAT)sin(ti);
	qx[1] = 0.0;
	qx[2] = 0.0;
	qx[3] = (FLOAT)cos(ti);

	qy[0] = 0.0;
	qy[1] = (FLOAT)sin(tj);
	qy[2] = 0.0;
	qy[3] = (FLOAT)cos(tj);

	qz[0] = 0.0;
	qz[1] = 0.0;
	qz[2] = (FLOAT)sin(tk);
	qz[3] = (FLOAT)cos(tk);

	MulQuat( qx, qy, qf);
	MulQuat( qf, qz, qf);


	// insure the quaternion is normalized
	// probably not necessary in most cases

	NormalizeQuat( qf );

	a[0] = qf[0];
	a[1] = qf[1];
	a[2] = qf[2];
	a[3] = qf[3];

	return;
}






//
//
//
void AngleAxis2Quat( FLOAT Angle, FLOAT x, FLOAT y, FLOAT z, quat_t a ) {

	FLOAT omega, s, c;

	s = (FLOAT)ffsqrt( x*x + y*y + z*z);

	if( FABS(s) > FLOAT_EPSILON ) {

		c = 1.0f / s;

		x *= c;
		y *= c;
		z *= c;

		omega = -0.5f * Angle;
		s = (FLOAT)sin( omega );

		a[0] = s * x;
		a[1] = s * y;
		a[2] = s * z;
		a[3] = (FLOAT)cos( omega );
	}
	else {
		a[0] = a[1] = 0.0f;
		a[2] = 0.0f;
		a[3] = 1.0f;
	}

	NormalizeQuat( a );

	return;
}







//
//
//
void Quat2Matrix( quat_t a, matrix_t matrix ) {

	ZEROMATRIX( matrix );

	matrix[0][0] = 1.0f - 2.0f*a[1]*a[1] - 2.0f*a[2]*a[2];
	matrix[1][0] =	      2.0f*a[0]*a[1] + 2.0f*a[3]*a[2];
	matrix[2][0] =	      2.0f*a[0]*a[2] - 2.0f*a[3]*a[1];

	matrix[0][1] =	      2.0f*a[0]*a[1] - 2.0f*a[3]*a[2];
	matrix[1][1] = 1.0f - 2.0f*a[0]*a[0] - 2.0f*a[2]*a[2];
	matrix[2][1] =	      2.0f*a[1]*a[2] + 2.0f*a[3]*a[0];

	matrix[0][2] =	      2.0f*a[0]*a[2] + 2.0f*a[3]*a[1];
	matrix[1][2] =	      2.0f*a[1]*a[2] - 2.0f*a[3]*a[0];
	matrix[2][2] = 1.0f - 2.0f*a[0]*a[0] - 2.0f*a[1]*a[1];

	return;
}







//
//
//
void SlerpQuat( quat_t a, quat_t b, float t, quat_t c ) {

	float omega, cosom, sinom, sclp, sclq;


	cosom = a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];


	if( (1.0f+cosom) > FLOAT_EPSILON ) {

		if( (1.0f-cosom) > FLOAT_EPSILON ) {

			omega = (FLOAT)acos( cosom);
			sinom = (FLOAT)sin( omega );
			sclp = (FLOAT)sin( (1.0f-t) * omega ) / sinom;
			sclq = (FLOAT)sin( t*omega ) / sinom;
		}
		else {

			sclp = 1.0f - t;
			sclq = t;
		}

		c[0] = sclp*a[0] + sclq*b[0];
		c[1] = sclp*a[1] + sclq*b[1];
		c[2] = sclp*a[2] + sclq*b[2];
		c[3] = sclp*a[3] + sclq*b[3];

	}
	else {

		c[0] = -a[1];
		c[1] =	a[0];
		c[2] = -a[3];
		c[3] =	a[2];

		sclp = (FLOAT)sin( (1.0f-t) * M_PI * 0.5f );
		sclq = (FLOAT)sin( t * M_PI * 0.5f );

		c[0] = sclp*a[0] + sclq*b[0];
		c[1] = sclp*a[1] + sclq*b[1];
		c[2] = sclp*a[2] + sclq*b[2];
	}

	return;
}





//
//
//
void ExpQuat( quat_t a ) {

	FLOAT Mul,Length;

	Length = (FLOAT)ffsqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );

	if( Length > (FLOAT)(1.0e-4) )
		Mul = (FLOAT)sin( Length ) / Length;
	else
		Mul = 1.0f;

	a[3] = (FLOAT)cos( Length );

	a[0] *= Mul;
	a[1] *= Mul;
	a[2] *= Mul;

	return;
}





//
//
//
void LogQuat( quat_t a ) {

	FLOAT Length;

	Length = (FLOAT)ffsqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
	Length = (FLOAT)atan( Length / a[3] );

	a[3] = 0.0f;

	a[0] *= Length;
	a[1] *= Length;
	a[2] *= Length;

	return;
}



