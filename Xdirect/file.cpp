/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#pragma warning(disable : 4996)

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <time.h>
#include <io.h>
#include <stdlib.h>
#include <stdarg.h>
#include <locale.h>

#include <xlib.h>

RCSID( "$Id: file.c,v 1.01 97-03-10 16:55:25 bernie Exp $" )

//#define DOOM
#include "disk.h"

static ULONG file_start = 0L;
static ULONG file_end  = 0L;

static BOOL tom_flag = LZAFLAG; 	// WriteFile-ban tomoritson-e?
#define MAXTOMFLAGSTACK 10
static int tom_flag_stack[MAXTOMFLAGSTACK];
static int tom_flag_sp = -1;


#define TRANSFER (1024*8)
static UCHAR *transfer=NULL;		// transfer buffer


static char *search_dir = NULL;  	// [ nsearchdirs * XMAX_PATH ];
static int nsearchdirs = 0;

#define SEARCHDIRLEN (XMAX_PATH + 1)


//
//
//
BOOL AddSearchDir( const char *dir ) {

	if( dir == NULL )
		return FALSE;

	if( (search_dir = (char *)realloc(search_dir, (nsearchdirs+1)*SEARCHDIRLEN)) == NULL ) {
		nsearchdirs = 0;
		return FALSE;
	}

	strncpy( &search_dir[ nsearchdirs*SEARCHDIRLEN ], dir, XMAX_PATH );

	return ++nsearchdirs;
}


//
//
//
int DumpSearchDir( void ) {

	int i;

	xprintf("DumpSearchDir: %d dir%s\n", nsearchdirs, (nsearchdirs>1)?"s:":":" );

	for( i=0; i<nsearchdirs; i++ )
		xprintf("%d. \"%s\"\n", i, &search_dir[ i * SEARCHDIRLEN ] );

	return nsearchdirs;
}



//
// helyek:
//	- a current dirben
//	- a search \DATA dirben
//	- .SEG fileokban
//	- .ZIP fileokban
//
FILE *XL_OpenFile( const char *name ) {

	static FILE *f;

	if( name == NULL )
		return NULL;

	if( (f=fopen(name,"rb")) == NULL ) {

		int i;
		char data_name[XMAX_PATH*2+1];

		for( i=0; i<nsearchdirs; i++ ) {

			snprintf( data_name, XMAX_PATH*2, "%s%s", &search_dir[i*SEARCHDIRLEN],name );

			if( (f=fopen(data_name,"rb")) != NULL )
				break;
		}

		if( f == NULL )
			if( (f = SEG_OpenFile((char *)name)) == NULL ) {
				if( (f = ZIP_OpenFile( (char *)name)) == NULL ) {
					if( (f = RAR_OpenFile( (char *)name)) == NULL ) {
						// xprintf("XL_OpenFile: can't open \"%s\" file. (%s)\n", name, strerror(errno) );
						return NULL;
					}
				}
			}
	}

	file_start = (ZIP_IsFileInZip( f ) || RAR_IsFileInRar( f )) ? 0 : ftell(f);
	file_end = file_start + FileSize(f);

	return f;
}







//
//
//
void XL_CloseFile( FILE *f ) {

	if( f == NULL )
		return;

	if( !SEG_IsFileInSeg( f ) && !ZIP_IsFileInZip( f ) && !RAR_IsFileInRar( f ) )
		fclose( f );

	return;
}








//
//
//
ULONG LoadFile( const char *name, memptr_ptr ptr ) {

	FILE *f;
	ULONG len;
	UCHAR *buf;

	if( (f = XL_OpenFile( name )) == NULL )
		return 0L;

	if( (len = FileSize( f )) == 0 )
		return 0L;

	// xprintf("%s: %ld bytes\n",name, len);

	if( !AllocMem( PTR(buf), len ) ) {
		XL_CloseFile( f );
		return 0L;
	}

	Read( buf, len, f );

	L1:

	if( IsZIP( (char *)buf, len ) ) {

		if( (len = ZIP_Uncompress( buf, ptr )) == FALSE ) {
			FREEMEM( buf );
			XL_CloseFile( f );
			return 0L;
		}

		FREEMEM( buf );
		ChangeMemBlockPtr( ptr, PTR(buf) );

		goto L1;
	}
	else
	if( IsLZA( (char *)buf, len ) ) {

		if( (len = DecodeLZA( buf, ptr )) == FALSE ) {
			FREEMEM( buf );
			XL_CloseFile( f );
			return 0L;
		}

		FREEMEM( buf );
		ChangeMemBlockPtr( ptr, PTR(buf) );

		goto L1;
	}
	else
	if( IsLZH( (char *)buf, 0 ) ) {

		if( (len = DecodeLZH( buf, ptr )) == FALSE ) {
			FREEMEM( buf );
			XL_CloseFile( f );
			return 0L;
		}

		FREEMEM( buf );
		ChangeMemBlockPtr( ptr, PTR(buf) );

		goto L1;
	}
	else
	if( IsLZS( (char *)buf, 0 ) ) {

		if( (len = DecodeLZS( buf, ptr )) == FALSE ) {
			FREEMEM( buf );
			XL_CloseFile( f );
			return 0L;
		}

		FREEMEM( buf );
		ChangeMemBlockPtr( ptr, PTR(buf) );

		goto L1;
	}
	else
	if( IsRLE( (char *)buf, 0 ) ) {

		if( (len = DecodeRLE( buf, ptr )) == FALSE ) {
			FREEMEM( buf );
			XL_CloseFile( f );
			return 0L;
		}

		FREEMEM( buf );
		ChangeMemBlockPtr( ptr, PTR(buf) );

		goto L1;
	}
	else
	if( IsUU( (char *)buf, MIN(len, 6) ) ) {

		if( (len = DecodeUU( buf, len, ptr )) == FALSE ) {
			FREEMEM( buf );
			XL_CloseFile( f );
			return 0L;
		}

		FREEMEM( buf );
		ChangeMemBlockPtr( ptr, PTR(buf) );

		goto L1;
	}
	else
		ChangeMemBlockPtr( PTR(buf), ptr );

	XL_CloseFile(f);

	return len;
}



//
//
//
ULONG kitom( UCHAR *buf, int len, memptr_ptr ptr ) {

	     if( IsLZA( (char *)buf, len ) ) return DecodeLZA( buf, ptr );
	else if( IsZIP( (char *)buf, len ) ) return ZIP_Uncompress( buf, ptr );
	else if( IsLZH( (char *)buf, len ) ) return DecodeLZH( buf, ptr );
	else if( IsLZS( (char *)buf, len ) ) return DecodeLZS( buf, ptr );
	else if( IsRLE( (char *)buf, len ) ) return DecodeRLE( buf, ptr );
	else if( IsUU ( (char *)buf, len ) ) return DecodeUU( buf, len, ptr );

	return 0L;
}




//
//
//
ULONG FileDate( char *name ) {

	struct stat buf;

	stat( name, &buf );

	return (ULONG)( buf.st_mtime );
}



//
//
//
BOOL SetFileDate( char *name, ULONG date_time ) {

	struct utimbuf t;

	t.actime = date_time;
	t.modtime = date_time;

	utime( name, &t );

	return TRUE;
}



//
//
//
BOOL UnlinkFile( char *name ) {

	return XLIB_winDeleteFile( name );
}



//
//
//
BOOL TomFlag( int flag ) {

	tom_flag = flag;

	return tom_flag;
}




//
//
//
void PushTomFlag( void ) {

	if( tom_flag_sp < (MAXTOMFLAGSTACK-1) )
		++tom_flag_sp;
	else
		xprintf("PushTomFlag: tom_flag_stack overflow.\n");

	tom_flag_stack[ tom_flag_sp ] = tom_flag;

	return;
}



//
//
//
void PopTomFlag( void ) {

	if( tom_flag_sp > -1 )
		tom_flag = tom_flag_stack[ tom_flag_sp-- ];

	return;
}






//
//
//
ULONG XL_WriteFile( char *name, void *param_b, ULONG len ) {

	FILE *f = NULL;
	UCHAR *buf = NULL;
	UCHAR *b = (UCHAR *)param_b;

	if( name == NULL || param_b == NULL || len == 0 )
		return 0L;

	switch( tom_flag ) {

		case LZAFLAG:

#if 0

			UCHAR *buf1 = NULL

			temp = EncodeRLE( b, len, PTR(buf1) );
			if( temp < len ) {
				/* xprintf("YES: %d -> %d\n", len, temp ); */
				len = Encode( buf1, temp, PTR(buf) );
			}
			else {
				/* xprintf("NO: %d -> %d\n", len, temp ); */
				len = Encode( b, len, PTR(buf) );
			}
			FREEMEM( buf1 );

#else

			len = EncodeLZA( b, len, PTR(buf) );

#endif

			break;


		case LZHFLAG:

			len = EncodeLZH( b, len, PTR(buf) );

			break;


		case LZSFLAG:

			len = EncodeLZS( b, len, PTR(buf) );

			break;


		case RLEFLAG:

			len = EncodeRLE( b, len, PTR(buf) );

			break;


		case UUFLAG:

			len = EncodeUU( name, b, len, PTR(buf) );

			break;


		case ZIPFLAG:

			len = ZIP_Compress( b, len, PTR(buf) );

			break;


		case NOFLAG:
		default:

			buf = b;

			break;
	}

	if( (f=fopen(name,"wb")) == NULL )
		return 0L;

	Write( buf, len, f );

	fclose(f);

	if( buf != b )
		FREEMEM( buf );

	return len;
}






static int numentries = 0;
static char *entries = NULL;		// az elsõ entry a pattern lesz
static int entry_index = -1;

#define SUBDIRPATTERN	((char *)&entries[0*ENTRYLEN])
#define INITDIR  	((char *)&entries[1*ENTRYLEN])
#define TEMPSPACE  	((char *)&entries[2*ENTRYLEN])
#define DIRFLAGS	(*(int *)&entries[3*ENTRYLEN+0])
#define ENTRYSORT	(*(int *)&entries[3*ENTRYLEN+sizeof(int)])

#define ENTRYLEN 	(XMAX_PATH + 1 + 9*sizeof(int))
#define DATAOFFSET 	(XMAX_PATH + 1)

// pattern = XMAX_PATH+1
// init_dir = XMAX_PATH+1
// flags = XMAX_PATH+1

// pattern + init_dir + flags
#define NUMINFOS 4

// .zip fájlokhoz
static BOOL bZip = FALSE;
static BOOL scanZip = FALSE;


//
// ext: ".pcx" vagy "pcx"
// pattern: "*.jpg;*.jpeg;*.gif;*.pcx;*.bmp;*.ico;*."
// "*." = extension nélküli file
//
BOOL cmpPattern( char *ext, char *pattern ) {

	// NULL check vagy "" üres string
	if( ext == NULL || pattern == NULL || pattern[0] == 0 )
		return FALSE;

	char full_ext[XMAX_PATH];

	if( ext[0] != '.' )
		sprintf( full_ext, ".%s", ext );
	else
		strcpy( full_ext, ext );

	int full_ext_len = strlen( full_ext );

	char *p = pattern;

	while( *p != 0 ) {

		while( *p != '.' ) {
			++p;
			if( *p == 0 && full_ext_len )		// az ext nélküli is jó legyen később
				return FALSE;
		}

		int i = 0;
		char ext[XMAX_PATH];
		ext[i++] = *p++;		// "."

		while( *p != ';' && *p != '*' && *p != '.' && *p != 0  )
			ext[i++] = *p++;

		ext[i] = 0;

		if( full_ext_len != i )
			continue;

		if( !stricmp( full_ext, ext ) )
			return TRUE;
	}

	return FALSE;
}






//
// .ZIP-et becachel
// "+valami.zip" akkor hozzáadja a zipekhez
//
static BOOL ZIP_CacheSubdir( char *filename, char *pattern, BOOL nosubdir ) {

	// char *ext;

	// más extension is van
	if( !filename /*|| !(ext = GetExtension(filename)) || (stricmp(ext,"zip") && stricmp(ext,"rar"))*/ )
		return FALSE;

	// xprintf( "ZIP_CacheSubdir: filename '%s'\n", filename );
	// xprintf( "ZIP_CacheSubdir: pattern '%s'\n", pattern );
	// xprintf( "ZIP_CacheSubdir: %d\n", numentries );

	int ziphandle;
	int rarhandle;

	BOOL addZip = FALSE;

	if( filename[0] == '+' ) {
		++filename;
		addZip = TRUE;
	}

	if( (ziphandle = OpenZipFile( filename )-1) < 0  &&
	    (rarhandle = OpenRarFile( filename )-1) < 0 )
		return FALSE;

	if( entries == NULL )
		addZip = FALSE;

	if( addZip == FALSE ) {

		numentries = 0;

		if( entries ) FREEMEM( entries );
		entries = NULL;

		bZip = FALSE;

		REALLOCMEM( entries, NUMINFOS*ENTRYLEN );

		strcpy( INITDIR, filename );
		strcpy( TEMPSPACE, "(archive file)" );

		DIRFLAGS = TRUE;
		ENTRYSORT = SORT_NOTSORTED;
		entry_index = 0;

		if( pattern && !stristr(pattern, "*.*") )
			// ha van pattern de nem *.*
			strcpy( SUBDIRPATTERN, pattern );
		else
			SUBDIRPATTERN[0] = 0;
	}

	char name[XMAX_PATH];

	int old_numentries = numentries;
	int alloced = numentries+NUMINFOS;
	int size;
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int milliseconds;

	while( 1 ) {

		BOOL res;

		if( ziphandle >= 0 ) res = ZIP_GetNextFilename( name, ziphandle, FALSE, &size, &year, &month, &day, &hour, &minute, &second, &milliseconds );
		else		     res = RAR_GetNextFilename( name, rarhandle, FALSE, &size, &year, &month, &day, &hour, &minute, &second, &milliseconds );

		if( res == FALSE )
			break;

		if( pattern && pattern[0] != 0 ) {

			char *p;

			if( (p = GetExtension( name )) == NULL )
				continue;

			if( cmpPattern( p, pattern ) == FALSE )
				continue;
		}

		if( nosubdir && !GetFilePath(name, TRUE) )
			continue;

		// a nevek
		if( (NUMINFOS+numentries+1) >= alloced ) {
			alloced += 10000;
			REALLOCMEM( entries, alloced * ENTRYLEN )
		}

		strcpy( &entries[ (NUMINFOS+numentries) * ENTRYLEN ], name );

		int *p = (int *)&entries[ (NUMINFOS+numentries) * ENTRYLEN + DATAOFFSET ];

		p[0] = size;
		p[1] = year;
		p[2] = month;
		p[3] = day;
		p[4] = hour;
		p[5] = minute;
		p[6] = second;
		p[7] = milliseconds;
		p[8] = FALSE;

		// xprintf("%d: %s\n", numentries, &entries[ (NUMINFOS+numentries) * ENTRYLEN ] );

		++numentries;
	}

	bZip = TRUE;

	xprintf( "ZIP_CacheSubdir: added %d entries.\n", numentries - old_numentries );

	return TRUE;
}



//
// filename: "valami\dir\valahol.ext"
//
static void AddEntry( char *filename, int size,
				      int year,
				      int month,
				      int day,
				      int hour,
				      int minute,
				      int second,
				      int milliseconds ) {

	// xprintf("AddEntry: '%s'\n", filename);

	char *ext = GetExtension( filename );

	// és ha .zip fájl?
	if( scanZip && ext && (!stricmp( ext, "zip" ) || !stricmp( ext, "rar" )) ) {
		char str[XMAX_PATH + 1];
		sprintf( str, "+%s", filename );
		// xprintf("scanZip: '%s'\n", str);
		char pattern[XMAX_PATH];
		strcpy( pattern, &SUBDIRPATTERN[0] );
		ZIP_CacheSubdir( str, pattern, FALSE );
		return;
	}

	if( SUBDIRPATTERN[0] != 0 ) {

		if( ext == NULL )
			return;

		if( cmpPattern( ext, SUBDIRPATTERN ) == FALSE )
			return;
	}

	// a nevek
	REALLOCMEM( entries, (NUMINFOS+numentries+1) * ENTRYLEN )

	strcpy( &entries[ (NUMINFOS+numentries) * ENTRYLEN ], filename );

	int *p = (int *)&entries[ (NUMINFOS+numentries) * ENTRYLEN + DATAOFFSET ];

	// WoWScrnShot_123106_090219.jpg
	// ha WOW screenshot:
	char *bare;
	if( ext && !stricmp( ext, "jpg" ) &&
	    (bare = GetFilenameNoPath( filename )) &&
	    (strlen(bare) == 29) &&
	    !strncmp(bare,"WoWScrnShot_",12) ) {

		char str[3];
		str[2] = 0;

		str[0] = bare[12];
		str[1] = bare[13];
		month = atoi(str);

		str[0] = bare[14];
		str[1] = bare[15];
		day = atoi(str);

		str[0] = bare[16];
		str[1] = bare[17];
		year = 2000 + atoi(str);

		str[0] = bare[19];
		str[1] = bare[20];
		hour = atoi(str);

		str[0] = bare[21];
		str[1] = bare[22];
		minute = atoi(str);

		str[0] = bare[23];
		str[1] = bare[24];
		second = atoi(str);

		// xprintf("%s: %d. %d %d.  %2d:%02d:%02d\n", bare, year, month, day, hour, minute, second );
	}

	p[0] = size;
	p[1] = year;
	p[2] = month;
	p[3] = day;
	p[4] = hour;
	p[5] = minute;
	p[6] = second;
	p[7] = milliseconds;
	p[8] = FALSE;

	/***
	xprintf( "AddEntry: pattern %s\n", SUBDIRPATTERN );
	xprintf( "AddEntry: init_dir %s\n", INITDIR );
	xprintf( "AddEntry: nosubdir = %d\n", DIRFLAGS );
	xprintf("AddEntry: %s\n", &entries[ (NUMINFOS+numentries) * ENTRYLEN ] );
	***/

	++numentries;

	return;
}





//
//
//
static int sort_name_cmp( const void *a, const void *b ) {

	return stricmp( GetFilenameNoPath((char *)a), GetFilenameNoPath((char *)b) );
}




//
//
//
static int sort_name_dec_cmp( const void *a, const void *b ) {

	int rez = stricmp( GetFilenameNoPath((char *)a), GetFilenameNoPath((char *)b) );

	if( rez == -1 ) return 1;
	if( rez == 1 ) return -1;

	return 0;
}



//
//
//
static int sort_ext_cmp( const void *a, const void *b ) {

	char *p1 = GetExtension( (char *)a );
	char *p2 = GetExtension( (char *)b );

	if( !p1 ) return -1;
	if( !p2 ) return 1;

	int rez = stricmp( p1, p2 );

	// xprintf("sort_ext_cmp = %d: %s, %s\n", rez, p1, p2 );

	if( !rez )
		return stricmp( GetFilenameNoPath((char *)a), GetFilenameNoPath((char *)b) );

	return rez;
}



//
//
//
static int sort_ext_dec_cmp( const void *a, const void *b ) {

	char *p1 = GetExtension( (char *)a );
	char *p2 = GetExtension( (char *)b );

	if( !p1 ) return 1;
	if( !p2 ) return -1;

	int rez = stricmp( p1, p2 );

	// xprintf("sort_ext_cmp = %d: %s, %s\n", rez, p1, p2 );

	if( !rez ) {
		rez = stricmp( GetFilenameNoPath((char *)a), GetFilenameNoPath((char *)b) );
		if( rez == -1 ) return 1;
		if( rez == 1 ) return -1;
	}

	return rez;
}



//
//
//
static int sort_date_cmp( const void *a, const void *b ) {

	char *s = (char *)a;

	int *p = (int *)&s[ DATAOFFSET ];

	int year1 = p[1];
	int month1 = p[2];
	int day1 = p[3];
	int hour1 = p[4];
	int minute1 = p[5];
	int second1 = p[6];
	int milliseconds1 = p[7];

	s = (char *)b;

	p = (int *)&s[ DATAOFFSET ];

	int year2 = p[1];
	int month2 = p[2];
	int day2 = p[3];
	int hour2 = p[4];
	int minute2 = p[5];
	int second2 = p[6];
	int milliseconds2 = p[7];

	if( year1 < year2 ) return -1;
	if( year1 > year2 ) return 1;

	if( month1 < month2 ) return -1;
	if( month1 > month2 ) return 1;

	if( day1 < day2 ) return -1;
	if( day1 > day2 ) return 1;

	if( hour1 < hour2 ) return -1;
	if( hour1 > hour2 ) return 1;

	if( minute1 < minute2 ) return -1;
	if( minute1 > minute2 ) return 1;

	if( second1 < second2 ) return -1;
	if( second1 > second2 ) return 1;

	if( milliseconds1 < milliseconds2 ) return -1;
	if( milliseconds1 > milliseconds2 ) return 1;

	return 0;
}


//
//
//
static int sort_date_dec_cmp( const void *a, const void *b ) {

	char *s = (char *)a;

	int *p = (int *)&s[ DATAOFFSET ];

	int year1 = p[1];
	int month1 = p[2];
	int day1 = p[3];
	int hour1 = p[4];
	int minute1 = p[5];
	int second1 = p[6];
	int milliseconds1 = p[7];

	s = (char *)b;

	p = (int *)&s[ DATAOFFSET ];

	int year2 = p[1];
	int month2 = p[2];
	int day2 = p[3];
	int hour2 = p[4];
	int minute2 = p[5];
	int second2 = p[6];
	int milliseconds2 = p[7];

	if( year1 < year2 ) return 1;
	if( year1 > year2 ) return -1;

	if( month1 < month2 ) return 1;
	if( month1 > month2 ) return -1;

	if( day1 < day2 ) return 1;
	if( day1 > day2 ) return -1;

	if( hour1 < hour2 ) return 1;
	if( hour1 > hour2 ) return -1;

	if( minute1 < minute2 ) return 1;
	if( minute1 > minute2 ) return -1;

	if( second1 < second2 ) return 1;
	if( second1 > second2 ) return -1;

	if( milliseconds1 < milliseconds2 ) return 1;
	if( milliseconds1 > milliseconds2 ) return -1;

	return 0;
}




//
//
//
static int sort_size_cmp( const void *a, const void *b ) {

	char *s = (char *)a;

	int *p = (int *)&s[ DATAOFFSET ];

	int size1 = p[0];

	s = (char *)b;

	p = (int *)&s[ DATAOFFSET ];

	int size2 = p[0];

	// xprintf("sort_size_cmp: %dkb, %dkb\n", size1/1024, size2/1024 );

	if( size1 < size2 ) return -1;
	if( size1 > size2 ) return 1;

	return 0;
}



//
//
//
static int sort_size_dec_cmp( const void *a, const void *b ) {

	char *s = (char *)a;

	int *p = (int *)&s[ DATAOFFSET ];

	int size1 = p[0];

	s = (char *)b;

	p = (int *)&s[ DATAOFFSET ];

	int size2 = p[0];

	// xprintf("sort_size_cmp: %dkb, %dkb\n", size1/1024, size2/1024 );

	if( size1 < size2 ) return 1;
	if( size1 > size2 ) return -1;

	return 0;
}



//
// directory szerint sorba teszi
//
static int sort_dir_cmp( const void *a, const void *b ) {

	char s1[XMAX_PATH];
	char s2[XMAX_PATH];

	strcpy( s1, GetFilePath((char *)a) );
	strcpy( s2, GetFilePath((char *)b) );

	if( strlen(s1) == strlen(s2) )
		return stricmp(s1,s2);

	return strlen(s1) - strlen(s2);
}



//
// directoryn belül sorba teszi ahogy volt
//
static int sort_indir_cmp( const void *a, const void *b ) {

	sort_e sort = (sort_e)ENTRYSORT;

	if( sort == SORT_NAME )
		return sort_name_cmp(a,b);

	if( sort == SORT_NAME_DEC )
		return sort_name_dec_cmp(a,b);

	if( sort == SORT_DATE )
		return sort_date_dec_cmp(a,b);

	if( sort == SORT_DATE_DEC  )
		return sort_date_cmp(a,b);

	if( sort == SORT_EXT  )
		return sort_ext_cmp(a,b);

	if( sort == SORT_EXT_DEC  )
		return sort_ext_dec_cmp(a,b);

	if( sort == SORT_SIZE  )
		return sort_size_dec_cmp(a,b);

	if( sort == SORT_SIZE_DEC  )
		return sort_size_cmp(a,b);

	// randomize
	if( sort == SORT_RANDOM )
		return 0;

	return 0;
}





//
// sort it
//
static void sortHelper( sort_e sort ) {

	if( sort == SORT_NAME   )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_name_cmp );

	if( sort == SORT_NAME_DEC  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_name_dec_cmp );

	if( sort == SORT_DATE  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_date_dec_cmp );

	if( sort == SORT_DATE_DEC  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_date_cmp );

	if( sort == SORT_EXT  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_ext_cmp );

	if( sort == SORT_EXT_DEC  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_ext_dec_cmp );

	if( sort == SORT_SIZE  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_size_dec_cmp );

	if( sort == SORT_SIZE_DEC  )
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_size_cmp );

	// randomize
	if( sort == SORT_RANDOM ) {

		xprintf("sortHelper: randomized.\n");

		// Fisher-Yates shuffle algorithm x3
		for( int i=0; i<3; i++ )
			for( int n=0; n<numentries; n++ ) {
				int k = rand() % numentries;
				char temp[ENTRYLEN];
				memcpy( temp, &entries[ (NUMINFOS + n) * ENTRYLEN ], ENTRYLEN );
				memmove( &entries[ (NUMINFOS + n) * ENTRYLEN ], &entries[ (NUMINFOS + k) * ENTRYLEN ], ENTRYLEN );
				memcpy( &entries[ (NUMINFOS + k) * ENTRYLEN ], temp, ENTRYLEN );
			}

		// reset a random flagekre
		for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {
			int *p = (int *)&entries[ i * ENTRYLEN + DATAOFFSET ];
			p[8] = FALSE;
		}
	}

	return;
}




//
//
//
static BOOL isPic( char *filename ) {

	char *pic_pattern = "*.jpeg;*.jpg;*.bmp;*.ico;*.png;*.gif;";
	char str[XMAX_PATH];
	char *p;

	strcpy( str, filename );
	strlwr( str );

	if( (p = GetExtension( str )) == NULL )
		return FALSE;

	if( cmpPattern( p, pic_pattern ) == TRUE )
		return TRUE;

	return FALSE;
}




//
//
//
static BOOL dirDump( void ) {

	FILE *f;
	char dumpname[XMAX_PATH];
	char currdir[XMAX_PATH];
	int idx = 0;

	if( !numentries )
		return FALSE;

	TomFlag( NOFLAG );

	if( DIRFLAGS == FALSE ) {

		// le kell sortolni dir szerint
		qsort( &entries[NUMINFOS*ENTRYLEN], numentries, ENTRYLEN, sort_dir_cmp );

		// aztán diren belül ahogy volt

		int start = 0;
		strcpy( currdir, GetFilePath( &entries[ (NUMINFOS + start) * ENTRYLEN ] ) );

		for( int i=start+1; i<numentries; i++ ) {

			char newdir[XMAX_PATH];
			strcpy( newdir, GetFilePath( &entries[ (NUMINFOS + i) * ENTRYLEN ] ) );

			if( stricmp( currdir, newdir ) ) {

				// xprintf( "currdir: %s\n", currdir );
				// xprintf( "newdir: %s\n", newdir );
				// xprintf( "start: %d, num: %d\n", start, i-start );

				qsort( &entries[(NUMINFOS+start)*ENTRYLEN], i-start, ENTRYLEN, sort_indir_cmp );

				strcpy( currdir, newdir );
				start = i;
			}
		}

		// az ucsó dirt is
		// xprintf( "start: %d, num: %d\n", start, numentries-start );
		qsort( &entries[(NUMINFOS+start)*ENTRYLEN], numentries-start, ENTRYLEN, sort_indir_cmp );
	}

	char cachedir[XMAX_PATH];
	{
	unsigned int crc = 0;
	unsigned int sum = 0;
	unsigned int len = ENTRYLEN;

	char tmpdir[XMAX_PATH];
	// strcpy( tmpdir, GetRealTemp() );
	getcwd( tmpdir, XMAX_PATH );

	char name[XMAX_PATH];
	strcpy( name, INITDIR );
	strlwr( name );

	CalcCrc( name, strlen(name), &crc, &sum );

	sprintf(cachedir,"%s%cthumbnails.%u%c", tmpdir, PATHDELIM, crc, PATHDELIM );
	CreatePath( cachedir );
	}

	int num=0;
	while( num < numentries ) {

		sprintf( dumpname, "dir-dump%02d.html", idx );

		if( (f = ffopen(dumpname, "wt")) == NULL ) {
			xprintf("dirDump: can't open %s file.\n", dumpname );
			return FALSE;
		}

		ffprintf( f, "<html>\n" );
		ffprintf( f, "<head>\n" );
		ffprintf( f, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n" );
		ffprintf( f, "<title>Dirdump%02d - Sort by %s</title>\n",idx, ENTRYSORT==SORT_EXT?"Extension":ENTRYSORT==SORT_EXT_DEC?"Externsion decrement":ENTRYSORT==SORT_NAME?"Name":ENTRYSORT==SORT_NAME_DEC?"Name decrement":ENTRYSORT==SORT_DATE?"Date":ENTRYSORT==SORT_DATE_DEC?"Date decrement":ENTRYSORT==SORT_SIZE?"Size":ENTRYSORT==SORT_SIZE_DEC?"Size decrement":"Random" );
		ffprintf( f, "<style type=\"text/css\">\n" );
		ffprintf( f, "BODY { MARGIN-TOP: 10px; FONT-SIZE: 11px; MARGIN-LEFT: 20px; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif; BACKGROUND-COLOR: #D3D3D3 }\n" );
		ffprintf( f, "TD { FONT-SIZE: 11px; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif }\n" );
		ffprintf( f, "P { BACKGROUND: #D3D3D3 }\n" );
		ffprintf( f, "H1 { FONT-SIZE: 12px; COLOR: #979797; FONT-FAMILY: Helvetica, Verdana, Arial }\n" );
		ffprintf( f, "A:hover { COLOR: #804c4c; BACKGROUND-COLOR: #f0efe3 }\n" );
		ffprintf( f, "H2 { FONT-SIZE: 10px; COLOR: #000000; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif }\n" );
		ffprintf( f, "H3 { FONT-SIZE: 18px; COLOR: #000000; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif }\n" );
		ffprintf( f, ".textsm { FONT-SIZE: 14px; COLOR: #000000; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif }\n" );
		ffprintf( f, ".textbg { FONT-SIZE: 20px; COLOR: #000000; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif }\n" );
		ffprintf( f, ".textreg { FONT-SIZE: 12px; COLOR: #666666; FONT-FAMILY: Verdana, Arial, Helvetica, sans-serif }\n" );
		ffprintf( f, "</style>\n" );
		ffprintf( f, "</head>\n" );
		ffprintf( f, "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n" );

		ffprintf( f, "<span class=\"textreg\">pattern: %s</span><br>\n", SUBDIRPATTERN );
		ffprintf( f, "<span class=\"textreg\">init_dir: %s</span><br>\n", INITDIR );
		if( DIRFLAGS == FALSE )
			ffprintf( f, "<span class=\"textreg\">subdir: %s</span><br>\n", DIRFLAGS?"False":"True" );
		ffprintf( f, "<span class=\"textreg\">Sort by %s.</span><br>\n", ENTRYSORT==SORT_EXT?"Extension":ENTRYSORT==SORT_EXT_DEC?"Externsion decrement":ENTRYSORT==SORT_NAME?"Name":ENTRYSORT==SORT_NAME_DEC?"Name decrement":ENTRYSORT==SORT_DATE?"Date":ENTRYSORT==SORT_DATE_DEC?"Date decrement":ENTRYSORT==SORT_SIZE?"Size":ENTRYSORT==SORT_SIZE_DEC?"Size decrement":"Random" );
		ffprintf( f, "<span class=\"textreg\">index (%d / %d)</span><br>\n", entry_index+1, numentries, num+1 );

		ffprintf( f, "<p><span class=\"textreg\">\n" );

		if( idx > 0 )
			ffprintf( f, "<a href=\"dir-dump%02d.html\">Prev</a> |\n", idx-1 );
		else
			ffprintf( f, "Prev |\n");

		ffprintf( f, "<a href=\"dir-dump%02d.html\">Next</a>\n", idx+1 );
		ffprintf( f, "</span>\n" );
		ffprintf( f, "<hr size=\"1\">\n" );

		currdir[0] = 0;

		int cnt = 0;
		int start = num;
		int area = 0;
		while( (area/(1000*1000) < 200) &&  num < numentries ) {

			int i = NUMINFOS + num;

			// xprintf("\n%s: ", &entries[ i * ENTRYLEN ] );

			// int *p = (int *)&entries[ i * ENTRYLEN + DATAOFFSET ];
			// ffprintf( f,"[%d]: \"%s\" %dkb  (%d. %d. %d) %d:%02d\n", i-NUMINFOS+1, &entries[ i * ENTRYLEN ], p[0]/1024, p[1], p[2], p[3], p[4], p[5] );
			char *p = GetRelativeFilename( &entries[ i * ENTRYLEN ] );

			char *filepath = GetFilePath( &entries[ i * ENTRYLEN ] );

			if( stricmp( currdir, filepath ) && DIRFLAGS == FALSE ) {
				strcpy( currdir, filepath );
				ffprintf( f,"<p align=\"center\"><span class=\"textreg\">%s</span></p>\n", currdir );
			}

			unsigned int crc = 0;
			unsigned int sum = 0;
			unsigned int len = ENTRYLEN;

			char name[XMAX_PATH];
			strcpy( name, &entries[ i * ENTRYLEN ] );
			strlwr( name );

			CalcCrc( name, strlen(name), &crc, &sum );

			char cache[XMAX_PATH];
			sprintf(cache, "%s%u.jpg", cachedir, crc );

			int x=150,y=140;
			UCHAR *spr;

			// ha nincsen a thumbnailbe és be tudja tölteni
			if( FileExist(cache) == FALSE ) {

				if( LoadFree( &entries[ i * ENTRYLEN ], PTR(spr), &x, &y ) ) {

					// ha nagyobb lenne mint az igazi fájl
					if( (SPRITEW(spr) < 150) && (SPRITEH(spr) < 140) ) {

						// pcx-et nem tud a firefox
						if( isPic(&entries[ i * ENTRYLEN ]) == FALSE )
							SaveFree( cache, spr );
						else
							strcpy( cache, &entries[ i * ENTRYLEN ] );
					}
					else {
						if( x > y ) {
							if( x > 150 ) {
								y = ftoi( (FLOAT)y / ((FLOAT)x / 150.0f) );
								x = 150;
							}
						}
						else
							if( y > 140 ) {
								x = ftoi( (FLOAT)x / ((FLOAT)y / 140.0f) );
								y = 140;
							}

						CLAMPMIN( x, 1 );
						CLAMPMIN( y, 1 );

						UCHAR *scaled;

						ScaleSprite( x,y, spr, PTR(scaled) );
						SaveFree( cache, scaled );

						FREEMEM( scaled );

						// xprintf("scaled and cached, ");
					}

					FREEMEM( spr );
				}
				else
				if( GetDimImageMagick( &entries[ i * ENTRYLEN ], &x,&y ) == TRUE ) {

					// xprintf("im: %s: %d, %d\n", &entries[ i * ENTRYLEN ], w,h );

					if( (x < 150) && (y < 140) ) {

						// pcx-et nem tud a firefox
						if( isPic(&entries[ i * ENTRYLEN ]) == FALSE )
							ConvertImageMagick( &entries[ i * ENTRYLEN ], cache, x,y );
						else
							strcpy( cache, &entries[ i * ENTRYLEN ] );
					}
					else {

						if( x > y ) {
							if( x > 150 ) {
								y = ftoi( (FLOAT)y / ((FLOAT)x / 150.0f) );
								x = 150;
							}
						}
						else
							if( y > 140 ) {
								x = ftoi( (FLOAT)x / ((FLOAT)y / 140.0f) );
								y = 140;
							}

						CLAMPMIN( x, 1 );
						CLAMPMIN( y, 1 );

						ConvertImageMagick( &entries[ i * ENTRYLEN ], cache, x,y );
					}
				}
			}

			if( FileExist(cache) == TRUE ) {

				++cnt;

				int *d = (int *)&entries[ i * ENTRYLEN + DATAOFFSET ];

				/***
				p[0] = size;
				p[1] = year;
				p[2] = month;
				p[3] = day;
				p[4] = hour;
				p[5] = minute;
				p[6] = second;
				p[7] = milliseconds;
				p[8] = FALSE;
				***/

				ffprintf( f,"<a href=\"%s\"><img align=\"center\" src=\"file://%s\" title=\"%s (%d. %s %d)\" border=\"0\"></a>\n", p, cache, &entries[ i * ENTRYLEN ], d[1],MonthStr(d[2]-1),d[3] );

				area += x*y;
			}

			// xprintf("done.\n");

			++num;
		}

		ffprintf( f, "<p><span class=\"textreg\">\n" );

		if( idx > 0 )
			ffprintf( f, "<a href=\"dir-dump%02d.html\">Prev</a> |\n", idx-1 );
		else
			ffprintf( f, "Prev |\n" );

		if( num < numentries )
			ffprintf( f, "<a href=\"dir-dump%02d.html\">Next</a>\n", idx+1 );
		else
			ffprintf( f, "Next\n" );

		ffprintf( f, "</span>\n<br><br>\n" );

		ffprintf( f, "<span class=\"textreg\">%d Megapixel in %d thumbnails.</span><br><br>\n", area/(1000*1000), cnt /*num-start*/ );

		ffprintf( f, "</body>\n" );
		ffprintf( f, "</html>\n" );

		ffclose( f );

		++idx;

		// sleep(1);
	}

	// visszaállítja a sortot ha kell
	if( DIRFLAGS == FALSE )
		sortHelper( (sort_e)ENTRYSORT );

	xprintf("dirDump: wrote %d entries into %s file.\n", numentries, dumpname );

	return TRUE;
}



//
// ha NULL akkor eldobjas
// pattern: "*.cpp;*.h;*.c"
//	     "*.*"
//
BOOL CacheSubdir( char *filename, char *pattern, BOOL nosubdir, BOOL scan_zips ) {

	if( filename == NULL && pattern == NULL ) {

		if( bZip )
			return TRUE;

		numentries = 0;
		if( entries ) FREEMEM( entries );
		entries = NULL;
		entry_index = -1;
		scanZip = FALSE;
		xprintf( "CacheSubdir: cache cleared.\n" );
		return TRUE;
	}

	// csak kilistázza a konzolra
	if( filename != NULL && !stricmp( filename, "Power Word: List" ) && pattern == NULL ) {

		if( !numentries ) {
			xprintf( "CacheSubdir: no entries to list.\n" );
			return FALSE;
		}

		xprintf( "CacheSubdir: pattern %s\n", SUBDIRPATTERN );
		xprintf( "CacheSubdir: init_dir %s\n", INITDIR );
		xprintf( "CacheSubdir: temp %s\n", TEMPSPACE );
		xprintf( "CacheSubdir: nosubdir = %d\n", DIRFLAGS );
		xprintf( "CacheSubdir: sort = %d\n", ENTRYSORT );
		xprintf( "CacheSubdir: index (%d / %d):\n", entry_index+1, numentries );

		for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {
			int *p = (int *)&entries[ i * ENTRYLEN + DATAOFFSET ];
			xprintf( "[%d]: \"%s\" %dkb  (%d. %d. %d) %d:%02d\n", i-NUMINFOS+1, &entries[ i * ENTRYLEN ], p[0]/1024, p[1], p[2], p[3], p[4], p[5] );
		}

		return TRUE;
	}

	if( filename != NULL && !stricmp( filename, "Power Word: Dump" ) && pattern == NULL ) {

		if( !numentries ) {
			xprintf( "CacheSubdir: no entries to dump.\n" );
			return FALSE;
		}

		return dirDump();
	}

	// a zipeket elereszti
	if( filename != NULL && !stricmp( filename, "Power Word: Dismount" ) && pattern == NULL ) {

		bZip = FALSE;

		numentries = 0;
		if( entries ) FREEMEM( entries );
		entries = NULL;
		entry_index = -1;
		scanZip = FALSE;

		xprintf( "CacheSubdir: .zip cleared.\n" );

		return TRUE;
	}

	// csak kilistázza a konzolra
	if( filename != NULL && !stricmp( filename, "Power Word: Exist" ) && pattern == NULL ) {

		if( !numentries )
			return FALSE;

		return TRUE;
	}

	// .ZIP-et becachel
	// "+valami.zip" akkor hozzáadja a zipekhez
	if( filename && ((GetExtension( filename ) &&
	    ( !stricmp( GetExtension( filename ), "zip" ) ||
	      !stricmp( GetExtension( filename ), "rar" ) ) ) ||
	      IsZipFromName(filename) ||
	      IsRarFromName(filename) ) )  {
		return ZIP_CacheSubdir( filename, pattern, FALSE );
	}

	// kidobálja a fájlokat amik nem patternek
	if( filename != NULL && !stricmp( filename, "Power Word: Repattern" ) && pattern != NULL ) {

		if( !numentries ) {
			xprintf( "CacheSubdir: no directory to repattern.\n" );
			return FALSE;
		}

		xprintf( "CacheSubdir: repatterning...\n" );

		int old_numentries = numentries;

		strcpy( SUBDIRPATTERN, pattern );

		for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {

			char *p = &entries[ i * ENTRYLEN ];

			if( (p = GetExtension( p )) == NULL ) {
				// delete
				if( i < ((NUMINFOS+numentries)-1) ) {
					memmove( &entries[ i * ENTRYLEN ],
						 &entries[ (i + 1) * ENTRYLEN ],
						 ((NUMINFOS+numentries) - (i+1)) * ENTRYLEN );
					--i;
				}
				--numentries;
				continue;
			}

			if( cmpPattern( p, pattern ) == FALSE ) {
				// delete
				if( i < ((NUMINFOS+numentries)-1) ) {
					memmove( &entries[ i * ENTRYLEN ],
						 &entries[ (i + 1) * ENTRYLEN ],
						 ((NUMINFOS+numentries) - (i+1)) * ENTRYLEN );
					--i;
				}
				--numentries;
				continue;
			}
		}

		REALLOCMEM( entries, (NUMINFOS+numentries+1) * ENTRYLEN )

		xprintf( "CacheSubdir: %d thrown away.\n", old_numentries - numentries );

		return TRUE;
	}

	// nem múködik a cache újraolvasása amíg fel nem oldaj a lockot mint a Zip-nél
	if( filename != NULL && !stricmp( filename, "Power Word: Lock" ) && pattern == NULL )
		bZip = TRUE;


	// ami be van olvasva újra beolvassa
	if( filename != NULL && !stricmp( filename, "Power Word: Refresh" ) && pattern == NULL ) {

		if( !numentries ) {
			xprintf( "CacheSubdir: no directory to refresh.\n" );
			return FALSE;
		}

		xprintf( "CacheSubdir: refreshing...\n" );

		if( entry_index < numentries )
			strcpy( TEMPSPACE, &entries[ (NUMINFOS + entry_index) * ENTRYLEN ] );
		else
			strcpy( TEMPSPACE, "(entry undefined)" );

		xprintf( "CacheSubdir: index (%d / %d) %s\n", entry_index+1, numentries, TEMPSPACE );
	}
	else {
		// tisztán új dirt olvas be

		if( bZip )		// Zipnél elször ki kell löni a zippet. Power Word: Dismount
			return TRUE;

		REALLOCMEM( entries, NUMINFOS*ENTRYLEN );

		strcpy( INITDIR, GetFilePath( filename ) );
		strcpy( TEMPSPACE, "(temp space)" );

		DIRFLAGS = nosubdir ? TRUE : FALSE;
		ENTRYSORT = SORT_NOTSORTED;

		if( pattern && !stristr(pattern, "*.*") )
			// ha van pattern de nem *.*
			strcpy( SUBDIRPATTERN, pattern );
		else
			SUBDIRPATTERN[0] = 0;
	}

	numentries = 0;

	// lecsippentjük a '\' vagy / a végérõl
	if( INITDIR[ strlen(INITDIR)-1 ] == '\\' || INITDIR[ strlen(INITDIR)-1 ] == '/' )
		INITDIR[ strlen(INITDIR)-1 ] = 0;

	xprintf( "CacheSubdir: caching \"%s\" directory%s.\n", INITDIR, DIRFLAGS?"":" recursively" );

	scanZip = scan_zips;
	if( scanZip )
		xprintf("CacheSubdir: will scan archive files.\n" );

	if( SUBDIRPATTERN[0] == 0 )
		xprintf( "CacheSubdir: no pattern.\n" );
	else
		xprintf( "CacheSubdir: pattern \"%s\".\n", SUBDIRPATTERN );

	xprintf( "CacheSubdir: from \"%s\".\n", filename );

	{
	char initdir[XMAX_PATH];
	strcpy( initdir, INITDIR );
	winListDir( initdir, AddEntry, FALSE, 0, DIRFLAGS );
	}

	// melyik file van kijelölve
	if( entry_index == -1 ) {

		// csak ha új a lista
		entry_index = 0;

		if( filename != NULL && filename[0] != 0 ) {

			char base_name[XMAX_PATH];
			char *p = GetFilenameNoPath( filename );
			strcpy( base_name, p );

			for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {
				char *p = &entries[ i * ENTRYLEN + strlen(INITDIR) + 1 ];
				// xprintf( "%s == %s ?\n", base_name, p );
				if( !stricmp( base_name, p ) ) {
					entry_index = i - NUMINFOS;
					// xprintf("ja: %d\n", entry_index );
					break;
				}
			}
		}
	}
	else {
		// refresh van csak
		int i;

		sortHelper( (sort_e)ENTRYSORT );

		// és most megkeressük sort után
		for( i=NUMINFOS; i<NUMINFOS+numentries; i++ )
			if( !stricmp( TEMPSPACE, &entries[ i * ENTRYLEN ] ) ) {
				entry_index = i - NUMINFOS;
				break;
			}

		// nincs benne a listában: lista 0, letörölték
		if( i-NUMINFOS >= numentries ) {
			entry_index = 0;
			xprintf( "CacheSubdir: refresh didn't find index.\n" );
		}
	}

	xprintf( "CacheSubdir: added %d entries (index = %d).\n", numentries, entry_index+1 );

	return TRUE;
}





static ULONG handle=0;



//
//
//
BOOL FindFirst( char *mask, findfile_ptr find ) {

	/***
	if( numentries > 0 ) {

		// cachelve van

		entry_index = 0;

		strcpy( find->name, &entries[ (1 + entry_index) * XMAX_PATH ] );
		find->size = 10;
		find->attrib = 0L;

		return TRUE;
	}
	***/

	if( (handle = (ULONG)XLIB_winFindFirstFile( mask, find )) == 0 )
		return FALSE;

	return TRUE;
}



//
//
//
BOOL FindNext( findfile_ptr find ) {

	/***
	if( numentries > 0 ) {

		++entry_index;

		if( entry_index >= numentries )
			return FALSE;

		strcpy( find->name, &entries[ (1 + entry_index) * XMAX_PATH ] );
		find->size = 10;
		find->attrib = 0L;

		return TRUE;
	}
	***/

	return XLIB_winFindNextFile( (void *)handle, find );
}



//
//
//
void MyFindClose( void ) {

	/***
	if( numentries > 0 ) {

		entry_index = 0;

		return;
	}
	***/

	XLIB_winFindClose( (void *)handle );

	return;
}






//
// merre:	 1: elõre
//		-1: visszafele
//		 2: End dir
//		-2: Begin dir
//
// char *pattern = "*.s3m;*.stm;*.xm;*.mod;*.it;*.mtm;*.mo3";
//
// ha csak 1 file van akkor NULL
//
char *SearchNextFile( char *curr_filename, int merre, sort_e sort, int *currEntry, int *maxEntry, char *pattern ) {

	static char nextFileName[ XMAX_PATH ] = "Herp, Derp!";
	char searchExt[ XMAX_PATH ] = "Asshatery";
	char searchDir[ XMAX_PATH ] = ".";
	char allnames[XMAX_PATH] = "*.*";

	char *p,*curr_filename_nopath;
	char full_path[XMAX_PATH],save_fullpath[XMAX_PATH];
	char wildcard[XMAX_PATH];

	char first_file[XMAX_PATH];
	char prev_file[XMAX_PATH];
	char last_file[XMAX_PATH];
	int i;

	first_file[0] = 0;
	prev_file[0] = 0;
	last_file[0] = 0;

	nextFileName[0] = 0;

	if( curr_filename == NULL )
		curr_filename = allnames;

	p = GetExtension( curr_filename );
	curr_filename_nopath = GetFilenameNoPath( curr_filename );

	if( p && stricmp( p, searchExt ) )
		strcpy( searchExt, p );

	if( winGetFullPathName( curr_filename, full_path, XMAX_PATH ) ) {
		strcpy( save_fullpath, full_path );
		p = GetFilenameNoPath( full_path );
		*p = 0;
		strcpy( searchDir, full_path );
	}
	else
		strcpy( searchDir, ".\\" );

	// ha pattern van, akkor minden fájl kelleni fog
	if( pattern )
		strcpy( searchExt, "*" );

	if( searchExt[0] == 0 )
		sprintf( wildcard, "%s*", searchDir );
	else
		sprintf( wildcard, "%s*.%s", searchDir, searchExt );

	// xprintf("SearchNextFile: wildcard = \"%s\", current = %s\n", wildcard, curr_filename_nopath );

	findfile_t data;
	BOOL found = FALSE;

	if( currEntry ) *currEntry = -1;
	if( maxEntry ) *maxEntry = 0;
	int num = 0;

	// ha cachebõl dolgozik
	if( numentries > 0 ) {

		// ha nem volt sortolva
		if( sort != ENTRYSORT ) {

			// elmentjük az aktuálisat
			char base_name[XMAX_PATH];

			// xprintf("numentries: %d  entry_index: %d\n",numentries,entry_index);

			// elvileg ez nem lehet
			if( entry_index >= numentries )
				entry_index = numentries-1;

			if( entry_index < 0 )
				entry_index = 0;

			strcpy( base_name, &entries[ (NUMINFOS+entry_index) * ENTRYLEN ] );

			sortHelper( sort );

			ENTRYSORT = sort;

			// és most megkeressük sort után
			for( i=NUMINFOS; i<NUMINFOS+numentries; i++ )
				if( !stricmp( base_name, &entries[ i * ENTRYLEN ] ) ) {
					entry_index = i - NUMINFOS;
					break;
				}

			// nincs benne a listában: lista 0, letörölték
			// elvileg ez soha nem lehet
			if( i-NUMINFOS >= numentries )
				entry_index = 0;
		}


		if( maxEntry ) {

			*maxEntry = numentries;

			// ha maxEntry is kell neki
			if( pattern && stricmp( pattern, SUBDIRPATTERN) ) {

				int cnt = 0;

				for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {

					char *p = &entries[ i * ENTRYLEN ];

					if( (p = GetExtension( p )) == NULL )
						continue;

					if( cmpPattern( p, pattern ) == FALSE )
						continue;

					++cnt;
				}

				*maxEntry = cnt;
			}
		}

// macro mert sok helyre kell
#define GETCURRENTRY { \
	int cnt = 0; \
	for( int i=NUMINFOS; i<NUMINFOS+entry_index; i++ ) { \
		char *p = &entries[ i * ENTRYLEN ];	\
		if( (p = GetExtension( p )) == NULL ) \
			continue;	\
		if( cmpPattern( p, pattern ) == FALSE ) \
			continue; \
		++cnt; \
	}	\
	*currEntry = cnt; }


		if( currEntry ) {

			*currEntry = entry_index;

			if( pattern && stricmp( pattern, SUBDIRPATTERN) )
				GETCURRENTRY;
		}

		// csak lekérdez
		if( merre == SNF_QUERY || merre == 0 ) {

			// ha van filenév akkor az hanyadik
			if( curr_filename != NULL ) {

				int index = -1;

				for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {
					char *p = &entries[ i * ENTRYLEN ];
					// xprintf( "%s == %s ?\n", curr_filename, p );
					if( !stricmp( curr_filename, p ) ) {
						index = i - NUMINFOS;
						// xprintf("ja: %d\n", index );
						break;
					}
				}

				if( currEntry && index != -1 ) *currEntry = index;
			}

			return &entries[ (NUMINFOS + entry_index) * ENTRYLEN ];
			// return NULL;
		}

		// új patternt adott meg, ami nem egyezik aivel beolvasott
		if( pattern && stricmp( pattern, SUBDIRPATTERN) ) {

			switch( merre ) {

				case SNF_BEGINDIR: {

					L1_PATTERNELJE:

					entry_index = -1;

					for( int i=NUMINFOS; i<NUMINFOS+numentries; i++ ) {

						char *p = &entries[ i * ENTRYLEN ];

						if( (p = GetExtension( p )) == NULL )
							continue;

						if( cmpPattern( p, pattern ) == FALSE )
							continue;

						entry_index = i - NUMINFOS;
						break;
					}

					if( entry_index != -1 ) {
						strcpy( nextFileName, &entries[ (NUMINFOS + entry_index) * ENTRYLEN ] );
						if( currEntry )	GETCURRENTRY;
						return nextFileName;
					}

					entry_index = 0;
					return NULL;

					break;
				}

				case SNF_ENDDIR: {

					L1_PATTERNVEGE:

					entry_index = -1;

					for( int i=NUMINFOS+numentries-1; i>=NUMINFOS; i-- ) {

						char *p = &entries[ i * ENTRYLEN ];

						if( (p = GetExtension( p )) == NULL )
							continue;

						if( cmpPattern( p, pattern ) == FALSE )
							continue;

						entry_index = i - NUMINFOS;
						break;
					}

					if( entry_index != -1 ) {
						strcpy( nextFileName, &entries[ (NUMINFOS + entry_index) * ENTRYLEN ] );
						if( currEntry ) GETCURRENTRY;
						return nextFileName;
					}

					entry_index = numentries - 1;
					return NULL;

					break;
				}

				case SNF_FOWARD: {

					// ++entry_index;

					int index = -1;

					for( int i=NUMINFOS+entry_index+1; i<NUMINFOS+numentries; i++ ) {

						char *p = &entries[ i * ENTRYLEN ];

						if( (p = GetExtension( p )) == NULL )
							continue;

						if( cmpPattern( p, pattern ) == FALSE )
							continue;

						index = i - NUMINFOS;
						break;
					}

					if( index != -1 ) {
						entry_index = index;
						strcpy( nextFileName, &entries[ (NUMINFOS + entry_index) * ENTRYLEN ] );
						if( currEntry ) GETCURRENTRY;
						return nextFileName;
					}

					goto L1_PATTERNELJE;

					break;
				}

				case SNF_BACK: {

					// --entry_index;

					int index = -1;

					for( int i=NUMINFOS+entry_index-1; i>=NUMINFOS; i-- ) {

						char *p = &entries[ i * ENTRYLEN ];

						if( (p = GetExtension( p )) == NULL )
							continue;

						if( cmpPattern( p, pattern ) == FALSE )
							continue;

						index = i - NUMINFOS;
						break;
					}

					if( index != -1 ) {
						entry_index = index;
						strcpy( nextFileName, &entries[ (NUMINFOS + entry_index) * ENTRYLEN ] );
						if( currEntry ) GETCURRENTRY;
						return nextFileName;
					}

					goto L1_PATTERNVEGE;

					break;
				}
			}
		}
		else
			switch( merre ) {
				case SNF_BEGINDIR: entry_index = 0; break;
				case SNF_ENDDIR:   entry_index = numentries - 1; break;
				case SNF_BACK:   --entry_index; break;
				case SNF_FOWARD:
				default:	 ++entry_index; break;
			}

		if( entry_index >= numentries )
			entry_index = 0;

		if( entry_index < 0 )
			entry_index = numentries-1;

		strcpy( nextFileName, &entries[ (NUMINFOS + entry_index) * ENTRYLEN ] );

		if( currEntry ) *currEntry = entry_index;

		return nextFileName;
	}


	if( FindFirst( wildcard, &data ) ) {

		do {
			if( !ISFLAG( data.attrib, FIND_SUBDIR ) ) {

				// ha megadott patternt akkor jön az összes file
				if( pattern ) {

					if( (p = GetExtension( data.name )) == NULL )
						continue;

					char full_ext[XMAX_PATH];

					sprintf( full_ext, ".%s", p );

					if( !stristr( pattern, full_ext ) )
						continue;
				}

				// xprintf( "ime ami nem dir: %s\n", data.name );

				++num;
				if( maxEntry ) *maxEntry = num;

				// a dir elsõ fájlja
				if( first_file[0] == 0 ) {
					strcpy( first_file, data.name );
					strcpy( prev_file, data.name );

					// elsõ file kell neki
					if( merre == SNF_BEGINDIR ) {
						sprintf( nextFileName, "%s%s", searchDir, first_file );
						if( currEntry ) *currEntry = 0;
						if( maxEntry ) { found = FALSE; merre = -100; continue; }
						else { MyFindClose(); return nextFileName; }
					}
				}

				strcpy( last_file, data.name );

				if( found == FALSE ) {

					found = stricmp( data.name, curr_filename_nopath )?FALSE:TRUE;
					// xprintf("nextFile: %s, found = %d\n", data.name, found );

					// az egyel azelõtti fájl
					if( found == FALSE )
						strcpy( prev_file, data.name );
					else {

						// csak számolja a maxEntryt
						if( (merre == 0 || merre == SNF_QUERY) && currEntry )
							*currEntry = num-1;

						// megtalálta de nem az elsõ a dirben tehát nem az utcsó kell
						if( merre == SNF_BACK ) {
							if( stricmp( first_file, curr_filename_nopath ) ) {
								// ha a elõzõ file kell és a mostani nem az elsõ
								sprintf( nextFileName, "%s%s", searchDir, prev_file );
								if( currEntry ) *currEntry = num-2;
								if( maxEntry ) { found = FALSE; merre = -100; continue; }
								else { MyFindClose(); return nextFileName; }
							}
							else
								found = FALSE;
						}
					}
				}
				else {
					// ez a kövi a sorban
					if( merre == SNF_FOWARD ) {
						sprintf( nextFileName, "%s%s", searchDir, data.name );
						if( currEntry ) *currEntry = num-1;
						if( maxEntry ) { found = FALSE; merre = -100; continue; }
						else { MyFindClose(); return nextFileName; }
					}
				}
			}

		} while( FindNext( &data ) == TRUE );

		MyFindClose();
	}

	// ha csak 1 file van a dirbe amit megadott akkor null
	if( (num == 1) && !stricmp(curr_filename_nopath, data.name) ) {
		return NULL;
	}

	// ha megtatlálta de már nem volt több fájl és a kövi kell neki akkor az elsõ
	if( merre == SNF_FOWARD && found && first_file[0] != 0 ) {
		sprintf( nextFileName, "%s%s", searchDir, first_file );
		if( currEntry ) *currEntry = 0;
		return nextFileName;
	}

	// visszafele kell, az elsõ filon volt, ezért az utcsó kell neki
	if( merre == SNF_BACK && !stricmp( first_file, curr_filename_nopath ) && last_file[0] != 0 ) {
		sprintf( nextFileName, "%s%s", searchDir, last_file );
		if( currEntry ) *currEntry = num-1;
		return nextFileName;
	}

	// az utsó file kell neki
	if( merre == SNF_ENDDIR && last_file[0] != 0 ) {
		sprintf( nextFileName, "%s%s", searchDir, last_file );
		if( currEntry ) *currEntry = num-1;
		return nextFileName;
	}

	return !nextFileName[0]?NULL:nextFileName;
}







//
//
//
int FileDateYear( int time ) {

	struct tm *t;

	t = localtime( (time_t *)&time );

	return t->tm_year;
}


//
//
//
int FileDateMonth( int time ) {

	struct tm *t;

	t = localtime( (time_t *)&time );

	return t->tm_mon+1;
}


//
//
//
int FileDateDay( int time ) {

	struct tm *t;

	t = localtime( (time_t *)&time );

	return t->tm_mday;
}


//
//
//
int FileDateHour( int time ) {

	struct tm *t;

	t = localtime( (time_t *)&time );

	return t->tm_hour;
}


//
//
//
int FileDateMin( int time ) {

	struct tm *t;

	t = localtime( (time_t *)&time );

	return t->tm_min;
}


//
//
//
int FileDateSec( int time ) {

	struct tm *t;

	t = localtime( (time_t *)&time );

	return t->tm_sec;
}



/***
//
//
//
void timestamp(const char * fromFile, const char * toFile) {

	struct stat st;

	if (stat(fromFile,&st)) {
		xprintf("++++ stat failed for %s\n", fromFile);
		return;
	}

	struct timeval time[2];

	time[0].tv_sec   = st.st_atime;
	time[0].tv_usec  = 0;
	time[1].tv_sec   = st.st_mtime;
	time[1].tv_usec  = 0;

	if (utimes(toFile, &time)) {
		xprintf("++++ utimes failed for %s\n", toFile);

	}

}
***/

//
//
//
BOOL Read( UCHAR *buf, int len, FILE *f ) {

	UCHAR *buf_ptr;
	int temp,todo;

	if( f == NULL || len == 0 || buf == NULL )
		return FALSE;

	if( ZIP_IsFileInZip( f ) )
		return ZIP_LoadFile( buf, len, f );

	if( RAR_IsFileInRar( f ) )
		return RAR_LoadFile( buf, len, f );

	ALLOCMEM( transfer, TRANSFER );
	buf_ptr = buf;
	temp = len;

	while( temp > 0 ) {
		todo = (temp > TRANSFER) ? TRANSFER : temp;
		DISK_PUT;
		if( fread( transfer, todo, 1, f ) != 1 )
			xprintf("Read(): can't read %d bytes from \"%s\" file.",len,"passz!");
		DISK_RES;
		memmove( buf_ptr, transfer, todo );
		buf_ptr += todo;
		temp -= todo;
	}

	FREEMEM( transfer );

	return TRUE;
}





//
//
//
BOOL Write( UCHAR *buf, int len, FILE *f ) {

	UCHAR *buf_ptr;
	int temp,todo;

	ALLOCMEM( transfer, TRANSFER );
	temp = len;
	buf_ptr = buf;

	while( temp > 0 ) {
		todo = (temp > TRANSFER) ? TRANSFER : temp;
		memmove( transfer, buf_ptr, todo );
		buf_ptr += todo;
		DISK_PUT;
		if( fwrite( transfer, todo, 1, f) != 1 )
			xprintf("Write(): can't write %d bytes to \"%s\" file.",len,f->_tmpfname);
		DISK_RES;
		temp -= todo;
	}

	fflush( NULL );

	FREEMEM( transfer );

	return TRUE;
}





//
//
//
BOOL FileSeek( FILE *f, long offset, int whence ) {

	if( ZIP_IsFileInZip( f ) )
		return TRUE;

	if( RAR_IsFileInRar( f ) )
		return TRUE;

	if( whence == SEEK_SET )
		fseek( f, file_start + offset, SEEK_SET);

	if( whence == SEEK_CUR )
		fseek( f, offset, SEEK_CUR);

	if( whence == SEEK_END )
		fseek( f, file_end + offset, SEEK_SET);

	return TRUE;
}





//
//
//
BOOL IsSameFile( char *file1, char *file2 ) {

	if( file1 == NULL || file2 == NULL )
		return FALSE;

	char first[XMAX_PATH];
	char second[XMAX_PATH];

	strcpy( first, file1 );
	strcpy( second, file2 );

	RemoveDuplicateChar( first, '\\' );
	RemoveDuplicateChar( first, '/' );
	ChangeChar( first, '\\', '/' );

	RemoveDuplicateChar( second, '\\' );
	RemoveDuplicateChar( second, '/' );
	ChangeChar( second, '\\', '/' );

	if( !stricmp( first, second) )
		return TRUE;

	return FALSE;
}



//
//
//
ULONG FileSize( FILE *f ) {

	long curr,len;

	if( f == NULL )
		return 0L;

	if( SEG_IsFileInSeg( f ) )
		return SEG_FileSize(f);

	if( ZIP_IsFileInZip( f ) )
		return ZIP_FileSize(f);

	if( RAR_IsFileInRar( f ) )
		return RAR_FileSize(f);

	curr = ftell( f );
	fseek( f, 0L, SEEK_END );
	len = ftell( f );
	fseek( f, curr, SEEK_SET );

	return len;
}





//
//
//
ULONG FileLong( char *name ) {

	FILE *f;
	long l;

	// if( FileExist( name ) == FALSE )
	//	return -1;

	f = XL_OpenFile(name);
	l = FileSize(f);
	XL_CloseFile(f);

	return l;
}






//
//
//
BOOL FileExist( char *n, BOOL physical ) {

	BOOL result = FALSE;
	FILE *f;

	// xprintf("FileExist: %s?\n",n);

	if( !n )
		return FALSE;

	// trollface
	if( n && (!strcmp(n, ".") || !strcmp(n, "..")) )
		return FALSE;

	// if( access(n,F_OK) ) {
	if( (f = fopen(n, "rb" )) == NULL ) {

		if( physical == TRUE )
			return FALSE;

		if( !SEG_FileExist(n) ) {

			if( !ZIP_FileExist(n) ) {

				if( !RAR_FileExist(n) ) {

					int i;
					char data_name[XMAX_PATH*2];

					for( i=0; i<nsearchdirs; i++ ) {

						sprintf(data_name,"%s%s",&search_dir[i*SEARCHDIRLEN],n);

						if( !access(data_name,F_OK) ) {
							result = TRUE;
							break;
						}
					}
				}
				else
					result = TRUE;
			}
			else
				result = TRUE;
		}
		else
			result = TRUE;
	}
	else {
		fclose( f );
		result = TRUE;
	}

	return result;
}



//
//
//
BOOL XL_CopyFile( char *src, char *dst, int flag, BOOL log ) {

	char str[100];

	switch( flag ) {

		case CF_APPEND:

			strcpy( str, "ab" );
			break;

		case CF_OVERWRITE:
		default:

			strcpy( str, "wb" );
			break;
	}

#define BUFFSIZE (16*1024)

	UCHAR *buf = NULL;
	size_t size;
	FILE *fp,*fpW;


	if( (buf = (UCHAR *)malloc( BUFFSIZE )) == NULL ) {
		if( log ) xprintf("XL_CopyFile: can't alloc temp buf.\n");
		return FALSE;
	}

	if( (fp=fopen(src,"rb")) == NULL ) {
		if( log ) xprintf("XL_CopyFile: can't open \"%s\" for Read.\n",src);
		return FALSE;
	}

	if( (fpW=fopen(dst,str)) == NULL ) {
		free(buf);
		fclose( fp );
		if( log ) xprintf("XL_CopyFile: can't open \"%s\" for Write.\n",dst);
		return FALSE;
	}

	while( (size = fread(buf, 1, BUFFSIZE, fp) ) > 0)
		fwrite(buf, 1, size, fpW);

	fclose( fp );
	fclose( fpW );

	free( buf );

	if( log ) xprintf("XL_CopyFile: %s -> %s\n", src, dst );

	return TRUE;
}


//
//
//
BOOL XL_CreateFile( char *name ) {

	FILE *f;

	if( (f=fopen(name,"wb")) == NULL )
		return FALSE;

	fclose( f );

	return TRUE;
}



//
//
//
BOOL EraseFile( char *name ) {

	FILE *f;

	if( (f = fopen(name, "wb")) == NULL )
		return FALSE;

#define DELSTR "this file meant to be deleted."

	fwrite( DELSTR, strlen(DELSTR), 1, f );

#undef DELSTR

	fclose( f );

	return TRUE;
}



//
//
//
void WriteByte( FILE *f, int i ) {

	fputc( i, f );

	return;
}



//
//
//
void WriteWord( FILE *f, int i ) {

	WriteByte( f, ( i & 0xff ) );
	WriteByte( f, ( i >> 8 ) & 0xff );

	return;
}




//
//
//
void WriteDword( FILE *f, int i ) {

	WriteWord( f, ( i & 0xffff ) );
	WriteWord( f, ( i >> 16 ) & 0xffff );

	return;
}




//
//
//
int ReadByte( FILE *f ) {

	return fgetc( f );
}



//
//
//
int ReadWord( FILE *f ) {

	int i;

	i = ReadByte( f );
	i += ( ReadByte( f ) * 0x0100 );

	return i;
}



//
//
//
int ReadDword( FILE *f ) {

	int i;

	i = ReadWord( f );
	i += ( ReadWord( f ) * 0x10000 );

	return i;
}






//
// .TXT file processing
//



#define STRING_TOKEN_MAX_LENGTH 512

#define IS_STRING_TOKEN_CHAR(c) (isalpha(c) || isdigit(c) || \
				 (c) == '/' || (c) == '.' || (c) == '~' || \
				 (c) == '_' || (c) == ':' || (c) == '\\' )
#define COMMENT_CHAR ';'

static int line_number=0;

#define PARSEERROR(s) Quit("parse error at %d line.",line_number);



//
//
//
int skip_whitespace( FILE *fp ) {

	int c = ffgetc(fp);

	while( (c == ' ') || (c == '\t') || (c == '\n') || (c == COMMENT_CHAR) ) {

		if( c == COMMENT_CHAR )
			while( (c != EOF) && (c != '\n') )
				c = ffgetc(fp);

		if( c  == '\n' )
			line_number++;

		if( c == EOF )
			return c;

		c = ffgetc(fp);
	}

	return c;
}




//
//
//
BOOL GetStringTxt( FILE *fp, char *tokenbuf ) {

	int length = 0;
	int c;

	*tokenbuf = '\0';

	if( (c = skip_whitespace(fp)) == EOF )
		return FALSE;

	if( c == '"' ) {

		c = ffgetc(fp);

		while( c != EOF ) {

			if( c == '"' ) {
				c = ffgetc(fp);
				break;
			}

			if( c == COMMENT_CHAR )
				break;

			if( length >= STRING_TOKEN_MAX_LENGTH - 1 )
				return FALSE;

			tokenbuf[length++] = c;
			tokenbuf[length] = 0;

			c = ffgetc(fp);
		}
	}
	else
	while( /* c != EOF && */ IS_STRING_TOKEN_CHAR(c)) {

		if( c == EOF )
			return BOOLEAN(length);

		if( length >= STRING_TOKEN_MAX_LENGTH - 1 )
			return FALSE;

		tokenbuf[length++] = c;
		tokenbuf[length] = 0;

		c = ffgetc(fp);
	}

	if( c != EOF )
		ffungetc( c, fp );

	return TRUE;
}


//
// TODO: ffgets
//
BOOL GetLineTxt( FILE *fp, char *tokenbuf ) {

	return FALSE;
}




//
//
//
BOOL GetFixedTxt( FILE *fp, FIXED *f ) {

	double d;
	int c;

	*f = FIXED_ZERO;

	c = skip_whitespace(fp);
	if( c == EOF )
		return FALSE;

	ffungetc(c, fp);
	if(ffscanf(fp, "%lf", &d) != 1)
		return FALSE;

	*f = FLOAT_TO_FIXED(d);

	return TRUE;
}


//
//
//
BOOL GetFloatTxt( FILE *fp, FLOAT *f ) {

	// float d;
	int c,cnt;
	char str[XMAX_PATH];

	*f = 0.0;

	c = skip_whitespace(fp);
	if( c == EOF )
		return FALSE;

	ffungetc(c, fp);

	cnt = 0;

	static char decimal = 0;
	if( decimal == 0 ) {
		struct lconv *lcp = localeconv();
		if( lcp != NULL && lcp->decimal_point != NULL && *lcp->decimal_point != '\0' )
			decimal = *lcp->decimal_point;
		else
			decimal = '.';
	}

	while( 1 ) {

		c = ffgetc( fp );

		if( c == EOF )
			break;

		if( isdigit(c) || (c == '.') || (c == ',') || (c == '-') ) {
			if( c == '.' ) c = decimal;
			str[cnt] = c;
			++cnt;
		}
		else {
			ffungetc( c, fp );
			break;
		}

		if( cnt >= XMAX_PATH )
			break;
	}

	str[cnt] = 0;

	*f = (FLOAT)atof( str );
	// sscanf( str, "%f", f );

	// xprintf("float: \"%s\", %.2f\n", str, *f );

	// if(ffscanf(fp, "%f", &d) != 1)
	//	return FALSE;

	// *f = d;

	return TRUE;
}



//
// decimal pont mint a FLOAT-ba
//
BOOL FIXME_GetDoubleTxt( FILE *fp, double *f ) {

	double d;
	int c;

	*f = 0.0;

	c = skip_whitespace(fp);
	if( c == EOF )
		return FALSE;

	ffungetc(c, fp);
	if(ffscanf(fp, "%lf", &d) != 1)
		return FALSE;

	*f = d;

	return TRUE;
}



//
//
//
BOOL GetIntTxt( FILE *fp, int *i ) {

	int c,c1;

	*i = 0L;

	c = skip_whitespace(fp);
	if( c == EOF )
		return FALSE;

	c1 = ffgetc(fp);

	if( (c == '0') && (c1 == 'x') ) {

		if( ffscanf(fp, "%x", i) != 1 )
			return FALSE;

		return TRUE;
	}

	ffungetc(c1, fp);
	ffungetc(c, fp);

	if( ffscanf(fp, "%d", i) != 1 )
		return FALSE;

	return TRUE;
}




//
//
//
BOOL IsNextStringTxt( FILE *fp ) {

	int c;

	c = skip_whitespace(fp);
	if( c == EOF )
		return FALSE;

	ffungetc(c, fp);

	if( c == '"' || isalpha(c) )
		return TRUE;

	return FALSE;
}



//
//
//
BOOL IsNextNumberTxt( FILE *fp ) {

	int c;

	c = skip_whitespace(fp);
	if( c == EOF )
		return FALSE;

	ffungetc(c, fp);

	if( isdigit(c) )
		return TRUE;

	return FALSE;
}



