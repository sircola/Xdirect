/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)


#include <math.h>
#include <limits.h>
#include <string.h>

#include <xlib.h>
#include <xinner.h>


RCSID( "$Id: rgb.c,v 1.0 97-03-10 17:34:41 bernie Exp $" )



UCHAR system_rgb[768];
UCHAR *transparent_rgb = NULL;		// [256*256]
UCHAR *aliasing_rgb = NULL;		// [256*256]


//
//
//
void Dither( int start, int num,
		int r1, int g1, int b1,
		int r2, int g2, int b2	) {


	int i,dr,dg,db,r,g,b;
	int anum = ABS(num),di;

	dr = ( (r2 - r1) * 256 ) / anum;
	dg = ( (g2 - g1) * 256 ) / anum;
	db = ( (b2 - b1) * 256 ) / anum;

	di = (num<0)?(-1):1 ;

	for( i=0; i<anum; i++ ) {

		r = ( (r1 * 256) + (dr * i) ) / 256;
		g = ( (g1 * 256) + (dg * i) ) / 256;
		b = ( (b1 * 256) + (db * i) ) / 256;

		SetRgb( start + (i*di), r,g,b );
	}

	return;
}






//
// phong illumination model
// color = specular + (cos x) * diffuse + (cos x)^n * specular
//
// intenzitas:
//	start ---> start+num
//
void Phong( int start, int num,
		int r1, int g1, int b1,
		int r2, int g2, int b2	) {

	int i,dr,dg,db;
	double r,g,b;
	double cos_theta,theta,dtheta;

	if( r1>r2 || g1>g2 || b1>b2 ) xprintf("Phong(): bad order.\n");

	dr = r2 - r1;
	dg = g2 - g1;
	db = b2 - b1;

	dtheta = M_PI_2 / (double)num;
	theta = 0.0;

	for( i=0; i<num; i+=(num<0)?(-1):1 ) {

		cos_theta = cos(theta);

		r = r1 + (double)dr * cos_theta;
		g = g1 + (double)dg * cos_theta;
		b = b1 + (double)db * cos_theta;

		SetRgb( start + num-(i+1), (int)r, (int)g, (int)b );

		theta += dtheta;
	}

	return;
}





//
//
//
void GetRgb( int color, int *red, int *green, int *blue ) {

	CLAMPMINMAX( color, 0, 255 );

	*red   = system_rgb[ (color*3) + 0 ];
	*green = system_rgb[ (color*3) + 0 ];
	*blue  = system_rgb[ (color*3) + 0 ];

	return;
}


//
//
//
void SetRgb(int c,int r,int g,int b) {

	CLAMPMINMAX( c, 0, 255 );

	system_rgb[(c*3)+0] = r;
	system_rgb[(c*3)+1] = g;
	system_rgb[(c*3)+2] = b;

	if( GFXDLL_SetRgb ) GFXDLL_SetRgb( c, r,g,b );

	return;
}




//
//
//
void SetRgbBuf( UCHAR *pal ) {

	int i;

	for( i=0; i<256; i++ )
		SetRgb( i, pal[(i*3)+0],
			   pal[(i*3)+1],
			   pal[(i*3)+2] );

	return;
}





//
//
//
void ConvRgb( UCHAR *pal ) {

	int i;

	for(i=0; i<768; i++)
		pal[i] >>= 2;

	return;
}




//
//
//
void SetRgb6(int c,int r,int g,int b) {

	SetRgb(c, (r>>2), (g>>2), (b>>2) );

	return;
}



//
//
//
void SetRgbBuf6( UCHAR *palbuf) {

	int i;
	UCHAR pal[768];


	for(i=0;i<768;i++)
		pal[i] = palbuf[i]>>2;

	SetRgbBuf(pal);

	return;
}




//
//
//
void SetDarkRgb(void) {

	UCHAR pal[768];

	memset(pal,0L,768);

	SetRgbBuf(pal);

	return;
}


//
//
//
void GetRgbBuf( UCHAR *pal ) {

	memmove( pal, system_rgb, 768 );

	return;
}






//
//
//
void CompileRgb( UCHAR *pal, UCHAR *col ) {

	int i;

	for( i=0; i<256; i++ ) {

		col[i] = FindRgb( pal[(i*3)+0], pal[(i*3)+1], pal[(i*3)+2] );

	}

	return;
}







//
//
//
void FadePal( UCHAR *uj ) {

#define LEPES	20
#define WAITPAL (TICKBASE/70UL)

	int r,g,b,i,s,d,x,y;
	UCHAR old[768],temp[768];

	GetRgbBuf(old);

	for(s=1;s<=LEPES;s++) {

		for(i=0;i<256;i++) {
			x = uj[i*3];
			y = old[i*3];
			d = x-y;d*=256;d/=LEPES;d*=s;r=y*256+d;r/=256;

			x = uj[i*3+1];
			y = old[i*3+1];
			d = x-y;d*=256;d/=LEPES;d*=s;g=y*256+d;g/=256;

			x = uj[i*3+2];
			y = old[i*3+2];
			d = x-y;d*=256;d/=LEPES;d*=s;b=y*256+d;b/=256;

			temp[i*3]   = ( UCHAR ) r;
			temp[i*3+1] = ( UCHAR ) g;
			temp[i*3+2] = ( UCHAR ) b;

		}

		WaitRetrace(1);
		SetRgbBuf(temp);

		WaitTimer(WAITPAL);
	}

	WaitRetrace(1);
	SetRgbBuf(uj);

	return;
}




//
//
//
void FadeBlack( void ) {

	UCHAR black_pal[768];

	memset( black_pal, 0L, 768 );

	FadePal( black_pal );

	return;
}




//
//  0.30 red, 0.59 green, 0.11 blue
//
void MakeGreyScale( UCHAR *pal ) {

	int i,c;

#define cRED	( 1.00 )
#define cGREEN	( 1.00 )
#define cBLUE	( 1.00 )

	for( c=0; c<256; c++ ) {

		i  = (int)( (double)(pal[c*3+0]) * cRED   );
		i += (int)( (double)(pal[c*3+1]) * cGREEN );
		i += (int)( (double)(pal[c*3+2]) * cBLUE  );

		i <<= 4;	  // fixed
		i /= 3;

		pal[c*3]   = i>>4;
		pal[c*3+1] = i>>4;
		pal[c*3+2] = i>>4;

	}

	return;
}



//
//
//
void SetGreyRgb( void ) {

	UCHAR pal[768];

	GetRgbBuf(pal);

	MakeGreyScale( pal );

	SetRgbBuf(pal);

	return;
}



//
//
//
void SetRgbFile(char *n) {

	UCHAR *pal;

	LoadFile( n, PTR(pal) );
	SetRgbBuf( pal );

	FREEMEM( pal );

	return;
}



//
//
//
void SetRgbFile6(char *n) {

	UCHAR *pal;

	LoadFile(n,PTR(pal));
	SetRgbBuf6(pal);

	FREEMEM( pal );

	return;
}





//
//
//
BOOL SetRgbTxt( char *name ) {

	FILE *f;
	int i,j,n;
	UCHAR pal[768];

	if( (f=ffopen(name,"rt")) == NULL )
		return FALSE;

	GetIntTxt( f, &n );

	for( j=0; j<n; j++ ) {

		GetIntTxt( f, &i );
		pal[ j*3 + 0 ] = i;
		GetIntTxt( f, &i );
		pal[ j*3 + 1 ] = i;
		GetIntTxt( f, &i );
		pal[ j*3 + 2 ] = i;
	}

	ffclose( f );

	SetRgbBuf( pal );

	return TRUE;
}



//
//
//
BOOL WriteRgbTxt( char *name, UCHAR *pal ) {

	FILE *f;
	int j;

	if( (f=fopen(name,"wt")) == NULL )
		return FALSE;

	fprintf(f,"%d\n",256);

	for( j=0; j<256; j++ ) {

		fprintf(f,"%2d ",pal[ j*3 + 0 ]);
		fprintf(f,"%2d ",pal[ j*3 + 1 ]);
		fprintf(f,"%2d\n",pal[ j*3 + 2 ]);
	}

	fclose( f );

	return TRUE;
}




//
//
//
USHORT NegateColor16( USHORT color ) {

	USHORT ncolor;
	int r,g,b;

	r = rmask - INTRED(color);
	g = gmask - INTGREEN(color);
	b = bmask - INTBLUE(color);

	ncolor = (USHORT)RGBINTP(r,g,b);

	return ncolor;
}





//
//
//
void PhotoNegativeRgb(void) {

	int i;
	UCHAR temp_pal[768];
	UCHAR *temp;

	GetRgbBuf( temp_pal );
	temp = temp_pal;

	for( i=0; i<256; i++ ) {
		*temp++ = (64 - (*temp));
		*temp++ = (64 - (*temp));
		*temp++ = (64 - (*temp));
	}

	SetRgbBuf( temp_pal );

	return;
}





//
//
//
void AdjustRgb( CHAR r, CHAR g, CHAR b ) {

	int i,j,scratch;
	UCHAR temp_pal[768];
	UCHAR *temp;
	CHAR dummy[3];

	GetRgbBuf( temp_pal );
	temp = temp_pal;

	dummy[0] = r;
	dummy[1] = g;
	dummy[2] = b;

	for( i=0; i<256; i++ ) {
		for( j=0; j<3; j++ ) {

			scratch = *temp + dummy[j];

			if( scratch <= 0 ) {
				*temp++ = 0;
			}
			else if( scratch >= 63 ) {
				*temp++ = 63;
			}
			else {
				*temp++ = scratch;
			}
		}
	}

	SetRgbBuf( temp_pal );

	return;
}






//
//
//
void RotateRgb( int dist ) {

	int shift;
	UCHAR temp_pal[768];

	shift = (dist * 3);

	memmove(temp_pal, system_rgb + shift, 768 - shift);
	memmove(temp_pal + (768 - shift), system_rgb, shift);

	SetRgbBuf( temp_pal );

	return;
}






//
//
//
UCHAR FindRgb( int r, int g, int b ) {

	int shortest_dist, temp_dist;
	int i, shortest_pal;

	shortest_pal = 0;
	shortest_dist = (r - system_rgb[0]) +
			(g - system_rgb[1]) +
			(b - system_rgb[2]);

	for( i=1; i<256; i++ ) {

		temp_dist = (r - system_rgb[(i * 3) + 0]) +
			    (g - system_rgb[(i * 3) + 1]) +
			    (b - system_rgb[(i * 3) + 2]);

		if( ABS(temp_dist) < ABS(shortest_dist) ) {
			shortest_dist = temp_dist;
			shortest_pal = i;
		}
	}

	return shortest_pal;
}







#define TRANSPARENCY 0.5



//
//
//
void DeinitTransColor( void ) {

	SAFE_FREEMEM( transparent_rgb );

	return;
}



//
//
//
void MakeTransColor( BOOL force_flag ) {

#define RED(i)	 system_rgb[(i)*3+0]
#define GREEN(i) system_rgb[(i)*3+1]
#define BLUE(i)  system_rgb[(i)*3+2]

	int fore,back;
	int r1,g1,b1,r2,g2,b2;
	FIXED trans = FLOAT_TO_FIXED( TRANSPARENCY );
	UCHAR *spr;

	xprintf("transparency table: ");

	DeinitTransColor();

	ALLOCMEM( transparent_rgb, 256*256 );

	if( !force_flag && FileExist(TRANSNAME) ) {
		LoadPcx(TRANSNAME,PTR(spr));
		memmove(transparent_rgb,&spr[SPRITEHEADER],256*256);
		FREEMEM(spr);
		xprintf("\"%s\" loaded.\n",TRANSNAME);
		return;
	}


	for( fore=0; fore<256; fore++ ) {

		r1 = RED(fore);
		g1 = GREEN(fore);
		b1 = BLUE(fore);

		for( back=0; back<256; back++ ) {

			r2 = RED(back);
			g2 = GREEN(back);
			b2 = BLUE(back);

			TRANSCOLOR(fore,back) = FindRgb( r2 + (int)( ( r1-r2 ) * trans ),
							 g2 + (int)( ( g1-g2 ) * trans ),
							 b2 + (int)( ( b1-b2 ) * trans ) );
		}
		ShowProgress();
	}

	/***
	ALLOCMEM( spr, SPRITEHEADER+256*256 );
	MKSPR8(spr);
	MKSPRW(spr,256);
	MKSPRH(spr,256);
	memmove(&spr[SPRITEHEADER],transparent_rgb,256*256);

	// trans.pcx
	SavePcx(TRANSNAME,spr,system_rgb);

	FREEMEM(spr);
	***/

	xprintf("\n");

	return;
}






//
//
//
void MakeAliasColor( BOOL force_flag ) {

	int prime,derv;
	int red,green,blue;
	UCHAR *spr;

	xprintf("aliasing table: ");

	if( aliasing_rgb != NULL ) FREEMEM( aliasing_rgb );
	ALLOCMEM( aliasing_rgb, 256*256 );


	if( !force_flag && FileExist(ALIASNAME) ) {
		LoadPcx(ALIASNAME,PTR(spr));
		memmove(aliasing_rgb,&spr[SPRITEHEADER],256*256);
		FREEMEM(spr);
		xprintf("\"%s\" loaded.\n",ALIASNAME);
		return;
	}

	for( derv=0; derv<256; derv++ ) {

		for( prime=0; prime<256; prime++ ) {

			/* compute the color component differences for red,
			 * green, and blue of the prime and the fourth.
			 * this rgb is the weighted average of the prime and the fourth.
			 */

			red   = (int)( (   RED(derv) * 0.25) + (   RED(prime) * 0.75) );
			green = (int)( ( GREEN(derv) * 0.25) + ( GREEN(prime) * 0.75) );
			blue  = (int)( (  BLUE(derv) * 0.25) + (  BLUE(prime) * 0.75) );

			/* find the closet color in the 256 color palette */

			aliasing_rgb[ derv*256+prime ] = FindRgb( red, green, blue );

		}
		ShowProgress();
	}

	ALLOCMEM( spr, SPRITEHEADER+256*256 );
	MKSPR8(spr);
	MKSPRW(spr,256);
	MKSPRH(spr,256);
	memmove(&spr[SPRITEHEADER],aliasing_rgb,256*256);

	SavePcx(ALIASNAME,spr,system_rgb);

	FREEMEM(spr);

	xprintf("\n");

	return;
}






//
//
//
void Rgb2Hsv( int r, int g, int b, FLOAT *h, FLOAT *s, FLOAT *v ) {

	FLOAT m, r1, g1, b1;
	FLOAT nr, ng, nb;		/* rgb values of 0.0 - 1.0 */
	FLOAT nh = 0.0, ns, nv; 	/* hsv local values */

	nr = (FLOAT)(r / 255.0f);
	ng = (FLOAT)(g / 255.0f);
	nb = (FLOAT)(b / 255.0f);

	nv = MAX (nr, MAX (ng, nb));
	m = MIN (nr, MIN (ng, nb));

	if( nv != 0.0 ) 		/* if no value, it's black! */
		ns = (nv - m) / nv;
	else
		ns = 0.0;		/* black = no colour saturation */

	if( ns == 0.0 ) {		/* hue undefined if no saturation */

		*h = 0.0;		/* return black level (?) */
		*s = 0.0;
		*v = nv;
		return;
	}

	r1 = (nv - nr) / (nv - m);	/* distance of color from red	*/
	g1 = (nv - ng) / (nv - m);	/* distance of color from green */
	b1 = (nv - nb) / (nv - m);	/* distance of color from blue	*/

	if( nv == nr ) {
		if( m == ng )
			nh = 5.0f + b1;
		else
			nh = 1.0f - g1;
	}

	if( nv == ng ) {
		if( m == nb )
			nh = 1.0f + r1;
		else
			nh = 3.0f - b1;
	}

	if( nv == nb ) {
		if( m == nr )
			nh = 3.0f + g1;
		else
			nh = 5.0f - r1;
	}

	*h = nh * 60.0f;		/* return h converted to degrees */
	*s = ns;
	*v = nv;

	return;
}



//
// hue (0.0-360.0) s and v from 0.0-1.0)
// values from 0 to 63
//
void Hsv2Rgb( FLOAT hue, FLOAT s, FLOAT v, int *r, int *g, int *b ) {

	FLOAT i, f, p1, p2, p3;
	FLOAT xh;
	FLOAT nr, ng, nb;	/* rgb values of 0.0 - 1.0 */

	if( hue == 360.0f )
		hue = 0.0;	/* (THIS LOOKS BACKWARDS BUT OK) */

	xh = hue / 60.0f;	/* convert hue to be in 0,6	*/
	i = (FLOAT)floor(xh);	/* i = greatest integer <= h	*/
	f = xh - i;		/* f = fractional part of h	*/
	p1 = v * (1 - s);
	p2 = v * (1 - (s * f));
	p3 = v * (1 - (s * (1 - f)));

	switch( (int) i) {
		case 0:
			nr = v;
			ng = p3;
			nb = p1;
			break;
		case 1:
			nr = p2;
			ng = v;
			nb = p1;
			break;
		case 2:
			nr = p1;
			ng = v;
			nb = p3;
			break;
		case 3:
			nr = p1;
			ng = p2;
			nb = v;
			break;
		case 4:
			nr = p3;
			ng = p1;
			nb = v;
			break;
		case 5:
			nr = v;
			ng = p1;
			nb = p2;
			break;
		default:
			nr = ng = nb = 0;
	}

	*r = (int)(nr * 255.0);	/* Normalize the values to 63 */
	*g = (int)(ng * 255.0);
	*b = (int)(nb * 255.0);

	return;
}


/***
void ARGBColor::SetHSV(double hue, double saturation, double value)
{
	double h,s,v;
	double r,g,b;

	h = hue;
	s = saturation;
	v = value;

	if (hue < 0.0)
		hue += 360.0;

	if (s != 0.0) {
		double f, p, q, t;
		if (h == 360.0)
			h = 0.0;
		h /= 60.0;

		int i = (int)h;
		f = h - i;
		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));

		switch (i) {
				case 0: r = v;	g = t;	b = p;	break;
				case 1: r = q;	g = v;	b = p;	break;
				case 2: r = p;	g = v;	b = t;	break;
				case 3: r = p;	g = q;	b = v;	break;
				case 4: r = t;	g = p;	b = v;	break;
				case 5: r = v;	g = p;	b = q;	break;
		}
	}
	else {
		r = v;
		g = v;
		b = v;
	}

	m_red = r * 255;
	m_green = g * 255;
	m_blue = b * 255;
}

void ARGBColor::GetHSV(double & hue, double & saturation, double & value)
{
	double r,g,b, rgb_max, rgb_min, delta, h, s;
	r = m_red / 255.0;
	g = m_green / 255.0;
	b = m_blue / 255.0;
	rgb_max = MAX3(r, g, b);
	rgb_min = MIN3(r, g, b);
	delta = rgb_max - rgb_min;
	h = 0;
	s = 0;

	if ( rgb_max != 0.0 )
		s = delta / rgb_max;

	if ( s != 0.0 )
	{
		double rc = (rgb_max - r) / delta;
		double gc = (rgb_max - g) / delta;
		double bc = (rgb_max - b) / delta;

		if ( r == rgb_max )
			h = bc - gc;
		else if ( g == rgb_max )
			h = 2.0f + rc - bc;
		else if ( b == rgb_max )
			h = 4.0f + gc - rc;

		h *= 60.0f;
		if ( h < 0.0 )
			h += 360.0f;
	}

	hue = h;
	saturation = s;
	value = rgb_max;
}
***/
