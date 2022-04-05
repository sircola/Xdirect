
#include <stdio.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: bmp.cpp,v 1.1.1.1 2003/08/19 17:44:45 bernie Exp $" )

// head,pal,bitmap
// PAL: b-g-r-unused ( >>2 )
// BITMAP: fejjel-lefele
// ez egy változás
// második változás

#pragma pack(push, 1)

typedef struct bmpheader_s {

	USHORT	type;		// 42 4d BM
	ULONG	filesize;	// 36 fe
	USHORT	unused1;
	USHORT	unused2;
	ULONG	offsetbits;	// 0c

	ULONG	infosize;
	USHORT	width;		// c8 00
	USHORT	w1;
	USHORT	height;
	USHORT	h1;
	USHORT	planes; 	// 1
	UCHAR	bitcount;	// 8
	ULONG	compression;	// 0
	ULONG	imagesize;	// 00 00 fa 00
	ULONG	xpels;
	ULONG	ypels;
	ULONG	colorsused;
	ULONG	colorsimportant;
	UCHAR	u1;

} bmpheader_t, *bmpheader_ptr;

#pragma pack(pop)



//
//
//
void GetRgbBmp( UCHAR *pal ) {

	int i;

	for( i=0; i<768; i++ )
		pal[i] = pic_pal[i];

	return;
}



//
//
//
void SetRgbBmp( void ) {

	UCHAR *pal;

	ALLOCMEM( pal, 768 );

	GetRgbBmp(pal); 			// 6 bit miatt
	SetRgbBuf(pal);

	FREEMEM( pal );

	return;
}




//
//
//
static void dumpheader( bmpheader_t head ) {

	xprintf("type: %c%c\n",head.type&0xff,head.type>>8);
	xprintf("filesize: %d\n",head.filesize);
	xprintf("unused1: %d\n",head.unused1);
	xprintf("unused2: %d\n",head.unused2);
	xprintf("offsetbits: %d\n",head.offsetbits);

	xprintf("infosize: %d\n",head.infosize);
	xprintf("width: %d\n",head.width);
	xprintf("w1: %d\n",head.w1);
	xprintf("height: %d\n",head.height);
	xprintf("h1: %d\n",head.h1);
	xprintf("planes: %d\n",head.planes);
	xprintf("bitcount: %d\n",head.bitcount);
	xprintf("compression: %d\n",head.compression);
	xprintf("imagesize: %d\n",head.imagesize);
	xprintf("xpels: %d\n",head.xpels);
	xprintf("ypels: %d\n",head.ypels);
	xprintf("colorsused: %d\n",head.colorsused);
	xprintf("colorsimportant: %d\n",head.colorsimportant);
	xprintf("u1: %d\n",head.u1);

	return;
}




//
//
//
BOOL LoadBmp( char *name, memptr_ptr ptr, int *w, int *h, int *bpp ) {

	bmpheader_t bmp;
	int i,x,y,wb;
	FILE *f;
	UCHAR *spr,buf[4];

	if( (f = ffopen(name,"rb")) == NULL )
		Quit( "LoadBmp(): can't open \"%s\" file.",name);

	ffread( &bmp, sizeof(bmpheader_t), 1, f );

	// dumpheader( bmp );

	if( bmp.type != 0x4d42 )
		Quit( "LoadBmp(\"%s\"): not a BMP file.",name);

	if( bmp.planes!=1 || bmp.compression!=0 )
		Quit( "LoadBmp(\"%s\"): only non-compressed are supported.",name);


	if( bmp.bitcount == 8 ) {

		ALLOCMEM( *ptr, bmp.width*bmp.height+SPRITEHEADER );

		MKSPRW( *ptr, bmp.width);
		MKSPRH( *ptr, bmp.height);
		MKSPR8( *ptr );

		wb = (bmp.width + 3) & ~3;

		for( i=0; i<256; i++ ) {
			pic_pal[(i*3)+2] = ffgetc(f) >> 2;
			pic_pal[(i*3)+1] = ffgetc(f) >> 2;
			pic_pal[(i*3)+0] = ffgetc(f) >> 2;
			ffgetc(f);
		}

		for( y=0; y<bmp.height; y++ ) {
			ffread( SPRLINE(*ptr,bmp.height-(y+1)), bmp.width, 1, f );
			if( wb != bmp.width )
				ffread( buf, wb - bmp.width, 1, f );
		}

		if( w ) *w = bmp.width;
		if( h ) *h = bmp.height;
		if( bpp ) *bpp = 8;
	}
	else
	if( bmp.bitcount == 24 ) {

		ALLOCMEM( *ptr, (bmp.width*bmp.height*3)+SPRITEHEADER );

		MKSPRW( *ptr, bmp.width);
		MKSPRH( *ptr, bmp.height);
		MKSPR24( *ptr );

		wb = (bmp.width * 3 + 3) & ~3;

		for( y=0; y<bmp.height; y++ ) {
			spr = SPRLINE(*ptr,bmp.height-(y+1));
			for( x=0; x<bmp.width; x++ ) {
				spr[2] = ffgetc(f);
				spr[1] = ffgetc(f);
				spr[0] = ffgetc(f);
				spr += 3;
			}
			if( wb != (bmp.width*3) )
				ffread( buf, wb - (bmp.width*3), 1, f );
		}

		if( w ) *w = bmp.width;
		if( h ) *h = bmp.height;
		if( bpp ) *bpp = 24;
	}
	else
		Quit("LoadBmp(\"%s\"): bitcount == %d.",name,bmp.bitcount);

	ffclose( f );

	return TRUE;
}



//
//
//
BOOL IsBmp( char *name ) {

	bmpheader_t bmp;
	FILE *f;

	if( (f = ffopen(name,"rb")) == NULL ) {
		//Quit( "IsBmp(): can't open \"%s\" file.",name);
		return FALSE;
	}

	ffread( &bmp, sizeof(bmpheader_t), 1, f );

	ffclose( f );

	if( bmp.type != 0x4d42 )
		return FALSE;

	if( (bmp.bitcount != 8) && (bmp.bitcount != 24) ) {
		xprintf("IsBmp: %d bitcount not supported.\n",bmp.bitcount);
		return FALSE;
	}

	return TRUE;
}



//
//
//
BOOL XLIB_BmpDim( char *name, int *w, int *h ) {

	bmpheader_t bmp;
	FILE *f;

	if( !name || !w || !h )
		return FALSE;

	if( (f = fopen(name,"rb")) == NULL )
		return FALSE;

	fread( &bmp, sizeof(bmpheader_t), 1, f );

	fclose( f );

	if( bmp.type != 0x4d42 )
		return FALSE;

	*w = bmp.width;
	*h = bmp.height;

	return TRUE;
}



//
//
//
BOOL SaveBmp( char *name, UCHAR *s, UCHAR *p ) {

	FILE *f;
	bmpheader_t bmp;
	int x,y,word,wb;
	UCHAR *spr,buf[4];

	if( ISSPR8(s) )
		return TRUE;

	PushTomFlag();
	TomFlag( NOFLAG );

	if( (f=ffopen(name,"wb")) == NULL ) {
		PopTomFlag();
		return FALSE;
	}

	bmp.type = 0x4d42;
	bmp.filesize = sizeof(bmpheader_t) + 3 * SPRITEW(s)*SPRITEH(s);
	bmp.unused1 = 0;
	bmp.unused2 = 0;
	bmp.offsetbits = 54;
	bmp.infosize = 40;
	bmp.width = SPRITEW(s);
	bmp.height = SPRITEH(s);
	bmp.w1 = 0;
	bmp.h1 = 0;
	bmp.planes = 1;
	bmp.bitcount = 24;
	bmp.compression = 0;
	bmp.imagesize = 0;
	bmp.xpels = 0;
	bmp.ypels = 0;
	bmp.colorsused = 0;
	bmp.colorsimportant = 0;
	bmp.u1 = 0;

	ffwrite( &bmp, sizeof(bmpheader_t), 1, f );

	wb = (bmp.width * 3 + 3) & ~3;

	if( ISSPR16(s) ) {
		//xprintf("high sprite.\n");
		for( y=0; y<SPRITEH(s); y++ ) {
			spr = SPRLINE(s,SPRITEH(s)-(y+1));
			for( x=0; x<SPRITEW(s); x++ ) {
				word = *spr++;
				word += ((int)(*spr++))<<8;
				ffputc(INTBLUE(word)<<(8-bsize),f);
				ffputc(INTGREEN(word)<<(8-gsize),f);
				ffputc(INTRED(word)<<(8-rsize),f);
			}
			if( wb != (bmp.width*3) )
				ffwrite( buf, wb - (bmp.width*3), 1, f );
		}
	}
	else
	if( ISSPR24(s) ) {
		//xprintf("true sprite.\n");
		for( y=0; y<SPRITEH(s); y++ ) {
			spr = SPRLINE(s,SPRITEH(s)-(y+1));
			for( x=0; x<SPRITEW(s); x++ ) {
				ffputc(spr[2],f);
				ffputc(spr[1],f);
				ffputc(spr[0],f);
				spr += 3;
			}
			if( wb != (bmp.width*3) )
				ffwrite( buf, wb - (bmp.width*3), 1, f );
		}
	}

	ffclose( f );

	PopTomFlag();

	return TRUE;
}







/*
 * general bitmap loading
 *
 */

UCHAR pic_pal[768];



//
//
//
void GetRgbPicture( UCHAR *pal ) {

	memcpy( pal, pic_pal, 768 );

	return;
}



//
//
//
void SetRgbPicture( void ) {

	SetRgbBuf( pic_pal );

	return;
}




//
// quiet fail. Jó lesz-e ez?
//
BOOL LoadPicture( char *name, memptr_ptr ptr, int *w,int *h, int *bpp, int *delay, BOOL bPanel ) {

	BOOL result = FALSE;

	// xprintf("LoadPicture: loading \"%s\" ...\n", name);

	     if( IsFree(name) ) { /* xprintf("freeimage\n"); */ result = LoadFree( name, ptr, w,h,bpp, delay, bPanel ); }
	else if( IsBmp(name) ) {  /* xprintf("bmp\n"); */ result = LoadBmp( name, ptr, w,h,bpp ); }
	// else if( XLIB_IsIjl(name) ) { /* xprintf("inteljpeg\n"); */ result = XLIB_LoadIjl( name, ptr ); }
	else if( IsSpr(name) ) {  /* xprintf("spr\n"); */ result = LoadSprite( name, ptr ); memcpy( pic_pal, system_rgb, 768 ); }
	else if( IsPcx(name) ) {  /* xprintf("pcx\n"); */ result = LoadPcx( name, ptr ); }
	else if( IsPsd(name) ) {  /* xprintf("psd\n"); */ result = LoadPsd( name, ptr ); }
	else if( IsRaw(name) ) {  /* xprintf("raw\n"); */ result = LoadRaw( name, ptr ); }
	// else if( IsJpeg(name) ) { /* xprintf("jpg\n"); */ result = LoadJpeg( name, ptr ); }
	// else if( IsTiff(name) ) { /* xprintf("tif\n"); */ result = LoadTiff( name, ptr ); }
	// else if( IsPng(name) ) { /* xprintf("png\n"); */ result = LoadPng( name, ptr ); }
	// else if( IsIco(name) ) { /* xprintf("ico\n"); */ result = LoadIco( name, ptr ); }
	else xprintf("LoadPicture: File not found or unsupported format in \"%s\" file!\n",name);

	return result;
}


//
//
//
BOOL GetPictureDim( char *name, int *width, int *height, int *bpp ) {

	UCHAR *spr;

	if( !LoadPicture(name,PTR(spr)) )
		Quit("GetPictureDim(): can't load \"%s\" picture.",name);

	*width = SPRITEW(spr);
	*height = SPRITEH(spr);

	FREEMEM( spr );

	return TRUE;
}


#if 0

int main() {

	FILE *f;
	bmp_t bmp;

	f=fopen("win95.bmp","rb");

	fread( &bmp, sizeof(bmp_t), 1, f );

	fclose(f);

	xprintf("size: %d\n",bmp.filesize);
	xprintf("bit: %d\n",bmp.bitcount);
	xprintf("comp: %d\n",bmp.compression);
	xprintf("pic: %d, %d\n",bmp.width,bmp.height);

	return 0;
}


#endif
