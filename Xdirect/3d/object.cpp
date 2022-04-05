/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)


#include <stdio.h>
#include <string.h>
#include <vector>

#include <xlib.h>

RCSID( "$Id: object.c,v 1.2 97-03-10 17:00:06 bernie Exp $" )

#define CLD_TEST

static BOOL cld_test = TRUE;

//
//
//
BOOL CldTest( BOOL flag ) {

	if( flag != (-1) )
		cld_test = flag;

	return cld_test;
}



//
//
//
BOOL DupObject( object_t *obj, object_ptr *obj_ptr ) {

	int i,npoints,nfaces,nverts;
	face_ptr f1,f2;

	*obj_ptr = NULL;
	ALLOCMEM( *obj_ptr, sizeof(object_t) );

	ALLOCMEM( (*obj_ptr)->name, (strlen(obj->name) + 1) );
	strcpy( (*obj_ptr)->name, obj->name );

	(*obj_ptr)->npoints = obj->npoints;
	(*obj_ptr)->nfaces = obj->nfaces;

	npoints = (*obj_ptr)->npoints;
	nfaces	= (*obj_ptr)->nfaces ;

	ALLOCMEM( (*obj_ptr)->point , npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->tpoint, npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->ppoint, npoints * sizeof(point_t)	);
	ALLOCMEM( (*obj_ptr)->pointnormal , npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->xform_flag , npoints * sizeof(ULONG) );
	memset( (*obj_ptr)->xform_flag, 0L, npoints * sizeof(ULONG) );

	memcpy( (UCHAR*)((*obj_ptr)->point), obj->point, npoints * sizeof(point3_t) );
	memcpy( (UCHAR*)((*obj_ptr)->pointnormal), obj->pointnormal, npoints * sizeof(point3_t) );

	CopyVector( (*obj_ptr)->center, obj->center );

	ALLOCMEM( (*obj_ptr)->face, nfaces * sizeof(face_t) );

	for( i=0; i<nfaces; i++) {

		ALLOCFACE( (*obj_ptr)->face[i], obj->face[i].nverts );

		f1 = &((*obj_ptr)->face[i]);
		f2 = &(obj->face[i]);

		nverts = f1->nverts;

		memcpy( (UCHAR*)(f1->vert), (UCHAR*)(f2->vert), nverts * sizeof(int) );

		CopyVector( f1->normal, f2->normal );
		CopyVector( f1->center, f2->center );

		memcpy( (UCHAR*)(f1->rgb), (UCHAR*)(f2->rgb), nverts * sizeof(rgb_t) );
		memcpy( (UCHAR*)(f1->rgb1), (UCHAR*)(f2->rgb1), nverts * sizeof(rgb_t) );
		memcpy( (UCHAR*)(f1->rgb2), (UCHAR*)(f2->rgb2), nverts * sizeof(rgb_t) );

		f1->flag = f2->flag;

		if( ISFLAG(f1->flag,FF_TEXTURE) ) {

			f1->texmapid = f2->texmapid;
			f1->texmapid1 = f2->texmapid1;
			f1->texmapid2 = f2->texmapid2;

			memcpy( (UCHAR*)(f1->texvert), (UCHAR*)(f2->texvert), nverts * sizeof(point_t) );
			memcpy( (UCHAR*)(f1->s), (UCHAR*)(f2->s), nverts * sizeof(FLOAT) );
			memcpy( (UCHAR*)(f1->t), (UCHAR*)(f2->t), nverts * sizeof(FLOAT) );

			if( f2->texmapid1 != -1 ) {

				memcpy( (UCHAR*)(f1->texvert1), (UCHAR*)(f2->texvert1), nverts * sizeof(point_t) );
				memcpy( (UCHAR*)(f1->s1), (UCHAR*)(f2->s1), nverts * sizeof(FLOAT) );
				memcpy( (UCHAR*)(f1->t1), (UCHAR*)(f2->t1), nverts * sizeof(FLOAT) );
			}

			if( f2->texmapid2 != -1 ) {

				memcpy( (UCHAR*)(f1->texvert2), (UCHAR*)(f2->texvert2), nverts * sizeof(point_t) );
				memcpy( (UCHAR*)(f1->s2), (UCHAR*)(f2->s2), nverts * sizeof(FLOAT) );
				memcpy( (UCHAR*)(f1->t2), (UCHAR*)(f2->t2), nverts * sizeof(FLOAT) );
			}
		}
		else
		if( ISFLAG(f1->flag,FF_COLOR) ||
		    ISFLAG(f1->flag,FF_RASTER)	) {

			f1->color = f2->color;
		}
		else
		if( ISFLAG(f1->flag,FF_SPRITE) ) {

			f1->texmapid = f2->texmapid;
		}
	}

#ifdef INCCOLDET

	(*obj_ptr)->model = NULL;

#ifdef CLD_TEST

	if( cld_test ) {

		(*obj_ptr)->model = CLD_CreateModel();

		for( i=0; i<nfaces; i++ ) {

			(*obj_ptr)->model->addTriangle(

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][2],

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][2],

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][2] );
		}

		(*obj_ptr)->model->finalize();
	}
#endif

#endif

	UpdateObjectFaceBbox( *obj_ptr );

	NAMEMEM( *obj_ptr, obj->name );

	return TRUE;
}




//
//
//
void InterpolateObject( object_t *o1, object_t *o2, object_t *o3, FLOAT scale ) {

	int i;

	if( (o3 == NULL) || (o1 == NULL) ) return;

	if( o3->npoints != o1->npoints ) return;

	if( o2 == NULL ) {

		for( i=0; o3->npoints; i++ ) {

			if( i >= o1->npoints ) break;

			o3->point[i][0] = o1->point[i][0];
			o3->point[i][1] = o1->point[i][1];
			o3->point[i][2] = o1->point[i][2];
		}
	}
	else {

		for( i=0; o3->npoints; i++ ) {

			if( (i >= o1->npoints) || (i >= o2->npoints) ) break;

			o3->point[i][0] = o1->point[i][0] + ( o2->point[i][0] - o1->point[i][0] ) * scale;
			o3->point[i][1] = o1->point[i][1] + ( o2->point[i][1] - o1->point[i][1] ) * scale;
			o3->point[i][2] = o1->point[i][2] + ( o2->point[i][2] - o1->point[i][2] ) * scale;
		}
	}

	// NormalizeObject( o3 );

	return;
}





//
// betolt egy Object-et
// out: face( normal, center, color )
//
BOOL LoadBin( char *name, object_ptr *obj_ptr ) {

	int i,npoints,nfaces,nverts;
	char string[XMAX_PATH];
	face_t *f;
	FILE *file;
	static int cnt = 0;

	if( (file = ffopen(name,"rb")) == NULL ) {
		xprintf("LoadBin: can't open \"%s\" file.\n",name);
		return FALSE;
	}

	ffread(string,4,1,file);
	if( !IdBufStr(string, (char *)FIS_OBJ) ) {
		xprintf("LoadBin: \"%s\" is not OBJ1.\n",name);
		ffclose( file );
		return FALSE;
	}

	*obj_ptr = NULL;
	ALLOCMEM( *obj_ptr, sizeof(object_t) );
	memset( *obj_ptr, 0L, sizeof(object_t) );

	ffread( &(*obj_ptr)->npoints, sizeof((*obj_ptr)->npoints), 1, file );
	ffread( &(*obj_ptr)->nfaces, sizeof((*obj_ptr)->nfaces), 1, file );

	npoints = (*obj_ptr)->npoints;
	nfaces	= (*obj_ptr)->nfaces ;

	// 0 pont vagy face
	if( !npoints || !nfaces ) {
		ffclose(file);
		FREEMEM( *obj_ptr );
		return FALSE;
	}

	ALLOCMEM( (*obj_ptr)->name, (strlen(name) + 1) );
	strcpy( (*obj_ptr)->name, name );

	ALLOCMEM( (*obj_ptr)->point , npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->tpoint, npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->ppoint, npoints * sizeof(point_t)	);
	ALLOCMEM( (*obj_ptr)->pointnormal , npoints * sizeof(point3_t) );
	//ALLOCMEM( (*obj_ptr)->tpointnormal, npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->xform_flag , npoints * sizeof(ULONG) );
	memset( (*obj_ptr)->xform_flag, 0L, npoints * sizeof(ULONG) );

	ffread( (UCHAR*)((*obj_ptr)->point), npoints * sizeof(point3_t), 1, file );

	ALLOCMEM( (*obj_ptr)->face, nfaces * sizeof(face_t) );
	memset( (*obj_ptr)->face, 0L, nfaces * sizeof(face_t) );

	for( i=0; i<nfaces; i++) {

		f = &((*obj_ptr)->face[i]);

		ffread( &f->nverts, sizeof(f->nverts), 1, file );

		// TODO: eldobni a a verteket FACE_MAXNVERTS felett
		f->nverts = MIN(FACE_MAXNVERTS,f->nverts);

		ALLOCFACE( (*obj_ptr)->face[i], f->nverts );

		nverts = f->nverts;

		(*obj_ptr)->face[i].nverts = nverts;

		ffread( (UCHAR*)(f->vert), nverts * sizeof(int), 1, file );

		for( int j=0; j<nverts; j++ ) f->rgb[j].a = 255;

		ffread( &f->flag, sizeof(f->flag), 1, file );

		f->color = (cnt++&1) ? RGBINT( 0x00, 0xff, 0x00 ) : RGBINT( 0xff, 0x00, 0x00 );

		if( ISFLAG(f->flag,FF_TEXTURE) ) {

			ffgetcs( string, file );

			if( (f->texmapid = TexNumForName( string )) == (-1) )
				if( (f->texmapid = LoadTexmap( string, 0, 0L )) == (-1) ) {
					xprintf("LoadBin: no \"%s\" named texture for %s object.\n",string,name);
					ffclose( file );
					return FALSE;
				}

			ffread( (UCHAR*)(f->texvert), nverts * sizeof(point_t), 1, file );

			// TODO: s,t ki kell számolni
			// ffread( (UCHAR*)(f->s), nverts * sizeof(FLOAT), 1, file );
			// ffread( (UCHAR*)(f->t), nverts * sizeof(FLOAT), 1, file );

			ffgetcs( string, file );

			if( !stricmp(string, NULLTEX) )
				f->texmapid1 = -1;
			else {
				if( (f->texmapid1 = TexNumForName( string )) == (-1) )
					if( (f->texmapid1 = LoadTexmap( string, 0, 0L )) == (-1) ) {
						xprintf("LoadBin: no \"%s\" named texture for %s object.\n",string,name);
						ffclose( file );
						return FALSE;
					}

				ffread( (UCHAR*)(f->texvert1), nverts * sizeof(point_t), 1, file );

				// TODO: s,t ki kell számolni
				// ffread( (UCHAR*)(f->s1), nverts * sizeof(FLOAT), 1, file );
				// ffread( (UCHAR*)(f->t1), nverts * sizeof(FLOAT), 1, file );
			}

			ffgetcs( string, file );

			if( !stricmp(string, NULLTEX) )
				f->texmapid2 = -1;
			else {
				if( (f->texmapid2 = TexNumForName( string )) == (-1) )
					if( (f->texmapid2 = LoadTexmap( string, 0, 0L )) == (-1) ) {
						xprintf("LoadBin: no \"%s\" named texture for %s object.\n",string,name);
						ffclose( file );
						return FALSE;
					}

				ffread( (UCHAR*)(f->texvert2), nverts * sizeof(point_t), 1, file );

				// TODO: s,t ki kell számolni
				// ffread( (UCHAR*)(f->s2), nverts * sizeof(FLOAT), 1, file );
				// ffread( (UCHAR*)(f->t2), nverts * sizeof(FLOAT), 1, file );
			}
		}
		else
		if( ISFLAG(f->flag,FF_COLOR) ||
		    ISFLAG(f->flag,FF_RASTER)  ) {

			ffread( (UCHAR*)(f->rgb), sizeof(rgb_t), 1, file );

			f->color = (nverts>0)?RGBINT( f->rgb[0].r, f->rgb[0].g, f->rgb[0].b ):0;
		}
		else
		if( ISFLAG(f->flag,FF_SPRITE) ) {

			ffgetcs( string, file );

			if( (f->texmapid = TexNumForName( string )) == (-1) ) {
				xprintf("LoadBin: %s -> %s.\n",name,string);
				ffclose( file );
				return FALSE;
			}
		}
	}

	ffclose( file );

	NormalizeObject( (*obj_ptr) );

#ifdef INCCOLDET

	(*obj_ptr)->model = NULL;

#ifdef CLD_TEST

	if( cld_test ) {

		(*obj_ptr)->model = CLD_CreateModel();

		for( i=0; i<nfaces; i++ ) {

			(*obj_ptr)->model->addTriangle(

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][2],

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][2],

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][2] );
		}

		(*obj_ptr)->model->finalize();
	}
#endif
#endif

	NAMEMEM( *obj_ptr, name );

	return TRUE;
}





//
//
//
BOOL SaveBin( object_ptr obj_ptr ) {

	int i,npoints,nfaces,nverts;
	char *name;
	face_ptr f;
	FILE *file;

	npoints = obj_ptr->npoints;
	nfaces	= obj_ptr->nfaces;

	if( (file = ffopen(obj_ptr->name,"wb")) == NULL ) {
		xprintf("SaveBin: can't open \"%s\" file.\n",obj_ptr->name);
		return FALSE;
	}

	ffwrite( FIS_OBJ, FI_SIZE, 1, file );

	ffwrite( &obj_ptr->npoints, sizeof(obj_ptr->npoints), 1, file );
	ffwrite( &obj_ptr->nfaces, sizeof(obj_ptr->nfaces), 1, file );

	ffwrite( (UCHAR*)(obj_ptr->point), npoints * sizeof(point3_t), 1, file );

	for( i=0; i<nfaces; i++) {

		f = &obj_ptr->face[i];

		nverts = f->nverts;

		ffwrite( &f->nverts, sizeof(f->nverts), 1, file );
		ffwrite( (UCHAR*)(f->vert), nverts * sizeof(int), 1, file );

		ffwrite( &f->flag, sizeof(f->flag), 1, file );

		if( ISFLAG(f->flag,FF_COLOR) ||
		    ISFLAG(f->flag,FF_RASTER)  ) {

			ffwrite( (UCHAR*)(f->rgb), sizeof(rgb_t), 1, file );
		}
		else
		if( ISFLAG(f->flag,FF_TEXTURE) ) {

			if( (name=TexNameForNum( f->texmapid)) == NULL )
				name = BADTEX;

			ffputcs( name, file );

			ffwrite( (UCHAR*)(f->texvert), f->nverts * sizeof(point_t), 1, file );

			if( f->texmapid1 != -1 ) {

				if( (name=TexNameForNum( f->texmapid1)) == NULL )
					name = BADTEX;

				ffputcs( name, file );

				ffwrite( (UCHAR*)(f->texvert1), f->nverts * sizeof(point_t), 1, file );
			}
			else
				ffputcs( NULLTEX, file );


			if( f->texmapid2 != -1 ) {

				if( (name=TexNameForNum( f->texmapid2)) == NULL )
					name = BADTEX;

				ffputcs( name, file );

				ffwrite( (UCHAR*)(f->texvert2), f->nverts * sizeof(point_t), 1, file );
			}
			else
				ffputcs( NULLTEX, file );

		}
		else
		if( ISFLAG(f->flag,FF_SPRITE) ) {

			if( (name=TexNameForNum( f->texmapid)) == NULL )
				name = BADTEX;

			ffputcs( name, file );
		}


	}

	ffclose( file );

	return TRUE;
}







//
// betolt egy Object-et
// out:
//
BOOL LoadAsc(char *name, object_ptr *obj_ptr) {

	FILE *f;
	int i,j,k,npoints,nfaces,nverts,flag;
	FLOAT X,Y,Z;
	char string[XMAX_PATH];
	texmap_t *tex;
	static int cnt = 0;
	BOOL scaled = FALSE;
	std::vector<int> texids;

	xprintf("LoadAsc: loading \"%s\" ...\n", name );

	if((f=ffopen(name,"rt"))==NULL) {
		xprintf("can't open file.\n");
		return FALSE;
	}

	GetIntTxt( f, &npoints );
	GetIntTxt( f, &nfaces );

	// 0 pont vagy face
	if( !npoints || !nfaces ) {
		ffclose(f);
		xprintf("npoints: %d, nfaces %d, null object.\n",npoints,nfaces);
		return FALSE;
	}

	*obj_ptr = NULL;
	ALLOCMEM( *obj_ptr, sizeof(object_t) );

	if( npoints > 1000 ) {
		winShowPanel(name, "Loading");
		winSetPanel(NULL,0,(npoints+nfaces)*2+nfaces);
	}

	(*obj_ptr)->npoints = npoints;
	(*obj_ptr)->nfaces  = nfaces;

	ALLOCMEM( (*obj_ptr)->name, (strlen(name) + 1) );
	strcpy( (*obj_ptr)->name, name );

	ALLOCMEM( (*obj_ptr)->point , npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->tpoint, npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->ppoint, npoints * sizeof(point_t)	);
	ALLOCMEM( (*obj_ptr)->pointnormal , npoints * sizeof(point3_t) );
	//ALLOCMEM( (*obj_ptr)->tpointnormal, npoints * sizeof(point3_t) );
	ALLOCMEM( (*obj_ptr)->xform_flag , npoints * sizeof(ULONG) );
	memset( (*obj_ptr)->xform_flag, 0, npoints * sizeof(ULONG) );

	for( i=0; i<npoints; i++ ) {

		GetFloatTxt( f, &X );
		GetFloatTxt( f, &Y );
		GetFloatTxt( f, &Z );

		MAKEVECTOR( (*obj_ptr)->point[i], X,Y,Z );

		// xprintf("%s: %.2f %.2f %.2f\n", (*obj_ptr)->name, X,Y,Z );

		winSetPanel(NULL,i);
	}

	ALLOCMEM( (*obj_ptr)->face, nfaces * sizeof(face_t) );

	for( i=0; i<nfaces; i++ ) {

		GetIntTxt( f, &nverts );

		if( nverts <= 1 || nverts > FACE_MAXNVERTS )
			Quit("ASC file error in %s:\n\nFace %d has %d verts (max: %d).",name,i,nverts,FACE_MAXNVERTS);

		// TODO: eldobni a a verteket FACE_MAXNVERTS felett
		nverts = MIN(FACE_MAXNVERTS,nverts);

		ALLOCFACE( (*obj_ptr)->face[i], nverts );

		for( j=0; j<nverts; j++ ) {

			GetIntTxt( f, &k );

			if( k < 0 || k >= npoints )
				Quit("ASC file error in %s:\n\nFace %d [%d/%d] references %d vert.\n\nnpoints: %d\nnfaces: %d",name,i+1,j+1,nverts,k,npoints,nfaces);

			(*obj_ptr)->face[i].vert[j] = k;
		}

		memset( (*obj_ptr)->face[i].rgb, 0L, nverts * sizeof(rgb_t) );
		memset( (*obj_ptr)->face[i].rgb1, 0L, nverts * sizeof(rgb_t) );
		memset( (*obj_ptr)->face[i].rgb2, 0L, nverts * sizeof(rgb_t) );

		for( j=0; j<nverts; j++ ) {
			(*obj_ptr)->face[i].rgb[j].a = 255;
			(*obj_ptr)->face[i].rgb1[j].a = 255;
			(*obj_ptr)->face[i].rgb2[j].a = 255;
		}

		(*obj_ptr)->face[i].flag = 0L;
		GetStringTxt( f, string );

		(*obj_ptr)->face[i].color = (cnt++&1) ? RGBINT( 0x00, 0xff, 0x00 ) : RGBINT( 0xff, 0x00, 0x00 );

		if( !stricmp( string, FS_COLOR) ) {

			// van-e double?
			while( IsNextStringTxt( f ) ) {

				GetStringTxt( f, string );

				if( !stricmp( string, FS_DOUBLE) ) {
					SETFLAG( (*obj_ptr)->face[i].flag, FF_DBLSIDE );
				}
			}

			SETFLAG((*obj_ptr)->face[i].flag,FF_COLOR);
			GetIntTxt( f, &k );

			for( j=0; j<nverts; j++ ) {

				(*obj_ptr)->face[i].rgb[j].r = (ULONG)((k>>16)&0xff); // >> (8-rsize)) & rmask);
				(*obj_ptr)->face[i].rgb[j].g = (ULONG)((k>> 8)&0xff); // >> (8-gsize)) & gmask);
				(*obj_ptr)->face[i].rgb[j].b = (ULONG)((k>> 0)&0xff); // >> (8-bsize)) & bmask);
				(*obj_ptr)->face[i].rgb[j].a = 255;

				(*obj_ptr)->face[i].rgb1[j].r = (*obj_ptr)->face[i].rgb[j].r;
				(*obj_ptr)->face[i].rgb1[j].g = (*obj_ptr)->face[i].rgb[j].g;
				(*obj_ptr)->face[i].rgb1[j].b = (*obj_ptr)->face[i].rgb[j].b;
				(*obj_ptr)->face[i].rgb1[j].a = 255;

				(*obj_ptr)->face[i].rgb2[j].r = (*obj_ptr)->face[i].rgb[j].r;
				(*obj_ptr)->face[i].rgb2[j].g = (*obj_ptr)->face[i].rgb[j].g;
				(*obj_ptr)->face[i].rgb2[j].b = (*obj_ptr)->face[i].rgb[j].b;
				(*obj_ptr)->face[i].rgb2[j].a = 255;
			}

			(*obj_ptr)->face[i].color = (nverts>0)?RGB8INT( (*obj_ptr)->face[i].rgb[0].r,
									(*obj_ptr)->face[i].rgb[0].g,
									(*obj_ptr)->face[i].rgb[0].b ) : 0;
		}
		else
		if( !stricmp( string, FS_RASTER ) ) {

			SETFLAG((*obj_ptr)->face[i].flag,FF_RASTER);
			GetIntTxt( f, &k );
			(*obj_ptr)->face[i].color = k;

		}
		else
		if( !stricmp( string, FS_LINE ) ) {

			SETFLAG((*obj_ptr)->face[i].flag,FF_LINE);
			GetIntTxt( f, &k );
			(*obj_ptr)->face[i].color = k;

		}
		else
		if( !stricmp( string, FS_TEXTURE ) ) {

			int w,h;

			SETFLAG((*obj_ptr)->face[i].flag,FF_TEXTURE);

			flag = 0;

			while( 1 ) {

				GetStringTxt( f, string );

				if( !stricmp( string, FS_DOUBLE) ) {
					SETFLAG( (*obj_ptr)->face[i].flag, FF_DBLSIDE );
				}
				else
				if( !stricmp( string, FS_CHROME) ) {
					SETFLAG( flag, TF_CHROMA );
				}
				else
				if( !stricmp( string, FS_ALPHA) ) {
					SETFLAG( flag, TF_ALPHA );
				}
				else
					break;
			}

			if( ( (*obj_ptr)->face[i].texmapid = TexNumForName(string) ) == (-1) ) {
				xprintf("%s: \"%s\" not found.\n",name,string);
				if( (FileExist( string ) == FALSE) || (((*obj_ptr)->face[i].texmapid = LoadTexmap( string, flag, 0L )) == (-1)) ) {
					// az object directoryjából is megpróbálja
					char str[XMAX_PATH];
					sprintf( str, "%s\\%s", GetFilePath( name ), string );
					if( (FileExist( str ) == FALSE) || (((*obj_ptr)->face[i].texmapid = LoadTexmap( str, flag, 0L ) ) == (-1)) ) {
						// xprintf("LoadAsc: no \"%s\" named texture for %s object.\n",string,name);
						if( ((*obj_ptr)->face[i].texmapid = AddCheckerTexmap() ) == (-1) ) {
							xprintf("LoadAsc: can't create placeholder texture for %s object.\n",name);
							ffclose( f );
							winHidePanel();
							XLIB_MinimalMessagePump();
							return FALSE;
						}
					}
				}
			}

			w = 1;
			h = 1;
			scaled = FALSE;

			if( (tex = TexForNum((*obj_ptr)->face[i].texmapid)) != NULL ) {
				w = tex->width1;
				h = tex->height1;
				scaled = tex->scaled;
			}
			else
				xprintf("TexForNum: can't find texmap, using default value.\n");

			for( j=0; j<(*obj_ptr)->face[i].nverts; j++ ) {

				int u,v;

				GetIntTxt( f, &u );
				GetIntTxt( f, &v );

				// TODO: kis textúráknál pontatlan: int helyett FLOAT-ba kellene tárolni
				if( tex->width1 != tex->origw )
					u = ftoi( (FLOAT)u * (FLOAT)tex->width1 / (FLOAT)tex->origw );

				if( tex->height1 != tex->origh )
					v = ftoi( (FLOAT)v * (FLOAT)tex->height1 / (FLOAT)tex->origh );

				if( scaled ) {
					u /= texture_scale;
					v /= texture_scale;
				}

				if( u<0 ) u = -u;
				if( v<0 ) v = -v;

				if( u>w ) u %= w;
				if( v>h ) v %= h;

				(*obj_ptr)->face[i].texvert[j][0] = u;
				(*obj_ptr)->face[i].texvert[j][1] = v;

				if( w == h ) {
					(*obj_ptr)->face[i].s[j] = (FLOAT)u * 255.0f / (FLOAT)w;
					(*obj_ptr)->face[i].t[j] = (FLOAT)v * 255.0f / (FLOAT)h;
				}
				else
				if( w > h ) {
					(*obj_ptr)->face[i].s[j] = (FLOAT)u * 255.0f / (FLOAT)w;
					(*obj_ptr)->face[i].t[j] = ( (FLOAT)h * 255.0f / (FLOAT)w ) * (FLOAT)v / 255.0f;
				}
				else
				if( w < h ) {
					(*obj_ptr)->face[i].s[j] = ( (FLOAT)w * 255.0f / (FLOAT)h ) * (FLOAT)u / 255.0f;
					(*obj_ptr)->face[i].t[j] = (FLOAT)v * 255.0f / (FLOAT)h;
				}

				(*obj_ptr)->face[i].s[j] += 0.5f;
				(*obj_ptr)->face[i].t[j] += 0.5f;
			}

			(*obj_ptr)->face[i].texmapid1 = -1;
			(*obj_ptr)->face[i].texmapid2 = -1;
		}
		else
		if( !stricmp( string, FS_SPRITE ) ) {

			SETFLAG((*obj_ptr)->face[i].flag,FF_SPRITE);

			GetStringTxt(f,string);
			if( ( (*obj_ptr)->face[i].texmapid = TexNumForName(string) ) == (-1) ) {
				xprintf("LoadAsc: %s -> %s.\n",name,string);
				ffclose( f );
				return FALSE;
			}

		}

		winSetPanel(NULL,npoints+i);
	}

	ffclose(f);

	NormalizeObject( (*obj_ptr) );

#ifdef INCCOLDET

	(*obj_ptr)->model = NULL;

#ifdef CLD_TEST

	if( cld_test ) {

		(*obj_ptr)->model = CLD_CreateModel();

		for( i=0; i<nfaces; i++ ) {

			(*obj_ptr)->model->addTriangle(

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[0] ][2],

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[1] ][2],

				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][0],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][1],
				(*obj_ptr)->point[ (*obj_ptr)->face[i].vert[2] ][2] );
		}

		(*obj_ptr)->model->finalize();
	}
#endif
#endif

	NAMEMEM( *obj_ptr, name );

	xprintf("LoadAsc: done.\n");

	winHidePanel();

	return TRUE;
}








//
//
//
BOOL SaveAsc( object_ptr obj_ptr ) {

	FILE *f;
	int i,j,color,npoints,nfaces,nverts;

	if((f=ffopen(obj_ptr->name,"wt"))==NULL)
		return FALSE;

	ffprintf(f,"; created \"%s\" on %s\n", obj_ptr->name, GetDateStr() );

	ffprintf(f,"\n; [npoints]  [nfaces]\n");

	ffprintf(f,"\n%d %d\n", obj_ptr->npoints, obj_ptr->nfaces);

	npoints = obj_ptr->npoints;
	nfaces	= obj_ptr->nfaces;

	ffprintf(f,"\n; [verts]\n\n");

	for( i=0; i<npoints; i++ ) {

		ffprintf(f,"%.6f %.6f %.6f\n", obj_ptr->point[i][0],obj_ptr->point[i][1],obj_ptr->point[i][2] );
	}

	ffprintf(f,"\n; [faces]\n\n");

	for( i=0; i<nfaces; i++) {

		ffprintf(f,"%d   ", obj_ptr->face[i].nverts );

		nverts = obj_ptr->face[i].nverts;

		for( j=0; j<nverts; j++ )
			ffprintf(f,"%d ", obj_ptr->face[i].vert[j] );


		if( ISFLAG(obj_ptr->face[i].flag,FF_COLOR) ) {

			color = (nverts>0)?((ULONG)obj_ptr->face[i].rgb[0].r << 16) | ((ULONG)obj_ptr->face[i].rgb[0].g << 8) | ((ULONG)obj_ptr->face[i].rgb[0].b << 0):0;

			ffprintf(f," %s  %d", FS_COLOR, color);
		}
		else
		if( ISFLAG(obj_ptr->face[i].flag,FF_RASTER)  ) {

			color = (nverts>0)?((ULONG)obj_ptr->face[i].rgb[0].r << 16) | ((ULONG)obj_ptr->face[i].rgb[0].g << 8) | ((ULONG)obj_ptr->face[i].rgb[0].b << 0):0;

			ffprintf(f," %s  %d", FS_RASTER, color);
		}
		else
		if( ISFLAG(obj_ptr->face[i].flag,FF_LINE)  ) {

			color = (nverts>0)?((ULONG)obj_ptr->face[i].rgb[0].r << 16) | ((ULONG)obj_ptr->face[i].rgb[0].g << 8) | ((ULONG)obj_ptr->face[i].rgb[0].b << 0):0;

			ffprintf(f," %s  %d", FS_LINE, color);
		}
		else
		if( ISFLAG(obj_ptr->face[i].flag,FF_TEXTURE) ) {

			texmap_t *tex;

			// ffprintf(f," %s  %s   ", FS_TEXTURE, TexNameForNum( obj_ptr->face[i].texmapid ) );

			ffprintf(f," %s  ", FS_TEXTURE );

			if( (tex = TexForNum( obj_ptr->face[i].texmapid )) != NULL ) {
				if( tex->isChromaKey )
					ffprintf(f,"%s ", FS_CHROME );
				if( tex->isAlpha )
					ffprintf(f,"%s ", FS_ALPHA );
			}

			if( ISFLAG(obj_ptr->face[i].flag,FF_DBLSIDE) )
				ffprintf(f,"%s ", FS_DOUBLE );

			ffprintf(f,"%s   ", TexNameForNum( obj_ptr->face[i].texmapid ) );

			for( j=0; j<obj_ptr->face[i].nverts; j++ ) {
				if( tex->scaled ) {
					ffprintf(f,"%d ", obj_ptr->face[i].texvert[j][0] * texture_scale );
					ffprintf(f,"%d ", obj_ptr->face[i].texvert[j][1] * texture_scale );
				}
				else {
					ffprintf(f,"%d ", obj_ptr->face[i].texvert[j][0] );
					ffprintf(f,"%d ", obj_ptr->face[i].texvert[j][1] );
				}
			}
		}
		else
		if( ISFLAG(obj_ptr->face[i].flag,FF_SPRITE) ) {

			ffprintf(f," %s  %s", FS_SPRITE, TexNameForNum( obj_ptr->face[i].texmapid ) );

		}

		ffprintf(f,"\n");
	}

	ffprintf(f,"\n; [EOF]");

	ffclose(f);

	return TRUE;
}




//
//
//
BOOL LoadObject( char *name, object_ptr *o ) {

	FILE *file;
	char id[4];

	if( (file = ffopen(name,"rb")) == NULL ) {
		xprintf("LoadObject: can't open \"%s\" file.\n",name);
		return FALSE;
	}

	ffread(id,4,1,file);

	ffclose( file );

	if( IdBufStr( id, (char *)FIS_OBJ ) )
		return LoadBin( name, o );

	return LoadAsc( name, o );
}



//
//
//
BOOL SaveObject( object_ptr o ) {


	return SaveBin( o );
	//return SaveAsc( o );
}


//
// example for output wavefront-OBJ file.
//
BOOL SaveWavefront( object_ptr o ) {

/***
doc = MQSystem.getDocument()

num = doc.numObject
for n in range(0,num):
	obj = doc.object[n]
	if obj is None: continue
	numVert = obj.numVertex
	for x in range(0,numVert):
		v = "v"
		v = v + " %(#).3f" % {"#":obj.vertex[x].pos.x}
		v = v + " %(#).3f" % {"#":obj.vertex[x].pos.y}
		v = v + " %(#).3f" % {"#":obj.vertex[x].pos.z}
		MQSystem.println(v)
	numFace = obj.numFace
	for x in range(0,numFace):
		if obj.face[x].numVertex == 0: continue
		f = "f"
		for y in range(0,obj.face[x].numVertex):
			f = f + " " + `obj.face[x].index[y]+1`
		MQSystem.println(f)
***/

	return TRUE;
}




//
//
//
BOOL DiscardObject( object_ptr *obj_ptr ) {

	if( *obj_ptr ) {

		xprintf("DiscardObject: unloading \"%s\" ...\n",(*obj_ptr)->name );

		FREEMEM( (*obj_ptr)->name );

		FREEMEM( (*obj_ptr)->point );
		FREEMEM( (*obj_ptr)->tpoint );
		FREEMEM( (*obj_ptr)->ppoint );
		FREEMEM( (*obj_ptr)->pointnormal );
		FREEMEM( (*obj_ptr)->xform_flag );

		// for( int i=0; i<(*obj_ptr)->nfaces; i++ )
		//	DISCARDMEM( (*obj_ptr)->face[i].memory );

		FREEMEM( (*obj_ptr)->face );

#ifdef INCCOLDET
		if( (*obj_ptr)->model ) delete (*obj_ptr)->model;
#endif

		FREEMEM( *obj_ptr );
	}

	return TRUE;
}







//
// valami.anim
//
BOOL LoadAnimObject( char *filename, animobject_ptr *animobj ) {

	if( filename == NULL || animobj == NULL )
		return FALSE;

#define ANIMOBJ (*animobj)

	xprintf("LoadAnimObject: loading \"%s\" ...\n", filename );

	FILE *f;

	if((f=ffopen(filename,"rt"))==NULL)
		return FALSE;

	ALLOCMEM( ANIMOBJ, sizeof(animobject_t) );
	memset( ANIMOBJ, 0L, sizeof(animobject_t) );

	GetIntTxt( f, &ANIMOBJ->numobj );
	GetIntTxt( f, &ANIMOBJ->delay );

	strcpy( ANIMOBJ->filename, filename );

	ALLOCMEM( ANIMOBJ->objs, sizeof(object_t*) * ANIMOBJ->numobj );

	for( int i=0; i<ANIMOBJ->numobj; i++ ) {
		char name[XMAX_PATH];
		GetStringTxt( f, name );
		if( LoadAsc( name, &ANIMOBJ->objs[i] ) == FALSE ) {
			ANIMOBJ->numobj = i;
			ffclose(f);
			return FALSE;
		}
	}

	ffclose(f);

	ZEROVECTOR( ANIMOBJ->pos );

	ANIMOBJ->tic = -1;

	xprintf("LoadAnimObject: done.\n");

	return TRUE;
}


//
//
//
void DiscardAnimObject( animobject_ptr *animobj ) {

	if( animobj == NULL || ANIMOBJ == NULL )
		return;

	for( int i=0; i<ANIMOBJ->numobj; i++ )
		DiscardObject( &ANIMOBJ->objs[i] );

	ANIMOBJ->numobj = 0;

	FREEMEM( ANIMOBJ->objs );
	ANIMOBJ->objs = NULL;

	FREEMEM( ANIMOBJ );

	return;
}



//
//
//
BOOL CreateGridObj( object_ptr *obj_ptr, int width, int height, int texmapid ) {

	xprintf("CreateGridObj: is under construction!\n");

	if( height == -1 )
		height = width;

	if( texmapid == -1 )
		texmapid = GetFirstTexNum();

	CLAMPMIN( width, 1 );
	CLAMPMIN( height, 1 );

#define O(x) ((*obj_ptr)->x)

	*obj_ptr = NULL;
	ALLOCMEM( *obj_ptr, sizeof(object_t) );

	char *obj_name = "GRID_OBJECT1";

	ALLOCMEM( O(name), (strlen(obj_name) + 1) );
	strcpy( O(name), obj_name );

	O(npoints) = (width + 1) * (height + 1);
	O(nfaces) = 2 * (width * height);

	FLOAT dist = 1.0f;

	return TRUE;
}
