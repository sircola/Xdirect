/* Copyright (C) 1997-99 Kirschner, Bernát. All Rights Reserved Worldwide. */
/* mailto: bernie@freemail.hu											   */
/* tel: +36 20 333 9517													   */

#ifndef _XTYPE_H_INCLUDED
#define _XTYPE_H_INCLUDED

#define CHAR	char
#define UCHAR	unsigned char
#define SCHAR	signed char
#define SHORT	short
#define USHORT	unsigned short
#define SSHORT	signed short
#define INT	int
#define UINT	unsigned int
#define SINT	signed int
#define LONG	long
#define ULONG	unsigned long
#define SLONG	signed long
#define FLOAT	float
#define FIXED	long
#define STR	char*

#define BOOL	int
#define TRUE	1
#define FALSE	0

#define BYTE	UCHAR
#define WORD	USHORT
#define DWORD	ULONG


typedef UCHAR *memptr_t,**memptr_ptr;


#define CVAR_MAXNAMELEN		( 64 )
#define CVAR_MAXSTRLEN		( 256 )

#ifndef XMAX_PATH
#define XMAX_PATH  512
#endif

//
//
//
typedef struct cvar_s {

	char	name[CVAR_MAXNAMELEN];
	char	string[CVAR_MAXSTRLEN];
	// BOOL	archive;	// set to true to cause it to be saved to vars.rc
	// BOOL	server;		// notifies players when changed

	union {

		FLOAT	value;
		double	dvalue;
		int		ivalue;
		char	cvalue;

		char	buf[8];		// a double miatt
	};

	void (*func)(void);		// ha történik valami akkor meghívja

	struct cvar_s *prev,*next;

} cvar_t, *cvar_ptr;

#pragma pack(push, 1)
//
//
//
typedef struct rgb_s {

	int r,g,b,a;

} rgb_t, *rgb_ptr;

#pragma pack(pop)


typedef struct findfile_s {

	char name[XMAX_PATH];
	int attrib;
	int size;

	int year,month,day;
	int hour,min,sec,msec;

} findfile_t,*findfile_ptr;



#define MATROW 3
#define MATCOL 4

typedef int point_t[2];
typedef point_t *point_ptr,point2_t,*point2_ptr;


typedef FLOAT point3_t[MATROW];
typedef point3_t *point3_ptr;

typedef int point3i_t[MATROW];
typedef point3i_t *point3i_ptr;

typedef FLOAT point4_t[4];
typedef point4_t *point4_ptr;


typedef FLOAT matrix_t[MATROW][MATCOL];
typedef matrix_t *matrix_ptr;

typedef FLOAT quat_t[MATCOL];

typedef FLOAT matrix4_t[16];		// 4x4
typedef matrix4_t *matrix4_ptr;


#define TOHWND(p)	( (HWND)(p) )
#define TOINT64(p) 	( (__int64)(p) )
#define TOVOIDP(p)	( (void *)(p) )


#define PTR(p) ( (memptr_ptr)&(p) )
#define MEMCAST(type,val) (*((type *)&(val)))


// C++ only

#if 1

#ifdef __cplusplus



//-----------------------------------------------------------------------------
//
// GarageGames.com
//
// Usage Examples:
//
// FILE *handle = fopen( TTempStr<256>("%s:\\%s\\%s.%s", drive, path, file, extension). "r" );
//
// TTempStr<48> text;
// strcpy(text, "this is a ");
// strcat(text, "test");
//
// TTempStr<24> buffer("hello %s", "world");
// puts(buffer);
// buffer.set("who is number %d?", 2);
// puts(buffer);
//
// puts( buffer.set("that's %s folks!", "all") );
//
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <assert.h>

template< int Size > struct string_t {

private:

	char mBuffer[Size];

public:

	string_t() {
		mBuffer[0] = '\0';
	}

	string_t( const char *format, ... ) {

		// Assert that the format string is not the internal buffer
		assert(format != mBuffer);

		va_list argList;
		va_start(argList, format);
		int length = vsprintf(mBuffer, format, argList);

		// Assert that there was no buffer overflow
		// assert(length<Size);

		va_end(argList);
	}


	inline char *set( const char *format, ... ) {

		// Assert that the format string is not the internal buffer

		assert(format != mBuffer);

		va_list argList;
		va_start(argList, format);
		int length = vsprintf(mBuffer, format, argList);

		// Assert that there was no buffer overflow
      		// assert(length<Size);

		va_end(argList);

		return mBuffer;
	}

	inline operator char*() {
		return mBuffer;
	}

};

// #ifdef _MSC_VER

//
// printf("próba szöveg. %d",  binary<1001>::value );
//
template< long int N > class binary {

public:

	enum {
		bit = N % 10,
		value = bit + (binary<N/10>::value << 1)
	};
};

/*
class binary<0> {

public:

	enum {
		bit = 0,
        value = 0
	};
};
*/
// #endif

#endif

#endif


#endif
