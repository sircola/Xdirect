/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <xlib.h>
#include <xinner.h>

RCSID( "$Id: face.c,v 1.0 97-03-10 17:37:01 bernie Exp $" )



static int bbox_num = 8;		// hány face tartozik egy bounding box-hoz


//
//
//
void MakeRasterFace( face_ptr f, UCHAR color ) {

	ERASEFLAG( f->flag, FF_TEXTURE );
	ERASEFLAG( f->flag, FF_COLOR );
	SETFLAG( f->flag, FF_RASTER );

	f->color = color;

	return;
}





//
//
//
void MakeColorFace( face_ptr f, UCHAR color ) {

	ERASEFLAG( f->flag, FF_TEXTURE );
	ERASEFLAG( f->flag, FF_RASTER );
	SETFLAG( f->flag, FF_COLOR );

	f->color = color;

	return;
}





//
//
//
void MakeTextureFace( face_ptr f, UCHAR *s ) {

	point_ptr p;

	ERASEFLAG( f->flag, FF_COLOR   );
	ERASEFLAG( f->flag, FF_RASTER  );
	SETFLAG(   f->flag, FF_TEXTURE );

	ALLOCMEM( f->texvert, f->nverts * sizeof(point_t) );

	p = f->texvert;
	f->texmapid = 0;	     // na ez kul lesz

	p[0][0] = 0;
	p[0][1] = 0;
	p[1][0] = 0;
	p[1][1] = SPRITEH(s) - 1;
	p[2][0] = SPRITEW(s) - 1;
	p[2][1] = SPRITEH(s) - 1;
	p[3][0] = SPRITEW(s) - 1;
	p[3][1] = 0;


	return;
}







//
// face center,
//
void NormalizeObject( object_ptr o ) {

	int i,j;
	// face_ptr f;
	clock_t start;

	start = clock();

	ZEROVECTOR( o->center );


	for( i=0; i<o->npoints; i++ )

		ZEROVECTOR( o->pointnormal[i] );


	for( i=0; i<o->nfaces; i++ ) {

#define F (&o->face[i])
		//f = &o->face[i];

		/* centerize */

		ZEROVECTOR( F->center );

		//f->pcenter[0] = CLIPMINX;
		//f->pcenter[1] = CLIPMINY;

		for(j=0; j<F->nverts; j++ ) {

			F->center[0] += o->point[ F->vert[j] ][0];
			F->center[1] += o->point[ F->vert[j] ][1];
			F->center[2] += o->point[ F->vert[j] ][2];

		}

		F->center[0] = F->center[0] / F->nverts;
		F->center[1] = F->center[1] / F->nverts;
		F->center[2] = F->center[2] / F->nverts;

		/* normalize face */

		NormalVectorPlane( o->point[ F->vert[0] ],
				   o->point[ F->vert[1] ],
				   o->point[ F->vert[2] ],
				   F->normal );

		NormalizeVector( F->normal );
		//ZEROVECTOR( f->tnormal );

		//f->pnormal[0] = CLIPMINX;
		//f->pnormal[1] = CLIPMINY;


		/* normalize points of face */

		for( j=0; j<F->nverts; j++ )

			AddVector( o->pointnormal[ F->vert[j] ], F->normal, o->pointnormal[ F->vert[j] ] );

//		NormalizeVector( F->normal );

		/* make object center */

		//AddVector( o->center, F->center, &o->center );
		o->center[0] += F->center[0];
		o->center[1] += F->center[1];
		o->center[2] += F->center[2];

		winSetPanel(NULL,(o->nfaces+o->npoints)+i);
	}

	// object center sulypontban

	//xprintf("o->center (%s): elott %.3f %.3f %.3f\n",o->name,o->center[0],o->center[1], o->center[2]);
	o->center[0] = o->center[0] / o->nfaces;
	o->center[1] = o->center[1] / o->nfaces;
	o->center[2] = o->center[2] / o->nfaces;
	//xprintf("o->center (%s): utan %.3f %.3f %.3f\n",o->name,o->center[0],o->center[1],o->center[2]);

	//xprintf("begin\n");
	for( i=0; i<o->npoints; i++ ) {

		//DumpVector( o->pointnormal[i], "elotte");
		NormalizeVector( o->pointnormal[i] );
		//DumpVector( o->pointnormal[i], "utana");

		winSetPanel(NULL,(o->nfaces+o->npoints)+o->nfaces+i);
	}
	//xprintf("ende\n");

	//xprintf("NormalizeObject: %.2f sec\n",(FLOAT)(clock()-start)/CLOCKS_PER_SEC);

	UpdateObjectFaceBbox( o );

#undef F

	return;
}





//
//
//
BOOL EnvmapObject( object_t *obj, int texmapid, int alpha ) {

	int i,j,u,v,w,h;
	texmap_t *texmap = NULL;

#define F (&obj->face[i])

	if( obj == NULL )
		return FALSE;

	if( (texmapid == -1) || (texmap = TexForNum(texmapid)) == NULL ) {

		// clear Envmap

		for( i=0; i<obj->nfaces; i++ ) {
			ERASEFLAG( F->flag, FF_ENVMAP );
		}

		return TRUE;
	}

	w = texmap->width1;
	h = texmap->height1;

	// xprintf("texmap: %d, %d\n",w,h);

	for( i=0; i<obj->nfaces; i++ ) {

		SETFLAG( F->flag, FF_ENVMAP );

		F->texmapid1 = texmapid;

		for( j=0; j<F->nverts; j++ ) {

			u = (int)(obj->pointnormal[ F->vert[j] ][0] * (FLOAT)(w / 2) + w/2);
			v = (int)(obj->pointnormal[ F->vert[j] ][1] * (FLOAT)(h / 2) + h/2);

			if( u<0 ) u = -u;
			if( v<0 ) v = -v;

			if( u>w ) u %= w;
			if( v>h ) v %= h;

			F->texvert1[j][0] = u;
			F->texvert1[j][1] = v;

			if( w == h ) {
				F->s1[j] = (FLOAT)u * 255.0f / (FLOAT)w;
				F->t1[j] = (FLOAT)v * 255.0f / (FLOAT)h;
			}
			else
			if( w > h ) {
				F->s1[j] = (FLOAT)u * 255.0f / (FLOAT)w;
				F->t1[j] = ( (FLOAT)h * 255.0f / (FLOAT)w ) * (FLOAT)v / 255.0f;
			}
			else
			if( w < h ) {
				F->s1[j] = ( (FLOAT)w * 255.0f / (FLOAT)h ) * (FLOAT)u / 255.0f;
				F->t1[j] = (FLOAT)v * 255.0f / (FLOAT)h;
			}

			F->s1[j] += 0.5f;
			F->t1[j] += 0.5f;

			// F->s1[j] = F->s[j];
			// F->t1[j] = F->t[j];

			F->rgb1[j].r = 255;
			F->rgb1[j].g = 255;
			F->rgb1[j].b = 255;
			F->rgb1[j].a = (alpha == -1) ? 127 : alpha;

			// xprintf("%d, %d -> %.2f, %.2f\n",u,v,F->s1[j],F->t1[j]);
		}
	}
#undef F

	return TRUE;
}





//
// elforgatja a objectet a TENGELYek korul, radianban
//
void __RotateObject( object_ptr o, FLOAT Xan, FLOAT Yan, FLOAT Zan ) {

	point3_t a;
	matrix_t m;
	int i;

	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;

	if( Xan != 0.0f )  RotateMatrixX( m, Xan );
	if( Yan != 0.0f )  RotateMatrixY( m, Yan );
	if( Zan != 0.0f )  RotateMatrixZ( m, Zan );

	for( i=0; i<o->npoints; i++ ) {

		o->point[i][0] -= o->center[0];
		o->point[i][1] -= o->center[1];
		o->point[i][2] -= o->center[2];

		MulMatrixVector( m, o->point[i], a );

		o->point[i][0] = a[0] + o->center[0];
		o->point[i][1] = a[1] + o->center[1];
		o->point[i][2] = a[2] + o->center[2];
	}


	for( i=0; i<o->nfaces; i++ ) {

		o->face[i].center[0] -= o->center[0];
		o->face[i].center[1] -= o->center[1];
		o->face[i].center[2] -= o->center[2];

		MulMatrixVector( m, o->face[i].center, a );

		o->face[i].center[0] = a[0] + o->center[0];
		o->face[i].center[1] = a[1] + o->center[1];
		o->face[i].center[2] = a[2] + o->center[2];
	}

	NormalizeObject( o );

	return;
}




//
//
//
void RotateObject( object_t *o, FLOAT y, FLOAT z, FLOAT x /*, FLOAT zd, FLOAT p */ ) {

	int i,j;

	// precalc values..

	FLOAT sx = GetSin(-x);
	FLOAT sy = GetSin(y);
	FLOAT sz = GetSin(z);

	FLOAT cx = GetCos(-x);
	FLOAT cy = GetCos(y);
	FLOAT cz = GetCos(z);

	FLOAT mx1 = sx*sy*sz + cz*cx;
	FLOAT my1 = sx*cy;
	FLOAT mz1 = sz*cx - sx*sy*cz;

	FLOAT mx2 = cx*sy*sz - cz*sx;
	FLOAT my2 = cx*cy;
	FLOAT mz2 = -cz*sy*cx - sz*sx;

	FLOAT mx3 = -sz*cy;
	FLOAT my3 = sy;
	FLOAT mz3 = cz*cy;

	for( i=0; i<o->npoints; i++ ) {

		// rotate coords

		x = o->point[i][0] - o->center[0];
		y = o->point[i][1] - o->center[1];
		z = o->point[i][2] - o->center[2];

		o->point[i][0] = x*mx1 + y*my1 + z*mz1 + o->center[0];
		o->point[i][1] = x*mx2 + y*my2 + z*mz2 + o->center[1];
		o->point[i][2] = x*mx3 + y*my3 + z*mz3 + o->center[2];

		// project

		/***
		z += zd;

		O->v[i].px = (x/z)*p + 160;
		O->v[i].py = (y/z)*p*.8 + 100;
		***/

		// rotate normals

		x = o->pointnormal[i][0];
		y = o->pointnormal[i][1];
		z = o->pointnormal[i][2];

		o->pointnormal[i][0] = x*mx1 + y*my1 + z*mz1;
		o->pointnormal[i][1] = x*mx2 + y*my2 + z*mz2;
		o->pointnormal[i][2] = x*mx3 + y*my3 + z*mz3;
	}

#define F (&o->face[i])

	for( i=0; i<o->nfaces; i++ ) {

		/*
		x = O->f[i].xn;
		y = O->f[i].yn;
		z = O->f[i].zn;

		O->f[i].xn = x*mx1 + y*my1 + z*mz1;
		O->f[i].yn = x*mx2 + y*my2 + z*mz2;
		O->f[i].zn = x*mx3 + y*my3 + z*mz3;
		*/

		FLOAT p1x = o->point[ F->vert[0] ][0];
		FLOAT p1y = o->point[ F->vert[0] ][1];
		FLOAT p1z = o->point[ F->vert[0] ][2];

		FLOAT p2x = o->point[ F->vert[1] ][0];
		FLOAT p2y = o->point[ F->vert[1] ][1];
		FLOAT p2z = o->point[ F->vert[1] ][2];

		FLOAT p3x = o->point[ F->vert[2] ][0];
		FLOAT p3y = o->point[ F->vert[2] ][1];
		FLOAT p3z = o->point[ F->vert[2] ][2];

		F->normal[0] = ((p3y - p2y) * (p1z - p2z)) - ((p1y - p2y) * (p3z - p2z));
		F->normal[1] = ((p3z - p2z) * (p1x - p2x)) - ((p1z - p2z) * (p3x - p2x));
		F->normal[2] = ((p3y - p2y) * (p1x - p2x)) - ((p1y - p2y) * (p3x - p2x));

		ZEROVECTOR( F->center );

		for( j=0; j<F->nverts; j++ ) {

			F->center[0] += o->point[ F->vert[j] ][0];
			F->center[1] += o->point[ F->vert[j] ][1];
			F->center[2] += o->point[ F->vert[j] ][2];

		}

		F->center[0] = F->center[0] / F->nverts;
		F->center[1] = F->center[1] / F->nverts;
		F->center[2] = F->center[2] / F->nverts;
   }

#undef F

   return;
}




//
//
//
void MoveObject( object_ptr o, FLOAT x, FLOAT y, FLOAT z ) {

	int i;
	point3_t v;

	MAKEVECTOR( v, x,y,z );

	AddVector( o->center, v, o->center );

	for( i=0; i<o->npoints; i++ )
		AddVector( o->point[i], v, o->point[i] );

	for( i=0; i<o->nfaces; i++ )
		AddVector( o->face[i].center, v, o->face[i].center );

#ifdef INCCOLDET
	o->model->moveModel( x,y,z );
#endif
	return;
}







//
//
//
void MulMatrixObject( matrix_t T, object_ptr o ) {

	point3_t a;
	int i;

	MulMatrixVector( T, o->center, a );
	CopyVector( o->center, a );

	for( i=0; i<o->npoints; i++ ) {

		MulMatrixVector( T, o->point[i], a );
		CopyVector( o->point[i], a );
	}


	for( i=0; i<o->nfaces; i++ ) {

		MulMatrixVector( T, o->face[i].center, a );
		CopyVector( o->face[i].center, a );
	}

	NormalizeObject( o );

	return;
}





//
//
//
void BoundObject( object_t *obj, bound_t box ) {

	int i;

	box.x1 = FLOAT_MIN;
	box.x2 = FLOAT_MAX;

	box.y1 = FLOAT_MIN;
	box.y2 = FLOAT_MAX;

	box.z1 = FLOAT_MIN;
	box.z2 = FLOAT_MAX;

	for( i=0; i<obj->npoints; i++ ) {

		if( obj->point[i][0] > box.x1 )
			box.x1 = obj->point[i][0];

		if( obj->point[i][0] < box.x2 )
			box.x2 = obj->point[i][0];

		if( obj->point[i][1] > box.y1 )
			box.y1 = obj->point[i][1];

		if( obj->point[i][1] < box.y2 )
			box.y2 = obj->point[i][1];

		if( obj->point[i][2] > box.z1 )
			box.z1 = obj->point[i][2];

		if( obj->point[i][2] < box.z2 )
			box.z2 = obj->point[i][2];
	}

	//xprintf("x1,x2: %.2f, %.2f\n",box.x1,box.x2);
	//xprintf("y1,y2: %.2f, %.2f\n",box.y1,box.y2);
	//xprintf("z1,z2: %.2f, %.2f\n",box.z1,box.z2);

	return;
}





//
//
//
int getBboxFaceNum( int new_value ) {

	if( new_value > 0 )
		bbox_num = new_value;

	return bbox_num;
}



//
//
//
BOOL UpdateObjectFaceBbox( object_t *obj ) {

	BOOL result = TRUE;
	face_t *face = NULL;
	bbox_t *bbox = NULL, *obj_bbox = NULL;

	// az object bounding box-a
	obj_bbox = &(obj->bbox);

	(*obj_bbox).min[0] = FLOAT_MAX;		// X min
	(*obj_bbox).min[1] = FLOAT_MAX;		// Y min
	(*obj_bbox).min[2] = FLOAT_MAX;		// Z min

	(*obj_bbox).max[0] = FLOAT_MIN;		// X min
	(*obj_bbox).max[1] = FLOAT_MIN;		// Y min
	(*obj_bbox).max[2] = FLOAT_MIN;		// Z min

	(*obj_bbox).dirty = FALSE;

	// a face-ké
	for( int i=0; i<obj->nfaces; i+=bbox_num ) {

		if( i >= obj->nfaces )
			break;

		bbox = &(obj->face[ i ].bbox);

		(*bbox).min[0] = FLOAT_MAX;		// X min
		(*bbox).min[1] = FLOAT_MAX;		// Y min
		(*bbox).min[2] = FLOAT_MAX;		// Z min

		(*bbox).max[0] = FLOAT_MIN;		// X min
		(*bbox).max[1] = FLOAT_MIN;		// Y min
		(*bbox).max[2] = FLOAT_MIN;		// Z min

		(*bbox).dirty = FALSE;

		for( int l=i; l<(i+bbox_num); l++ ) {

			if( l >= obj->nfaces )
				break;

			face = &obj->face[ l ];

			for( int j=0; j<face->nverts; j++ ) {

				int n = face->vert[j];

				// MIN
				for( int k=0; k<MATROW; k++ ) {

					// face
					if( obj->point[ n ][k] < (*bbox).min[k] )
						(*bbox).min[k] = obj->point[ n ][k];

					// object
					if( obj->point[ n ][k] < (*obj_bbox).min[k] )
						(*obj_bbox).min[k] = obj->point[ n ][k];
				}

				// MAX
				for( int k=0; k<MATROW; k++ ) {

					// face
					if( obj->point[ n ][k] > (*bbox).max[k] )
						(*bbox).max[k] = obj->point[ n ][k];

					// object
					if( obj->point[ n ][k] > (*obj_bbox).max[k] )
						(*obj_bbox).max[k] = obj->point[ n ][k];
				}
			}

			winSetPanel(NULL,(obj->nfaces+obj->npoints)+(obj->nfaces+obj->npoints)+l);
		}
	}

	return result;
}


/***
function NextPow2(iNum: integer): integer;
var
iTmp: integer;
begin
   iTmp := 1;
   while (iTmp<iNum) do
    iTmp:= iTmp shl 1;

   result := iTmp;
end;
***/