/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <io.h>

#include <xlib.h>

RCSID( "$Id: segshell.c,v 1.1 97-03-10 17:35:10 bernie Exp $" )


typedef struct segfile_s {

	char name[PATH_MAX];

	FILE *fp;

	ULONG start,offset;
	LONG len;
	ULONG current_len;

	ULONG temp1,temp2,temp3;

} segfile_t,*segfile_ptr;


static int nsegfiles = 0L;
static segfile_ptr segfile;


#define MAXFILENAMELEN 256


typedef struct lump_s {

	char name[MAXFILENAMELEN];

	FILE *fp;

	ULONG start,len;
	ULONG time;

	int seg;		// segfile[seg]-ben van benne

} lump_t, *lump_ptr;


static int nlumps = 0L;
static lump_t *lump;


#define ADDLUMP(nam,f,star,le,tim,hand) {			\
	REALLOCMEM( lump, (nlumps+1)*sizeof(lump_t) );	\
	strcpy( lump[nlumps].name, (nam) );				\
	lump[nlumps].fp = (f);							\
	lump[nlumps].start = (star);					\
	lump[nlumps].len = (le);						\
	lump[nlumps].time = (tim);						\
	lump[nlumps].seg = (hand);						\
	++nlumps; }




//
// return:  (hand + 1)
//
BOOL OpenSegFile( char *name ) {

	segfile_t *s;
	FILE *f;
	char str[MAX_PATH];
	char id[ FI_SIZE ];
	int i,j;
	char lump_name[MAXFILENAMELEN];
	ULONG flen,time;
	int lz_len,textsize;
	UCHAR lz[512],*buf,*ptr;

	strcpy( str, name );

	if( (f = fopen(str,"rb")) == NULL ) {
		
		sprintf( str, "DATA\\%s", name );

		if( (f=fopen(str,"rb")) == NULL ) {
			xprintf( "OpenSegFile: can't add \"%s\" file.\n", name );
			return FALSE;		
		}
	}

	fread( id, FI_SIZE, 1, f );
	if( !IdBufStr( id, (char*)FIS_SEG ) ) {
		fclose( f );
		return FALSE;
	}

	// a segfile... feltoltese

	// xprintf("OpenSegFile: segfile \"%s\" ",name,nlumps-i);

	REALLOCMEM( segfile, (nsegfiles+1)*sizeof(segfile_t) );

	s = &segfile[nsegfiles];

	strcpy( s->name, str );
	s->fp		= f;
	s->len		= filelength( fileno(f) );
	s->start	= ftell( f );
	s->offset	= 0L;
	s->current_len	= 0L;


	// lumpok olvsasa

	i = nlumps;

	while( ftell(f) < s->len ) {

		textsize = fgetc( f );
		lz_len = fgetc( f );

		fread( lz, lz_len, 1, f );

		DecodeRaw( lz, textsize, PTR(buf) );

		ptr = &buf[0];

		j = 0;
		while( (lump_name[j++] = *ptr++) != 0 );

		memcpy(&flen,ptr,sizeof(ULONG));     // length
		ptr += sizeof(ULONG);
		memcpy(&time,ptr,sizeof(ULONG));     // time

		ADDLUMP(lump_name,f,ftell(f),flen,time,nsegfiles);

		fseek( f, flen, SEEK_CUR );

		FREEMEM( buf );
	}

	// xprintf( "with %d lumps added.\n", nlumps-i );

	fseek( f, s->start, SEEK_SET );

	return ++nsegfiles;			// a mutet sikerult
}



//
// csak az utolsot tudja
//
BOOL CloseSegFile( int hand ) {

	segfile_t *s;

	s = &segfile[hand];

	fclose( s->fp );

	// xprintf("name: \"%s\"\n",s->name);
	s->fp		= NULL;
	s->start	= 0L;
	s->len		= 0L;
	s->current_len	= 0L;
	s->offset	= 0L;

	--nsegfiles;
	REALLOCMEM( segfile, nsegfiles*sizeof(segfile_t) );

	return TRUE;
}




//
//
//
BOOL CloseAllSegFile(void) {

	int i,n;

	n = nsegfiles-1;

	for( i=n; i>=0; i-- )

		CloseSegFile( i );


	return TRUE;
}





//
//
//
FILE *SEG_OpenFile( char *name ) {

	BOOL found=FALSE;
	int i;

	found = FALSE;
	for( i=0; i<nlumps && !found; i++ ) {

		if( !stricmp( name, lump[i].name ) )
			found = TRUE;

		if( found ) {
			segfile[ lump[i].seg ].current_len = lump[i].len;
			fseek( lump[i].fp, lump[i].start, SEEK_SET );
			return lump[i].fp;
		}
	}

	return NULL;
}




//
//
//
BOOL SEG_FileExist( char *name ) {

	int i;

	for( i=0; i<nlumps; i++ )

		if( !stricmp(name,lump[i].name) )
			return TRUE;

	return FALSE;
}




//
//
//
ULONG SEG_FileDate( char *name ) {

	int i;

	for( i=0; i<nlumps; i++ )

		if( !stricmp(name,lump[i].name) )
			return lump[i].time;

	return 0L;
}





//
//
//
LONG SEG_FileSize( FILE *f ) {

	int i;

	for( i=0; i<nsegfiles; i++ )
		if( f == segfile[i].fp )
			return segfile[i].current_len;

	return -1;
}



//
//
//
BOOL SEG_IsFileInSeg( FILE *f ) {

	int i;

	for( i=0; i<nsegfiles; i++ )
		if( f == segfile[i].fp )
			return TRUE;

	return FALSE;
}



//
//
//
void SEG_CloseFile( FILE *f ) {

	USEPARAM(f);

	return;
}



static ULONG pointer = 0L;

//
//
//
BOOL SEG_FindFirst( char *name, ULONG *size, int hand ) {

	int i;

	if( hand >= nsegfiles ) return FALSE;

	for( i=0; i<nlumps; i++ )
		if( lump[i].seg == hand ) {
			strcpy( name, lump[i].name );
			memcpy( size, &lump[i].len, sizeof(ULONG) );
			pointer = i+1;
			return TRUE;
		}

	return FALSE;
}


//
//
//
BOOL SEG_FindNext( char *name, ULONG *size, int hand ) {

	int i;

	if( hand >= nsegfiles ) return FALSE;

	for( i=pointer; i<nlumps; i++ )
		if( lump[i].seg == hand ) {
			strcpy( name, lump[i].name );
			memcpy( size, &lump[i].len, sizeof(ULONG) );
			pointer = i+1;
			return TRUE;
		}

	return FALSE;
}





/****************************************
 * miscallenous segfile
 *
 */




// ----------------------------------------------
// ezek a rutinok fogjak krealni 
// kulonalloak!
//


static FILE *f;
static int seg_flag = NOFLAG;


//
//
//
void SegTomFlag( int flag ) {

	seg_flag = flag;

	return;
}

//
//
//
BOOL CreateSegFile( char *name ) {

	if( (f=fopen(name,"rb") ) != NULL ) {
		fclose( f );
		f = fopen(name,"ab");
	}
	else {
		if( (f=fopen(name,"wb") ) == NULL )
			return FALSE;
		fwrite( FIS_SEG, FI_SIZE, 1, f );
	}

	return TRUE;
}



//
// milyen nev, pointer, size
//
BOOL SEG_AddFile( char *name, UCHAR *ptr, ULONG size, ULONG time ) {

	int i,lz_len;
	UCHAR buf[512],*lz,*lzptr;

	if( seg_flag == LZAFLAG )
		size = EncodeLZA( (UCHAR *)ptr, size, PTR(lzptr) );

	i = strlen(name) + 1;

	strcpy( (char *)buf, name );
	memcpy( &buf[i], &size, sizeof(ULONG) );
	memcpy( &buf[i+4], &time, sizeof(ULONG) );

	lz_len = EncodeLZA( buf, i+8, PTR(lz) ) - 12;

	fseek(f,0,SEEK_END);

	fputc( i+8, f );
	fputc( lz_len, f );
	fwrite( &lz[12], lz_len, 1, f );

	FREEMEM(lz);

	if( seg_flag == LZAFLAG ) {
		fwrite( lzptr, size, 1, f );
		FREEMEM( lzptr );
	}
	else
		fwrite( ptr, size, 1, f );


	return TRUE;
}


//
//
//
void CloseCreatedSegFile( void ) {

	fclose( f );

	return;
}


