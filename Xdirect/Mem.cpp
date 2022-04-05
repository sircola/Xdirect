/* Copyright (C) 1997 Kirschner, Bernát. All Rights Reserved Worldwide. */


#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)


#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include <xlib.h>

RCSID( "$Id: mem.c,v 1.0 97-03-10 16:59:24 bernie Exp $" )


static BOOL mem_inited = FALSE;

union memctrl_u {
	UCHAR flags;
	struct {
		UCHAR check: 1;			// BoundsChecker féleség
		UCHAR compress: 1;		// garbage collector
	};
} memctrl = { 1 | (1<<1) };



ULONG	mem_used = 0L,			// mennyit hasznalt
	mem_all  = 0L;			// mennyi volt szabadon kezdeskor

#define MEM_ID	(0xdeadbeef)


#define MEMBLOCK_MAXSTRLEN (256)

typedef struct memblock_s {
	ULONG id;
	memptr_ptr ptr;
	memptr_t alloced;			// erre lesz a malloc
	__int64 base;				// mem_pool + base = ptr
	int size;
	UCHAR used;
	BOOL checked;				// bound checker
	// UCHAR locked;			// DPMI lock
	char file[MEMBLOCK_MAXSTRLEN+1];	// fajlban
	SHORT line;				// sorban
	char name[MEMBLOCK_MAXSTRLEN+1];	// a buffer neve
	char func[MEMBLOCK_MAXSTRLEN+1];	// a funkcio neve
} memblock_t,*memblock_ptr;

#define BLOCKALLOC	1000

static memblock_ptr memblock = NULL;
static int nmemblocks = 0;			// aktualisan mennyi az a ALLOCMEM-mel
static int allocedmemblocks = 0;		// allokalva


#define MAKEMEMBLOCK(i,p,b,s) { 		\
	memblock[(i)].id      = MEM_ID; 	\
	memblock[(i)].ptr     = (p);		\
	memblock[(i)].base    = (__int64)(b);	\
	memblock[(i)].size    = (s);		\
	memblock[(i)].used    = TRUE;		\
	memblock[(i)].checked = memctrl.check;	\
	memblock[(i)].file[0] = 0;		\
	memblock[(i)].line    = 0;		\
	memblock[(i)].name[0] = 0;		\
	memblock[(i)].func[0] = 0;		\
}


//
// memset( &memblock[(i)], 0L, sizeof(memblock_s) );	\
//
#define ERASEMEMBLOCK(i) {	 		\
	memblock[(i)].used = FALSE;		\
	memblock[(i)].checked = FALSE;		\
	memblock[(i)].id = ~MEM_ID; 		\
}


static memptr_t dummy = (memptr_t)0xDEADBEEF;

//		     123456789abc
#define MEM_SIG_STR "U MAD BRO?"
#define MEM_SIG_SIZE (10+1)



//
//
//
char *memorySizeString( unsigned long size ) {

	static char str[90];

	     if( size > (1024*1024))	sprintf(str, "%10s (%7.2fM)", insertCommas(size), (FLOAT)(size) / (1024.0f * 1024.0f));
	else if( size > 1024)		sprintf(str, "%10s (%7.2fK)", insertCommas(size), (FLOAT)(size) / 1024.0f);
	else				sprintf(str, "%10s bytes     ", insertCommas(size));

	return str;
}


#if 0

//
// We use DWORDS as our padding, and a long is guaranteed to be 4 bytes, but an int is not (ANSI defines an int as
// being the standard word size for a processor; on a 32-bit machine, that's 4 bytes, but on a 64-bit machine, it's
// 8 bytes, which means an int can actually be larger than a long.)
//
static int calculateActualSize( int reportedSize ) {

	return reportedSize + paddingSize * sizeof(long) * 2;
}



//
// We use DWORDS as our padding, and a long is guaranteed to be 4 bytes, but an int is not (ANSI defines an int as
// being the standard word size for a processor; on a 32-bit machine, that's 4 bytes, but on a 64-bit machine, it's
// 8 bytes, which means an int can actually be larger than a long.)
//
static int calculateReportedSize( int actualSize ) {

	return actualSize - paddingSize * sizeof(long) * 2;
}


//
//
//
static void *calculateReportedAddress( void *actualAddress ) {

	// We allow this...

	if( !actualAddress ) return NULL;

	// Just account for the padding

	return (char *)(actualAddress) + sizeof(long) * paddingSize;
}



//
// For a serious test run, we use wipes of random a random value. However, if this causes a crash, we don't want it to
// crash in a differnt place each time, so we specifically DO NOT call srand. If, by chance your program calls srand(),
// you may wish to disable that when running with a random wipe test. This will make any crashes more consistent so they
// can be tracked down easier.
//
static void wipeWithPattern( sAllocUnit *allocUnit, unsigned long pattern, const unsigned int originalReportedSize = 0 ) {

	BOOL randomWipe = TRUE;

	if( randomWipe )
		pattern = ((rand() & 0xff) << 24) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 8) | (rand() & 0xff);

	// -DOC- We should wipe with 0's if we're not in debug mode, so we can help hide bugs if possible when we release the
	// product. So uncomment the following line for releases.
	//
	// Note that the "alwaysWipeAll" should be turned on for this to have effect, otherwise it won't do much good. But we'll
	// leave it this way (as an option) because this does slow things down.

//	pattern = 0;

	// This part of the operation is optional

	if (alwaysWipeAll && allocUnit->reportedSize > originalReportedSize)
	{
		// Fill the bulk

		long	*lptr = reinterpret_cast<long *>(reinterpret_cast<char *>(allocUnit->reportedAddress) + originalReportedSize);
		int	length = static_cast<int>(allocUnit->reportedSize - originalReportedSize);
		int	i;
		for (i = 0; i < (length >> 2); i++, lptr++)
		{
			*lptr = pattern;
		}

		// Fill the remainder

		unsigned int	shiftCount = 0;
		char		*cptr = reinterpret_cast<char *>(lptr);
		for (i = 0; i < (length & 0x3); i++, cptr++, shiftCount += 8)
		{
			*cptr = static_cast<char>((pattern & (0xff << shiftCount)) >> shiftCount);
		}
	}

	// Write in the prefix/postfix bytes

	long *pre = (long *)(allocUnit->actualAddress);
	long *post = (long *)((char *((allocUnit->actualAddress) + allocUnit->actualSize - paddingSize * sizeof(long));

	for( unsigned int i = 0; i < paddingSize; i++, pre++, post++ ) {
		*pre = prefixPattern;
		*post = postfixPattern;
	}

	return;
}

#endif




//
//
//
void MemCtrl( BOOL check, BOOL compress ) {

	memctrl.check = BOOLEAN( check );
	memctrl.compress = BOOLEAN( compress );

	return;
}



//
//
//
static char *commaprint( ULONG n ) {

	static int comma = '\0';
	static char retbuf[30];
	char *p = &retbuf[sizeof(retbuf)-1];
	int i = 0;

	if(comma == '\0') {

		struct lconv *lcp = localeconv();

		if(lcp != NULL) {
			if(lcp->thousands_sep != NULL &&
				*lcp->thousands_sep != '\0')
				comma = *lcp->thousands_sep;
			else	comma = ',';
		}
	}

	*p = '\0';

	do {
		if(i%3 == 0 && i != 0)
			*--p = comma;
		*--p = '0' + n % 10;
		n /= 10;
		i++;
	}
	while( n != 0 );

	return p;
}



//
//
//
void MemStats( void ) {

	int i,n=0;
	__int64 start = -1;

	xprintf("memory dump:\n");

	xprintf("%-8s %-8s %-8s %-8s %-8s %s\n","attr:","num:","start:","end:","size:","name:");
	xprintf("----------------------------------------------------------------------------\n");

	for( i=0; i<allocedmemblocks; i++ ) {

		if( start == -1 )
			start = memblock[i].base;

		if( memblock[i].used == TRUE ) {

			++n;

			if( memblock[i].base < start )
				start = memblock[i].base;

			xprintf("%-8d %-8d %-8d %-8d %s(%d): %s\n",
					n,
					(int)memblock[i].base,
					(int)memblock[i].base + memblock[i].size,
					memblock[i].size,
					memblock[i].file,
					memblock[i].line,
					memblock[i].name );
		}

	}

	xprintf("----------------------------------------------------------------------------\n");
	xprintf("all: %s  used: %s  start: %d\n",commaprint(mem_all),commaprint(mem_used),start);
	xprintf("nmemblocks: %d, counted: %d, alloced: %d\n\n",nmemblocks,n,allocedmemblocks);

	return;
}




//
//
//
int mem_cmp( const void *a, const void *b ) {
	return ( (int)(((memblock_ptr)b)->base) - (int)(((memblock_ptr)a)->base) );
}



//
//
//
void MemLeak( void ) {

	int width,height;
	int ux,uy,lx,ly;
	int *tomb,*ptr,i,y1,y2,even;

	rgb_t feher = { 63,63,63, 255 };
	rgb_t fekete = { 0,0,0, 255 };
	rgb_t szurke = { 30,30,30, 255 };

	ux = 20;
	uy = SCREENH/2 - 35;
	lx = SCREENW - 20;
	ly = uy + 70;

	width = lx - ux;
	height = ly - uy;

	FilledBox( 0,0, CLIPMAXX, CLIPMAXY, feher );
	Box( ux,uy, lx,ly, fekete );

	//FilledBox( ux+20,uy, ux+60,ly, szurke );

	ALLOCMEM( tomb, nmemblocks );
	ptr = tomb;

	for( i=0; i<allocedmemblocks; i++ )
		if( memblock[i].used == TRUE )
			*ptr++ = i;

	qsort( tomb, nmemblocks, sizeof(int), mem_cmp );

	even = 1;
	for( i=0; i<nmemblocks; i++ ) {
		if( even ) {
			y1 = uy - 10;
			y2 = ly - 10;
			even = 0;
		}
		else {
			y1 = uy + 10;
			y2 = ly + 10;
			even = 1;
		}
		FilledBox( ux+ ((int)memblock[i].base*width)/mem_all, y1,
			   ux+ ((int)(memblock[i].base+memblock[i].size)*width)/mem_all, y2,
			   szurke );
		Box( ux+ ((int)memblock[i].base*width)/mem_all, y1,
		     ux+ (((int)memblock[i].base+memblock[i].size)*width)/mem_all, y2,
		     fekete );
	}

	FREEMEM( tomb );

	return;
}





//
//
//
static int SearchMemBase( memptr_t ptr ) {

	int i;
	ULONG base;
	static BOOL sema = FALSE;

	if( sema == TRUE ) return -1;
	sema = TRUE;

	base = (ULONG)ptr;

	for( i=0; i<allocedmemblocks; i++ )

		if( (memblock[i].base == base) && (memblock[i].used == TRUE) ) {
			sema = FALSE;
			return i;
		}

	// xprintf( "SearchMemBase: can't find 0x%p pointer.\n", (int)ptr );

	sema = FALSE;

	return -1;
}




//
//
//
BOOL NameMem( memptr_t ptr, const char *file, int line, const char *name ) {

	int i;
	char *p;

	if( (i = SearchMemBase(ptr)) != -1 ) {

		if( line != -1 )
			memblock[i].line = line;

		if( file != NULL ) {

			if( (p = strrchr(file,'\\')) == NULL )
				p = (char *)file;
			else
				++p;

			// memcpy( memblock[i].file, p, MIN(FILEMAX,strlen(file)+1) );
			// memcpy( memblock[i].name, name, MIN(NAMEMAX,strlen(name)+1) );

			strncpy( memblock[i].file, p, MEMBLOCK_MAXSTRLEN );
		}

		if( name != NULL )
			strncpy( memblock[i].name, name, MEMBLOCK_MAXSTRLEN );

		// memcpy( memblock[i].func, __FUNCTION__, MIN(FUNCMAX,strlen(__FUNCTION__)+1) );

		memblock[i].file[MEMBLOCK_MAXSTRLEN] = 0;
		memblock[i].name[MEMBLOCK_MAXSTRLEN] = 0;
		memblock[i].func[MEMBLOCK_MAXSTRLEN] = 0;
	}
	else
		xprintf("NameMem: can't find it WTF?!\nmem: %s(%d): %s",file, line, name);


	return TRUE;
}




//
//
//
ULONG GetMemInfo( void ) {

	return (mem_all - mem_used);
}



//
//
//
static BOOL AllocMemBlock( int num ) {

	int i;

	if( !num )
		return TRUE;

	if( (memblock = (memblock_t*)realloc( memblock,
					      (allocedmemblocks + num) * sizeof(memblock_s) )
							) == NULL ) {
		xprintf("AllocMemBlock: can't realloc %d more memblocks (%d).",num, allocedmemblocks);
		return FALSE;
	}

	for( i=allocedmemblocks; i<allocedmemblocks+num; i++ ) {
		ERASEMEMBLOCK( i );
	}

	allocedmemblocks += num;

	return TRUE;
}




//
// -1: ha nincs elég memória
//
static int GetFreeMemBlock( void ) {

	int i;

	for( i=0; i<allocedmemblocks; i++ )
		if( memblock[i].used == FALSE )
			break;

	if( i >= allocedmemblocks ) {
		i = allocedmemblocks;
		if( AllocMemBlock( BLOCKALLOC ) == FALSE )
			return -1;
	}

	return i;
}



#define INITAMOUNT 8000000L		// ha nem hivtam InitMemory-t
					// akkor ennyi lesz


//
//
//
void InitMemory( void ) {

	winmeminfo_t winmeminfo;
	int num;
	FLOAT flt;
	int required = 0;

	if( mem_inited ) return;

	mem_inited = TRUE;

	XLIB_GetWinMemInfo( &winmeminfo );

	mem_all = winmeminfo.dwTotalPhys;

	// xprintf("free memory: %u Mbytes\n", (ULONG)(winmeminfo.dwAvailPhys/(1024*1024)) /*(FLOAT)winmeminfo.dwAvailPhys/(1024.0*1024.0)*/ );
	// xprintf("total memory: %u Mbytes\n", (ULONG)(winmeminfo.dwTotalPhys/(1024*1024)) /*(FLOAT)mem_all/(1024.0*1024.0)*/ );

	xprintf("log: %u of %u MB free memory.\n", (ULONG)(winmeminfo.dwAvailPhys/(1024*1024)), (ULONG)(mem_all/(1024*1024)) );

	allocedmemblocks = 0L;
	AllocMemBlock( BLOCKALLOC );

	if( (num=CheckParm("mem")) ) {
		sscanf(myargv[num+1],"%f",&flt);
		required = (int)(flt * 1000.0 * 1000.0);
	}

	if( required == 0 )
		required = INITAMOUNT;

	mem_used   = 0L;
	nmemblocks = 0L;

	xprintf("InitMemory: pooling %u MB, bound checking is %s, garbage collection is %s.\n",(ULONG)(mem_all/(1024*1024)), memctrl.check ? "ON" : "OFF", memctrl.compress ? "ON" : "OFF" );

	return;
}





//
//
//
void DeinitMemory(void) {

	int i;

	for( i=0; i<allocedmemblocks; i++ ) {
		ERASEMEMBLOCK( i );
	}

	mem_all  = 0L;
	mem_used = 0L;

	if( memblock ) free( memblock );
	memblock = NULL;

	allocedmemblocks = 0;
	nmemblocks = 0;

	mem_inited = FALSE;

	return;
}









//
//
//
static void CompressMemBlock( void ) {

	if( memctrl.compress == FALSE )
		return;

	for( int i=0; i<allocedmemblocks; i++ )
		if( memblock[i].used == FALSE )
			// find next used
			for( int j=i+1; j<allocedmemblocks; j++ )
				if( memblock[j].used == TRUE ) {
					memcpy( &memblock[i], &memblock[j], sizeof(memblock_t) );
					ERASEMEMBLOCK(j);
					i = j;
					break;
				}

	return;
}



//
//
//
int memblock_cmp( const void *a, const void *b ) {

	return (int)( (int)(((memblock_ptr)a)->base) - (int)(((memblock_ptr)b)->base) );
}





//
//
//
static void SortMemBlock( void ) {

#if 0

	int j,k;
	memblock_t temp;

	for( j=0; j<nmemblocks; j++ ) {

		for( k=j+1; k<nmemblocks; k++ ) {

			if( memblock[j].base > memblock[k].base ) {

				memcpy( &temp,	      &memblock[j], sizeof(memblock_t) );
				memcpy( &memblock[j], &memblock[k], sizeof(memblock_t) );
				memcpy( &memblock[k], &temp,	    sizeof(memblock_t) );

			}
		}
	}
#else

	qsort( memblock, nmemblocks, sizeof(memblock_t), memblock_cmp );

#endif
	//xprintf("after Sort: %d\n",nmemblocks);
	//j = 0;
	//while( j<nmemblocks ) xprintf("%d\n",(int)memblock[j++].base);

	return;
}





//
//
//
BOOL ChangeMemBlockPtr( memptr_ptr old, memptr_ptr uj ) {

	int i;

	// *uj = *old;
	// *old = NULL;
	// return TRUE;

	if( (i = SearchMemBase( *old )) == -1 ) {

		// xprintf("old: %x, uj: %x\n",*old,*uj);
		// MemStats();

		Quit("ChangeMemBlockPtr: can't find memblock.\n");
	}

	memblock[i].ptr = uj;
	*uj = (memptr_t)(memblock[i].base);

	return TRUE;
}









//
//
//
static void makeMemSig( int num, int size ) {

	memblock[ num ].checked = memctrl.check;

	if( memctrl.check == FALSE )
		return;

	static int sema = 0;
	static int cnt = 0;

	++cnt;

	// újra meghívás ellen
	if( sema ) { memblock[ num ].checked = FALSE; return; }
	++sema;

	UCHAR *buf = (UCHAR *)memblock[ num ].alloced;

	strcpy( (char *)&buf[0], MEM_SIG_STR );
	strcpy( (char *)&buf[size+MEM_SIG_SIZE], MEM_SIG_STR );

	// xprintf( "makeMemSig(%d): written.\n", cnt );

	sema = 0;
	--cnt;

	return;
}




//
//
//
static BOOL checkMemSig( int i ) {

	if( memctrl.check == FALSE )
		return TRUE;

	static int sema = 0;
	static int cnt = 0;

	++cnt;

	// újra meghívás ellen
	if( sema ) return FALSE;
	++sema;

	if( i<0 || i>=allocedmemblocks || memblock[i].used == FALSE || memblock[i].checked == FALSE ) {
		sema = 0;
		--cnt;
		return TRUE;
	}

	BOOL overflow = FALSE;
	BOOL underflow = FALSE;

	char *buf = (char *)&memblock[i].alloced[ MEM_SIG_SIZE + memblock[i].size ];

	// túlcsordulás ellenõrzése
	if( strcmp( buf, MEM_SIG_STR ) ) {

		// megkeresi mennyi van átírva és honnantól

		char *str = (char *)MEM_SIG_STR;
		int corr = 0,start=-1;

		for( int j=0; j<MEM_SIG_SIZE; j++ )
			if( buf[ j ] != str[j] ) {
				++corr;
				if( start == -1 )
					start = j;
			}

		xprintf("checkMemSig: OVERFLOW at memblock[%d].size = %d, \"%s\"(%d): \"%s\"\ncorruption: %d byte(s) out of %d, with start at %d\n",
				i,
				memblock[i].size,
				memblock[i].file,
				memblock[i].line,
				memblock[i].name,
				corr, MEM_SIG_SIZE, start );

		overflow = TRUE;
	}


	buf = (char *)&memblock[i].alloced[ 0 ];

	// alulcsordulás ellenõrzése
	if( strcmp( buf, MEM_SIG_STR ) ) {

		// megkeresi mennyi van átírva és honnantól

		char *str = (char *)MEM_SIG_STR;
		int corr = 0,start=-1;

		for( int j=0; j<MEM_SIG_SIZE; j++ )
			if( buf[ memblock[i].size + j ] != str[j] ) {
				++corr;
				if( start == -1 )
					start = j;
			}

		xprintf("checkMemSig: UNDERFLOW at memblock[%d].size = %d, \"%s\"(%d): \"%s\"\ncorruption: %d byte(s) out of %d, with start at %d\n",
				i,
				memblock[i].size,
				memblock[i].file,
				memblock[i].line,
				memblock[i].name,
				corr, MEM_SIG_SIZE, start );

		underflow = TRUE;
	}

	if( overflow || underflow )
		Quit("checkMemSig: Heap %s detected!\n\n(check log)",overflow?"overflow":"underflow");

	sema = 0;
	--cnt;

	return TRUE;
}



//
//
//
void MemCheck( void ) {

	int i,n=0;

	xprintf("MemCheck: checking %d blocks:\n", allocedmemblocks );

	for( i=0; i<allocedmemblocks; i++ ) {

		if( memblock[i].used == TRUE ) {
			++n;
			checkMemSig( i );

			xprintf("%d: at [%d] \"%s\" is ok.\n", n, i, memblock[i].name );
		}
	}

	xprintf("MemCheck: checked %d out of %d.\n", n, allocedmemblocks );

	return;
}






static int which;	// hackerance de kell az AllocMemNoPtr-hez


//
//
//
BOOL AllocMem( memptr_ptr ptr, int size ) {

	InitMemory();

	if( size <= 0 ) {
		xprintf("AllocMem: can't alloc negative size = %d.\n",size);
		return FALSE;
	}

	if( (which = GetFreeMemBlock()) == -1 )
		return FALSE;

	int plus_size = memctrl.check ? MEM_SIG_SIZE*2 : 0;

#define ALIGMENT_REQUEST 4	//32bits alignment requested
#define ALIGMENT_REQUEST_MINUS_ONE (ALIGMENT_REQUEST-1)

	// void* memAddress = (((char*)unAligned)) + ALIGMENT_REQUEST_MINUS_ONE) & ~ALIGMENT_REQUEST_MINUS_ONE ;

	// Page alignment (on 8KB):
	// size = (size + 8191) & ~8191;

	if( (memblock[ which ].alloced = (memptr_t)malloc( (size+plus_size))) == NULL )
		return FALSE;

	++nmemblocks;

	*ptr = (memptr_t)&memblock[ which ].alloced[ (memctrl.check ? MEM_SIG_SIZE : 0) ];

	makeMemSig( which, size );

	MAKEMEMBLOCK( which, ptr, *ptr, size );

	mem_used += size;

	return TRUE;
}




//
//
//
BOOL FreeMem( memptr_ptr ptr ) {

	int i;

	if( (i = SearchMemBase( *ptr )) == -1 )
		return FALSE;

	// xprintf("\n--> freemem: %s\n",memblock[i].name);

	if( memblock[i].id != MEM_ID )
		xprintf("FreeMem: memblock[%d].name = \"%s\" is corrupted.\n",i,memblock[i].name);

	mem_used -= memblock[i].size;

	checkMemSig( i );

	free( memblock[i].alloced );
	*ptr = NULL;

	ERASEMEMBLOCK( i );

	--nmemblocks;

	CompressMemBlock();

	return TRUE;
}





//
//
//
BOOL EraseMem( memptr_t ptr ) {

	int i,j;

	if( (i = SearchMemBase( ptr )) == -1 )
		return FALSE;

	if( memblock[i].id != MEM_ID )
		xprintf("EraseMem: memblock[%d].name = \"%s\" is corrupted.\n",i,memblock[i].name);

	for( j=0; j<memblock[i].size; j++ )
		ptr[j] = j+13;

	return TRUE;
}








//
//
//
BOOL ReallocMem( memptr_ptr ptr, int ujsize ) {

	int oldsize;
	int i;

	if( (i = SearchMemBase( *ptr )) == -1 ) {
		if( ujsize == 0L )
			return TRUE;
		return AllocMem( ptr, ujsize );
	}

	int plus_size = memctrl.check ? MEM_SIG_SIZE*2 : 0;

	if( (memblock[ i ].alloced=(memptr_t)realloc(memblock[ i ].alloced,ujsize+plus_size))==NULL )
		return FALSE;

	*ptr = (memptr_t)&memblock[ i ].alloced[ (memctrl.check ? MEM_SIG_SIZE : 0) ];

	makeMemSig( i, ujsize );

	oldsize = memblock[i].size;
	memblock[i].size = ujsize;
	memblock[i].base = (__int64)*ptr;
	mem_used += (ujsize - oldsize);

	return TRUE;
}





//
//
//
ULONG BufSize( memptr_ptr ptr ) {

	int i;

	if( (i = SearchMemBase( *ptr )) == -1 )
		return 0L;

	return memblock[i].size;
}





/************************************************
 *						*
 * NoPtr versions				*
 *						*
 ************************************************/


//
//
//
void *AllocMemNoPtr( int size ) {

	InitMemory();

	ALLOCMEM( dummy, size );

	//LOG("AllocMemNoPtr: %ld",(ULONG)dummy);

	memblock[which].base = (__int64)dummy;

	return dummy;
}





//
//
//
BOOL FreeMemNoPtr( void *ptr ) {

	int i;

	//LOG("FreeMemNoPtr: %ld",(ULONG)ptr);
	if( (i = SearchMemBase( (memptr_t)ptr )) == -1 )
		return FALSE;
	//WriteLog("end");

// xprintf("\n--> freememnoptr: %s\n",memblock[i].name);

	if( memblock[i].id != MEM_ID )
		xprintf("FreeMemNoPtr: memblock[%d].name = \"%s\" is corrupted.\n",i,memblock[i].name);

	mem_used -= memblock[i].size;

	checkMemSig( i );

	free( memblock[i].alloced );
	ptr = NULL;

	ERASEMEMBLOCK( i );

	--nmemblocks;

	CompressMemBlock();

	return TRUE;
}







//
//
//
void *ReallocMemNoPtr( memptr_t ptr, int ujsize ) {

	int oldsize;
	int i;

	if( (i = SearchMemBase( ptr )) == -1 ) {
		if( ujsize == 0L )
			return NULL;
		return (memptr_t)AllocMemNoPtr( ujsize );
	}

	int plus_size = memctrl.check ? MEM_SIG_SIZE*2 : 0;

	if( (memblock[ i ].alloced=(memptr_t)realloc(memblock[ i ].alloced,ujsize+plus_size))==NULL )
		return NULL;

	ptr = (memptr_t)&memblock[ i ].alloced[ (memctrl.check ? MEM_SIG_SIZE : 0) ];

	makeMemSig( i, ujsize );

	oldsize = memblock[i].size;
	memblock[i].size = ujsize;
	memblock[i].base = (__int64)ptr;
	mem_used += (ujsize - oldsize);

	return ptr;
}






//
//
//
ULONG BufSizeNoPtr( void *ptr ) {

	int i;

	//WriteLog("BUFSIZE: %ld",(ULONG)ptr);
	if( (i = SearchMemBase( (memptr_t)ptr )) == -1) return 0L;

	return memblock[i].size;
}


