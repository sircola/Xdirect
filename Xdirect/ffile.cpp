/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */


#pragma warning(disable : 4996)

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <time.h>
#include <io.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <locale.h>
#include <math.h>

#include <xlib.h>

RCSID( "$Id: ffile.cpp,v 1.2 2003/09/22 13:59:57 bernie Exp $" )



/*
 *  ff....
 *  routines for The Best
 *
 */


#define FF_NONE  0
#define FF_READ  1
#define FF_WRITE 2
#define FF_REAL  3

typedef struct ffile_s {

	FILE *ff_f;
	char ff_name[XMAX_PATH];
	UCHAR *ff_data;
	int ff_size;
	int ff_offset;
	int ff_mode;
	int ff_alloced;

} ffile_t, *ffile_ptr;


#define MAXFILES 30

static ffile_t *ffile = NULL;


#define INVALIDHANDLE	(-1)

static int FlipToUnix( UCHAR *,int,UCHAR * );



//
//
//
static int SearchFile( FILE *f ) {

	int i;

	for( i=0; i<MAXFILES; i++ )
		if( ffile[i].ff_f == f )
			return i;

	// Quit("SearchFile: can't find \"%s\" handle.","valaki");

	return INVALIDHANDLE;
}



//
//
//
int GetNew( void ) {

	int i;

	for( i=0; i<MAXFILES; i++ )
		if( ffile[i].ff_mode == FF_NONE )
			return i;

	return INVALIDHANDLE;
}




//
//
//
BOOL InitFfile( void ) {

	if( ffile ) FREEMEM( ffile );
	ALLOCMEM( ffile, (MAXFILES * sizeof(ffile_t)) );

	memset( ffile, 0L, (MAXFILES * sizeof(ffile_t)) );

	return TRUE;
}



//
//
//
void DeinitFfile( void ) {

	int i;

	if( ffile != NULL ) {

		for( i=0; i<MAXFILES; i++ ) {

			if( ffile[i].ff_mode != FF_NONE ) {
				xprintf("file \"%s\" is not closed.\n",ffile[i].ff_name);
				ffclose( ffile[i].ff_f );
			}
		}

		FREEMEM( ffile );
		ffile = NULL;
	}

	return;
}



static int ff_num = INVALIDHANDLE;
#define FF(tag) ffile[ff_num].tag


//
//
//
FILE *ffopen( char *name, const char *mode ) {

	if( (ff_num = GetNew()) == INVALIDHANDLE ) {
		xprintf("ffopen: increase MAXFILES.\n");
		return NULL;
	}

	strncpy( FF(ff_name), name, XMAX_PATH );

	switch( mode[0] ) {

		case 'r':

			if( !FileExist( name ) )
				return NULL;

			SAFE_FREEMEM( FF(ff_data) );

			FF(ff_offset) = 0;

			FF(ff_alloced) = 0;

			FF(ff_size) = LoadFile( name, PTR(FF(ff_data)) );

			FF(ff_mode) = FF_READ;

			break;

		case 'w':

			FF(ff_offset) = 0;

			FF(ff_alloced) = 0;

			FF(ff_size) = 0;

			SAFE_FREEMEM( FF(ff_data) );

			FF(ff_mode) = FF_WRITE;

			break;

		default:

			Quit("ffopen: mode undefined.");

			break;
	}

	if( (FF(ff_mode) == FF_READ) && (mode[1] == 't') ) {
		UCHAR *ptr;
		ALLOCMEM( ptr, FF(ff_size) );
		FF(ff_size) = FlipToUnix( FF(ff_data), FF(ff_size), ptr );
		REALLOCMEM( FF(ff_data), FF(ff_size) );
		memcpy( FF(ff_data), ptr, FF(ff_size) );
		FREEMEM( ptr );
	}

	if( FF(ff_f) ) FREEMEM( FF(ff_f) );
	ALLOCMEM( FF(ff_f), sizeof(FILE) );

	return FF(ff_f);
}




//
//
//
FILE *ffopen2( char *name, const char *mode, UCHAR *buf, int buf_len ) {

	if( (ff_num = GetNew()) == INVALIDHANDLE ) {
		xprintf("ffopen2: increase MAXFILES.\n");
		return NULL;
	}

	strncpy( FF(ff_name), name, XMAX_PATH );

	switch( mode[0] ) {

		case 'r':

			if( FF(ff_data) != NULL ) FREEMEM( FF(ff_data) );

			ALLOCMEM( FF(ff_data), buf_len );

			memcpy( FF(ff_data), buf, buf_len );

			FF(ff_offset) = 0;

			FF(ff_alloced) = 0;

			FF(ff_size) = buf_len;

			FF(ff_mode) = FF_READ;

			break;

		case 'w':

			FF(ff_offset) = 0;

			FF(ff_size) = 0;

			FF(ff_alloced) = 0;

			if( FF(ff_data) != NULL ) FREEMEM( FF(ff_data) );

			FF(ff_mode) = FF_WRITE;

			break;

		default:

			Quit("ffopen2: mode undefined.");

			break;
	}

	if( FF(ff_mode) == FF_READ && mode[1] == 't' ) {
		UCHAR *ptr;
		ALLOCMEM( ptr, FF(ff_size) );
		FF(ff_size) = FlipToUnix( FF(ff_data), FF(ff_size), ptr );
		REALLOCMEM( FF(ff_data), FF(ff_size) );
		memcpy( FF(ff_data), ptr, FF(ff_size) );
		FREEMEM( ptr );
	}

	if( FF(ff_f) ) FREEMEM( FF(ff_f) );
	ALLOCMEM( FF(ff_f), sizeof(FILE) );

	return FF(ff_f);
}





//
//
//
FILE *ffopen3( char *name, const char *mode ) {

	if( (ff_num = GetNew()) == INVALIDHANDLE ) {
		xprintf("ffopen3: increase MAXFILES.\n");
		return NULL;
	}

	strncpy( FF(ff_name), name, XMAX_PATH );

	switch( mode[0] ) {

		case 'r':

			if( !FileExist( name ) )
				return NULL;

			FF(ff_f) = XL_OpenFile( name );

			if( FF(ff_data) != NULL ) FREEMEM( FF(ff_data) );

			FF(ff_offset) = 0;

			FF(ff_alloced) = 0;

			FF(ff_size) = filelength( fileno(FF(ff_f)) );

			FF(ff_mode) = FF_REAL;

			break;

		default:

			Quit("ffopen3: mode undefined.");

			break;
	}

	return FF(ff_f);
}




//
//
//
static int FlipToUnix( UCHAR *in, int size, UCHAR *out ) {

	int i,out_size;

	out_size = 0;
	for( i=0; i<size; i++,out_size++ )

		if( (out[out_size] = in[i]) == '\r' )
			--out_size;

	return out_size;
}








//
//
//
void ffclose( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffclose: can't find handle.\n");
		return;
	}

	if( FF(ff_mode) == FF_REAL ) {
		fclose( FF(ff_f) );
		FF(ff_f) = NULL;
	}
	else
		if( FF(ff_f) ) FREEMEM( FF(ff_f) );

	if( FF(ff_mode) == FF_WRITE )
		XL_WriteFile( FF(ff_name), FF(ff_data), FF(ff_size) );

	FF(ff_mode) = FF_NONE;
	FF(ff_offset) = 0;
	FF(ff_size) = 0;
	FF(ff_alloced) = 0;
	if( FF(ff_data) ) FREEMEM( FF(ff_data) );

	return;
}



//
//
//
void ffree( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffree: can't find handle.\n");
		return;
	}

	if( FF(ff_mode) == FF_REAL )
		fclose( FF(ff_f) );
	else
		if( FF(ff_f) ) FREEMEM( FF(ff_f) );

	FF(ff_mode) = FF_NONE;
	FF(ff_offset) = 0;
	FF(ff_size) = 0;
	FF(ff_alloced) = 0;
	if( FF(ff_data) ) FREEMEM( FF(ff_data) );

	return;
}




//
//
//
BOOL ffeof( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffeof: handle not found.\n");
		return TRUE;
	}

	//xprintf("%d, %d\n",F(ff_offset),F(ff_size));

	return ( FF(ff_offset) >= FF(ff_size) );
}




//
//
//
int ffread( void *ptr, int size, int snum, FILE *f ) {

	int amount;

	if( (ff_num=SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffread: can't find handle.\n");
		return (-1);
	}

	if( FF(ff_mode) == FF_NONE ) {
		xprintf("ffread: mode not defined.\n");
		return (-1);
	}

	amount = size * snum;

	if( FF(ff_offset) /* + (size*snum)) */ > FF(ff_size) ) {
		// xprintf("ffread: %d %d %d.\n",F(ff_size),F(ff_offset),size*snum);
		return EOF;
	}

	if( FF(ff_mode) == FF_REAL )
		fread( ptr, size, snum, FF(ff_f) );
	else {
		CLAMPMINMAX( amount, 0, FF(ff_size) -  FF(ff_offset) );
		memcpy( ptr, &FF(ff_data)[FF(ff_offset)], amount );
	}

	FF(ff_offset) += amount;

	return snum;
}




//
//
//
int ffwrite( void *ptr, int size, int snum, FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffwrite: can't find handle.\n");
		return (-1);
	}

	if( FF(ff_mode) != FF_WRITE ) {
		xprintf("ffwrite: mode not defined.\n");
		return EOF;
	}

	if( (FF(ff_offset) + (size*snum)) >= FF(ff_alloced)  )  {
		FF(ff_alloced) = FF(ff_offset) + MAX(1024,(size*snum));
		REALLOCMEM( FF(ff_data), FF(ff_alloced) );
	}

	memcpy( &FF(ff_data)[FF(ff_offset)], ptr, size*snum );

	FF(ff_offset) += size*snum;

	if( FF(ff_offset) > FF(ff_size) )
		FF(ff_size) = FF(ff_offset);

	return snum;
}



//
//
//
int ffgetc( FILE *f ) {

	int value;

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffgetc: can't find handle.\n");
		return TRUE;
	}

	if( FF(ff_mode) == FF_NONE ) {
		xprintf("ffgetc: w/o file.\n");
		return EOF;
	}

	if( FF(ff_offset) >= FF(ff_size) ) {
		// xprintf("ffgetc: reading behind eof.\n");
		return EOF;
	}

	if( FF(ff_mode) == FF_REAL )
		value = fgetc( FF(ff_f) );
	else
		value = FF(ff_data)[FF(ff_offset)++];

	// xprintf("'%c'",value);

	return value;
}





//
//
//
void ffungetc( int c, FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffungetc: can't find handle.\n");
		return;
	}

	if( FF(ff_mode) == FF_REAL ) {
		ungetc( c, FF(ff_f) );
		--FF(ff_offset);
	}
	else
		if( FF(ff_offset) > 0 )
			--FF(ff_offset);

	return;
}





//
//
//
int ffputc( int value, FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffputc: can't find handle.\n");
		return TRUE;
	}

	if( FF(ff_mode) != FF_WRITE ) {
		xprintf("ffputc: mode not defined.\n");
		return 0;
	}

	// xprintf("%c",value);

	if( (FF(ff_offset) + 1) >= FF(ff_alloced) )  {
		FF(ff_alloced) = FF(ff_offset) + 1024;
		REALLOCMEM( FF(ff_data), FF(ff_alloced) );
	}

	FF(ff_data)[FF(ff_offset)] = value;

	++FF(ff_offset);

	if( FF(ff_offset) > FF(ff_size) )
		FF(ff_size) = FF(ff_offset);

	return value;
}





//
//
//
int ffseek( FILE *f, int offset, int whence ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffseek: can't find handle.\n");
		return TRUE;
	}

	if( FF(ff_mode) == FF_REAL )
		fseek( FF(ff_f), offset, whence );

	if( whence == SEEK_SET )
		FF(ff_offset) = offset;

	if( whence == SEEK_CUR )
		FF(ff_offset) += offset;

	if( whence == SEEK_END )
		FF(ff_offset) = FF(ff_size) + offset;

	return 0;
}






//
//
//
int fftell( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("fftell: can't find handle.\n");
		return (-1);
	}

	if( FF(ff_mode) == FF_NONE ) return (-1);

	return FF(ff_offset);
}




//
//
//
int ffilelength( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffilelength: can't find handle.\n");
		return TRUE;
	}

	if( FF(ff_mode) == FF_NONE ) return (-1);

	return FF(ff_size);
}





//
//
//
BOOL ffgetcs( char *string, FILE *f ) {

	int i;

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffgetcs: can't find handle.\n");
		return TRUE;
	}

	i = 0;
	while( (string[i++] = ffgetc(f) ) != 0 );

	return TRUE;
}




//
//
//
BOOL ffputcs( char *string, FILE *f ) {

	int i;

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffputcs: can't find handle.\n");
		return FALSE;
	}

	i = 0;
	do { ffputc( string[i], f ); } while( string[i++] != 0 );

	return TRUE;
}





//
//
//
char *ffgets( char *string, int num, FILE *f ) {

	int i,c;

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffgets: can't find handle.\n");
		return NULL;
	}

	for( i=0; i<num; i++ ) {

		c = ffgetc(f);

		if( c == EOF ) {
			if( i == 0 )
				return NULL;
			else
				break;
		}

		string[i] = c;

		if( (string[i] == 13) || (string[i] == 10) ) {
			// ++i;
			c = ffgetc(f);
			if( c == EOF )
				break;
			if( (c != 13) || (c != 10) )
				ffungetc( c, f );
			break;
		}
	}

	string[i] = 0;

	return string;
}






//
//
//
UCHAR *ffdata( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffdata: can't find handle.\n");
		return NULL;
	}

	return FF(ff_data);
}


//
//
//
int ffsize( FILE *f ) {

	if( (ff_num = SearchFile(f)) == INVALIDHANDLE ) {
		xprintf("ffsize: can't find handle.\n");
		return TRUE;
	}

	return FF(ff_size);
}


#undef SPC
#undef STP

#define SPC	01
#define STP	02

#undef mSHORT
#undef mREGULAR
#undef mLONG
#undef mLONGDOUBLE
#undef mINT
#undef mFLOAT


#define mSHORT			0
#define mREGULAR		1
#define mLONG			2
#define mLONGDOUBLE		4
#define mINT			0
#define mFLOAT			1

static int _innum(int **ptr, int type, int len, int size, FILE *iop,
		  int (*scan_getc)(FILE *), void (*scan_ungetc)(int, FILE *),
		  int *eofptr);
static int _instr(char *ptr, int type, int len, FILE *iop,
		  int (*scan_getc)(FILE *), void (*scan_ungetc)(int, FILE *),
		  int *eofptr);
static const char *_getccl(const unsigned char *s);

int _doscan_low(FILE *iop, int (*scan_getc)(FILE *), void (*scan_ungetc)(int, FILE *), const char *fmt, void **argp);

static char _sctab[256] = {
	0,0,0,0,0,0,0,0,
	0,SPC,SPC,SPC,SPC,SPC,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	SPC,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

static int nchars = 0;


//
//
//
int _doscan( FILE *iop, const char *fmt, void **argp ) {

	return _doscan_low(iop, ffgetc, ffungetc, fmt, argp);
}


//
//
//
int _doscan_low( FILE *iop, int (*scan_getc)(FILE *), void (*scan_ungetc)(int, FILE *), const char *fmt, void **argp ) {

	int ch;
	int nmatch, len, ch1;
	int **ptr, fileended, size;

	nchars = 0;
	nmatch = 0;
	fileended = 0;

	for( ;; )

		switch( ch = *fmt++ ) {

			case '\0':

				return (nmatch);

			case '%':

				if ((ch = *fmt++) == '%')
					goto def;
				ptr = 0;
				if (ch != '*')
					ptr = (int **)argp++;
				else
					ch = *fmt++;
				len = 0;
				size = mREGULAR;

				while (isdigit(ch)) {
					len = len*10 + ch - '0';
					ch = *fmt++;
				}

				if (len == 0)
					len = 30000;

				if (ch=='l') {
					size = mLONG;
					ch = *fmt++;
					if (ch=='l') {
						size = mLONGDOUBLE; /* for long long 'll' format */
						ch = *fmt++;
					}
				}
				else
				if (ch=='h') {
					size = mSHORT;
					ch = *fmt++;
				}
				else
				if (ch=='L') {
					size = mLONGDOUBLE;
					ch = *fmt++;
				}
				else
				if (ch=='[')
					fmt = _getccl((const unsigned char *)fmt);

				if( isupper(ch) ) {
					/* ch = tolower(ch);
					gcc gives warning: ANSI C forbids braced
					groups within expressions */
					ch += 'a' - 'A';
					if (size==mLONG)
						size = mLONGDOUBLE;
					else
						size = mLONG;
				}

				if (ch == '\0')
					return(-1);

				if(ch == 'n') {
					if (!ptr)
						break;
					if (size==mLONG)
						**(long**)ptr = nchars;
					else
					if (size==mSHORT)
						**(short**)ptr = nchars;
					//else if (size==mLONGDOUBLE)
					//  **(long long**)ptr = nchars;
					else
						**(int**)ptr = nchars;
					break;
				}

				if (_innum(ptr, ch, len, size, iop, scan_getc, scan_ungetc, &fileended)) {
					if (ptr)
						nmatch++;
				}
				else {
					if (fileended && nmatch==0)
						return(-1);

					return(nmatch);
				}
			break;

			case ' ':
			case '\n':
			case '\t':
			case '\r':
			case '\f':
			case '\v':
				while (((nchars++, ch1 = scan_getc(iop))!=EOF) && (_sctab[ch1] & SPC));

				if (ch1 != EOF) {
					scan_ungetc(ch1, iop);
				}
				nchars--;
				break;

			default:
				def:
				ch1 = scan_getc(iop);
				if (ch1 != EOF) nchars++;
				if (ch1 != ch) {
					if (ch1==EOF)
						return(nmatch? nmatch: -1);
					scan_ungetc(ch1, iop);
					nchars--;
					return(nmatch);
				}
		}

		return 0;
}



//
//
//
static int _innum(int **ptr, int type, int len, int size, FILE *iop,
					int (*scan_getc)(FILE *), void (*scan_ungetc)(int, FILE *), int *eofptr ) {

	char *np;
	char numbuf[64];
	int c, base;
	int expseen, scale, negflg, c1, ndigit;
	//long long lcval;
	long lcval;
	int cpos;

	if (type=='c' || type=='s' || type=='[')
		return(_instr(ptr? *(char **)ptr: (char *)NULL, type, len, iop, scan_getc, scan_ungetc, eofptr));

	lcval = 0;
	ndigit = 0;
	scale = mINT;

	if (type=='e'||type=='f'||type=='g')
		scale = mFLOAT;

	base = 10;

	if (type=='o')
		base = 8;
	else
	if (type=='x')
		base = 16;

	np = numbuf;
	expseen = 0;
	negflg = 0;

	while (((nchars++, c = scan_getc(iop)) != EOF) && (_sctab[c] & SPC));

	if (c == EOF) nchars--;

	if (c=='-') {
		negflg++;
		*np++ = c;
		c = scan_getc(iop);
		nchars++;
		len--;
	}
	else
	if (c=='+') {
		len--;
		c = scan_getc(iop);
		nchars++;
	}

	cpos = 0;

	for( ; --len>=0; *np++ = c, c = scan_getc(iop), nchars++) {
		cpos++;

		if (c == '0' && cpos == 1 && type == 'i')
			base = 8;

		if ((c == 'x' || c == 'X') && (type == 'i' || type == 'x') 	&& cpos == 2 && lcval == 0) {
			base = 16;
			continue;
		}

		if (isdigit(c) || (base==16 && (('a'<=c && c<='f') || ('A'<=c && c<='F')))) {
			ndigit++;
			if (base==8)
				lcval <<=3;
			else
			if (base==10)
				lcval = ((lcval<<2) + lcval)<<1;
			else
				lcval <<= 4;

			c1 = c;

			if (isdigit(c))
				c -= '0';
			else
			if ('a'<=c && c<='f')
				c -= 'a'-10;
			else
				c -= 'A'-10;

			lcval += c;
			c = c1;

			continue;
		}
		else
		if (c=='.') {
			if (base!=10 || scale==mINT)
				break;
			ndigit++;
			continue;
		}
		else
		if ((c=='e'||c=='E') && expseen==0) {
			if (base!=10 || scale==mINT || ndigit==0)
				break;
			expseen++;
			*np++ = c;
			c = scan_getc(iop);
			nchars++;
			if (c!='+'&&c!='-'&&('0'>c||c>'9'))
				break;
		}
		else
			break;
	}

	if (negflg)
		lcval = -lcval;

	if (c != EOF) {
		scan_ungetc(c, iop);
		*eofptr = 0;
	}
	else
		*eofptr = 1;

	nchars--;

	if (np==numbuf || (negflg && np==numbuf+1) ) /* gene dykes*/
		return(0);

	if (ptr==NULL)
		return(1);

	*np++ = 0;
	switch((scale<<4) | size) {

		case (mFLOAT<<4) | mSHORT:
		case (mFLOAT<<4) | mREGULAR:
			**(float **)ptr = (FLOAT)atof(numbuf);
			break;

		case (mFLOAT<<4) | mLONG:
			**(double **)ptr = atof(numbuf);
			break;

		//case (mFLOAT<<4) | mLONGDOUBLE:
		//  **(long double **)ptr = _atold(numbuf);
		//  break;

		case (mINT<<4) | mSHORT:
			**(short **)ptr = (short)lcval;
			break;

		case (mINT<<4) | mREGULAR:
			**(int **)ptr = (int)lcval;
			break;

		case (mINT<<4) | mLONG:
			**(long **)ptr = lcval;
			break;

		//case (mINT<<4) | mLONGDOUBLE:
		//  **(long long **)ptr = lcval;
		//  break;
	}

	return(1);
}


//
//
//
static int _instr(char *ptr, int type, int len, FILE *iop, int (*scan_getc)(FILE *), void (*scan_ungetc)(int, FILE *), int *eofptr) {

	int ch;
	char *optr;
	int ignstp;

	*eofptr = 0;
	optr = ptr;

	if (type=='c' && len==30000)
		len = 1;

	ignstp = 0;

	if (type=='s')
		ignstp = SPC;

	while ((nchars++, ch = scan_getc(iop)) != EOF && _sctab[ch] & ignstp);

	ignstp = SPC;

	if (type=='c')
		ignstp = 0;
	else
	if (type=='[')
		ignstp = STP;

	while (ch!=EOF && (_sctab[ch]&ignstp)==0) {
		if (ptr)
			*ptr++ = ch;
		if (--len <= 0)
			break;
		ch = scan_getc(iop);
		nchars++;
	}

	if (ch != EOF) {
		if (len > 0) {
			scan_ungetc(ch, iop);
			nchars--;
		}
		*eofptr = 0;
	}
	else {
		nchars--;
		*eofptr = 1;
	}

	if (!ptr)
		return(1);

	if (ptr!=optr) {
		if (type!='c')
			*ptr++ = '\0';
		return(1);
	}

	return(0);
}



//
//
//
static const char *_getccl( const unsigned char *s ) {

	int c, t;

	t = 0;

	if( *s == '^' ) {
		t++;
		s++;
	}

	for (c = 0; c < (sizeof _sctab / sizeof _sctab[0]); c++)
		if (t)
			_sctab[c] &= ~STP;
		else
			_sctab[c] |= STP;

	if ((c = *s) == ']' || c == '-') { /* first char is special */
		if (t)
			_sctab[c] |= STP;
		else
			_sctab[c] &= ~STP;
		s++;
	}

	while ((c = *s++) != ']') {
		if (c==0)
			return((const char *)--s);
    	else
		if (c == '-' && *s != ']' && s[-2] < *s) {
			for (c = s[-2] + 1; c < *s; c++)
				if (t)
					_sctab[c] |= STP;
				else
					_sctab[c] &= ~STP;
		}
		else
		if (t)
			_sctab[c] |= STP;
		else
			_sctab[c] &= ~STP;
	}

	return((const char *)s);
}



//
//
//
int ffscanf( FILE *f, const char *fmt, ... ) {

	int r;
	va_list a=0;

	va_start(a, fmt);
	r = _doscan(f, fmt, (void **)a);
	va_end(a);

	return r;
}



static char decimal = '.';

/* 11-bit exponent (VAX G floating point) is 308 decimal digits */
#define MAXEXP		308
#define MAXEXPLD	4952 /* this includes subnormal numbers */
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define MAXFRACT	39

#define DEFPREC 	6
#define DEFLPREC	6

#define BUF		(MAXEXPLD+MAXFRACT+1)	/* + decimal point */

#define PUTC(ch)	(void)ffputc( ch, fp )

#define ARG(basetype) _ulonglong = \
		flags&LONGINT ? va_arg(argp, long basetype) : \
		flags&SHORTINT ? (short basetype)va_arg(argp, int) : \
		va_arg(argp, int)

static int xlib_nan = 0;

//
//
//
static int todigit( char c ) {

	if( c <= '0' ) return 0;
	if( c >= '9' ) return 9;

	return c-'0';
}


//
//
//
static char tochar( int n ) {

	if( n >= 9 ) return '9';
	if( n <= 0 ) return '0';

	return n+'0';
}


/* have to deal with the negative buffer count kludge */

#define LONGINT 	0x01		/* long integer */
#define LONGDBL 	0x02		/* long double */
#define SHORTINT	0x04		/* short integer */
#define ALT		0x08		/* alternate form */
#define LADJUST 	0x10		/* left adjustment */
#define ZEROPAD 	0x20		/* zero (as opposed to blank) pad */
#define HEXPREFIX	0x40		/* add 0x or 0X prefix */

static int cvtl( long double number, int prec, int flags, char *signp, unsigned char fmtch, char *startp, char *endp );
static char *roundl( long double fract, int *expv, char *start, char *end, char ch, char *signp );
static char *exponentl( char *p, int expv, unsigned char fmtch );
static int isspeciall( long double d, char *bufp );

static char NULL_REP[] = "(null)";


//
//
//
static int _doprnt( const char *fmt0, va_list argp, FILE *fp ) {

	const char *fmt;		/* format string */
	int ch;				/* character from fmt */
	int cnt;			/* return value accumulator */
	int n;				/* random handy integer */
	char *t;			/* buffer pointer */
	long double _ldouble; 		/* double and long double precision arguments %L.[eEfgG] */
	//unsigned long long _ulonglong; /* integer arguments %[diouxX] */
	unsigned long _ulonglong; 	/* integer arguments %[diouxX] */
	int base;			/* base for [diouxX] conversion */
	int dprec;			/* decimal precision in [diouxX] */
	int fieldsz;			/* field size expanded by sign, etc */
	int flags;			/* flags as above */
	int fpprec;			/* `extra' floating precision in [eEfgG] */
	int prec;			/* precision from format (%.3d), or -1 */
	int realsz;			/* field size expanded by decimal precision */
	int size;			/* size of converted field or string */
	int width;			/* width from format (%8d), or 0 */
	char sign;			/* sign prefix (' ', '+', '-', or \0) */
	char softsign;			/* temporary negative sign for floats */
	const char *digs;		/* digits for [diouxX] conversion */
	char buf[BUF];			/* space for %c, %[diouxX], %[eEfgG] */

	// decimal = localeconv()->decimal_point[0];
	decimal = '.';
	struct lconv *lcp = localeconv();
	if( lcp != NULL && lcp->decimal_point != NULL && *lcp->decimal_point != '\0' )
		decimal = *lcp->decimal_point;

	/***
	if( fp->_flag & _IORW ) {
		fp->_flag |= _IOWRT;
		fp->_flag &= ~(_IOEOF|_IOREAD);
	}

	if( (fp->_flag & _IOWRT) == 0 )
		return (EOF);
	***/

	fmt = fmt0;
	digs = "0123456789abcdef";
	for( cnt = 0; ; ++fmt ) {

		while( (ch = *fmt) && ch != '%' ) {
			PUTC (ch);
			fmt++;
			cnt++;
		}

		if( !ch )
			return cnt;

		flags = 0; dprec = 0; fpprec = 0; width = 0;
		prec = -1;
		sign = '\0';

	rflag:
		switch( *++fmt ) {
    case ' ':
      /*
       * ``If the space and + flags both appear, the space
       * flag will be ignored.''
       *	-- ANSI X3J11
       */
      if (!sign)
	sign = ' ';
      goto rflag;
    case '#':
      flags |= ALT;
      goto rflag;
    case '*':
      /*
       * ``A negative field width argument is taken as a
       * - flag followed by a  positive field width.''
       *	-- ANSI X3J11
       * They don't exclude field widths read from args.
       */
      if ((width = va_arg(argp, int)) >= 0)
	goto rflag;
      width = -width;
      /* FALLTHROUGH */
    case '-':
      flags |= LADJUST;
      goto rflag;
    case '+':
      sign = '+';
      goto rflag;
    case '.':
      if (*++fmt == '*')
	n = va_arg(argp, int);
      else
      {
	n = 0;
	while (isascii(*fmt) && isdigit(*fmt))
	  n = 10 * n + todigit(*fmt++);
	--fmt;
      }
      prec = n < 0 ? -1 : n;
      goto rflag;
    case '0':
      /*
       * ``Note that 0 is taken as a flag, not as the
       * beginning of a field width.''
       *	-- ANSI X3J11
       */
      flags |= ZEROPAD;
      goto rflag;
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      n = 0;
      do {
	n = 10 * n + todigit(*fmt);
      } while (isascii(*++fmt) && isdigit(*fmt));
      width = n;
      --fmt;
      goto rflag;
    case 'L':
      flags |= LONGDBL;
      goto rflag;
    case 'h':
      flags |= SHORTINT;
      goto rflag;
    case 'l':
      if (flags&LONGINT)
	flags |= LONGDBL; /* for 'll' - long long */
      else
	flags |= LONGINT;
      goto rflag;
    case 'c':
      *(t = buf) = va_arg(argp, int);
      size = 1;
      sign = '\0';
      goto pforw;
    case 'D':
      flags |= LONGINT;
      /*FALLTHROUGH*/
    case 'd':
    case 'i':
      ARG(int);
      if ((long )_ulonglong < 0)
      {
	_ulonglong = -_ulonglong;
	sign = '-';
      }
      base = 10;
      goto number;
    case 'e':
    case 'E':
    case 'f':
    case 'g':
    case 'G':
      if (flags & LONGDBL)
	_ldouble = va_arg(argp, long double);
      else
	_ldouble = (long double)va_arg(argp, double);
      /*
       * don't do unrealistic precision; just pad it with
       * zeroes later, so buffer size stays rational.
       */
      if (prec > MAXFRACT)
      {
	if (*fmt != 'g' && (*fmt != 'G' || (flags&ALT)))
	  fpprec = prec - MAXFRACT;
	prec = MAXFRACT;
      }
      else if (prec == -1)
      {
	if (flags&LONGINT)
	  prec = DEFLPREC;
	else
	  prec = DEFPREC;
      }
      /*
       * softsign avoids negative 0 if _double is < 0 and
       * no significant digits will be shown
       */
      if (_ldouble < 0)
      {
	softsign = '-';
	_ldouble = -_ldouble;
      }
      else
	softsign = 0;
      /*
       * cvt may have to round up past the "start" of the
       * buffer, i.e. ``intf("%.2f", (double)9.999);'';
       * if the first char isn't NULL, it did.
       */
      *buf = 0;
      size = cvtl(_ldouble, prec, flags, &softsign, *fmt, buf,
		  buf + sizeof(buf));
      if (softsign && !xlib_nan)
	sign = '-';
      xlib_nan = 0;
      t = *buf ? buf : buf + 1;
      goto pforw;
    case 'n':
      if (flags & LONGINT)
	*va_arg(argp, long *) = cnt;
      else if (flags & SHORTINT)
	*va_arg(argp, short *) = cnt;
      else
	*va_arg(argp, int *) = cnt;
      break;
    case 'O':
      flags |= LONGINT;
      /*FALLTHROUGH*/
    case 'o':
      ARG(unsigned);
      base = 8;
      goto nosign;
    case 'p':
      /*
       * ``The argument shall be a pointer to void.  The
       * value of the pointer is converted to a sequence
       * of printable characters, in an implementation-
       * defined manner.''
       *	-- ANSI X3J11
       */
      /* NOSTRICT */
      _ulonglong = (unsigned long)va_arg(argp, void *);
      base = 16;
      goto nosign;
    case 's':
      if (!(t = va_arg(argp, char *)))
	t = NULL_REP;
      if (prec >= 0)
      {
	/*
	 * can't use strlen; can only look for the
	 * NUL in the first `prec' characters, and
	 * strlen() will go further.
	 */
	char *p 		/*, *memchr() */;

	if ((p = (char *)memchr(t, 0, prec)))
	{
	  size = p - t;
	  if (size > prec)
	    size = prec;
	}
	else
	  size = prec;
      }
      else
	size = strlen(t);
      sign = '\0';
      goto pforw;
    case 'U':
      flags |= LONGINT;
      /*FALLTHROUGH*/
    case 'u':
      ARG(unsigned);
      base = 10;
      goto nosign;
    case 'X':
      digs = "0123456789ABCDEF";
      /* FALLTHROUGH */
    case 'x':
      ARG(unsigned);
      base = 16;
      /* leading 0x/X only if non-zero */
      if (flags & ALT && _ulonglong != 0)
	flags |= HEXPREFIX;

    nosign:
      /* unsigned conversions */
      sign = '\0';
    number:
	/*
	 * ``... diouXx conversions ... if a precision is
	 * specified, the 0 flag will be ignored.''
	 *	-- ANSI X3J11
	 */
	if( (dprec = prec) >= 0 )
		flags &= ~ZEROPAD;

      /*
       * ``The result of converting a zero value with an
       * explicit precision of zero is no characters.''
       *	-- ANSI X3J11
       */
      t = buf + BUF;

      if (_ulonglong != 0 || prec != 0)
      {
	/* conversion is done separately since operations
	  with long long are much slower */
#define CONVERT(type) \
	{ \
	  type _n = (type)_ulonglong; \
	  do { \
	    *--t = digs[_n % base]; \
	    _n /= base; \
	  } while (_n); \
	}
	  CONVERT(unsigned long) /* no ; */
#undef CONVERT
	if (flags & ALT && base == 8 && *t != '0')
	  *--t = '0';           /* octal leading 0 */
      }

      digs = "0123456789abcdef";
      size = buf + BUF - t;

    pforw:
      /*
       * All reasonable formats wind up here.  At this point,
       * `t' points to a string which (if not flags&LADJUST)
       * should be padded out to `width' places.  If
       * flags&ZEROPAD, it should first be prefixed by any
       * sign or other prefix; otherwise, it should be blank
       * padded before the prefix is emitted.  After any
       * left-hand padding and prefixing, emit zeroes
       * required by a decimal [diouxX] precision, then print
       * the string proper, then emit zeroes required by any
       * leftover floating precision; finally, if LADJUST,
       * pad with blanks.
       */

      /*
       * compute actual size, so we know how much to pad
       * fieldsz excludes decimal prec; realsz includes it
       */
      fieldsz = size + fpprec;
      realsz = dprec > fieldsz ? dprec : fieldsz;
      if (sign)
	realsz++;
      if (flags & HEXPREFIX)
	realsz += 2;

      /* right-adjusting blank padding */
      if ((flags & (LADJUST|ZEROPAD)) == 0 && width)
	for (n = realsz; n < width; n++)
	  PUTC(' ');
      /* prefix */
      if (sign)
	PUTC(sign);
      if (flags & HEXPREFIX)
      {
	PUTC('0');
	PUTC((char)*fmt);
      }
      /* right-adjusting zero padding */
      if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
	for (n = realsz; n < width; n++)
	  PUTC('0');
      /* leading zeroes from decimal precision */
      for (n = fieldsz; n < dprec; n++)
	PUTC('0');

      /* the string or number proper */
      for (n = size; n > 0; n--)
	PUTC(*t++);
      /* trailing f.p. zeroes */
      while (--fpprec >= 0)
	PUTC('0');
      /* left-adjusting padding (always blank) */
      if (flags & LADJUST)
	for (n = realsz; n < width; n++)
	  PUTC(' ');
      /* finally, adjust cnt */
      cnt += width > realsz ? width : realsz;
      break;
    case '\0':                  /* "%?" prints ?, unless ? is NULL */
      return cnt;
    default:
      PUTC((char)*fmt);
      cnt++;
    }
  }
  /* NOTREACHED */
}


static long double pten[] = {
	1e1L, 1e2L, 1e4L, 1e8L, 1e16L, 1e32L, 1e64L, 1e128L, 1e256L,
	/* 1e512L, 1e1024L, 1e2048L, 1e4096L */
};

static long double ptenneg[] = {
	1e-1L, 1e-2L, 1e-4L, 1e-8L, 1e-16L, 1e-32L, 1e-64L, 1e-128L, 1e-256L,
	1e-512L, 1e-1024L, 1e-2048L, 1e-4096L
};

#define MAXP 4096
#define NP   12
#define P    (4294967296.0L * 4294967296.0L * 2.0L)   /* 2^65 */
static long double INVPREC = P;
static long double PREC = 1.0L/P;
#undef P

/*
 * Defining FAST_LDOUBLE_CONVERSION results in a little bit faster
 * version, which might be less accurate (about 1 bit) for long
 * double. For 'normal' double it doesn't matter.
 */
/* #define FAST_LDOUBLE_CONVERSION */

static int cvtl( long double number, int prec, int flags, char *signp, unsigned char fmtch, char *startp, char *endp ) {

	char *p, *t;
	long double fract;
	int dotrim, expcnt, gformat;
	long double integer, tmp;

	if( (expcnt = isspeciall(number, startp)) )
		return(expcnt);

	dotrim = expcnt = gformat = 0;
	/* fract = modfl(number, &integer); */
	integer = number;

  /* get an extra slot for rounding. */
  t = ++startp;

  p = endp - 1;
  if (integer)
  {
    int i, lp=NP, pt=MAXP;
#ifndef FAST_LDOUBLE_CONVERSION
    long double oint = integer, dd=1.0L;
#endif
    if (integer > INVPREC)
    {
      integer *= PREC;
      while(lp >= 0) {
	if (integer >= pten[lp])
	{
	  expcnt += pt;
	  integer *= ptenneg[lp];
#ifndef FAST_LDOUBLE_CONVERSION
	  dd *= pten[lp];
#endif
	}
	pt >>= 1;
	lp--;
      }
#ifndef FAST_LDOUBLE_CONVERSION
      integer = oint/dd;
#else
      integer *= INVPREC;
#endif
    }
    /*
     * Do we really need this ?
     */
    for (i = 0; i < expcnt; i++)
      *p-- = '0';
  }
  number = integer;
  fract = modfl(number, &integer);
  /*
   * get integer portion of number; put into the end of the buffer; the
   * .01 is added for modf(356.0 / 10, &integer) returning .59999999...
   */
  for (; integer; ++expcnt)
  {
    tmp = modfl(integer * 0.1L , &integer);
    *p-- = tochar((int)((tmp + .01L) * 10));
  }
  switch(fmtch)
  {
  case 'f':
    /* reverse integer into beginning of buffer */
    if (expcnt)
      for (; ++p < endp; *t++ = *p);
    else
      *t++ = '0';
    /*
     * if precision required or alternate flag set, add in a
     * decimal point.
     */
    if (prec || flags&ALT)
      *t++ = decimal;
    /* if requires more precision and some fraction left */
    if (fract)
    {
      if (prec)
	do {
	  fract = modfl(fract * 10.0L, &tmp);
	  *t++ = tochar((int)tmp);
	} while (--prec && fract);
      if (fract)
	startp = roundl(fract, (int *)NULL, startp,
			t - 1, (char)0, signp);
    }
    for (; prec--; *t++ = '0');
    break;
  case 'e':
  case 'E':
  eformat:
    if (expcnt)
    {
      *t++ = *++p;
      if (prec || flags&ALT)
	*t++ = decimal;
      /* if requires more precision and some integer left */
      for (; prec && ++p < endp; --prec)
	*t++ = *p;
      /*
       * if done precision and more of the integer component,
       * round using it; adjust fract so we don't re-round
       * later.
       */
      if (!prec && ++p < endp)
      {
	fract = 0;
	startp = roundl((long double)0.0L, &expcnt,
			startp, t - 1, *p, signp);
      }
      /* adjust expcnt for digit in front of decimal */
      --expcnt;
    }
    /* until first fractional digit, decrement exponent */
    else if (fract)
    {
      int lp=NP, pt=MAXP;
#ifndef FAST_LDOUBLE_CONVERSION
      long double ofract = fract, dd=1.0L;
#endif
      expcnt = -1;
      if (fract < PREC)
      {
	fract *= INVPREC;
	while(lp >= 0)
	{
	  if (fract <= ptenneg[lp])
	  {
	    expcnt -= pt;
	    fract *= pten[lp];
#ifndef FAST_LDOUBLE_CONVERSION
	    dd *= pten[lp];
#endif
	  }
	  pt >>= 1;
	  lp--;
	}
#ifndef FAST_LDOUBLE_CONVERSION
	fract = ofract*dd;
#else
	fract *= PREC;
#endif
      }
      /* adjust expcnt for digit in front of decimal */
      for (			/* expcnt = -1 */ ;; --expcnt)
      {
	fract = modfl(fract * 10.0L, &tmp);
	if (tmp)
	  break;
      }
      *t++ = tochar((int)tmp);
      if (prec || flags&ALT)
	*t++ = decimal;
    }
    else
    {
      *t++ = '0';
      if (prec || flags&ALT)
	*t++ = decimal;
    }
    /* if requires more precision and some fraction left */
    if (fract)
    {
      if (prec)
	do {
	  fract = modfl(fract * 10.0L, &tmp);
	  *t++ = tochar((int)tmp);
	} while (--prec && fract);
      if (fract)
	startp = roundl(fract, &expcnt, startp,
			t - 1, (char)0, signp);
    }
    /* if requires more precision */
    for (; prec--; *t++ = '0');

    /* unless alternate flag, trim any g/G format trailing 0's */
    if (gformat && !(flags&ALT))
    {
      while (t > startp && *--t == '0');
      if (*t == decimal)
	--t;
      ++t;
    }
    t = exponentl(t, expcnt, fmtch);
    break;
  case 'g':
  case 'G':
    /* a precision of 0 is treated as a precision of 1. */
    if (!prec)
      ++prec;
    /*
     * ``The style used depends on the value converted; style e
     * will be used only if the exponent resulting from the
     * conversion is less than -4 or greater than the precision.''
     *	-- ANSI X3J11
     */
    if (expcnt > prec || (!expcnt && fract && fract < .0001))
    {
      /*
       * g/G format counts "significant digits, not digits of
       * precision; for the e/E format, this just causes an
       * off-by-one problem, i.e. g/G considers the digit
       * before the decimal point significant and e/E doesn't
       * count it as precision.
       */
      --prec;
      fmtch -= 2;		/* G->E, g->e */
      gformat = 1;
      goto eformat;
    }
    /*
     * reverse integer into beginning of buffer,
     * note, decrement precision
     */
    if (expcnt)
      for (; ++p < endp; *t++ = *p, --prec);
    else
      *t++ = '0';
    /*
     * if precision required or alternate flag set, add in a
     * decimal point.  If no digits yet, add in leading 0.
     */
    if (prec || flags&ALT)
    {
      dotrim = 1;
      *t++ = decimal;
    }
    else
      dotrim = 0;
    /* if requires more precision and some fraction left */
    while (prec && fract)
    {
      fract = modfl(fract * 10.0L, &tmp);
      *t++ = tochar((int)tmp);
      prec--;
    }
    if (fract)
      startp = roundl(fract, (int *)NULL, startp, t - 1,
		      (char)0, signp);
    /* alternate format, adds 0's for precision, else trim 0's */
    if (flags&ALT)
      for (; prec--; *t++ = '0');
    else if (dotrim)
    {
      while (t > startp && *--t == '0');
      if (*t != decimal)
	++t;
    }
  }
  return t - startp;
}


//
//
//
static char *roundl(long double fract, int *expv, char *start, char *end, char ch, char *signp) {

	long double tmp;

	if (fract) {

		if (fract == 0.5L) {
			char *e = end;
			if (*e == '.')
				e--;

			if (*e == '0' || *e == '2' || *e == '4' || *e == '6' || *e == '8') {
				tmp = 3.0;
				goto start;
			}
		}

		(void)modfl(fract * 10.0L, &tmp);
	}
	else
		tmp = todigit(ch);
start:

	if (tmp > 4)
		for (;; --end) {
			if (*end == decimal)
				--end;
			if (++*end <= '9')
				break;

			*end = '0';
			if (end == start) {

				if (expv) {		/* e/E; increment exponent */
					*end = '1';
					++*expv;
				}
				else {			/* f; add extra digit */
					*--end = '1';
					--start;
			}
			break;
		}
	}

	/* ``"%.3f", (double)-0.0004'' gives you a negative 0. */
	else
	if (*signp == '-')
		for (;; --end) {
			if (*end == decimal)
				--end;
			if (*end != '0')
				break;
			if (end == start)
				*signp = 0;
		}

	return start;
}



//
//
//
static char *exponentl( char *p, int expv, unsigned char fmtch ) {

	char *t;
	char expbuf[MAXEXPLD];

	*p++ = fmtch;
	if( expv < 0 ) {
		expv = -expv;
		*p++ = '-';
	}
	else
		*p++ = '+';

	t = expbuf + MAXEXPLD;
	if( expv > 9 ) {

		do {
			*--t = tochar(expv % 10);
		} while( (expv /= 10) > 9 );

		*--t = tochar(expv);
		for( ; t < expbuf + MAXEXPLD; *p++ = *t++ );
	}
	else {
		*p++ = '0';
		*p++ = tochar(expv);
	}

	return p;
}



//
//
//
static int isspeciall( long double d, char *bufp ) {

	struct IEEExp {
		unsigned manl:32;
		unsigned manh:32;
		unsigned exp:15;
		unsigned sign:1;
	} *ip = (struct IEEExp *)&d;

	xlib_nan = 0;  /* don't assume the static is 0 (emacs) */

	if( ip->exp != 0x7fff )
		return 0;

	if( (ip->manh & 0x7fffffff) || ip->manl ) {
		strcpy(bufp, "NaN");
		xlib_nan = 1;		/* kludge: we don't need the sign,  it's not nice but it should work */
	}
	else
		strcpy( bufp, "Inf" );

	return 3;
}




//
//
//
int ffprintf( FILE *iop, const char *fmt, ...) {

	int len;

	len = _doprnt( fmt, (char *)((&fmt)+1), iop );

	return len;
}




