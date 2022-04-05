/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#include <xlib.h>
#include <xinner.h>

#include "xsoft.h"



RCSID( "$Id: Texmap16.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )




static int StepEdge( edgescan_ptr );
static int SetUpEdge( edgescan_ptr, int );
static void ScanOutLine(edgescan_ptr, edgescan_ptr);
static void ScanOutLineNoZ(edgescan_ptr, edgescan_ptr);




static int MaxVert,NumVerts,DestY;

static point_ptr VertexPtr;
static point_ptr TexVertsPtr;
static point3_ptr TPolyPtr;
static rgb_t *PolyRgb;

static USHORT *TexMap16;
static int TexMapWidth,TexMapHeight;

static BOOL isChromaKey;
static ULONG chromaColor;
static BOOL isAlpha;
static BOOL isFarthest = FALSE;


//
//
//
void SW_DrawTexturedFaceHigh( polygon_ptr poly, point_ptr texvert,
			      texmap_t *texmap, polygon3_ptr tpoly,
			      int face_flag, rgb_t *rgb ) {

	int MinY,MaxY,MinVert = 0,i;
	edgescan_t	LeftEdge,RightEdge;

	NumVerts	= poly->npoints;
	VertexPtr	= poly->point;
	TexVertsPtr	= texvert;
	TPolyPtr	= tpoly->point;
	PolyRgb		= rgb;

	TexMap16	= (USHORT*)&(texmap->sprite1[SPRITEHEADER]);
	TexMapWidth	= texmap->width1;
	TexMapHeight	= texmap->height1;
	isChromaKey	= texmap->isChromaKey;
	chromaColor	= texmap->chromaColor;
	isAlpha 	= texmap->isAlpha;
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

	for( ;; ) {

		if( DestY > hw_state.CLIPMAXY )
			break;

		if( DestY >= hw_state.CLIPMINY ) {

			if( !ISFLAG( face_flag, FF_DBLSIDE ) ) {
				ScanOutLine( &LeftEdge, &RightEdge );
			}
			else {
				if( LeftEdge.DestX > RightEdge.DestX )
					ScanOutLine( &RightEdge, &LeftEdge );
				else
					ScanOutLine( &LeftEdge, &RightEdge );
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

	Edge->SourceX += Edge->SourceStepX;
	Edge->SourceY += Edge->SourceStepY;

	Edge->DestX += Edge->DestXStep;

	Edge->DestZ += Edge->DestZStep;

	Edge->ooz += Edge->oozstep;

	Edge->red += Edge->redstep;
	Edge->green += Edge->greenstep;
	Edge->blue += Edge->bluestep;
	Edge->alpha += Edge->alphastep;

	return 1;
}





//
//
//
static int SetUpEdge( edgescan_ptr Edge, int StartVert ) {

	int NextVert, DestXWidth;
	FLOAT ooremain,destooz;
	FLOAT z0,z1;

	for( ;; ) {

		if( StartVert == MaxVert )
			return 0;

		NextVert = StartVert + Edge->Direction;

		if( NextVert >= NumVerts ) {
			NextVert = 0;
		}
		else
			if( NextVert < 0 )
				NextVert = NumVerts - 1;

		if( (Edge->RemainingScans = VertexPtr[NextVert][1] - VertexPtr[StartVert][1]) != 0 ) {

			ooremain = FLOAT_ONE / (FLOAT)Edge->RemainingScans;

			z0 = TPolyPtr[StartVert][2];
			z1 = TPolyPtr[NextVert][2];

			if( FABS(z0) < FLOAT_EPSILON ) z0 = FLOAT_EPSILON;
			if( FABS(z1) < FLOAT_EPSILON ) z1 = FLOAT_EPSILON;

			Edge->ooz = FLOAT_ONE / z0;
			destooz = FLOAT_ONE / z1;
			Edge->oozstep = ( destooz - Edge->ooz) * ooremain;

			#define TX(i) ( (FLOAT)(TexVertsPtr[i][0]) )
			#define TY(i) ( (FLOAT)(TexVertsPtr[i][1]) )

			// texture mapping

			Edge->CurrentEnd	= NextVert;

			Edge->SourceX		= TX(StartVert) * Edge->ooz;
			Edge->SourceY		= TY(StartVert) * Edge->ooz;

			Edge->SourceStepX	= (TX(NextVert) * destooz - Edge->SourceX) * ooremain;
			Edge->SourceStepY	= (TY(NextVert) * destooz - Edge->SourceY) * ooremain;

			Edge->DestX = (FLOAT)VertexPtr[StartVert][0];

			if( (DestXWidth = (VertexPtr[NextVert][0] - VertexPtr[StartVert][0])) < 0 )
				Edge->DestXStep = -( (FLOAT)(-DestXWidth) * ooremain );
			else
				Edge->DestXStep =  (FLOAT)DestXWidth * ooremain;

			Edge->DestZ = z0;
			Edge->DestZStep = (z1 - Edge->DestZ) * ooremain;

			Edge->red = (FLOAT)PolyRgb[StartVert].r;
			Edge->redstep = (FLOAT)(PolyRgb[NextVert].r - PolyRgb[StartVert].r) * ooremain;
			Edge->green = (FLOAT)PolyRgb[StartVert].g;
			Edge->greenstep = (FLOAT)(PolyRgb[NextVert].g - PolyRgb[StartVert].g) * ooremain;
			Edge->blue = (FLOAT)PolyRgb[StartVert].b;
			Edge->bluestep = (FLOAT)(PolyRgb[NextVert].b - PolyRgb[StartVert].b) * ooremain;

			Edge->alpha = (FLOAT)PolyRgb[StartVert].a;
			Edge->alphastep = (FLOAT)(PolyRgb[NextVert].a - PolyRgb[StartVert].a) * ooremain;

			return 1;	// success
		}

		StartVert = NextVert;
	}

	return 0;
}




/***
Assume factor has the percentage of the sprite you want, and oneminusfactor has
the percentage of the background you want.  Both are scaled by 256 (so 90%
would be 230; for more precision, use 16 bits instead).  Note that I ran
out of registers, so I refetch the pixel each time.  Once it's cached, this
is a low-cost operation.

	xor	eax, eax
	xor	edx, edx
	mov	bx, 0

loop:
	mov	ax, [esi]	; extract red
	mov	edx, ebx
	and	ax, 07c00h
	and	dx, 07c00h
	mul	eax, [factor]
	mul	edx, [oneminusfactor]
	add	eax, edx	; eax has new red x scale factor
	mov	ecx, eax	; accumulate in ecx

	mov	ax, [esi]	; extract green
	mov	edx, ebx
	and	ax, 03e0h
	and	dx, 03e0h
	mul	eax, [factor]
	mul	edx, [oneminusfactor]
	add	eax, edx	; eax has new green x scale factor
	or	ecx, eax

	mov	ax, [esi]	; extract blue
	mov	edx, ebx
	and	ax, 01fh
	and	dx, 01fh
	mul	eax, [factor]
	mul	edx, [oneminusfactor]
	add	eax, edx	; eax has new blue x scale factor
	or	ecx, eax

	shr	ecx, 8		; (8 is scale factor)
	mov	[edi], cx

***/



//
//
//
static void BlendColor( USHORT *video, USHORT color, int dest_alpha, int src_alpha, int red, int green, int blue ) {

	int r,g,b;
	int trshift = 8;
	int tgshift = 8;
	int tbshift = 8;

	// CLAMPMINMAX( src_alpha, 0, 255 );
	if( src_alpha == 0 )
		return;

	// CLAMPMINMAX( dest_alpha, 0, 255 );

	r = INTRED16(color) * src_alpha + INTRED16(*video) * dest_alpha;
	g = INTGREEN16(color) * src_alpha + INTGREEN16(*video) * dest_alpha;
	b = INTBLUE16(color) * src_alpha + INTBLUE16(*video) * dest_alpha;

	if( red < 255 ) {
		trshift = 16;
		r *= red;
	}

	if( green < 255 ) {
		tgshift = 16;
		g *= green;
	}

	if( blue < 255 ) {
		tbshift = 16;
		b *= blue;
	}

	r >>= trshift;
	g >>= tgshift;
	b >>= tbshift;

	CLAMPMAX( r, drmask );
	CLAMPMAX( g, dgmask );
	CLAMPMAX( b, dbmask );

	*video = RGBINT( r, g, b );

	// *video += color;

	return;
}





//
//
//
static void ScanOutLine( edgescan_ptr LeftEdge, edgescan_ptr RightEdge ) {

	FLOAT	SourceX = LeftEdge->SourceX;
	FLOAT	SourceY = LeftEdge->SourceY;
	int	DestX	= ftoi(LeftEdge->DestX);
	int	DestXMax = ftoi(RightEdge->DestX);
	FLOAT	DestWidth;
	FLOAT	SourceXStep,SourceYStep;
	FLOAT	DestZ	= LeftEdge->DestZ;
	FLOAT	DestZStep;
	FLOAT	DestOOZ	= LeftEdge->ooz;
	FLOAT	DestOOZStep;
	FLOAT	DestR = LeftEdge->red;
	FLOAT	DestRStep;
	FLOAT	DestG = LeftEdge->green;
	FLOAT	DestGStep;
	FLOAT	DestB = LeftEdge->blue;
	FLOAT	DestBStep;
	FLOAT	DestA = LeftEdge->alpha;
	FLOAT	DestAStep;
	USHORT	*video,color;
	FLOAT	*zbuffer;
	int 	x,y;
	int	per;
	FLOAT	tx,ty,dx,dy;
	FLOAT	fog = 0.0f,alpha;

	if( (DestXMax < hw_state.CLIPMINX) || (DestX > hw_state.CLIPMAXX) )
		return;

	if((DestXMax - DestX) <= 0)
		return;

	DestWidth = FLOAT_ONE / (FLOAT)(DestXMax - DestX);

	SourceXStep = (RightEdge->SourceX - SourceX) * DestWidth;
	SourceYStep = (RightEdge->SourceY - SourceY) * DestWidth;

	DestZStep = (RightEdge->DestZ - DestZ) * DestWidth;

	DestOOZStep = (RightEdge->ooz - DestOOZ) * DestWidth;

	DestRStep = (RightEdge->red - DestR) * DestWidth;
	DestGStep = (RightEdge->green - DestG) * DestWidth;
	DestBStep = (RightEdge->blue - DestB) * DestWidth;
	DestAStep = (RightEdge->alpha - DestA) * DestWidth;

	if( DestXMax > hw_state.CLIPMAXX + 1 )
		DestXMax = hw_state.CLIPMAXX + 1;

	if( DestX < hw_state.CLIPMINX ) {
		SourceX += ( SourceXStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		SourceY += ( SourceYStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestZ += ( DestZStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestOOZ += ( DestOOZStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestR += ( DestRStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestG += ( DestGStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestB += ( DestBStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestA += ( DestAStep * (FLOAT)(hw_state.CLIPMINX - DestX) );
		DestX = hw_state.CLIPMINX;
	}

	if( (video = (USHORT *)SW_GetVideoPtr( DestX, DestY )) == NULL ) return;
	zbuffer = (FLOAT *)&hw_state.zbuffer[ DestY * hw_state.SCREENW + DestX ];

	per = 0;
	tx = (SourceX / DestOOZ);
	ty = (SourceY / DestOOZ);

	for( ; DestX<DestXMax; DestX++ ) {

		if( !(per & (sf_perdiv-1)) ) {

			dx = ((SourceX + (FLOAT)sf_perdiv * SourceXStep) / (DestOOZ + (FLOAT)sf_perdiv * DestOOZStep) - (SourceX / DestOOZ)) / (FLOAT)sf_perdiv;
			dy = ((SourceY + (FLOAT)sf_perdiv * SourceYStep) / (DestOOZ + (FLOAT)sf_perdiv * DestOOZStep) - (SourceY / DestOOZ)) / (FLOAT)sf_perdiv;
		}

		// zbuffer
		if( ((DestZ > *zbuffer) || (DestZ > fzclipfar)) && !isFarthest ) {
			SourceX += SourceXStep;
			SourceY += SourceYStep;
			DestZ	+= DestZStep;
			DestOOZ	+= DestOOZStep;
			DestR	+= DestRStep;
			DestG	+= DestGStep;
			DestB	+= DestBStep;
			DestA	+= DestAStep;
			++video;
			++zbuffer;
			++per;
			tx += dx;
			ty += dy;
			continue;
		}

		// texture warping

		x = ftoi(tx);
		y = ftoi(ty);

		CLAMPMINMAX( x, 0, TexMapWidth-1 );
		CLAMPMINMAX( y, 0, TexMapHeight-1 );

		// while( FIXED_TO_INT(SourceX) >= TexMapWidth ) SourceX -= INT_TO_FIXED(TexMapWidth);
		// while( FIXED_TO_INT(SourceY) >= TexMapHeight ) SourceY -= INT_TO_FIXED(TexMapHeight);
		// while( FIXED_TO_INT(SourceX) < 0 ) SourceX += INT_TO_FIXED(TexMapWidth);
		// while( FIXED_TO_INT(SourceY) < 0 ) SourceY += INT_TO_FIXED(TexMapHeight);

		// #define SAMPLE TexMap16[ (TexMapWidth*(SourceY>>FP_SHIFT)) + (SourceX>>FP_SHIFT) ]
		#define SAMPLE TexMap16[ (TexMapWidth*y) + x ]
		#define COLOR SAMPLE

		color = COLOR;

		// ha farthest, ami PutSprite akkor nincs fog
		if( isFarthest == FALSE ) {

			if( (DestZ > dfognear) && (DestZ < dfogfar) )
				fog = (DestZ - dfognear) / (dfogfar - dfognear);
			else
			if( DestZ > dfogfar )
				fog = 1.0f;
			else
				fog = 0.0f;

			alpha = DestA - DestA * fog;
			CLAMPMINMAX( alpha, 0.0f,255.0f );
		}
		else {
			alpha = DestA;
			fog = 0;
		}

		// PutPixelNC( DestX, DestY, color );
		if( !(isChromaKey && (chromaColor == color)) ) {
			if( isAlpha == FALSE ) {

				if( (alpha < 255.0f) || (DestR < 255.0f) || (DestG < 255.0f) || (DestB < 255.0f) ) {

					int src_alpha = ftoi( alpha );
					int dest_alpha = 255 - src_alpha;

					BlendColor( video, color, dest_alpha, src_alpha, ftoi(DestR), ftoi(DestG), ftoi(DestB) );
				}
				else
					*video = color;

				if( !isFarthest )
					*zbuffer = DestZ;
			}
			else {
				int dest_alpha = 255, src_alpha = 127;

				BlendColor( video, color, dest_alpha-(dest_alpha*fog), src_alpha-(src_alpha*fog), ftoi(DestR), ftoi(DestG), ftoi(DestB) );
			}
		}

		++video;
		++zbuffer;

		SourceX += SourceXStep;
		SourceY += SourceYStep;
		DestZ	+= DestZStep;
		DestOOZ	+= DestOOZStep;
		DestR	+= DestRStep;
		DestG	+= DestGStep;
		DestB	+= DestBStep;
		DestA	+= DestAStep;

		++per;
		tx += dx;
		ty += dy;
	}

	return;
}



