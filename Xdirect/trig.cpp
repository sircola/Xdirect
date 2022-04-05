/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#include <math.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: trig.c,v 1.0 97-03-10 17:28:13 bernie Exp $" )


static FLOAT *sintable = NULL;		// 180 fok,  1/10 pontossaggal
static FIXED *fsintable = NULL; 	//	     1/8
static FIXED *facostable = NULL;	// -1 ... 1  256
static FIXED *fatantable = NULL;	//
static FIXED max_atan,min_atan;

static FIXED *sqrt_table = NULL;



//
//
//
FLOAT GetSin( FLOAT x ) {

	return (FLOAT)sin(x);

	FLOAT result;

	while( x>=(FLOAT)(M_PI*2.0) ) x-=(FLOAT)(M_PI*2.0);
	while( x<0.0f ) x+=(FLOAT)(M_PI*2.0);

	result = (x<(FLOAT)M_PI)? sintable[ (int)(x*5729.577951f) ] : -sintable[ (int)((x-M_PI)*5729.577951f) ];

	return result;
}



//
//
//
FLOAT GetCos( FLOAT x ) {

	return (FLOAT)cos(x);

	return GetSin( x + (FLOAT)M_PI_2 );
}



//
//
//
FLOAT GetAsin( FLOAT x ) {

	return (FLOAT)asin( x );
}


//
//
//
FLOAT GetAcos( FLOAT x ) {

	if( FABS(x - 1.0f) < FLOAT_EPSILON )
		return 0.0f;

	if( FABS(x + 1.0f) < FLOAT_EPSILON )
		return (FLOAT)M_PI;

	return (FLOAT)acos( x );
}



//
//
//
FLOAT GetAtan( FLOAT x ) {

	return (FLOAT)atan( x );
}





//
//
//
FIXED fsin( FIXED x ) {

	FIXED result;

	while( x >= INT_TO_FIXED(360) ) x -= INT_TO_FIXED(360);
	while( x < FIXED_ZERO ) x += INT_TO_FIXED(360);

	result = (x<INT_TO_FIXED(180))? fsintable[ (int)(x>>(FP_SHIFT-3)) ] : -fsintable[ (int)((x-INT_TO_FIXED(180))>>(FP_SHIFT-3)) ];

	return result;
}



//
//
//
FIXED fcos( FIXED x ) {

	return fsin( x + INT_TO_FIXED(90) );
}



//
//
//
FIXED facos( FIXED x ) {

	x &= 0x8001FFFF;   // -1 ... 1

	return facostable[ ((x+FIXED_ONE)>>9) & 0xFF ];
}



//
//
//
FIXED fasin( FIXED x ) {

	return	FIXED_HALF_PI - facos( x );
}





//
//
//
FIXED fatan( FIXED x ) {

	if( x<=min_atan )
		return fatantable[0];
	else
	if( x>=max_atan )
		return fatantable[1023];
	else
	if( x >= FIXED_ZERO )
		return fatantable[((x>>13)&511)+512];
	else
	if( x < FIXED_ZERO )
		return fatantable[((x>>13)&511)];

	return FIXED_ZERO;
}




//
// original C version
//
FLOAT ffsqrt( FLOAT n ) {

	unsigned int *num = (unsigned *)&n;	// to access the bits of a float in C
										// we must misuse pointers
	short e;							// the exponent
	if( n == 0 ) return 0.0f;			/* check for square root of 0 */
    e = (*num >> 23) - 127;				/* get the exponent - on a SPARC the */
										/* exponent is stored with 127 added */
    *num &= 0x7fffff;					/* leave only the mantissa */
    if( e & 0x01 ) *num |= 0x800000;
										/* the exponent is odd so we have to */
										/* look it up in the second half of  */
										/* the lookup table, so we set the high bit */
	e >>= 1;							/* divide the exponent by two */
										/* note that in C the shift */
										/* operators are sign preserving */
										/* for signed operands */

	// Do the table lookup, based on the quaternary mantissa,
    // then reconstruct the result back into a float
    *num = ((sqrt_table[*num >> 16])) + ((e + 127) << 23);

	return n;
}






//
// Fast square root functions:
//
static void BuildSqrtTable( void ) {

	unsigned short i;
	float f;
	unsigned int *fi = (unsigned*)&f;	// To access the bits of a float in
						// C quickly we must misuse pointers

	REALLOCMEM( sqrt_table, (0x100)*sizeof(FIXED) );

	for( i=0; i<=0x7f; i++ ) {

		*fi = 0;

		// Build a float with the bit pattern i as mantissa
		// and an exponent of 0, stored as 127
		*fi = (i << 16) | (127 << 23);
		f = (FLOAT)sqrt(f);

		// Take the square root then strip the first 7 bits of
		// the mantissa into the table
		sqrt_table[i] = (*fi & 0x7fffff);

		// Repeat the process, this time with an exponent of 1,
		// stored as 128
		*fi = 0;
		*fi = (i << 16) | (128 << 23);
		f = (FLOAT)sqrt(f);
		sqrt_table[i+0x80] = (*fi & 0x7fffff);
	}

	return;
}





//
//
//
void GenTrig( void ) {

	double i;
	int j,cnt;

	xprintf("init: generating luts.\n");

	REALLOCMEM( sintable, (180*100)*sizeof(FLOAT) );
	REALLOCMEM( fsintable, (180*8)*sizeof(FIXED) );
	REALLOCMEM( facostable, (256)*sizeof(FIXED) );
	REALLOCMEM( fatantable, (1024)*sizeof(FIXED) );


	cnt = 0;
	for( i=0.0, j=0; j<180*100; i+=0.01, j++ )
		sintable[j] = (FLOAT)sin(FOK2RAD(i));


	cnt = 0;
	for( i=0.0, j=0; j<180*8; i+=0.125, j++ )
		fsintable[j] = FLOAT_TO_FIXED(sin(FOK2RAD(i)));


	cnt = 0;
	for( i=(-1.0), j=0; j<256; i+=(1.0/128.0), j++ )
		facostable[j] = FLOAT_TO_FIXED(acos(i));


	cnt = 0;
	for( i=tan(FOK2RAD(-89.5)), j=0; j<1024; i+=((2*tan(FOK2RAD(89.5)))/1024.0), j++ )
		fatantable[j] = FLOAT_TO_FIXED(atan(i));


	min_atan = FLOAT_TO_FIXED( tan( FOK2RAD(-89.5) ) );
	max_atan = FLOAT_TO_FIXED( tan( FOK2RAD(89.5) ) );

	BuildSqrtTable();

	return;
}










