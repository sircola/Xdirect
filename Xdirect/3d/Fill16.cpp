/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#include <xlib.h>
#include <xinner.h>

#include "xsoft.h"



RCSID( "$Id: fill.c,v 1.0 97-03-10 16:56:21 bernie Exp $" )



static int StepEdge( edgescan_ptr );
static int SetUpEdge( edgescan_ptr, int );
static void ScanOutLine(edgescan_ptr, edgescan_ptr);
static void ScanOutLineNoZ(edgescan_ptr, edgescan_ptr);




static int MaxVert,NumVerts,DestY;

static point_ptr VertexPtr;
static point3_ptr TPolyPtr;

static USHORT fillColor;
static BOOL isFarthest;



//
//
//
void SW_FillFaceHigh( polygon_ptr poly, USHORT color, polygon3_ptr tpoly, int face_flag, rgb_t *rgb ) {

	int		MinY,MaxY,MinVert = 0,i;
	edgescan_t	LeftEdge,RightEdge;

	NumVerts	= poly->npoints;
	VertexPtr	= poly->point;
	TPolyPtr	= tpoly->point;

	fillColor = color; // ((color >> 8) & 0xff) | ((color & 0xff) << 8);
	isFarthest	= ISFLAG( face_flag, FF_FARTHEST );

	if( NumVerts < 3 )
		return;

	MinY = 32767;
	MaxY = -32768;


	for( i=0; i<NumVerts; i++ ) {

		if( VertexPtr[i][1] < MinY ) {
			MinY = VertexPtr[i][1];
			MinVert = i;
		}

		if( VertexPtr[i][1] > MaxY ) {
			MaxY = VertexPtr[i][1];
			MaxVert = i;
		}
	}


	if( MinY >= MaxY )
		return;

	DestY = MinY;

	LeftEdge.Direction = 1;
	SetUpEdge(&LeftEdge, MinVert);

	RightEdge.Direction = -1;
	SetUpEdge(&RightEdge, MinVert);

	for( ; ; ) {

		if( DestY > hw_state.CLIPMAXY )
			break;

		if( DestY >= hw_state.CLIPMINY ) {

			if( !ISFLAG( face_flag, FF_DBLSIDE ) ) {
				if( hw_state.bZbuffer == TRUE ) ScanOutLine( &LeftEdge, &RightEdge );
			}
			else {
				if( LeftEdge.DestX > RightEdge.DestX ) {
					if( hw_state.bZbuffer == TRUE ) ScanOutLine( &RightEdge, &LeftEdge );
				}
				else {
					if( hw_state.bZbuffer == TRUE ) ScanOutLine( &LeftEdge, &RightEdge );
				}
			}
		}
		if( !StepEdge( &LeftEdge ) )
			break;

		if( !StepEdge( &RightEdge ) )
			break;

		DestY++;
	}

	return;
}




//
//
//
static int StepEdge( edgescan_ptr Edge ) {

	if( --Edge->RemainingScans == 0 ) {
		if( SetUpEdge(Edge, Edge->CurrentEnd) == 0 )
			return 0;
		return 2;
	}

	Edge->DestX += Edge->DestXStep;

	Edge->DestZ += Edge->DestZStep;

	return 1;
}





//
//
//
static int SetUpEdge( edgescan_ptr Edge, int StartVert ) {

	int	NextVert, DestXWidth;
	FLOAT	DestYHeight;

	for( ;; ) {

		if (StartVert == MaxVert)
			return 0;


		NextVert = StartVert + Edge->Direction;

		if( NextVert >= NumVerts ) {
			NextVert = 0;
		}
		else
			if( NextVert < 0 )
				NextVert = NumVerts - 1;


		if( (Edge->RemainingScans = VertexPtr[NextVert][1] - VertexPtr[StartVert][1]) != 0 ) {

			Edge->CurrentEnd	= NextVert;

			DestYHeight	= ( FLOAT_ONE / FLOAT(Edge->RemainingScans) );

			Edge->DestX = FLOAT(VertexPtr[StartVert][0]);

			if( (DestXWidth = (VertexPtr[NextVert][0] - VertexPtr[StartVert][0])) < 0 )
				Edge->DestXStep = -( FLOAT(-DestXWidth) / FLOAT(Edge->RemainingScans) );
			else
				Edge->DestXStep =  ( FLOAT( DestXWidth) / FLOAT(Edge->RemainingScans) );


			Edge->DestZ = ( TPolyPtr[StartVert][2] );
					// FLOAT_TO_FIXED( DistanceVector( TPolyPtr[StartVert], origo3 ) );
			Edge->DestZStep = (  (FLOAT( TPolyPtr[NextVert][2] ) - Edge->DestZ ) * DestYHeight );
					// fmul(  FLOAT_TO_FIXED( DistanceVector( TPolyPtr[NextVert], origo3 ) ) - Edge->DestZ , DestYHeight );


			return 1;	// success
		}

		StartVert = NextVert;
	}
	return 0;
}







//
//
//
static void ScanOutLine( edgescan_ptr LeftEdge, edgescan_ptr RightEdge ) {

	int	DestX		= ftoi(LeftEdge->DestX);
	int	DestXMax	= ftoi(RightEdge->DestX);
	FLOAT	DestWidth;
	FLOAT	DestZ		= LeftEdge->DestZ;
	FLOAT	DestZStep;
	USHORT	*video		= NULL;
	FLOAT	*zbuffer	= NULL;

	if( (DestXMax < hw_state.CLIPMINX) || (DestX > hw_state.CLIPMAXX) )
		return;

	if((DestXMax - DestX) <= 0)
		return;

	DestWidth = FLOAT_ONE / FLOAT(DestXMax - DestX);

	DestZStep = ( RightEdge->DestZ - DestZ) * DestWidth;

	if( DestXMax > hw_state.CLIPMAXX + 1 )
		DestXMax = hw_state.CLIPMAXX + 1;


	if( DestX < hw_state.CLIPMINX ) {
		DestZ += ( DestZStep * FLOAT(hw_state.CLIPMINX - DestX) );
		DestX = hw_state.CLIPMINX;
	}

	video = (USHORT*)SW_GetVideoPtr( DestX, DestY );
	zbuffer = (FLOAT*)&hw_state.zbuffer[ DestY * hw_state.SCREENW + DestX ];

	for( ; DestX<DestXMax; DestX++ ) {

		#if 0
		if( DestZ >= ffogfar ) {
			DestZ	+= DestZStep;
			++video;
			++zbuffer;
			continue;
		}
		#endif

		// zbuffer
		if( !isFarthest && ((DestZ > *zbuffer) || (DestZ > fzclipfar)) ) {
			DestZ += DestZStep;
			++video;
			++zbuffer;
			continue;
		}

		*video = fillColor;

		++video;

		if( !isFarthest ) *zbuffer++ = DestZ;
		else zbuffer++;

		DestZ += DestZStep;
	}

	return;
}



