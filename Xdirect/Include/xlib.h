/* Copyright (C) 1997-99 Kirschner, Bernát. All Rights Reserved Worldwide. */
/* mailto: bernie@freemail.hu						   */
/* tel: +36 20 333 9517							   */

#ifndef _XLIB_H_INCLUDED
#define _XLIB_H_INCLUDED

// visual studio faszsága miatt kell:
// #define _CRT_SECURE_NO_DEPRECATE 1
// #pragma warning(disable : 4996)

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include <xdllconv.h>

// a debughlp.h-ból
#define VAPI(exp)	((void)(exp))
#define VERIFY(exp)	((void)(exp))

#ifdef XDLL_EXPORTS
	#ifdef __cplusplus
		#define XLIBDEF extern "C" __declspec( dllexport )
	#else
		#define XLIBDEF extern __declspec( dllexport )
	#endif
#else
	#ifdef __cplusplus
		#define XLIBDEF extern "C" __declspec( dllimport )
	#else
		#define XLIBDEF extern __declspec( dllimport )
	#endif
#endif


#ifndef F_OK
#define F_OK 0
#endif

#define XMAX_PATH  512

#ifndef PATH_MAX
#define PATH_MAX XMAX_PATH
#endif

#ifndef MAX_PATH
#define MAX_PATH XMAX_PATH
#endif


#define MAXFIXED LONG_MAX
#define MINFIXED LONG_MIN


#define MC_NONE     0
#define MC_LEFT     1
#define MC_RIGHT    2
#define MC_FORWARD  3
#define MC_BACKWARD 4
#define MC_UP	    5
#define MC_DOWN     6
#define MC_JUSTFORWARD 11

#include <xtype.h>
#include <xrcs.h>


// #define NOUNREF #pragma off (unreferenced)
#define USEPARAM(x) (x) = (x)

#define ABS(i)		( ((i) >= 0 )	 ? (i) : -(i) )
#define FABS(i) 	( ((i) >= 0.0f ) ? (i) : -(i) )
#define SGN(i)		( ((i) >  0 )	 ?  1  :  -1  )
#define FSGN(i) 	( ((i) >  0.0f ) ?  1  :  -1  )
#define MAX(i,j)	( ((i) > (j))	 ? (i) :  (j) )
#define MIN(i,j)	( ((i) < (j))	 ? (i) :  (j) )
#define MID(x,y,z)	MAX( (x), MIN( (y), (z) ) )
#define SQR(number) 	(number*number)

#define max3(x,y,z)	(((x)>(y))?(((x)>(z))?1:3):(((y)>(z))?2:3))
#define MAX3(x,y,z)	(((x)>(y))?(((x)>(z))?(x):(z)):(((y)>(z))?(y):(z)))
#define MIN3(x,y,z)	(((x)<(y))?(((x)<(z))?(x):(z)):(((y)<(z))?(y):(z)))
#define BETWEEN(a,min,max) ( (a)>=(min) && (a)<=(max) )

#define CLAMPMIN(value,minvalue) { if( (value) < (minvalue) ) (value) = (minvalue); }
#define CLAMPMAX(value,maxvalue) { if( (value) > (maxvalue) ) (value) = (maxvalue); }
#define CLAMPMINMAX(value,minvalue,maxvalue) { if( (value) < (minvalue) ) { (value) = (minvalue); } if( (value) > (maxvalue) ) { (value) = (maxvalue); } }

#define dimof(a)	( sizeof(a) / sizeof((a)[0]) )

#ifndef LOBYTE
#define LOBYTE(n)	( (n) & 0x00FF )
#endif

#ifndef HIBYTE
#define HIBYTE(n)	( ( (n)>>8 ) & 0x00FF )
#endif

#define BOOLEAN(x)	(!(!(x)))
#define RAND(x,y)	((x) + (rand()%((y)-(x)+1)))		// Get a random number
#define RANDOM_FLOAT 	(((FLOAT)rand()-(FLOAT)rand())/RAND_MAX)

#ifndef FUNC
// #define FUNC(x)
// #define _FUNCTION_ ""
#define FUNC(x)      static char _FUNCTION_[] = {x};_FUNCTION_;
#endif


// i szamban az f-edik bitet
#define ISFLAG(i,f)		BOOLEAN( ( (i)	&  ( 1L << (f) ) ) )
#define SETFLAG(i,f)		( ( (i) |=  ( 1L << (f) ) ) )
#define ERASEFLAG(i,f)		( ( (i) &= ~( 1L << (f) ) ) )
#define FLIPFLAG(i,f)		( ( (i) ^=  ( 1L << (f) ) ) )
#define MAKEFLAG(f)		( 1L << (f) )

#define SWAPL(x,y) { long temp=(x); (x)=(y); (y)=temp; }
#define SWAPF(x,y) { FLOAT temp=(x); (x)=(y); (y)=temp; }

// bit(b,n)   - get the Nth bit of BYTE b
#define BIT(b,n) (UCHAR)(((b) & (1 << (n))) >> (n))

#define NOCOLOR (0)			// transparent color
#define BLACK	(1)
#define LGREY	(2)
#define GREY	(3)
#define DGREY	(4)
#define WHITE	(255)

#define COLOR_BLACK	((ULONG)(0x000000))
#define COLOR_RED	((ULONG)(0xff0000))
#define COLOR_GREEN	((ULONG)(0x00ff00))
#define COLOR_YELLOW	((ULONG)(0xffff00))
#define COLOR_BLUE	((ULONG)(0x0000ff))
#define COLOR_PURPLE	((ULONG)(0xff00ff))
#define COLOR_CYAN	((ULONG)(0x00ffff))
#define COLOR_WHITE	((ULONG)(0xffffff))
#define COLOR_LT_GRAY	((ULONG)(0xc0c0c0))
#define COLOR_MD_GRAY	((ULONG)(0x808080))
#define COLOR_DK_GRAY	((ULONG)(0x404040))
#define COLOR_BROWN	((ULONG)(0x804000))
#define COLOR_ORANGE	((ULONG)(0xff7f00))


// if Windoze
#ifndef PATHDELIM
#define PATHDELIM '\\'
#endif

#ifndef PATHDELIMSTR
#define PATHDELIMSTR "\\"
#endif



#ifdef __cplusplus
extern "C" {
#endif

// #define VAL	(5)


#ifndef NO3DH

#ifdef INCCOLDET
#include "c:\\MYPROJ~1\\Coldet\\src\\coldet.h"
#endif

#include <xfixed.h>
#include <x3d.h>

#endif  // NO3DH


// xdllmain.cpp

#ifdef __cplusplus
XLIBDEF BOOL CheckCreateMutex( char *name, char *filename=NULL );
#endif

// spr.c

XLIBDEF BOOL IsSpr( char *name );
XLIBDEF BOOL WriteSprite(char *fname,unsigned char *b);
XLIBDEF BOOL LoadSprite(char *fname,memptr_ptr);
XLIBDEF void GetSpriteDim(char *name,int *w,int *h);

XLIBDEF void MungeSprite(UCHAR *s);
XLIBDEF void DupSprite( UCHAR *s1, memptr_ptr );

XLIBDEF void RotateScaleSprite( UCHAR *spr1, FLOAT scale, FLOAT angle, UCHAR *spr2 );
XLIBDEF void FlipHorizontalSprite( UCHAR *b );
XLIBDEF void FlipVerticalSprite( UCHAR *buf );
XLIBDEF void ColumnMajorTransformSprite( UCHAR *spr );

XLIBDEF void ScaleSprite( int dest_x, int dest_y, UCHAR *buf1, memptr_ptr buf2 );
XLIBDEF void VerticalScaleSprite( int dest_y, UCHAR *buf1, memptr_ptr buf2 );
XLIBDEF void StretchSprite( int factor, UCHAR *buf );
XLIBDEF void ScaleScanline( UCHAR *source, UCHAR *dest, int smap_size, int dmap_size, int dline_size);

// system_rgb-re lekonvertalja
XLIBDEF void CompileSprite( UCHAR *pal, UCHAR *spr1, UCHAR *spr2 );
XLIBDEF void SystemSprite( UCHAR *pal, UCHAR *spr );

XLIBDEF void BilinearAliasing( UCHAR *spr );
XLIBDEF void TrilinearAliasing( UCHAR *spr );
XLIBDEF void HyperlinearAliasing( UCHAR *spr );
XLIBDEF void ColorBilinearAliasing( UCHAR *spr );	// aliasing_rgb
XLIBDEF void AccurateColorBilinearAliasing( UCHAR *spr );


#ifdef __cplusplus
XLIBDEF void HighSprite( memptr_ptr ptr, UCHAR *pal=NULL );
XLIBDEF void TrueSprite( memptr_ptr ptr, UCHAR *pal=NULL );
XLIBDEF void GammaSprite( UCHAR *ptr, UCHAR *pal=NULL );
XLIBDEF void OutlineSprite( UCHAR *ptr, UCHAR *pal=NULL );
#endif
XLIBDEF void GreySprite( UCHAR *spr );
extern int Rgb2Gray( int red, int green, int blue );

XLIBDEF void FillSprite( UCHAR *spr, UCHAR *pal, rgb_t rgb );
XLIBDEF void FillSpriteRect( UCHAR *spr, UCHAR *pal, int x1,int y1, int x2, int y2, rgb_t rgb );
XLIBDEF void CheckerTexture( UCHAR **orig_spr, int width, int height );
XLIBDEF void ColorTexture( UCHAR **orig_spr, int width, int height, int r, int g, int b );
XLIBDEF void DrawChecker( void );
XLIBDEF int AddCheckerTexmap( void );
XLIBDEF void CompositeSprite( UCHAR *dst, UCHAR *src, int x, int y );
#ifdef __cplusplus
XLIBDEF void DrawTexLine( int line_x, int line_y, int line_x1, int line_y1, int color=0, int border=1, int alpha=255 );
XLIBDEF void DrawTexRect( int x1, int y1, int x2, int y2, int color=1, int alpha=255 );
#endif

#define BIGSPR_NOFLAG 	0
#define BIGSPR_FLIPHOR 	(1<<1)
#define BIGSPR_FLIPVERT (1<<2)
#define BIGSPR_FLIPH (BIGSPR_FLIPHOR)
#define BIGSPR_FLIPV (BIGSPR_FLIPVERT)


#ifdef __cplusplus
XLIBDEF int LoadBigSprite( char *filename, BOOL alpha=FALSE, BOOL chroma=FALSE, BOOL force_reload=FALSE, int load_flag = BIGSPR_NOFLAG, int cut_x = -1, int cut_y = -1, int cut_w = -1, int cut_h = -1, FLOAT fok=0.0f, BOOL fromSprite=FALSE );
XLIBDEF int ReloadBigSprite( int handle, int load_flag = BIGSPR_NOFLAG );
XLIBDEF void PutBigSprite( int handle, int x, int y, int width=-1, int height=-1, int alpha=-1, BOOL chroma=FALSE );
XLIBDEF void PutBigSpritePart( int handle, int screenx, int screeny, int width, int height, int sx=-1, int sy=-1, int partw=-1, int parth=-1, int alpha=-1 );
XLIBDEF BOOL GetBigSpriteParam( int handle, int *w=NULL, int *h=NULL, BOOL *alpha=NULL, BOOL *chroma=NULL, int *bpp=NULL, int *pages=NULL, int *delay=NULL, FLOAT *fok=NULL, point_t *bbox=NULL, int *curr_page=NULL );
XLIBDEF BOOL SetBigSpriteParam( int handle, int alpha=-1, int chroma=-1, int delay=-1, FLOAT fok=0.0f, int page=-1 );
XLIBDEF BOOL GetBigSpritePixel( int handle, int x, int y, int *r=NULL, int *g=NULL, int *b=NULL );
XLIBDEF BOOL ModifyBigSprite( int handle, UCHAR *spr, UCHAR *pal=NULL, int flag=-1 );
XLIBDEF BOOL SpriteSheet( char *txt, char *outname=(char *)"sprite-sheet", int maxtexw=(1024*4), int maxtexh=(1024*4) );
#endif
XLIBDEF UCHAR *GetBigSpriteData( int handle );
XLIBDEF void FreeBigSprite( int handle );
XLIBDEF void DiscardAllBigSprite( void );
XLIBDEF int IsBigSpriteLoaded( char *filename );


#define ATLASLINELEN ( sizeof(int)*4 + XMAX_PATH+1 )
#define ATLASX(atlas,num) *((int *)( (UCHAR*)(atlas) + ATLASLINELEN*num + sizeof(int)*0 ))
#define ATLASY(atlas,num) *((int *)( (UCHAR*)(atlas) + ATLASLINELEN*num + sizeof(int)*1 ))
#define ATLASW(atlas,num) *((int *)( (UCHAR*)(atlas) + ATLASLINELEN*num + sizeof(int)*2 ))
#define ATLASH(atlas,num) *((int *)( (UCHAR*)(atlas) + ATLASLINELEN*num + sizeof(int)*3 ))
#define ATLASNAME(atlas,num) ((char *)( (UCHAR*)(atlas) + ATLASLINELEN*num + sizeof(int)*4 ))

XLIBDEF int LoadAtlas( char *name, memptr_ptr ptr );


#define PUTPICTURE(name,x,y) { PutBigSprite( LoadBigSprite( (name) ), (x), (y) ); }
// BOOL alpha, chroma
#define PUTPICTUREPARAM(name,x,y,bAlpha,bChroma) { PutBigSprite( LoadBigSprite( (name), (bAlpha)?TRUE:FALSE, (bChroma) ), (x), (y), -1,-1, (bAlpha)?255:-1, (bChroma)  ); }
#define PUTPICTUREPARAMSCALE(name,x,y,bAlpha,bChroma,iScalew,iScaleh) { PutBigSprite( LoadBigSprite( (name), (bAlpha)?TRUE:FALSE, (bChroma) ), (x), (y), (iScalew),(iScaleh), (bAlpha)?255:-1, (bChroma)  ); }
#define PUTPICTUREPARAMSCALEFLAG(name,x,y,bAlpha,bChroma,iScalew,iScaleh,flag) { PutBigSprite( LoadBigSprite( (name), (bAlpha)?TRUE:FALSE, (bChroma), FALSE, flag ), (x), (y), (iScalew),(iScaleh), (bAlpha)?255:-1, (bChroma)  ); }
#define PUTPICTUREPARAMSCALEFLAGROT(name,x,y,bAlpha,bChroma,iScalew,iScaleh,flag,fok) { 		\
	int handle = LoadBigSprite( (name), (bAlpha)?TRUE:FALSE, (bChroma), FALSE, flag, fok ); 	\
	if( fok != 0.0f ) SetBigSpriteParam( handle, -1, -1, -1, fok );					\
	PutBigSprite( handle, (x), (y), (iScalew),(iScaleh), (bAlpha)?255:-1, (bChroma)  ); }
// 0-255
#define PUTPICTUREFADE(name,x,y,alpha) { PutBigSprite( LoadBigSprite( (name) ), (x), (y), -1,-1, (alpha) ); }
#define PUTPICTUREPART(name,x,y,px,py,w,h) { PutBigSpritePart( LoadBigSprite( (name) ), (x), (y), (w), (h), (px), (py), (w),(h)); }

#define LOADBIGSPRITEFROMMEM(spr) LoadBigSprite( (char *)(spr), FALSE, FALSE, FALSE, BIGSPR_NOFLAG, -1, -1, -1, -1, 0.0f, TRUE )
#define LOADBIGSPRITEFROMSPRITE LOADBIGSPRITEFROMMEM

//
// rgb.c
//

#define GRAYRGB {			\
	int i;				\
	for(i=0;i<256;i++)		\
		SetRgb(i,(63*i)/256,	\
			 (63*i)/256,	\
			 (63*i)/256 );	\
	SetRgb(0,0,0,0);		\
	SetRgb(255,63,63,63);		}

#define INITRGB GetRgbBuf( system_rgb );

XLIBDEF UCHAR system_rgb[768];
XLIBDEF UCHAR *transparent_rgb;			// [256*256];
XLIBDEF UCHAR *aliasing_rgb;			// [256*256];

#define TRANSNAME "trans.pcx"
#define ALIASNAME "alias.pcx"

#define TRANSCOLOR(fore,back) transparent_rgb[(fore)+((back)<<8)]

XLIBDEF void SetRgbFile(char *n);
XLIBDEF void SetRgbFile6(char *n);
XLIBDEF void Dither(int,int,int,int,int,int,int,int);
XLIBDEF void Phong(int,int,int,int,int,int,int,int);
XLIBDEF void GetRgb(int color,int *red,int *green,int *blue);
XLIBDEF void SetRgb(int c,int r,int g,int b);
XLIBDEF void SetRgbBuf(UCHAR *pal);
/* XLIBDEF void ConvRgb( UCHAR *pal );  kurva directx */
XLIBDEF void SetRgb6(int c,int r,int g,int b);
XLIBDEF void SetRgbBuf6( UCHAR *palbuf );
XLIBDEF void SetDarkRgb(void);
XLIBDEF void GetRgbBuf(UCHAR *pal);
XLIBDEF void FadePal( UCHAR *pal );
XLIBDEF void FadeBlack(void);
XLIBDEF void MakeGreyScale( UCHAR *pal );
XLIBDEF void SetGreyRgb(void);
XLIBDEF void DeinitTransColor( void );
XLIBDEF void MakeTransColor( BOOL force_flag );
XLIBDEF void MakeAliasColor( BOOL force_flag );
XLIBDEF BOOL WriteRgbTxt( char *, UCHAR* );
XLIBDEF BOOL SetRgbTxt( char * );

XLIBDEF void PhotoNegativeRgb(void);
XLIBDEF USHORT NegateColor16( USHORT color );
XLIBDEF void AdjustRgb( CHAR r, CHAR g, CHAR b );
XLIBDEF void RotateRgb( int dist );				// distenkent leptet
XLIBDEF UCHAR FindRgb( int r, int g, int b );


// system_rgb, col[256] !!!
XLIBDEF void CompileRgb( UCHAR *pal, UCHAR *col );

XLIBDEF void Rgb2Hsv( int r, int g, int b, FLOAT *h, FLOAT *s, FLOAT *v );
XLIBDEF void Hsv2Rgb( FLOAT hue, FLOAT s, FLOAT v, int *r, int *g, int *b );

#define COPYRGB( dst, src ) {	\
	(dst).r = (src).r;	\
	(dst).g = (src).g;	\
	(dst).b = (src).b;	\
	(dst).a = (src).a;	}


// bitmap.c

XLIBDEF void bmFilledBox(int x1,int y1,int x2,int y2,UCHAR color, UCHAR *seg,int w,int h);
XLIBDEF void bmBox(int x1,int y1,int x2,int y2,unsigned char color,unsigned char *seg,int w,int h);
XLIBDEF void bmPutSprite(int x,int y,UCHAR *p,UCHAR *seg,int w,int h);
XLIBDEF void bmGetSprite(memptr_ptr,int x,int y,int w,int h,UCHAR *seg,int sw,int sh);
XLIBDEF void bmGetSpriteHigh(memptr_ptr,int x,int y,int w,int h,UCHAR *seg,int sw,int sh);
XLIBDEF void bmPutPixelNC(int x, int y, UCHAR c, UCHAR *s,int w,int h);
XLIBDEF void bmPutPixel(int x, int y, UCHAR c, UCHAR *s,int w,int h);
XLIBDEF UCHAR bmGetPixel(int x,int y,UCHAR *s,int w,int h);


// xmode.c



// speech.cpp

XLIBDEF BOOL InitSpeak( void );
XLIBDEF void DeinitSpeak( void );
XLIBDEF BOOL SetSpeakVoice( int flag );
XLIBDEF BOOL SpeakText( char *, ... );
XLIBDEF void PauseSpeak( void );
XLIBDEF void ResumeSpeak( void );
XLIBDEF void StopSpeak( void );


// espeakshell.cpp

XLIBDEF BOOL InitEspeak( void );
XLIBDEF void DeinitEspeak( void );
XLIBDEF BOOL EspeakText( char *, ... );
XLIBDEF void PauseEspeak( void );
XLIBDEF void ResumeEspeak( void );
XLIBDEF void StopEspeak( void );
#ifdef __cplusplus
XLIBDEF BOOL EspeakSetVoice( char *lang=NULL, int gender=0 );
#endif
XLIBDEF BOOL IsEspeaking( void );


// fractal.cpp

#ifdef __cplusplus
XLIBDEF void CreateFractalMap( UCHAR **hm, int size, UCHAR **cm=NULL );
#endif
XLIBDEF UCHAR *GetFractalPalette( void );
XLIBDEF UCHAR *GetFractalPalette2( void );
XLIBDEF void CreateFractalMap2( UCHAR **hm, int size );



// file.c

#define NOFLAG		0
#define LZAFLAG 	1
#define UUFLAG		2
#define RLEFLAG 	3
#define LZHFLAG 	4
#define LZSFLAG 	5
#define ZIPFLAG 	6

XLIBDEF BOOL AddSearchDir( const char *dir );
XLIBDEF int DumpSearchDir( void );
XLIBDEF FILE *XL_OpenFile( const char *name);
XLIBDEF void XL_CloseFile(FILE *f);
XLIBDEF ULONG LoadFile( const char *name, memptr_ptr );
XLIBDEF BOOL FileSeek( FILE *f,long offs,int whence );
XLIBDEF ULONG FileSize( FILE *f );
XLIBDEF ULONG FileLong(char *name );
#ifdef __cplusplus
XLIBDEF BOOL FileExist( char *name, BOOL physical=FALSE );
#endif
XLIBDEF ULONG FileDate( char * );
XLIBDEF BOOL SetFileDate( char *, ULONG );
XLIBDEF BOOL TomFlag( int );
XLIBDEF void PushTomFlag( void );
XLIBDEF void PopTomFlag( void );
XLIBDEF ULONG XL_WriteFile(char *name,void*,ULONG);
XLIBDEF BOOL XL_CreateFile( char *name );
XLIBDEF BOOL Read( UCHAR *buf, int len, FILE *f );
XLIBDEF BOOL Write( UCHAR *buf, int len, FILE *f );
XLIBDEF BOOL EraseFile( char *name );
XLIBDEF BOOL IsSameFile( char *file1, char *file2 );

#define CF_APPEND	1
#define CF_OVERWRITE	0

#ifdef __cplusplus
XLIBDEF BOOL XL_CopyFile( char *src, char *dst, int flag=CF_OVERWRITE, BOOL log=TRUE );
#endif
XLIBDEF ULONG kitom( UCHAR *buf, int len, memptr_ptr ptr );
XLIBDEF BOOL UnlinkFile( char *name );

#define FIND_SUBDIR	0
#define FIND_ARCHIVE	1
#define FIND_HIDDEN	2
#define FIND_RDONLY	3
#define FIND_SYSTEM	4

XLIBDEF BOOL FindFirst( char *mask, findfile_ptr find );
XLIBDEF BOOL FindNext( findfile_ptr find );
XLIBDEF void MyFindClose( void );

XLIBDEF BOOL cmpPattern( char *full_ext, char *pattern );


//
// merre:	 1: elõre
//		-1: visszafele
//		 2: End dir
//		-2: Begin dir

#define SNF_FOWARD	1
#define SNF_BACK	-1
#define SNF_ENDDIR	2
#define SNF_BEGINDIR	-2
#define SNF_QUERY	13


typedef enum {
	SORT_NAME = 0,
	SORT_NAME_DEC,
	SORT_DATE,
	SORT_DATE_DEC,
	SORT_SIZE,
	SORT_SIZE_DEC,
	SORT_EXT,
	SORT_EXT_DEC,
	SORT_RANDOM,
	SORT_NOTSORTED
} sort_e;


#ifdef __cplusplus
XLIBDEF char *SearchNextFile( char *curr_filename, int merre=1, sort_e sort=SORT_NAME,
				int *currEntry=NULL, int *maxEntry=NULL,
				char *pattern=NULL );
// ha mindkettõ NULL akkor eldobja
XLIBDEF BOOL CacheSubdir( char *filename = NULL, char *pattern = NULL, BOOL nosubdir = FALSE, BOOL scan_zips = FALSE );
#endif


XLIBDEF void WriteByte(FILE *f,int i);
XLIBDEF void WriteWord(FILE *f,int i);
XLIBDEF void WriteDword(FILE *f,int i);

XLIBDEF int ReadByte(FILE *f);
XLIBDEF int ReadWord(FILE *f);
XLIBDEF int ReadDword(FILE *f);

XLIBDEF int skip_whitespace( FILE *fp );
XLIBDEF BOOL GetLineTxt( FILE *, char * );
XLIBDEF BOOL GetStringTxt( FILE *, char * );
XLIBDEF BOOL GetFloatTxt( FILE *, FLOAT * );
XLIBDEF BOOL GetFixedTxt( FILE *, FIXED * );
XLIBDEF BOOL GetIntTxt( FILE *, int * );
XLIBDEF BOOL IsNextStringTxt( FILE *fp );
XLIBDEF BOOL IsNextNumberTxt( FILE *fp );


XLIBDEF FILE *ffopen( char *name, const char *mode );
XLIBDEF FILE *ffopen2( char *name, const char *mode, UCHAR *buf, int buf_len );
XLIBDEF FILE *ffopen3( char *name, const char *mode );
XLIBDEF void ffclose( FILE *f );
XLIBDEF void ffree( FILE *f );
XLIBDEF BOOL ffeof( FILE *f );
XLIBDEF int ffread( void *ptr, int size, int num, FILE *f );
XLIBDEF int ffwrite( void *ptr, int size, int num, FILE *f );
XLIBDEF int ffseek( FILE *f, int offset, int whence );
XLIBDEF int fftell( FILE *f );
XLIBDEF int ffilelength( FILE *f );
XLIBDEF int ffgetc( FILE *f );
XLIBDEF int ffputc( int value, FILE *f );
XLIBDEF void ffungetc( int c, FILE *f );
XLIBDEF int ffscanf( FILE *f, const char *fmt, ... );
XLIBDEF int ffprintf( FILE *iop, const char *fmt, ...);
XLIBDEF BOOL ffgetcs( char *string, FILE *f );
XLIBDEF BOOL ffputcs( char *string, FILE *f );
XLIBDEF char *ffgets( char *string, int num, FILE *f );

XLIBDEF UCHAR *ffdata( FILE *f );
XLIBDEF int ffsize( FILE *f );

XLIBDEF BOOL InitFfile( void );
XLIBDEF void DeinitFfile( void );


// font.cpp

#define DEFAULTFONT "default.fnt"

#define FNTW 8
#define FNTH 8

#define SMFNTW 5
#define SMFNTH 6

#define FT_MONO1	0			// azonos méretûek
#define FT_MONO2	1			// különbözõ méretûek
#define FT_COLOR	2			// 8:8:8
#define FT_3D		3			//

#define MAX_ASCII	(1024)			// TODO: dinamikussá kellene tenni (malloc)

#ifdef NO3DH
typedef char texmap_t;
#endif


typedef struct font_s {

	int type;
	int id;
	int ref;

	char filename[XMAX_PATH];
	UCHAR *font;
	ULONG color;
	int width;
	int height;
	int minchar;
	int maxchar;
	int ttf_render_width;
	int ttf_render_height;

	BOOL ascii[MAX_ASCII];			// BOOL: van-e ilyen karakter?
	int size[MAX_ASCII][2];			// data meretei
	UCHAR *data[MAX_ASCII];

	BOOL use_kerning;
	char kerning[MAX_ASCII][MAX_ASCII];	// [ Ez ][ Ez elõtti ]

	int offx[MAX_ASCII];
	int offy[MAX_ASCII];			// felülrõl offset: j,q,g,y,p

	int texmapid;
	texmap_t *texmap;
	int flag;				// alpha, chroma

	point_t point[MAX_ASCII][4];		// a négyzet mind a 4 csucsának külön
	point3_t st[MAX_ASCII][4];
	rgb_t rgb[MAX_ASCII][4];

} font_t, *font_ptr;


XLIBDEF int FontWidth( void );
XLIBDEF int FontHeight( void );
XLIBDEF ULONG FontColor( void );
XLIBDEF UCHAR *FontSprite( void );
XLIBDEF int FontFullHeight( void );
XLIBDEF font_t *GetFont( void );

#ifdef __cplusplus
XLIBDEF BOOL SetFont( int id, BOOL load_something = FALSE );
#endif

#define SETFONT(name) { SetFont( LoadFont3D(name) ); }

XLIBDEF void SetFontColor( ULONG c );
XLIBDEF int LoadFont( char *name );
XLIBDEF void SetFontKerning( BOOL );

#define FA_ALL		(-100)
#define FA_EFFECT	(-101)
#define FR_BW		1
#define FR_RED		2
#define FR_GREEN	3
#define FR_BLUE		4
#define FR_YELLOW	5
#define FR_MEGANTA	6
#define FR_CYAN		7
#define FR_PURPLE	8
#define FR_GOLD		9
#define FR_ORANGE	10
#define FR_TEAL		11
#define FL_NORMAL	0
#define FL_OUTLINE	2

typedef struct fontloader_s {

	int ascii;
	int x,y;
	int x1,y1;

	int flag;	// normal, outline

} fontloader_t, *fontloader_ptr;


XLIBDEF void SetFontRgb( int ascii, rgb_t quad[4] );
XLIBDEF void SetFontAlpha( int ascii, int red, int green, int blue, int alpha );
XLIBDEF BOOL LoadTtf( char *filename, BOOL smooth, font_t *font, fontloader_t *fontloader, int nfontloaders );
#ifdef __cplusplus
XLIBDEF BOOL GetTtfName( const char *font_name, char *file_name, BOOL is_bold = FALSE, BOOL is_italic = FALSE );
XLIBDEF int LoadFont3D( char *texname, BOOL chroma=TRUE, BOOL alpha=FALSE, fontloader_t *fontloader=NULL, int nfontloaders=0 );
#endif

XLIBDEF BOOL LoadWinFon( char *path, font_t *font, fontloader_t *fontloader, int nfontloaders );


XLIBDEF void TextMode( int );
XLIBDEF void DiscardFont( void );
XLIBDEF void FreeFont( int id );
XLIBDEF BOOL WriteChar( int x, int y, int c );
XLIBDEF BOOL WriteString( int x, int y, char *s, ... );

#define WS_NORMAL 0
#define WS_ZIGZAG 1
#define WS_ISTR   2

XLIBDEF BOOL WriteStringEx( int x, int y, int flag, char *s, ... );
#define WriteIString(x,y,str) WriteStringEx( x,y, MAKEFLAG(WS_ISTR), (char *)str )

#ifdef __cplusplus
XLIBDEF int MeasureString( char *str, int *istr = NULL, int *height = NULL );
#endif
#define MeasureIString(str) MeasureString( NULL, str )

XLIBDEF int XL_GetCharWidth( char c );
XLIBDEF int XL_GetCharHeight( char c );


XLIBDEF void PushFont( void );
XLIBDEF void PopFont( void );

#ifdef __cplusplus
XLIBDEF void *BuildBubbleString( const char *in, int kerning = 0 );
#endif

XLIBDEF int GetFontId( char *filename );
XLIBDEF int GetRandomFontId( void );

// tooltip.cpp


//
//
//
typedef struct tooltip_s {

	int x,y;
	int dx,dy;

	char str[512];

} tooltip_t, *tooltip_ptr;


XLIBDEF BOOL InitTooltip( void );
#ifdef __cplusplus
XLIBDEF BOOL DrawTooltip( tooltip_t *tooltip, int num, int flag=FALSE );
#endif
XLIBDEF void ClearTooltip( void );




// gfx.cpp

#define CF_TARGET	(1<<0)
#define CF_ZBUFFER	(1<<1)

XLIBDEF void Clear( ULONG color );
XLIBDEF void ClearDepth( ULONG depth );

#ifdef __cplusplus
XLIBDEF void FlipPage( BOOL noflip = FALSE );
XLIBDEF BOOL ToggleFullscreen( int flag = -1 );
#endif


#define LS_READ 	1
#define LS_WRITE	2

XLIBDEF BOOL LockSurface( int flag );
XLIBDEF void UnlockSurface(void);

XLIBDEF ULONG (*XL_GetPixel)(int,int);
XLIBDEF void (*PutSprite)(int,int,UCHAR*);
XLIBDEF void (*GetSprite)(memptr_ptr,int,int,int,int);
XLIBDEF void (*PutSpriteAlpha)( int , int, UCHAR*, int );
XLIBDEF void (*PutSpriteNC)(int,int,UCHAR*);
XLIBDEF void (*GetSpriteNC)(memptr_ptr,int,int,int,int);
XLIBDEF void dummyfunc(void);
XLIBDEF void ResetGraphics( void );

#ifdef __cplusplus
XLIBDEF void ScreenSave( char *filename=NULL );
#endif
XLIBDEF UCHAR *CaptureWindow( __int64 in_hwnd );

XLIBDEF void DrawLineRgb( int x1, int y1, int x2, int y2, rgb_t color );
XLIBDEF void PutPixel( int x, int y, rgb_t color );


#define ScreenShot ScreenSave


typedef struct sprite_s {

	int nsprites;
	int x,y;
	UCHAR *spr;

} sprite_t, *sprite_ptr;

XLIBDEF void PutMoreSprite( sprite_t *, int );

XLIBDEF ULONG AntiAlias( int,int,int );
XLIBDEF void HLine(int,int,int,rgb_t);
XLIBDEF void VLine(int,int,int,rgb_t);

XLIBDEF void XL_Ellipse( int xc, int yc, int a0, int b0, rgb_t PixelValue );
XLIBDEF void Circle(int,int,int,rgb_t);
XLIBDEF void FilledCircle(int,int,int,rgb_t);
XLIBDEF void XL_FloodFill( int x, int y, rgb_t color );

XLIBDEF void GenPutSpriteNC(int,int,UCHAR*);
XLIBDEF void GenGetSpriteNC(memptr_ptr,int,int,int,int);
XLIBDEF void GenPutSprite(int,int,UCHAR*);
XLIBDEF void PutSpriteBlackNC(int,int,UCHAR*);
XLIBDEF void PutSpriteBlack(int,int,UCHAR*);
XLIBDEF void PutSpriteBlackMask(int,int,UCHAR*,ULONG);
XLIBDEF void PutSpriteBlackInvMask(int,int,UCHAR*,ULONG);
XLIBDEF void PutSpriteBlackTrans(int,int,UCHAR*);
XLIBDEF void PutSpriteRect( int x, int y, UCHAR *buf, int x1, int y1, int w, int h );
XLIBDEF void PutSpriteMaskBuf( int x, int y, UCHAR *buf, UCHAR *mask );
XLIBDEF void GenGetSprite(memptr_ptr,int,int,int,int);
XLIBDEF void GetSpriteBuf(memptr_t,int,int,int,int);
XLIBDEF void Box(int,int,int,int,rgb_t);
XLIBDEF void FilledBox(int,int,int,int,rgb_t);
XLIBDEF void Line(int,int,int,int,rgb_t);
XLIBDEF void XorLine(int,int,int,int);
XLIBDEF void XorLineNC(int,int,int,int);
XLIBDEF void DashBox( int ux, int uy, int bx, int by, int be, int ki, rgb_t color );
XLIBDEF void ShadeRegion( int x, int y, int w, int h, int percent );
extern void PostProcess( int flag );
XLIBDEF BOOL GfxThreadSafe( void );

XLIBDEF __int64 *line_offset;		//[1200];

#ifdef __cplusplus
XLIBDEF void DrawArrow( int x0, int y0, int x1, int y1, rgb_t in_color,
			int width = 1,
			int arrowLength=20,
			int iarrowBase=20,
			BOOL drawFirstArrow=FALSE,
			BOOL drawSecondArrow=TRUE,
			BOOL openArrowHead=FALSE );
#endif

#ifdef __cplusplus
XLIBDEF void P64( char *str=NULL );
#endif


XLIBDEF void DrawStatus( void );


typedef enum {

	ST_BW = 0,
	ST_YELLOW,
	ST_RED,
	ST_GREEN,
	ST_BLUE,
	ST_ORANGE,
	ST_PURPLE,
	ST_GOLD,

	ST_RESET,

} stcolor_e;


#ifdef __cplusplus
XLIBDEF void StatusText( stcolor_e color=ST_RESET, char *s=NULL, ... );
#endif
XLIBDEF void ClearStatusText( void );
// XLIBDEF BOOL InitStatus( void );



// console.cpp

//  Alt - '0'
#define CHECKCONSOLE(k) {								\
	int check_console_key = k;							\
	if( /* check_console_key == EXT(sc_F12) || */ (check_console_key == 48 /*'0'*/ && ( /*keys[sc_Control] || */ keys[sc_Alt])) )	\
		display_zprintf ^= 1;							\
}

#define ISCONSOLEKEY(k) ( ((k) == 48) && keys[sc_Alt] )


#define CONSOLE( cmd ) consoleExec( -1, cmd )

XLIBDEF BOOL consoleInit( void );
XLIBDEF void consolePrintf( char *s, ... );
XLIBDEF void consoleClearBuffers( void );
XLIBDEF void consoleHistoryRecall( void );
XLIBDEF void consoleDraw( void );
XLIBDEF void consoleFlush( void );
XLIBDEF void consoleAddChar( int c );
XLIBDEF void consoleCommandLineClear( void );
XLIBDEF void consoleResize( void );
XLIBDEF int consoleAnimate( void );
XLIBDEF void consoleDrawRawString( int *s, FLOAT x, FLOAT y, rgb_t color );
XLIBDEF void consoleBufferMove( int dir );
XLIBDEF int consoleGetCommandNum( void );
XLIBDEF BOOL consoleAddCommand( char *name, void (*function)(char *name,char *function) );
XLIBDEF BOOL consoleIsCommand( char *cmd );

enum {
	CON_NORESULT = 0,
	CON_NUMBER,
	CON_STRING,
	CON_BOOL
};

#ifdef __cplusplus
XLIBDEF int consoleExec( BOOL state=FALSE, char *cmd=NULL );
#endif

XLIBDEF char *consoleGetString( void );
XLIBDEF double consoleGetNumber( void );
XLIBDEF BOOL consoleGetBool( void );



// cvar.cpp -- dynamic variable tracking

/*
 * cvar_t variables are used to hold scalar or string variables
 * that can be changed or displayed at the console or prog code
 * as well as accessed directly in C code.
 *
 * it is sufficient to initialize a cvar_t with just the first
 * two fields, or you can add a ,true flag for variables that
 * you want saved to the configuration file when the game is quit:
 *
 * cvar_t	r_draworder = {"r_draworder","1"};
 * cvar_t	scr_screensize = {"screensize","1",true};
 *
 * Cvars must be registered before use, or they will have a 0 value
 * instead of the float interpretation of the string.  Generally,
 * all cvar_t declarations should be registered in the apropriate
 * init function before any console commands are executed:
 *
 * CV_RegisterVariable (&host_framerate);
 *
 * C code usually just references a cvar in place:
 *
 * if( r_draworder.value )
 *
 * It could optionally ask for the value to be looked up for a
 * string name:
 *
 * if( CV_VariableValue("r_draworder") )
 *
 * Interpreted prog code can access cvars with the cvar(name) or
 * cvar_set(name, value) internal functions:
 *
 * teamplay = cvar("teamplay");
 * cvar_set("registered", "1");
 *
 * The user can access cvars from the console in two ways:
 *
 * r_draworder			prints the current value
 * r_draworder 0		sets the current value to 0
 *
 * Cvars are restricted from having the same names as commands to
 * keep this interface from being ambiguous.
 */


// registers a cvar that allready has the name, string, and
// optionally the archive elements set.
XLIBDEF void CV_RegisterVariable( cvar_t *variable );

// equivelant to "<name> <variable>" typed at the console
XLIBDEF void CV_Set( char *var_name, char *value );

// expands value to a string and calls Cvar_Set
XLIBDEF void CV_SetValue( char *var_name, FLOAT value );
XLIBDEF void CV_SetFunc( char *var_name, void (*func)(void) );

// returns 0 if not defined or non numeric
XLIBDEF FLOAT CV_VariableValue( char *var_name );

// returns an empty string if not defined
XLIBDEF char *CV_VariableString( char *var_name );

// attempts to match a partial variable name for command line
// completion returns NULL if nothing fits
XLIBDEF char *CV_CompleteVariable( char *partial );

// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)
XLIBDEF BOOL CV_Command( char *cmd, char *parms );

// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.
XLIBDEF void CV_WriteVariables( FILE *f );

XLIBDEF cvar_t *CV_FindVar( char *var_name );



// kbd.c


// extern int key_hook;
XLIBDEF int scan_code;
XLIBDEF int ascii_code;
XLIBDEF BOOL keys[MAX_ASCII];			    // keys[256]

XLIBDEF BOOL CapsLock,NumLock,ScrollLock;

// scancodes

#define EXTKEY 0x0100
#define EXT(s) ( (s) | EXTKEY )
#define ISEXTKEY(k)  ( (k) > EXT( 0 ) )

#define sc_None 			0
#define sc_Plus 			13
#define sc_Minus			12
#define sc_Pause			0x00
#define sc_Bad				0xff
#define sc_Return			0x1c
#define sc_Enter			sc_Return
#define sc_Escape			0x01
#define sc_Space			0x39
#define sc_BackSpace			0x0e
#define sc_Tab				0x0f
#define sc_Alt				0x38
#define sc_Control			0x1d
#define sc_CapsLock			0x3a
#define sc_LeftShift			0x2a
#define sc_RightShift			0x36
#define sc_PrintScreen			183 		/* 0x37 */
#define sc_Up				0x48
#define sc_Down 			0x50
#define sc_Left 			0x4b
#define sc_Right			0x4d
#define sc_Insert			0x52
#define sc_Delete			0x53
#define sc_Home 			0x47
#define sc_End				0x4f
#define sc_PageUp			0x49
#define sc_PageDown			0x51
#define sc_F1				0x3b
#define sc_F2				0x3c
#define sc_F3				0x3d
#define sc_F4				0x3e
#define sc_F5				0x3f
#define sc_F6				0x40
#define sc_F7				0x41
#define sc_F8				0x42
#define sc_F9				0x43
#define sc_F10				0x44
#define sc_F11				0x57
#define sc_F12				0x58

#define sc_1				0x02
#define sc_2				0x03
#define sc_3				0x04
#define sc_4				0x05
#define sc_5				0x06
#define sc_6				0x07
#define sc_7				0x08
#define sc_8				0x09
#define sc_9				0x0a
#define sc_0				0x0b

#define sc_A				0x1e
#define sc_B				0x30
#define sc_C				0x2e
#define sc_D				0x20
#define sc_E				0x12
#define sc_F				0x21
#define sc_G				0x22
#define sc_H				0x23
#define sc_I				0x17
#define sc_J				0x24
#define sc_K				0x25
#define sc_L				0x26
#define sc_M				0x32
#define sc_N				0x31
#define sc_O				0x18
#define sc_P				0x19
#define sc_Q				0x10
#define sc_R				0x13
#define sc_S				0x1f
#define sc_T				0x14
#define sc_U				0x16
#define sc_V				0x2f
#define sc_W				0x11
#define sc_X				0x2d
#define sc_Y				0x15
#define sc_Z				0x2c
#define sc_NumLock			0x45
#define sc_ScrollLock			0x46
#define sc_GreyPlus			0x4e
#define sc_GreyMinus			0x4a

#define sc_LeftSquare			0x1a
#define sc_RightSquare			0x1b

#define key_None			0
#define key_Return			0x0d
#define key_Enter			key_Return
#define key_Escape			0x1b
#define key_Space			0x20
#define key_BackSpace			0x08
#define key_Tab 			0x09
#define key_Delete			0x7f


XLIBDEF void DX_keyboard_handler( int key, int data, int localizedkey );
XLIBDEF void WIN32_keyboard_handler( int key, int data );
extern void XLIB_FeedKey( void );
extern int translateWin32Text( int kc );

XLIBDEF void InitKeyboard(void);
XLIBDEF void DeinitKeyboard(void);
XLIBDEF int GetFirstKey(void);
XLIBDEF void WaitKeyUp(int i);
XLIBDEF int WaitKey(void);
XLIBDEF char GetAscii(void);
XLIBDEF char *GetScanName(int scan);
XLIBDEF void ClearKey(void);
XLIBDEF BOOL LineInput(int x,int y,char *buf,char *def,int maxchars,BOOL escok);
XLIBDEF int GetChar( void );					// ascii
XLIBDEF int GetKey( void );						// ascii + extended scan_code
XLIBDEF void PushbackKey( int key );
XLIBDEF int NumKey( void );
XLIBDEF int PeekKey( void );
XLIBDEF BOOL KeyHit( void );

// mem.cpp

// XLIBDEF int XV_MEMALL;		// erdekes
// #define INITMEMORY( x ) int XV_MEMALL = x

XLIBDEF ULONG mem_used,mem_all;

XLIBDEF void MemCtrl( BOOL check, BOOL compress );
XLIBDEF void MemCheck( void );
XLIBDEF void MemStats( void );
XLIBDEF void MemLeak( void );
XLIBDEF BOOL NameMem( memptr_t, const char*, int, const char* );
XLIBDEF void InitMemory( void );
XLIBDEF void DeinitMemory( void );

XLIBDEF ULONG GetMemInfo(void);
XLIBDEF BOOL ChangeMemBlockPtr( memptr_ptr old, memptr_ptr uj );

XLIBDEF BOOL AllocMem( memptr_ptr, int size );
XLIBDEF BOOL ReallocMem( memptr_ptr, int size );
XLIBDEF BOOL FreeMem( memptr_ptr );
XLIBDEF BOOL EraseMem( memptr_t );


XLIBDEF BOOL LockMem( memptr_ptr );
XLIBDEF BOOL UnlockMem( memptr_ptr );

XLIBDEF void *AllocMemNoPtr( int );
XLIBDEF void *ReallocMemNoPtr( memptr_t, int );
XLIBDEF BOOL FreeMemNoPtr( void * );

XLIBDEF BOOL LockMemNoPtr(void *);
XLIBDEF BOOL UnlockMemNoPtr(void *);

XLIBDEF ULONG BufSize( memptr_ptr );
XLIBDEF ULONG BufSizeNoPtr( void * );


//
// csak a memblock-ban a nevet írja át
//
#define NAMEMEM( ptr, str ) {						\
	if( NameMem( (memptr_t)(ptr), NULL, -1, (str) ) == FALSE )	\
		Quit("%s(%d): can't NameMem %s(%d):%s",__FILE__,__LINE__,(str));	\
		/* errNameMem( __FILE__, __LINE__, (str) ); */		\
}

//
// Xlib-ben belüli elnevezésre
//
#define NAMEMEM_ALL( ptr, str ) {						\
	if( NameMem( (memptr_t)(ptr), __FILE__, __LINE__, (str) ) == FALSE )	\
		Quit("%s(%d): can't NameMem %s(%d):%s",__FILE__,__LINE__,(str));	\
		/* errNameMem( __FILE__, __LINE__, (str) ); */		\
}


#define ALLOCMEM( ptr, num ) {						\
	if( AllocMem( PTR(ptr), (num) ) == FALSE )			\
		Quit("%s(%d): can't allocate %d bytes for \"%s\".\n\nOut of memroy.",__FILE__,__LINE__,(num),#ptr); \
		/* errAllocMem( __FILE__, __LINE__, #ptr, (num) ); */	\
	NAMEMEM_ALL( ptr, #ptr);						\
}


#define ALLOCMEMNP( ptr, num ) {					\
	if( ( (ptr) = (void*)AllocMemNoPtr( (num) )) == NULL )		\
		Quit("%s(%d): can't allocate %d bytes for \"%s\".",__FILE__,__LINE__,(num),#ptr); \
		/* errAllocMem( __FILE__, __LINE__, #ptr, (num) ); */	\
	NAMEMEM_ALL( (ptr), #ptr);					\
}


#define CALLOCMEM( ptr, num ) { 					\
	if( (num) > 0 ) {						\
		ALLOCMEM( (ptr), (num) );				\
		memset( (ptr), 0L, (num) );				\
	}								\
	else								\
		(ptr) = NULL;						\
}


#define FREEMEM( ptr ) {						\
	if( (ptr) != NULL ) {						\
		if( FreeMem( PTR(ptr) ) == FALSE )			\
			if( FreeMemNoPtr( (ptr) ) == FALSE )		\
				xprintf("%s(%d): can't free %d \"%s\".\n",__FILE__,__LINE__,*(int*)(&(ptr)),#ptr); \
				/* errFreeMem( __FILE__, __LINE__, #ptr ); */	\
		(ptr) = NULL;						\
	}								\
	else								\
		xprintf("%s(%d): attempting free %d \"%s\" a NULL pointer.\n",__FILE__,__LINE__,*(int*)(&(ptr)),#ptr); \
}

#define SAFEFREEMEM(p) { if( p ){ FREEMEM((p)); } (p) = NULL; }
#define SAFE_FREEMEM SAFEFREEMEM

#define DISCARDMEM( ptr ) {			\
	MemCtrl( FALSE, FALSE );		\
	FREEMEM( ptr );				\
	MemCtrl( FALSE, FALSE );		\
}


#define ERASEMEM( ptr ) {						\
	if( (ptr) != NULL ) {						\
		EraseMem( ptr );					\
		if( FreeMem( PTR(ptr) ) == FALSE )			\
			if( FreeMemNoPtr( (ptr) ) == FALSE )		\
				xprintf("%s(%d): can't free %d \"%s\".",__FILE__,__LINE__,*(int*)(&(ptr)),#ptr); \
				/* errFreeMem( __FILE__, __LINE__, #ptr ); */	\
		(ptr) = NULL;						\
	}								\
	else								\
		xprintf("%s(%d): attempting free %d \"%s\" a NULL pointer.",__FILE__,__LINE__,*(int*)(&(ptr)),#ptr); \
}


// type cast needed
// if( ((ptr) = ReallocMemNoPtr( (memptr_t)(ptr), (num) )) == NULL )

#define REALLOCMEM( ptr, num ) {					\
	if( ReallocMem( PTR(ptr), (num) ) == FALSE )			\
		Quit("%s(%d): can't reallocate %d bytes for \"%s\".",__FILE__,__LINE__,#ptr,(num)); \
		/* errReallocMem( __FILE__, __LINE__, #ptr , (num) ); */ \
	if( (num) > 0 ) 						\
		NAMEMEM_ALL( (ptr), #ptr);				\
}

// misc.cpp

#define LOGFILE "xlog"
#define CONFILE "xcon.html"

#define MAX_ARGC 256

XLIBDEF int myargc;
XLIBDEF char *myargv[MAX_ARGC];
XLIBDEF char **myenvp;
XLIBDEF char *fullcommandline;

XLIBDEF void makefooter( void );

XLIBDEF BOOL striremove( char *pszString, const char *pcszSub );
XLIBDEF BOOL strremove( char *pszString, const char *pcszSub );
XLIBDEF const char *strright( const char *pcsz, int n );
XLIBDEF char *stristr( char *pszSource, const char *pcszSearch );
XLIBDEF void strrepc( char *pszString, char cSearch, char cReplace );
XLIBDEF char *strltrim( char *pszString );
XLIBDEF char *strrtrim( char *pszString );

XLIBDEF char *insertCommas( unsigned int value );

XLIBDEF void RemoveDuplicateChar( char *p, char ch );
XLIBDEF void ChangeChar( char *p, char ch, char to );
XLIBDEF void ChangeCharStr( char *p, char ch, char *to );


//					'/'                        '-'
#define ISPARM(s) ( (((char*)(s))[0] == 47) || (((char*)(s))[0] == 45) )

XLIBDEF void Quit( const char *s,...);
XLIBDEF BOOL GFXDLLCALLCONV winShowQuitDlg(char *s,...);
XLIBDEF void QuitTitle(char *title, char *s,...);


#define LOG WriteLog

XLIBDEF FILE *OpenLog( void );
XLIBDEF BOOL WriteLog(char *s,...);
XLIBDEF char *getXlibLogFilename( void );

XLIBDEF void GFXDLLCALLCONV xprintf( const char *, ... );

extern BOOL XLIB_winRegNoLog( void );
extern BOOL (*winRegNoLog)( void );
// extern __declspec(dllexport) void ___tempy_func1( void );
// #define WINNOLOG	void ___tempy_func1( void ) { return; }

extern BOOL XLIB_winNoSplash( void );
extern BOOL (*winNoSplash)( void );
// extern __declspec(dllexport) void ___tempy_func10( void );
// #define WINNOSPLASH	void ___tempy_func10( void ) { return; }


// playlist.cpp

XLIBDEF int PLS_GetMax( const char *playlist_filename );
XLIBDEF BOOL PLS_IsValid( const char *playlist_filename );
XLIBDEF void PLS_ClearFile( void );
XLIBDEF char *PLS_GetNext( const char *playlist_filename );

#ifdef __cplusplus
XLIBDEF char *PLS_GetName( const char *playlist_filename, int num = 0 );
XLIBDEF BOOL PLS_SetFile( const char *playlist_filename, BOOL force=FALSE );
#endif


extern void XLIB_DumpBit( ULONG bits, int num );


XLIBDEF __int64 rdtsc( void );

// RDTSC Macro...
#define RDTSC( var )			\
	_asm _emit 0x0F			\
	_asm _emit 0x31			\
	_asm mov DWORD PTR var,eax	\
	_asm mov DWORD PTR var+4,edx

XLIBDEF int GFXDLLCALLCONV CheckParm( const char *check);
XLIBDEF int NumArgs( void );
XLIBDEF char *GetArg( int );
XLIBDEF void InitRandom( BOOL );
XLIBDEF int xrand( void );
XLIBDEF void SetRandom( int );
XLIBDEF void xsrand( UINT seed );
XLIBDEF BOOL bernie(void);
//extern char *GetTempDir( void );
XLIBDEF const char *GetRealTemp( void );
#define GetTempDir XLIB_winTemp
XLIBDEF int ShowProgress(void);

XLIBDEF void QuickSort(void *base0, int n, int size, int (*compar)(const void *, const void *));

XLIBDEF void RemoveChar( char *p, char ch );

#ifdef __cplusplus
XLIBDEF void CalcCrc( char *Data = NULL, unsigned int NumBytes = 0, unsigned int *crc = NULL, unsigned int *sum = NULL );
#endif

XLIBDEF ULONG CalcCrc32( const unsigned char* buf, unsigned size );


XLIBDEF char *searchpath( const char * );
XLIBDEF void ChangeExtension( char *orig, char *ext, char *uj );
XLIBDEF char *GetExtension( char *name );	// NULL or .ext
XLIBDEF char *GetFilenameNoPath( char *filename );
XLIBDEF char *GetRelativeFilename( char *filename );
#ifdef __cplusplus
XLIBDEF char *GetFilePath( char *filename, BOOL nocwd = FALSE );
#endif

#define XLIB_COPYRIGHT_YEAR	3001
#define XLIB_COPYRIGHT_MONTH	2
#define XLIB_COPYRIGHT_DAY	1

XLIBDEF BOOL CopyCheck( void );
#ifdef __cplusplus
XLIBDEF const char *GetDate( int *year=NULL, int *ho=NULL, int *nap=NULL );
XLIBDEF void GetTime( int *hour=NULL, int *min=NULL, int *mp=NULL );
#endif
XLIBDEF int GetYear(void);
XLIBDEF const char *GetMonth(void);
XLIBDEF int GetMon( void );
XLIBDEF int GetDay(void);
XLIBDEF const char *GetWeek(void);
XLIBDEF char *GetDateStr( void );
XLIBDEF int GetHour( void );
XLIBDEF int GetMin( void );
XLIBDEF int GetSec( void );
XLIBDEF int MoonPhase( void );		// 0-7, with 0: new, 4: full
XLIBDEF BOOL Night( void );
XLIBDEF BOOL Midnight( void );
XLIBDEF char *MonthStr( int month );


XLIBDEF UINT ratio( long, long );

XLIBDEF BOOL IdBufLong( char *buf, ULONG id );
XLIBDEF BOOL IdBufStr( char *buf, const char *id );


#define FNM_NOESCAPE	(1<<0)
#define FNM_PATHNAME	(1<<1)
#define FNM_NOCASE	(1<<2)

#define FNM_NOMATCH	TRUE
#define FNM_MATCH	FALSE

XLIBDEF int fnmatch( const char *pattern, const char *string, int flags );


XLIBDEF int IsLittleEndian( void );
XLIBDEF short LittleShort( short l );
XLIBDEF short BigShort( short l );
XLIBDEF long LittleLong( long l );
XLIBDEF long BigLong( long l );
XLIBDEF float LittleFloat( float l );
XLIBDEF float BigFloat( float l );

XLIBDEF UCHAR *GetDPpal( void );
XLIBDEF int NextPow2( int Number );


// istring.cpp

XLIBDEF int istrlen( int *istr );
XLIBDEF int *toistr( char *str );
XLIBDEF char *istrtostr( int *istr );
XLIBDEF BOOL istrcat( int *dst, int *src );
XLIBDEF BOOL istrcatstr( int *dst, char *src );
XLIBDEF int istrcmpstr( int *istr, char *str );
XLIBDEF int istrcmp( int *istr, int *istr2 );
XLIBDEF int istricmp( int *istr, int *istr2 );
XLIBDEF int istricmpstr( int *istr, char *str );
XLIBDEF int istrncmpstr( int *istr, char *str, int len );
XLIBDEF int istrnicmpstr( int *istr, char *str, int len );
XLIBDEF BOOL istrcpy( int *dst, int *src );
XLIBDEF BOOL istrncpy( int *dst, int *src, int num );
XLIBDEF BOOL istrcpystr( int *dst, char *src );
XLIBDEF BOOL istrncpystr( int *dst, char *src, int num );
XLIBDEF int *istrchr( int *istr, int c );
XLIBDEF int *istrdup( int *istr );
XLIBDEF int wstrlen( USHORT *wstr );
XLIBDEF char *wstrtostr( USHORT *wstr );


// string.cpp

#ifdef __cplusplus
XLIBDEF BOOL InitString( char *filename=NULL );
#endif
XLIBDEF void DeinitString( void );
XLIBDEF char *GetString( int id );
XLIBDEF int GetStringLen( int id );



// wipe.c

XLIBDEF void InitWipe( UCHAR *scr );
XLIBDEF BOOL DoWipe( void );	// until it FALSE
XLIBDEF void ExitWipe( void );


// uu.c

// #define UUMAGIC  0x6265
// #define UUSTR	"section"

XLIBDEF int EncodeUU( char *name, UCHAR *buf, int len, memptr_ptr ptr );
XLIBDEF int DecodeUU( UCHAR *buf, int len, memptr_ptr ptr );
XLIBDEF BOOL IsUU( char *buf, int len );


// joystick.c

XLIBDEF signed int joyx,joyy,joyz,joydx,joydy,joydz,joypov,maxjoyx,maxjoyy,maxjoyz;
XLIBDEF BOOL joyb[32];
XLIBDEF BOOL joybd[32];

// sima win32-es joy
XLIBDEF BOOL XLIB_RefreshJoystick( void );


// mouse.c

#define MOUSELEFTBUTTON 0				// enter
#define MOUSERIGHTBUTTON 1				// space
#define MOUSEMIDBUTTON 2				// insert

#define MOUSEW 64
#define MOUSEH 64

#define MOUSETRESHOLD 1

XLIBDEF signed int mousex,mousey,mousez,mousedx,mousedy,mousedz;
XLIBDEF BOOL mousebr,mousebl,mousebm,mouseb4,mouseb5;
XLIBDEF BOOL mousebrd,mousebld,mousebmd,mouseb4d,mouseb5d;
XLIBDEF BOOL mouse_status;			   // TRUE lehet razolni
XLIBDEF BOOL mouseinstalled;

#define MF_3DCURSOR 1
#define MF_NOCLIP 2
#define MF_HOOK 3

XLIBDEF BOOL InitMouse( int flag );
XLIBDEF void DeinitMouse(void);
XLIBDEF void HideMouse(void);
XLIBDEF void ShowMouse(void);
XLIBDEF void ChangeMouseCursor(UCHAR *b,UCHAR *pal,int hotx, int hoty );
XLIBDEF BOOL MousePointerFromPcx( char *name, int x, int y );
XLIBDEF void DrawMouse(void);
XLIBDEF void ReadMouse(void);
XLIBDEF void SetMouseRange(signed int x,signed int y,signed int x1,signed int y1);
XLIBDEF void MoveMouse(int,int);
XLIBDEF void ClearMouse( void );
XLIBDEF void RotateMouse( FLOAT ang );
XLIBDEF void DrawWindowButton( void );

XLIBDEF void DPMouseRemapColor( void );
XLIBDEF void MakeDpMouse( void );
XLIBDEF void MakeHourGlassMouse( void );

XLIBDEF void SetMouseRgb( rgb_t *rgb );

XLIBDEF void RotateRect( point_t point[4], FLOAT mouse_ang );

XLIBDEF BOOL SetMouseClip( BOOL clipit_flag );
XLIBDEF BOOL GetMouseClip( void );
XLIBDEF void winClipMouse( void );
XLIBDEF void winMoveCursor( int x, int y );

// az utsó mouse action másodpercben
XLIBDEF int GetLastMouse( void );

// text.c

XLIBDEF int DumpAscii( UCHAR *spr, UCHAR *rgb, memptr_ptr, char *name );


// timer.c

#define TICKBASE ( CLOCKS_PER_SEC )

XLIBDEF int GetTic( void );
XLIBDEF void WaitTimer( int n );

#define SET_CLOCK( variable ) 			ULONG variable = GetTic();
#define STOP_CLOCK( collector, variable ) 	collector += ( GetTic() - variable );

// lza.c

// #define LZMAGIC 0x1213
// #define LZHEADER sizeof(USHORT)

XLIBDEF ULONG DecodeLZA( UCHAR *p, memptr_ptr ptr );
XLIBDEF ULONG DecodeFile( FILE*, FILE* );
XLIBDEF void DecodeRaw( UCHAR *source, int textsize, memptr_ptr ptr );
XLIBDEF ULONG EncodeLZA( UCHAR *source, ULONG size, memptr_ptr ptr );
XLIBDEF ULONG EncodeFile( FILE*, ULONG, FILE* );
XLIBDEF BOOL IsLZA( char *buf, int len );

XLIBDEF void SetHufCallBack( void (*func)(int) );

#define BASECRC  ((ULONG)0x6BE421E9)
XLIBDEF ULONG MakeCRC( UCHAR *, int );
XLIBDEF ULONG MakeCRCFile( FILE*, int );

XLIBDEF BOOL comp_show;



// lzh.c

XLIBDEF ULONG DecodeLZH( UCHAR *p, memptr_ptr ptr );
XLIBDEF ULONG EncodeLZH( UCHAR *source, ULONG size, memptr_ptr ptr );
XLIBDEF BOOL IsLZH( char *buf, int len );


// lzs.c

XLIBDEF ULONG DecodeLZS( UCHAR *p, memptr_ptr ptr );
XLIBDEF ULONG EncodeLZS( UCHAR *source, ULONG size, memptr_ptr ptr );
XLIBDEF BOOL IsLZS( char *buf, int len );




// rle.c

// #define RLEMAGIC 0x1215
// #define RLEHEADER sizeof(USHORT)

XLIBDEF ULONG EncodeRLE( UCHAR *, ULONG, memptr_ptr );
XLIBDEF ULONG DecodeRLE( UCHAR *, memptr_ptr );
XLIBDEF BOOL IsRLE( char *ptr, int len );


// pcx.c

XLIBDEF BOOL IsPcx( char *name );
XLIBDEF BOOL LoadPcx(char*,memptr_ptr);
XLIBDEF BOOL LoadSpriteFromPcx(char *name,memptr_ptr,int x,int y,int w,int h);
XLIBDEF BOOL GetPcxDim(char*,int*,int*);
XLIBDEF void SetRgbPcx(void);
XLIBDEF void GetRgbPcx(UCHAR *pal);
#ifdef __cplusplus
XLIBDEF BOOL SavePcx(char *name, UCHAR *spr, UCHAR *pal = NULL);
#endif

// psd.cpp

XLIBDEF BOOL IsPsd( char *name );
XLIBDEF BOOL LoadPsd( char *fname, memptr_ptr ptr );
XLIBDEF void SetRgbPsd(void);
XLIBDEF void GetRgbPsd(UCHAR *pal);


// bmp.c

XLIBDEF BOOL IsBmp( char *name );
#ifdef __cplusplus
XLIBDEF BOOL LoadBmp(char*,memptr_ptr,int *w=NULL, int *h=NULL, int *bpp=NULL);
XLIBDEF BOOL SaveBmp( char *name, UCHAR *s, UCHAR *p=NULL );
#endif
XLIBDEF void SetRgbBmp(void);
XLIBDEF void GetRgbBmp(UCHAR *pal);
extern GFXDLLCALLCONV BOOL XLIB_BmpDim( char *name, int *w, int *h );

XLIBDEF UCHAR pic_pal[768];

#ifdef __cplusplus
XLIBDEF BOOL LoadPicture( char *name, memptr_ptr ptr, int *w=NULL, int *h=NULL, int *bpp=NULL, int *delay=NULL, BOOL bPanel=FALSE );
XLIBDEF BOOL GetPictureDim( char *name, int *width, int *hight, int *bpp=NULL );
#endif
XLIBDEF void GetRgbPicture( UCHAR *pal );
XLIBDEF void SetRgbPicture( void );


// ico.cpp

XLIBDEF BOOL IsIco( char *name );
XLIBDEF BOOL LoadIco( char *fname, memptr_ptr ptr );
XLIBDEF BOOL SaveIco( char *name, UCHAR *s, UCHAR *p );
XLIBDEF void SetRgbIco(void);
XLIBDEF void GetRgbIco(UCHAR *pal);


// gif.c

XLIBDEF BOOL IsGif( char *name );
XLIBDEF BOOL LoadGif( char *name, memptr_ptr );
XLIBDEF BOOL SaveGif( char *name, UCHAR *s, UCHAR *p );
XLIBDEF void SetRgbGif( void );
XLIBDEF void GetRgbGif( UCHAR* );


// raw.c
XLIBDEF BOOL IsRaw( char *name );
XLIBDEF BOOL LoadRaw(char*,memptr_ptr);
XLIBDEF BOOL LoadSpriteFromRaw(char *name,memptr_ptr,int x,int y,int w,int h);
XLIBDEF void SaveRaw(char *name,UCHAR *s,UCHAR *p);
XLIBDEF void SetRgbRaw(void);
XLIBDEF void GetRgbRaw(UCHAR *pal);


// tiffshell.c
XLIBDEF BOOL IsTiff( char *name );
XLIBDEF BOOL LoadTiff( char*, memptr_ptr );
XLIBDEF BOOL SaveTiff( char *name, UCHAR *s, UCHAR *p );
XLIBDEF void SetRgbTiff( void );
XLIBDEF void GetRgbTiff( UCHAR *pal );

// pngshell.c
XLIBDEF BOOL IsPng( char *name );
XLIBDEF BOOL LoadPng( char*, memptr_ptr );
XLIBDEF BOOL SavePng( char *name, UCHAR *s, UCHAR *p );
XLIBDEF void SetRgbPng( void );
XLIBDEF void GetRgbPng( UCHAR *pal );


// freeshell.cpp

XLIBDEF BOOL IsFree( char *name );
XLIBDEF BOOL LoadSpriteFromFree(char *name,memptr_ptr,int x,int y,int w,int h);
XLIBDEF void SetRgbFree(void);
XLIBDEF void GetRgbFree(UCHAR *pal);
#ifdef __cplusplus
XLIBDEF BOOL SaveFree(char *name, UCHAR *spr, UCHAR *pal = NULL);
XLIBDEF BOOL GetFreeDim(char*,int*w=NULL,int*h=NULL,int*bpp=NULL,int *delay=NULL);
XLIBDEF BOOL LoadFree(char*,memptr_ptr,int *w=NULL,int *h=NULL,int *bpp=NULL, int *delay=NULL, BOOL bPanel=FALSE);
#endif

// imagemagickshell.cpp

#ifdef __cplusplus
XLIBDEF BOOL ConvertImageMagick( char *src, char *dst, int w=0, int h=0 );
XLIBDEF BOOL GetDimImageMagick(char*,int*w=NULL,int*h=NULL,int*bpp=NULL,int *delay=NULL);
XLIBDEF BOOL LoadImageMagick( char *filename, memptr_ptr spr, int *w=NULL,int *h=NULL,int *bpp=NULL, int *delay=NULL );
XLIBDEF BOOL SaveImageMagick(char *name, UCHAR *spr, UCHAR *pal = NULL);
#endif

XLIBDEF BOOL ImageMagickCommand( char *str );



// ijlshell.cpp

extern BOOL GFXDLLCALLCONV XLIB_IsIjl( char *name );
extern BOOL GFXDLLCALLCONV XLIB_LoadIjl( char *fname, memptr_ptr ptr );
extern BOOL GFXDLLCALLCONV XLIB_SaveIjl( char *name, UCHAR *s, UCHAR *p );
extern void GFXDLLCALLCONV XLIB_SetRgbIjl(void);
extern void GFXDLLCALLCONV XLIB_GetRgbIjl(UCHAR *pal);
extern BOOL GFXDLLCALLCONV XLIB_GetIjlDim(char*,int*,int*);

extern BOOL (GFXDLLCALLCONV *IsIjl)( char *name );
extern BOOL (GFXDLLCALLCONV *LoadIjl)( char *fname, memptr_ptr ptr );
extern BOOL (GFXDLLCALLCONV *SaveIjl)( char *name, UCHAR *s, UCHAR *p );
extern void (GFXDLLCALLCONV *SetRgbIjl)(void);
extern void (GFXDLLCALLCONV *GetRgbIjl)(UCHAR *pal);
extern BOOL (GFXDLLCALLCONV *GetIjlDim)(char*,int*,int*);


// segshell.c

// #define SEGMAGIC 0x1214
// #define SEGHEADER sizeof(USHORT)

// #define MAXSEGFILE 10

//
// for MS-DOS int 21h 5700, 5701
//
// #define YEAR(t)   (((((t)>>16) & 0xFE00) >> 9) + 1980)
// #define MONTH(t)  ((((t)>>16) & 0x01E0) >> 5)
// #define DAY(t)	  (((t)>>16) & 0x001F)
// #define HOUR(t)   (((t) & 0xF800) >> 11)
// #define MINUTE(t) (((t) & 0x07E0) >> 5)
// #define SECOND(t) (((t) & 0x001F) << 1)


#define YEAR(t)   FileDateYear(t)
#define MONTH(t)  FileDateMonth(t)
#define DAY(t)	  FileDateDay(t)
#define HOUR(t)   FileDateHour(t)
#define MINUTE(t) FileDateMin(t)
#define SECOND(t) FileDateSec(t)

XLIBDEF int FileDateYear( int time );
XLIBDEF int FileDateMonth( int time );
XLIBDEF int FileDateDay( int time );
XLIBDEF int FileDateHour( int time );
XLIBDEF int FileDateMin( int time );
XLIBDEF int FileDateSec( int time );

XLIBDEF BOOL OpenSegFile(char *name);
XLIBDEF BOOL CloseSegFile( int hand );
XLIBDEF BOOL CloseAllSegFile(void);

XLIBDEF FILE *SEG_OpenFile( char *name );
XLIBDEF void SEG_CloseFile( FILE *f );
XLIBDEF LONG SEG_FileSize( FILE* );
XLIBDEF BOOL SEG_FileExist( char *n );
XLIBDEF BOOL SEG_IsFileInSeg( FILE* );
XLIBDEF ULONG SEG_FileDate( char * );

XLIBDEF BOOL SEG_FindFirst( char *, ULONG *, int hand );
XLIBDEF BOOL SEG_FindNext( char *, ULONG *, int hand );

XLIBDEF void SegTomFlag( int flag );
XLIBDEF BOOL CreateSegFile( char *name );
XLIBDEF BOOL SEG_AddFile( char *name, UCHAR *p, ULONG size, ULONG time );
XLIBDEF void CloseCreatedSegFile( void );


// zipshell.cpp

#ifdef __cplusplus
XLIBDEF int OpenZipFile( char *name, BOOL for_write = FALSE );
XLIBDEF BOOL ZIP_Append( int handle, char *filename = NULL, UCHAR *buf = NULL, int len=0, char *name=NULL, ULONG time=0 );
#endif
XLIBDEF void CloseAllZipFile(void);
XLIBDEF void CloseZipFile( char *filename );
XLIBDEF BOOL ZIP_LoadFile( UCHAR *buf, int len, FILE *f );
XLIBDEF int ZIP_FileSize( FILE *f );
XLIBDEF BOOL ZIP_FileExist( char *name );
XLIBDEF FILE *ZIP_OpenFile( char *name );
XLIBDEF BOOL ZIP_IsFileInZip( FILE *f );
#ifdef __cplusplus
XLIBDEF BOOL ZIP_GetNextFilename( char *filename, int handle=0, int flag=0,
	int *size=NULL, int *year=NULL, int *month=NULL, int *day=NULL,
	int *hour=NULL, int *minute=NULL, int *second=NULL, int *milliseconds=NULL );
#endif

XLIBDEF BOOL IsZIP( char *ibuf, int len );
XLIBDEF ULONG ZIP_Compress( UCHAR *uncompr, int ulen, memptr_ptr ptr );
XLIBDEF ULONG ZIP_Uncompress( UCHAR *compr, memptr_ptr ptr );
XLIBDEF BOOL IsZipFromName( char *name );

// unrarshell.cpp

XLIBDEF int OpenRarFile( char *name );
XLIBDEF void CloseAllRarFile(void);
XLIBDEF void CloseRarFile( char *filename );
XLIBDEF BOOL RAR_LoadFile( UCHAR *buf, int len, FILE *f );
XLIBDEF int RAR_FileSize( FILE *f );
XLIBDEF BOOL RAR_FileExist( char *name );
XLIBDEF FILE *RAR_OpenFile( char *name );
XLIBDEF BOOL RAR_IsFileInRar( FILE *f );
#ifdef __cplusplus
XLIBDEF BOOL RAR_GetNextFilename( char *filename, int handle=0, int flag=0,
	int *size=NULL, int *year=NULL, int *month=NULL, int *day=NULL,
	int *hour=NULL, int *minute=NULL, int *second=NULL, int *milliseconds=NULL );
#endif
XLIBDEF BOOL IsRarFromName( char *name );



// fli.c

XLIBDEF BOOL PlayFli( char *name );

// selector.c

XLIBDEF BOOL Selector( char *my_dir, char *result_file, char *my_ext );


// jpgshell.c

#define JPEGQUALITY 75


XLIBDEF BOOL IsJpeg( char *name );
XLIBDEF BOOL LoadJpeg( char *name, memptr_ptr spr );
XLIBDEF BOOL GetRgbJpeg( UCHAR *pal );
XLIBDEF void SetRgbJpeg( void );
XLIBDEF BOOL SaveJpeg( char *name, UCHAR *spr, UCHAR *pal );


// file type IDs

#define FI_SIZE 4

#define FIS_LZA "LZA1"
#define FIS_LZH "LZH1"
#define FIS_LZS "LZS1"
#define FIS_RLE "RLE1"
#define FIS_UU	"begi"
#define FIS_ZIP "ZIP1"

#define FIS_SEG ((void*)"SEG1")

#define FIS_SPR "SPR1"          // 8bit palettized or 24bit
#define FIS_FNT "FNT1"
#define FIS_FNT2 "FNT2"
#define FIS_FNT3 "FNT3"

#define FIS_OBJ ((void*)"OBJ1")

#define FIL_RLE 0x524c4531
#define FIL_OBJ 0x4f424a31


// cdrom.c

#define CDDEVICE "/dev/cdrom"

XLIBDEF int StartTrack,EndTrack;

XLIBDEF BOOL InitCdrom( void );
XLIBDEF BOOL DeinitCdrom( void );

XLIBDEF BOOL CD_GetInfo( void );
XLIBDEF void CD_PlayRange( int track, int endtrack );
XLIBDEF void CD_PlayTrack( int track );
XLIBDEF void CD_PlayAll( void );
XLIBDEF void CD_Stop( void );
XLIBDEF void CD_Eject( void );
XLIBDEF void CD_NextTrack( void );
XLIBDEF void CD_PreviousTrack( void );

// mp3.c


typedef struct sound_info_s {

	int size;	   // size of sound data in bytes
	int sample_rate;   // anything i.e. 22000
	int channels;	   // 1 or 2
	int sample_size;   // 1 or 2

} sound_info_t;


XLIBDEF sound_info_t *getsoundinfo( void );
XLIBDEF BOOL load_mp3( FILE *in, FILE *out );


// midi.c


XLIBDEF BOOL MIDI_PlayFile( char *Filename );
XLIBDEF BOOL MIDI_Stop( void );
XLIBDEF BOOL MIDI_Pause( void );
XLIBDEF BOOL MIDI_Resume( void );
XLIBDEF BOOL MIDI_Restart( void );
XLIBDEF void MIDI_PlayRandom( void );
XLIBDEF int MIDI_GetPos( void );
XLIBDEF int MIDI_GetLength( void );
XLIBDEF int MIDI_IsPlaying( void );
XLIBDEF int MIDI_GetVolume( void );

#ifdef __cplusplus
XLIBDEF void MIDI_SetVolume( int left, int right = -1 );
XLIBDEF void checkMixer( BOOL flag = FALSE );
#endif


// Direct X


XLIBDEF int CLIPMINX,CLIPMINY;
XLIBDEF int CLIPMAXX,CLIPMAXY;
XLIBDEF int SCREENW,SCREENH;
#ifdef __cplusplus
XLIBDEF BOOL winWantToQuit( int flag = -1 );
#endif

#define CLIPPANIC(x,y) { if( ((x)<CLIPMINX) || ((x)>CLIPMAXX) || ((y)<CLIPMINY) || ((y)>CLIPMAXY) ) return; }
#define CLIPPANIC0(x,y) { if( ((x)<CLIPMINX) || ((x)>CLIPMAXX) || ((y)<CLIPMINY) || ((y)>CLIPMAXY) ) return NOCOLOR; }

#define SPRITEHEADER		7	/* FLAG(1 byet), X(2), Y(2), FRAME(2) */
#define SPRITEW(x)		(((UCHAR*)(x))[1]+(((UCHAR*)(x))[2]*256))
#define SPRITEH(x)		(((UCHAR*)(x))[3]+(((UCHAR*)(x))[4]*256))
#define SPRITEF(x)		(((UCHAR*)(x))[5]+(((UCHAR*)(x))[6]*256))
#define MKSPRW(spr,w)		{(spr)[1]=(UCHAR)((w)&0xFF);(spr)[2]=(UCHAR)((int)(w)>>8);}
#define MKSPRH(spr,h)		{(spr)[3]=(UCHAR)((h)&0xFF);(spr)[4]=(UCHAR)((int)(h)>>8);}
#define MKSPRF(spr,f)		{(spr)[5]=(UCHAR)((f)&0xFF);(spr)[6]=(UCHAR)((int)(f)>>8);}
#define MKSPR8(spr)		{ ((UCHAR*)(spr))[0] &= ~15; ((UCHAR*)(spr))[0] |= ( 1L << (0) ); }
#define MKSPR16(spr)		{ ((UCHAR*)(spr))[0] &= ~15; ((UCHAR*)(spr))[0] |= ( 1L << (1) ); }
#define MKSPR24(spr)		{ ((UCHAR*)(spr))[0] &= ~15; ((UCHAR*)(spr))[0] |= ( 1L << (2) ); }
#define MKSPR32(spr)		{ ((UCHAR*)(spr))[0] &= ~15; ((UCHAR*)(spr))[0] |= ( 1L << (3) ); }
#define MKANIMSPR(spr)		{ ((UCHAR*)(spr))[0] |= ( 1L << (4) ); }
#define MKSPRBPP(spr)		{ switch( bpp ) { case 8: MKSPR8(spr); break; case 16: MKSPR16(spr); break; case 24: MKSPR24(spr); case 32: MKSPR32(spr); break; } }
#define ISSPR8(spr)		ISFLAG( ((UCHAR*)(spr))[0], 0 )
#define ISSPR16(spr)		ISFLAG( ((UCHAR*)(spr))[0], 1 )
#define ISSPR24(spr)		ISFLAG( ((UCHAR*)(spr))[0], 2 )
#define ISSPR32(spr)		ISFLAG( ((UCHAR*)(spr))[0], 3 )
#define ISANIMSPR(spr)		ISFLAG( ((UCHAR*)(spr))[0], 4 )
#define SPRPIXELLEN(spr)	( ISSPR8(spr) ? 1 : (ISSPR16(spr) ? 2 : (ISSPR24(spr) ? 3 :4) ) )
#define SPRITESIZE(spr) 	( SPRITEHEADER + (SPRITEW(spr)*SPRITEH(spr)*SPRPIXELLEN(spr)) )
#define SPR(spr,x,y)		(*((UCHAR*)(spr) + SPRITEHEADER + (SPRITEW(spr)*(y)*SPRPIXELLEN(spr)) + ((x)*SPRPIXELLEN(spr)) ))
#define SPRLINE(spr,y)		((UCHAR*)((UCHAR*)(spr) + SPRITEHEADER + ((y)* ( SPRITEW(spr)*SPRPIXELLEN(spr) )) ))
#define SPRPTR(spr,x,y)		((UCHAR*)(spr) + SPRITEHEADER + (SPRITEW(spr)*(y)*SPRPIXELLEN(spr)) + ((x)*SPRPIXELLEN(spr)))
#define MKSPR(spr,flag,w,h)	{}


XLIBDEF int bpp,pixel_len;
XLIBDEF int rshift,gshift,bshift;
XLIBDEF int rmask,gmask,bmask;
XLIBDEF int rsize,gsize,bsize;
XLIBDEF __int64 gfxmem;

XLIBDEF int active_flag;
XLIBDEF BOOL bZBuffer;
XLIBDEF BOOL display_zprintf;

XLIBDEF volatile int TimeCount;
XLIBDEF volatile int didupdate;

#define RGBINT(r,g,b)	( (bpp==8) ? (ULONG)FindRgb(r,g,b) : (ULONG) ( ( ( ((ULONG)(r)>>(6-rsize)) & rmask ) << rshift ) | ( ( ((ULONG)(g)>>(6-gsize)) & gmask ) << gshift ) | ( ( ((ULONG)(b)>>(6-bsize)) & bmask ) << bshift ) ) )
#define RGB8INT(r,g,b)	( (bpp==8) ? (ULONG)FindRgb(r,g,b) : (ULONG) ( ( ( ((ULONG)(r)>>(8-rsize)) & rmask ) << rshift ) | ( ( ((ULONG)(g)>>(8-gsize)) & gmask ) << gshift ) | ( ( ((ULONG)(b)>>(8-bsize)) & bmask ) << bshift ) ) )
#define RGBINTP(r,g,b)	( (ULONG) ( ( ( ((ULONG)(r)) & rmask ) << rshift ) | ( ( ((ULONG)(g)) & gmask ) << gshift ) | ( ( ((ULONG)(b)) & bmask ) << bshift ) ) )
#define RGBINT24(r,g,b) ( (((r) << 16) & 0xff0000) | (((g) << 8) & 0xff00) | ((b) & 0xff) )

#define INTRED24(color)		( (UCHAR) ( ( (int)(color) >> 16 ) & 0xff ) )
#define INTGREEN24(color)	( (UCHAR) ( ( (int)(color) >>  8 ) & 0xff ) )
#define INTBLUE24(color)	( (UCHAR) ( ( (int)(color) >>  0 ) & 0xff ) )

#define INTRED16(c)		( (ULONG) ( ((c)>>rshift) & rmask ) )
#define INTGREEN16(c)	( (ULONG) ( ((c)>>gshift) & gmask ) )
#define INTBLUE16(c)	( (ULONG) ( ((c)>>bshift) & bmask ) )

#define INTRED(c)	( (bpp==8) ? (ULONG)system_rgb[((c)*3)+0] : INTRED16(c) )
#define INTGREEN(c)	( (bpp==8) ? (ULONG)system_rgb[((c)*3)+1] : INTGREEN16(c) )
#define INTBLUE(c)	( (bpp==8) ? (ULONG)system_rgb[((c)*3)+2] : INTBLUE16(c) )


#define RGBINT8(c)	( (bpp== 8) ? (ULONG)c : (ULONG)( ((((ULONG)system_rgb[((c)*3)+0]>>(6-rsize)) & rmask) << rshift ) | ((((ULONG)system_rgb[((c)*3)+1]>>(6-gsize)) & gmask) << gshift ) | ((((ULONG)system_rgb[((c)*3)+2]>>(6-bsize)) & bmask) << bshift ) ) )
#define RGBINT16(c)	( (bpp==16) ? (ULONG)c : (ULONG)FindRgb( (ULONG) ( ((c)>>rshift) & rmask ), (ULONG) ( ((c)>>gshift) & gmask ), (ULONG) ( ((c)>>bshift) & bmask ) ) )

#define RGB24(c)	( (ULONG)( ( ( c >> (16+(8-rsize)) ) & rmask ) << rshift ) | (ULONG)( ( ( c >> (8+(8-gsize)) ) & gmask ) << gshift ) | (ULONG)( ( ( c >> (0+(8-bsize)) ) & bmask ) << bshift ) )

// extern void  DX_FreeScreen( void );

// dx.cpp

#define XLIBWNDCLASSNAME "BERNIE_XLIB_WINDOWCLASS"

#define XWINGYOGYDLLNAME  "WINGYOGY.DLL"


XLIBDEF void DX_SetupWindow( void );
XLIBDEF void DX_Setup( void );
XLIBDEF BOOL DX_SetScreen( int w, int h, int d );
XLIBDEF void DX_Cleanup( void );

XLIBDEF void DX_SetRgb( int c, int r, int g, int b );
XLIBDEF void DX_GetRgb( int c, int *r, int *g, int *b );

XLIBDEF int DX_GetPitch( void );

XLIBDEF void DX_InitKeyboard( void );
XLIBDEF void DX_InitMouse( void );
XLIBDEF long GetKeyDX( void );
XLIBDEF void DX_RefreshKey( void );
XLIBDEF void DX_RefreshMouse( void );

XLIBDEF __int64 GFXDLLCALLCONV GetHwnd( void );
XLIBDEF __int64 GFXDLLCALLCONV GetHinstance( void );


extern GFXDLLCALLCONV BOOL XLIB_SendKey( int vk, int sc, int delay );
#ifdef __cplusplus
extern GFXDLLCALLCONV BOOL XLIB_SendMouse( int x, int y, int z=0, int button=-1, int delay=-2 );
#endif

extern GFXDLLCALLCONV BOOL XLIB_HookMouseProcLL( void );
extern GFXDLLCALLCONV void XLIB_UnHookMouseProcLL( void );
extern GFXDLLCALLCONV void XLIB_SetMouseSpeed( FLOAT value );

extern GFXDLLCALLCONV BOOL XLIB_HookMouseProc( void );
extern GFXDLLCALLCONV void XLIB_UnHookMouseProc( void );

extern GFXDLLCALLCONV BOOL XLIB_HookKeyboardProc( void );
extern GFXDLLCALLCONV void XLIB_UnHookKeyboardProc( void );
extern GFXDLLCALLCONV void XLIB_CheckKeyboardHook( void );

extern UCHAR *hardware_keys;


XLIBDEF BOOL DX_SaveWindowPos( void );

#ifdef __cplusplus
XLIBDEF char *winGetDragAndDrop( char *filename = NULL );
XLIBDEF BOOL winSystemTray( int flag=-1, char *tooltip=NULL );
XLIBDEF void winMinimize( int flag=-1 );
#endif


#define SETX_JUSTWINDOW		0
#define SETX_NORMAL		1
#define SETX_FORCERND		2	// random
#define SETX_FORCEDX		3	// force DirectX
#define SETX_FORCEOGL		4	// opengl
#define SETX_FORCESFT		5	// software
#define SETX_FORCESDL		6	// SDL
#define SETX_FORCEGLD		7	// Glide
#define SETX_NOBORDER		8	// no border

extern BOOL GFXDLLCALLCONV XLIB_SetX( int width, int height, int depth, int flag );
extern void GFXDLLCALLCONV XLIB_ResetX( void );

extern BOOL (GFXDLLCALLCONV *SetX)( int width, int height, int depth, int flag );
extern void (GFXDLLCALLCONV *ResetX)( void );

XLIBDEF void WaitRetrace(int i);
XLIBDEF void WaitVerticalRetraceStart(void);
XLIBDEF void WaitVerticalRetraceEnd(void);
XLIBDEF void WaitHorizontalRetraceStart(void);
XLIBDEF void WaitHorizontalRetraceEnd(void);

XLIBDEF void *GetVideoPtr( int x, int y );

XLIBDEF void Message( char *s, ... );
XLIBDEF void MessageTitle( char *title, char *s, ... );
XLIBDEF void MessageError( char *s, ... );
XLIBDEF BOOL AskYesNo( char *s, ...);
XLIBDEF BOOL AskYesNoTitle( char *title, char *s, ...);
XLIBDEF BOOL winAskYesNo( char *title, char *s, ... );
XLIBDEF void winMsgTimer( int time, char *title, char *s, ... );
XLIBDEF void winTextBox( char *title, char *s, ... );

#ifdef __cplusplus
#define XLIB_WLBMAXSTRLEN  128
XLIBDEF int winListBox( char *ptr /* = num * WLBMAXSTRLEN*/, int num, char *title=NULL );
XLIBDEF BOOL winGetInput( char *str, char *title=NULL );
#endif


XLIBDEF void SetupTimer( void );
XLIBDEF void CleanupTimer( void );

#ifdef __cplusplus
XLIBDEF void FrameRateLimiter( int fps=60 );
#endif


// avi.cpp

// olvasás

extern BOOL GFXDLLCALLCONV XLIB_AVI_Open( char* szFilename );
extern void GFXDLLCALLCONV XLIB_AVI_Close( void );
#ifdef __cplusplus
extern void GFXDLLCALLCONV XLIB_AVI_DrawFrame( int Frame, int x=-1, int y=-1, int w=-1, int h=-1 );
#endif
extern UCHAR* GFXDLLCALLCONV XLIB_AVI_GetFrame( int Frame, int flag );

extern BOOL (GFXDLLCALLCONV *AVI_Open)( char* szFilename );
extern void (GFXDLLCALLCONV *AVI_Close)( void );
// extern void (GFXDLLCALLCONV *AVI_DrawFrame)( int Frame, int x=-1, int y=-1, int w=-1, int h=-1 );
extern void (GFXDLLCALLCONV *AVI_DrawFrame)( int Frame, int x, int y, int w, int h );
extern UCHAR* (GFXDLLCALLCONV *AVI_GetFrame)( int Frame, int flag );

XLIBDEF int AVI_GetNumFrame( void );
XLIBDEF int AVI_GetFps( void );
XLIBDEF int AVI_GetTimeTick( void );


// írás

#ifdef __cplusplus
XLIBDEF BOOL AVI_Create( char *filename, int dwFrameRate=25 );
#endif
XLIBDEF void AVI_Finalize( void );
XLIBDEF BOOL AVI_Frame( void );


// ffmpegshell.cpp

XLIBDEF BOOL IsFFMPEG( char *filename );
XLIBDEF BOOL FFMPEG_Open( char *filename );
#ifdef __cplusplus
XLIBDEF BOOL FFMPEG_GetFrame( UCHAR *spr, UCHAR *sound=NULL, int *maxsoundlen=NULL );
XLIBDEF BOOL FFMPEG_GetInfo( int *width=NULL, int *height=NULL, int *fps=NULL, int *bpp=NULL, int *currframe=NULL, int *numframes=NULL );
#endif
XLIBDEF BOOL FFMPEG_SkipFrame( int num );
XLIBDEF void FFMPEG_Close( void );


// debug.cpp

XLIBDEF BOOL DB_InitDebug( void );
XLIBDEF void DB_DeinitDebug( void );
XLIBDEF void DB_StackWalk( void );


// win.cpp

extern GFXDLLCALLCONV ULONG XLIB_FindRandomPid( void );
extern GFXDLLCALLCONV BOOL XLIB_FindRandomWindow( char *str );
extern GFXDLLCALLCONV void XLIB_PrintWindow( void );
#ifdef __cplusplus
extern GFXDLLCALLCONV __int64 XLIB_FindWindow( char *name, int mode=0, BOOL onlyvisible = FALSE, BOOL currentuseronly = FALSE );
extern GFXDLLCALLCONV __int64 XLIB_FindNextWindow( char *name, int mode=0, BOOL onlyvisible = FALSE, BOOL currentuseronly = FALSE );
extern GFXDLLCALLCONV __int64 XLIB_FindNextHWND( __int64 hwnd, BOOL onlyvisible = FALSE, BOOL currentuseronly = FALSE );
#endif
extern GFXDLLCALLCONV BOOL XLIB_KillWindow( __int64 ihwnd );
extern GFXDLLCALLCONV ULONG XLIB_GetPidFromExe( char *procName );
extern GFXDLLCALLCONV __int64 XLIB_GetWindowUnderMouse( void );

extern GFXDLLCALLCONV BOOL XLIB_GetExeNameFromPid( ULONG dwProcessID, char *exename );
extern GFXDLLCALLCONV BOOL XLIB_GetExeNameFromHwnd( __int64 hwnd, char *exename );


#ifdef __cplusplus
extern GFXDLLCALLCONV ULONG XLIB_GetModuleBaseAddress( UINT dwProcessIdentifier, ULONG *size=NULL );
XLIBDEF BOOL SetCore( int inprocessid=-1, int core=-1 );
#endif
extern GFXDLLCALLCONV void XLIB_SetPrivileges( void );

// memcpy() replacements:

XLIBDEF void int32copy( ULONG *dst, ULONG *src, int count );
XLIBDEF void int24copy( UCHAR *dst, UCHAR *src, int count );
XLIBDEF void int16copy( USHORT *dst, USHORT *src, int count );
#define int8copy memcpy

XLIBDEF BOOL KockaEffekt( __int64 ul_hwnd, ULONG ul_msg, ULONG ul_wParam, ULONG ul_lParam );
XLIBDEF void SetKockaEffektParam( int button );

XLIBDEF void Delay( ULONG delay );
extern GFXDLLCALLCONV void XLIB_Delay( ULONG delay );  // threaded
extern GFXDLLCALLCONV int XLIB_SetWindowTheme( __int64 ihwnd, char *szSubAppName, char *szSubIdList );

extern void GFXDLLCALLCONV XLIB_winBeep( void );
extern void (GFXDLLCALLCONV *winBeep)( void );
#ifdef __cplusplus
extern void GFXDLLCALLCONV XLIB_Shutdown( BOOL reboot = FALSE );
#endif


#define IsKeyDown9xNT(vk) ((GetAsyncKeyState(vk) & 0x8000) || ((GetKeyState(vk) & 0x8000)))
#define IsKeyDown2kXP(vk) (GetKeyState(vk) & 0x8000)
#define IsKeyDownAsync(vk) (GetAsyncKeyState(vk) & 0x8000)
#define IsKeyToggledOn(vk) (GetKeyState(vk) & 0x01)

extern GFXDLLCALLCONV int StringToVirtkey( char *str );
extern GFXDLLCALLCONV int StringToScancode( char *str );
extern GFXDLLCALLCONV int sc2vk( int aSC );
#ifdef __cplusplus
extern GFXDLLCALLCONV int vk2sc( int aVK, BOOL aReturnSecondary = FALSE );
#endif

// memset() replacements:

XLIBDEF void int32set( ULONG *dst, int data, int count );
XLIBDEF void int24set( UCHAR *dst, int data, int count );
XLIBDEF void int16set( USHORT *dst, int data, int count );
#define int8set memset

extern char* GFXDLLCALLCONV XLIB_GetWinError( void );
extern char* (GFXDLLCALLCONV *GetWinError)( void );

extern char* GFXDLLCALLCONV XLIB_winTemp( void );
extern char* (GFXDLLCALLCONV *winTemp)( void );

extern void GFXDLLCALLCONV XLIB_DumpProcessInfo( void );
extern void (GFXDLLCALLCONV *DumpProcessInfo)( void );

typedef struct winmeminfo_s {
	ULONG dwMemoryLoad;
	ULONG dwTotalPhys;
	ULONG dwAvailPhys;
	ULONG dwTotalPageFile;
	ULONG dwAvailPageFile;
	ULONG dwTotalVirtual;
	ULONG dwAvailVirtual;
} winmeminfo_t,*winmeminfo_ptr;


extern void GFXDLLCALLCONV XLIB_GetWinMemInfo( winmeminfo_ptr );
extern void (GFXDLLCALLCONV *GetWinMemInfo)( winmeminfo_ptr );

extern const char* GFXDLLCALLCONV XLIB_winGetMachineName( void );
extern const char* GFXDLLCALLCONV XLIB_winGetUserName( void );
extern const char *(GFXDLLCALLCONV *winGetMachineName)( void );
extern const char *(GFXDLLCALLCONV *winGetUserName)( void );

XLIBDEF char winGetDecimalSep( void );
XLIBDEF void winCurrScreenRes( int *x, int *y, int *bpp );

XLIBDEF BOOL winGlideDll( void );

extern void GFXDLLCALLCONV XLIB_winSetCaption( char *str, ... );
extern void (GFXDLLCALLCONV *winSetCaption)( char *str, ... );

XLIBDEF ULONG GetDriveInfo( void );
XLIBDEF BOOL GetDriveSpace( char *lpRootPathName,
				int *lpSectorsPerCluster,
				int *lpBytesPerSector,
				int *lpNumberOfFreeClusters,
				int *lpTotalNumberOfClusters );

extern int GFXDLLCALLCONV XLIB_winGetCurrentDirectory( char *dir, int num );
extern BOOL GFXDLLCALLCONV XLIB_winSetCurrentDirectory( char *dir );
extern int (GFXDLLCALLCONV *winGetCurrentDirectory)( char *dir, int num );
extern BOOL (GFXDLLCALLCONV *winSetCurrentDirectory)( char *dir );

extern ULONG GFXDLLCALLCONV XLIB_winFindTargetWindow( char *windowName );
extern ULONG (GFXDLLCALLCONV *winFindTargetWindow)( char *windowName );

extern BOOL GFXDLLCALLCONV XLIB_winBrowseForFolder( char *szPath );
extern BOOL (GFXDLLCALLCONV *winBrowseForFolder)( char *szPath );

extern BOOL GFXDLLCALLCONV XLIB_winIsConnected( void );
extern BOOL (GFXDLLCALLCONV *winIsConnected)( void );

extern int GFXDLLCALLCONV XLIB_winLoadBitmap( void );
extern int GFXDLLCALLCONV XLIB_winPlayWave( char *str );
extern int (GFXDLLCALLCONV *winLoadBitmap)( void );
extern int (GFXDLLCALLCONV *winPlayWave)( char *str );

extern void* GFXDLLCALLCONV XLIB_winFindFirstFile( char *mask, findfile_ptr find );
extern BOOL GFXDLLCALLCONV XLIB_winFindNextFile( void *handle, findfile_ptr find );
extern BOOL GFXDLLCALLCONV XLIB_winFindClose( void *handle );
extern BOOL GFXDLLCALLCONV XLIB_winDeleteFile( char *name );

extern void *(GFXDLLCALLCONV *winFindFirstFile)( char *mask, findfile_ptr find );
extern BOOL (GFXDLLCALLCONV *winFindNextFile)( void *handle, findfile_ptr find );
extern BOOL (GFXDLLCALLCONV *winFindClose)( void *handle );
extern BOOL (GFXDLLCALLCONV *winDeleteFile)( char *name );

extern int GFXDLLCALLCONV XLIB_winCopyTextToClipboard( char *str );
extern BOOL GFXDLLCALLCONV XLIB_winGetSysDirectory( char *szPath );
extern char* GFXDLLCALLCONV XLIB_winPasteTextFromClipboard( void );
extern char *(GFXDLLCALLCONV *winPasteTextFromClipboard)( void );
extern int (GFXDLLCALLCONV *winCopyTextToClipboard)( char *str );
extern UCHAR * GFXDLLCALLCONV XLIB_winPasteImageFromClipboard( void );
extern UCHAR *(GFXDLLCALLCONV *winPasteImageFromClipboard)( void );
extern BOOL (GFXDLLCALLCONV *winGetSysDirectory)( char *szPath );



extern BOOL GFXDLLCALLCONV XLIB_winGetFontDir( char *szPath );
extern BOOL (GFXDLLCALLCONV *winGetFontDir)( char *szPath );

extern BOOL GFXDLLCALLCONV XLIB_winGetProgramFilesDir( char *szPath );
extern BOOL (GFXDLLCALLCONV *winGetProgramFilesDir)( char *szPath );

XLIBDEF BOOL DirExist( char *dirname );

XLIBDEF void CreatePath( char *str );
XLIBDEF char *winMakeProgDataDir( void );

extern BOOL GFXDLLCALLCONV XLIB_winGetMyDocumentsDir( char *szPath );
extern BOOL (GFXDLLCALLCONV *winGetMyDocumentsDir)( char *szPath );

extern void GFXDLLCALLCONV XLIB_GetUptime( void );
extern void (GFXDLLCALLCONV *GetUptime)( void );

extern int GFXDLLCALLCONV XLIB_GetCPUSpeed( void );
extern int (GFXDLLCALLCONV *GetCPUSpeed)( void );

extern int GFXDLLCALLCONV XLIB_winTimeZone( void );
extern int (GFXDLLCALLCONV *winTimeZone)( void );

extern void GFXDLLCALLCONV XLIB_GetVariables( void );

extern void GFXDLLCALLCONV XLIB_GetBios( void );

extern BOOL XLIB_SetBrightness( int wBrightness );

XLIBDEF int winGotoUrl( char *url );

XLIBDEF void winRegisterExtensions( void );
XLIBDEF void winUnRegisterExtensions( void );


extern BOOL GFXDLLCALLCONV XLIB_CheckBernie( void );
extern BOOL (GFXDLLCALLCONV *CheckBernie)( void );

XLIBDEF BOOL winNoDinput( void );
XLIBDEF BOOL winNoMusic( void );
XLIBDEF BOOL winNoSound( void );
XLIBDEF BOOL winNoLog( void );
XLIBDEF BOOL winCDPlayer( void );
XLIBDEF BOOL winJoystick( void );
XLIBDEF BOOL winEmail( void );
XLIBDEF BOOL winDebug( void );
XLIBDEF BOOL GFXDLLCALLCONV winFullscreen( void );
XLIBDEF BOOL winTexture( void );
XLIBDEF BOOL GFXDLLCALLCONV winOpengl( void );
extern int texture_scale;
XLIBDEF BOOL winAlwaysUse( void );

extern BOOL GFXDLLCALLCONV XLIB_winMagyar( void );
extern BOOL (GFXDLLCALLCONV *winMagyar)( void );


XLIBDEF void winSetNoDinput( BOOL val );
XLIBDEF void winSetJoystick( BOOL val );
XLIBDEF void winSetNoMusic( BOOL val );
XLIBDEF void winSetNoSound( BOOL val );
XLIBDEF void winSetNoLog( BOOL val );
XLIBDEF void winSetCDPlayer( BOOL val );
XLIBDEF void winSetEmail( BOOL val );
XLIBDEF void winSetDebug( BOOL val );
XLIBDEF void winSetFullscreen( BOOL val );
XLIBDEF void winSetTexture( BOOL val );
XLIBDEF void winSetOpengl( BOOL val );
XLIBDEF void winSetAlwaysUse( BOOL val );

#ifdef __cplusplus
XLIBDEF BOOL winLogActionButton( BOOL reset=TRUE );
XLIBDEF BOOL winGetFullPathName( char *filename, char *full_path, int len=0 );
XLIBDEF void winListDir( char *dirname=NULL, void dirlistproc(char *filename,int,int,int,int,int,int,int,int)=NULL, BOOL hidden=FALSE, int depth=0, BOOL nosubdir=FALSE );
XLIBDEF BOOL winClock( int duration = -1 );
#endif

XLIBDEF BOOL winIsSameFile( char *file1, char *file2 );


#ifdef __cplusplus
BOOL winShareware( BOOL force_dlg = FALSE );
#endif
BOOL checkShareware( void );
XLIBDEF char *winGetRegisteredName( void );
XLIBDEF char *winGetRegisteredKey( void );

#ifdef __cplusplus
extern int GFXDLLCALLCONV XLIB_winGetUrlDocument( char *url, UCHAR **buf=NULL, char *filename=NULL );
#endif
extern BOOL GFXDLLCALLCONV XLIB_getNetVersion( char *url, int *major, int *minor, char *download );
extern BOOL GFXDLLCALLCONV XLIB_CheckVersion( char *name, char *version );
extern int (GFXDLLCALLCONV *winGetUrlDocument)( char *url, UCHAR **buf );
extern BOOL (GFXDLLCALLCONV *getNetVersion)( char *url, int *major, int *minor, char *download );
extern BOOL (GFXDLLCALLCONV *CheckVersion)( char *name, char *version );

XLIBDEF BOOL loadWininet( void );
XLIBDEF void unloadWininet( void );
XLIBDEF char *winInetErrorStr( char *s, ... );
XLIBDEF void winInetAgent( char *s );

XLIBDEF BOOL Path2URL( char *path, char *url );
XLIBDEF BOOL winShortPath( char *fullpath, char *shortpath );

extern void XLIB_SetCurlAgent( char *s );

#ifdef __cplusplus
extern int GFXDLLCALLCONV XLIB_GetCurlURL( char *url, UCHAR **buf=NULL, char *filename=NULL );
#endif


// gui.cpp

XLIBDEF BOOL InitLogWindow( void );
XLIBDEF void DeinitLogWindow( void );
XLIBDEF void AddLogWindow( char * );

XLIBDEF void winShowLog( void );
XLIBDEF void winHideLog( void );
XLIBDEF void winToggleLog( void );
XLIBDEF BOOL winLogState( void );
XLIBDEF void winSetLogCaption( char *s, ... );
XLIBDEF void winLogClear( void );
XLIBDEF BOOL winLogActionState( BOOL state );
XLIBDEF BOOL winLogSendMsg( char *str );
XLIBDEF BOOL winLogGetMsg( char *str );


extern void GFXDLLCALLCONV XLIB_winSetWindowTrans( __int64 hwnd, BOOL bTransparent, int nTransparency );
extern void (GFXDLLCALLCONV *winSetWindowTrans)( __int64 hwnd, BOOL bTransparent, int nTransparency );

extern UINT GFXDLLCALLCONV XLIB_winGetMouseScrollLines( void );
extern UINT (GFXDLLCALLCONV *winGetMouseScrollLines)( void );

XLIBDEF char *SetLauncherHtml( char *str );


// str = http://www.google.com
// html_page = maga a .html file
// return (__int64)BROWSERHWND
#ifdef __cplusplus
XLIBDEF __int64 winBrowser( char *str = NULL, char *html_page = NULL );
#endif

XLIBDEF BOOL winBrowserCallback( void );
XLIBDEF BOOL winBrowserEnd( void );
XLIBDEF BOOL winBrowserActive( void );
XLIBDEF BOOL winBrowserGetPage( char *str, int maxlen );
XLIBDEF BOOL winBrowserReady( void );
XLIBDEF BOOL winBrowserClick( char *str );
XLIBDEF BOOL winBrowserSetValue( char *tag, char *str );
XLIBDEF BOOL winBrowserComboBox( char *tag, int index );



#define COMBOBOX_MAXSEL 30
#define COMBOBOX_MAXDESC 10
#define COMBOBOX_MAXSTR 256

typedef struct combobox_s {

	char str[COMBOBOX_MAXSEL][COMBOBOX_MAXDESC][COMBOBOX_MAXSTR];
	char data[COMBOBOX_MAXSEL][COMBOBOX_MAXSTR];
	int nstrs,cursel;

} combobox_t, *combobox_ptr;



XLIBDEF int winInitComboBox( void );
XLIBDEF int winComboBoxAddString( char *string );
XLIBDEF int winComboBox( int cursel, int *width, int *height, int *depth, int *flag );


XLIBDEF char *GetExeDir( void );
XLIBDEF char *GetExeName( void );
XLIBDEF char *GetDataDir( void );


#ifdef __cplusplus
XLIBDEF BOOL winOpenFile( char *filter, char *title=NULL, char *dir=NULL );
#endif
extern char* GFXDLLCALLCONV XLIB_winGetNextFile( void );
extern char *(GFXDLLCALLCONV *winGetNextFile)( void );

extern void GFXDLLCALLCONV XLIB_EnumerateProcessorType( void );
extern void (GFXDLLCALLCONV *EnumerateProcessorType)( void );

extern BOOL GFXDLLCALLCONV XLIB_IsSofticeLoaded( void );
extern BOOL (GFXDLLCALLCONV *IsSofticeLoaded)( void );

extern int GFXDLLCALLCONV XLIB_winReadProfileInt( char *key, int default_value );
extern void GFXDLLCALLCONV XLIB_winWriteProfileInt( char *key, int value );
extern FLOAT GFXDLLCALLCONV XLIB_winReadProfileFloat( char *key, FLOAT default_value );
extern void GFXDLLCALLCONV XLIB_winWriteProfileFloat( char *key, FLOAT value );
extern void GFXDLLCALLCONV XLIB_winReadProfileString( char *key, char *default_string, char *string, int size );
extern void GFXDLLCALLCONV XLIB_winWriteProfileString( char *key, char *string );
extern BOOL GFXDLLCALLCONV XLIB_winDeleteProfileKey( char *key );

extern int (GFXDLLCALLCONV *winReadProfileInt)( char *key, int default_value );
extern void (GFXDLLCALLCONV *winWriteProfileInt)( char *key, int value );
extern FLOAT (GFXDLLCALLCONV *winReadProfileFloat)( char *key, FLOAT default_value );
extern void (GFXDLLCALLCONV *winWriteProfileFloat)( char *key, FLOAT value );
extern void (GFXDLLCALLCONV *winReadProfileString)( char *key, char *default_string, char *string, int size );
extern void (GFXDLLCALLCONV *winWriteProfileString)( char *key, char *string );
extern BOOL (GFXDLLCALLCONV *winDeleteProfileKey)( char *key );

extern int GFXDLLCALLCONV XLIB_winLogLevel( void );
extern int (GFXDLLCALLCONV *winLogLevel)( void );

#ifdef __cplusplus
extern int GFXDLLCALLCONV XLIB_Message( char *str, char *title=NULL );
#endif

XLIBDEF void winGetMessage( void );
XLIBDEF void DX_GetMessage( void );
extern void GFXDLLCALLCONV XLIB_MinimalMessagePump( void );

extern BOOL GFXDLLCALLCONV XLIB_xsystem( const char *s, ... );  // non-blocking
extern BOOL GFXDLLCALLCONV XLIB_winExec( char *sz ); 		// blocking

extern void* GFXDLLCALLCONV XLIB_winAlloc( int amount );
extern void GFXDLLCALLCONV XLIB_winFree( void *buf );

extern void (GFXDLLCALLCONV *xsystem)( char * );
extern void *(GFXDLLCALLCONV *winAlloc)( int amount );
extern void (GFXDLLCALLCONV *winFree)( void *buf );

XLIBDEF BOOL winSetSplashText( char *str );
XLIBDEF BOOL winShowSplash( void );
XLIBDEF void winHideSplash( void );

#ifdef __cplusplus
XLIBDEF void winSetPanel( char *str=NULL, int curr=-1, int max=-1 );
XLIBDEF BOOL winShowPanel( char *str=NULL, char *title=NULL );
#endif
XLIBDEF void winHidePanel( void );

extern char *XLIB_winSetProgramDesc( char *str );		// xlib.dll
extern char *getProgramDesc( void );				// applikációban


XLIBDEF void winWaitCursor( void );
XLIBDEF void winNormalCursor( void );

XLIBDEF __int64 GFXDLLCALLCONV GetBackHwnd( void );
XLIBDEF void winShowBackWindow( void );
#ifdef __cplusplus
XLIBDEF void winHideBackWindow( BOOL destroy = TRUE );
#else
XLIBDEF void winHideBackWindow( BOOL );
#endif
XLIBDEF void winSetBackImage( BOOL regist );

extern BOOL GFXDLLCALLCONV XLIB_winLoadFonts( void );
extern void GFXDLLCALLCONV XLIB_winRemoveFonts( void );
extern BOOL (GFXDLLCALLCONV *winLoadFonts)( void );
extern void (GFXDLLCALLCONV *winRemoveFonts)( void );

XLIBDEF char *GetSplashFilename( void );
XLIBDEF char *GetBackgroundFilename( void );
XLIBDEF void FreeFilenamString( void );


extern int GFXDLLCALLCONV XLIB_winGetMaxWidth( void );
extern int GFXDLLCALLCONV XLIB_winGetMaxHeight( void );
extern int (GFXDLLCALLCONV *winGetMaxWidth)( void );
extern int (GFXDLLCALLCONV *winGetMaxHeight)( void );


// cefshell.cpp

XLIBDEF BOOL InitCef( __int64 ul_hwnd, char *url );
XLIBDEF void DeinitCef( void );
XLIBDEF BOOL LoadCef( void );
XLIBDEF BOOL IsCef( void );
XLIBDEF void CefLoop( void );

// xmain.cpp

extern int GFXDLLCALLCONV usermain( int argc, char *argv[], char *envp[] );

extern BOOL (GFXDLLCALLCONV *XLIB_winShareware)( BOOL );


// fullos deinit sound, kiegeszitendo ha tobb lesz
#define DEINITSOUND \
	SL_StopMod(); \
	SL_FreeAllWave(); \
	SID_StopSid(); \
	AD_Stop(); \
	YM_Stop(); \
	NR_Stop(); \
	SL_Deinit(); \
	MIKLIB_Deinit(); \
	/* MIDI_Stop(); */ \
	DS_Deinit();


// flacshell.cpp
XLIBDEF BOOL IsFlac( char *buf, int len );
XLIBDEF BOOL DecodeFlac( UCHAR *in_ptr, int in_ptr_size, UCHAR **out_ptr, int *out_ptr_size );
XLIBDEF BOOL EncodeFlac( UCHAR *in_ptr, int in_ptr_size, UCHAR **out_ptr, int *out_ptr_size );

// libsndshell.cpp
XLIBDEF BOOL IsSnd( char *buf, int len );
#ifdef __cplusplus
XLIBDEF BOOL DecodeSnd( UCHAR *in_ptr, int in_ptr_size, UCHAR **out_ptr, int *out_ptr_size, int *pChannel=NULL, int *pBps=NULL, int *pFreq=NULL, BOOL convert_mono=FALSE );
#endif
XLIBDEF BOOL EncodeSnd( UCHAR *in_ptr, int in_ptr_size, UCHAR **out_ptr, int *out_ptr_size );


// sound.cpp

#define MINPAN	0
#define MIDPAN	127
#define MAXPAN	255
#define MINVOL	0
#define MIDVOL	32
#define MAXVOL	64


XLIBDEF BOOL DS_Init( void );
XLIBDEF void DS_Deinit( void );
XLIBDEF BOOL DS_IsLoaded( char *name );
XLIBDEF int DS_LoadWave( char *szFileName );
XLIBDEF int DS_LoadMp3( char *szFileName );
XLIBDEF void DS_PlayWave( int handler );
XLIBDEF void DS_StopWave( int handler );
XLIBDEF void DS_FreeWave( int handler );
XLIBDEF void DS_FreeAllWave( void );
XLIBDEF void DS_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped );
XLIBDEF int DS_IsPlaying( int handler );
XLIBDEF int DS_NewSound( int id, UCHAR *header, int audiosize );
XLIBDEF BOOL DS_CopyWaveBuffer( int handle, UCHAR *data, int len );



// SEAL xaudio.h
// mikwin
// fmod

#include "music.h"

#ifdef FMOD_LIB

#define SL_NAME "FMOD"

#define SL_Init				FMOD_Init
#define SL_Deinit			FMOD_Deinit
#define SL_PlayMod			FMOD_PlayMod
#define SL_StopMod			FMOD_StopMod
#define SL_SetModuleParam	FMOD_SetModuleParam
#define SL_GetModuleParam	FMOD_GetModuleParam
#define SL_IsModulePlaying	FMOD_IsModulePlaying
#define SL_LoadWave			FMOD_LoadWave
#define SL_LoadStream		FMOD_LoadStream
#define SL_PlayWave			FMOD_PlayWave
#define SL_StopWave			FMOD_StopWave
#define SL_FreeAllWave		FMOD_FreeAllWave
#define SL_SetWaveParam		FMOD_SetWaveParam
#define SL_IsLoaded			FMOD_IsLoaded
#define SL_IsPlaying		FMOD_IsPlaying
#define SL_Pause			FMOD_Pause
#define SL_Paused			FMOD_Paused

#endif


XLIBDEF BOOL FMOD_Init( void );
XLIBDEF BOOL FMOD_Deinit( void );
XLIBDEF BOOL FMOD_PlayMod( char *txt );
XLIBDEF BOOL FMOD_StopMod( void );
XLIBDEF BOOL FMOD_SetModuleParam( int vol, int nOrder, int nRow );
XLIBDEF BOOL FMOD_GetModuleParam( int *vol, int *ord, int *row );
XLIBDEF BOOL FMOD_IsModulePlaying( void );
XLIBDEF int FMOD_LoadWave( char *szFileName, int id );
XLIBDEF int FMOD_LoadStream( char *szFileName, int id );
XLIBDEF void FMOD_PlayWave( int handler );
XLIBDEF void FMOD_FreeAllWave( void );
XLIBDEF void FMOD_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped );
XLIBDEF BOOL FMOD_IsLoaded( char *name );
XLIBDEF int FMOD_IsPlaying( int handler );
XLIBDEF void FMOD_StopWave( int handler );


#ifdef SEAL_LIB

#define SL_NAME "SEAL"

#define SL_Init				SEAL_Init
#define SL_Deinit			SEAL_Deinit
#define SL_PlayMod			SEAL_PlayMod
#define SL_StopMod			SEAL_StopMod
#define SL_SetModuleParam	SEAL_SetModuleParam
#define SL_GetModuleParam	SEAL_GetModuleParam
#define SL_IsModulePlaying	SEAL_IsModulePlaying
#define SL_LoadWave			SEAL_LoadWave
#define SL_LoadStream		SEAL_LoadStream
#define SL_PlayWave			SEAL_PlayWave
#define SL_StopWave			SEAL_StopWave
#define SL_FreeAllWave		SEAL_FreeAllWave
#define SL_SetWaveParam		SEAL_SetWaveParam
#define SL_IsLoaded			SEAL_IsLoaded
#define SL_IsPlaying		SEAL_IsPlaying
#define SL_Pause			SEAL_Pause
#define SL_Paused			SEAL_Paused

#endif

XLIBDEF BOOL SEAL_Init( void );
XLIBDEF BOOL SEAL_Deinit( void );
XLIBDEF BOOL SEAL_PlayMod( char *txt );
XLIBDEF BOOL SEAL_StopMod( void );
XLIBDEF int SEAL_LoadWave( char *szFileName, int id );
XLIBDEF void SEAL_PlayWave( int handler );
XLIBDEF void SEAL_StopWave( int handler );
XLIBDEF void SEAL_FreeWave( int handler );
XLIBDEF void SEAL_FreeAllWave( void );
XLIBDEF void SEAL_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped );
XLIBDEF BOOL SEAL_IsLoaded( char *name );
XLIBDEF BOOL SEAL_IsPlaying( int handler );
XLIBDEF BOOL SEAL_IsModulePlaying( void );
XLIBDEF BOOL SEAL_SetModuleParam( int vol, int nOrder, int nRow );
XLIBDEF BOOL SEAL_GetModuleParam( int *vol, int *ord, int *row );

XLIBDEF void SEAL_PlayRandom( void );



#ifdef MIKMOD_LIB

#define SL_NAME "MIKMOD"

#define SL_Init			MIKLIB_Init
#define SL_Deinit		MIKLIB_Deinit
#define SL_PlayMod		MIKLIB_PlayMod
#define SL_StopMod		MIKLIB_StopMod
#define SL_SetModuleParam	MIKLIB_SetModuleParam
#define SL_GetModuleParam	MIKLIB_GetModuleParam
#define SL_IsModulePlaying	MIKLIB_IsModulePlaying
#define SL_LoadWave		MIKLIB_LoadWave
#define SL_LoadStream		MIKLIB_LoadStream
#define SL_PlayWave		MIKLIB_PlayWave
#define SL_StopWave		MIKLIB_StopWave
#define SL_FreeAllWave		MIKLIB_FreeAllWave
#define SL_SetWaveParam		MIKLIB_SetWaveParam
#define SL_IsLoaded		MIKLIB_IsLoaded
#define SL_IsPlaying		MIKLIB_IsPlaying
#define SL_Pause		MIKLIB_Pause
#define SL_Paused		MIKLIB_Paused

#endif

XLIBDEF void MIKLIB_Deinit( void );
XLIBDEF BOOL MIKLIB_PlayMod( char * filename );
XLIBDEF void MIKLIB_StopMod( void );
XLIBDEF BOOL MIKLIB_IsModulePlaying( void );
#ifdef __cplusplus
XLIBDEF BOOL MIKLIB_Init( int freq = -1 );
XLIBDEF BOOL MIKLIB_SetModuleParam( int vol=-1, int order=-1, int row=-1, int loop=-1 );
XLIBDEF BOOL MIKLIB_GetModuleParam( int *vol=NULL, int *ord=NULL, int *row=NULL,
				   int *max_order = NULL, int *percent=NULL,
				   char **song_name = NULL,
				   int *max_chan = NULL );

XLIBDEF BOOL MIKLIB_GetWaveParam( int handler, int *percent=NULL, int *time=NULL, int *maxTime=NULL, int *mypos=NULL, int *myMaxPos=NULL, char **name=NULL );
XLIBDEF void MIKLIB_SetWaveParam( int handler, int dwFreq=-1, int dwVol=-1, int dwPan=-1, int fLooped=-1, int percent=-1 );
XLIBDEF char *MIKLIB_GetStrings( int num, int flag = 0 );
XLIBDEF int MIKLIB_GetInfo( int flag=0 );
XLIBDEF int MIKLIB_LoadWave( char *szFileName, int id = -1 );
#endif
XLIBDEF void MIKLIB_PlayWave( int handler );
XLIBDEF void MIKLIB_FreeWave( int handler );
XLIBDEF void MIKLIB_FreeAllWave( void );
XLIBDEF BOOL MIKLIB_IsLoaded( char *name );
XLIBDEF int MIKLIB_IsPlaying( int handler );
XLIBDEF void MIKLIB_StopWave( int handler );
XLIBDEF void MIKLIB_Pause( void );
XLIBDEF BOOL MIKLIB_Paused( void );


#define PLAYSOUNDMIKMOD( name ) { MIKLIB_PlayWave( MIKLIB_LoadWave( name ) ); }
#define PLAYSOUNDVOLMIKMOD( name, vol ) { MIKLIB_SetWaveParam( MIKLIB_LoadWave( name ), -1, (vol), -1, -1 ); MIKLIB_PlayWave( MIKLIB_LoadWave( name ) ); }
#define PLAYSOUNDPARAMMIKMOD( name, vol, pan, loop ) { MIKLIB_SetWaveParam( MIKLIB_LoadWave( name ), -1, (vol), (pan), (loop) ); MIKLIB_PlayWave( MIKLIB_LoadWave( name ) ); }


// bass.cpp
// bassshell.cpp

#ifdef BASS_LIB

#define SL_NAME "BASS"

#define SL_Init			BLIB_Init
#define SL_Deinit		BLIB_Deinit
#define SL_PlayMod		BLIB_PlayMod
#define SL_StopMod		BLIB_StopMod
#define SL_SetModuleParam	BLIB_SetModuleParam
#define SL_GetModuleParam	BLIB_GetModuleParam
#define SL_IsModulePlaying	BLIB_IsModulePlaying
#define SL_LoadWave		BLIB_LoadWave
#define SL_LoadStream		BLIB_LoadStream
#define SL_PlayWave		BLIB_PlayWave
#define SL_StopWave		BLIB_StopWave
#define SL_FreeWave		BLIB_FreeWave
#define SL_FreeAllWave		BLIB_FreeAllWave
#define SL_SetWaveParam		BLIB_SetWaveParam
#define SL_GetWaveParam		BLIB_GetWaveParam
#define SL_IsLoaded		BLIB_IsLoaded
#define SL_IsPlaying		BLIB_IsPlaying
#define SL_Pause		BLIB_Pause
#define SL_Paused		BLIB_Paused
#define SL_ChannnelGetData	BLIB_ChanGetData
#define SL_GetSilenceLength	BLIB_GetSilenceLength
#define SL_GetStrings		BLIB_GetStrings
#define SL_DrawSpectrum		BLIB_DrawSpectrum
#define SL_GetInfo		BLIB_GetInfo
#define SL_StopMidi		BLIB_StopMidi
#define SL_GetMidiParam		BLIB_GetMidiParam
#define SL_SetMidiParam		BLIB_SetMidiParam
#define SL_PlayMidi		BLIB_PlayMidi
#define SL_IsMidiPlaying	BLIB_IsMidiPlaying
#define SL_LoadSoundFont	BLIB_LoadSoundFont
#define SL_PlaySound		BLIB_PlaySound
#define SL_StopSound		BLIB_StopSound

#endif

#ifdef __cplusplus
XLIBDEF BOOL BLIB_Init( int freq = -1 );
#endif
XLIBDEF BOOL BLIB_Deinit( void );
XLIBDEF BOOL BLIB_PlayMod( const char *txt );
XLIBDEF BOOL BLIB_StopMod( void );
XLIBDEF BOOL BLIB_IsModulePlaying( void );
XLIBDEF void BLIB_PlayWave( int handler );
XLIBDEF void BLIB_FreeAllWave( void );
XLIBDEF BOOL BLIB_IsLoaded( const char *name );
XLIBDEF int BLIB_IsPlaying( int handler );
XLIBDEF void BLIB_StopWave( int handler );
XLIBDEF void BLIB_FreeWave( int handler );
XLIBDEF BOOL BLIB_StopMidi( void );
XLIBDEF BOOL BLIB_PlayMidi( const char *file );
XLIBDEF BOOL BLIB_IsMidiPlaying( void );
XLIBDEF void *BLIB_ChanGetData( void );
#ifdef __cplusplus
XLIBDEF int BLIB_LoadWave( const char *szFileName, int id=-1 );
XLIBDEF int BLIB_LoadStream( const char *szFileName, int id=-1 );
XLIBDEF BOOL BLIB_SetModuleParam( int vol=-1, int nOrder=-1, int nRow=-1, int loop=-1 );
XLIBDEF BOOL BLIB_GetModuleParam( int *vol=NULL, int *ord=NULL, int *row=NULL,
				   int *max_order = NULL, int *percent=NULL,
				   char **song_name = NULL,
				   int *max_chan = NULL );
XLIBDEF BOOL BLIB_GetWaveParam( int handler, int *percent=NULL, int *time=NULL, int *maxTime=NULL, int *mypos=NULL, int *myMaxPos=NULL, char **name=NULL, int *freq=NULL, int *bits=NULL, int *chn=NULL );
XLIBDEF void BLIB_SetWaveParam( int handler, int dwFreq=-1, int dwVol=-1, int dwPan=-1, int fLooped=-1, int percent=-1 );
XLIBDEF char *BLIB_GetStrings( int num, int flag = 0 );
XLIBDEF int BLIB_DrawSpectrum( int specmode=0, int x=-1, int y=-1, int w=-1, int h=-1 );
XLIBDEF int BLIB_GetInfo( int flag=0 );
XLIBDEF BOOL BLIB_GetMidiParam( int *vol=NULL, int *pos=NULL, int *len=NULL, int *percent=NULL, int *tempo=NULL, char **name=NULL, char **str_lyrics=NULL );
XLIBDEF BOOL BLIB_SetMidiParam( int vol=-1, int percent=-1, int tempo=-1, int loop=-1 );
XLIBDEF BOOL BLIB_LoadSoundFont( const char *filename=NULL );
#endif
XLIBDEF int BLIB_PlaySound( const char *filename, int vol, int pan, BOOL loop );
XLIBDEF void BLIB_StopSound( int id );
XLIBDEF void BLIB_GetSilenceLength( const char *file, int threshold, ULONG *start, ULONG *end );
XLIBDEF BOOL BLIB_Mic2Wav( int time, const char *wavname );
XLIBDEF BOOL BLIB_Midi2Wav( const char *midiname, const char *wavname );


#define SL_PLAYWAVE( handle, freq, vol, pan, loop ) {		\
	SL_SetWaveParam( handle, freq, vol, pan, loop );	\
	SL_PlayWave( handle );					\
	SL_SetWaveParam( handle, freq, vol, pan, loop );	}


#define PLAYSOUND( name ) { SL_PlaySound( name, MIDVOL, MIDPAN, FALSE ); }
#define PLAYSOUNDVOL( name, vol ) { SL_PlaySound( name, vol, MIDPAN, FALSE ); }
#define PLAYSOUNDPARAM( name, vol, pan, loop ) { SL_PlaySound( name, vol, pan, loop ); }
#define CACHESOUND( name ) { PLAYSOUNDVOL( name, 0 ); }



// openalshell.cpp

XLIBDEF BOOL OAL_Init( void );
XLIBDEF void OAL_Deinit( void );
XLIBDEF BOOL OAL_IsLoaded( char *name );
#ifdef __cplusplus
XLIBDEF int OAL_LoadWave( char *filename, BOOL force_reload = FALSE );
XLIBDEF int OAL_LoadStream( char *filename, BOOL force_reload = FALSE ); // Ogg
XLIBDEF BOOL OAL_GetWaveParam( int handler, int *percent=NULL, int *time=NULL, int *maxTime=NULL, int *mypos=NULL, int *myMaxPos=NULL, char **name=NULL );
#endif
XLIBDEF void OAL_PlayWave( int handler );
XLIBDEF void OAL_StopWave( int handler );
XLIBDEF void OAL_FreeWave( int handler );
XLIBDEF void OAL_FreeAllWave( void );
XLIBDEF void OAL_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped );
XLIBDEF int OAL_IsPlaying( int handler );


#ifdef OPENAL_LIB

#define SL_NAME "OpenAL"

#define SL_Init			OAL_Init
#define SL_Deinit		OAL_Deinit
#define SL_LoadWave		OAL_LoadWave
#define SL_PlayWave		OAL_PlayWave
#define SL_StopWave		OAL_StopWave
#define SL_FreeWave		OAL_FreeWave
#define SL_FreeAllWave		OAL_FreeAllWave
#define SL_SetWaveParam		OAL_SetWaveParam
#define SL_GetWaveParam		OAL_GetWaveParam
#define SL_IsLoaded		OAL_IsLoaded
#define SL_IsPlaying		OAL_IsPlaying

#endif

#define PLAYSOUNDOAL( name ) { OAL_PlayWave( OAL_LoadWave( name ) ); }
#define PLAYSOUNDVOLOAL( name, vol ) { OAL_SetWaveParam( OAL_LoadWave( name ), -1, (vol), -1, -1 ); OAL_PlayWave( OAL_LoadWave( name ) ); }
#define PLAYSOUNDPARAMOAL( name, vol, pan, loop ) { OAL_SetWaveParam( OAL_LoadWave( name ), -1, (vol), (pan), (loop) ); OAL_PlayWave( OAL_LoadWave( name ) ); }


// oggshell.cpp

XLIBDEF BOOL LoadOgglib( void );
XLIBDEF void UnloadOgglib( void );
XLIBDEF const char *Ogg_ErrorString( int code );

// csak ha kell az Ogg
#ifdef _OGG_H

extern int (*_ov_clear)(OggVorbis_File *vf);
extern vorbis_info *(*_ov_info)(OggVorbis_File *vf,int link);
extern long (*_ov_read)(OggVorbis_File *vf,char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream);
extern ogg_int64_t (*_ov_pcm_total)(OggVorbis_File *vf,int i);
extern int (*_ov_open_callbacks)( void *datasource, OggVorbis_File *vf, const char *initial, long ibytes, ov_callbacks callbacks );
extern vorbis_comment *(*_ov_comment)(OggVorbis_File *vf,int link);
extern int (*_ov_time_seek)(OggVorbis_File *vf,double pos);

#endif



// modplugshell.cpp


XLIBDEF BOOL MPP_Init( void );
XLIBDEF void MPP_Deinit( void );
XLIBDEF BOOL MPP_PlayMod( const char *filename );
XLIBDEF void MPP_StopMod( void );
XLIBDEF BOOL MPP_IsModulePlaying( void );
#ifdef __cplusplus
XLIBDEF BOOL MPP_SetModuleParam( int vol=-1, int nOrder=-1, int nRow=-1, int loop=-1 );
XLIBDEF BOOL MPP_GetModuleParam( int *vol, int *ord, int *row,
				   int *max_order = NULL, int *percent=NULL,
				   char **song_name = NULL,
				   int *max_chan = NULL );
#endif



// sidshell.cpp

XLIBDEF BOOL SID_StopSid( void );
XLIBDEF BOOL SID_CallbackSid( void );
XLIBDEF BOOL SID_PauseSid( void );
XLIBDEF BOOL SID_ResumeSid( void );
XLIBDEF BOOL SID_IsPlaying( void );
#ifdef __cplusplus
XLIBDEF int SID_PlaySid( const char *filename = NULL, int tune = -1 );
XLIBDEF int SID_GetParam( int *arg_cur_song=NULL, int *arg_num_song=NULL,
			  int *vol=NULL, int *pos=NULL, int *max_pos=NULL,
			  char *arg_title=NULL, char *arg_author=NULL, char *arg_copyright=NULL,
			  char *arg_version=NULL );
XLIBDEF BOOL SID_SetParam( int tune = -1, int vol=-1, int pos=-1, int loop = -1 );
#endif


// adplugshell.cpp

XLIBDEF BOOL AD_Stop( void );
XLIBDEF BOOL AD_Callback( void );
XLIBDEF BOOL AD_Pause( void );
XLIBDEF BOOL AD_Resume( void );
XLIBDEF BOOL AD_IsPlaying( void );
#ifdef __cplusplus
XLIBDEF int AD_Play( const char *filename = NULL, int tune = -1 );
XLIBDEF int AD_GetParam( int *arg_cur_song=NULL, int *arg_num_song=NULL,
			  int *vol=NULL, int *pos=NULL, int *max_pos=NULL,
			  char *arg_title=NULL, char *arg_author=NULL, char *arg_copyright=NULL,
			  char *arg_version=NULL );
XLIBDEF BOOL AD_SetParam( int tune = -1, int vol=-1, int pos=-1, int loop = -1 );
#endif


// ymshell.cpp

XLIBDEF BOOL YM_Stop( void );
XLIBDEF BOOL YM_Callback( void );
XLIBDEF BOOL YM_Pause( void );
XLIBDEF BOOL YM_Resume( void );
XLIBDEF BOOL YM_IsPlaying( void );
#ifdef __cplusplus
XLIBDEF int YM_Play( const char *filename = NULL );
XLIBDEF int YM_GetParam( int *vol=NULL, int *pos=NULL, int *max_pos=NULL,
			 char *arg_title=NULL, char *arg_author=NULL, char *arg_desc=NULL,
			 char *arg_version=NULL );
XLIBDEF BOOL YM_SetParam( int vol=-1, int pos=-1, int loop = -1 );
#endif



// putstreamshell.cpp

XLIBDEF BOOL PUT_Pause( void );
XLIBDEF BOOL PUT_Resume( void );
XLIBDEF BOOL PUT_IsPlaying( void );
XLIBDEF void PUT_WriteStream( void *buffer, int length );
XLIBDEF void PUT_Deinit( void );
#ifdef __cplusplus
XLIBDEF int PUT_Init(int freq=-1, int bits=-1);
XLIBDEF int PUT_GetParam( int *vol=NULL, int *pan=NULL);
XLIBDEF BOOL PUT_SetParam( int vol=-1, int pan=-1 );
#endif



// nosefartshell.cpp

XLIBDEF BOOL NSF_Stop( void );
XLIBDEF BOOL NSF_Pause( void );
XLIBDEF BOOL NSF_Resume( void );
XLIBDEF BOOL NSF_IsPlaying( void );
#ifdef __cplusplus
XLIBDEF int NSF_Play( const char *filename = NULL, int tune = -1 );
XLIBDEF int NSF_GetParam( int *arg_cur_song=NULL, int *arg_num_song=NULL,
			  int *vol=NULL, int *pos=NULL, int *max_pos=NULL,
			  char *arg_title=NULL, char *arg_author=NULL, char *arg_copyright=NULL,
			  char *arg_version=NULL );
XLIBDEF BOOL NSF_SetParam( int tune = -1, int vol=-1, int pos=-1, int loop = -1 );
#endif


// NetRadio or Playlist .PLS vagy .M3U

XLIBDEF BOOL NR_Stop( void );
XLIBDEF BOOL NR_Pause( void );
XLIBDEF BOOL NR_Resume( void );
XLIBDEF BOOL NR_IsPlaying( void );

#ifdef __cplusplus
XLIBDEF int NR_Play( const char *filename, int playlist_num = 0, char *arg_proxy = NULL );
XLIBDEF BOOL NR_GetParam( int *vol=NULL, int *pos = NULL, char *title=NULL, char *artist=NULL,
			  char *status=NULL, char *connection=NULL,
			  int *playlist_num=NULL, int *playlist_maxnum=NULL );
XLIBDEF BOOL NR_SetParam( int vol=-1, int pos=-1, int playlist_num=-1 );
#endif



#ifdef AUD_LIB

#define SL_NAME "AUDIERE"

#define SL_Init			AUD_Init
#define SL_Deinit		AUD_Deinit
#define SL_PlayMod		AUD_PlayMod
#define SL_StopMod		AUD_StopMod
#define SL_SetModuleParam	AUD_SetModuleParam
#define SL_GetModuleParam	AUD_GetModuleParam
#define SL_IsModulePlaying	AUD_IsModulePlaying
#define SL_LoadWave		AUD_LoadWave
#define SL_LoadStream		AUD_LoadStream
#define SL_PlayWave		AUD_PlayWave
#define SL_StopWave		AUD_StopWave
#define SL_FreeAllWave		AUD_FreeAllWave
#define SL_SetWaveParam		AUD_SetWaveParam
#define SL_IsLoaded		AUD_IsLoaded
#define SL_IsPlaying		AUD_IsPlaying
#define SL_Pause		AUD_Pause
#define SL_Paused		AUD_Paused

#endif

XLIBDEF BOOL AUD_Init( void );
XLIBDEF BOOL AUD_Deinit( void );
XLIBDEF BOOL AUD_PlayMod( char *txt );
XLIBDEF BOOL AUD_StopMod( void );
XLIBDEF BOOL AUD_SetModuleParam( int vol, int nOrder, int nRow );
XLIBDEF BOOL AUD_GetModuleParam( int *vol, int *ord, int *row );
XLIBDEF BOOL AUD_IsModulePlaying( void );
XLIBDEF int AUD_LoadWave( char *szFileName, int id );
XLIBDEF int AUD_LoadStream( char *szFileName, int id );
XLIBDEF void AUD_PlayWave( int handler );
XLIBDEF void AUD_FreeAllWave( void );
XLIBDEF void AUD_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped );
XLIBDEF BOOL AUD_IsLoaded( char *name );
XLIBDEF int AUD_IsPlaying( int handler );
XLIBDEF void AUD_StopWave( int handler );




#ifdef MIDAS_LIB

#define SL_NAME "MIDAS"

#pragma message("No MIDAS interface.")


#endif

// SDL_mixer

XLIBDEF BOOL MIX_IsModulePlaying( void );
XLIBDEF void MIX_PlayWave( int handler );
XLIBDEF BOOL MIX_IsPlaying( int handler );
#ifdef __cplusplus
XLIBDEF int MIX_LoadWave( const char *szFileName, int id=-1 );
XLIBDEF BOOL MIX_GetWaveParam( int handler, int *percent=NULL, int *time=NULL, int *maxTime=NULL, int *mypos=NULL, int *myMaxPos=NULL, char **name=NULL );
XLIBDEF void MIX_SetWaveParam( int handler, int dwFreq=-1, int dwVol=-1, int dwPan=-1, int fLooped=-1, int percent=-1 );
XLIBDEF BOOL MIX_SetModuleParam( int vol=-1, int nOrder=-1, int nRow=-1, int loop=-1 );
XLIBDEF BOOL MIX_GetModuleParam( int *vol=NULL, int *ord=NULL, int *row=NULL,
				   int *max_order = NULL, int *percent=NULL,
				   char **song_name = NULL,
				   int *max_chan = NULL );
#endif
XLIBDEF void MIX_StopWave( int handler );
XLIBDEF void MIX_FreeWave( int handler );
XLIBDEF void MIX_FreeAllWave( void );
XLIBDEF void MIX_StopMod( void );
XLIBDEF BOOL MIX_PlayMod( char *filename );
XLIBDEF void MIX_Deinit( void );
XLIBDEF BOOL MIX_Init( void );

#define PLAYSOUNDSDL( name ) { MIX_PlayWave( MIX_LoadWave( name ) ); }
#define PLAYSOUNDVOLSDL( name, vol ) { MIX_SetWaveParam( MIX_LoadWave( name ), -1, (vol), -1, -1 ); MIX_PlayWave( MIX_LoadWave( name ) ); }
#define PLAYSOUNDPARAMSDL( name, vol, pan, loop ) { MIX_SetWaveParam( MIX_LoadWave( name ), -1, (vol), (pan), (loop) ); MIX_PlayWave( MIX_LoadWave( name ) ); }


// Aureal3D v3.0

XLIBDEF int AU_LoadWave( char *filename );
XLIBDEF BOOL AU_Init( void );
XLIBDEF void AU_Update( void );
XLIBDEF void AU_Deinit( void );
XLIBDEF void AU_SetListener( point3_t v0, point3_t v1 );
XLIBDEF BOOL AU_SetParam( int handler, point3_t v0, int vol, int loop );



// mp3.cpp

XLIBDEF BOOL DM_Init( void );
XLIBDEF void DM_Deinit( void );
XLIBDEF BOOL DM_Load( char *filename );
XLIBDEF void DM_Pause( void );
XLIBDEF void DM_Play( void );
XLIBDEF void DM_Stop( void );
XLIBDEF void DM_SetVol( int vol );
XLIBDEF void DM_GetVol( int *vol );


// winmm.cpp

#ifdef __cplusplus
XLIBDEF BOOL MM_Open( void *callback = NULL, int totalBufferedSoundLen = 500 );
#endif
XLIBDEF void MM_Close( void );
XLIBDEF void MM_SetCallback( void *callback );



// scentshell.cpp


typedef enum {

	SCID_RASPBERRY,
	SCID_CINNAMON,
	SCID_GRASS,
	SCID_OCEAN,
	SCID_SMOKE,

} scent_e;

XLIBDEF void DeinitScent( void );
XLIBDEF BOOL InitScent( void );
XLIBDEF BOOL EmitScent( scent_e scent );


// mapi.cpp

XLIBDEF BOOL MA_Init( void );
XLIBDEF void MA_Deinit( void );
XLIBDEF void MA_ReadFirst( void );
XLIBDEF BOOL MA_ReadNext( void );
XLIBDEF BOOL MA_ReadMail( char *felado, char *cimzett, char *date, char *subject, char *body, BOOL *unread );
XLIBDEF BOOL MA_SendMail( char *cimzett, char *subject, char *body, char *attach );
XLIBDEF BOOL MA_SendDoc( char *filename );
XLIBDEF BOOL MA_DeleteMail( void );


// network.cpp

#define SERVER_SOCKET_EVENT_HAS_HAPPENED       25252
#define CLIENT_SOCKET_EVENT_HAS_HAPPENED       25253

XLIBDEF BOOL InitNetwork( void );
XLIBDEF void DeinitNetwork( void );

XLIBDEF BOOL SetupServer( void );
XLIBDEF void ShutdownServer( void );

XLIBDEF BOOL SetupClient( char *str );
XLIBDEF void ShutdownClient( void );

XLIBDEF void NetworkServerCallback( int wparam, int lparam );
XLIBDEF void NetworkClientCallback( int wparam, int lparam );

// overlay_renderer.cpp

#ifdef __cplusplus
XLIBDEF void SetOverlayParam( int alpha=0, int red=-1, int green=-1, int blue=-1, int x=-1, int y=-1, int w=-1, int h=-1 );
#endif

XLIBDEF BOOL Overlay( const char *s, ... );


// color.cpp

#ifdef __cplusplus
XLIBDEF rgb_t *GetColorRGB( char *name=NULL, int pos=-1 );
#endif

// 147 darab szín
XLIBDEF char *GetColorName( int pos );


// mingw miatt

#ifndef cprintf
#define cprintf _cprintf
#endif

#ifdef __cplusplus
}
#endif

#endif
