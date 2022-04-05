/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#include <xlib.h>

#include "xsoft.h"

RCSID( "$Id: texmap8.cpp,v 1.1.1.1 2003/08/19 17:44:51 bernie Exp $" )




static int StepEdge( edgescan_ptr );
static int SetUpEdge( edgescan_ptr, int );
static void ScanOutLine(edgescan_ptr, edgescan_ptr);




static int MaxVert,NumVerts,DestY;

static point_ptr VertexPtr;
static point_ptr TexVertsPtr;

static UCHAR *TexMap;
static int TexMapWidth,TexMapHeight;




//
//
//
void SW_DrawTexturedFace( polygon_ptr poly, point_ptr texvert, texmap_t *texmap ) {

	int		MinY,MaxY,MinVert = 0,i;
	edgescan_t	LeftEdge,RightEdge;

	NumVerts	= poly->npoints;
	VertexPtr	= poly->point;
	TexVertsPtr	= texvert;

	TexMap		= &(texmap->sprite1[SPRITEHEADER]);
	TexMapWidth	= texmap->width1;
	TexMapHeight	= texmap->height1;


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

		if( DestY >= hw_state.CLIPMINY )
			ScanOutLine(&LeftEdge, &RightEdge);

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
static int StepEdge(edgescan_ptr Edge) {

	if( --Edge->RemainingScans == 0 ) {
		if( SetUpEdge(Edge, Edge->CurrentEnd) == 0 )
			return 0;
		return 2;
	}

	Edge->SourceX += Edge->SourceStepX;
	Edge->SourceY += Edge->SourceStepY;

	Edge->DestX += Edge->DestXStep;

	return 1;
}





//
//
//
static int SetUpEdge( edgescan_ptr Edge, int StartVert ) {

	int	NextVert, DestXWidth;
	FIXED	DestYHeight;

	for( ; ; ) {

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

			#define TX(i) ( INT_TO_FIXED(TexVertsPtr[i][0]) )
			#define TY(i) ( INT_TO_FIXED(TexVertsPtr[i][1]) )

			// texture mapping

			Edge->CurrentEnd	= NextVert;

			Edge->SourceX		= TX(StartVert);
			Edge->SourceY		= TY(StartVert);

			DestYHeight		= fdiv( FIXED_ONE, INT_TO_FIXED(Edge->RemainingScans) );
			Edge->SourceStepX	= fmul( TX(NextVert) - TX(StartVert) , DestYHeight );
			Edge->SourceStepY	= fmul( TY(NextVert) - TY(StartVert) , DestYHeight );

			Edge->DestX = INT_TO_FIXED(VertexPtr[StartVert][0]);

			if( (DestXWidth = (VertexPtr[NextVert][0] - VertexPtr[StartVert][0])) < 0 )
				Edge->DestXStep = -fdiv( INT_TO_FIXED(-DestXWidth), INT_TO_FIXED(Edge->RemainingScans) );
			else
				Edge->DestXStep =  fdiv( INT_TO_FIXED( DestXWidth), INT_TO_FIXED(Edge->RemainingScans) );


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

	FIXED	SourceX 	= LeftEdge->SourceX;
	FIXED	SourceY 	= LeftEdge->SourceY;
	int	DestX		= FIXED_TO_INT(LeftEdge->DestX);
	int	DestXMax	= FIXED_TO_INT(RightEdge->DestX);
	FIXED	DestWidth;
	FIXED	SourceXStep,SourceYStep;
	UCHAR  *video;

	if( (DestXMax < hw_state.CLIPMINX) || (DestX > hw_state.CLIPMAXX) )
		return;

	if((DestXMax - DestX) <= 0)
		return;

	DestWidth = fdiv( FIXED_ONE, INT_TO_FIXED(DestXMax - DestX) );

	SourceXStep = fmul( (RightEdge->SourceX - SourceX) , DestWidth );
	SourceYStep = fmul( (RightEdge->SourceY - SourceY) , DestWidth );

	if( DestXMax > hw_state.CLIPMAXX + 1 )
		DestXMax = hw_state.CLIPMAXX + 1;


	if( DestX < hw_state.CLIPMINX ) {
		SourceX += fmul( SourceXStep , INT_TO_FIXED(hw_state.CLIPMINX - DestX) );
		SourceY += fmul( SourceYStep , INT_TO_FIXED(hw_state.CLIPMINX - DestX) );
		DestX = hw_state.CLIPMINX;
	}

	video = (UCHAR*)SW_GetVideoPtr( DestX, DestY );

	for( ; DestX<DestXMax; DestX++ ) {

		#define SAMPLE TexMap[ (TexMapWidth*(SourceY>>FP_SHIFT)) + (SourceX>>FP_SHIFT) ]
		#define COLOR SAMPLE

		//PutPixelNC( DestX, DestY, COLOR );
		*video++ = COLOR;

		SourceX += SourceXStep;
		SourceY += SourceYStep;
	}

	return;
}




