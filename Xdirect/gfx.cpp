/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include <xlib.h>
#include <xinner.h>

RCSID( "$Id: gfx.c,v 1.0 97-03-10 16:57:17 bernie Exp $" )


#define CHECKACTIVE	{ if( active_flag < 0 ) return; }
#define CHECKACTIVEN	{ if( active_flag < 0 ) return FALSE; }


// SDL_draw-1.2.13.tar.gz -ban van geometria rajzolás

static BOOL SurfaceOn = FALSE;
__int64 *line_offset = NULL;


//
// Line() = clippelt verzió
//
void DrawLineRgb( int x1, int y1, int x2, int y2, rgb_t color ) {

	if( GFXDLL_Line )
		GFXDLL_Line( x1,y1, x2,y2, color );
	else
		xprintf( "DrawLineRgb: GFXDLL_Line is NULL.\n");

	return;
}



//
//
//
void PutPixel( int x, int y, rgb_t color ) {

	if( x<0 || x>=SCREENW || y<0 || y>=SCREENH )
		return;

	if( GFXDLL_PutPixel )
		GFXDLL_PutPixel( x,y, color );
	else
		xprintf( "PutPixel: GFXDLL_PutPixel is NULL.\n");

	return;
}




//
//
//
static ULONG dummy_GetPixel(int x,int y) {

	xprintf("dummy: GetPixel( %d, %d)\n",x,y);

	return NOCOLOR;
}


//
//
//
static void dummy_PutSprite(int x,int y,UCHAR *spr) {

	xprintf("dummy: PutSprite( %d, %d, ptr)\n",x,y);

	return;
}


//
//
//
static void dummy_GetSprite(memptr_ptr ptr, int x,int y,int w, int h) {

	xprintf("dummy: GetSprite( ptr, %d, %d, %d, %d )\n",x,y,w,h);

	ALLOCMEM( *ptr, SPRITEHEADER+5*5 );
	MKSPRW( *ptr, 5 );
	MKSPRH( *ptr, 5 );
	MKSPR8( *ptr );

	return;
}


//
//
//
static void dummy_PutSpriteNC(int x,int y,UCHAR *spr) {

	xprintf("dummy: PutSpriteNC( %d, %d, ptr)\n",x,y);

	return;
}


//
//
//
static void dummy_GetSpriteNC(memptr_ptr ptr, int x,int y,int w, int h) {

	xprintf("dummy: GetSpriteNC( ptr, %d, %d, %d, %d )\n",x,y,w,h);

	ALLOCMEM( *ptr, SPRITEHEADER+5*5 );
	MKSPRW( *ptr, 5 );
	MKSPRH( *ptr, 5 );
	MKSPR8( *ptr );

	return;
}








ULONG (*XL_GetPixel)(int,int)			 = dummy_GetPixel;
void  (*PutSprite)(int,int,UCHAR*)		 = dummy_PutSprite;
void  (*GetSprite)(memptr_ptr,int,int,int,int)	 = dummy_GetSprite;
void  (*PutSpriteNC)(int,int,UCHAR*)		 = dummy_PutSpriteNC;
void  (*GetSpriteNC)(memptr_ptr,int,int,int,int) = dummy_GetSpriteNC;


void dummyfunc(void) {

	xprintf("dummy function called.\n");

	return;
}


//
//
//
void ResetGraphics( void ) {

	bpp = 0;

	XL_GetPixel	= dummy_GetPixel;
	PutSprite	= dummy_PutSprite;
	GetSprite	= dummy_GetSprite;
	PutSpriteNC	= dummy_PutSpriteNC;
	GetSpriteNC	= dummy_GetSpriteNC;

	return;
}








//
// az rgb.c -bõl
//
ULONG AntiAlias( int x, int y, int color ) {

	int r,g,b;

	r = system_rgb[(color*3)+0];
	g = system_rgb[(color*3)+1];
	b = system_rgb[(color*3)+2];
	//xprintf("%d: %d %d %d\n",color,r,g,b);

	color = XL_GetPixel(x-1,y);
	r += system_rgb[(color*3)+0];
	g += system_rgb[(color*3)+1];
	b += system_rgb[(color*3)+2];
	//xprintf("%d: %d %d %d\n",color,r,g,b);

	color = XL_GetPixel(x-1,y-1);
	r += system_rgb[(color*3)+0];
	g += system_rgb[(color*3)+1];
	b += system_rgb[(color*3)+2];
	//xprintf("%d: %d %d %d\n",color,r,g,b);

	color = XL_GetPixel(x,y-1);
	r += system_rgb[(color*3)+0];
	g += system_rgb[(color*3)+1];
	b += system_rgb[(color*3)+2];
	//xprintf("%d: %d %d %d\n",color,r,g,b);


	r>>=2;
	g>>=2;
	b>>=2;
	//xprintf("%d: %d %d %d\n",color,r,g,b);

	color = FindRgb(r,g,b);
	//xprintf("color: %d\n",color);

	return color;
}





//
// generic
//
void HLine(int x_left,int len,int y, rgb_t color) {

	int x;

	CHECKACTIVE;

	// LockSurface( LS_WRITE );

	for(x=x_left;x<x_left+len;x++)
		PutPixel(x,y,color);

	// UnlockSurface();

	return;
}



//
// generic
//
void VLine(int x,int y,int len,rgb_t c) {

	int i;

	CHECKACTIVE;

	// LockSurface( LS_WRITE );

	for(i=y;i<y+len;i++)
		PutPixel(x,i,c);

	// UnlockSurface();

	return;
}



//
//
//
void Box(int ulx,int uly,int brx,int bry,rgb_t c) {

	if( ulx>brx ) SWAPL(ulx,brx);
	if( uly>bry ) SWAPL(uly,bry);

	HLine(ulx,brx-ulx,uly,c);
	HLine(ulx,brx-ulx,bry,c);
	VLine(ulx,uly,bry-uly+1,c);
	VLine(brx,uly,bry-uly+1,c);

	return;
}


//
//
//
void FilledBox(int x1,int y1,int x2,int y2,rgb_t color) {

	int x,y;

	CHECKACTIVE;

	// LockSurface( LS_WRITE );

	for(y=y1;y<=y2;y++)
		for(x=x1;x<=x2;x++)
			PutPixel(x,y,color);

	// UnlockSurface();

	return;
}





//
//
//
void DashBox( int ux, int uy, int bx, int by, int be, int ki, rgb_t color ) {

	int x,y,cnt,temp;
	BOOL flag;

	CHECKACTIVE;

	// LockSurface( LS_WRITE );

	flag = TRUE;
	cnt = 0;

	if( ux > bx ) {
		temp = bx;
		bx = ux;
		ux = temp;
	}
	if( uy > by ) {
		temp = by;
		by = uy;
		uy = temp;
	}

	// 1.
	for( x=ux; x<=bx; x++ ) {
		++cnt;
		if( flag ) {
			if( cnt >= be ) {
				flag = 0;
				cnt = 0;
			}
		}
		else {
			if( cnt >= ki ) {
				flag = 1;
				cnt = 0;
			}
		}

		if( flag )
			PutPixel( x, uy, color );
	}

	// 2.
	for( y=uy; y<=by; y++ ) {
		++cnt;
		if( flag ) {
			if( cnt >= be ) {
				flag = 0;
				cnt = 0;
			}
		}
		else {
			if( cnt >= ki ) {
				flag = 1;
				cnt = 0;
			}
		}

		if( flag )
			PutPixel( bx, y, color );
	}

	// 3.
	for( x=bx; x>=ux; x-- ) {
		++cnt;
		if( flag ) {
			if( cnt >= be ) {
				flag = 0;
				cnt = 0;
			}
		}
		else {
			if( cnt >= ki ) {
				flag = 1;
				cnt = 0;
			}
		}

		if( flag )
			PutPixel( x, by, color );
	}



	// 4.
	for( y=by; y>=uy; y-- ) {
		++cnt;
		if( flag ) {
			if( cnt >= be ) {
				flag = 0;
				cnt = 0;
			}
		}
		else {
			if( cnt >= ki ) {
				flag = 1;
				cnt = 0;
			}
		}

		if( flag )
			PutPixel( ux, y, color );
	}

	// UnlockSurface();

	return;
}




//
// avoids setting the same pixel twice
//
static void Set4Pixels( int x, int y, int xc, int yc, rgb_t n ) {

	CHECKACTIVE;

	// LockSurface( LS_WRITE );

	if( x!=0 ) {

		PutPixel( xc+x, yc+y, n );
		PutPixel( xc-x, yc+y, n );

		if( y!=0 ) {
			PutPixel( xc+x, yc-y, n );
			PutPixel( xc-x, yc-y, n );
		}
	}
	else {

		PutPixel( xc, yc+y, n );
		if( y!=0 )
			PutPixel( xc, yc-y, n );
	}

	// UnlockSurface();

	return;
}





//
// xc,yc: center of ellipse
// a0,b0: semiaxes
//
void XL_Ellipse( int xc, int yc, int a0, int b0, rgb_t PixelValue ) {

	int	x = 0;
	int	y = b0;

	long	a = a0; 			/* use 32-bit precision */
	long	b = b0;

	long	Asquared = a * a;		/* initialize values outside */
	long	TwoAsquared = 2 * Asquared;	/*  of loops */
	long	Bsquared = b * b;
	long	TwoBsquared = 2 * Bsquared;

	long	d;
	long	dx,dy;

	d = Bsquared - Asquared*b + Asquared/4L;
	dx = 0;
	dy = TwoAsquared * b;

	while (dx<dy)
	{
	  Set4Pixels( x, y, xc, yc, PixelValue );

	  if (d > 0L)
	  {
	    --y;
	    dy -= TwoAsquared;
	    d -= dy;
	  }

	  ++x;
	  dx += TwoBsquared;
	  d += Bsquared + dx;
	}


	d += (3L*(Asquared-Bsquared)/2L - (dx+dy)) / 2L;

	while (y>=0)
	{
	  Set4Pixels( x, y, xc, yc, PixelValue );

	  if (d < 0L)
	  {
	    ++x;
	    dx += TwoBsquared;
	    d += dx;
	  }

	  --y;
	  dy -= TwoAsquared;
	  d += Asquared - dy;
	}
}





//
//
//
void Circle( int x, int y, int r, rgb_t color ) {

	int ix,iy,d,deltaE,deltaSE;

	CHECKACTIVE;

	ix = 0;
	iy = r;
	d = 1 - r;
	deltaE = 3;
	deltaSE = (-2 * r) + 5;

	// LockSurface( LS_WRITE );

	PutPixel(x + ix, y + iy, color);
	PutPixel(x + ix, y - iy, color);
	PutPixel(x + iy, y + ix, color);
	PutPixel(x + iy, y - ix, color);
	PutPixel(x - ix, y + iy, color);
	PutPixel(x - ix, y - iy, color);
	PutPixel(x - iy, y + ix, color);
	PutPixel(x - iy, y - ix, color);

	while(iy > ix++) {
		if(d < 0) {
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		} else {
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			iy--;
		}

		PutPixel(x + ix, y + iy, color);
		PutPixel(x + ix, y - iy, color);
		PutPixel(x + iy, y + ix, color);
		PutPixel(x + iy, y - ix, color);
		PutPixel(x - ix, y + iy, color);
		PutPixel(x - ix, y - iy, color);
		PutPixel(x - iy, y + ix, color);
		PutPixel(x - iy, y - ix, color);
	}

	// UnlockSurface();

	return;
}



//
//
//
void FilledCircle( int x, int y, int r, rgb_t color) {

	int ix,iy,d,deltaE,deltaSE;

	ix = 0;
	iy = r;
	d = 1 - r;
	deltaE = 3;
	deltaSE = (-2 * r) + 5;

	HLine(x - ix,(ix << 1) + 1, y + iy,  color);
	HLine(x - ix,(ix << 1) + 1, y - iy,  color);
	HLine(x - iy,(iy << 1) + 1, y + ix,  color);
	HLine(x - iy,(iy << 1) + 1, y - ix,  color);

	while(iy > ix++) {
		if (d < 0) {
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		} else {
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			iy--;
		}

		HLine(x - ix,(ix << 1) + 1, y + iy,  color);
		HLine(x - ix,(ix << 1) + 1, y - iy,  color);
		HLine(x - iy,(iy << 1) + 1, y + ix,  color);
		HLine(x - iy,(iy << 1) + 1, y - ix,  color);
	}
	return;
}






//
//
//
void XorLineNC( int x1, int y1, int x2, int y2 ) {

	rgb_t c = {205,205,205,255};

	DrawLineRgb( x1,y1, x2,y2, c );

	/***
	int kx,ky,nx,ny,lx,ly,px,py;
	ULONG c;

	CHECKACTIVE;

	kx = x2 - x1;
	ky = y2 - y1;
	nx = ABS(kx);
	ny = ABS(ky);

	LockSurface( LS_WRITE );

	c = ( GetPixel(x1,y1) ^ 0xff );
	PutPixel(x1,y1,c);

	if(nx>ny) {
		ly=0;
		for(lx=0;lx<nx;lx++) {
			if( (2*(ly*nx-lx*ny)+nx) <= 0)
				ly++;
			px=(kx>0 ? x1+lx : x1-lx);
			py=(ky>0 ? y1+ly : y1-ly);
			c = ( GetPixel(px,py) ^ 0xff );
			PutPixel(px,py,c);
		}
	}
	else {
		lx=0;
		for(ly=0;ly<ny;ly++) {
			if( (2*(lx*ny-ly*nx)+ny) <= 0)
				lx++;
			px=(kx>0 ? x1+lx : x1-lx);
			py=(ky>0 ? y1+ly : y1-ly);
			c = ( GetPixel(px,py) ^ 0xff );
			PutPixel(px,py,c);
		}
	}

	c = ( GetPixel(x2,y2) ^ 0xff );
	PutPixel(x2,y2,c);

	UnlockSurface();
	***/

	return;
}





//
//
//
void obsolate_LineNC( int x1, int y1, int x2, int y2, rgb_t c ) {

	if( GFXDLL_Line )
		GFXDLL_Line( x1,y1, x2,y2, c );

	/***
	int kx,ky,nx,ny,lx,ly,px,py;

	CHECKACTIVE;

	kx = x2 - x1;
	ky = y2 - y1;
	nx = ABS(kx);
	ny = ABS(ky);

	LockSurface( LS_WRITE );

	PutPixel(x1,y1,c);

	if(nx>ny) {
		ly=0;
		for(lx=0;lx<nx;lx++) {
			if( (2*(ly*nx-lx*ny)+nx) <= 0)
				ly++;
			px=(kx>0 ? x1+lx : x1-lx);
			py=(ky>0 ? y1+ly : y1-ly);
			PutPixel(px,py,c);
		}
	}
	else {
		lx=0;
		for(ly=0;ly<ny;ly++) {
			if( (2*(lx*ny-ly*nx)+ny) <= 0)
				lx++;
			px=(kx>0 ? x1+lx : x1-lx);
			py=(ky>0 ? y1+ly : y1-ly);
			PutPixel(px, py,c);
		}
	}

	PutPixel(x2,y2,c);

	UnlockSurface();
	***/

	return;
}





//
// bresenham
//
void obsolate_BresenLineNC( int x1, int y1, int x2, int y2, ULONG c ) {

	rgb_t color = {255,255,255,255};

	if( GFXDLL_Line )
		GFXDLL_Line( x1,y1, x2,y2, color );

	/***
	int i,e;
	int dx,dy;

	CHECKACTIVE;

	if(x1>x2) SWAPL(x1,x2);
	if(y1>y2) SWAPL(y1,y2);

	dx = x2 - x1;
	dy = y2 - y1;

	e = (2*dy) - dx;

	LockSurface( LS_WRITE );

	for( i=0; i<dx; i++ ) {

		PutPixel( x1,y1, color );

		if( e>0 ) {
			++y1;
			e += (2*dy) - (2*dx);
		}
		else
			e += (2*dy);

		++x1;
	}

	PutPixel( x2,y2, color );

	UnlockSurface();
	***/

	return;
}





//
//
//
static int CutCode( int x, int y ) {

	int code = 0;

	if( x < CLIPMINX )
		code = 1;
	else
	if( x > CLIPMAXX )
		code = 2;

	if( y < CLIPMINY )
		return (code | 4);
	else
	if( y > CLIPMAXY )
		return (code | 8);

	return code;
}






//
// Cohen-Sutherland
//
//	 abx  ajx
//    0101³0100³0110
// afyÄÄÄÄ³ÄÄÄÄ³ÄÄÄÄ
//    0001³0000³0010
// aayÄÄÄÄ³ÄÄÄÄ³ÄÄÄÄ
//    1001³1000³1010
//
static void ClipLine( int *x1, int *y1, int *x2, int *y2 ) {

	int c1, c2;
	int cc;
	int xx, yy;

	c1 = CutCode( (*x1), (*y1) );
	c2 = CutCode( (*x2), (*y2) );

	while( c1 || c2 ) {

		if( c1 & c2 )
			return;

		if( c1 == 0 ) {
			cc = c1; c1 = c2; c2 = cc;
			xx = (*x1); (*x1) = (*x2); (*x2) = xx;
			yy = (*y1); (*y1) = (*y2); (*y2) = yy;
		}

		if( c1 & 1 ) {
			(*y1) += ftoi( (FLOAT)((*y2) - (*y1)) / (FLOAT)((*x2) - (*x1)) ) * (CLIPMINX - (*x1));
			(*x1) = CLIPMINX;
		}
		else
		if( c1 & 2 ) {
			(*y1) += ftoi( (FLOAT)((*y2) - (*y1)) / (FLOAT)((*x2) - (*x1)) ) * (CLIPMAXX - (*x1));
			(*x1) = CLIPMAXX;
		}
		else
		if( c1 & 4 ) {
			(*x1) += ftoi( (FLOAT)((*x2) - (*x1)) / (FLOAT)((*y2) - (*y1)) ) * (CLIPMINY - (*y1));
			(*y1) = CLIPMINY;
		}
		else
		if( c1 & 8 ) {
			(*x1) += ftoi( (FLOAT)((*x2) - (*x1)) / (FLOAT)((*y2) - (*y1)) ) * (CLIPMAXY - (*y1));
			(*y1) = CLIPMAXY;
		}

		c1 = CutCode( (*x1), (*y1) );
	}

	return;
}






//
//
//
void Line( int x1, int y1, int x2, int y2, rgb_t color ) {

	FlushScene();

	ClipLine( &x1, &y1, &x2, &y2 );

	DrawLineRgb( x1, y1, x2, y2, color );

	return;
}




//
//
//
void DrawPolygon( polygon_t poly, rgb_t color ) {

	int i;

	if( poly.npoints < 2 )
		return;

	for( i=0; i<poly.npoints-1; i++ )
		Line( poly.point[i][0], poly.point[i][1],
			poly.point[i+1][0], poly.point[i+1][1], color );

	if( poly.npoints == 2 )
		return;

	Line( poly.point[0][0], poly.point[0][1],
		poly.point[poly.npoints-1][0], poly.point[poly.npoints-1][1], color );

	return;
}




//
//
//
void DrawPolyline( polygon_t poly, rgb_t color ) {

	int i;

	if( poly.npoints < 2 )
		return;

	for( i=0; i<poly.npoints-1; i++ )
		Line( poly.point[i][0], poly.point[i][1],
			poly.point[i+1][0], poly.point[i+1][1], color );

	return;
}



#ifndef __GNUC__

//
// Rounds a number to the nearest integer
//
long round( double number ) {
	return (long)(number+0.5);
}

#endif

/*
             arrowLength
               <--->

               |\     ^
   arrowBase ->|  \   |
---------------|   >  | arrowBase
               |  /   |
	       |/     v
*/


//
// Draws an arrow from m_One to m_Two with the given options
//
void DrawArrow( int x0, int y0, int x1, int y1, rgb_t in_color,
			int width,
			int arrowLength,
			int iarrowBase,
			BOOL drawFirstArrow,
			BOOL drawSecondArrow,
			BOOL openArrowHead ) {

	point2_t vert[3];
	double slopy , cosy , siny;
	double arrowBase = iarrowBase/2.0; //we need the length of half arrow base
	rgb_t color = {255,255,255, 255};

	polygon_t poly;

	poly.npoints = 3;
	poly.point = vert;

	color.r = in_color.r;
	color.g = in_color.g;
	color.b = in_color.b;
	color.a = in_color.a;

	//draw a line between the 2 endpoints
	Line( x0,y0, x1,y1, color );

	if(!(drawFirstArrow || drawSecondArrow))
		return;

	slopy = atan2((double)( y0 - y1 ),
		      (double)( x0 - x1 ) );
	cosy = cos( slopy );
	siny = sin( slopy ); //need math.h for these functions

	//here is the tough part - actually drawing the arrows
	//a total of 6 lines drawn to make the arrow shape
	//first arrow
	if( drawFirstArrow ) {

		vert[0][0] = x0 + int(round( - arrowLength * cosy - ( arrowBase * siny ) ));
		vert[0][1] = y0 + int(round( - arrowLength * siny + ( arrowBase * cosy ) ));
		vert[1][0] = x0;
		vert[1][1] = y0;
		vert[2][0] = x0 + int(round( - arrowLength * cosy + ( arrowBase * siny ) ));
		vert[2][1] = y0 - int(round( arrowBase * cosy + arrowLength * siny ));

		if( x0 == x1 )
			vert[0][1] = vert[2][1];

		if( y0 == y1 )
			vert[0][0] = vert[2][0];

		if( openArrowHead )
			DrawPolyline( poly, color );
		else
			DrawPolygon( poly, color );
	}

	//second arrow
	if( drawSecondArrow ) {

		vert[0][0] = x1 + int( arrowLength * cosy - ( arrowBase * siny ) );
		vert[0][1] = y1 + int( arrowLength * siny + ( arrowBase * cosy ) );
		vert[1][0] = x1;
		vert[1][1] = y1;
		vert[2][0] = x1 + int( arrowLength * cosy + arrowBase * siny );
		vert[2][1] = y1 - int( arrowBase * cosy - arrowLength * siny );

		if( x0 == x1 )
			vert[0][1] = vert[2][1];

		if( y0 == y1 )
			vert[0][0] = vert[2][0];

		if( openArrowHead )
			DrawPolyline( poly, color );
		else
			DrawPolygon( poly, color );
	}

	return;
}




//
//
//
void XorLine( int x1, int y1, int x2, int y2 ) {

	ClipLine( &x1, &y1, &x2, &y2 );

	XorLineNC( x1, y1, x2, y2 );

	return;
}


//
//
//
extern void ShadeRegion( int x, int y, int w, int h, int percent ) {

	int i,j;

	CHECKACTIVE;

	switch( bpp >> 3 ) {

		case 1:

			break;

		case 2: {

			USHORT *screen, *scr_ptr, color;
			rgb_t c;
			// int r,g,b;

			ALLOCMEM( screen, w*h*2 );
			scr_ptr = screen;

			LockSurface( LS_READ );

			for(i=y;i<y+h;i++)     // row
			for(j=x;j<x+w;j++)
				*scr_ptr++ = (USHORT)XL_GetPixel(j,i);

			UnlockSurface();

			scr_ptr = screen;

			// LockSurface( LS_WRITE );

			for(i=y;i<y+h;i++)     // row
			for(j=x;j<x+w;j++) {  // col

				c.r = INTRED16(*scr_ptr) >> 1;
				c.g = INTGREEN16(*scr_ptr) >> 1;
				c.b = INTBLUE16(*scr_ptr) >> 1;
				c.a = 255;

				++scr_ptr;

				// color = (USHORT)RGBINTP( r,g,b );

				PutPixel( j,i, c );
			}

			// UnlockSurface();

			FREEMEM( screen );

			break;
		}

		case 3:

			break;
	}


	return;
}




//
//
//
BOOL LockSurface( int flag ) {

	if( !GFXDLL_LockLfb || (active_flag< 0) ) return FALSE;

	if( !SurfaceOn ) {

		SurfaceOn = TRUE;

		if( GFXDLL_LockLfb( &gfxmem, flag ) == FALSE ) {
			xprintf("LockSurface: lock failed.\n");
			return FALSE;
		}
	}

	return TRUE;
}




//
//
//
void UnlockSurface( void ) {

	if( !GFXDLL_UnlockLfb || (active_flag< 0) ) return;

	if( SurfaceOn ) {

		SurfaceOn = FALSE;

		GFXDLL_UnlockLfb();
	}

	return;
}




//
// full screen glow effect
//
void PostProcess( int flag ) {

	if( !GFXDLL_LockLfb || !GFXDLL_UnlockLfb || !GFXDLL_GetPitch || (active_flag< 0) )
		return;

	int pitch=GFXDLL_GetPitch();

	if( GFXDLL_LockLfb( &gfxmem, 0 ) == FALSE )
		return;

	UCHAR *p = (UCHAR *)gfxmem;

	// xprintf("bpp: %d  pitch: %d\n",bpp,pitch);

	for( int y=0; y<SCREENH; y++ )
	for( int x=0; x<SCREENW; x++ ) {
		p[y*pitch+x*bpp/8] = 100;
	}

	GFXDLL_UnlockLfb();

	return;
}





//
// floodfill from MIKALLEGRO
//
typedef struct FLOODED_LINE {	 /* store segments which have been flooded */
	short flags; 		 /* status of the segment */
	short lpos, rpos;	 /* left and right ends of segment */
	short y;		 /* y coordinate of the segment */
	short next;		 /* linked list if several per line */
} FLOODED_LINE;

static int flood_count; 	 /* number of flooded segments */

#define FLOOD_IN_USE		 1
#define FLOOD_TODO_ABOVE	 2
#define FLOOD_TODO_BELOW	 4

#define FLOOD_LINE(c)		 (((FLOODED_LINE *)_scratch_mem) + c)
static void *_scratch_mem = NULL;

//
// flooder:
//  Fills a horizontal line around the specified position, and adds it
//  to the list of drawn segments. Returns the first x coordinate after
//  the part of the line which it has dealt with.
//
static int flooder( int x, int y, ULONG src_color, rgb_t dest_color ) {

	int c;
	FLOODED_LINE *p;
	int left, right;

	/* check start pixel */
	if( XL_GetPixel( x, y ) != src_color )
		return x+1;

	/* work left from starting point */
	for( left=x-1; left>=CLIPMINX; left-- )
		if( XL_GetPixel( left, y) != src_color )
			break;

	/* work right from starting point */
	for( right=x+1; right<=CLIPMAXX; right++ )
		if( XL_GetPixel( right, y) != src_color )
			break;

	left++;
	right--;

	/* draw the line */
	HLine( left, (right-left)+1, y, dest_color );

	/* store it in the list of flooded segments */
	c = y;
	p = FLOOD_LINE(c);

	if( p->flags ) {
		while( p->next ) {
			c = p->next;
			p = FLOOD_LINE(c);
		}

		p->next = c = flood_count++;
		REALLOCMEM( _scratch_mem, (sizeof(FLOODED_LINE) * flood_count) );
		p = FLOOD_LINE(c);
	}

	p->flags = FLOOD_IN_USE;
	p->lpos = left;
	p->rpos = right;
	p->y = y;
	p->next = 0;

	if( y > CLIPMINY )
		p->flags |= FLOOD_TODO_ABOVE;

	if( y < CLIPMAXY )
		p->flags |= FLOOD_TODO_BELOW;

	return right+2;
}


//
// check_flood_line:
//  Checks a line segment, using the scratch buffer is to store a list of
//  segments which have already been drawn in order to minimise the required
//  number of tests.
//
static int check_flood_line( int y, int left, int right, ULONG src_color, rgb_t dest_color) {

	int c;
	FLOODED_LINE *p;
	BOOL ret = FALSE;

	while( left <= right ) {
		c = y;

		do {
			p = FLOOD_LINE(c);
			if( (left >= p->lpos) && (left <= p->rpos) ) {
				left = p->rpos+2;
				goto no_flood;
			}

			c = p->next;

		} while( c );

		left = flooder( left, y, src_color, dest_color );
		ret = TRUE;

		no_flood:;
	}

	return ret;
}




/* floodfill:
 *  Fills an enclosed area (starting at point x, y) with the specified color.
 */
void XL_FloodFill( int x, int y, rgb_t color ) {

	ULONG src_color;
	int c;
	BOOL done;
	FLOODED_LINE *p;

	/* make sure we have a valid starting point */
	if( (x < CLIPMINX) || (x > CLIPMAXX) || (y < CLIPMINY) || (y > CLIPMAXY))
		return;

	LockSurface( LS_WRITE );

	/* what color to replace? */
	src_color = XL_GetPixel( x, y );

	if( src_color == color.r ) {
		UnlockSurface();
		return;
	}

	/* set up the list of flooded segments */
	REALLOCMEM( _scratch_mem, (sizeof(FLOODED_LINE) * CLIPMAXY) );
	flood_count = CLIPMAXY;
	p = (FLOODED_LINE*)_scratch_mem;

	for( c=0; c<flood_count; c++) {
		p[c].flags = 0;
		p[c].lpos = SHRT_MAX;
		p[c].rpos = SHRT_MIN;
		p[c].y = y;
		p[c].next = 0;
	}

	/* start up the flood algorithm */
	flooder( x, y, src_color, color);

	/* continue as long as there are some segments still to test */
	do {
		done = TRUE;

		/* for each line on the screen */
		for( c=0; c<flood_count; c++ ) {

			p = FLOOD_LINE(c);

			/* check below the segment? */
			if( p->flags & FLOOD_TODO_BELOW ) {
				p->flags &= ~FLOOD_TODO_BELOW;
				if( check_flood_line( p->y+1, p->lpos, p->rpos, src_color, color) ) {
					done = FALSE;
					p = FLOOD_LINE(c);
				}
			}

			/* check above the segment? */
			if( p->flags & FLOOD_TODO_ABOVE ) {
				p->flags &= ~FLOOD_TODO_ABOVE;
				if( check_flood_line( p->y-1, p->lpos, p->rpos, src_color, color) ) {
					done = FALSE;
					/* special case shortcut for going backwards */
					if( (c < CLIPMAXY) && (c > 0) )
						c -= 2;
				}
			}
		}

	} while( !done );

	FREEMEM( _scratch_mem );

	UnlockSurface();

	return;
}





//
//
//
void Fill( polygon_t *PL, rgb_t C ) {

	typedef struct Vektor {
		int	 X, Y, XMax, DX, DY, DZ, Z, Spalte;
	} Vektor;
	typedef int VekPoly[MAXVERTNUM][3][3];

	int P[MAXVERTNUM][3];
	VekPoly Sp;
	Vektor	V[10];
	int	S[(2*25)+2];
	int	I, J, K, N, SX, YRMin, YRMax, YR, XMin, YMin, YMax, I2, temp;
	int	Size = MIN(PL->npoints,MAXVERTNUM);

	if(Size > 10)
		return;

	for(temp=1; temp<=Size; temp++) {
		P[temp][1]= PL->point[temp-1][0];    // X
		P[temp][2]= PL->point[temp-1][1];    // Y
	}


	K = 1;
	for (I = 1; I <= Size; I++) {
		Sp[K][1][1] = P[I][1];
		Sp[K][1][2] = P[I][2];
		if (I == Size) {
			Sp[K][2][1] = P[1][1];
			Sp[K][2][2] = P[1][2];
		}
		else {
			Sp[K][2][1] = P[I + 1][1];
			Sp[K][2][2] = P[I + 1][2];
		}
		if(Sp[K][2][2] - Sp[K][1][2] < 0) {
			J = Sp[K][2][1];
			Sp[K][2][1] = Sp[K][1][1];
			Sp[K][1][1] = J;
			J = Sp[K][2][2];
			Sp[K][2][2] = Sp[K][1][2];
			Sp[K][1][2] = J;
		}
		++K;
	}
	YRMin = CLIPMAXY; //199;
	YRMax = CLIPMINY; //0;
	for (K = 1; K <= Size; K++)
		for (I = 1; I <= 2; I++) {
			if (Sp[K][I][2] > YRMax)
				YRMax = Sp[K][I][2];
			if (Sp[K][I][2] < YRMin)
				YRMin = Sp[K][I][2];
		}
	if (YRMin < CLIPMINY) //0)
		YRMin = CLIPMINY; //0;
	if (YRMax > CLIPMAXY) //199)
		YRMax = CLIPMAXY; //199;
	for (K = 1; K <= Size; K++) {
		XMin	  = Sp[K][1][1];
		YMin	  = Sp[K][1][2];
		V[K].XMax = Sp[K][2][1];
		YMax	  = Sp[K][2][2];
		V[K].DX   = ABS(XMin - V[K].XMax);
		V[K].DY   = ABS(YMin - YMax);
		V[K].X	  = XMin;
		V[K].Y	  = YMin;
		if(XMin < V[K].XMax)
			V[K].Z = 1;
		else
			V[K].Z = -1;
		if (V[K].DX > V[K].DY)
			I2 = V[K].DX;
		else
			I2 = V[K].DY;
		V[K].DZ =(I2/2);
		V[K].Spalte = XMin;
	}

	for (YR = YRMin; YR <= YRMax; YR++) {
		N = 0;
		for (K = 1; K <= Size; K++)
		if (((Sp[K][1][2] <= YR) && (YR < Sp[K][2][2])) || ((YR == YRMax) && (YRMax == Sp[K][2][2]) && (YRMax != Sp[K][1][2]))) {
			++N;
			S[N] = V[K].X;
			SX   = V[K].X;
			do {
				if (V[K].DZ < V[K].DX) {
					V[K].DZ += V[K].DY;
					V[K].X	+= V[K].Z;
				}
				if (V[K].DZ >= V[K].DX) {
					V[K].DZ -= V[K].DX;
					++V[K].Y;
				}
				if (V[K].Y == YR)
					SX = V[K].X;
				V[K].Spalte+=V[K].Z;
			}while ( !( (V[K].Y > YR) || (V[K].Spalte == V[K].XMax) ) );
			++N;
			S[N] = SX;
		}
		for (I = 2; I <= N; I++)
			for (K = N; K >= I; K--)
				if (S[K-1] > S[K]) {
					J = S[K-1];
					S[K-1] = S[K];
					S[K] = J;
				}
		K = 1;
		while (K <= N) {
			if (S[K] < CLIPMINX)
				S[K] = CLIPMINX;
			if (S[K + 3] > CLIPMAXX) //319)
				S[K + 3] = CLIPMAXX; //319;
			HLine(S[K],S[K+3]-S[K],YR,C);
			K += 4;
		}
	}

	return;
}




//
//
//
static void rasterfill_HLine( int x1,int x2,int y,rgb_t color ) {

	static int x,start=0;

	x = x1;

	if( (1-start) ) {

		// start == 0

		if( (x1&1) )
			x = x1 + 1;

	}
	else {

		// start == 1

		if(  !(x1&1) )
			x = x1 + 1;

	}

	start = 1 - start;

	for(;x<x2;x+=2)
		PutPixel(x,y,color);

	return;
}




//
//
//
void RasterFill( polygon_t *PL, rgb_t C ) {

	typedef struct Vektor {
		int	 X, Y, XMax, DX, DY, DZ, Z, Spalte;
	} Vektor;
	typedef int VekPoly[10][3][3];

	int P[10][3];
	VekPoly Sp;
	Vektor	V[10];
	int	S[(2*25)+2];
	int	I, J, K, N, SX, YRMin, YRMax, YR, XMin, YMin, YMax, I2, temp;
	int	Size = MIN(PL->npoints,MAXVERTNUM);

	CHECKACTIVE;

	if(Size > 10)
		return;

	for(temp=1; temp<=Size; temp++) {
		P[temp][1]= PL->point[temp-1][0];    // X
		P[temp][2]= PL->point[temp-1][1];    // Y
	}


	K = 1;
	for (I = 1; I <= Size; I++) {
		Sp[K][1][1] = P[I][1];
		Sp[K][1][2] = P[I][2];
		if (I == Size) {
			Sp[K][2][1] = P[1][1];
			Sp[K][2][2] = P[1][2];
		}
		else {
			Sp[K][2][1] = P[I + 1][1];
			Sp[K][2][2] = P[I + 1][2];
		}
		if(Sp[K][2][2] - Sp[K][1][2] < 0) {
			J = Sp[K][2][1];
			Sp[K][2][1] = Sp[K][1][1];
			Sp[K][1][1] = J;
			J = Sp[K][2][2];
			Sp[K][2][2] = Sp[K][1][2];
			Sp[K][1][2] = J;
		}
		++K;
	}
	YRMin = CLIPMAXY; //199;
	YRMax = CLIPMINY; //0;
	for (K = 1; K <= Size; K++)
		for (I = 1; I <= 2; I++) {
			if (Sp[K][I][2] > YRMax)
				YRMax = Sp[K][I][2];
			if (Sp[K][I][2] < YRMin)
				YRMin = Sp[K][I][2];
		}
	if (YRMin < CLIPMINY) //0)
		YRMin = CLIPMINY; //0;
	if (YRMax > CLIPMAXY) //199)
		YRMax = CLIPMAXY; //199;
	for (K = 1; K <= Size; K++) {
		XMin	  = Sp[K][1][1];
		YMin	  = Sp[K][1][2];
		V[K].XMax = Sp[K][2][1];
		YMax	  = Sp[K][2][2];
		V[K].DX   = ABS(XMin - V[K].XMax);
		V[K].DY   = ABS(YMin - YMax);
		V[K].X	  = XMin;
		V[K].Y	  = YMin;
		if(XMin < V[K].XMax)
			V[K].Z = 1;
		else
			V[K].Z = -1;
		if (V[K].DX > V[K].DY)
			I2 = V[K].DX;
		else
			I2 = V[K].DY;
		V[K].DZ =(I2/2);
		V[K].Spalte = XMin;
	}

	// LockSurface( LS_WRITE );

	for (YR = YRMin; YR <= YRMax; YR++) {
		N = 0;
		for (K = 1; K <= Size; K++)
		if (((Sp[K][1][2] <= YR) && (YR < Sp[K][2][2])) || ((YR == YRMax) && (YRMax == Sp[K][2][2]) && (YRMax != Sp[K][1][2]))) {
			++N;
			S[N] = V[K].X;
			SX   = V[K].X;
			do {
				if (V[K].DZ < V[K].DX) {
					V[K].DZ += V[K].DY;
					V[K].X	+= V[K].Z;
				}
				if (V[K].DZ >= V[K].DX) {
					V[K].DZ -= V[K].DX;
					++V[K].Y;
				}
				if (V[K].Y == YR)
					SX = V[K].X;
				V[K].Spalte+=V[K].Z;
			}while ( !( (V[K].Y > YR) || (V[K].Spalte == V[K].XMax) ) );
			++N;
			S[N] = SX;
		}
		for (I = 2; I <= N; I++)
			for (K = N; K >= I; K--)
				if (S[K-1] > S[K]) {
					J = S[K-1];
					S[K-1] = S[K];
					S[K] = J;
				}
		K = 1;
		while (K <= N) {
			if (S[K] < CLIPMINX)
				S[K] = CLIPMINX;
			if (S[K + 3] > CLIPMAXX) //319)
				S[K + 3] = CLIPMAXX; //319;
			rasterfill_HLine(S[K],S[K+3],YR,C);
			K += 4;
		}
	}

	// UnlockSurface();

	return;
}




//
// generic
//
void GenPutSprite( int x, int y, UCHAR *buf ) {

	int i,j,w,h;
	UCHAR *buf_ptr;
	USHORT *wbuf_ptr;

	xprintf("GenPutSprite: is old!\n");

	CHECKACTIVE;

	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;

	// LockSurface( LS_WRITE );

	if( ISSPR8(buf) ) {
		buf_ptr = &buf[SPRITEHEADER];
		for(i=y;i<h;i++)   // col
			for(j=x;j<w;j++,buf_ptr++)  // row
				// FIXME
				; // PutPixel(j,i,RGBINT8(*buf_ptr));
	}
	else
	if( ISSPR16(buf) ) {
		wbuf_ptr = (USHORT*)(&buf[SPRITEHEADER]);
		for(i=y;i<h;i++)   // col
			for(j=x;j<w;j++,wbuf_ptr++)  // row
				// FIXME
				; // PutPixel(j,i,RGBINT16(*wbuf_ptr));
	}

	// UnlockSurface();

	return;
}



//
// generic, nice, eh?
//
void GenPutSpriteNC( int x, int y, UCHAR *buf ) {

	GenPutSprite( x,y, buf );

	return;
}






//
// generic
//
void PutSpriteBlackNC( int x, int y, UCHAR *buf ) {

	int i,j,w,h;
	UCHAR *buf_ptr;

	xprintf( "PutSpriteBlackNC: is old.\n");

	CHECKACTIVE;

	buf_ptr = &buf[SPRITEHEADER];
	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;

	LockSurface( LS_WRITE );

	for(i=y;i<h;i++)   // row
		for(j=x;j<w;j++) {  // col
			if(*buf_ptr!=NOCOLOR)
				// FIXME
				; //PutPixelNC(j,i,*buf_ptr);
			buf_ptr++;
		}

	UnlockSurface();

	return;
}




//
//
//
static __inline USHORT BlendColor( USHORT video, USHORT color ) {

	int r,g,b;

#undef RGBINT
#define RGBINT(r,g,b)	(USHORT)( ( (ULONG)r << rshift ) | ( (ULONG)g << gshift ) | ( (ULONG)b << bshift ) )

	r = INTRED16(color) + INTRED16(video);
	g = INTGREEN16(color) + INTGREEN16(video);
	b = INTBLUE16(color) + INTBLUE16(video);

	CLAMPMAX( r, rmask );
	CLAMPMAX( g, gmask );
	CLAMPMAX( b, bmask );

	return RGBINT( r, g, b );

#undef RGBINT
}




//
// generic
//
void PutSpriteBlack( int x, int y, UCHAR *buf ) {

	int i,j,w,h;

	xprintf( "PutSpriteBlack: is old.\n");

	CHECKACTIVE;

	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;


	switch( SPRPIXELLEN( buf ) ) {

		case 1: {

			UCHAR *buf_ptr = &buf[SPRITEHEADER];

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)    // row
			for(j=x;j<w;j++) {  // col
				if(*buf_ptr!=NOCOLOR)
					; // PutPixel(j,i,*buf_ptr);
				++buf_ptr;
			}

			UnlockSurface();

			break;
		}

		case 2: {

			USHORT *buf_ptr = (USHORT*)&(buf[SPRITEHEADER]);

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)     // row
			for(j=x;j<w;j++) {  // col

				if( *buf_ptr != NOCOLOR )
					; // PutPixel( j,i, *buf_ptr );

				++buf_ptr;
			}

			UnlockSurface();

			break;
		}

		case 3: {

			ULONG color;
			UCHAR *buf_ptr = &buf[SPRITEHEADER];

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)    // row
			for(j=x;j<w;j++) {  // col
				color = ( (ULONG)(*buf_ptr++) << 16 ) |
					( (ULONG)(*buf_ptr++) << 8 ) |
					( (ULONG)(*buf_ptr++) );
				if(color!=NOCOLOR)
					; // PutPixel(j,i,color);
			}

			UnlockSurface();

			break;
		}
	}


	return;
}




//
// generic
//
void PutSpriteBlackMask(int x,int y,UCHAR *buf,ULONG color) {

	int i,j,w,h;
	UCHAR *buf_ptr;

	xprintf("PutSpriteBlackMask: is old!\n");

	CHECKACTIVE;

	buf_ptr = &buf[SPRITEHEADER];
	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;

	LockSurface( LS_WRITE );

	for(i=y;i<h;i++)   // row
		for(j=x;j<w;j++) {  // col
			if(*buf_ptr!=NOCOLOR)
				; // PutPixel(j,i,color);
			++buf_ptr;
		}

	UnlockSurface();

	return;
}





//
// generic
//
void PutSpriteBlackInvMask(int x,int y,UCHAR *buf,ULONG color) {

	int i,j,w,h;
	UCHAR *buf_ptr;

	xprintf("PutSpriteBlackInvMask: is old!\n");

	CHECKACTIVE;

	buf_ptr = &buf[SPRITEHEADER];
	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;

	LockSurface( LS_WRITE );

	for(i=y;i<h;i++)   // row
		for(j=x;j<w;j++) {  // col
			if(*buf_ptr==NOCOLOR)
				; // PutPixel(j,i,color);
			++buf_ptr;
		}

	UnlockSurface();

	return;
}





//
// generic
//
void PutSpriteBlackTrans( int x, int y, UCHAR *buf ) {

	int i,j,w,h;
	UCHAR *buf_ptr;

	xprintf("PutSpriteBlackTrans: is old!\n");

	CHECKACTIVE;

	buf_ptr = &buf[SPRITEHEADER];
	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;

	switch( SPRPIXELLEN( buf ) ) {

		case 1:

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)   // row
			for(j=x;j<w;j++) {  // col
				if(*buf_ptr!=NOCOLOR)
					; // PutPixel(j,i,TRANSCOLOR(*buf_ptr,XL_GetPixel(j,i)));
				++buf_ptr;
			}

			UnlockSurface();

		case 2: {

			USHORT *screen, *scr_ptr;
			USHORT *buf_ptr = (USHORT*)&(buf[SPRITEHEADER]);

			ALLOCMEM( screen, SPRITESIZE(buf) );
			scr_ptr = (USHORT *)((UCHAR *)screen + SPRITEHEADER);

			LockSurface( LS_READ );

			for(i=y;i<h;i++)     // row
			for(j=x;j<w;j++)
				*scr_ptr++ = (USHORT)XL_GetPixel(j,i);

			UnlockSurface();

			scr_ptr = (USHORT *)((UCHAR *)screen + SPRITEHEADER);

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)     // row
			for(j=x;j<w;j++) {  // col

				; // PutPixel( j,i, BlendColor( *scr_ptr++, *buf_ptr++ ) );
			}

			UnlockSurface();

			FREEMEM( screen );
		}
	}

	return;
}




//
// generic
//
void PutSpriteMaskBuf( int x, int y, UCHAR *buf, UCHAR *mask ) {

	int i,j,w,h;
	UCHAR *line;

	xprintf("PutSpriteMaskBuf: is old!\n");

	if( (active_flag<0) || (mask == NULL) ) return;

	w = SPRITEW(buf) + x;
	h = SPRITEH(buf) + y;

	switch( SPRPIXELLEN( buf ) ) {

		case 1: {

			UCHAR *buf_ptr = &buf[SPRITEHEADER];

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)    // row
			for(j=x;j<w;j++) {  // col
				if(*buf_ptr!=NOCOLOR)
					; // PutPixel(j,i,*buf_ptr);
				++buf_ptr;
			}

			UnlockSurface();

			break;
		}

		case 2: {

			USHORT *buf_ptr = (USHORT*)&(buf[SPRITEHEADER]);

			LockSurface( LS_WRITE );

			for( i=y; i<h; i++ ) {	// row

				line = SPRLINE(mask, i-y);

				for( j=x; j<w; j++ ) {	// col

					if( *(line++) != 0 )
						; // PutPixel( j,i, *buf_ptr );

					++buf_ptr;
				}
			}

			UnlockSurface();

			break;
		}

		case 3: {

			ULONG color;
			UCHAR *buf_ptr = &buf[SPRITEHEADER];

			LockSurface( LS_WRITE );

			for(i=y;i<h;i++)    // row
			for(j=x;j<w;j++) {  // col
				color = ( (ULONG)(*buf_ptr++) << 16 ) |
					( (ULONG)(*buf_ptr++) << 8 ) |
					( (ULONG)(*buf_ptr++) );
				if(color!=NOCOLOR)
					; // PutPixel(j,i,color);
			}

			UnlockSurface();

			break;
		}
	}


	return;
}



//
// generic
//
void PutSpriteRect( int x, int y, UCHAR *buf, int ox, int oy, int w, int h ) {

	int sx,sy;
	USHORT *wbuf_ptr;

	xprintf("PutSpriteRect: is old!\n");

	CHECKACTIVE;

	LockSurface( LS_WRITE );

	CLAMPMINMAX( w, 0, SPRITEW(buf) - ox );
	CLAMPMINMAX( h, 0, SPRITEH(buf) - oy );

	if( ISSPR16(buf) ) {

		for( sy=y; sy<y+h; sy++ ) {

			wbuf_ptr = (USHORT*)&SPR( buf, ox, oy+(sy-y) );

			// memcpy( (UCHAR*)GetVideoPtr(x,sy), wbuf_ptr, w*2 );

			for( sx=x; sx<x+w; sx++ ) {

				; // PutPixel( sx, sy, RGBINT16(*wbuf_ptr) );

				++wbuf_ptr;
			}
		}
	}

	UnlockSurface();

	return;
}





//
//
//
void PutMoreSprite( sprite_t *s, int num ) {

	int x0,y0,x1,y1,x,y;
	int i;
	ULONG color;

	xprintf("PutMoreSprite: is old!\n");

	CHECKACTIVE;

	x0 = INT_MAX;
	y0 = INT_MAX;
	x1 = INT_MIN;
	y1 = INT_MIN;


	for( i=0; i<num; i++ ) {
		if( s[i].x<x0 )
			x0 = s[i].x;
		if( s[i].y<y0 )
			y0 = s[i].y;
		if( (s[i].x+SPRITEW(s[i].spr))>x1 )
			x1 = s[i].x+SPRITEW(s[i].spr);
		if( (s[i].y+SPRITEH(s[i].spr))>y1 )
			y1 = s[i].y+SPRITEH(s[i].spr);
	}


	LockSurface( LS_WRITE );

	for( y=y0; y<y1; y++ )
	for( x=x0; x<x1; x++ ) {

		for( i=0; i<num; i++ ) {
			if( x>=s[i].x && y>=s[i].y )
				if( x<(s[i].x+SPRITEW(s[i].spr)) && y<(s[i].y+SPRITEH(s[i].spr)) )
					color = SPR( s[i].spr, x-s[i].x, y-s[i].y );
		}

		; // PutPixel( x,y, color );
	}

	UnlockSurface();

	return;
}





//
// generic
//
void GenGetSprite(memptr_ptr ptr,int x,int y,int w,int h) {

	int i,j,dx,dy;
	UCHAR *buf_ptr;

	CHECKACTIVE;

	ALLOCMEM( *ptr, SPRITEHEADER+w*h );
	MKSPRW(*ptr,w);
	MKSPRH(*ptr,h);
	MKSPR8( *ptr );

	buf_ptr = &(*ptr)[SPRITEHEADER];
	dx = w + x;
	dy = h + y;

	LockSurface( LS_WRITE );

	for(i=y;i<dy;i++)   // row
		for(j=x;j<dx;j++)  // col
			*(buf_ptr++) = (UCHAR)XL_GetPixel(j,i);

	UnlockSurface();

	return;
}


//
// generic
//
void GenGetSpriteNC(memptr_ptr ptr,int x,int y,int w,int h) {

	int i,j,dx,dy;
	UCHAR *buf_ptr;

	CHECKACTIVE;

	ALLOCMEM( *ptr, SPRITEHEADER+w*h );
	MKSPRW(*ptr,w);
	MKSPRH(*ptr,h);
	MKSPR8( *ptr );

	buf_ptr = &(*ptr)[SPRITEHEADER];
	dx = w + x;
	dy = h + y;

	LockSurface( LS_WRITE );

	for(i=y;i<dy;i++)   // row
		for(j=x;j<dx;j++)  // col
			*(buf_ptr++) = (UCHAR)XL_GetPixel(j,i);

	UnlockSurface();

	return;
}





//
// generic
//
void GetSpriteBuf( memptr_t ptr, int x0,int y0, int w,int h ) {

	int x,y,x1,y1;

	CHECKACTIVE;

	MKSPRW(ptr,w);
	MKSPRH(ptr,h);
	MKSPRBPP( ptr );

	x1 = w + x0;
	y1 = h + y0;

	LockSurface( LS_READ );

	switch( bpp ) {

		case 8: {

			UCHAR *buf = &ptr[SPRITEHEADER];

			for(y=y0;y<y1;y++)  // row
			for(x=x0;x<x1;x++)  // col
				*buf++ = (UCHAR)XL_GetPixel(x,y);

			break;
		}

		case 16: {

			USHORT *buf = (USHORT*)&(ptr[SPRITEHEADER]);

			for(y=y0;y<y1;y++)
			for(x=x0;x<x1;x++)
				*buf++ = (UCHAR)XL_GetPixel(x,y);

			break;
		}

		case 24:
		default:
			Quit("GetSpriteBuf: not yet!");
			break;
	}

	UnlockSurface();

	return;
}




//
// Opengl nem az
// Dx az
//
BOOL GfxThreadSafe( void ) {

	if( GFXDLL_GetData ) {
		ULONG ulong = GFXDLL_THREADSAFE;
		GFXDLL_GetData( &ulong, sizeof(ulong) );
		return (BOOL)ulong;
	}

	return FALSE;
}



////////////////////////////
//
// Project 64 szerű kiírás
//
////////////////////////////


//
//
//
void P64( char *str ) {

	static int p64_tic = -TICKBASE;
	static char p64_str[256] = "";
	int tic = GetTic();

#define P64EFFECTTIME (TICKBASE/3)
#define P64FULLTIME (TICKBASE*5)
#define P64MAXW (SCREENW*7/8)
#define P64MAXH (SCREENH/10)

	if( str ) {
		strncpy( p64_str, str, 256 );
		p64_tic = tic + P64FULLTIME;
	}

	if( tic > p64_tic )
		return;

	rgb_t main_font_color = { 205,210,220, 255 };
	rgb_t sec_font_color = { 120,166,215, 255 };

	int h = P64MAXH;
	int w = P64MAXW;
	int state = 0;
	FLOAT percent = 1.0f;

	if( tic < (p64_tic - (P64FULLTIME-P64EFFECTTIME)) )
		// kijön
		percent = 0.2f + 0.8f-((((FLOAT)p64_tic - (P64FULLTIME-P64EFFECTTIME))-(FLOAT)tic)/P64EFFECTTIME)*0.8f;

	if( tic > p64_tic - P64EFFECTTIME )
		// összemegy
		percent = 0.2f + (((FLOAT)p64_tic-(FLOAT)tic)/P64EFFECTTIME)*0.8f;

	// xprintf("percent: %f\n",percent);

	w = ftoi((FLOAT)w * percent);
	int x = SCREENW/2 - w/2;

	int y = SCREENH - (h + h/2) + ftoi((FLOAT)h/2 * (1-percent));
	h = ftoi((FLOAT)h * percent);

	const int border = ftoi( 5 * (FLOAT)SCREENH / 600 );

	DrawTexRect( x,y, x+w,y+h, 7 );
	DrawTexRect( x+border,y+border, x+w-border,y+h-border, 6 );

	if( strlen(p64_str) &&
	   ( tic > (p64_tic - (P64FULLTIME-P64EFFECTTIME*4/5)) ) &&
	   ( tic < p64_tic - P64EFFECTTIME*4/5 ) ) {

		fontloader_t fontloader;

		if( SCREENW <= 800 ) {
			fontloader.x = ftoi( 26 * (FLOAT)SCREENW / 800 );
			fontloader.y = ftoi( 26 * (FLOAT)SCREENH / 600 );
		}
		else {
			fontloader.x = ftoi( 26 + 5 );
			fontloader.y = ftoi( 26 + 5 );
		}

	 	SetFont(LoadFont3D("LiberationSans-Regular.ttf",TRUE,FALSE,&fontloader,1));
		SetFontAlpha( FA_ALL, main_font_color.r,main_font_color.g,main_font_color.b, main_font_color.a );

		int sw = MeasureString(p64_str);
		int sh = ftoi( (FLOAT)FontHeight() + 10 * (FLOAT)SCREENH / 600 );

		y = SCREENH - (P64MAXH + P64MAXH/2);

		WriteString((SCREENW-sw)/2,y+P64MAXH/2-sh/2,p64_str);
	}

	return;
}



////////////////////////////
//
// Status
//
////////////////////////////

static int status_font = 1;

#define ST_MAXLEN	( XMAX_PATH * 2 )
#define ST_TIMEOUT	( TICKBASE * 7 )
#define ST_MAXROW	3


typedef struct statusrow_s {

	char str[ST_MAXLEN+1];
	ULONG tic;

	BOOL onscreen;

	stcolor_e color;
	rgb_t rgb;
	int font;

} statusrow_t, *statusrow_ptr;


static statusrow_t st_table[ST_MAXROW] = {
	{ "",0,0,ST_RESET,{0,0,0,0},0 },
	{ "",0,0,ST_RESET,{0,0,0,0},0 },
	{ "",0,0,ST_RESET,{0,0,0,0},0 },
};
static int ntexts = 0;



//
//
//
static BOOL InitStatus( void ) {

	static BOOL status_inited = FALSE;

	if( status_inited == TRUE )
		return FALSE;

	char font_name[XMAX_PATH];
	char *key_name = "xlib_status_font";

	// Creabbb_.ttf
	// LiberationSans-Bold.ttf
	XLIB_winReadProfileString( key_name, "BebasNeue Bold.ttf", font_name, MAX_PATH  );
	XLIB_winWriteProfileString( key_name, font_name );

	fontloader_t fontloader;
	fontloader.x = 40;
	fontloader.y = 40;
	fontloader.flag = FL_OUTLINE;
	status_font = LoadFont3D( font_name, TRUE, FALSE, &fontloader, 1 );

	memset( st_table, 0, sizeof(statusrow_t)*ST_MAXROW );

	status_inited = TRUE;
	xprintf("InitStatus: status inited with %d rows, %d sec.\n", ST_MAXROW, ST_TIMEOUT / TICKBASE );

	return TRUE;
}




//
//
//
static void TomoritStatus( void ) {

	int i;

	// tömörít
	for( i=0; i<ST_MAXROW; i++ )
		if( st_table[i].onscreen == FALSE ) {
			if( i < (ST_MAXROW-1) )
				memmove( &st_table[i],
					&st_table[i+1],
					sizeof(statusrow_t)*(ST_MAXROW-(i+1)) );

			memset( &st_table[ST_MAXROW-1], 0, sizeof(statusrow_t) );
		}

	return;
}



//
//
//
void StatusText( stcolor_e color, char *s, ... ) {

	char text[ST_MAXLEN+1];
	va_list args;
	int i;

	if( color == ST_RESET || s == NULL ) {

		for( i=0; i<ST_MAXROW; i++ ) {
			st_table[i].onscreen = FALSE;
			st_table[i].tic = 0L;
		}

		return;
	}

	va_start( args, s );
	_vsnprintf( text, ST_MAXLEN, s, args );
	va_end( args );

	if( text[0] == 0 )
		return;

	InitStatus();

	int game_tic = GetTic();

	for( i=0; i<ST_MAXROW; i++ )
		if( (st_table[i].tic + ST_TIMEOUT) < game_tic )
			st_table[i].onscreen = FALSE;

	TomoritStatus();

	for( i=0; i<ST_MAXROW; i++ )
		if( st_table[i].onscreen == FALSE )
			break;

	// fullos
	if( i>=ST_MAXROW ) {
		memmove( &st_table[0],
			 &st_table[1],
			 sizeof(statusrow_t)*(ST_MAXROW-1) );
		i = ST_MAXROW - 1;
	}

	strncpy( st_table[i].str, text, ST_MAXLEN );

	st_table[i].tic = game_tic;
	st_table[i].onscreen = TRUE;
	st_table[i].color = color;

	xprintf("StatusText: adding \"%s\" at %d row.\n", st_table[i].str, i );

	return;
}







//
//
//
void DrawStatus( void ) {

	int game_tic = GetTic();
	int i,x,y,d;

	y = SCREENH / 4;

	for( i=ST_MAXROW-1; i>=0; i-- ) {

		if( (st_table[i].tic + ST_TIMEOUT) < game_tic )
			st_table[i].onscreen = FALSE;

		// biztonsági szelep
		if( st_table[i].tic > (game_tic + ST_TIMEOUT*2) ) {
			xprintf("DrawStatus: strange timeout.\n");
			st_table[i].tic = game_tic;
		}

		if( st_table[i].str[0] == 0 )
			st_table[i].onscreen = FALSE;

		if( st_table[i].onscreen == FALSE )
			continue;

		// csak ha van mit kiírni akkor init
		InitStatus();
		SetFont( status_font );

		x = (SCREENW - MeasureString( st_table[i].str )) / 2;

		d = (st_table[i].tic + ST_TIMEOUT) - game_tic;
		CLAMPMINMAX( d, 0, ST_TIMEOUT / 4 );

		int alpha = ftoi( ((FLOAT)d / (ST_TIMEOUT / 4.0f)) * 256.0f );

#define SHADOWMAXALPHA 127

		// árnyék félig átlátszó
		SetFontAlpha( FA_ALL, 10,10,10, alpha>SHADOWMAXALPHA?SHADOWMAXALPHA:alpha );
		WriteString( x+3,y+3, st_table[i].str );

		// SetFontAlpha( FA_ALL, status_rgb.r,status_rgb.g,status_rgb.b, status_rgb.a );
		switch( st_table[i].color ) {

			case ST_YELLOW: SetFontAlpha( FA_EFFECT, FA_ALL,FR_YELLOW,0, alpha ); break;
			case ST_RED:	SetFontAlpha( FA_EFFECT, FA_ALL,FR_RED,0, alpha ); break;
			case ST_GREEN:	SetFontAlpha( FA_EFFECT, FA_ALL,FR_GREEN,0, alpha ); break;
			case ST_BLUE:	SetFontAlpha( FA_EFFECT, FA_ALL,FR_BLUE,0, alpha ); break;
			case ST_PURPLE:	SetFontAlpha( FA_EFFECT, FA_ALL,FR_PURPLE,0, alpha ); break;
			case ST_GOLD:	SetFontAlpha( FA_EFFECT, FA_ALL,FR_GOLD,0, alpha ); break;
			case ST_BW:
			default:	SetFontAlpha( FA_EFFECT, FA_ALL,FR_BW,0, alpha); break;
		}

		WriteString(x,y, st_table[i].str );

		y += FontHeight() * 1.0f;
	}

	TomoritStatus();

	return;
}




//
//
//
void ClearStatusText( void ) {

	int i;

	memset( st_table, 0, sizeof(statusrow_t)*ST_MAXROW );

	xprintf("ClearStatusText: table cleared.\n");

	return;
}


