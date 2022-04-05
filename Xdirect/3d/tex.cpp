/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#include <xlib.h>

RCSID( "$Id: tex.cpp,v 1.0 97-03-10 17:38:35 bernie Exp $" )

#if 0

typedef struct edgescan_s {

	int	Direction;
	int	RemainingScans;
	int	CurrentEnd;

	FIXED	DestX;
	FIXED	DestXStep;

	FIXED	SourceX;
	FIXED	SourceY;
	FIXED	SourceStepX;
	FIXED	SourceStepY;

} edgescan_t,*edgescan_ptr;




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
void DrawTexturedPoly( polygon_ptr poly, point_ptr texvert, UCHAR *texmap ) {

	int		MinY,MaxY,MinVert = 0,i;
	edgescan_t	LeftEdge,RightEdge;

	NumVerts	= poly->npoints;
	VertexPtr	= poly->point;
	TexVertsPtr	= texvert;
	TexMap		= &texmap[SPRITEHEADER];
	TexMapWidth	= SPRITEW(texmap);
	TexMapHeight	= SPRITEH(texmap);

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

	LockSurface( LS_WRITE );

	for( ; ; ) {

		if( DestY > CLIPMAXY )
			break;

		if( DestY >= CLIPMINY )
			ScanOutLine(&LeftEdge, &RightEdge);

		if( !StepEdge( &LeftEdge ) )
			break;

		if( !StepEdge( &RightEdge ) )
			break;

		DestY++;
	}

	UnlockSurface();

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

		if( NextVert >= NumVerts )
			NextVert = 0;
		else
		if( NextVert < 0 )
			NextVert = NumVerts - 1;


		if( (Edge->RemainingScans = VertexPtr[NextVert][1] - VertexPtr[StartVert][1]) != 0 ) {

			#define TX(i) ( INT_TO_FIXED(TexVertsPtr[i][0]) )
			#define TY(i) ( INT_TO_FIXED(TexVertsPtr[i][1]) )

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
static void ScanOutLine(edgescan_ptr LeftEdge, edgescan_ptr RightEdge) {

	FIXED	SourceX 	= LeftEdge->SourceX;
	FIXED	SourceY 	= LeftEdge->SourceY;
	int	DestX		= FIXED_TO_INT(LeftEdge->DestX);
	int	DestXMax	= FIXED_TO_INT(RightEdge->DestX);
	FIXED	DestWidth;
	FIXED	SourceXStep,SourceYStep;
	UCHAR  *video;

	if( (DestXMax < CLIPMINX) || (DestX > CLIPMAXX) )
		return;

	if((DestXMax - DestX) <= 0)
		return;

	DestWidth = fdiv( FIXED_ONE, INT_TO_FIXED(DestXMax - DestX) );

	SourceXStep = fmul( (RightEdge->SourceX - SourceX) , DestWidth );
	SourceYStep = fmul( (RightEdge->SourceY - SourceY) , DestWidth );


	if( DestXMax > CLIPMAXX + 1 )
		DestXMax = CLIPMAXX + 1;


	if( DestX < CLIPMINX ) {
		SourceX += fmul( SourceXStep , INT_TO_FIXED(CLIPMINX - DestX) );
		SourceY += fmul( SourceYStep , INT_TO_FIXED(CLIPMINX - DestX) );
		DestX = CLIPMINX;
	}

	video = (UCHAR*)GetVideoPtr( DestX, DestY );

	for( ; DestX<DestXMax; DestX++ ) {

		#define COLOR TexMap[ TexMapWidth*FIXED_TO_INT(SourceY) + FIXED_TO_INT(SourceX) ]

		//PutPixelNC( DestX, DestY, COLOR );
		*video++ = COLOR;

		SourceX += SourceXStep;
		SourceY += SourceYStep;
	}

	return;
}


#else


/////////////////////////////////////////////////////////////
//
//
// old texmap.c: régi textúrázások, mind a kettő jó
//
//
////////////////////////////////////////////////////////////



typedef struct edgescan_s {

	int	Direction;
	int	RemainingScans;
	int	CurrentEnd;

	FIXED	DestX;
	FIXED	DestXStep;

	FIXED	SourceX;
	FIXED	SourceY;
	FIXED	SourceStepX;
	FIXED	SourceStepY;

	FIXED	NormalX;
	FIXED	NormalY;
	FIXED	NormalStepX;
	FIXED	NormalStepY;

	FIXED	Light;
	FIXED	LightStep;

} edgescan_t,*edgescan_ptr;




static int StepEdge( edgescan_ptr );
static int SetUpEdge( edgescan_ptr, int );
static void ScanOutLine(edgescan_ptr, edgescan_ptr);




static int MaxVert,NumVerts,DestY;
static point_ptr VertexPtr[MAXVERTNUM];
static point_ptr TexVertsPtr;
static UCHAR *TexMap,*BumpMap;
static int TexMapWidth,TexMapHeight;
static int TexMapId;
static int wmask,hmask;
static FIXED *intensity;		       // light 0 - 16 minden pontnak
static face_ptr F;
static object_ptr O;

#define VP(i,j) ((*VertexPtr[i])[j])

//#define GETIMAGEPIXEL(x,y) GETSHADEDCOLOR(TexMap[(TexMapWidth*((int)(y)))+((int)(x))])
// GETSHADEDCOLOR( *( (UCHAR*)texmap[TexMapId]->sprite_lookup[y] + x ) );
#define TESTSPRITE { int x,y; for(y=0;y<texmap[TexMapId].h;y++) for(x=0;x<texmap[TexMapId].w;x++) PutPixel(x,y,TexMap[y*texmap[TexMapId].w + x]); FlipPage();}


//
//
//
void ___DrawTexturedFace( object_ptr obj, face_ptr face, FIXED *intensity_parm ) {

	int		MinY,MaxY,MinVert = 0,i;
	edgescan_t	LeftEdge,RightEdge;

	O		= obj;
	F		= face;

	NumVerts	= F->nverts;
	TexVertsPtr	= F->texvert;
	TexMapId	= F->texmapid;
	TexMap		= &texmap[TexMapId].sprite1[SPRITEHEADER];
	BumpMap 	= &texmap[TexMapId].bump1[SPRITEHEADER];
	TexMapWidth	= texmap[TexMapId].width1;
	TexMapHeight	= texmap[TexMapId].height1;
	wmask		= texmap[TexMapId].wmask1;
	hmask		= texmap[TexMapId].hmask1;
	intensity	= intensity_parm;

	//TESTSPRITE;

	if( NumVerts < 3 )
		return;


	MinY = 32767;
	MaxY = -32768;


	for( i=0; i<NumVerts; i++ ) {

		VertexPtr[i] = &(O->ppoint[ F->vert[i] ]);

		if( VP(i,1) < MinY ) {
			MinY = VP(i,1);
			MinVert = i;
		}

		if( VP(i,1) > MaxY ) {
			MaxY = VP(i,1);
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

		if( DestY > CLIPMAXY )
			return;

		if( DestY >= CLIPMINY )
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

	Edge->NormalX += Edge->NormalStepX;
	Edge->NormalY += Edge->NormalStepY;

	Edge->DestX += Edge->DestXStep;

	Edge->Light += Edge->LightStep;

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


		if( (Edge->RemainingScans = VP(NextVert,1) - VP(StartVert,1)) != 0 ) {

#define VNX(i) ( ( FLOAT_TO_FIXED(O->pointnormal[F->vert[(i)]][0]) << 7) + INT_TO_FIXED(128) )
#define VNY(i) ( ( FLOAT_TO_FIXED(O->pointnormal[F->vert[(i)]][1]) << 7) + INT_TO_FIXED(128) )
#define VNZ(i) ( ( FLOAT_TO_FIXED(O->pointnormal[F->vert[(i)]][2]) << 7) + INT_TO_FIXED(128) )
#define TX(i) ( INT_TO_FIXED(TexVertsPtr[i][0]) )
#define TY(i) ( INT_TO_FIXED(TexVertsPtr[i][1]) )

			// texture mapping

			Edge->CurrentEnd	= NextVert;

			Edge->SourceX		= TX(StartVert);
			Edge->SourceY		= TY(StartVert);

			DestYHeight		= fdiv( FIXED_ONE, INT_TO_FIXED(Edge->RemainingScans) );
			Edge->SourceStepX	= fmul( TX(NextVert) - TX(StartVert) , DestYHeight );
			Edge->SourceStepY	= fmul( TY(NextVert) - TY(StartVert) , DestYHeight );


			// vertex normal for phong

			Edge->NormalX		= VNX(StartVert);
			Edge->NormalY		= VNY(StartVert);

			Edge->NormalStepX	= fmul( VNX(NextVert) - VNX(StartVert) , DestYHeight );
			Edge->NormalStepY	= fmul( VNY(NextVert) - VNY(StartVert) , DestYHeight );

			/*
			printf("sx: %.3f sy: %.3f  nx: %.3f ny: %.3f\n",
					FIXED_TO_FLOAT(VNX(StartVert)),
					FIXED_TO_FLOAT(VNY(StartVert)),
					FIXED_TO_FLOAT(VNX(NextVert)),
					FIXED_TO_FLOAT(VNY(NextVert)) );
			*/

			Edge->DestX = INT_TO_FIXED(VP(StartVert,0));

			if( (DestXWidth = (VP(NextVert,0) - VP(StartVert,0))) < 0 )
				Edge->DestXStep = -fdiv( INT_TO_FIXED(-DestXWidth), INT_TO_FIXED(Edge->RemainingScans) );
			else
				Edge->DestXStep =  fdiv( INT_TO_FIXED( DestXWidth), INT_TO_FIXED(Edge->RemainingScans) );



			// light for gouraud

			Edge->Light = (intensity[ StartVert ]<FIXED_ZERO)?FIXED_ZERO:intensity[ StartVert ];
			Edge->LightStep = fmul( ((intensity[ NextVert ]<FIXED_ZERO)?FIXED_ZERO:intensity[ NextVert ]) - Edge->Light, DestYHeight );
			//printf("i1: %.3f  i2: %.3f\n",FIXED_TO_FLOAT(intensity[StartVert]),FIXED_TO_FLOAT(intensity[NextVert]) );

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
	FIXED	NormalXStep,NormalYStep;
	FIXED	NormalX 	= LeftEdge->NormalX;
	FIXED	NormalY 	= LeftEdge->NormalY;
	FIXED	Light		= LeftEdge->Light;
	FIXED	LightStep;

	if( (DestXMax < CLIPMINX) || (DestX > CLIPMAXX) )
		return;

	if((DestXMax - DestX) <= 0)
		return;

	DestWidth = fdiv( FIXED_ONE, INT_TO_FIXED(DestXMax - DestX) );

	SourceXStep = fmul( (RightEdge->SourceX - SourceX) , DestWidth );
	SourceYStep = fmul( (RightEdge->SourceY - SourceY) , DestWidth );

	LightStep = fmul( (RightEdge->Light - Light), DestWidth );
	//printf("l: %.2f, ls: %.2f\n",FIXED_TO_FLOAT(Light),FIXED_TO_FLOAT(LightStep));

	NormalXStep = fmul( (RightEdge->NormalX - NormalX) , DestWidth );
	NormalYStep = fmul( (RightEdge->NormalY - NormalY) , DestWidth );

	if( DestXMax > CLIPMAXX + 1 )
		DestXMax = CLIPMAXX + 1;


	if( DestX < CLIPMINX ) {
		SourceX += fmul( SourceXStep , INT_TO_FIXED(CLIPMINX - DestX) );
		SourceY += fmul( SourceYStep , INT_TO_FIXED(CLIPMINX - DestX) );
		NormalX += fmul( NormalXStep , INT_TO_FIXED(CLIPMINX - DestX) );
		NormalY += fmul( NormalYStep , INT_TO_FIXED(CLIPMINX - DestX) );
		Light	+= fmul( LightStep   , INT_TO_FIXED(CLIPMINX - DestX) );
		DestX = CLIPMINX;
	}

#define ENVIR environment_map[ ((NormalY>>8)&(255<<8)) + (NormalX>>16) ]
//////////////////#define SAMPLE TexMap[ ((SourceY>>10)&(63<<6)) + ((SourceX>>16)&63) ]
//#define SAMPLE TexMap[ ((SourceY>>(FP_SHIFT-wmask))&((TexMapHeight-1)<<wmask)) + ((SourceX>>FP_SHIFT)&(TexMapWidth-1)) ]
//#define BUMP BumpMap[ ((SourceY>>(FP_SHIFT-wmask))&((TexMapHeight-1)<<wmask)) + ((SourceX>>FP_SHIFT)&(TexMapWidth-1)) ]
#define SAMPLE TexMap[ (TexMapWidth*(SourceY>>FP_SHIFT)) + (SourceX>>FP_SHIFT) ]
#define BUMP BumpMap[ (TexMapWidth*(SourceY>>FP_SHIFT)) + (SourceX>>FP_SHIFT) ]
#define LIGHT ( Light>>12 )
//#define COLOR STABLE( SAMPLE, ENVIR )
#define COLOR STABLE( SAMPLE, LIGHT )
//#define COLOR TRANSCOLOR( STABLE( SAMPLE, LIGHT ), GetPixel(DestX,DestY) )
//#define COLOR TRANSCOLOR( STABLE( SAMPLE, ENVIR) , GetPixel(DestX,DestY) )
//#define COLOR ( STABLE( SAMPLE, LIGHT ) + BUMP )
//#define COLOR SAMPLE

	// LockSurface( LS_WRITE );

	for( ; DestX<DestXMax; DestX++ ) {

		//int color = COLOR;
		//color = AntiAlias( DestX, DestY, color );
		//printf("%d\n",color);

		rgb_t c = {100,200,200,255};

		PutPixel(DestX, DestY, c );

		//printf("sx: %d  sy: %d\n",FIXED_TO_INT(SourceX),FIXED_TO_INT(SourceY));
		//printf("x: %d  y: %d\n",((SourceX>>16)&63),((SourceY>>10)&(63<<6)));
		//printf("tex: %d  env: %d  col: %d\n",SAMPLE,ENVIR,COLOR);

		SourceX += SourceXStep;
		SourceY += SourceYStep;

		NormalX += NormalXStep;
		NormalY += NormalYStep;

		Light += LightStep;

		//if( keys[sc_Escape] ) Quit("...");
	}

	// UnlockSurface();

	//if( keys[sc_Space] )
	//while( !keys[sc_Space] );
	//WaitTimer( TICKBASE / 40 );
	//while( keys[sc_Space] );


	return;
}


#endif


