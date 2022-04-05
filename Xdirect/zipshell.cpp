
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <io.h>



// #include <DebugHlp.h>

#include <xlib.h>


#undef Z_PREFIX

#include "zlib.h"
#include "ziparchive.h"


RCSID( "$Id: zipshell.cpp,v 1.2 2003/09/22 13:59:58 bernie Exp $" )


// http://www.zlib.net/

#define ZLIB1DLLNAME "zlib1.dll"


static ZIPArchive **pArchive;
static int nzipfiles = 0L;

static FILE zip_file;
static char zip_file_name[XMAX_PATH];


int (_stdcall *_deflateInit_)(z_streamp strm, int level, const char *version, int stream_size) = NULL;
int (_stdcall *_deflate)(z_streamp strm, int flush) = NULL;
int (_stdcall *_deflateEnd)(z_streamp strm) = NULL;
int (_stdcall *_inflate)(z_streamp strm, int flush) = NULL;
int (_stdcall *_inflateEnd)(z_streamp strm) = NULL;
int (_stdcall *_inflateInit2_)(z_streamp strm, int  windowBits, const char *version, int stream_size) = NULL;
uLong (_stdcall *_crc32)(uLong crc, const Bytef *buf, uInt len) = NULL;
const char * (_stdcall *_zlibVersion)(void) = NULL;


UNCOMPRESSFN _uncompress;
UNCOMPRESSFN _compress;

static HINSTANCE hDll = NULL;



//
//
//
static void error_zlib( char *func_name ) {

	xprintf( "Error_Zlib: can't load \"%s\" (%s) library.\n", ZLIB1DLLNAME, func_name );

	return;
}


//
//
//
static BOOL LoadZlibDll( void ) {

	if( hDll != NULL )
		return TRUE;

	char zlibdllname[XMAX_PATH];

	sprintf( zlibdllname, "%s\\%s", GetExeDir(), ZLIB1DLLNAME );

	if( (hDll = LoadLibraryEx( zlibdllname, NULL, 0L )) == NULL ) {
		error_zlib( "init" );
		return FALSE;
	}

	if( (_compress = (UNCOMPRESSFN)GetProcAddress( hDll, "compress" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "compress" );
		return FALSE;
	}

	if( (_uncompress = (UNCOMPRESSFN)GetProcAddress( hDll, "uncompress" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "uncompress" );
		return FALSE;
	}

	if( (_crc32 = (uLong (_stdcall *)(uLong crc, const Bytef *buf, uInt len))GetProcAddress( hDll, "crc32" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "crc32" );
		return FALSE;
	}

	if( (_inflateInit2_ = (int (_stdcall *)(z_streamp strm, int  windowBits, const char *version, int stream_size))GetProcAddress( hDll, "inflateInit2_" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "inflateInit2_" );
		return FALSE;
	}

	if( (_inflateEnd = (int (_stdcall *)(z_streamp strm))GetProcAddress( hDll, "inflateEnd" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "inflateEnd" );
		return FALSE;
	}

	if( (_inflate = (int (_stdcall *)(z_streamp strm, int flush))GetProcAddress( hDll, "inflate" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "inflate" );
		return FALSE;
	}

	if( (_deflateEnd = (int (_stdcall *)(z_streamp strm))GetProcAddress( hDll, "deflateEnd" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "deflateEnd" );
		return FALSE;
	}

	if( (_deflate = (int (_stdcall *)(z_streamp strm, int flush))GetProcAddress( hDll, "deflate" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "deflate" );
		return FALSE;
	}

	if( (_deflateInit_ = (int (_stdcall *)(z_streamp strm, int level, const char *version, int stream_size))GetProcAddress( hDll, "deflateInit_" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "deflateInit_" );
		return FALSE;
	}

	if( (_zlibVersion = (const char * (_stdcall *)(void))GetProcAddress( hDll, "zlibVersion" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		error_zlib( "zlibVersion" );
		return FALSE;
	}

	xprintf("LoadZlibDll: \"%s\" %s loaded.\n", zlibdllname, _zlibVersion() );

	return TRUE;
}




//
// return: handle + 1
//
int OpenZipFile( char *name, BOOL for_write ) {

	FILE *f;
	char str[XMAX_PATH];
	int i;

	if( name == NULL )
		return 0;

	// be van-e már töltve
	for( i=0; i<nzipfiles; i++ )
		if( !stricmp( pArchive[i]->GetFilename(), name ) )
			break;

	// igen be
	if( i < nzipfiles )
		return i+1;

	strcpy( str, name );

	if( for_write == FALSE ) {

		// nem akar új Zip fájlt

		if( (f=fopen(str,"rb")) == NULL ) {

			sprintf( str, "DATA\\%s", name );

			if( (f=fopen(str,"rb")) == NULL ) {
				xprintf( "OpenZipFile: can't open \"%s\" file.\n", name );
				return 0;
			}
		}

		fclose( f );
	}

	if( LoadZlibDll() == FALSE )
		return 0;

	REALLOCMEM( pArchive, (nzipfiles+1) * sizeof(ZIPArchive*) );

	pArchive[ nzipfiles ] = new ZIPArchive( str, for_write?FALSE:TRUE, "" );

	// placement new operator
	// new (pArchive[ nzipfiles ]) ZIPArchive(str, TRUE, "");

	if( !pArchive[ nzipfiles ]->first_file() && for_write == FALSE ) {
		delete pArchive[ nzipfiles ];
		return 0;
	}

	xprintf("OpenZipFile: %s opened (idx = %d, cnt = %d).\n", name, nzipfiles, pArchive[ nzipfiles ]->file_cnt);

	return ++nzipfiles;
}



//
//
//
void CloseZipFile( char *filename ) {

	int i;

	for( i=0; i<nzipfiles; i++ )
		if( !stricmp( pArchive[i]->GetFilename(), filename ) )
			break;

	if( i >= nzipfiles )
		return;

	// írásra nyitva
	if( pArchive[i]->getreadonly() == false )
		pArchive[i]->write_archive();

	delete pArchive[i];

	// ha csak egy volt
	if( nzipfiles == 1 ) {
		nzipfiles = 0;
		FREEMEM( pArchive );
		pArchive = NULL;
		return;
	}

	// nem az utsó volt
	if( i < (nzipfiles-1) )
		memmove( &pArchive[i],
			 &pArchive[i+1],
			 sizeof(ZIPArchive)*(nzipfiles-(i+1)) );

	// ha az utsó volt leszarjuk

	--nzipfiles;

	xprintf("CloseZipFile: %s closed (idx = %d).\n", filename,i);

	return;
}





//
//
//
void CloseAllZipFile( void ) {

	int i;

	for( i=0; i<nzipfiles; i++ ) {

		// írásra nyitva
		if( pArchive[i]->getreadonly() == false )
			pArchive[i]->write_archive();

		delete pArchive[i];
	}

	FREEMEM( pArchive );
	pArchive = NULL;

	nzipfiles = 0;

	return;
}






//
//
//
FILE *ZIP_OpenFile( char *name ) {

	size_t filesize;
	int i;

	if( name == NULL )
		return NULL;

	for( i=0; i<nzipfiles; i++ ) {

		if( pArchive[i]->file_exists( name, &filesize ) ) {

			strncpy( zip_file_name, name, XMAX_PATH );

			return &zip_file;
		}
	}

	return NULL;
}






//
//
//
BOOL ZIP_LoadFile( UCHAR *buf, int len, FILE *f ) {

	size_t filesize;
	char *pMemory;
	int i;

	if( f != (FILE *)&zip_file )
		return FALSE;

	for( i=0; i<nzipfiles; i++ ) {

		if( pArchive[i]->file_exists( zip_file_name, &filesize ) ) {

			pMemory = pArchive[i]->read( zip_file_name, &filesize );

			memcpy( buf, pMemory, MIN( len, (int)filesize ) );

			delete pMemory;

			return TRUE;
		}
	}

	return FALSE;
}






//
//
//
int ZIP_FileSize( FILE *f ) {

	size_t filesize;
	int i;

	if( f != (FILE*)&zip_file )
		return -1;

	for( i=0; i<nzipfiles; i++ ) {

		if( pArchive[i]->file_exists( zip_file_name, &filesize ) )
			return filesize;
	}

	return -1;
}





//
//
//
BOOL ZIP_FileExist( char *name ) {

	int i;

	for( i=0; i<nzipfiles; i++ ) {

		if( pArchive[i]->file_exists( name, NULL ) )
			return TRUE;
	}

	return FALSE;
}





//
//
//
BOOL ZIP_IsFileInZip( FILE *f ) {


	if( f != (FILE *)&zip_file )
		return FALSE;

	return TRUE;
}


//
// flag = RESET
//
BOOL ZIP_GetNextFilename( char *filename, int handle, int flag,
			  int *size, int *year, int *month, int *day,
			  int *hour, int *minute, int *second, int *milliseconds ) {

	if( filename == NULL || handle < 0 || handle >= nzipfiles )
		return FALSE;

#define ZIP pArchive[handle]

	if( flag ) {
		ZIP->curr_entry = NULL;
		ZIP->curr_file = 0;
	}

	if( pArchive[handle]->curr_entry == NULL ) {
		ZIP->curr_entry = ZIP->first_file();
		ZIP->curr_file = 0;
	}
	else
	if( (ZIP->curr_entry = ZIP->next_file(ZIP->curr_entry)) == NULL ) {
		ZIP->curr_file = 0;
		ZIP->curr_entry = NULL;
		return FALSE;
	}

	++(ZIP->curr_file);

	if( size )
		*size = ZIP->get_file_size(ZIP->curr_entry);

	tm ztime;

	ZIP->get_file_time( ZIP->curr_entry, ztime );

	if( year ) *year = 1900 + ztime.tm_year;
	if( month ) *month = ztime.tm_mon;
	if( day ) *day = ztime.tm_mday;
	if( hour ) *hour = ztime.tm_hour;
	if( minute ) *minute = ztime.tm_min;
	if( second ) *second = ztime.tm_sec;
	if( milliseconds ) *milliseconds = 0;

	strcpy( filename, ZIP->entry_filename(ZIP->curr_entry) );

	return TRUE;
}



//
//
//
BOOL IsZIP( char *ibuf, int len ) {

	USEPARAM( len );

	return IdBufStr( ibuf, FIS_ZIP );
}



//
// TODO: a fájlok mod timeja is belekerüljön
//
BOOL ZIP_Append( int handle, char *filename, UCHAR *buf, int len, char *name, ULONG time ) {

	if( filename == NULL /*&& (buf == NULL || len == 0)*/ )
		return FALSE;

	if( handle < 0 || handle >= nzipfiles )
		return FALSE;

	char *ptr;
	int size;

	size = LoadFile( filename, PTR(ptr) );

	if( size == 0 )
		return FALSE;

	void *entry = pArchive[handle]->new_file( filename, size );

	if( entry == NULL ) {
		FREEMEM( ptr );
		return FALSE;
	}

	if( time != 0 ) {
		struct tm *p = localtime( (time_t *)&time );
		pArchive[handle]->set_file_time( entry, *p );
	}

	if( pArchive[handle]->write( entry, ptr, size ) == false ) {
		FREEMEM( ptr );
		return FALSE;
	}

	FREEMEM( ptr );

	// FIXME: ez jó lesz-e
	static int append_size = 0;
	append_size += size;
	if( append_size > (1024*1024*1024) ) {
		// 1GB-nél több van a memóban, ideje dumpolni
		// pArchive[handle]->write_archive();
		append_size = 0;
	}

	return TRUE;
}



//
//
//
ULONG ZIP_Compress( UCHAR *uncompr, int ulen, memptr_ptr ptr ) {

	int err;
	uLong comprLen;
	UCHAR *compr;

	if( LoadZlibDll() == FALSE )
		return FALSE;

	comprLen = (int)((FLOAT)ulen * 2.0f) + 12;

	ALLOCMEM( *ptr, comprLen + 12 );

	compr = *ptr;

	memcpy( compr, FIS_ZIP, FI_SIZE );
	compr += FI_SIZE;

	memcpy( compr, &ulen, sizeof(int));
	compr += sizeof(int);

	compr += sizeof(int);	// compressed length

	// xprintf("compress elõtt...\n");

	err = _compress( compr, &comprLen, uncompr, (uLong)ulen );

	// xprintf("compress UTÁN...\n");

	ulen = comprLen;
	compr = &(*ptr)[8];

	memcpy( compr, &ulen, sizeof(int));

	return (err == Z_OK) ? (comprLen + 12) : 0L;
}




//
//
//
ULONG ZIP_Uncompress( UCHAR *compr, memptr_ptr ptr ) {

	int err,size,clen;
	uLong uncomprLen;

	if( !IsZIP( (char *)compr, 0) )
		return 0L;

	if( LoadZlibDll() == FALSE )
		return FALSE;

	compr += FI_SIZE;

	memcpy( &size, compr, sizeof(int) );

	compr += sizeof(int);

	if( size == 0 )
		return 0L;

	uncomprLen = size;

	ALLOCMEM( *ptr, uncomprLen );

	memcpy( &clen, compr, sizeof(int) );
	compr += sizeof(int);

	err = _uncompress(*ptr, &uncomprLen, compr, clen);

	return (err == Z_OK) ? size : 0L;
}




#if 0

FTFile* FTFileSystem::ArchiveOpen( const char* pathname ) {

	if( !pathname ) return NULL;

  FTFile* fp = new FTFile( this, pathname );
  if (!fp) return NULL;

  // Try the actual file first
  fp->fp = fopen( pathname, "rb" );
	if (fp->fp)
	{
		fseek( fp->fp, 0, SEEK_END );
		fp->fileSize = ftell( fp->fp );
		fseek( fp->fp, 0, SEEK_SET );
		fp->filePos = 0;
	}
  else
  {
    // File wasn't there, check archives
    ZIPArchive* pArchive = NULL;
    char archivepath[MAXPATHLEN];
    char newfilepath[MAXPATHLEN];
    strcpy( archivepath, pathname );
    strcpy( newfilepath, pathname );
    char* pFilename;
    char* ptr;

    while (ptr = strrchr( archivepath, '/' ))
    {
      *ptr = 0;
      pFilename = newfilepath + ( ptr - archivepath + 1 );
      strcat( archivepath, ".zip" );

      pArchive = new ZIPArchive( archivepath, true, "" );

      if (pArchive->first_file())
      {
        // Our desired archive exist...
        // Now check for the file in the archive
        size_t filesize;
        if (pArchive->file_exists( pFilename, &filesize ))
        {
          fp->bFileInMemory = true;
          fp->pMemory       = pArchive->read( pFilename, &fp->fileSize );
          fp->filePos       = 0;
          if (!fp->pMemory) FrogError("ArchiveOpen(): Arrrg!");

          delete pArchive;
          return fp;
        }
      }
    }

    if (pArchive)
    {
      delete pArchive;
      pArchive = NULL;
    }
    delete fp;
    fp = NULL;
  }

  return fp;
}


#define USEZLIBDLL
#include "zlib.h"

#define BUFLEN	65536

static int gz_uncompress(gzFile in, FILE   *out)
{
    static char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = gzread(in, buf, BUFLEN);
        if (len == 0) break;

        if (fwrite(buf, 1, len, out) != len)
        	return 0;
    }
    if (fclose(out))
    	return 0;

    if (gzclose(in) != Z_OK)
    	return 0;

    return 1;
}

int file_uncompress(char *infile, char *outfile)
{
    FILE  *out;
    gzFile in;

    in = gzopen(infile, "rb");
    if (in == NULL) {
		return 0;
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
		return 1;
    }

    return gz_uncompress(in, out);
}



////////////// 7-Zip


#define SEVENZIPDLLNAME  "SEVENZIP.DLL"

static HINSTANCE hZip = 0;		// bass handle

#define LOADZIPFUNCTION(f,g) { 							\
	*((void**)&g)=(void*)GetProcAddress(hZip,#f); 				\
	if( (g) == NULL ) { 							\
		if( hZip ) FreeLibrary( hZip ); 				\
		hZip = NULL; 							\
		xprintf("Load7zip: can't load \"%s\" function.\n", #f ); 	\
		return FALSE; 							\
	} }


static int (*_OpenArc)( char *filename );
static void (*_CloseArc)( int handle );
static BOOL (*_LoadFile)( char *filename, char *buf );
static char *(*_GetError)( void );
static BOOL (*_GetNextFilename)( char *filename, int handle, int flag,
					int *size, int *year, int *month, int *day,
					int *hour, int *minute, int *second, int *milliseconds );


static FILE sevenzip_file;
static char sevenzip_file_name[XMAX_PATH];
static char sevenzip_disc_name[XMAX_PATH];

static int n7zipfiles = 0;



//
//
//
static BOOL Load7zip( void ) {

	char zipdllname[XMAX_PATH];

	sprintf( zipdllname, "%s\\%s", GetExeDir(), SEVENZIPDLLNAME );

	// load the temporary BASS.DLL library
	if( (hZip = LoadLibrary(zipdllname)) == NULL ) {
		xprintf("Load7zip: couldn't load \"%s\" library.\n",zipdllname);
		return FALSE;
	}

	LOADZIPFUNCTION( OpenArc, _OpenArc );
	LOADZIPFUNCTION( CloseArc, _CloseArc );
	LOADZIPFUNCTION( LoadFile, _LoadFile );
	LOADZIPFUNCTION( GetError, _GetError );
	LOADZIPFUNCTION( GetNextFilename, _GetNextFilename );

	xprintf("Load7zip: %s loaded.\n", zipdllname );

	return TRUE;
}



//
// return: handle + 1
//
int Open7zipFile( char *name ) {

	char str[XMAX_PATH];
	FILE *f;

	if( Load7zip() == FALSE || name == NULL )
		return 0;

	strcpy( str, name );

	if( (f=fopen(str,"rb")) == NULL ) {

		sprintf( str, "DATA\\%s", name );

		if( (f=fopen(str,"rb")) == NULL ) {
			xprintf( "Open7zipFile: can't open \"%s\" file.\n", name );
			return 0;
		}
	}

	fclose( f );

	if( _OpenArc( str ) == 0 ) {
		xprintf( "Open7zipFile: OpenArc phailed.\n", name );
		return 0;
	}

	strcpy( sevenzip_file_name, name );
	strcpy( sevenzip_disc_name, str );

	xprintf("Open7zipFile: %s opened (idx = %d).\n", name, n7zipfiles);

	return ++n7zipfiles;
}



//
//
//
void Close7zipFile( char *filename ) {

	_CloseArc( n7zipfiles-1 );

	--n7zipfiles;

	xprintf("CloseRarFile: %s closed.\n", filename );

	return;
}



//
// flag = RESET
//
BOOL SZIP_GetNextFilename( char *filename, int handle, int flag,
			  int *size, int *year, int *month, int *day,
			  int *hour, int *minute, int *second, int *milliseconds ) {

	if( filename == NULL || n7zipfiles <= 0 )
		return FALSE;

	return _GetNextFilename( filename, handle, flag,
				 size, year, month, day,
				 hour, minute, second, milliseconds );
}


void test() {

	if( Open7zipFile( "elso.zip" ) ) {

		xprintf("opened.\n");

		char filename[XMAX_PATH];
		int size;
		int year, month, day, hour, minute, second, milliseconds;

		while( SZIP_GetNextFilename( filename, 1, 0, &size, &year, &month, &day, &hour, &minute, &second, &milliseconds ) )
			xprintf("%s   %d\n", filename, size );

		Close7zipFile( NULL );
	}
}


#endif

