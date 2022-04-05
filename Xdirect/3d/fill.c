/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <malloc.h>
#include <string.h>
#include <xlib.h>

#include "xsoft.h"

RCSID( "$Id: fill.c,v 1.1.1.1 2003/08/19 17:44:50 bernie Exp $" )


typedef struct {
	int XStart;
	int XEnd;
	FIXED DestZ, DestZStep;
} sHLine;


typedef struct {
	int	Length;
	int	YStart;
	sHLine	*HLinePtr;
} HLineList;


static int alloced_len = 0L;
static HLineList WorkingHLineList = { 0L, 0L, NULL };

//#define MYMALLOC


//
//
//
void DrawHorizontalLineListHigh( HLineList *HLineListPtr, int Color ) {

	sHLine		*HLinePtr;
	int		Length, Width, i;
	unsigned int	step_line;
	USHORT *video;

	if(HLineListPtr->YStart>hw_state.CLIPMAXY) return;
	if(HLineListPtr->YStart+HLineListPtr->Length<hw_state.CLIPMINY) return;

	HLinePtr = HLineListPtr->HLinePtr;
	Length = HLineListPtr->Length;

	if(HLineListPtr->YStart<hw_state.CLIPMINY)
		while((HLineListPtr->YStart)++ < hw_state.CLIPMINY) {
			--Length;
			++HLinePtr;
		}

	if(HLineListPtr->YStart+Length > hw_state.CLIPMAXY )
		Length -= HLineListPtr->YStart+Length - hw_state.CLIPMAXY;

	step_line = HLineListPtr->YStart;


	while( Length-- > 0 ) {

		if ((Width = HLinePtr->XEnd - HLinePtr->XStart + 1) > 0) {

			if( ((HLinePtr->XStart+Width)<hw_state.CLIPMINX) || (HLinePtr->XStart>hw_state.CLIPMAXX))
				goto NextLine;
			else {

				if(HLinePtr->XStart<hw_state.CLIPMINX) {
					Width -= ABS(hw_state.CLIPMINX-HLinePtr->XStart);
					HLinePtr->XStart=hw_state.CLIPMINX;
				}

				if(HLinePtr->XStart+Width>hw_state.CLIPMAXX)
					Width -= ( (HLinePtr->XStart+Width) - hw_state.CLIPMAXX);
			}

			//HLine( HLinePtr->XStart, Width, step_line, Color );

			video = SW_GetVideoPtr( HLinePtr->XStart, step_line );

			for( i=0; i<Width; i++ )
				*video++ = Color;

			// memset( video, Color, Width*hw_state.pixel_len);

		}

		NextLine:

		HLinePtr++;
		++step_line;

	}

	return;
}






//
//
//
void ScanEdge( int X1, int Y1, FLOAT Z1, int X2, int Y2, FLOAT Z2,
	       int SetXStart, int SkipFirst, sHLine **EdgePointPtr ) {


	int DeltaX, Height, Width, AdvanceAmt, ErrorTerm, i;
	int ErrorTermAdvance, XMajorAdvanceAmt;
	sHLine *WorkingEdgePointPtr;

	WorkingEdgePointPtr = *EdgePointPtr;
	AdvanceAmt = ((DeltaX = X2 - X1) > 0) ? 1 : -1;

	if( (Height = Y2 - Y1) <= 0 )
		return;

	if( (Width = ABS(DeltaX)) == 0 ) {

      		for( i=Height-SkipFirst; i-->0; WorkingEdgePointPtr++ ) {

	 		if( SetXStart == 1 )
	    			WorkingEdgePointPtr->XStart = X1;
	 		else
	    			WorkingEdgePointPtr->XEnd = X1;
      		}
   	}
	else
	if( Width == Height ) {

		if( SkipFirst )
	 		X1 += AdvanceAmt;

      		for( i=Height-SkipFirst; i-->0; WorkingEdgePointPtr++ ) {

	 		if( SetXStart == 1 )
	    			WorkingEdgePointPtr->XStart = X1;
	 		else
	    			WorkingEdgePointPtr->XEnd = X1;

			X1 += AdvanceAmt;
      		}
   	}
	else
	if( Height > Width ) {

      		if( DeltaX >= 0 )
	 		ErrorTerm = 0;
      		else
	 		ErrorTerm = -Height + 1;

      		if( SkipFirst ) {

	 		if( (ErrorTerm += Width) > 0 ) {
	    			X1 += AdvanceAmt;
	    			ErrorTerm -= Height;
	 		}
      		}

      		for( i=Height-SkipFirst; i-->0; WorkingEdgePointPtr++ ) {

	 		if( SetXStart == 1 )
	    			WorkingEdgePointPtr->XStart = X1;
	 		else
	    			WorkingEdgePointPtr->XEnd = X1;

	 		if( (ErrorTerm += Width) > 0 ) {
	    			X1 += AdvanceAmt;
	    			ErrorTerm -= Height;
	 		}
      		}
   	}
	else {

      		XMajorAdvanceAmt = (Width / Height) * AdvanceAmt;
      		ErrorTermAdvance = Width % Height;

      		if( DeltaX >= 0 )
	 		ErrorTerm = 0;
      		else
	 		ErrorTerm = -Height + 1;

      		if( SkipFirst ) {
	 		X1 += XMajorAdvanceAmt;
	 		if( (ErrorTerm += ErrorTermAdvance) > 0 ) {
	    			X1 += AdvanceAmt;
	    			ErrorTerm -= Height;
	 		}
      		}

      		for( i=Height-SkipFirst; i-->0; WorkingEdgePointPtr++ ) {

	 		if( SetXStart == 1 )
	    			WorkingEdgePointPtr->XStart = X1;
	 		else
	    			WorkingEdgePointPtr->XEnd = X1;

	 		X1 += XMajorAdvanceAmt;

	 		if( (ErrorTerm += ErrorTermAdvance) > 0 ) {
	    			X1 += AdvanceAmt;
	    			ErrorTerm -= Height;
	 		}
      		}
   	}

	*EdgePointPtr = WorkingEdgePointPtr;

	return;
}




#define INDEX_FORWARD(Index) Index = (Index + 1) % VertexList->npoints;
#define INDEX_BACKWARD(Index) Index = (Index - 1 + VertexList->npoints) % VertexList->npoints;


#define INDEX_MOVE(Index,Direction)				\
   if (Direction > 0)						\
      Index = (Index + 1) % VertexList->npoints;		\
   else 							\
      Index = (Index - 1 + VertexList->npoints) % VertexList->npoints;



//
//
//
void FillConvex( polygon_ptr VertexList, polygon3_t *tpoly, ULONG Color ) {


	int i, MinIndexL, MaxIndex, MinIndexR, SkipFirst, Temp;
	int MinPoint_Y, MaxPoint_Y, TopIsFlat, LeftEdgeDir;
	int NextIndex, CurrentIndex, PreviousIndex;
	int DeltaXN, DeltaYN, DeltaXP, DeltaYP;
	sHLine	*EdgePointPtr;
	point_t *VertexPtr;


	VertexPtr = VertexList->point;


	if( VertexList->npoints == 0 )
		return;


	MaxPoint_Y = MinPoint_Y = VertexPtr[MinIndexL = MaxIndex = 0][1];


	for( i=1; i<VertexList->npoints; i++ ) {
		if( VertexPtr[i][1] < MinPoint_Y )
			MinPoint_Y = VertexPtr[MinIndexL = i][1];
		else
			if( VertexPtr[i][1] > MaxPoint_Y )
				MaxPoint_Y = VertexPtr[MaxIndex = i][1];
	}


	if( MinPoint_Y == MaxPoint_Y )
		return;

	MinIndexR = MinIndexL;

	while( VertexPtr[MinIndexR][1] == MinPoint_Y )
		INDEX_FORWARD(MinIndexR);

	INDEX_BACKWARD(MinIndexR);

	while( VertexPtr[MinIndexL][1] == MinPoint_Y )
		INDEX_BACKWARD(MinIndexL);

	INDEX_FORWARD(MinIndexL);

	LeftEdgeDir = -1;
	if( (TopIsFlat = (VertexPtr[MinIndexL][0] != VertexPtr[MinIndexR][0]) ? 1 : 0) == 1 ) {

		if( VertexPtr[MinIndexL][0] > VertexPtr[MinIndexR][0] ) {
			LeftEdgeDir = 1;
			Temp = MinIndexL;
			MinIndexL = MinIndexR;
			MinIndexR = Temp;
		}
	}
	else {
		NextIndex = MinIndexR;
		INDEX_FORWARD(NextIndex);
		PreviousIndex = MinIndexL;
		INDEX_BACKWARD(PreviousIndex);

		DeltaXN = VertexPtr[NextIndex][0] - VertexPtr[MinIndexL][0];
		DeltaYN = VertexPtr[NextIndex][1] - VertexPtr[MinIndexL][1];
		DeltaXP = VertexPtr[PreviousIndex][0] - VertexPtr[MinIndexL][0];
		DeltaYP = VertexPtr[PreviousIndex][1] - VertexPtr[MinIndexL][1];

		if( ((long)DeltaXN * DeltaYP - (long)DeltaYN * DeltaXP) < 0L ) {
			LeftEdgeDir = 1;
			Temp = MinIndexL;
			MinIndexL = MinIndexR;
			MinIndexR = Temp;
		}
	}

	if( (WorkingHLineList.Length = MaxPoint_Y - MinPoint_Y - 1 + TopIsFlat) <= 0 )
		return;

	WorkingHLineList.YStart = MinPoint_Y + 1 - TopIsFlat;

#ifndef MYMALLOC
	if( (WorkingHLineList.HLinePtr = (sHLine *) (malloc(sizeof(sHLine) * WorkingHLineList.Length))) == NULL)
		dllQuit("SW_FillConvex: out of memory.");
#else
	if( WorkingHLineList.Length > alloced_len ) {
		if( alloced_len == 0 ) WorkingHLineList.HLinePtr = NULL;
		REALLOCMEM( WorkingHLineList.HLinePtr, sizeof(sHLine) * WorkingHLineList.Length );
		alloced_len = WorkingHLineList.Length;
	}
#endif
	// if( WorkingHLineList.Length	> SCREENH ) xprintf("len: %d\n",WorkingHLineList.Length);


   	EdgePointPtr = WorkingHLineList.HLinePtr;

   	PreviousIndex = CurrentIndex = MinIndexL;

   	SkipFirst = TopIsFlat ? 0 : 1;

   	do {
      		INDEX_MOVE(CurrentIndex,LeftEdgeDir);
      		ScanEdge( VertexPtr[PreviousIndex][0],
	    		  VertexPtr[PreviousIndex][1],
			  tpoly.point[PreviousIndex][2],
	    		  VertexPtr[CurrentIndex][0],
	    		  VertexPtr[CurrentIndex][1],
			  tpoly.point[CurrentIndex][2], 1, SkipFirst, &EdgePointPtr );
      		PreviousIndex = CurrentIndex;
      		SkipFirst = 0;

	} while( CurrentIndex != MaxIndex );

	EdgePointPtr = WorkingHLineList.HLinePtr;
	PreviousIndex = CurrentIndex = MinIndexR;
	SkipFirst = TopIsFlat ? 0 : 1;

	do {
		INDEX_MOVE(CurrentIndex,-LeftEdgeDir);
		ScanEdge( VertexPtr[PreviousIndex][0] - 1,
			  VertexPtr[PreviousIndex][1],
			  tpoly.point[PreviousIndex][2],
			  VertexPtr[CurrentIndex][0] - 1,
			  VertexPtr[CurrentIndex][1],
			  tpoly.point[CurrentIndex][2], 0, SkipFirst, &EdgePointPtr );
		PreviousIndex = CurrentIndex;
		SkipFirst = 0;

	} while( CurrentIndex != MaxIndex );

	if( hw_state.bpp > 8 )
		DrawHorizontalLineListHigh( &WorkingHLineList, RGB24(Color) );

#ifndef MYMALLOC
	free(WorkingHLineList.HLinePtr);
#endif

	return;
}



