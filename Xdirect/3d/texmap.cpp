/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <xlib.h>
#include <xinner.h>

RCSID( "$Id: texmap.c,v 1.0 97-03-10 17:38:35 bernie Exp $" )


#define INITTEXMAP 50			// ha nem lesz eleg akkor, ennyivel többet allokál

static int alloced_texmap = 0;

int ntexmaps = 0;
texmap_t *texmap = NULL;

static int inner_texmapid_cnt = 1;

int texture_scale = 1;


//
//
//
void DumpTexmap( void ) {

	for( int i=0; i<ntexmaps; i++ ) {

		char str[XMAX_PATH];
		char name[XMAX_PATH];
		UCHAR pal[768];

		strcpy( name, GetFilenameNoPath( texmap[i].name ) );
		strrepc( name, '.', '_' );

		sprintf( str, "%s%cxlib_tex%02d_%s.jpg", GetRealTemp(), PATHDELIM, i, name );
		SaveFree( str, texmap[i].sprite1, pal );
	}

	return;
}



//
//
//
void ListTexmap( void ) {

	int i;

	for( i=0; i<ntexmaps; i++ ) {

		xprintf("next: %d, prev: %d\n",texmap[i].next,texmap[i].prev);

		xprintf("texmap[%d]: \"%s\", id: %d\n",i,texmap[i].name,texmap[i].texmapid);

		xprintf("sprite: %d, %d, %d\n",texmap[i].sprite1,texmap[i].sprite2,texmap[i].sprite3);
		xprintf("bump: %d, %d, %d\n",texmap[i].bump1,texmap[i].bump2,texmap[i].bump3);

		xprintf("w1: %d (%d), h1: %d (%d)\n",texmap[i].width1,(texmap[i].sprite1)?SPRITEW(texmap[i].sprite1):-1,texmap[i].height1,(texmap[i].sprite1)?SPRITEH(texmap[i].sprite1):-1);
		xprintf("w2: %d (%d), h2: %d (%d)\n",texmap[i].width2,(texmap[i].sprite2)?SPRITEW(texmap[i].sprite2):-1,texmap[i].height2,(texmap[i].sprite2)?SPRITEH(texmap[i].sprite2):-1);
		xprintf("w3: %d (%d), h3: %d (%d)\n",texmap[i].width3,(texmap[i].sprite3)?SPRITEW(texmap[i].sprite3):-1,texmap[i].height3,(texmap[i].sprite3)?SPRITEH(texmap[i].sprite3):-1);
	}

	return;
}




//
//
//
int GetFirstTexNum( void ) {

	if( ntexmaps <= 0 )
		return -1;

	return texmap[0].texmapid;
}




//
//
//
int TexNumForName( const char *name ) {

	int i;
	// char clipped_name[MAXTEXMAPNAMELEN];

	// memcpy( clipped_name, name, MIN(MAXTEXMAPNAMELEN,strlen(name)+1) );
	// clipped_name[MAXTEXMAPNAMELEN] = 0;

	for( i=0; i<ntexmaps; i++ )

		if( !stricmp( name, texmap[i].name) )
			return texmap[i].texmapid;

	return (-1);
}




//
//
//
char *TexNameForNum( int texmapid ) {

	int i;

	for( i=0; i<ntexmaps; i++ )

		if( texmap[i].texmapid == texmapid )
			return texmap[i].name;

	return NULL;
}





//
//
//
texmap_t *TexForNum( int texmapid ) {

	int i;

	for( i=0; i<ntexmaps; i++ )

		if( texmap[i].texmapid == texmapid )
			return &texmap[i];

	return NULL;
}



//
//
//
static BOOL checkTexSize( int w ) {

	int i, maxTexSize = GFXDLL_MAXTEXSIZE;

	if( GFXDLL_GetData )
		GFXDLL_GetData( &maxTexSize, sizeof(int) );
	else
		maxTexSize = 256;

	for( i=0; pow(2.0, i) <= maxTexSize; i++ )
		if( w == pow(2.0, i) )
			return TRUE;

	return FALSE;
}




//
//
//
int AddTexMap( UCHAR *spr, const char *name, const char *filename, int flag, ULONG chromaColor ) {

	int x,w,h,size;
	FLOAT log2w,log2h;

	xprintf("texture map: ");

	if( (x=TexNumForName(name)) != -1 ) {
		xprintf("\"%s\" already loaded.\n",name);
		return x;
	}

	if( alloced_texmap < (ntexmaps + 1) ) {

		alloced_texmap += INITTEXMAP;
		REALLOCMEM( texmap, alloced_texmap * sizeof(texmap_t) );
	}

	ERASETEXMAP( ntexmaps );

	texmap[ntexmaps].texmapid = inner_texmapid_cnt++;

	texmap[ntexmaps].isAlpha = ISFLAG( flag, TF_ALPHA );
	texmap[ntexmaps].isChromaKey = ISFLAG( flag, TF_CHROMA );
	texmap[ntexmaps].chromaColor = chromaColor;

	// memcpy( texmap[ntexmaps].name, name, MIN(MAXTEXMAPNAMELEN,strlen(name)+1) );
	// texmap[ntexmaps].name[MAXTEXMAPNAMELEN] = 0;
	strcpy( texmap[ntexmaps].name, name );

	w = SPRITEW(spr);
	h = SPRITEH(spr);
	size = (w*h) + SPRITEHEADER;

	log2w = (FLOAT)(log((float)w)/log(2.0f));
	if( log2w - ceil(log2w) != FLOAT_ZERO )
		xprintf("log: \"%s\" width log2.",name);

	log2h = (FLOAT)ceil(log((float)h)/log(2.0f));
	if( log2h - floor(log2h) != FLOAT_ZERO )
		xprintf("log: \"%s\" height log2.",name);


	ALLOCMEM( texmap[ntexmaps].sprite1, size );
	NAMEMEM( texmap[ntexmaps].sprite1, name );

	memcpy( texmap[ntexmaps].sprite1, spr, size );

	texmap[ntexmaps].width1 = w;
	texmap[ntexmaps].height1 = h;

	texmap[ntexmaps].wmask1 = (int)log2w;
	texmap[ntexmaps].hmask1 = (int)log2h;


	/*-----------------97.12.09 02.06-------------------
	 * bump
	 --------------------------------------------------*/

	/***
	ALLOCMEM( texmap[ntexmaps].bump1, size );
	memset( texmap[ntexmaps].bump1, 0L, size );

	MKSPRW( texmap[ntexmaps].bump1, w );
 	MKSPRH( texmap[ntexmaps].bump1, h );
	MKSPR8( texmap[ntexmaps].bump1 );

	for( y=1; y<h-1; y++ )
	for( x=1; x<w-1; x++ ) {

		col = SPR( texmap[ntexmaps].sprite1, x+1, y ) -
		      SPR( texmap[ntexmaps].sprite1, x-1, y ) +
		      SPR( texmap[ntexmaps].sprite1, x, y+1 ) -
		      SPR( texmap[ntexmaps].sprite1, x, y-1 );

		if( col < 0L ) col = 0L;
		if( col > 0xff ) col = 0xff;

		SPR( texmap[ntexmaps].bump1, x, y ) = col;
	}
	***/

	++ntexmaps;

	xprintf("\"%s\" loaded.\n",name);

	return texmap[ntexmaps-1].texmapid;
}



//
//
//
static __inline void BlendColor( USHORT *video, USHORT color ) {

	int r,g,b;

	r = INTRED16(color) + INTRED16(*video);
	g = INTGREEN16(color) + INTGREEN16(*video);
	b = INTBLUE16(color) + INTBLUE16(*video);

	CLAMPMAX( r, rmask );
	CLAMPMAX( g, gmask );
	CLAMPMAX( b, bmask );

	*video = (USHORT)RGBINTP( r, g, b );

	return;
}



//
//
//
void makeBumpMap( UCHAR *spr ) {

	int x,y;
	UCHAR *bump;
	USHORT *ptr;

	ALLOCMEM( bump, SPRITESIZE(spr) );

	ptr = (USHORT*)(&bump[ SPRITEHEADER ]);

	for( y=2; y<SPRITEH(spr)+2; y++ )
	for( x=2; x<SPRITEW(spr)+2; x++ ) {

		memcpy( ptr++, &SPR(spr,x%SPRITEW(spr),y%SPRITEH(spr)), sizeof(USHORT) );
	}

	ptr = (USHORT*)(&bump[ SPRITEHEADER ]);

	for( y=0; y<SPRITEH(spr); y++ )
	for( x=0; x<SPRITEW(spr); x++ ) {

		*ptr = NegateColor16( *ptr );

		BlendColor( (USHORT*)(&SPR(spr,x,y)), *ptr++ );
	}

	FREEMEM( bump );

	return;
}




//
//
//
static int makeTexMapHigh( UCHAR *orig_spr, UCHAR *pal, const char *name, int flag ) {

	int x,w,h,size;
	FLOAT log2w,log2h;
	UCHAR *spr = NULL;
	BOOL bit24 = GFXDLL_24BIT;

	if( (x=TexNumForName(name)) != (-1) ) {
		xprintf("makeTexMapHigh: \"%s\" already loaded.\n",name);
		return x;
	}

	if( alloced_texmap <= ntexmaps ) {

		alloced_texmap += INITTEXMAP;
		REALLOCMEM( texmap, (alloced_texmap*sizeof(texmap_t)) );
	}

	ERASETEXMAP( ntexmaps );

	texmap[ntexmaps].texmapid = inner_texmapid_cnt++;

	// memcpy( texmap[ntexmaps].name, name, MIN(MAXTEXMAPNAMELEN,strlen(name)+1) );
	// texmap[ntexmaps].name[MAXTEXMAPNAMELEN] = 0;
	strcpy( texmap[ntexmaps].name, name );

	if( winTexture() ) {
		xprintf("compressing ");
		ScaleSprite( SPRITEW(orig_spr)/texture_scale, SPRITEH(orig_spr)/texture_scale, orig_spr, PTR(spr) );
	}
	else {
		ALLOCMEM( spr, SPRITESIZE(orig_spr) );
		memcpy( spr, orig_spr, SPRITESIZE(orig_spr) );
	}

	if( GFXDLL_GetData )
		GFXDLL_GetData( &bit24, sizeof(BOOL) );

	// GammaSprite( spr, pal );

	if( bit24 == TRUE ) {
		if( SPRPIXELLEN(spr) < 3 )
			TrueSprite( PTR(spr), pal );
	}
	else
	if( SPRPIXELLEN(spr) != 2 )
		HighSprite( PTR(spr), pal );

	if( ISFLAG(flag,TF_BUMP) )
		makeBumpMap( spr );

	w = SPRITEW(spr);
	h = SPRITEH(spr);

	log2w = (FLOAT)(log((float)w)/log(2.0f));
	log2h = (FLOAT)ceil(log((float)h)/log(2.0f));

	size = SPRITESIZE(spr);

	ALLOCMEM( texmap[ntexmaps].sprite1, size );

	NAMEMEM( texmap[ntexmaps].sprite1, name );

	memcpy( texmap[ntexmaps].sprite1, spr, size );

	SAFE_FREEMEM( spr );

	texmap[ntexmaps].scaled = winTexture();
	texmap[ntexmaps].width1 = w;
	texmap[ntexmaps].height1 = h;

	texmap[ntexmaps].wmask1 = (int)log2w;
	texmap[ntexmaps].hmask1 = (int)log2h;

	++ntexmaps;

	return texmap[ntexmaps-1].texmapid;
}




//
//
//
int AddTexMapHigh( UCHAR *orig_spr, UCHAR *pal, const char *name, const char *filename, int flag, ULONG chromaColor ) {

	int texmapid;
	texmap_t *tex;
	UCHAR *spr = NULL;

	xprintf("texture map 16bpp: ");

	if( (texmapid = TexNumForName(name)) != (-1) )
		return texmapid;

	texmapid = makeTexMapHigh( orig_spr, pal, name, flag );

	if( (tex = TexForNum(texmapid)) == NULL )
		Quit("AddTexMapHigh: WTF?!");

	tex->isAlpha = ISFLAG( flag, TF_ALPHA );
	tex->isChromaKey = ISFLAG( flag, TF_CHROMA );
	tex->chromaColor = chromaColor;

	DupSprite( tex->sprite1, PTR(spr) );
	GammaSprite( tex->sprite1 );

	// xprintf("texmap: %d, %d\n", sizeof(texmap_t), sizeof(tex));

	if( GFXDLL_AddTexMapHigh )
		tex->hwtexmapid = GFXDLL_AddTexMapHigh( tex );

	memcpy( tex->sprite1, spr, SPRITESIZE(spr) );
	SAFE_FREEMEM( spr );

	xprintf("\"%s\" (%d,%d) loaded.\n",name, SPRITEW(orig_spr),SPRITEH(orig_spr) );

	return texmapid;
}




//
//
//
int ReloadTexMapHigh( int texmapid, UCHAR *orig_spr, UCHAR *pal, int flag, ULONG chromaColor ) {

	int w,h,size;
	FLOAT log2w,log2h;
	UCHAR *spr = NULL;
	BOOL bit24 = GFXDLL_24BIT;
	texmap_t *tex = NULL;

	if( (tex = TexForNum(texmapid)) == NULL ) {
		// xprintf("ReloadTexMapHigh: no %d texmapid.\n");
		texmapid = makeTexMapHigh( orig_spr, pal, "helper_reload_spr", flag );
		if( (tex = TexForNum(texmapid)) == NULL )
			Quit("shyte happens!");
	}

	if( winTexture() ) {
		xprintf("compressing ");
		ScaleSprite( SPRITEW(orig_spr)/texture_scale, SPRITEH(orig_spr)/texture_scale, orig_spr, PTR(spr) );
	}
	else {
		ALLOCMEM( spr, SPRITESIZE(orig_spr) );
		memcpy( spr, orig_spr, SPRITESIZE(orig_spr) );
	}

	if( GFXDLL_GetData )
		GFXDLL_GetData( &bit24, sizeof(BOOL) );

	// GammaSprite( spr, pal );

	if( bit24 == TRUE ) {
		if( SPRPIXELLEN(spr) < 3 )
			TrueSprite( PTR(spr), pal );
	}
	else
	if( SPRPIXELLEN(spr) != 2 )
		HighSprite( PTR(spr), pal );

	if( ISFLAG(flag,TF_BUMP) )
		makeBumpMap( spr );

	w = SPRITEW(spr);
	h = SPRITEH(spr);

	log2w = (FLOAT)(log((float)w)/log(2.0f));
	log2h = (FLOAT)ceil(log((float)h)/log(2.0f));

	size = SPRITESIZE(spr);

	SAFE_FREEMEM( tex->sprite1 );
	ALLOCMEM( tex->sprite1, size );

	memcpy( tex->sprite1, spr, size );

	// if( spr ) FREEMEM( spr );

	tex->scaled = winTexture();
	tex->width1 = w;
	tex->height1 = h;

	tex->wmask1 = (int)log2w;
	tex->hmask1 = (int)log2h;

	tex->isAlpha = ISFLAG( flag, TF_ALPHA );
	tex->isChromaKey = ISFLAG( flag, TF_CHROMA );
	tex->chromaColor = chromaColor;

	GammaSprite( tex->sprite1 );
	// GreySprite( tex->sprite1 );

	if( GFXDLL_ReloadTexMapHigh )
		tex->hwtexmapid = GFXDLL_ReloadTexMapHigh( tex );

	memcpy( tex->sprite1, spr, SPRITESIZE(spr) );
	FREEMEM( spr );

	// xprintf("texture \"%s\" reloaded.\n", tex->name );

	return texmapid;
}



//
//
//
BOOL ReloadTexMap( int texmapid ) {

	int i, tex_flag;
	UCHAR *spr = NULL;

	for( i=0; i<ntexmaps; i++ )
		if( texmap[i].texmapid == texmapid )
			break;

	if( i >= ntexmaps )
		// nincs ilyen
		return FALSE;

	DupSprite( texmap[i].sprite1, PTR(spr) );

	tex_flag = 0L;
	if( texmap[i].isAlpha ) SETFLAG( tex_flag, TF_ALPHA );
	if( texmap[i].isChromaKey ) SETFLAG( tex_flag, TF_CHROMA );

	ReloadTexMapHigh( texmap[i].texmapid, spr, NULL, tex_flag, texmap[i].chromaColor );

	memcpy( texmap[i].sprite1, spr, SPRITESIZE(spr) );
	SAFE_FREEMEM( spr );

	return TRUE;
}




//
//
//
BOOL ReloadAllTexMap( BOOL flag ) {

	for( int i=0; i<ntexmaps; i++ ) {

		UCHAR *spr = NULL;

		DupSprite( texmap[i].sprite1, PTR(spr) );

		int tex_flag = 0L;
		if( texmap[i].isAlpha ) SETFLAG( tex_flag, TF_ALPHA );
		if( texmap[i].isChromaKey ) SETFLAG( tex_flag, TF_CHROMA );

		ReloadTexMapHigh( texmap[i].texmapid, spr, NULL, tex_flag, texmap[i].chromaColor );

		if( flag == TRUE ) {
			UCHAR *spr1;
			char str[128];
			DupSprite( texmap[i].sprite1, PTR(spr1) );
			TrueSprite( PTR(spr1), NULL );
			sprintf(str, "re_%d.bmp", i );
			SaveBmp( str, spr1, NULL );
			FREEMEM( spr1 );
		}

		memcpy( texmap[i].sprite1, spr, SPRITESIZE(spr) );
		SAFE_FREEMEM( spr );
	}

	xprintf( "ReloadAllTexMap: reloaded %d textures.\n", ntexmaps );

	return TRUE;
}





//
//
//
BOOL ModifyTexMap( int texmapid, UCHAR *spr, UCHAR *pal, int flag ) {

	texmap_t *texmap = NULL;

	if( (texmap = TexForNum(texmapid)) == NULL ) {
		xprintf("ModifyTexMap: can't find %d texture.\n",texmapid);
		return FALSE;
	}

	if( SPRITESIZE(texmap->sprite1) != SPRITESIZE(spr) ) {
		xprintf("ModifyTexMap: sprite size mismatch.\n");
		return FALSE;
	}

	BOOL isAlpha = ISFLAG( flag, TF_ALPHA );
	BOOL isChromaKey = ISFLAG( flag, TF_CHROMA );
	int chromaColor = 0;

	// xprintf("%d\n",flag);

	memcpy( texmap->sprite1, spr, SPRITESIZE(spr) );

	/*** kell-e ez?
	if( isAlpha != texmap->isAlpha || isChromaKey != texmap->isChromaKey  ) {

		// TODO: a chromakey nem müködik a driverekbe
		// de lehet hogy jó tesztelni kell nagyon nagy texekkel
		xprintf("itt egy reload %d\n",GetTic());

		texmap->isAlpha = ISFLAG( flag, TF_ALPHA );
		texmap->isChromaKey = ISFLAG( flag, TF_CHROMA );
		texmap->chromaColor = 0;

		if( GFXDLL_ReloadTexMapHigh )
			texmap->hwtexmapid = GFXDLL_ReloadTexMapHigh( texmap );
	}
	else
	***/
	if( GFXDLL_ModifyTexMapHigh )
		texmap->hwtexmapid = GFXDLL_ModifyTexMapHigh( texmap );

	return TRUE;
}





// #define MAXTEXW 256
// #define MAXTEXH 256

#define MAXTEXW 1024
#define MAXTEXH 1024

#define MINTEXW 1
#define MINTEXH 1

//
//
//
int LoadTexmap( char *name, int flag, ULONG color ) {

	UCHAR *spr=NULL,*scaled=NULL,pal[768],*tex=NULL;
	int num,destx,desty,w,h,d,origw,origh;

	if( (num = TexNumForName(name)) != (-1) )
		return num;

	if( LoadPicture( name, PTR(spr) ) == FALSE )
		return (-1);

	GetRgbPicture( pal );

	/***
	static int cnt = 0;
	char filename[PATH_MAX];
	sprintf( filename, "valami_face_%02d.pcx", cnt++ );
	SavePcx( filename, spr, pal );
	***/

	origw = SPRITEW(spr);
	origh = SPRITEH(spr);


	if( bpp > 8 ) {

		w = SPRITEW(spr);
		h = SPRITEH(spr);

		destx = w;
		desty = h;

		// kettõ hatványai legyen a w és h
		if( destx > desty ) {
			destx = (int)powf( 2, ceil( log(destx) / log(2) ) );
			desty = destx;
		}
		else {
			desty = (int)powf( 2, ceil( log(desty) / log(2) ) );
			destx = desty;
		}

		CLAMPMINMAX( destx, MINTEXW, MAXTEXW );
		CLAMPMINMAX( desty, MINTEXH, MAXTEXH );

		if( (destx != w) || (desty != h) ) {
			xprintf( "Scaling (%d, %d) -> (%d, %d) [max: %d,%d] ", w,h, destx,desty,MAXTEXW,MAXTEXH);
			ScaleSprite( destx, desty, spr, PTR(scaled) );
			xprintf("done.\n");
			tex = scaled;
		}
		else
			tex = spr;

		num = AddTexMapHigh( tex, pal, name, name, flag, color );
	}
	else {
		SystemSprite( pal, spr );
		num = AddTexMap( spr, name, name, flag, color );
	}

	if( spr ) FREEMEM( spr );
	if( scaled ) FREEMEM( scaled );

	texmap_t *texmap = TexForNum( num );
	if( texmap ) {
		texmap->origw = origw;
		texmap->origh = origh;
	}

	return TexNumForName( name );
}




//
//
//
void DiscardAllTexmap( void ) {

	int i;

	if( GFXDLL_DiscardAllTexture ) GFXDLL_DiscardAllTexture();

	for( i=0; i<ntexmaps; i++ ) {
		SAFE_FREEMEM( texmap[i].sprite1 );
		SAFE_FREEMEM( texmap[i].sprite2 );
		SAFE_FREEMEM( texmap[i].sprite3 );

		SAFE_FREEMEM( texmap[i].bump1 );
		SAFE_FREEMEM( texmap[i].bump2 );
		SAFE_FREEMEM( texmap[i].bump3 );
	}

	SAFE_FREEMEM( texmap );

	ntexmaps = 0;
	alloced_texmap = 0;
	texmap = NULL;

	return;
}



//
//
//
void DiscardTexmap( int texmapid ) {

	int i;

	xprintf("texture map: ");

	for( i=0; i<ntexmaps; i++ )
		if( texmap[i].texmapid == texmapid )
			break;

	if( i >= ntexmaps ) {
		xprintf("DiscardTexmap: no %d texture (ntexs = %d).\n",texmapid, ntexmaps);
		return;
	}

	if( GFXDLL_DiscardTexture ) GFXDLL_DiscardTexture( texmap[i].hwtexmapid );

	SAFE_FREEMEM( texmap[i].sprite1 );
	SAFE_FREEMEM( texmap[i].sprite2 );
	SAFE_FREEMEM( texmap[i].sprite3 );

	SAFE_FREEMEM( texmap[i].bump1 );
	SAFE_FREEMEM( texmap[i].bump2 );
	SAFE_FREEMEM( texmap[i].bump3 );

	xprintf("unloaded \"%s\" texture.\n",texmap[i].name);

	if( ((ntexmaps-i)-1) > 0 )
		memmove( &texmap[i], &texmap[i+1], ((ntexmaps-i)-1) * sizeof(texmap_t) );

	memset( &texmap[ntexmaps-1], 0L, sizeof(texmap_t) );

	--ntexmaps;

	return;
}
