/* Copyright (C) 1997-99 Kirschner, Bernát. All Rights Reserved Worldwide. */
/* mailto: bernie@freemail.hu											   */
/* tel: +36 20 333 9517													   */

#ifndef _FIXED_H_INCLUDED
#define _FIXED_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#define FP_SHIFT 16

/* fixed point conversions */
#define INT_TO_FIXED(i)			((FIXED)( (i) << FP_SHIFT ))
#define FIXED_TO_INT(f)			((int)( (f) >> FP_SHIFT ))
#define FIXED_TO_FLOAT(f)		((double)(((double) (f)) * 1.52587890625e-5))
#define FLOAT_TO_FIXED(f)		((FIXED)( (f) * 65536.0) )
#define ROUND_FIXED_TO_INT(x)	(((x) + 0x8000) >> 16)

/* functions */
#define FIXED_ABS(f)				( (f) < 0 ? -(f) : (f) )
#define FIXED_TRUNC(f)				( (f) & 0xffff0000)
#define FIXED_SIGN(f)				( (unsigned int)(f) >> 31 )
#define FIXED_PRODUCT_SIGN(f, g)	( (unsigned int)( (f) ^ (g) ) >> 31 )
#define FIXED_HALF(f)				( (f) / 2 )
#define FIXED_DOUBLE(f)				( (f) << 1 )

/* perform integer scaling of a fixed point number */
#define FIXED_SCALE(f, i) ( (f) * (i) )

/* fixed point constants */
#define FIXED_ZERO		(INT_TO_FIXED(0))
#define FIXED_ONE		(INT_TO_FIXED(1))
#define FIXED_ONE_HALF	(FIXED_HALF(FIXED_ONE))
#define FIXED_PI		(FLOAT_TO_FIXED(3.14159265))
#define FIXED_2PI		(FLOAT_TO_FIXED(6.28318531))
#define FIXED_HALF_PI	(FLOAT_TO_FIXED(1.57079633))
#define FIXED_MIN		((FIXED)0xffffffff)
#define FIXED_MAX		((FIXED)0x7fffffff)
#define FIXED_EPSILON	((FIXED) 0x100)

#include <float.h>

#define FLOAT_MAX		(FLT_MAX)
#define FLOAT_MIN		(-FLT_MAX)
#define FLOAT_MIN_PLUS	(FLT_MIN)
#define FLOAT_EPSILON	((FLOAT)0.01)
#define FLOAT_ZERO		((FLOAT)0.000000000)


//
// constant from BC++ math.h
//
//#define M_E		2.71828182845904523536
//#define M_LOG2E	1.44269504088896340736
//#define M_LOG10E	0.434294481903251827651
//#define M_LN2 	0.693147180559945309417
#ifndef M_PI_4
#define M_PI_4	0.785398163397448309616
#endif
//#define M_1_PI	0.318309886183790671538
//#define M_2_PI	0.636619772367581343076
#define M_LN10		2.30258509299404568402
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif
#define M_PI_2		1.57079632679489661923
#define M_1_SQRTPI	0.564189583547756286948
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2 	1.41421356237309504880
#define M_SQRT_2	0.707106781186547524401



XLIBDEF FIXED fsin( FIXED );		  // GenTrig utan ONLY!!!
XLIBDEF FIXED fcos( FIXED );
XLIBDEF FIXED facos( FIXED );
XLIBDEF FIXED fasin( FIXED );
XLIBDEF FIXED fatan( FIXED );


/* Normally, you should define "better" functions for better sqrt,
   sin and cos using fixed point. For example, orthonormalize
   should be optimized for close-to-normal vectors already using
   taylor approx. If you need +/- .0002 precision for square root
   of numbers between .9604 and 1.0404, you can use sqrt(x)=(1+x)/2 */

/***
 *	eax - first argument
 *	edx - second argument
 *	ebx - third argument
 *	ecx - fourth argument
 ***/

XLIBDEF FIXED TriHtp( FIXED, FIXED, FIXED );
XLIBDEF FIXED ftrihyp(FIXED a, FIXED b, FIXED c);
// ftrihyp = sqrt ( a*a+b*b+c*c )
XLIBDEF FIXED fhyp(FIXED a, FIXED b);
// fhyp = sqrt ( a*a+b*b )

XLIBDEF FIXED falmosthyp(FIXED a, FIXED b);
// returns approximation to sqrt( a*a + b*b)

XLIBDEF FIXED f_double_div(FIXED denom_lo, FIXED denom_hi, FIXED nom, FIXED *dec_part);
XLIBDEF FIXED f_double_shift(FIXED *lo_ptr, FIXED *hi_ptr, FIXED aritm_flag, FIXED steps);

XLIBDEF long isqrt(long a);
// returns the integer!! root of a. NOT A FIXFLOAT ROUTINE !!

XLIBDEF FIXED ff_solve_2nd_poly(FIXED p, FIXED q, FIXED *conj_ptr, FIXED *pre_ptr);
// solves 2nd degree equation by std formula.
// nonzero return indicates error


XLIBDEF FIXED fmmd( FIXED a, FIXED b, FIXED c );
// ( a * d ) / c

XLIBDEF FIXED fmuldiv( FIXED a, FIXED b, FIXED c, FIXED d );
// ( a * b ) / ( c * d )

// __inline
XLIBDEF FIXED fmul( FIXED f1, FIXED f2 );
XLIBDEF FIXED fdiv( FIXED f1, FIXED f2 );
XLIBDEF FIXED fsqrt( FIXED f1 );


//
// DLLIMPORT extern __inline int ftoi( float f );
//
static __inline int ftoi( float f ) {

	int res;

#ifdef __GNUC__
	res = (int)f;
#else
	__asm {
		fld	f
		fistp	res
	}
#endif

	return res;
}


//
//
//
static __inline int ifloor( float x ) {

	unsigned long e = (0x7F + 31) - ((* (unsigned long *) &x & 0x7F800000) >> 23);
	unsigned long m = 0x80000000 | (* (unsigned long *) &x << 8);

	return (m >> e) & -(e < 32);
}



#define FIST_MAGIC2 ((((65536.0 * 16)+(0.5))* 65536.0))

//
//
//
static __inline long QuickInt16( float inval ) {

	double dtemp = FIST_MAGIC2 + inval;

	return ((*(long *)&dtemp) - 0x80000000);
}


#define FP_BITS(fp) (*(DWORD *)&(fp))
#define FP_ABS_BITS(fp) (FP_BITS(fp)&0x7FFFFFFF)
#define FP_SIGN_BIT(fp) (FP_BITS(fp)&0x80000000)
#define FP_ONE_BITS 0x3F800000


// r = 1/p
#define FP_INV(r,p) {                                                        \
    int _i = 2 * FP_ONE_BITS - *(int *)&(p);                                 \
    r = *(float *)&_i;                                                       \
    r = r * (2.0f - (p) * r);                                                \
}


#define FP_EXP(e,p) {                                                        \
    int _i;                                                                  \
    e = -1.44269504f * (float)0x00800000 * (p);                              \
    _i = (int)e + 0x3F800000;                                                \
    e = *(float *)&_i;                                                       \
}

#define FP_NORM_TO_BYTE(i,p) {                                               \
    float _n = (p) + 1.0f;                                                   \
    i = *(int *)&_n;                                                         \
    if (i >= 0x40000000)     i = 0xFF;                                       \
    else if (i <=0x3F800000) i = 0;                                          \
    else i = ((i) >> 15) & 0xFF;                                             \
}



//
//
//
static __inline unsigned long FP_NORM_TO_BYTE2( float p ) {

	float fpTmp = p + 1.0f;

	return ((*(unsigned *)&fpTmp) >> 15) & 0xFF;
}


//
//
//
static __inline unsigned long FP_NORM_TO_BYTE3(float p) {

	float ftmp = p + 12582912.0f;

	return ((*(unsigned long *)&ftmp) & 0xFF);
}



#define PLATFORM_WIN32 1
#define PLATFORM_LINUX 2
#define PLATFORM_APPLE 3

#define COMPILER_MSVC 1
#define COMPILER_GNUC 2
#define COMPILER_BORL 3

#if defined( _MSC_VER )
	#define XLIB_COMPILER COMPILER_MSVC
	#define XLIB_COMP_VER _MSC_VER

#elif defined( __GNUC__ )
	#define XLIB_COMPILER COMPILER_GNUC
	#define XLIB_COMP_VER __VERSION__

#elif defined( __BORLANDC__ )
	#define XLIB_COMPILER COMPILER_BORL
	#define XLIB_COMP_VER __BCPLUSPLUS__

#else
	#pragma error "No known compiler. Abort!"
#endif


/*=============================================================================
 ASM math routines posted by davepermen et al on flipcode forums
=============================================================================*/

#ifdef __cplusplus
static const float pi = (FLOAT)(4.0 * atan( 1.0 ));
static const float half_pi = (FLOAT)(0.5 * pi);
#endif

/*=============================================================================
	NO EXPLICIT RETURN REQUIRED FROM THESE METHODS!!
=============================================================================*/
#if XLIB_COMPILER == COMPILER_MSVC
	#pragma warning( push )
	#pragma warning( disable: 4035 )
#endif


//
// return half_pi + arctan( r / -sqr( 1.f - r * r ) );
//
static __inline float asm_arccos( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    float asm_one = 1.f;
    float asm_half_pi = half_pi;
    __asm {
        fld r // r0 = r
        fld r // r1 = r0, r0 = r
        fmul r // r0 = r0 * r
        fsubr asm_one // r0 = r0 - 1.f
        fsqrt // r0 = sqrtf( r0 )
        fchs // r0 = - r0
        fdiv // r0 = r1 / r0
        fld1 // {{ r0 = atan( r0 )
        fpatan // }}
        fadd asm_half_pi // r0 = r0 + pi / 2
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return (float)( acos( r ) );

#endif
}



//
// return arctan( r / sqr( 1.f - r * r ) );
//
static __inline float asm_arcsin( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    const float asm_one = 1.f;
    __asm {
        fld r // r0 = r
        fld r // r1 = r0, r0 = r
        fmul r // r0 = r0 * r
        fsubr asm_one // r0 = r0 - 1.f
        fsqrt // r0 = sqrtf( r0 )
        fdiv // r0 = r1 / r0
        fld1 // {{ r0 = atan( r0 )
        fpatan // }}
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return (float)( asin( r ) );

#endif

}


//
//
//
static __inline float asm_arctan( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    __asm {
        fld r // r0 = r
        fld1 // {{ r0 = atan( r0 )
        fpatan // }}
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return (float)( atan( r ) );

#endif

}


//
//
//
static __inline float asm_sin( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    __asm {
        fld r // r0 = r
        fsin // r0 = sinf( r0 )
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return sin( r );

#endif

}



//
//
//
static __inline float asm_cos( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    __asm {
        fld r // r0 = r
        fcos // r0 = cosf( r0 )
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return cos( r );

#endif
}



//
//
//
static __inline float asm_tan( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    // return sin( r ) / cos( r );
    __asm {
        fld r // r0 = r
        fsin // r0 = sinf( r0 )
        fld r // r1 = r0, r0 = r
        fcos // r0 = cosf( r0 )
        fdiv // r0 = r1 / r0
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return tan( r );

#endif
}


//
// returns a for a * a = r
//
static __inline float asm_sqrt( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    __asm {
        fld r // r0 = r
        fsqrt // r0 = sqrtf( r0 )
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return sqrt( r );

#endif
}


//
// returns 1 / a for a * a = r
// -- Use this for Vector normalisation!!!
//
static __inline float asm_rsq( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    __asm {
        fld1 // r0 = 1.f
        fld r // r1 = r0, r0 = r
        fsqrt // r0 = sqrtf( r0 )
        fdiv // r0 = r1 / r0
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return 1.0 / sqrt( r );

#endif
}


//
// returns 1 / a for a * a = r
// Another version
//
static __inline float apx_rsq( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    const float asm_dot5 = 0.5f;
    const float asm_1dot5 = 1.5f;

    __asm {
        fld r // r0 = r
        fmul asm_dot5 // r0 = r0 * .5f
        mov eax, r // eax = r
        shr eax, 0x1 // eax = eax >> 1
        neg eax // eax = -eax
        add eax, 0x5F400000 // eax = eax & MAGICAL NUMBER
        mov r, eax // r = eax
        fmul r // r0 = r0 * r
        fmul r // r0 = r0 * r
        fsubr asm_1dot5 // r0 = 1.5f - r0
        fmul r // r0 = r0 * r
    } // returns r0

#elif XLIB_COMPILER == COMPILER_GNUC

	return 1.0 / sqrt( r );

#endif
}



#if XLIB_COMPILER == COMPILER_MSVC

//
// very MS-specific, commented out for now
// Finally the best InvSqrt implementation?
// Use for vector normalisation instead of 1/length() * x,y,z
//
__declspec(naked) static float __fastcall InvSqrt( float fValue ) {

	__asm {
        mov        eax, 0be6eb508h
        mov        dword ptr[esp-12],03fc00000h
        sub        eax, dword ptr[esp + 4]
        sub        dword ptr[esp+4], 800000h
        shr        eax, 1
        mov        dword ptr[esp -  8], eax

        fld        dword ptr[esp -  8]
        fmul    st, st
        fld        dword ptr[esp -  8]
        fxch    st(1)
        fmul    dword ptr[esp +  4]
        fld        dword ptr[esp - 12]
        fld        st(0)
        fsub    st,st(2)

        fld        st(1)
        fxch    st(1)
        fmul    st(3),st
        fmul    st(3),st
        fmulp    st(4),st
        fsub    st,st(2)

        fmul    st(2),st
        fmul    st(3),st
        fmulp    st(2),st
        fxch    st(1)
        fsubp    st(1),st

        fmulp    st(1), st
        ret 4
	}
}

#endif


//
// returns a random number
//
static __inline  float asm_rand( void ) {

#if XLIB_COMPILER == COMPILER_MSVC
  #if XLIB_COMP_VER == 1300

	static unsigned __int64 q = time( NULL );

	_asm {
		movq mm0, q

		// do the magic MMX thing
		pshufw mm1, mm0, 0x1E
		paddd mm0, mm1

		// move to integer memory location and free MMX
		movq q, mm0
		emms
	}

	return float( q );
  #else
    // VC6 does not support pshufw
    return float( rand() );
  #endif
#else
    // GCC etc

	return (float)( rand() );

#endif
}



//
// returns the maximum random number
//
static __inline  float asm_rand_max( void ) {

#if XLIB_COMPILER == COMPILER_MSVC
  #if XLIB_COMP_VER == 1300

	return std::numeric_limits< unsigned __int64 >::max();
	return 9223372036854775807.0f;
  #else
    // VC6 does not support unsigned __int64
    return float( RAND_MAX );
  #endif

#else
    // GCC etc
	return (float)( RAND_MAX );

#endif
}




//
// returns log2( r ) / log2( e )
//
static __inline float asm_ln( float r ) {

#if XLIB_COMPILER == COMPILER_MSVC

    const float asm_e = 2.71828182846f;
    const float asm_1_div_log2_e = .693147180559f;
    const float asm_neg1_div_3 = -.33333333333333333333333333333f;
    const float asm_neg2_div_3 = -.66666666666666666666666666667f;
    const float asm_2 = 2.f;

    int log_2 = 0;

    __asm {
        // log_2 = ( ( r >> 0x17 ) & 0xFF ) - 0x80;
        mov eax, r
        sar eax, 0x17
        and eax, 0xFF
        sub eax, 0x80
        mov log_2, eax

        // r = ( r & 0x807fffff ) + 0x3f800000;
        mov ebx, r
        and ebx, 0x807FFFFF
        add ebx, 0x3F800000
        mov r, ebx

        // r = ( asm_neg1_div_3 * r + asm_2 ) * r + asm_neg2_div_3;   // (1)
        fld r
        fmul asm_neg1_div_3
        fadd asm_2
        fmul r
        fadd asm_neg2_div_3
        fild log_2
        fadd
        fmul asm_1_div_log2_e
    }

#elif XLIB_COMPILER == COMPILER_GNUC

	return log( r );

#endif
}

#if XLIB_COMPILER == COMPILER_MSVC
#pragma warning( pop )
#endif



#if 0

// Turn off warnings generated by long std templates
// This warns about truncation to 255 characters in debug/browse info
#pragma warning (disable : 4786)

// disable: "conversion from 'double' to 'float', possible loss of data
#pragma warning (disable : 4244)

// disable: "truncation from 'double' to 'float'
#pragma warning (disable : 4305)

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#pragma warning (disable : 4251)

// disable: "non dll-interface class used as base for dll-interface class"
// Happens when deriving from Singleton because bug in compiler ignores
// template export
#pragma warning (disable : 4275)

// disable: "C++ Exception Specification ignored"
// This is because MSVC 6 did not implement all the C++ exception
// specifications in the ANSI C++ draft.
#pragma warning( disable : 4290 )

// disable: "no suitable definition provided for explicit template
// instantiation request" Occurs in VC7 for no justifiable reason on all
// #includes of Singleton
#pragma warning( disable: 4661)

#endif


#ifdef __cplusplus

//
//
//
class vec3_c {

public:

	vec3_c() { v[0] = v[1] = v[2] = 0.0f; }
	vec3_c( FLOAT px, FLOAT py, FLOAT pz ) { v[0] = px; v[1] = py; v[2] = pz; }
	vec3_c( double px, double py, double pz ) { v[0] = (FLOAT)px; v[1] = (FLOAT)py; v[2] = (FLOAT)pz; }
	vec3_c( vec3_c &pVec ) { v[0] = pVec.v[0]; v[1] = pVec.v[1]; v[2] = pVec.v[2]; }
	vec3_c( FLOAT *pVec ) { v[0] = pVec[0]; v[1] = pVec[1]; v[2] = pVec[2]; }

	vec3_c operator=( vec3_c &pVec )	  { return vec3_c( v[0] = pVec.v[0], v[1] = pVec.v[1], v[2] = pVec.v[2] ); }
	vec3_c operator=( FLOAT *ptr )		  { return vec3_c( v[0] = ptr[0], v[1] = ptr[1], v[2] = ptr[2] ); }
	int operator==( vec3_c &pVec )		  { return (v[0] == pVec.v[0] && v[1] == pVec.v[1] && v[2] == pVec.v[2]); }
	int operator==( FLOAT *pVec)		  { return (v[0] == pVec[0] && v[1] == pVec[1] && v[2] == pVec[2]); }
	inline int operator!=( vec3_c &pVec ) { return !(pVec == (*this)); }
	inline int operator!=( FLOAT *pVec )  { return !(pVec == (*this)); }

	vec3_c operator+=( vec3_c &pVec ) { vec3_c ret; ret = *this = *this + pVec; return ret; }
	vec3_c operator-=( vec3_c &pVec ) { vec3_c ret; ret = *this = *this - pVec; return ret; }
	vec3_c operator*=( vec3_c &pVec ) { vec3_c ret; ret = *this = *this * pVec; return ret; }
	vec3_c operator*=( FLOAT val )    { vec3_c ret; ret = *this = *this * val;  return ret; }
	vec3_c operator/=( vec3_c &pVec)  { vec3_c ret; ret = *this = *this / pVec; return ret; }
	vec3_c operator/=( FLOAT val )    { vec3_c ret; ret = *this = *this / val;  return ret; }

	vec3_c operator+( vec3_c &pVec ) { return vec3_c(v[0] + pVec.v[0], v[1] + pVec.v[1], v[2] + pVec.v[2]); }
	vec3_c operator-( vec3_c &pVec ) { return vec3_c(v[0] - pVec.v[0], v[1] - pVec.v[1], v[2] - pVec.v[2]); }
	vec3_c operator*( vec3_c &pVec ) { return vec3_c(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2]); }
	vec3_c operator*( FLOAT val )    { return vec3_c(v[0] * val, v[1] * val, v[2] * val); }
	vec3_c operator/( vec3_c &pVec ) { return vec3_c(v[0] / pVec.v[0], v[1] / pVec.v[1], v[2] / pVec.v[2]); }
	vec3_c operator/( FLOAT val )    { return vec3_c(v[0] / val, v[1] / val, v[2] / val); }

	void Clear( void ) { v[0] = v[1] = v[2] = 0.0f; }

	//
	//
	//
	void Normalize( void ) {

		FLOAT length, len = 0.0f;

		length = Length();

		if( length == 0.0f )
			return;

		len = 1.0f / length;

		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}


	//
	//
	//
	FLOAT Length( void ) {

		FLOAT length = 0.0f;

		length = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);

		return (FLOAT)sqrt( length );
	}

	FLOAT Dot( vec3_c &pVec ) { return v[0] * pVec.v[0] + v[1] * pVec.v[1] + v[2] * pVec.v[2]; }


	//
	//
	//
	void Cross( vec3_c &p, vec3_c &q ) {

		v[0] = p.v[1] * q.v[2] - p.v[2] * q.v[1];
		v[1] = p.v[2] * q.v[0] - p.v[0] * q.v[2];
		v[2] = p.v[0] * q.v[1] - p.v[1] * q.v[0];

		return;
	}


	void Set( FLOAT x, FLOAT y, FLOAT z) { v[0] = x; v[1] = y; v[2] = z; }

	FLOAT X( void ) { return v[0]; }
	FLOAT Y( void ) { return v[1]; }
	FLOAT Z( void ) { return v[2]; }
	void X( FLOAT nx ) { v[0] = nx; }
	void Y( FLOAT ny ) { v[1] = ny; }
	void Z( FLOAT nz ) { v[2] = nz; }

	const FLOAT &operator[]( int ndx ) const { return v[ndx]; }
	FLOAT &operator[]( int ndx ) { return v[ndx]; }
	operator FLOAT*( void ) { return v; }

	//
	//
	//
	void Clamp( FLOAT min, FLOAT max ) {

		if( (v[0] > max) || (v[0] < min) )
			v[0] = 0.0f;

		if( (v[1] > max) || (v[1] < min) )
			v[1] = 0.0f;

		if( (v[2] > max) || (v[2] < min) )
			v[2] = 0.0f;

		return;
	}

	//
	//
	//
	void RotateX( FLOAT amnt ) {

		FLOAT s = (FLOAT)sin( amnt );
		FLOAT c = (FLOAT)cos( amnt );
		FLOAT y = v[1];
		FLOAT z = v[2];

		v[1] = (y * c) - (z * s);
		v[2] = (y * s) + (z * c);

		return;
	}

	//
	//
	//
	void RotateY( FLOAT amnt ) {

		FLOAT s = (FLOAT)sin( amnt );
		FLOAT c = (FLOAT)cos( amnt );
		FLOAT x = v[0];
		FLOAT z = v[2];

		v[0] = (x * c) + (z * s);
		v[2] = (z * c) - (x * s);

		return;
	}

	//
	//
	//
	void RotateZ( FLOAT amnt ) {

		FLOAT s = (FLOAT)sin( amnt );
		FLOAT c = (FLOAT)cos( amnt );
		FLOAT x = v[0];
		FLOAT y = v[1];

		v[0] = (x * c) - (y * s);
		v[1] = (y * c) + (x * s);

		return;
	}


	// friend ostream &operator<<(ostream &os, vec3_c &vec) { os << vec.v[0] << " " << vec.v[1] << " " << vec.v[2]; return os; }

	protected:

		point3_t v;
};
#endif


// half.cpp

#ifndef HALF_H
#define HALF_H

// uint16_t
XLIBDEF float half_to_float( int h );
XLIBDEF int float_to_half( float fl );
XLIBDEF int half_add( int arg0, int arg1 );

//
// (a-b) is the same as (a+(-b))
//
static inline int half_sub( int ha, int hb ) {
	return half_add( ha, hb ^ 0x8000 );
}

#endif /* HALF_H */



#ifdef __cplusplus
}
#endif

#endif
