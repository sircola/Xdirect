/* Copyright (C) 1997 Kirschner, Bernát. All Rights Reserved Worldwide. */


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <xlib.h>
#include <xinner.h>



RCSID( "$Id: fx.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )


// Light


static light_t *light_header = NULL, *light_last = NULL;



//
//
//
void GetLightRgb( point3_t v, point3_t pointnormal, rgb_t *rgb ) {

	light_t *light;
	FLOAT dist,dot,scale;

	rgb->r = 0; rgb->g = 0; rgb->b = 0;

	for( light = light_header; light != NULL; light = light->next ) {

		if( light->type == LT_NORMAL ) {

			dot = DotProduct( light->vdir, pointnormal );
			CLAMPMIN( dot, 0.0f );

			rgb->r += ftoi( dot * (FLOAT)light->r );
			rgb->g += ftoi( dot * (FLOAT)light->g );
			rgb->b += ftoi( dot * (FLOAT)light->b );
		}
		else
		if( light->type == LT_POINT ) {

			dist = DistanceVector( v, light->pos );

			/***
			if( dist < light->dist ) {
				*r += light->r;
				*g += light->g;
				*b += light->b;
			}
			***/

			// Physically, light intensity falls off with the square of distance travelled.
			// i1 = i0 / 4*PI*(d*d);
			// scale = 1.0f / (4.0f*(FLOAT)M_PI*(dist*dist));

			if( dist <= light->dist ) {

				scale = (1.0f - ( dist / light->dist ));

				rgb->r += ftoi( (FLOAT)(light->r) * scale );
				rgb->g += ftoi( (FLOAT)(light->g) * scale );
				rgb->b += ftoi( (FLOAT)(light->b) * scale );
			}
		}
		else
		if( light->type == LT_AMBIENT ) {

			rgb->r += light->r;
			rgb->g += light->g;
			rgb->b += light->b;
		}
	}

	CLAMPMINMAX( rgb->r, 0, 255 );
	CLAMPMINMAX( rgb->g, 0, 255 );
	CLAMPMINMAX( rgb->b, 0, 255 );

	return;
}







//
//
//
BOOL RemoveLight( light_t *light ) {

	if( light == NULL )
		return TRUE;

	if( (light == light_header) && (light == light_last) ) {
		light_header = NULL;
		light_last = NULL;
	}
	else
	if( light == light_header ) {
		light_header = light_header->next;
		if( light_header ) light_header->prev = NULL;
	}
	else
	if( light == light_last ) {
		light_last = light_last->prev;
		if( light_last ) light_last->next = NULL;
	}
	else {
		if( light->prev ) light->prev->next = light->next;
		if( light->next ) light->next->prev = light->prev;
	}

	FREEMEM( light );

	return TRUE;
}




//
//
//
light_t *AddLight( lighttype_t type, int r,int g,int b,int a, FLOAT dist, point3_t pos, point3_t dir ) {

	light_t *light;

	if( type <= LT_NONE || type >= LT_NUMTYPES ) {
		xprintf("AddLight: unknown type.\n");
		return NULL;
	}

	ALLOCMEM( light, sizeof(light_t) );

	memset( light, 0, sizeof(light_t) );

	light->type = type;

	if( light->type != LT_AMBIENT ) {

		MAKEVECTOR( light->pos, 1,1,1 );
		MAKEVECTOR( light->dir, 0,0,0 );

		if( pos ) CopyVector( light->pos, pos );
		if( dir ) CopyVector( light->dir, dir );

		SubVector( light->dir, light->pos, light->vdir );
		NormalizeVector( light->vdir );
	}

	light->r = r;
	light->g = g;
	light->b = b;
	light->a = a;

	light->dist = dist;

	if( light_header == NULL ) {
		light_header = light;
		light_last = light;
		light_header->next = NULL;
		light_header->prev = NULL;
	}
	else {
		light_last->next = light;
		light->prev = light_last;
		light->next = NULL;
		light_last = light;
	}

	return light;
}







// Shade


UCHAR *shadetable; 			// [][0] melyik szin
					// [][1] melyik + hany darab
					// [][2] mekkora a dZ lepes
static FLOAT shadenear = FLOAT_MAX - 1.0;
static FLOAT shadefar  = FLOAT_MAX;
static FLOAT shadestep = FLOAT_EPSILON; // ennyi tavolsagonkent fog
					// csokkeni
int shadeoffset = 0L;			// pal[ color + shadeoffset ];



//
//
//
void SetupShading( FLOAT sn, FLOAT sf ) {

	int color,i;

	if( shadetable ) FREEMEM( shadetable );
	ALLOCMEM( shadetable, 256 * COLORDEPTH );

	for( color = 0; color < 256; color++ ) {

		for( i=0; i<COLORDEPTH; i++ ) {

			STABLE(color,i) = (UCHAR)color;
		}
	}

	shadenear = sn;
	shadefar  = sf;

	shadestep = (shadefar - shadenear) / COLORDEPTH;

	//xprintf("sn: %.2f, sf: %.2f, ss: %.2f\n",sn,sf,shadestep);

	return;
}



//
//
//
void AddShadeColor( int c, int num ) {

	int i,color;

	for( color=c; color<c+num; color++,num-- ) {

		for( i=0; i<COLORDEPTH; i++ ) {

			STABLE(color,i) = (color + ((i<num)?i:(((num-1)<0)?0:(num-1))));

			//xprintf("st[%d][%d] = %d\n",color,i,STABLE(color,i));
		}
	}

	return;
}






// Phong & Environment
// Henri Gouraud and Bui-Tuong Phong

UCHAR *environment_map=NULL;		  // phong_map
static int phong_depth = 0L;


//
//
//
void DeinitPhongMap( void ) {

	if( environment_map ) {
		FREEMEM( environment_map );
		environment_map = NULL;
	}

	phong_depth = 0L;

	return;
}





//
//
//
void MakePhongMap( int depth ) {

	int i,j;

	xprintf("phong map: creating...");

	DeinitPhongMap();

	phong_depth = depth-1;

	ALLOCMEM( environment_map, 256*256 );

	for( i=0; i<256; i++ )
	for( j=0; j<256; j++ ) {

		environment_map[i*256+j] = (UCHAR)(sqr(sqr(GetSin(deg2rad((float)i/81.487f))))*sqr(sqr(GetSin(deg2rad((float)j/81.487f))))*(depth-1))+1;

	}

	xprintf("\n");

	return;
}





//
//
//
void StretchPhongMap( int clip ) {

	int i;

	for( i=0; i<(256*256); i++ ) {

		if( environment_map[i] + clip < phong_depth )
			environment_map[i] += clip;

	}

	return;
}






//
//
//
void MakeLensTexture( void ) {

	UCHAR *spr,pal[768];
	int flag;

	MakePhongMap( 128 );

	ALLOCMEM( spr, SPRITEHEADER+256*256 );
	MKSPRW( spr, 256 );
	MKSPRH( spr, 256 );
	MKSPR8( spr );

	memcpy( &spr[SPRITEHEADER], environment_map, 256*256 );

	flag = 0;
	SETFLAG( flag, TF_ALPHA );

	AddTexMapHigh( spr, pal, "LENS", "phong.bmp", flag, 0L );

	FREEMEM( spr );

	DeinitPhongMap();

	return ;
}



// Fog

FLOAT fognear,fogfar;
FIXED ffognear,ffogfar;



//
//
//
void SetupFog( FLOAT fn, FLOAT ff, ULONG fogcolor ) {

	fognear = fn;
	fogfar = ff;

	ffognear = FLOAT_TO_FIXED( fn );
	ffogfar = FLOAT_TO_FIXED( ff );

	if( GFXDLL_Fog ) GFXDLL_Fog( fn, ff, fogcolor );

	return;
}






///////// perlin

#define IBUFSIZE (1<<16)


static BOOL perlin_inited = FALSE;

static FLOAT perlin_buffer[4096];
static UCHAR perlin_ucbuf[IBUFSIZE];
static UCHAR perlin_wtb[256];

extern FLOAT PerlinGet( FLOAT x );
extern FLOAT PerlinGet2D( FLOAT x, FLOAT y );
extern FLOAT PerlinGet3D( FLOAT x, FLOAT y, FLOAT t );
extern UCHAR PerlinGetI3D( FLOAT x, FLOAT y, FLOAT t );
extern UCHAR PerlinGetI3DI( UINT xfp, UINT yfp, UINT tfp );


#define YWRAP	16
#define YWRAPB	4
#define TWRAP	256
#define TWRAPB	8

#define OCTAVEN 2


//
// __forceinline
//
static FLOAT __inline fsc( float i ) {

	return 0.5f*(1.0f-cos(i*3.1415926535f));
}



//
//
//
static void InitPerlin( void ) {

	int i;

	for( i=0; i<4096; i++ )
		perlin_buffer[i] = rand()/32767.0f;

	for( i=0; i<IBUFSIZE; i++ )
		perlin_ucbuf[i] = rand();

	for( i=0; i<256; i++ )
		perlin_wtb[i] = 255*fsc(i/256.0f);

	perlin_inited = TRUE;

	return;
}







//
//
//
FLOAT PerlinGet( FLOAT x ) {

	if( perlin_inited == FALSE )
		InitPerlin();

	FLOAT r = 0.0f;
	FLOAT ampl = 0.5f;
	int i;

	if( x < 0 ) x = -x;
	int xi = (int)x;
	FLOAT xf = x-xi;

	for( i=0; i<OCTAVEN; i++ ) {

		FLOAT n1;

		n1 = perlin_buffer[xi&4095];
    	n1 += fsc(xf)*(perlin_buffer[(xi+1)&4095]-n1);
		r  += n1*ampl;
		ampl *= 0.5f;
		xi<<=1; xf*=2;

		if( xf >= 1.0f) xi++, xf-=1.0f;
	}
	return r;
}



//
//
//
FLOAT PerlinGet2D( FLOAT x, FLOAT y ) {

	if( perlin_inited == FALSE )
		InitPerlin();

	int i;

	if( x<0 ) x = -x;
	if( y<0 ) y = -y;
	int xi=x, yi=y;

	FLOAT r=0;
	FLOAT ampl=0.5f;
	FLOAT xf=x-xi, yf=y-yi, rxf;

	xf = yf = 0.0f;

	for( i=0; i<OCTAVEN; i++ ) {

		int of = xi + (yi<<YWRAPB);
		FLOAT n1,n2;

		rxf = fsc(xf);

		n1 = perlin_buffer[of&4095];
		n1 += rxf*(perlin_buffer[(of+1)&4095]-n1);
		n2 = perlin_buffer[(of+YWRAP)&4095];
		n2 += rxf*(perlin_buffer[(of+YWRAP+1)&4095]-n2);
		n1 += fsc(yf)*(n2-n1);

		r  += n1*ampl;

		ampl *= 0.5f;
		xf *= 2; xi<<=1;
		yf *= 2; yi<<=1;

		if( xf>=1.0f ) xi++, xf-=1.0f;
		if( yf>=1.0f ) yi++, yf-=1.0f;
	}

	return r;
}




//
//
//
FLOAT PerlinGet3D( FLOAT x, FLOAT y, FLOAT t ) {

	if( perlin_inited == FALSE )
		InitPerlin();

	int i;

	if( x<0 ) x = -x;
	if( y<0 ) y = -y;
	if( t<0 ) t = -t;

	int xi=x, yi=y, ti=t;
	FLOAT xf = x-xi;
	FLOAT yf = y-yi;
	FLOAT tf = t-ti;
	FLOAT rxf, ryf;

	FLOAT r=0;
	FLOAT ampl=0.5f;

	for( i=0; i<OCTAVEN; i++ ) {

		int of=xi+(yi<<YWRAPB)+(ti<<TWRAPB);
		float n1,n2,n3;

		rxf = fsc(xf);
		ryf = fsc(yf);

		n1  = perlin_buffer[of&4095];
		n1 += rxf*(perlin_buffer[(of+1)&4095]-n1);
		n2  = perlin_buffer[(of+YWRAP)&4095];
		n2 += rxf*(perlin_buffer[(of+YWRAP+1)&4095]-n2);
		n1 += ryf*(n2-n1);

		of += TWRAP;
		n2  = perlin_buffer[of&4095];
		n2 += rxf*(perlin_buffer[(of+1)&4095]-n2);
		n3  = perlin_buffer[(of+YWRAP)&4095];
		n3 += rxf*(perlin_buffer[(of+YWRAP+1)&4095]-n3);
		n2 += ryf*(n3-n2);

		n1 += fsc(tf)*(n2-n1);

		r += n1*ampl;
		ampl *= 0.5f;
		xi<<=1; xf*=2;
		yi<<=1; yf*=2;
		ti<<=1; tf*=2;

		if( xf>=1.0f ) xi++, xf-=1.0f;
		if( yf>=1.0f ) yi++, yf-=1.0f;
		if( tf>=1.0f ) ti++, tf-=1.0f;
	}

	return r;
}



//
//
//
UCHAR PerlinGetI3DI( UINT xfp, UINT yfp, UINT tfp ) {

	if( perlin_inited == FALSE )
		InitPerlin();

	int r = 0;

	// in is 16.16 fixed point

	int amplshift = 17;
	while (amplshift<25) {		// do 7 ovtaves.. way enough for char result

		unsigned int xindex = (xfp>>16)&0xffff;
		unsigned int yindex = ((yfp>>16)&0xffff)<<5; // ROL BLAH!!! .. hate c
		unsigned int tindex = ((tfp>>16)&0xffff)<<10; // ROL BLAH!!!
		int xf = perlin_wtb[(xfp>>8)&0xff];
		int yf = perlin_wtb[(yfp>>8)&0xff];
		int tf = perlin_wtb[(tfp>>8)&0xff];
		unsigned int off=xindex+yindex+tindex;

		int n1 = perlin_ucbuf [off&0xffff];
		int n2 = perlin_ucbuf [(off+1)&0xffff];
		int nh1 = (n1<<8) + (n2-n1)*xf; // 8.8
		n1 = perlin_ucbuf [(off+32)&0xffff];
		n2 = perlin_ucbuf [(off+33)&0xffff];
		int nh2 = (n1<<8) + (n2-n1) * xf; // 8.8
		int nv1 = ((nh1<<8) +  (nh2-nh1) * yf) >> 8; // 8.8

		n1 = perlin_ucbuf[ (off+1024)&0xffff ];
		n2 = perlin_ucbuf[ (off+1025)&0xffff ];
		nh1 = (n1<<8) + (n2-n1) * xf; // .8
		n1 = perlin_ucbuf[ (off+1056)&0xffff ];
		n2 = perlin_ucbuf[ (off+1057)&0xffff ];
		nh2 = (n1<<8) + (n2-n1) * xf; // .8
		int nv2 = ((nh1<<8) +  (nh2-nh1) * yf) >> 8; // .8

		int n = ((nv1<<8) + (nv2-nv1)*tf)>>(amplshift);

		r+=n;
		xfp<<=1; yfp<<=1; tfp<<=1;
		amplshift++;
	}

	return r;
}




//
//
//
UCHAR PerlinGetI3D( FLOAT x, FLOAT y, FLOAT t ) {

	if( perlin_inited == FALSE )
		InitPerlin();

	// convert float to fixed point
	unsigned int xfp = (unsigned int)(x*(FLOAT)((1<<16)-1));
	unsigned int yfp = (unsigned int)(y*(FLOAT)((1<<16)-1));
	unsigned int tfp = (unsigned int)(t*(FLOAT)((1<<16)-1));

	return PerlinGetI3DI( xfp, yfp, tfp );
}











#if 0

light ppSpotLight ( float intensity = 1;
					color lightcolor = 1;
					point from = point "shader" (0,0,0);
					point to = point "shader" (0,0,1);
					float conefull = 0.349;
					float coneoff = 0.7854;
					float fadebegin = 0;
					float fadeend = 0; ) {

	float atten, cosangle, dist, dist2;
	uniform vector A = normalize(to-from);

	illuminate( from ) {

		if( fadebegin >= 0 && fadeend > 0 ) {

			dist2 = L . L;
			dist = sqrt(dist2);

			if( dist > fadebegin ) {
				if( dist < fadeend ) {
					atten = ((fadeend*fadeend) - dist2) /
							((fadeend*fadeend) - (fadebegin*fadebegin));
				}
				else {
					atten = 0.0;
				}
			}
			else {
				atten = 1.0;
			}
		}
		else {
			atten = 1.0;
		}

		cosangle = (L . A) / length(L);
		atten *= smoothstep (cos(coneoff), cos(conefull), cosangle);
		Cl = atten * intensity * lightcolor ;
	}
}


#endif
