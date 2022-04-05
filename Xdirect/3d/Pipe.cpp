/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#include <float.h>
#include <limits.h>
#include <string.h>

#include <xlib.h>
#include <xinner.h>

RCSID( "$Id: pipe.c,v 1.2 97-03-10 17:00:06 bernie Exp $" )



static point3_t vpos,		// viewer's position
				vdir,		//  Ä´ÃÄ    direction
				vnormal,	//  Ä´ÃÄ    normal = vpos - vdir;
				mvnormal;	// mvnormal = dir - pos;

// static FLOAT zclipnear = FLOAT_EPSILON;
static FLOAT zclipnearP2 = FLOAT_EPSILON;
// static FLOAT zclipfar = FLOAT_MAX;
static FLOAT zclipfarP2 = FLOAT_MAX;



renderinfo_t renderinfo = {0,0,0,0,0,0,0,0,0};



//
//
//
void SetupCulling( point3_t pos, point3_t dir ) {

	CopyVector( vpos, pos );
	CopyVector( vdir, dir );

	SubVector( pos, dir, vnormal );
	NormalizeVector( vnormal );

	SubVector( dir, pos, mvnormal );
	NormalizeVector( mvnormal );

	// zclipnear = zn;
	zclipnearP2 = zclipnear.value * zclipnear.value;
	// zclipfar = zf;
	zclipfarP2 = zclipfar.value * zclipfar.value;

	if( GFXDLL_SetupCulling ) GFXDLL_SetupCulling( pos, dir, vnormal, zclipnear.value, zclipfar.value );

	return;
}




//
//
//
void BeginScene( void ) {

	if( GFXDLL_BeginScene ) GFXDLL_BeginScene();

	renderinfo.objs = 0;
	renderinfo.rejectedobjs = 0;

	renderinfo.faces = 0;
	renderinfo.rejectedfaces = 0;

	renderinfo.verts = 0;
	renderinfo.xformedverts = 0;

	renderinfo.bboxfacenum = 0;
	renderinfo.rejectedbboxfacenum = 0;

	renderinfo.xformframe_cnt += 1;

	return;
}




//
//
//
void EndScene( void ) {

	if( GFXDLL_EndScene ) GFXDLL_EndScene();

	// xprintf("polys: %d, %d  verts: %d, %d\n",render_info.polys,render_info.rejected_polys,render_info.verts,render_info.xformed_verts);

	return;
}



//
//
//
void FlushScene( void ) {

	if( GFXDLL_FlushScene ) GFXDLL_FlushScene();

	return;
}





//
//
//
void PutSpritePoly( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) {

	if( GFXDLL_PutSpritePoly ) GFXDLL_PutSpritePoly( ppoly, inst, texmapid, rgb );

	renderinfo.faces += 1;
	renderinfo.verts += ppoly.npoints;

	return;
}


//
//
//
void PutPoly( polygon_t ppoly, rgb_t *rgb ) {

	if( GFXDLL_PutPoly ) GFXDLL_PutPoly( ppoly, rgb );
	else xprintf("no GFXDLL_PutPoly.\n");

	renderinfo.faces += 1;
	renderinfo.verts += ppoly.npoints;

	return;
}




//
//
//
BOOL PolyFacesViewer( object_t *obj, int num, clippoly_t unclipped_poly, point3_t fnormal, point3_t fcenter ) {

	FLOAT dist;
	int i;
	BOOL bIn = FALSE;

	for( i=0; i<unclipped_poly.nverts; i++ ) {

		dist = ((unclipped_poly.vert[i][0] - vpos[0]) * (unclipped_poly.vert[i][0] - vpos[0])) +
		       ((unclipped_poly.vert[i][1] - vpos[1]) * (unclipped_poly.vert[i][1] - vpos[1])) +
		       ((unclipped_poly.vert[i][2] - vpos[2]) * (unclipped_poly.vert[i][2] - vpos[2]));

		if( dist <= zclipfarP2 )
			bIn = TRUE;
	}

	if( bIn == FALSE )
		return FALSE;

	/***
	dist = (center[0] - vpos[0]) * (center[0] - vpos[0]) +
	       (center[1] - vpos[1]) * (center[1] - vpos[1]) +
	       (center[2] - vpos[2]) * (center[2] - vpos[2]);

	if( ( dist > zclipfarP2 ) )	// ( dist < zclipnearP2 ) )
		return FALSE;
	***/

//	if( !ISFLAG( obj->face[num].flag, FF_DBLSIDE ) )
//	if( DotProduct( vnormal, fnormal ) > deg2rad(37.0f) )	// 0.65 (49)
//		return FALSE;

	//SubVector( center, vpos, v );
	//NormalizeVector( v );

	//if( DotProduct( v, mvnormal ) < 0.30 )
	//	return FALSE;

	return TRUE;
}




//
//
//
BOOL _______PolyFacesViewerDblside( point3_t normal, point3_t center ) {

	FLOAT dist;

	dist = (center[0] - vpos[0]) * (center[0] - vpos[0]) +
	       (center[1] - vpos[1]) * (center[1] - vpos[1]) +
	       (center[2] - vpos[2]) * (center[2] - vpos[2]);

	if( ( dist > zclipfarP2 ) )	// ( dist < zclipnearP2 ) )
		return FALSE;

	//if( DotProduct( vnormal, normal ) > 0.65 )
	//	return FALSE;

	//SubVector( center, vpos, v );
	//NormalizeVector( v );

	//if( DotProduct( v, mvnormal ) < -M_PI_2 )
	//	return FALSE;

	return TRUE;
}





#if 0
/*

					Simple polygon rendering by Nathan Whitaker
					-------------------------------------------

About this document
-------------------

I accept no responsibility for anything to do with this document etc....


I spend a lot of time on comp.graphics.algorithms and the rec.games.programmer newsgroups and
have read many questions from many different beginners. I have noticed that these questions
all seem to follow a common trend and the most widely asked question is how do I fill a polygon.
In short I have created this document to answer the question. The polygon rendering techniques
I am going to describe are very easy to code and understand but, are certainly not the
fastest ( they were my first - hence the sloppy code ). I personally use other rendering methods
for all the polygon primitives described below and all of my polygon code is in optimised
assembler.

This document was originally released as part of a zip file which contains an executable which
demonstrates all the items in this document. Hit + and - on the numeric keypad to change the
render type when in the demo. render types are Flat with Lambert, Gourard Lambert, Phong,
affine textured, affine environment mapped, bump mapped with Lambert, environment & bump mapped
with Lambert. The routines are old and are meant to demonstrate - not to impress.



		THESE POLYGON ROUTINES WORK ON THE ASSUMPTION THAT THE POLYGON IS CONVEX!
		-------------------------------------------------------------------------





Contact
-------
Nathan Whitaker,
Games Programmer,
Reflections Software 		Email : Naif@rflect.demon.co.uk





Contents
--------

( 1 ) 	Simple line draw algorithm - strange enough this is at the heart of our polygon
		primitives.

( 2 )	Flat shaded polygon rendering - converting the line draw routine.

( 3 )	Gourard shaded polygons.

( 4 )	Affine texture mapped polygons.

( 5 )	Perspective correct texture mapping.

( 6 )	Adding gourard shading to texture mapping.

( 7 )	Bump mapped polygons.

( 8 )	Environment mapping.

( 9 )	Phong shaded polygons.






( 1 )	Simple line draw algorithm
----------------------------------
There are many ways to render a polygon but, by far the easiest is to trace along the edges
of the polygon storing the leftmost and rightmost x coorinate for each row of the polygon then
filling horizontly between these points. To do this we need a routine which returns calculates
points on a line so lets start with a simple line drawing algorithm which we will convert into
a polygon edge tracer in section 2.

I am going to describe one of the simplest yet fastest line drawing algorithms I know of. This
line draw routine first of all calculates the x and y delta between the two line points:-

	deltax = x1 - x0;
	deltay = y1 - y0;

Next the routine branches to one of two drawing cases based upon the largest absolute value of
the two deltas. If the xdelta is the greater of the two we draw one pixel horizontally in the
x direction of the line every loop iteration for a loop count of abs( dx ). You are correct in
assuming that this will produce a horizontal line but, thats not what we want so we somehow need
to know when we should move the y position up / down a row. We work out when to move a row by
keeping a total which we will call error - every loop iteration we add the lesser delta - in this
case we add abs( ydelta ). We then compare error with the greater delta and if error is larger
we then move a row in the y direction and subtract the greater delta from the error. That
basically is the line routine and in the case of dy > dx we step in the y direction one pixel
every iteration and check when to move horizontally in the manner above.

I have made a right mess of explaining this so I have enclosed source code for the line routine.


void DrawLine( int x0, int y0, int x1, int y1, int color )
{
	int udx, udy, dx, dy, error, loop, xadd, yadd;

	dx = x1 - x0;	   	//	Work out x delta
	dy = y1 - y0;	   	//	Work out y delta

	udx = abs( dx );	//	udx is the unsigned x delta
	udy = abs( dy );	//	udy is the unsigned y delta

	if ( dx < 0 )		//	Work out direction to step in the x direction.
		xadd = -1;
	else
		xadd = 1;

	if ( dy < 0 )		//	Work out direction to step in the y direction.
		yadd = -1;
	else
		yadd = 1;

	error = 0;

	loop = 0;

	if ( udx > udy )
	{
		do						//	Delta X > Delta Y
		{
			error += udy;

			if ( error >= udx )	//	Time to move up / down?
			{

				error -= udx;

				y0 += yadd;
			}

			loop++;

			d_buffer[ ( y0 * 320 ) + x0 ] = color;	// Plot pixel - d_buffer is my screen buffer.
			x0 += xadd;			// Move horizontally.
		}
		while ( loop < udx );	// Repeat for x length of line.
	}
	else
	{
		do						// Delta Y > Delta X
		{
			error += udx;

			if ( error >= udy )	// Time to move left / right?
			{
				error -= udy;

				x0 += xadd;		// Move across.
			}

			loop++;

			d_buffer[ ( y0 * 320 ) + x0 ] = color;	// Plot pixel.
			y0 += yadd;	// Move up / down a row.
		}
		while ( loop < udy );	// Repeat for y length of line.
	}
}









( 2 )	Flat shaded polygon rendering - converting the line draw routine.
-------------------------------------------------------------------------
All that we need to do to turn our line drawing routine into a polygon edge tracer is to modify
the part where we plot a pixel at x,y to store an x coordinate for that row. We are going to
store these x coordinates ( there should be 2 for a convex poly ) in the structure below:-

typedef struct	{
					signed short x;
				}	EDGE_LIST;

EDGE_LIST poly_edge_list[ HEIGHT_OF_SCREEN ][ 2 ];

poly_edge_list is an array of screen height elements ( screen height as you should clip polys to
the screen ) containing two x coordinates. What this means is for every y row of the polygon we
have two x coordinates which we can quickly fill between to create a polygon. What this means for
our edge tracer is that it should only store one x coordinate per row every time it is called.
This is simple to implement as in the case of deltax > deltay we have a section of code which
is entered only when we move a row....

if ( error >= udx )	//	Time to move up / down?
{

	error -= udx;

	y0 += yadd;
}

....and in the case of deltay > deltax we increment / decrement the row every iteration anyway.
Your modified line routine should resemble the code below.

void DrawEdge( int x0, int y0, int x1, int y1 )
{
	int udx, udy, dx, dy, error, loop, yadd, max, temp;
	EDGE_LIST *left_edge_ptr, *right_edge_ptr;

	if ( x0 > x1 )			// Draw left - right always
	{

		temp = x0;
		x0 = x1;
		x1 = temp;

		temp = y0;
		y0 = y1;
		y1 = temp;
	}

	left_edge_ptr = &poly_edge_list[ y0 ][ 0 ];
	right_edge_ptr = &poly_edge_list[ y0 ][ 1 ];

	dx = x1 - x0;
	dy = y1 - y0;

	if ( dx || dy )			// Check coordinates are not equal
	{
		udx = abs( dx );
		udy = abs( dy );

		if ( dy < 0 )
			yadd = -2;			// One row is 2 elements left & right - hence 2 not 1
		else
			yadd = 2;

		error = 0;

		loop = 0;

		if ( udx > udy )
		{
			max = udx;

			if ( left_edge_ptr->x < 0 )
				left_edge_ptr->x = x0;
			else
				if ( right_edge_ptr->x < 0 && x0 != left_edge_ptr->x )
					right_edge_ptr->x = x0;

			do
			{
				error += udy;

				loop++;

				x0++;

				if ( error >= udx )
				{

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;

					error -= udx;

					if ( left_edge_ptr->x < 0 )	// edge_buffer initialsed to -1
						left_edge_ptr->x = x0;
					else
						if ( x0 != left_edge_ptr->x )
							right_edge_ptr->x = x0;
				}
			}
			while ( loop < max );
		}
		else
			if ( udx < udy )
			{
				max = udy + 1;

				do
				{
					if ( left_edge_ptr->x < 0 )
						left_edge_ptr->x = x0;
					else
						if ( x0 != left_edge_ptr->x )
							right_edge_ptr->x = x0;

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;

					error += udx;

					if ( error >= udy )
					{
						error -= udy;

						x0++;
					}

					loop++;
				}
				while ( loop < max );
			}
			else				// New case for delta x = delta y
			{
				max = udy + 1;

				do
				{
					if ( left_edge_ptr->x < 0 )
						left_edge_ptr->x = x0;
					else
						if ( x0 != left_edge_ptr->x )
							right_edge_ptr->x = x0;

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;
					x0++;

					loop++;
				}
				while ( loop < max );
			}
	}
}

You have probably noticed a few strange things about the code above heres an explanation:

( 1 ) The third case deltax = deltay has been added to prevent leaking at 45 degrees.

( 2 ) What does this do -

		if ( left_edge_ptr->x < 0 )
			left_edge_ptr->x = x0;
		else
			if ( x0 != left_edge_ptr->x )
				right_edge_ptr->x = x0;	????

	  If the slot 0 of the array is empty, set to x otherwise check slot 2 and if empty set to x.
	  The whole array of slots is set to -1 ( empty ) at the start of each polygon.

( 3 ) Why are we drawing length + 1 and why is the order different in each case?
	  This is due to the fact these were my first sloppy polygon routines and they were made in a
	  hurry - they are there to prevent leaking. I guarantee that you will be sick of leaking by
	  the end of your first polygon routine.



Take a quick look at figure 1 below and come back...... Now I've explained the edge tracer
conversion I will show you how you would render the polygon in figure 1.

			( 1 ) Work out the height of the polygon - largest y - smallest y.

				  Make vertex coordinates relative to smallest y e.g for every
				  vertex y subtract smallest y.

			( 2 ) Fill polygon height entries in poly_edge_list with -1 to indicate
				  that they are empty slots.

		    ( 3 ) Edge trace vertex 0 - vertex 1.
				  Edge trace vertex 1 - vertex 2.
				  Edge trace vertex 2 - vertex 3.
				  Edge trace vertex 3 - vertex 0.

			( 4 ) Loop through the poly_edge_list drawing horizontal lines of pixels
				  between any slots that contain 2 x values.




		2
	   /\
	  /	 \
	 /	  \
   1/	   \
	!		\
	!		 \
	!         \
	!         /	3
	!		 /
	!		/
	!	   /
	!	  /
	!	 /
	!	/
	!  /
	! /
	!/
	0

Figure 1
--------
Above is a really crap 4 vertex ascii polygon with the vertices labelled 0, 1, 2, 3.



Finally this is the main core of a routine to draw a 4 point flat polygon.

void PolyF4( int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, char col )
{

								   // set initial best values.
	signed short yloop, tempx, tempy, top = 4096, bot = -4096, height;

	if ( y0 < top )		// Get top and bottom values of polygon.
		top = y0;

	if ( y1 < top )
		top = y1;

	if ( y2 < top )
		top = y2;

	if ( y3 < top )
		top = y3;

	if ( y0 > bot )
		bot = y0;

	if ( y1 > bot )
		bot = y1;

	if ( y2 > bot )
		bot = y2;

	if ( y3 > bot )
		bot = y3;

	height = bot - top;

	if ( height == 0 )
		return;

	for ( yloop = 0; yloop < height; yloop++ )
	{
		poly_edge_list[ yloop ][ 0 ].x = -1;
		poly_edge_list[ yloop ][ 1 ].x = -1;
	}

	y0 -= top;
	y1 -= top;
	y2 -= top;
	y3 -= top;

	DrawEdge( x0, y0, x1, y1 );
	DrawEdge( x1, y1, x2, y2 );
	DrawEdge( x2, y2, x3, y3 );
	DrawEdge( x3, y3, x0, y0 );

	for ( yloop = 0; yloop < height; yloop++ )
	{
		if ( poly_edge_list[ yloop ][ 0 ].x > 0 )		// Check an x1 and an x2 exist.
		{												//
			if ( poly_edge_list[ yloop ][ 1 ].x > 0 )	//
					DrawFRun( yloop + top, poly_edge_list[ yloop ][ 0 ].x, poly_edge_list[ yloop ][ 1 ].x, col );
		}
	}
}


// ------------------------------------------------------------------------

This routine fills between the two edges in the set fill color.

void DrawFRun( int y, int x0, int x1, char color )
{
	short loop;
	char *byte;

	if ( x0 == x1 )
		d_buffer[ ( y * 320 ) + x0 ] = color;
	else
		if ( x0 < x1 )
		{
			byte = &d_buffer[ ( y * 320 ) + x0 ];

			for ( loop = x0; loop < x1; loop++ )
				*byte++ = color;
		}
		else
		{
			byte = &d_buffer[ ( y * 320 ) + x1 ];

			for ( loop = x1; loop < x0; loop++ )
				*byte++ = color;
		}
}










( 3 )	Gourard shaded polygons.
--------------------------------
If you have read the code above and understood it gourard shading shouldn't be a problem. In this
section I will tell you how to draw a gourard shaded polygon given a set of shade values for each
vertex. I won't tell you how to calculate the shade at each vertex as I assume you know it -
dot product with face normal and light source etc ( any good 3d doc will tell you this ).

Given that you have a working flat polygon routine it isn't very hard to convert to a gourard
shaded routine. In terms of shades I assume you have set up a progressive dark to light palette
and you don't with to shade via a clut ( colour lookup table ), instead you want to shade directly
between the specified colours.
	The first step to converting the flat shader to a gourard shader is to change our EDGE_LIST
structure to include shade information like below:-

typedef struct	{
					signed short x;
					unsigned short col;
				}	EDGE_LIST;

Next we need to create a modified edge trace function which interpolates between the two vertex
shades and stores the shade into the poly_edge_list ( col ). We interpolate between the shades
by calculating a shade delta ( shade1 - shade0 ), multiplying this value by 256 ( for smoothing
purposes ) and dividing the result by the largest delta......

void DrawGEdge( int x0, int y0, int x1, int y1, unsigned char c0, unsigned char c1 )
{
	int udx, udy, dx, dy, error, loop, yadd, max, temp;
	EDGE_LIST *left_edge_ptr, *right_edge_ptr;
	unsigned short coladd;
	unsigned short col;

	if ( x0 > x1 )
	{

		temp = x0;
		x0 = x1;
		x1 = temp;

		temp = y0;
		y0 = y1;
		y1 = temp;

		temp = c0;
		c0 = c1;
		c1 = temp;

	}

	left_edge_ptr = &poly_edge_list[ y0 ][ 0 ];
	right_edge_ptr = &poly_edge_list[ y0 ][ 1 ];

	dx = x1 - x0;
	dy = y1 - y0;

	if ( dx || dy )
	{
		udx = abs( dx );
		udy = abs( dy );

		if ( dy < 0 )
			yadd = -2;
		else
			yadd = 2;

		error = 0;

		loop = 0;

		if ( udx > udy )
		{
			max = udx;

			coladd = ( ( c1 - c0 ) * 256 ) / udx;
			col = c0 * 256;

			if ( left_edge_ptr->x < 0 )
			{
				left_edge_ptr->x = x0;
				left_edge_ptr->col = c0;
			}
			else
				if ( right_edge_ptr->x < 0 && x0 != left_edge_ptr->x )
				{
					right_edge_ptr->x = x0;
					right_edge_ptr->col = c0;
				}

			do
			{
				error += udy;

				col += coladd;

				loop++;

				x0++;

				if ( error >= udx )
				{

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;

					error -= udx;

					if ( left_edge_ptr->x < 0 )
					{
						left_edge_ptr->x = x0;
						left_edge_ptr->col = col >> 8;
					}
					else
						if ( x0 != left_edge_ptr->x )
						{
							right_edge_ptr->x = x0;
							right_edge_ptr->col = col >> 8;
						}
				}
			}
			while ( loop < max );
		}
		else
			if ( udx < udy )
			{
				max = udy + 1;

				coladd = ( ( c1 - c0 ) * 256 ) / udy;
				col = c0 * 256;

				do
				{
					if ( left_edge_ptr->x < 0 )
					{
						left_edge_ptr->x = x0;
						left_edge_ptr->col = col >> 8;
					}
					else
						if ( x0 != left_edge_ptr->x )
						{
							right_edge_ptr->x = x0;
							right_edge_ptr->col = col >> 8;
						}

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;

					error += udx;

					if ( error >= udy )
					{
						error -= udy;

						x0++;
					}

					loop++;

					col += coladd;


				}
				while ( loop < max );
			}
			else
			{
				max = udy + 1;

				coladd = ( ( c1 - c0 ) * 256 ) / udy;
				col = c0 * 256;

				do
				{
					if ( left_edge_ptr->x < 0 )
					{
						left_edge_ptr->x = x0;
						left_edge_ptr->col = col >> 8;
					}
					else
						if ( x0 != left_edge_ptr->x )
						{
							right_edge_ptr->x = x0;
							right_edge_ptr->col = col >> 8;
						}

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;
					x0++;

					loop++;

					col += coladd;
				}
				while ( loop < max );
			}
	}
}

You have probably noticed how little actually changed from the original flat shader and to
complete the gourard routine all you now need to do is change the horizontal filler loop.
All we have to do the horizontal filler is to interpolate between the shades at each edge of
the poly and fill with that shade - see below.

void DrawGFRun( int y, int x0, int x1, unsigned char c0, unsigned char c1 )
{
	unsigned short coladd, col, temp;
	short loop, length;
	char *byte, val;

	if ( x0 == x1 )
		d_buffer[ ( y * 320 ) + x0 ] = c0;
	else
	{
		if ( x0 > x1 )
		{
			temp = x0;
			x0 = x1;
			x1 = temp;

			temp = c0;
			c0 = c1;
			c1 = temp;
		}

		length = x1 - x0;

		coladd = ( ( c1 - c0 ) * 256 ) / length;
		col = c0 * 256;

		byte = &d_buffer[ ( y * 320 ) + x0 ];

		for ( loop = 0; loop <= length; loop++ )
		{
			*byte++ = col >> 8;

			col += coladd;
		}
	}
}










( 4 )	Affine texture mapped polygons.
---------------------------------------
To convert the flat shader to be affine ( fitted ) texture mapped is just as simple as turning it
into a gourard shader except we interpolate U and V instead of shade. I assume you know what U
and V are but, in case you don't then in short they are X,Y positions in texture space.

I won't bother explaining the procedure as its almost identical to the gourard shader - I'll
merely present you with the code:-

typedef struct	{
					signed short x;
					unsigned short u, v;
				}	EDGE_LIST;

void DrawTEdge( int x0, int y0, int x1, int y1, int u0, int v0, int u1, int v1 )
{
	int udx, udy, dx, dy, error, loop, yadd, max, temp;
	unsigned short uadd, u, vadd, v;
	GEDGE *left_edge_ptr, *right_edge_ptr;

	if ( x0 > x1 )
	{

		temp = x0;
		x0 = x1;
		x1 = temp;

		temp = y0;
		y0 = y1;
		y1 = temp;

		temp = u0;
		u0 = u1;
		u1 = temp;

		temp = v0;
		v0 = v1;
		v1 = temp;
	}

	left_edge_ptr = &poly_edge_list[ y0 ][ 0 ];
	right_edge_ptr = &poly_edge_list[ y0 ][ 1 ];

	dx = x1 - x0;
	dy = y1 - y0;

	if ( dx || dy )
	{
		udx = abs( dx );
		udy = abs( dy );

		if ( dy < 0 )
			yadd = -2;
		else
			yadd = 2;

		error = 0;

		loop = 0;

		if ( udx > udy )
		{
			uadd = ( ( u1 - u0 ) * 256 ) / udx;
			vadd = ( ( v1 - v0 ) * 256 ) / udx;
			u = u0 * 256;
			v = v0 * 256;

			max = udx;

			if ( left_edge_ptr->x < 0 )
			{
				left_edge_ptr->x = x0;
				left_edge_ptr->u = u0;
				left_edge_ptr->v = v0;
			}
			else
				if ( right_edge_ptr->x < 0 && x0 != left_edge_ptr->x )
				{
					right_edge_ptr->x = x0;
					right_edge_ptr->u = u0;
					right_edge_ptr->v = v0;
				}

			do
			{
				error += udy;

				u += uadd;
				v += vadd;

				loop++;

				x0++;

				if ( error >= udx )
				{

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;

					error -= udx;

					if ( left_edge_ptr->x < 0 )
					{
						left_edge_ptr->x = x0;
						left_edge_ptr->u = u >> 8;
						left_edge_ptr->v = v >> 8;
					}
					else
						if ( x0 != left_edge_ptr->x )
						{
							right_edge_ptr->x = x0;
							right_edge_ptr->u = u >> 8;
							right_edge_ptr->v = v >> 8;
						}
				}
			}
			while ( loop < max );
		}
		else
			if ( udx < udy )
			{
				uadd = ( ( u1 - u0 ) * 256 ) / udy;
				vadd = ( ( v1 - v0 ) * 256 ) / udy;
				u = u0 * 256;
				v = v0 * 256;

				max = udy + 1;

				do
				{
					if ( left_edge_ptr->x < 0 )
					{
						left_edge_ptr->x = x0;
						left_edge_ptr->u = u >> 8;
						left_edge_ptr->v = v >> 8;
					}
					else
						if ( x0 != left_edge_ptr->x )
						{
							right_edge_ptr->x = x0;
							right_edge_ptr->u = u >> 8;
							right_edge_ptr->v = v >> 8;
						}

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;

					error += udx;

					if ( error >= udy )
					{
						error -= udy;

						x0++;
					}

					loop++;

					u += uadd;
					v += vadd;
				}
				while ( loop < max );
			}
			else
			{
				uadd = ( ( u1 - u0 ) * 256 ) / udy;
				vadd = ( ( v1 - v0 ) * 256 ) / udy;
				u = u0 * 256;
				v = v0 * 256;

				max = udy + 1;

				do
				{
					if ( left_edge_ptr->x < 0 )
					{
						left_edge_ptr->x = x0;
						left_edge_ptr->u = u >> 8;
						left_edge_ptr->v = v >> 8;
					}
					else
						if ( x0 != left_edge_ptr->x )
						{
							right_edge_ptr->x = x0;
							right_edge_ptr->u = u >> 8;
							right_edge_ptr->v = v >> 8;
						}

					left_edge_ptr += yadd;
					right_edge_ptr += yadd;
					x0++;

					loop++;

					u += uadd;
					v += vadd;
				}
				while ( loop < max );
			}
	}
}



// ------------------------------------------------------------------------

void DrawTRun( int y, int x0, int x1, unsigned char u0, unsigned char v0, unsigned char u1, unsigned char v1 )
{
	int x, xdiff, ydiff, index;
	unsigned short uadd, u, vadd, v, temp;
	short loop, length;
	char *byte, val;

	if ( x0 != x1 )
	{
		if ( x0 > x1 )
		{
			temp = x0;
			x0 = x1;
			x1 = temp;

			temp = u0;
			u0 = u1;
			u1 = temp;

			temp = v0;
			v0 = v1;
			v1 = temp;

		}

		length = x1 - x0;

		uadd = ( ( u1 - u0 ) * 256 ) / length;
		u = u0 * 256;
		vadd = ( ( v1 - v0 ) * 256 ) / length;
		v = v0 * 256;

		byte = &d_buffer[ ( y * 320 ) + x0 ];

		for ( loop = 0; loop <= length; loop++ )
		{

			// t_page is a 256 * 256 area of memory where my texture is loaded.

			*byte++ = t_page[ ( v & 0xff00  ) + ( u >> 8 ) ];

			u += uadd;
			v += vadd;
		}
	}
}








( 5 )	Perspective correct texture mapping.
--------------------------------------------
I haven't wrote a perspective correct texture mapper using the slooowww edge tracing method but,
it would be fairly simple to do. Store the data as in the affine mapper but, change the EDGE_LIST
to store an interpolated 1 / Z value as well as the U and V values. You must also now interpolate
between.....

			U0 * ( 1 / Z0 ) and U1 * ( 1 / Z1 ).
			V0 * ( 1 / Z0 ) and V1 * ( 1 / Z1 ).


...and store the above values into U, V.

Finally when you come to drawing a horizontal run divide the interpolated U and V values by
the interpolated 1 / Z value.

You may have noticed that the 1 / Z calculation won't convert to the integer scheme we have been
using but, don't worry - using the FPU is no longer considered voodoo and any good perspective
texture mapper utilises the FPU.

You may also have noticed that doing the above operations for every pixel in the fill loop will
be slooowww. It is a good idea to calculate perspective corrected U and V values every ? pixels
( ? usually is 8 ) and interpolate ( normally - affine like ) between these ? pixel spans. This
is a common technique and even has a nice long name - Bi Linear perspective correct texture
mapping.








( 6 )	Adding gourard shading to texture mapping.
--------------------------------------------------
This is simply a case of doing the changes to make the flat filler, gourard and also doing the
changes to make the flat filler, affine texture mapped. You should now have the following
structure:-


typedef struct	{
					signed short x;
					unsigned short u, v, col;
				}	EDGE_LIST;


Once you have changed your edge tracer to reflect the data above you will have to change your
horizontal filler but, how you shade your texture is up to you. I suggest two ways -

		( 1 ) If your texture is 16 colours then arrange your 256 colour palette like this.

					0 -  15 = original 16 colours darkest shades.
				   16 -  31 = original 16 colours second darkest shades.
					.....
					.....
					.....
					.....
				  240 - 255 = orignal 16 colours brighest shades.

			  Your filler's inner loop would look something like this:-


				for ( loop = 0; loop <= length; loop++ )
				{
					shade = col >> 8;

					// shade is and'ed to get the appropriate shade offset in multiples of 16.
					// t_page values are 0 - 15.

					*byte++ = ( shade & 0xf0 ) + t_page[ ( ( v & 0xff00 ) + ( u >> 8 ) ];

					u += uadd;
					v += vadd;
					col += coladd;
				}

				As in all the gourard shading code I have shown, I assume shades go anywhere
				in the region of 0 - 255.



		( 2 ) You may want to create a clut ( colour lookup table ) - for every colour in the
			  palette build a table of shade values which reflect the index of the closest
			  colour for each shade ( am I speaking japenese again... ):-


			  This is a clut of indexes to the 16 closest light to dark shades for each of the
			  256 colours.....

				char clut[ 256 ][ 16 ];


			  To use the clut above your filler's inner loop would look something like this:-


				for ( loop = 0; loop <= length; loop++ )
				{
					shade = col >> 8;

					texel = t_page[ ( ( v & 0xff00 ) + ( u >> 8 ) ];

					*byte++ = clut[ texel ][ shade >> 4 ];

					u += uadd;
					v += vadd;
					col += coladd;
				}


Of course you may want to use another method?????????










( 7 )	Bump mapped polygons.
-----------------------------
I have to be honest and say that bump mapping really ticks me off..... the reason is lame artists.
Myself and the other programmers here at Reflections spend all day, 5 days a week writing 'real'
routines such as freescape car ai, 3d clipping routines, fast 3d collisions, realistic car
simulations etc.... We regularly download demos over the internet and the bright flashing colours
and beepy music attract a herd of artists... The artists watch on until usually half way
through a demo a large light sourced bump mapped object spins into view and then it starts....
They all look at each other and start chatting about how we should employ the demo coders as
they have programmed this monster of a routine and we ( Reflections coders ) are all crap...
The annoying thing is that if these artists knew a single thing about programming
they would realise that bump mapping is extremely simple to program.
									    ----------------------------


So how do we do it???

Well, I think it would help if I quickly described how 2d bump mapping works.......

Setup -

First of all you create ( or in my case borrow ) an image which has been designed so that
the colour value of a pixel represents a height. Second of all you create a nice palette
which fades from dark to light. Third of all you create a predefined overlay representing
your light source. This overlay usually takes the form of circle which fades from dark at
the outer edges to light at the centre. Below is a sample piece of code to create the overlay.


#define LIGHT_DIAMETER 256		// 256 shades in overlay

char BumpOverlay[ LIGHT_DIAMETER * LIGHT_DIAMETER ];
int ypremultable[ 256 ];

void BuildBumpOverlay()
{
	int x, y, x1, y1, dist;

	for ( y = 0; y < LIGHT_DIAMETER; y++ )
	{
		for ( x = 0; x < LIGHT_DIAMETER; x++ )
		{
			x1 = x - ( LIGHT_DIAMETER / 2 );
			y1 = y - ( LIGHT_DIAMETER / 2 );

//			Work out distance from center of light source.
			dist = sqrt( ( x1 * x1 ) + ( y1 * y1 ) );

//			Closer to center = brighter.
			dist = ( LIGHT_DIAMETER / 2 ) - dist;

			if ( dist > 127 )
				dist = 127;
			else
				if ( dist < 0 )
					dist = 0;

//			Output values in the rance 0 - 254

			BumpOverlay[ ( y * LIGHT_DIAMETER ) + x ] = dist * 2;
		}
	}

//	This table is precalculated to remove an expensive inner loop multiply operation.

	for ( y = 0; y < 256; y++ )
		ypremultable[ y ] = y * LIGHT_DIAMETER;
}



Drawing -

To draw the bump mapped picture we first define an x and y position for the light source then
we draw 200 rows of 320 pixels performing the following calculations:-

			( 1 ) For every pixel we calculate a pseudo x normal - we do this by saying the
				  normal is the pixel to the right of the current pixel - the pixel to the left
				  of the current pixel. We also calculate a pseudo y normal - similiarly we
				  calculate it from the pixel below this pixel - the pixel above.

			( 2 ) We add a base value and the distance on the respected axis from the center of
				  the light source to each normal.

			( 3 ) We adjust both normals so that they are in the 0 - 255 range.

			( 4 ) We plot using the colour in the light overlay at x normal, y normal.


void DrawBumpMap()
{
	char *sp, *dp;
	int x, y, xdiff, ydiff;

	sp = t_page;			// Height map
	dp = d_buffer;			// Screen


//	Subtract light source center from x and y coordinates.

	for ( y = 0 - by; y < 200 - by; y++ )
	{
		for ( x = 0 - bx; x < 320 - bx; x++, sp++ )
		{

//			Get normals.

			xdiff = *( sp + 1 ) - *( sp - 1 );
			ydiff = *( sp + 320 ) - *( sp - 320 );

//			Add distance and base.
			xdiff = ( xdiff + x ) + 128;
			ydiff = ( ydiff + y ) + 128;

//			Keep in 0 - 255.
			if ( xdiff < 0 )
				xdiff = 0;

			if ( xdiff > 255 )
				xdiff = 255;

			if ( ydiff < 0 )
				ydiff = 0;

			if ( ydiff > 255 )
				ydiff = 255;

//			Set pixel to colour.

			*dp++ = BumpOverlay[ xdiff + ypremultable[ ydiff ] ];
		}
	}
}



Anyway, enough of the 2d stuff.... As I mentioned earlier 3d bump mapping is soooooooooo
simple. First of all take your textured gourard shader and add the following to the fill loop:

		for ( loop = 0; loop <= length; loop++ )
		{

//			Get current U, V coordinate
			index = ( v & 0xff00 ) + ( u >> 8 );

//			Perform the pseudo normal code - texture is 256 * 256.
			xdiff = t_page[ index + 1 ] - t_page[ index - 1 ];
			ydiff = t_page[ index + 256 ] - t_page[ index - 256 ] ;

//			Add two normals together.
			xdiff = xdiff + ydiff;

//		    Add light at this pixel.
			xdiff += col >> 8;


//			Limit range of value.
			if ( xdiff < 0 )
				xdiff = 0;

			if ( xdiff > 255 )
				xdiff = 255;

			*byte++ = xdiff;

			u += uadd;
			v += vadd;
			col += coladd;
		}

The code above is slightly different to the 2d version - I just plugged it with similiar but,
faster calculations. We can optimise the code above even more by precalculating the normals into
a huge table - leaving us with the following.


		for ( loop = 0; loop <= length; loop++ )
		{
			xdiff = precalc[ ( v & 0xff00 ) + ( u >> 8 ) ];

//		    Add light at this pixel.
			xdiff += col >> 8;

//			Limit range of value.
			if ( xdiff < 0 )
				xdiff = 0;

			if ( xdiff > 255 )
				xdiff = 255;

			*byte++ = xdiff;

			u += uadd;
			v += vadd;
			col += coladd;
		}

precalc contains the following for each pixel:-

precalc[ x + ( y * 256 ) ] = t_page[ x + ( y * 256 ) + 1 ] - t_page[ x + ( y * 256 ) - 1 ] +
							 t_page[ x + ( ( y + 1 ) * 256 ) ] - t_page[ x + ( ( y - 1 ) * 256 ) ];

It must be noted that if you are clever about the range of the height values you can get rid of
the xdiff > 255 check.









( 8 )	Environment mapping.
----------------------------
This is another simple effect if you code it the same way as demo coders. If you calculate a
pseudo normal for each vertex ( as in gourard shading ) and you set each normal to a common
unit length ( I use 4096 ) do something along the lines of the following to get the environment
map U,V coordinates:


Assume texture map is 256 * 256 and unit length is 4096 ( ONE ).

//		Get vertex numbers.
		v0 = poly_list[ 0 ];
		v1 = poly_list[ 1 ];
		v2 = poly_list[ 2 ];
		v3 = poly_list[ 3 ];

//		Get UV coordinates for each vertex in range of 0 - 255.
		uvinfo.u0 = ( point_normals[ v0 ].vx / 32 ) + 128;
		uvinfo.v0 = ( point_normals[ v0 ].vy / 32 ) + 128;

		uvinfo.u1 = ( point_normals[ v1 ].vx / 32 ) + 128;
		uvinfo.v1 = ( point_normals[ v1 ].vy / 32 ) + 128;

		uvinfo.u2 = ( point_normals[ v2 ].vx / 32 ) + 128;
		uvinfo.v2 = ( point_normals[ v2 ].vy / 32 ) + 128;

		uvinfo.u3 = ( point_normals[ v3 ].vx / 32 ) + 128;
		uvinfo.v3 = ( point_normals[ v3 ].vy / 32 ) + 128;

All the code is doing is taking the x and y component of the normal and using it to calculate
U, V coordinates in the range 0 - 255 ( my texture is 256 * 256 ). Simple.........









( 9 )	Phong shaded polygons.
------------------------------
Real phong shading involves calculating a normal and the amount of light for every pixel in the
polygon. You would be right in assuming that demo coders are only human and they must be somehow
cheating in the many phong shaded demos. It just so happens that if you have an environment
mapping routine like above you can easily phong shade ( fake!!! ). What demo coders do is to
precalculate a light overlay based upon the phong illumination model ( I haven't got the formula
to hand ). This light overlay is almost identical in style to the light overlay that I showed you
how to create in the bump mapping section. This overlay is simply used as a texture or a light
index and when combined with the environment mapping produces fast and fake yet realistic looking
phong shading.

*/
#endif

