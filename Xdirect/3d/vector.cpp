/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>

#include <xlib.h>


RCSID( "$Id: vector.c,v 1.0 97-03-10 17:36:54 bernie Exp $" )

point_t origo = { 0, 0 };
point3_t origo3 = { 0.0, 0.0, 0.0 };

const FLOAT PI = 3.14159265358979323846264338327950288419716939937510f;
const FLOAT M_2PIf = 6.283185307179586476925286766559f;



/*
In 3D graphics you use surface normals, 3-coordinate vectors of length 1, 
to express lighting and reflection. You use a lot of surface normals. 
And calculating them involves normalizing a lot of vectors. 
How do you normalize a vector? You find the length of the 
vector and then divide each of the coordinates with it. 
That is, you multiply each coordinate with

1/sqrt{x^2+y^2+z^2}

Calculating x^2+y^2+z^2 is relatively cheap. 
Finding the square root and dividing by it is expensive. 
 
float InvSqrt (float x){
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i>>1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    return x;
}
*/

//
// Return the log base 2 of the argument, or -1 if the argument is not
// an integer power of 2.
//
int ilog2( int x ) {

	int i;
	unsigned int n;

	if( x <= 0 )
		return -1;
	else
		n = (unsigned int)x;

	for( i=0; (n&0x1)==0; i++,n>>=1 );

	if( n == 1 )
		return i;

	return -1;
}





//
// Integer square root is easy, it's just not very accurate.
// integer sqrt(143) = 11, 11*11 = 121
//
int lsqrt( int value ) {

	int a, b;

	if( value < 0 )
		return -1;

	/* a = first guess = value / 2 */

	a = value / 2;
	if (a == 0) a = 1;

	while( 1 ) {
		b = value / a;
		a = (a + b) / 2;
		if( ((a * a) <= value) && ((a+1) * (a+1) > value) )
			return a;
	}

	return 0;
}




//
// If your points are (x1,y1) and (x2,y2), where the coordinates are all
// long signed integers) call it as lyhpot((x2-x1),(y2-y1)) and it'll tell
// you the distance as an integer value.
//
int ihypot( int dx, int dy ) {

	ULONG r, dd;

	dd = dx*dx+dy*dy;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;

	/* initial hypotenuse guess (from Gems) */
	r = (dx > dy) ? (dx+(dy>>1)) : (dy+(dx>>1));

	if( r == 0 )
		return (int)r;

	/* converge 3 times */
	r = (dd/r+r)>>1;
	r = (dd/r+r)>>1;
	r = (dd/r+r)>>1;

	return (int)r;
}





//
//
//
FLOAT randf( FLOAT min, FLOAT max ) {

	return min + (max - min) * rand() / (float)RAND_MAX;
}



//
//
//
void RandVector( point3_t v ) {

	FLOAT z = randf(-1.0f, 1.0f);
	FLOAT a = randf(0.0f, M_2PIf);

	FLOAT r = sqrtf(1.0f - z*z);

	FLOAT x = r * GetCos(a);
	FLOAT y = r * GetSin(a);

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return;
}



/***
n is your normal of the wall,
and you want your sparks to fly out at +/- d radians from the normal,
p0, p1, p2 are three points on the wall where the particles will emit from,
clockwise, where p0 is the common point.

precalculate these when you know where you want your particles to originate -

sd = sin(d);
cd = cos(d);

v1 = point1 - point 0
normalize v1
v2 = point2 - point 0
normalize v2

then when you want a new trajectory -

xoff = rand(-sd, sd);
yoff = rand(-cd, cd);

velocity = normal + xoff * v1 + yoff * v2;

scaler = particleSpeed / velocity.length();
velocity *= scaler;
***/




//
//  Input parameters:
//     npol		  : number of vertices
//     xp[npol], yp[npol] : x,y coord of vertices
//     x,y		  : coord of point to test
//
//  Return Value:
//     0 : test point is outside polygon
//     1 : test point is inside polygon
//
//  Notes:
//     if test point is on the border, 0 or 1 is returned.
//     If there exists an adiacent polygon, the point is
//     in only in one of the two.
//
BOOL IsPointInPoly( int npol, float *xp, float *yp, float x, float y ) {

	int i, j, c = 0;

	for( i=0, j=npol-1; i<npol; j=i++ ) {

		if( ( ((yp[i]<=y) && (y<yp[j])) ||
		      ((yp[j]<=y) && (y<yp[i])) ) &&
			(x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]) )
			c = !c;
	}

	return c;
}





//
//
//
void DumpVector( point3_t a, char *name ) {

	xprintf("%s = ( %.2f, %.2f, %.2f )\n",name,a[0],a[1],a[2]);

	return;
}






/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char *vtos( point3_t v ) {

	static int index = 0;
	static char str[8][32];
	char *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	sprintf( s, "(%.2f %.2f %.2f)", v[0], v[1], v[2] );

	return s;
}



//
//
//
BOOL EqualVector( point3_t a, point3_t b ) {

	if( (FABS(a[0] - b[0]) < FLOAT_EPSILON) &&
	    (FABS(a[1] - b[1]) < FLOAT_EPSILON) &&
	    (FABS(a[2] - b[2]) < FLOAT_EPSILON) )
			return TRUE;

	return FALSE;
}






//
//
//
void CopyVector( point3_t dst, point3_t src ) {

	int i;

	for( i=0; i<MATROW; i++ )
		dst[i] = src[i];

	return;
}





//
//
//
void CopyPoint( point_t dst, point_t src ) {

	// memcpy( dst, src, sizeof(point_t) );

	dst[0] = src[0];
	dst[1] = src[1];

	return;
}





//
//
//
void SwapVector( point3_t a, point3_t b ) {

	point3_t t;

	memcpy( t, a, sizeof(point3_t) );
	memcpy( a, b, sizeof(point3_t) );
	memcpy( b, t, sizeof(point3_t) );

	return;
}








//
// vektor osszeadsa ( vektor paralelogramma)
// ret: osszeg vektor
//
void AddVector( point3_t a, point3_t b, point3_t c ) {

	int i;

	for( i=0; i<MATROW; i++ )
		c[i] = a[i] + b[i];

	return;
}







//
// vektor kivonas ( vektor paralelogramma)
// elso - masodik
// ret: kulonbseg vektor
//
void SubVector( point3_t a, point3_t b, point3_t c ) {

	int i;

	for( i=0; i<MATROW; i++ )
		c[i] = a[i] - b[i];

	return;
}







//
// ket vektor skalaris szorzata
// ret: skalar
//
FLOAT DotProduct( point3_t v1, point3_t v2 ) {

	return (v1[0]*v2[0]) + (v1[1]*v2[1]) + (v1[2]*v2[2]);
}





//
// line: v1,v2
// pont: v3
// a:    vetület
//
FLOAT DistanceLinePoint( point3_t v1, point3_t v2, point3_t v3, point3_t *a ) {

	FLOAT u,dist;
	point3_t p;

	u = ( (v3[0] - v1[0])*(v2[0] - v1[0]) + (v3[1] - v1[1])*(v2[1] - v1[1]) + (v3[2] - v1[2])*(v2[2] - v1[2]) ) /
			ffsqrt( (v2[0] - v1[0])*(v2[0] - v1[0]) + (v2[1] - v1[1])*(v2[1] - v1[1]) + (v2[2] - v1[2])*(v2[2] - v1[2]) );

	p[0] = v1[0] + u * (v2[0] - v1[0]);
	p[1] = v1[1] + u * (v2[1] - v1[1]);
	p[2] = v1[2] + u * (v2[2] - v1[2]);

	dist = DistanceVector( p, v3 );

	if( a != NULL ) CopyVector( *a, p );

	return dist;
}





//
// vektor abszolut erteke
// ret: skalar
//
FLOAT AbsoluteVector( point3_t v ) {

	FLOAT abs;

	abs = ffsqrt( (v[0]*v[0]) + (v[1]*v[1]) + (v[2]*v[2]) );

	//abs = fsqrt26_6( fmul26_6(v[0]>>10,v[0]>>10) + fmul26_6(v[1]>>10,v[1]>>10) + fmul26_6(v[2]>>10,v[2]>>10) );

	return abs;
}







//
// ket vektor altal bezart szog
//
FLOAT AngleVector( point3_t v1, point3_t v2 ) {

	FLOAT ang,a1,a2,t;

	a1 = AbsoluteVector(v1);
	a2 = AbsoluteVector(v2);

	if( a1 == 0.0f || a2 == 0.0f )
		return 0.0f;

	// ang = GetAcos( DotProduct(v1,v2) / (a1*a2) );

	t = ( DotProduct(v1,v2) / (a1*a2) );

	ang = GetAcos( t );

	// xprintf("acos: %.2f = %.2f\n",t,ang);

	return ang;
}








//
// a vektor (i,j,k) - val bezart szogei
//
void VectorAngles( point3_t v, FLOAT *a, FLOAT *b, FLOAT *g ) {

	FLOAT rabs;

	rabs = 1.0f / AbsoluteVector( v );

	*a = GetAcos( v[0] * rabs );
	*b = GetAcos( v[1] * rabs );
	*g = GetAcos( v[2] * rabs );

	return;
}









//
// ket pont tavolsaga
//
FLOAT DistanceVector( point3_t v1, point3_t v2 ) {

	FLOAT dst;

	dst = ffsqrt( ( (v1[0]-v2[0]) * (v1[0]-v2[0]) ) +
		      ( (v1[1]-v2[1]) * (v1[1]-v2[1]) ) +
		      ( (v1[2]-v2[2]) * (v1[2]-v2[2]) )  );

	return dst;
}







//
// skalar szorzas vektorral
// ret: no
//
void ScaleVector( point3_t v, FLOAT s ) {

	int i;

	for( i=0; i<MATROW; i++ )
		v[i] *= s;

	return;
}







//
// vektorialis szorzat
// ret: VEKTOR meroleges A,B -re; jobbsodrasu rdsz-t alkotnak
// abszolut erteke megegyezik a paralelogramma teruletevel
//
void CrossProduct( point3_t v1, point3_t v2, point3_t v3 ) {

	#if 1

	v3[0] =    (v1[1] * v2[2]) - (v1[2] * v2[1])  ;
	v3[1] = -( (v1[0] * v2[2]) - (v1[2] * v2[0]) );
	v3[2] =    (v1[0] * v2[1]) - (v1[1] * v2[0])  ;

	#else

	__asm {

		push	eax
		push	ebx
		push	ecx


		mov	eax, dword ptr [v2]
		mov	ebx, dword ptr [v1]
		mov	ecx, dword ptr [v3]


		fld	dword ptr [ebx+4]
		fmul	dword ptr [eax+8]
		fld	dword ptr [ebx+8]
		fmul	dword ptr [eax]
		fld	dword ptr [ebx]
		fmul	dword ptr [eax+4]
		fld	dword ptr [ebx+8]
		fmul	dword ptr [eax+4]
		fld	dword ptr [ebx]
		fmul	dword ptr [eax+8]
		fld	dword ptr [ebx+4]
		fmul	dword ptr [eax]
		fxch	st(2)
		fsubp	st(5), st
		fsubp	st(3), st
		fsubp	st(1), st
		fxch	st(2)
		fstp	dword ptr [ecx]
		fstp	dword ptr [ecx+4]
		fstp	dword ptr [ecx+8]

		pop	ecx
		pop	ebx
		pop	eax

	}

	#endif

	return;
}














//
// v1,v2,v3 a siknak egy pontjai, v2-v1-v3
// ret: a sik normal vektora
//
void NormalVectorPlane( point3_t v1, point3_t v2, point3_t v3, point3_t n ) {

	point3_t a,b;

	SubVector(v2,v1,a);
	SubVector(v3,v1,b);

	CrossProduct(a,b,n);

	return;
}







//
// v1,v2 pontok altal meghat egyenes sik normal
// ret: normal vektor
//
void NormalVector( point3_t v1, point3_t v2, point3_t n ) {

	CrossProduct(v1,v2,n);

	return;
}







//
// egyseg vektort csinal egy vektorbol
//
void NormalizeVector( point3_t a ) {

	FLOAT l;

	l = AbsoluteVector(a);

	if( l != 0.0f ) {

		if( l != 1.0f ) {

			//l = fdiv( FIXED_ONE, TriHyp( *a[0], *a[1], *a[2] ) );

			ScaleVector( a, (FLOAT)(1.0f / l) );
		}
	}
	else
		a[0] = a[1] = a[2] = 0.0f;

	return;
}





//
// egy poligon által meghatározott sík
//
void VectorPlane( point3_t a, point3_t b, point3_t c, plane_t p ) {

	NormalVectorPlane( a, b, c, p.normal );

	NormalizeVector( p.normal );

	p.distance = DotProduct( p.normal, a );

	return;
}





//
// elforgatja a vektort a TENGELYek körül, fokban
//
void RotateVectorAxis( point3_t a, FLOAT Xan, FLOAT Yan, FLOAT Zan ) {

	FLOAT s1,s2,s3,c1,c2,c3,x,y,z;

	if( (Xan == 0.0f) && (Yan == 0.0f) && (Zan == 0.0f) )
		return;

	s1 = GetSin(Xan); c1 = GetCos(Xan);
	s2 = GetSin(Yan); c2 = GetCos(Yan);
	s3 = GetSin(Zan); c3 = GetCos(Zan);
	x = a[0];
	y = a[1];
	z = a[2];

	a[0] = (x*((s1*(s2*s3)) + (c1*c3))) + (y*(c2*s3)) + (z*((c1*(s2*s3)) - (c3*s1)));
	a[1] = (x*((c3*(s1*s2)) - (c1*s3))) + (y*(c2*c3)) + (z*((c1*(c3*s2)) + (s1*s3)));
	a[2] = (x*((c1*(s2*s3)) - (c3*s1))) + (y*(-s2))   + (z*(c1*c2));

	return;
}







//
// elforgatja a vektort egy VEKTOR körül
//
void RotateVectorVector( point3_t a, point3_t b, FLOAT Xan, FLOAT Yan, FLOAT Zan ) {

	if( (Xan == 0.0f) && (Yan == 0.0f) && (Zan == 0.0f) )
		return;

	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];

	RotateVectorAxis( a, Xan, Yan, Zan );

	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];

	return;
}



/*
 * The following routine converts an angle and a unit axis vector
 * to a matrix, returning the corresponding unit quaternion at no
 * extra cost.
 * The routine just given, MatrixFromAxisAngle, performs rotation about
 * an axis passing through the origin, so only a unit vector was needed
 * in addition to the angle. To rotate about an axis not containing the
 * origin, a point on the axis is also needed, as in the following. For
 * mathematical purity, the type POINT is used, but may be defined as:
 */
void MatrixFromAxisAngle( point3_t axis, FLOAT theta, matrix_t m ) {

	point3_t q;
	FLOAT halfTheta = theta/2.0f;
	FLOAT cosHalfTheta = GetCos(halfTheta);
	FLOAT sinHalfTheta = GetSin(halfTheta);
	FLOAT xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

	q[0] = axis[0]*sinHalfTheta;
	q[1] = axis[1]*sinHalfTheta;
	q[2] = axis[2]*sinHalfTheta;

	xs = q[0]*q[0];       ys = q[1]*q[1];	    zs = q[2]*q[2];
	wx = cosHalfTheta*xs; wy = cosHalfTheta*ys; wz = cosHalfTheta*zs;
	xx = q[0]*xs;	      xy = q[0]*ys;	    xz = q[0]*zs;
	yy = q[1]*ys;	      yz = q[1]*zs;	    zz = q[2]*zs;

	m[0][0] = 1.0f - (yy + zz); m[0][1] = xy - wz;		m[0][2] = xz + wy;
	m[1][0] = xy + wz;	    m[1][1] = 1.0f - (xx + zz); m[1][2] = yz - wx;
	m[2][0] = xz - wy;	    m[2][1] = yz + wx;		m[2][2] = 1.0f - (xx + yy);

	/* Fill in remainder of 4x4 homogeneous transform matrix. */
	m[0][3] = m[1][3] = m[2][3] = 0.0f;

	return;
}







//
// 3D grafika PC-en 31. oldal
// vizszintesen kering a pont körül
//
void Azimut( point3_t v1, point3_t v2, FLOAT Zan ) {

	FLOAT cosZan,sinZan;
	FLOAT a,a1,b,b1,c,c1;

	if( Zan == 0.0f )
		return;

	a = v1[0] - v2[0];
	b = v1[1] - v2[1];
	c = v1[2] - v2[2];

	sinZan = GetSin( Zan );
	cosZan = GetCos( Zan );

	a1 =  (a*cosZan) + (b*sinZan);
	b1 = -(a*sinZan) + (b*cosZan);
	c1 = c;

	v1[0] = v2[0] + a1;
	v1[1] = v2[1] + b1;
	v1[2] = v2[2] + c1;

	return;
}







//
// 3D grafika PC-en 32. oldal
// függőlegesen kering a pont körül
//
void Nutacio( point3_t v1, point3_t v2, FLOAT Zan ) {

	FLOAT cosZan,sinZan;
	FLOAT a,a1,b,b1,c,c1,r;

	if( Zan == 0.0f )
		return;

	a = v1[0] - v2[0];
	b = v1[1] - v2[1];
	c = v1[2] - v2[2];

	sinZan = GetSin( Zan );
	cosZan = GetCos( Zan );

	r = ffsqrt( (a*a) + (b*b) );

	if( r == 0.0f ) r = FLOAT_EPSILON;

	a1 = (a*cosZan) + ( (a*c*sinZan) / r );
	b1 = (b*cosZan) + ( (b*c*sinZan) / r );
	c1 = (c*cosZan) - (r*sinZan);

	v1[0] = v2[0] + a1;
	v1[1] = v2[1] + b1;
	v1[2] = v2[2] + c1;

	return;
}



//
//
//
void Dampen( FLOAT *v, FLOAT amt ) {

	if( *v > 0.0f ) {

		*v -= amt;

		if( *v < 0.0f )
			*v = 0.0f;
	}
	else
	if( *v < 0.0f ) {

		*v += amt;

		if( *v > 0.0f )
			*v = 0.0f;
	}

	return;
}



//
//
//
void DampenTo( FLOAT *v, FLOAT amt, FLOAT center ) {

	if( *v > center ) {

		*v -= amt;

		if( *v < center )
			*v = center;
	}
	else
	if( *v < center ) {

		*v += amt;

		if( *v > center )
			*v = center;
	}

	return;
}




/*
 * Note that since a normalized vector has a known length (1.0), if you have any
 * two of the three lengths, you can determine the third. That is how I am able to
 * send only the x and z values, and still know what y, but I do need to send the
 * sign bit for y.
 * Since one bit is left unused, you could add one more bit of precision to either
 * x or z. Also, I never took the time to optimize ReadDir(), so there are several
 * improvements I see that can be made, but I leave that exercise to you!
 */


//
// dir is a normal direction vector, each float is -1.0 to 1.0
// converts dir to a byte value for network packet
//
UCHAR WriteDir( point3_t dir ) {

	FLOAT x = dir[0];
	FLOAT y = dir[1];
	FLOAT z = dir[2];
	UCHAR temp;

	x += 1.0f;				/* convert to 0 to 1 range to eliminate negative values */
	x /= 2.0f;
	z += 1.0f;
	z /= 2.0f;
	x = MAX(0.0f, x);
	x = MIN(1.0f, x);
	z = MAX(0.0f, z);
	z = MIN(1.0f, z);
	temp = (UCHAR)(x * 7);			/* bits 0-2 x rotation */
	temp |= ((UCHAR)(z * 7) << 3);		/* bits 3-5 z rotation */
	temp |= ((UCHAR)(y<0.0f?1:0) << 6);	/* bit 6 sign of y */

	// WriteByte( temp );

	return temp;
}



//
//
//
void ReadDir( UCHAR b, point3_t *dir ) {

	FLOAT x, y, z, temp;
	// UCHAR b;

	// b = ReadByte();
	x = (FLOAT)(b&0x7) / 7.0f;
	x *= 2.0f;
	x -= 1.0f;
	z = (FLOAT)(((b >> 3)&0x7)) / 7.0f;
	z *= 2.0f;
	z -= 1.0f;

	if( (x==1) || (x == -1) ) {
		z = 0;
	}
	else
	if( (z==1) || (z==-1) ) {
		x = 0;
	}

	temp = 1.0f - x * x - z * z;
	if( temp <=0 ) {
		y = 0;
	}
	else {
		y = (FLOAT)ffsqrt(temp);
	}

	if( (b >> 6) & 1 ) {
		y = -y;
	}

	*dir[0] = x;
	*dir[1] = y;
	*dir[2] = z;

	return;
}



//
//
//
BOOL VertexInTriangle( point3_t v0, point3_t v1, point3_t v2, point3_t p ) {

	FLOAT ang;
	point3_t vec0,vec1,vec2;

	SubVector( p, v0, vec0 );
	SubVector( p, v1, vec1 );
	SubVector( p, v2, vec2 );

	NormalizeVector( vec0 );
	NormalizeVector( vec1 );
	NormalizeVector( vec2 );

	ang = (FLOAT)( acos( DotProduct(vec0,vec1) ) +
		       acos( DotProduct(vec1,vec2) ) +
		       acos( DotProduct(vec2,vec0) ) );

	if( FABS( ang - 2.0f*M_PI ) < FLOAT_EPSILON )
		return TRUE;

	return FALSE;
}



//
// find the distance between a ray and a plane.
//
FLOAT DistanceRayPlane( point3_t vRayOrigin, point3_t vnRayVector, point3_t vnPlaneNormal, FLOAT planeD ) {

	FLOAT cosAlpha;
	FLOAT deltaD;

	cosAlpha = DotProduct( vnRayVector, vnPlaneNormal );

	// parallel to the plane (alpha=90)
	if( cosAlpha == 0 )
		return -1.0f;

	deltaD = planeD - DotProduct( vRayOrigin, vnPlaneNormal );

	return deltaD / cosAlpha;
}




//
// x1 = x*cos(a) - y*sin(a)
// y1 = x*sin(a) + y*cos(a)
//
void RotatePoint( FLOAT x0, FLOAT y0, FLOAT *x1, FLOAT *y1, FLOAT fok, FLOAT centerx, FLOAT centery ) {

	FLOAT dx,dy,sina,cosa;

	if( (fok == 0.0f) || (fok == 360.0f) ) {
		*x1 = x0;
		*y1 = x0;
		return;
	}

	// centerx = (int)((FLOAT)(point[0][0] + point[1][0] + point[2][0] + point[3][0]) / 4.0f);
	// centery = (int)((FLOAT)(point[0][1] + point[1][1] + point[2][1] + point[3][1]) / 4.0f);

	// xprintf("RotatePoint: %.2f\n", fok );

	sina = GetSin( deg2rad(fok) );
	cosa = GetCos( deg2rad(fok) );

	dx = x0 - centerx;
	dy = y0 - centery;

	*x1 = centerx + (dx*cosa - dy*sina);
	*y1 = centery + (dx*sina + dy*cosa);

	return;
}




// 
// Returns a reflection vector from a vector and a normal vector
// reflect = vec - ((2 * DotVecs(vec, mirror)) * mirror)
//
void ReflectVector( point3_t v1, point3_t v2, point3_t out ) {

	point3_t vec, normal;
	point3_t reflect = {0.0f, 0.0f, 0.0f};
	FLOAT dot2;

	CopyVector(vec, v1);
	CopyVector(normal, v2);

	dot2 = 2 * DotProduct(vec, normal);

	reflect[0] = vec[0] - (dot2 * normal[0]);
	reflect[1] = vec[1] - (dot2 * normal[1]);
	reflect[2] = vec[2] - (dot2 * normal[2]);

	CopyVector(out, reflect);

	return;
}


//
// Rotate a point p by angle theta around an arbitrary axis r
//  http://local.wasp.uwa.edu.au/~pbourke/geometry/
//
void RotateVectorAroundAxis( point3_t r, point3_t p, point3_t axis, FLOAT angle ) {

	FLOAT costheta = cos(angle);
	FLOAT sintheta = sin(angle);

	r[0]= ((costheta + (1 - costheta) * axis[0] * axis[0]) * p[0]) +
	      (((1 - costheta) * axis[0] * axis[1] - axis[2] * sintheta) * p[1]) +
	      (((1 - costheta) * axis[0] * axis[2] + axis[1] * sintheta) * p[2]);

	r[1]= (((1 - costheta) * axis[0] * axis[1] + axis[2] * sintheta) * p[0]) +
	      ((costheta + (1 - costheta) * axis[1] * axis[1]) * p[1]) +
	      (((1 - costheta) * axis[1] * axis[2] - axis[0] * sintheta) * p[2]);

	r[2]= (((1 - costheta) * axis[0] * axis[2] - axis[1] * sintheta) * p[0]) +
	      (((1 - costheta) * axis[1] * axis[2] + axis[0] * sintheta) * p[1]) +
	      ((costheta + (1 - costheta) * axis[2] * axis[2]) * p[2]);

	return;
}




// ----------------------------------------------------------------------------
// for debugging: prints one line with a given C expression, an equals sign,
// and the value of the expression.  For example "angle = 35.6"
//
// #define debugPrint(e) (std::cout << #e" = " << (e) << std::endl << std::flush)


//
// Returns a float randomly distributed between 0 and 1
//
static FLOAT frandom01( void ) {
	return ((FLOAT)rand()) / ((FLOAT)RAND_MAX);
}


//
// Returns a float randomly distributed between lowerBound and upperBound
//
static FLOAT frandom2( FLOAT lowerBound, FLOAT upperBound ) {
	return lowerBound + (frandom01() * (upperBound - lowerBound));
}



// ----------------------------------------------------------------------------
// Returns a position randomly distributed inside a sphere of unit radius
// centered at the origin.  Orientation will be random and length will range
// between 0 and 1
//
void RandomUnitVector( point3_t v ) {

	do {
		v[0] = (frandom01() * 2) - 1;
		v[1] = (frandom01() * 2) - 1;
		v[2] = (frandom01() * 2) - 1;
	} while( VectorLength(v) >= 1.0f );

	return;
}




// ----------------------------------------------------------------------------
// Returns a position randomly distributed on a disk of unit radius
// on the XZ (Y=0) plane, centered at the origin.  Orientation will be
// random and length will range between 0 and 1
//
void RandomUnitPoint( point_t v ) {

	FLOAT abs;

	do {
		v[0] = (frandom01() * 2) - 1;
		v[1] = (frandom01() * 2) - 1;

		abs = ffsqrt( (v[0]*v[0]) + (v[1]*v[1]) );
	} while( abs >= 1.0f );

	return;
}





// ----------------------------------------------------------------------------
// Does a "ceiling" or "floor" operation on the angle by which a given vector
// deviates from a given reference basis vector.  Consider a cone with "basis"
// as its axis and slope of "cosineOfConeAngle".  The first argument controls
// whether the "source" vector is forced to remain inside or outside of this
// cone.  Called by vecLimitMaxDeviationAngle and vecLimitMinDeviationAngle.
//
void vecLimitDeviationAngleUtility( point3_t source, point3_t out, point3_t basis, FLOAT cosineOfConeAngle, BOOL insideOrOutside ) {

	MAKEVECTOR( out, source[0], source[1], source[2] );

	// immediately return zero length input vectors
	FLOAT sourceLength = VectorLength( source );
	if( sourceLength == 0.0f )
		return;
/***
	// measure the angular diviation of "source" from "basis"
	const Vec3 direction = source / sourceLength;
	float cosineOfSourceAngle = direction.dot (basis);

	// Simply return "source" if it already meets the angle criteria.
	// (note: we hope this top "if" gets compiled out since the flag
	// is a constant when the function is inlined into its caller)
	if( insideOrOutside ) {
		// source vector is already inside the cone, just return it
		if( cosineOfSourceAngle >= cosineOfConeAngle )
			return;
	}
	else {
		// source vector is already outside the cone, just return it
		if( cosineOfSourceAngle <= cosineOfConeAngle )
			return;
	}

	// find the portion of "source" that is perpendicular to "basis"
	const Vec3 perp = source.perpendicularComponent( basis );

	// normalize that perpendicular
	const Vec3 unitPerp = perp.normalize ();

	// construct a new vector whose length equals the source vector,
	// and lies on the intersection of a plane (formed the source and
	// basis vectors) and a cone (whose axis is "basis" and whose
	// angle corresponds to cosineOfConeAngle)
	float perpDist = sqrtXXX (1 - (cosineOfConeAngle * cosineOfConeAngle));
	const Vec3 c0 = basis * cosineOfConeAngle;
	const Vec3 c1 = unitPerp * perpDist;

	out = (c0 + c1) * sourceLength;
***/
	return;
}



// ----------------------------------------------------------------------------
// given a vector, return a vector perpendicular to it.  arbitrarily selects
// one of the infinitely many perpendicular vectors.  a zero vector maps to
// itself, otherwise length is irrelevant (empirically, output length seems to
// remain within 20% of input length).
//
void findPerpendicularIn3d( point3_t in, point3_t out, point3_t direction ) {

	// to be filled in:
	point3_t quasiPerp;  // a direction which is "almost perpendicular"
	point3_t result;     // the computed perpendicular to be returned

	// three mutually perpendicular basis vectors
	point3_t i = {1, 0, 0};
	point3_t j = {0, 1, 0};
	point3_t k = {0, 0, 1};
/***
	// measure the projection of "direction" onto each of the axes
	const float id = i.dot (direction);
	const float jd = j.dot (direction);
	const float kd = k.dot (direction);

	// set quasiPerp to the basis which is least parallel to "direction"
	if ((id <= jd) && (id <= kd)) {
		quasiPerp = i;               // projection onto i was the smallest
	}
	else {
		if( (jd <= id) && (jd <= kd) )
			quasiPerp = j;           // projection onto j was the smallest
		else
			quasiPerp = k;           // projection onto k was the smallest
	}

	// return the cross product (direction x quasiPerp)
	// which is guaranteed to be perpendicular to both of them
	result.cross (direction, quasiPerp);
***/

	return;
}






//
// Function:
//   void plane_ABCD(float *p, float *a, float *b, float *c);
// Description:
//   Compute a plane from 3 vectors.
// Parms:
//   p : The place where return the plane.
//   a : The 1st vector.
//   b : The 2nd vector.
//   c : The 3rd vector.
// Return:
//   The computed plane in p.
//
void planeABCD( point3_t *p, point3_t a, point3_t b, point3_t c ) {

	FLOAT rx1 = b[0] - a[0];
	FLOAT ry1 = b[1] - a[1];
	FLOAT rz1 = b[2] - a[2];
	FLOAT rx2 = c[0] - a[0];
	FLOAT ry2 = c[1] - a[1];
	FLOAT rz2 = c[2] - a[2];

	*p[0] = ry1*rz2 - ry2*rz1;
	*p[1] = rz1*rx2 - rz2*rx1;
	*p[2] = rx1*ry2 - rx2*ry1;

	NormalizeVector( *p );
	*p[3] = DotProduct( *p, b );

	return;
}





//
// Function:
//   float plane_dist(float *v, float *plane)
// Description:
//   Compute the distance between a plane and a point.
// Parms:
//   plane : The plane to check.
//   p : The point to check.
// Return:
//   The distance between the point and the plane.
//
FLOAT planeDistance( point3_t plane, point3_t p ) {
	return plane[0]*p[0] + plane[1]*p[1] + plane[2]*p[2] - plane[3];
}





//
// Function:
//   float plane_poly_dist(float *point, float *polynorm, float *polypoint);
// Description:
//   Compute the distance between a 3d point and a polygone.
// Parms:
//   point : The point to check.
//   polynorm : The normal of the polygon to check
//   pointpoint : A point in the polygon needed for the computing.
// Return:
//   The distance between the point and the plane.
//
FLOAT planePolyDistance( point3_t point, point3_t polynorm, point3_t polypoint ) {

	FLOAT l1[3];

	l1[0] = polypoint[0] - point[0];
	l1[1] = polypoint[1] - point[1];
	l1[2] = polypoint[2] - point[2];

	return l1[0]*polynorm[0] + l1[1]*polynorm[1] + l1[2]*polynorm[2];
}





//
// Function:
//   BOOL get_plane_intersection(float *lstart, float *lend,
//                                float *plane,  float *planepoint,
//           				      float *ipoint, float *percentage);
// Description:
//   Get the intersection point between a plane and a line.
// Parms:
//   lstart : The line start point.
//   lend   : The line end point.
//   plane  : The plane or polygon normal to check.
//   planepoint : A point (or a vertex in a polygon) included in the plane
//                to check.
//   ipoint : Place where put the computed intersection point.
//   percentage : Place where put the computed percentage.
//                0% mean intersection at line start and 100% at line end.
// Return:
//   TRUE if the line intersect with plane, FALSE otherwise.
//
BOOL getPlaneIntersection( point3_t lstart,  point3_t lend,
						   point3_t plane,   point3_t planepoint,
						   point3_t *ipoint, FLOAT *percentage ) {

	point3_t dir,l1;
	FLOAT llen, d;

	// Compute the direction vector.
	dir[0] = lend[0] - lstart[0];
	dir[1] = lend[1] - lstart[1];
	dir[2] = lend[2] - lstart[2];

	// Compute line length (use the dot product).
	llen = dir[0]*plane[0] + dir[1]*plane[1] + dir[2]*plane[2];

	// Check it does not = 0 with tolerance for floating point rounding errors.
	// If it does it means that the line is parallel to the plane so can not
	// intersect with it.

	if( FABS(llen) < FLOAT_EPSILON )
		return FALSE;

	// Compute vector l1.
	l1[0] = planepoint[0] - lstart[0];
	l1[1] = planepoint[1] - lstart[1];
	l1[2] = planepoint[2] - lstart[2];

	// Compute the distance from the plane (use dot product)
	d = l1[0]*plane[0] + l1[1]*plane[1] + l1[2]*plane[2];

	// Compute precentage.
	// How far from lstart, intersection is as a percentage of 0 to 1.
 	*percentage = d / llen;

	// The plane is behind the start of the line.
	if( *percentage < 0.0f )
		return FALSE;

	// The line does not reach the plane.
	if( *percentage > 1.0f )
		return FALSE;

	// We have it the plane so compute the intersection point.
	*ipoint[0] = lstart[0]+(dir[0]*(*percentage));
	*ipoint[1] = lstart[1]+(dir[1]*(*percentage));
	*ipoint[2] = lstart[2]+(dir[2]*(*percentage));

	return TRUE;
}




//
// Name: plane_normalize()
// Desc: Normalize a plane.
//
void planeNormalize( point3_t *plane ) {

	FLOAT ilen = 1.0f / ( AbsoluteVector(*plane) + FLOAT_EPSILON );

	*plane[0] *= ilen;
	*plane[1] *= ilen;
	*plane[2] *= ilen;
	*plane[3] *= ilen;

	return;
}



//
// Name: plane_normalizeb()
// Desc: Normalize a plane and return the normalized plane in planeb.
//
void planeNormalizeb( point3_t plane, point3_t *planeb ) {

	FLOAT ilen = 1.0f / ( ffsqrt( plane[0]*plane[0] +
                                  plane[1]*plane[1] +
                                  plane[2]*plane[2] ) + FLOAT_EPSILON );

	*planeb[0] = plane[0]*ilen;
	*planeb[1] = plane[1]*ilen;
	*planeb[2] = plane[2]*ilen;
	*planeb[3] = plane[3]*ilen;

	return;
}


#if 0

//
// Given a ray origin (orig) and direction (dir), and three vertices of
// of a triangle, this function returns TRUE and the interpolated texture
// coordinates if the ray intersects the triangle
//
BOOL IntersectTriangle( const D3DXVECTOR3 &orig, const D3DXVECTOR3 &dir,
			D3DXVECTOR3 &v0, D3DXVECTOR3 &v1, D3DXVECTOR3 &v2,
			FLOAT *t, FLOAT *u, FLOAT *v ) {

	// Find vectors for two edges sharing vert0
	D3DXVECTOR3 edge1 = v1 - v0;
	D3DXVECTOR3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	D3DXVECTOR3 pvec;
	D3DXVec3Cross( &pvec, &dir, &edge2 );

	// If determinant is near zero, ray lies in plane of triangle
	FLOAT det = D3DXVec3Dot( &edge1, &pvec );
	if( det < 0.0001f )
		return FALSE;

	// Calculate distance from vert0 to ray origin
	D3DXVECTOR3 tvec = orig - v0;

	// Calculate U parameter and test bounds
	*u = D3DXVec3Dot( &tvec, &pvec );
	if( *u < 0.0f || *u > det )
		return FALSE;

	// Prepare to test V parameter
	D3DXVECTOR3 qvec;
	D3DXVec3Cross( &qvec, &tvec, &edge1 );

	// Calculate V parameter and test bounds
	*v = D3DXVec3Dot( &dir, &qvec );
	if( *v < 0.0f || *u + *v > det )
		return FALSE;

	// Calculate t, scale parameters, ray intersects triangle
	*t = D3DXVec3Dot( &edge2, &qvec );
	FLOAT fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return TRUE;
}


//
// Cull this box against a given plane.
// Assumes plane and box are in the same coordinate space.
//
CullResult Box::CullToPlane( const Plane &plane) const {

	Vector3 min, max;

	// Find the minimum and maximum extreme
	// points along the plane's normal vector.
	for( int i = 0; i < 3; i++ ) {
		if( plane.normal[i] >= 0.0f ) {
			min[i] = mMin[i];
			max[i] = mMax[i];
		} 
		else {
			min[i] = mMax[i];
			max[i] = mMin[i];
		}
	}

	// If the minimum extreme point is outside,
	// the whole box must be outside this plane.
	if( plane.Offset(min) > 0.0f ) 
		return CULL_EXCLUSION;

	// The minimum extreme point is inside. Hence, if the
	// maximum is outside, the box must intersect the plane.
	if( plane.Offset(max) >= 0.0f ) 
		return CULL_INTERSECT;

	return CULL_INCLUSION;
}


//
// Sets the minimum and maximum extents of this box.
//
void Box::Set(const Vector3 &min, const Vector3 &max) {

	mMin = min;
	mMax = max;

	return;
}



//
// Determine whether or not the line segment p1,p2
// Intersects the 3 vertex facet bounded by pa,pb,pc
// Return true/false and the intersection point p
// The equation of the line is p = p1 + mu (p2 - p1)
// The equation of the plane is
//   A x + B y + C z + d = 0
//   n.x x + n.y y + n.z z + d = 0
//
int LineFacet( point3_t p1, point3_t p2, point3_t pa, point3_t pb, point3_t pc, point3_t *p ) {

	FLOAT d;
	FLOAT a1,a2,a3;
	FLOAT total,denom,mu;
	point3_t n,pa1,pa2,pa3;

	/* Calculate the parameters for the plane */
	n[0] = (pb[1] - pa[1])*(pc[2] - pa[2]) - (pb[2] - pa[2])*(pc[1] - pa[1]);
	n[1] = (pb[2] - pa[2])*(pc[0] - pa[0]) - (pb[0] - pa[0])*(pc[2] - pa[2]);
	n[2] = (pb[0] - pa[0])*(pc[1] - pa[1]) - (pb[1] - pa[1])*(pc[0] - pa[0]);
	NormalizeVector(n);

	d = - n[0] * pa[0] - n[1] * pa[1] - n[2] * pa[2];

	/* Calculate the position on the line that intersects the plane */
	denom = n[0] * (p2[0] - p1[0]) + n[1] * (p2[1] - p1[1]) + n[2] * (p2[2] - p1[2]);

	/* Line and plane don't intersect */
	if( FABS(denom) < FLOAT_EPSILON )
		return FALSE;

	mu = - (d + n[0] * p1[0] + n[1] * p1[1] + n[2] * p1[2]) / denom;
	*p[0] = p1[0] + mu * (p2[0] - p1[0]);
	*p[1] = p1[1] + mu * (p2[1] - p1[1]);
	*p[2] = p1[2] + mu * (p2[2] - p1[2]);

	/* Intersection not along line segment */
	if( mu < 0 || mu > 1 )
		return FALSE;

	/* Determine whether or not the intersection point is bounded by pa,pb,pc */
	pa1[0] = pa[0] - *p[0];
	pa1[1] = pa[1] - *p[1];
	pa1[2] = pa[2] - *p[2];
	NormalizeVector(pa1);

	pa2[0] = pb[0] - *p[0];
	pa2[1] = pb[1] - *p[1];
	pa2[2] = pb[2] - *p[2];
	NormalizeVector(pa2);

	pa3[0] = pc[0] - *p[0];
	pa3[1] = pc[1] - *p[1];
	pa3[2] = pc[2] - *p[2];
	NormalizeVector(pa3);

	a1 = pa1[0]*pa2[0] + pa1[1]*pa2[1] + pa1[2]*pa2[2];
	a2 = pa2[0]*pa3[0] + pa2[1]*pa3[1] + pa2[2]*pa3[2];
	a3 = pa3[0]*pa1[0] + pa3[1]*pa1[1] + pa3[2]*pa1[2];
	total = rad2deg(acos(a1) + acos(a2) + acos(a3));

	if( FABS(total - 360.0f) > FLOAT_EPSILON)
		return FALSE;

	return TRUE;
}





enum {
	DIFF_CLOCKNESS = 0,
	SAME_CLOCKNESS = 1,
};

enum {
	DARK = 0,
	FULL = 255,
};


//
//
//
int check_same_clock_dir( point3_t pt1, point3_t pt2, point3_t pt3, point3_t norm ) {

	FLOAT testi, testj, testk;
	FLOAT dotprod;

	// normal of trinagle
	testi = (((pt2[1] - pt1[1])*(pt3[2] - pt1[2])) - ((pt3[1] - pt1[1])*(pt2[2] - pt1[2])));
	testj = (((pt2[2] - pt1[2])*(pt3[0] - pt1[0])) - ((pt3[2] - pt1[2])*(pt2[0] - pt1[0])));
	testk = (((pt2[0] - pt1[0])*(pt3[1] - pt1[1])) - ((pt3[0] - pt1[0])*(pt2[1] - pt1[1])));

	// Dot product with triangle normal
	dotprod = testi*norm[0] + testj*norm[1] + testk*norm[2];

	if( dotprod < 0.0f )
		return DIFF_CLOCKNESS;

	return SAME_CLOCKNESS;
}



//
// Basic Idea is to find out if line intersect the plain of the triangle.
// And then test if this intersection point is inside the triangle.
// Using Clockness test: if the three triangles from by each side of the triangle
// to intersect point have same clocknees as the triangle then point is inside triangle.
//
BOOL RayTriIntersect( point3_t pt1, point3_t pt2, point3_t pt3,
					  point3_t linept, point3_t lineend,
					  point3_t *pt_int ) {

	point3_t norm,vect,v1,v2;
	FLOAT dotprod,t,d1,d2;

	SubVector( lineend, linept, vect );
	NormalizeVector( vect );

	SubVector( pt2, pt1, v1 );
	SubVector( pt3, pt1, v2 );
//	NormalizeVector( v1 );
//	NormalizeVector( v2 );
	CrossProduct( v1, v2, norm );

	// ugyanaz az oldal?

	SubVector( pt1, linept, v1 );
	SubVector( pt1, lineend,v2 );

	d1 = DotProduct( v1, vect );
	d2 = DotProduct( v2, vect );

	if( ((d1 < 0.0f) && (d2 < 0.0f)) || ((d1 > 0.0f) && (d2 > 0.0f)) ) {
		// xprintf("same side.\n");
		return FALSE;
	}

	// dot product of normal and line's vector if zero line is parallel to triangle
	dotprod = DotProduct( norm, vect ); // norm[0]*vect[0] + norm[1]*vect[1] + norm[2]*vect[2];

	if( dotprod < FLOAT_ZERO ) {

		// Find point of intersect to triangle plane.
		// find t to intersect point

		t = -(norm[0]*(linept[0]-pt1[0])+norm[1]*(linept[1]-pt1[1])+norm[2]*(linept[2]-pt1[2]))/
             (norm[0]*vect[0]+norm[1]*vect[1]+norm[2]*vect[2]);

		// if ds is neg line started past triangle so can't hit triangle.
		// if( t < 0.0f )
		if( t < FLOAT_ZERO )
			return FALSE;

		(*pt_int)[0] = linept[0] + vect[0]*t;
		(*pt_int)[1] = linept[1] + vect[1]*t;
		(*pt_int)[2] = linept[2] + vect[2]*t;

		if( check_same_clock_dir(pt1, pt2, *pt_int, norm) == SAME_CLOCKNESS ) {
			if( check_same_clock_dir(pt2, pt3, *pt_int, norm) == SAME_CLOCKNESS ) {
				if( check_same_clock_dir(pt3, pt1, *pt_int, norm) == SAME_CLOCKNESS ) {
					// answer in pt_int is insde triangle
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

	{
		point3_t v0 ={ 0.0f, 0.0f, 10.0f };
		point3_t v1 ={ 0.0f, 0.0f, 0.0f };
		point3_t v2 ={ 10.0f, 0.0f, 0.0f };

		point3_t l0 ={ 5.2f, 100.0f, 2.2f};
		point3_t l1 ={ 5.2f, 10.0f, 1.2f};
		point3_t inter;

		if( RayTriIntersect( v1,v0,v2, l0,l1, &inter ) ) {
			DumpVector( inter, "inter" );
		}
		else
			xprintf( "no intersection.\n");
	}


#endif


/***
//=================== Point Rotating Around Line ===========================
// p	- original point
// pp	- pivot point
// pl	- pivot line (vector)
// a	- angle to rotate in radians
// outp - output point
//==========================================================================
void RotateAroundLine(struct cvPoint *p, struct cvPoint *pp, struct cvPoint *pl, float a, struct cvPoint *outp)
{
	struct cvPoint p1, p2;
	float l, m, n, ca, sa;

	p1.x = p->x - pp->x;
	p1.y = p->y - pp->y;
	p1.z = p->z - pp->z;

	l = pl->x;
	m = pl->y;
	n = pl->z;

	ca = cos(a);
	sa = sin(a);

	p2.x = p1.x*((l*l)+ca*(1-l*l)) + p1.y*(l*(1-ca)*m+n*sa) + p1.z*(l*(1-ca)*n-m*sa);
	p2.y = p1.x*(l*(1-ca)*m-n*sa) + p1.y*(m*m+ca*(1-m*m)) + p1.z*(m*(1-ca)*n+l*sa);
	p2.z = p1.x*(l*(1-ca)*n+m*sa) + p1.y*(m*(1-ca)*n-l*sa) + p1.z*(n*n+ca*(1-n*n));

	outp->x = p2.x + pp->x;
	outp->y = p2.y + pp->y;
	outp->z = p2.z + pp->z;
}
***/




#if 0
void main() {

	point3_t U,V,A;
	matrix_t b,c;
	matrix_t a = {	{-1, 1, 2.1},
					{-3.4, 5.0, 56.8},
					{-3.4, 5.0, 56.8}, };

	V[0] = 1;
	V[1] = 1;
	V[2] = 1;
//	V[3] = 1;

	U[0] = 1;
	U[1] = 0;
	U[2] = 0;
//	U[3] = 1;

	xprintf("fi: %3.2f\n",RAD2FOK(AngleVector(U,V)));
	xprintf("UV tav: %f\n",DistanceVector(U,V));
	AddVector(U,V,&A);
	xprintf("add: ( %f, %f, %f )\n",A[0],A[1],A[2]);
	SubVector(U,V,&A);
	xprintf("sub: ( %f, %f, %f )\n",A[0],A[1],A[2]);
	CrossProduct(V,U,&A);
	xprintf("cross: ( %f, %f, %f )\n",A[0],A[1],A[2]);
	NormalVector(V,U,&A);
	xprintf("normal: ( %f, %f, %f )\n",A[0],A[1],A[2]);


	//CopyMatrix(&a,&b);
	//TransponeMatrix(&b);
	//MulMatrix(&a,&b,&c);

	xprintf("a:\n");
	DumpMatrix(&a);
	/*
	printf("b:\n");
	DumpMatrix(&b);
	printf("c:\n");
	DumpMatrix(&c);
	*/
	return;

}
#endif

