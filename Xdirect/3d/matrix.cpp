/* Copyright (C) 1997 Kirschner, Bernát. All Rights Reserved Worldwide. */

#include <math.h>
#include <float.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: matrix.c,v 1.0 97-03-10 17:36:54 bernie Exp $" )



//
// kiir egy matrixot
//
void DumpMatrix( matrix_t a ) {

	int j;

	xprintf("Ú                         ¿\n");
	xprintf("³ "); for(j=0;j<MATCOL;j++) xprintf("%5.2f ", a[0][j] ); xprintf("³\n");
	xprintf("³ "); for(j=0;j<MATCOL;j++) xprintf("%5.2f ", a[1][j] ); xprintf("³\n");
	xprintf("³ "); for(j=0;j<MATCOL;j++) xprintf("%5.2f ", a[2][j] ); xprintf("³\n");
	//xprintf("³ "); for(j=0;j<MATCOL;j++) xprintf("%5.2f ", a[3][j] ); xprintf("³\n");
	xprintf("À                         Ù\n");

	return;
}



//
// transzponal egy matrixot
//
void TransposeMatrix( matrix_t a ) {

	int i,j;
	FLOAT temp;

	for( i=0; i<MATCOL; i++ )
		for( j=i; j<MATROW; j++ ) {
			temp = a[i][j];
			a[i][j] = a[j][i];
			a[j][i] = temp;
		}

	return;
}



//
//
//
void AdjMatrix( matrix_t a ) {

	USEPARAM(a);


	return;
}




//
// inv(a) = adj( a^(-1) ) / det(a);
//
void InverseMatrix( matrix_t a ) {

	FLOAT det;

	det = DetMatrix( a );

	TransposeMatrix( a );

	return;
}



//
// osszszoroz ket matrixot
// ÚÄÄÄ¿ ÚÄÄÄ¿ ÚÄÄÄ¿
// ³ a ³X³ b ³=³ c ³
// ÀÄÄÄÙ ÀÄÄÄÙ ÀÄÄÄÙ
//
void MulMatrix( matrix_t a, matrix_t b, matrix_t c ) {

	int i,j,k;

	for( i=0; i<MATROW; i++ ) {
		for( j=0; j<MATROW;j++ ) {			// MATCOL

			c[i][j] = 0.0f;

			for( k=0; k<MATROW; k++ )		// MATCOL
				c[i][j] += ( a[i][k] * b[k][j] );
		}
	}

	// a második maátrix 3. sora 1.0f, 1.0f, 1.0f

	c[0][3] = a[0][3];
	c[1][3] = a[1][3];
	c[2][3] = a[2][3];

	return;
}



//
// diagonalis szorazata a matrixnak
//
FLOAT DiagonalMulMatrix( matrix_t a ) {

	int i;
	FLOAT result=1.0;

	for(i=0; i<MATROW; i++ )
		result = a[i][i] * result;

	return result;
}



//
// skalar szorzas matrixal
// ÚÄÄÄ¿     ÚÄÄÄ¿
// ³ a ³X S =³ a ³
// ÀÄÄÄÙ     ÀÄÄÄÙ
//
void ScaleMatrix( matrix_t a, FLOAT s ) {

	int i,j;

	for( i=0; i<MATROW; i++ )
		for( j=0; j<MATCOL; j++ )
			a[i][j] *= s;

	return;
}




//
// osszad ket matrixot
// ÚÄÄÄ¿ ÚÄÄÄ¿ ÚÄÄÄ¿
// ³ a ³+³ b ³=³ c ³
// ÀÄÄÄÙ ÀÄÄÄÙ ÀÄÄÄÙ
//
void AddMatrix( matrix_t a, matrix_t b, matrix_t c ) {

	int i,j;

	for( i=0; i<MATROW; i++ )   // a
		for( j=0; j<MATCOL; j++ ) // a
			c[i][j] = a[i][j] + b[i][j];

	return;
}



//
// kivon ket matrixot
// ÚÄÄÄ¿ ÚÄÄÄ¿ ÚÄÄÄ¿
// ³ a ³-³ b ³=³ c ³
// ÀÄÄÄÙ ÀÄÄÄÙ ÀÄÄÄÙ
//
void SubMatrix( matrix_t a, matrix_t b, matrix_t c) {

	int i,j;

	for( i=0; i<MATROW; i++ ) // a
		for( j=0; j<MATCOL; j++ ) // a
			c[i][j] = a[i][j] - b[i][j];


	return;
}




//
//
//
void NormalizeMatrix( matrix_t m ) {

	NormalizeVector( m[0] );
	NormalizeVector( m[1] );
	NormalizeVector( m[2] );

	return;
}




//
//
//
void OrthoNormalizeMatrix( matrix_t m ) {

	point3_t temp,e1,e2,e3;

	CopyVector( e1, m[0] );
	CopyVector( e2, m[1] );
	CopyVector( e3, m[2] );

	NormalizeVector( e1 );

	CopyVector( temp, e1 );
	ScaleVector( temp, DotProduct( e2, e1 ) );

	SubVector( e2, temp, e2 );
	NormalizeVector( e2 );

	CrossProduct( e1, e2, e3 );

	CopyVector( m[0], e1 );
	CopyVector( m[1], e2 );
	CopyVector( m[2], e3 );

	return;
}



//
//
//
void CopyMatrix( matrix_t dst, matrix_t src ) {

	memmove( dst, src, sizeof(matrix_t) );

	return;
}




//
//
//
void IdentityMatrix( matrix_t m ) {

	MakeMatrix( m,
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f );

	return;
}




//
//
//
void MakeMatrix( matrix_t m,
	FLOAT a00, FLOAT a01, FLOAT a02, FLOAT a03,
	FLOAT a10, FLOAT a11, FLOAT a12, FLOAT a13,
	FLOAT a20, FLOAT a21, FLOAT a22, FLOAT a23 ) {
	// FLOAT a30, FLOAT a31, FLOAT a32, FLOAT a33 ) {

#define z(x,y)	m[x][y] = (a##x##y)

	z(0,0);
	z(0,1);
	z(0,2);
	z(0,3);

	z(1,0);
	z(1,1);
	z(1,2);
	z(1,3);

	z(2,0);
	z(2,1);
	z(2,2);
	z(2,3);

/*
	z(3,0);
	z(3,1);
	z(3,2);
	z(3,3);
*/
	return;
}









//  Ú		       ¿   Ú		¿    Ú		¿
//  ³ a00  a01	a02  x ³ * ³ x	y  z  1 ³ =  ³..........³
//  ³ a10  a11	a12  y ³   À		Ù    À		Ù
//  ³ a20  a21	a22  z ³
//  ³	0    0	  0  1 ³
//  À		       Ù



// ÚÄÄÄ¿
// ³3X3³ matrix DETERMINANS-a
// ÀÄÄÄÙ
FLOAT DetMatrix( matrix_t m ) {

	return ( ( m[0][0] * ( (m[1][1] * m[2][2]) - (m[1][2] * m[2][1]) ) ) -
		 ( m[0][1] * ( (m[1][0] * m[2][2]) - (m[1][2] * m[2][0]) ) ) +
		 ( m[0][2] * ( (m[1][0] * m[2][1]) - (m[1][1] * m[2][0]) ) )  );
}



//
// osszeszoroz egy matrixot egy VEKTOR-ral (point3_t)
// ÚÄÄÄ¿ ÚÄ¿ ÚÄ¿
// ³ m ³X³a³=³b³
// ÀÄÄÄÙ ÀÄÙ ÀÄÙ
//
void MulMatrixVector( matrix_t m, point3_t a, point3_t b ) {

	#if 1

	 //		X		   Y		      Z        helyvektor
	b[0] = (m[0][0] * a[0]) + (m[0][1] * a[1]) + (m[0][2] * a[2]) + m[0][3];
	b[1] = (m[1][0] * a[0]) + (m[1][1] * a[1]) + (m[1][2] * a[2]) + m[1][3];
	b[2] = (m[2][0] * a[0]) + (m[2][1] * a[1]) + (m[2][2] * a[2]) + m[2][3];
	// b[3] = 1.0;

	#else

	__asm {

		push	eax
		push	ebx
		push	ecx

		mov	eax, dword ptr [ b ]
		mov	ebx, dword ptr [ a ]
		mov	ecx, dword ptr [ m ]

		fld		dword ptr [ebx]
		fmul	dword ptr [ecx]

		fld		dword ptr [ebx]
		fmul	dword ptr [ecx+16]

		fld		dword ptr [ebx]
		fmul	dword ptr [ecx+32]

		fld		dword ptr [ebx+4]
		fmul	dword ptr [ecx+4]

		fld		dword ptr [ebx+4]
		fmul	dword ptr [ecx+20]

		fld		dword ptr [ebx+4]
		fmul	dword ptr [ecx+36]

		fxch	st(2)
		faddp	st(5),st
		faddp	st(3),st
		faddp	st(1),st

		fld		dword ptr [ebx+8]
		fmul	dword ptr [ecx+8]

		fld		dword ptr [ebx+8]
		fmul	dword ptr [ecx+24]

		fld		dword ptr [ebx+8]
		fmul	dword ptr [ecx+40]

		fxch	st(2)
		faddp	st(5),st
		faddp	st(3),st
		faddp	st(1),st
		fxch	st(2)
		fadd	dword ptr [ecx+12]
		fxch	st(1)
		fadd	dword ptr [ecx+28]
		fxch	st(2)
		fadd	dword ptr [ecx+44]
		fxch	st(1)

		fstp	dword ptr [eax]
		fstp	dword ptr [eax+8]
		fstp	dword ptr [eax+4]

		pop	ecx
		pop	ebx
		pop	eax
	}

	#endif

	return;
}



//
// osszeszoroz egy matrixot egy VEKTOR-ral (point3_t),
// displacement nélkul.
// ÚÄÄÄ¿ ÚÄ¿ ÚÄ¿
// ³ m ³X³a³=³b³
// ÀÄÄÄÙ ÀÄÙ ÀÄÙ
//
void MulMatrixVectorNoDisplace( matrix_t m, point3_t a, point3_t b ) {

	 //		X		   Y		      Z        helyvektor
	b[0] = (m[0][0] * a[0]) + (m[0][1] * a[1]) + (m[0][2] * a[2]); // + m[0][3];
	b[1] = (m[1][0] * a[0]) + (m[1][1] * a[1]) + (m[1][2] * a[2]); // + m[1][3];
	b[2] = (m[2][0] * a[0]) + (m[2][1] * a[1]) + (m[2][2] * a[2]); // + m[2][3];
	// b[3] = 1.0;

	return;
}




//
// transzformacios matrix generalo
//
void GenTransMatrix( point3_t pos, point3_t dir, matrix_t T ) {

	FLOAT k,krecip;
	point3_t c;

	SubVector(dir,pos,c);
	NormalizeVector(c);

	k      =  ffsqrt( (c[0]*c[0]) + (c[1]*c[1]) );
	krecip =  1.0f / k;

	// transzformacios matrix
	T[0][0] = c[1]*krecip;			 T[0][1] = -(c[0]*krecip);		  T[0][2] = 0.0;
	T[1][0] = -((c[0]*c[2])*krecip); T[1][1] = -((c[1]*c[2])*krecip); T[1][2] = k;
	T[2][0] = c[0]; 				 T[2][1] = c[1];				  T[2][2] = c[2];


	// a nezopont helyvektora
	T[0][3] = -( (pos[0] * T[0][0]) +
				 (pos[1] * T[0][1]) +
				 (pos[2] * T[0][2]) );

	T[1][3] = -( (pos[0] * T[1][0]) +
				 (pos[1] * T[1][1]) +
				 (pos[2] * T[1][2]) );

	T[2][3] = -( (pos[0] * T[2][0]) +
				 (pos[1] * T[2][1]) +
				 (pos[2] * T[2][2]) );

	return;
}




//
// Geometria 227. oldal
//		       view	      pos			up
void RotateAround( point3_t view, point3_t dis, FLOAT ang, point3_t p0 ) {

	matrix_t A;
	FLOAT sin0,cos0,cos1;
	point3_t v,p1;


	sin0 = GetSin( ang );
	cos0 = GetCos( ang );

	SubVector( view, dis, v );
	NormalizeVector( v );

	SubVector( p0, dis, p1 );

	cos1 = (1.0f - cos0);
/***
  transform->matrix[0][0] = V->x * V->x + cosx * (1.0 - V->x * V->x);
  transform->matrix[0][1] = V->x * V->y * (1.0 - cosx) + V->z * sinx;
  transform->matrix[0][2] = V->x * V->z * (1.0 - cosx) - V->y * sinx;
  transform->matrix[1][0] = V->x * V->y * (1.0 - cosx) - V->z * sinx;
  transform->matrix[1][1] = V->y * V->y + cosx * (1.0 - V->y * V->y);
  transform->matrix[1][2] = V->y * V->z * (1.0 - cosx) + V->x * sinx;
  transform->matrix[2][0] = V->x * V->z * (1.0 - cosx) + V->y * sinx;
  transform->matrix[2][1] = V->y * V->z * (1.0 - cosx) - V->x * sinx;
  transform->matrix[2][2] = V->z * V->z + cosx * (1.0 - V->z * V->z);
***/
	A[0][0] = cos0 + ( cos1 * ( v[0] * v[0] ) );
	A[0][1] = ( cos1 * ( v[0] * v[1] ) )  -  ( v[2] * sin0 );
	A[0][2] = ( cos1 * ( v[0] * v[2] ) )  +  ( v[1] * sin0 );
	A[0][3] = dis[0];

	A[1][0] = ( cos1 * ( v[0] * v[1] ) )  +  ( v[2] * sin0 );
	A[1][1] = cos0 + ( cos1 * ( v[1] * v[1] ) );
	A[1][2] = ( cos1 * ( v[1] * v[2] ) )  -  ( v[0] * sin0 );
	A[1][3] = dis[1];

	A[2][0] = ( cos1 * ( v[0] * v[2] ) )  -  ( v[1] * sin0 );
	A[2][1] = ( cos1 * ( v[1] * v[2] ) )  +  ( v[0] * sin0 );
	A[2][2] = cos0 + ( cos1 * ( v[2] * v[2] ) );
	A[2][3] = dis[2];


	MulMatrixVector( A, p1, p0 );


	return;
}





//
//
//
void RotateMatrixX( matrix_t T, FLOAT Xan ) {

	FLOAT CosTemp, SinTemp;
	FLOAT Temp10, Temp11, Temp12, Temp20, Temp21, Temp22;

	SinTemp = GetSin( Xan );
	CosTemp = GetCos( Xan );

	Temp10 = (CosTemp * T[1][0]) + (-SinTemp * T[2][0]);
	Temp11 = (CosTemp * T[1][1]) + (-SinTemp * T[2][1]);
	Temp12 = (CosTemp * T[1][2]) + (-SinTemp * T[2][2]);
	Temp20 = (SinTemp * T[1][0]) + ( CosTemp * T[2][0]);
	Temp21 = (SinTemp * T[1][1]) + ( CosTemp * T[2][1]);
	Temp22 = (SinTemp * T[1][2]) + ( CosTemp * T[2][2]);

	T[1][0] = Temp10; T[1][1] = Temp11;
	T[1][2] = Temp12; T[2][0] = Temp20;
	T[2][1] = Temp21; T[2][2] = Temp22;

	return;
}




//
//
//
void RotateMatrixY( matrix_t T, FLOAT Yan ) {

	FLOAT CosTemp, SinTemp;
	FLOAT Temp00, Temp01, Temp02, Temp20, Temp21, Temp22;

	SinTemp = GetSin( Yan );
	CosTemp = GetCos( Yan );

	Temp00 = ( CosTemp * T[0][0]) + (SinTemp * T[2][0]);
	Temp01 = ( CosTemp * T[0][1]) + (SinTemp * T[2][1]);
	Temp02 = ( CosTemp * T[0][2]) + (SinTemp * T[2][2]);
	Temp20 = (-SinTemp * T[0][0]) + (CosTemp * T[2][0]);
	Temp21 = (-SinTemp * T[0][1]) + (CosTemp * T[2][1]);
	Temp22 = (-SinTemp * T[0][2]) + (CosTemp * T[2][2]);

	T[0][0] = Temp00; T[0][1] = Temp01;
	T[0][2] = Temp02; T[2][0] = Temp20;
	T[0][1] = Temp21; T[2][2] = Temp22;

	return;
}



//
//
//
void RotateMatrixZ( matrix_t T, FLOAT Zan ) {

	FLOAT CosTemp, SinTemp;
	FLOAT Temp00, Temp01, Temp02, Temp10, Temp11, Temp12;

	SinTemp = GetSin( Zan );
	CosTemp = GetCos( Zan );

	Temp00 = (CosTemp * T[0][0]) + (-SinTemp * T[1][0]);
	Temp01 = (CosTemp * T[0][1]) + (-SinTemp * T[1][1]);
	Temp02 = (CosTemp * T[0][2]) + (-SinTemp * T[1][2]);
	Temp10 = (SinTemp * T[0][0]) + ( CosTemp * T[1][0]);
	Temp11 = (SinTemp * T[0][1]) + ( CosTemp * T[1][1]);
	Temp12 = (SinTemp * T[0][2]) + ( CosTemp * T[1][2]);

	T[0][0] = Temp00; T[0][1] = Temp01;
	T[0][2] = Temp02; T[1][0] = Temp10;
	T[1][1] = Temp11; T[1][2] = Temp12;

	return;
}



//
//
//
void TransMatrixX( matrix_t T, FLOAT Distance ) {

	T[0][3] += ( T[0][0] * Distance );
	T[1][3] += ( T[1][0] * Distance );
	T[2][3] += ( T[2][0] * Distance );

	return;
}



//
//
//
void TransMatrixY( matrix_t T, FLOAT Distance ) {

	T[0][3] += ( T[0][1] * Distance );
	T[1][3] += ( T[1][1] * Distance );
	T[2][3] += ( T[2][1] * Distance );

	return;
}




//
//
//
void TransMatrixZ( matrix_t T, FLOAT Distance ) {

	T[0][3] += ( T[0][2] * Distance );
	T[1][3] += ( T[1][2] * Distance );
	T[2][3] += ( T[2][2] * Distance );

	return;
}




//
//
//
void RotateMatrix( matrix_t T, FLOAT Xan, FLOAT Yan, FLOAT Zan ) {

	matrix_t m,temp;

	if( FABS(Xan) > FLOAT_EPSILON ) {

		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;

		RotateMatrixX( m, Xan );

		MulMatrix( T, m, temp );

		CopyMatrix( T, temp );
	}

	if( FABS(Yan) > FLOAT_EPSILON ) {

		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;

		RotateMatrixY( m, Yan );

		MulMatrix( T, m, temp );

		CopyMatrix( T, temp );
	}

	if( FABS(Zan) > FLOAT_EPSILON ) {

		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;

		RotateMatrixZ( m, Zan );

		MulMatrix( T, m, temp );

		CopyMatrix( T, temp );
	}

	return;
}



//
//
//
void MatrixRotateAxis( point3_t axis, FLOAT angleDeg, matrix_t m ) {

	FLOAT angleRad = deg2rad(angleDeg); // angleDeg * M_PI / 180.0f,
	FLOAT c = (FLOAT)cos(angleRad);
	FLOAT s = (FLOAT)sin(angleRad);
	FLOAT t = 1.0f - c;

	NormalizeVector( axis );
	IdentityMatrix( m );

#define VX 0
#define VY 1
#define VZ 2

	m[0][0] = t * axis[VX] * axis[VX] + c;
	m[0][1] = t * axis[VX] * axis[VY] - s * axis[VZ];
	m[0][2] = t * axis[VX] * axis[VZ] + s * axis[VY];
	m[0][3] = 0.0f;

	m[1][0] = t * axis[VX] * axis[VY] + s * axis[VZ];
	m[1][1] = t * axis[VY] * axis[VY] + c;
	m[1][2] = t * axis[VY] * axis[VZ] - s * axis[VX];
	m[1][3] = 0.0f;

	m[2][0] = t * axis[VX] * axis[VZ] - s * axis[VY];
	m[2][1] = t * axis[VY] * axis[VZ] + s * axis[VX];
	m[2][2] = t * axis[VZ] * axis[VZ] + c;
	m[2][3] = 0.0f;

#undef VX
#undef VY
#undef VZ

	return;
}



//
//
//
void Rotate2Matrix( point3_t line, FLOAT angle, matrix_t rotate ) {

	int row, col, mid;
	matrix_t I, A, A2;
	FLOAT sn, omcs;

	// identity matrix
	I[0][0] = 1.0f; I[0][1] = 0.0f; I[0][2] = 0.0f; I[0][3] = 0.0f;
	I[1][0] = 0.0f; I[1][1] = 1.0f; I[1][2] = 0.0f; I[1][3] = 0.0f;
	I[2][0] = 0.0f; I[2][1] = 0.0f; I[2][2] = 1.0f; I[2][3] = 0.0f;

	// infinitesimal rotation about line
	A[0][0] = 0.0f;     A[0][1] = +line[2]; A[0][2] = -line[1]; A[0][3] = 0.0f;
	A[1][0] = -line[2]; A[1][1] = 0.0f;		A[1][2] = +line[0]; A[1][3] = 0.0f;
	A[2][0] = +line[1]; A[2][1] = -line[0]; A[2][2] = 0.0f;     A[2][3] = 0.0f;

	// A2 = A*A
	for( row=0; row<3; row++ )
	for( col=0; col<3; col++ ) {

		A2[row][col] = 0.0f;

		for( mid=0; mid<3; mid++ )
			A2[row][col] += A[row][mid]*A[mid][col];
	}

	sn = GetSin(angle);
	omcs = 1-GetCos(angle);

	// rotation is I+sin(angle)*A+[1-cos(angle)]*A*A
	for( row=0; row<3; row++ )
	for( col=0; col<3; col++ )
		rotate[row][col] = I[row][col] + sn * A[row][col] + omcs * A2[row][col];

	rotate[0][3] = 0.0f;
	rotate[1][3] = 0.0f;
	rotate[2][3] = 0.0f;

	return;
}




//
//
//
void Rotate4D( point4_t u, point4_t v, FLOAT angle, FLOAT rotate[4][4] ) {

	int row, col, mid;
	FLOAT I[4][4], A[4][4], A2[4][4];
	FLOAT length, dot, sn, omcs;

	/* orthonormalize vectors u and v */
	length = ffsqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]+u[3]*u[3]);
	if( length <= 0.0f )
		return;

	for( row=0; row<4; row++ )
		u[row] /= length;

	dot = u[0]*v[0]+u[1]*v[1]+u[2]*v[2]+u[3]*v[3];
	for( row=0; row<4; row++ )
		v[row] -= dot*u[row];

	length = ffsqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]);
	if( length <= 0.0f )
		return;

	for( row=0; row<4; row++ )
		v[row] /= length;

	/* identity matrix */
	I[0][0] = 1.0f; I[0][1] = 0.0f; I[0][2] = 0.0f; I[0][3] = 0.0f;
	I[1][0] = 0.0f; I[1][1] = 1.0f; I[1][2] = 0.0f; I[1][3] = 0.0f;
	I[2][0] = 0.0f; I[2][1] = 0.0f; I[2][2] = 1.0f; I[2][3] = 0.0f;
	I[3][0] = 0.0f; I[3][1] = 0.0f; I[3][2] = 0.0f; I[3][3] = 1.0f;

	/* infinitesimal rotation through uv-plane */
	A[0][0] = 0.0f;
	A[0][1] = u[2]*v[3]-u[3]*v[2];
	A[0][2] = u[3]*v[1]-u[1]*v[3];
	A[0][3] = u[1]*v[2]-u[2]*v[1];
	A[1][0] = -A[0][1];
	A[1][1] = 0.0f;
	A[1][2] = u[0]*v[3]-u[3]*v[0];
	A[1][3] = u[2]*v[0]-u[0]*v[2];
	A[2][0] = -A[0][2];
	A[2][1] = -A[1][2];
	A[2][2] = 0.0f;
	A[2][3] = u[0]*v[1]-u[1]*v[0];
	A[3][0] = -A[0][3];
	A[3][1] = -A[1][3];
	A[3][2] = -A[2][3];
	A[3][3] = 0.0f;

	/* A2 = A*A */
	for( row=0; row<4; row++ )
	for( col=0; col<4; col++ ) {
		A2[row][col] = 0;
		for( mid=0; mid<4; mid++ )
			A2[row][col] += A[row][mid]*A[mid][col];
	}

	sn = GetSin(angle);
	omcs = 1.0f - GetCos(angle);

	/* rotation is I+sin(angle)*A+[1-cos(angle)]*A*A */
	for( row=0; row<4; row++ )
	for( col=0; col<4; col++ )
		rotate[row][col] = I[row][col]+sn*A[row][col]+omcs*A2[row][col];

	return;
}




#if 0

//---------------------------------------------------------------------------
// Name: matrix_view()
// Desc: Given an eye point, a lookat point, and an up vector, this
//       function builds a 4x4 view matrix.
//---------------------------------------------------------------------------
void matrix_view( float *m, float *from, float *lookat, float *worldup ) {

	float dot, view[3], up[3], right[3];

	// Get the z basis vector, which points straight ahead. This is the
	// difference from the eyepoint to the lookat point.
	view[0] = lookat[0] - from[0];
	view[1] = lookat[1] - from[1];
	view[2] = lookat[2] - from[2];
	vector_normalize(view);

	// Get the dot product, and calculate the projection of the z basis
	// vector onto the up vector. The projection is the y basis vector.
	dot = vector_dot(worldup, view);

	up[0] = worldup[0] - (view[0]*dot);
	up[1] = worldup[1] - (view[1]*dot);
	up[2] = worldup[2] - (view[2]*dot);
	vector_normalize(up);

	vector_cross(up, view, right);

	MTX(m,0,0) = right[0];
	MTX(m,1,0) = right[1];
	MTX(m,2,0) = right[2];
	MTX(m,3,0) = -vector_dot(from, right);

	MTX(m,0,1) = up[0];
	MTX(m,1,1) = up[1];
	MTX(m,2,1) = up[2];
	MTX(m,3,1) = -vector_dot(from, up);

	MTX(m,0,2) = view[0];
	MTX(m,1,2) = view[1];
	MTX(m,2,2) = view[2];
	MTX(m,3,2) = -vector_dot(from, view);

	MTX(m,0,3) = 0.0;
	MTX(m,1,3) = 0.0;
	MTX(m,2,3) = 0.0;
	MTX(m,3,3) = 1.0;

	return;
}



//---------------------------------------------------------------------------
// Name: matrix_proj()
// Desc: Makes a projection matrix. fov must be in radians.
//---------------------------------------------------------------------------
void matrix_proj( float *m, float fov, float aspect, float nearplane, float farplane ) {

	float cotfov, quot;

	fov *= 0.5;

	cotfov = cos(fov) / sin(fov);
	quot = farplane / (farplane - nearplane);

	memset(m, 0, sizeof(float)<<4);
	MTX(m,0,0) = cotfov;            // W
	MTX(m,1,1) = aspect * cotfov;   // H
	MTX(m,2,2) = quot;              // Q
	MTX(m,2,3) = 1.0f;
	MTX(m,3,2) = -quot * nearplane; // Q*NEARPLANE

	return;
}



//-----------------------------------------------------------------------------
// Name: matrix_fustrum_planes()
// Desc: Extracts the frustum planes from a combined view/projection matrix.
//-----------------------------------------------------------------------------
void matrix_frustrum_planes( float *m, float planes[6][4] ) {

	// Left clipping plane
	planes[0][0] =  (MTX(m,0,3) + MTX(m,0,0));
	planes[0][1] =  (MTX(m,1,3) + MTX(m,1,0));
	planes[0][2] =  (MTX(m,2,3) + MTX(m,2,0));
	planes[0][3] = -(MTX(m,3,3) + MTX(m,3,0));
	plane_normalize(planes[0]);

	// Right clipping plane
	planes[1][0] =  (MTX(m,0,3) - MTX(m,0,0));
	planes[1][1] =  (MTX(m,1,3) - MTX(m,1,0));
	planes[1][2] =  (MTX(m,2,3) - MTX(m,2,0));
	planes[1][3] = -(MTX(m,3,3) - MTX(m,3,0));
	plane_normalize(planes[1]);

	// Top clipping plane
	planes[2][0] =  (MTX(m,0,3) - MTX(m,0,1));
	planes[2][1] =  (MTX(m,1,3) - MTX(m,1,1));
	planes[2][2] =  (MTX(m,2,3) - MTX(m,2,1));
	planes[2][3] = -(MTX(m,3,3) - MTX(m,3,1));
	plane_normalize(planes[2]);

	// Bottom clipping plane
	planes[3][0] =  (MTX(m,0,3) + MTX(m,0,1));
	planes[3][1] =  (MTX(m,1,3) + MTX(m,1,1));
	planes[3][2] =  (MTX(m,2,3) + MTX(m,2,1));
	planes[3][3] = -(MTX(m,3,3) + MTX(m,3,1));
	plane_normalize(planes[3]);

	// Far clipping plane
	planes[4][0] =  (MTX(m,0,3) - MTX(m,0,2));
	planes[4][1] =  (MTX(m,1,3) - MTX(m,1,2));
	planes[4][2] =  (MTX(m,2,3) - MTX(m,2,2));
	planes[4][3] = -(MTX(m,3,3) - MTX(m,3,2));
	plane_normalize(planes[4]);

	// Near clipping plane
	planes[5][0] =  (MTX(m,0,3) + MTX(m,0,2));
	planes[5][1] =  (MTX(m,1,3) + MTX(m,1,2));
	planes[5][2] =  (MTX(m,2,3) + MTX(m,2,2));
	planes[5][3] = -(MTX(m,3,3) + MTX(m,3,2));
	plane_normalize(planes[5]);

	return;
}


#endif


#if 0
void main() {

	point3_t U,V,A;
	matrix_t b,c;
	matrix_t a = {
		{-1, 1, 2.1},
		{-3.4, 5.0, 56.8},
		{-3.4, 5.0, 56.8},
	};

	V[0] = 1;
	V[1] = 1;
	V[2] = 1;
//	  V[3] = 1;

	U[0] = 1;
	U[1] = 0;
	U[2] = 0;
//	  U[3] = 1;

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
	xprintf("b:\n");
	DumpMatrix(&b);
	xprintf("c:\n");
	DumpMatrix(&c);
	*/
	return;

}
#endif

