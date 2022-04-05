/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <windows.h>

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <xlib.h>
#include <xinner.h>

RCSID( "$Id: spr.c,v 1.0 97-03-10 17:35:25 bernie Exp $" )


//
// 8 bit, 24 bit
//
BOOL IsSpr( char *name ) {

	FILE *f;
	char id[ FI_SIZE ];
	BOOL result = FALSE;

	if( (f = ffopen(name,"rb")) == NULL )
		return FALSE;

	ffread( id, FI_SIZE, 1, f );
	result = IdBufStr( (char*)id, FIS_SPR );

	ffclose( f );

	return result;
}




//
//
//
BOOL WriteSprite( char *fname, UCHAR *b ) {

	UCHAR *save_spr;

	ALLOCMEM( save_spr, SPRITESIZE(b) + FI_SIZE );

	memcpy( save_spr, FIS_SPR, FI_SIZE );
	memcpy( save_spr+FI_SIZE, b, SPRITESIZE(b) );

	int res = XL_WriteFile( fname, save_spr, FI_SIZE + SPRITESIZE(b) );

	FREEMEM( save_spr );

	return res;
}






//
//
//
BOOL LoadSprite( char *name, memptr_ptr ptr ) {

	int len;

	if( !(len = LoadFile(name,ptr)) )
		return FALSE;

	if( !IdBufStr( (char*)(*ptr), FIS_SPR ) ) {
		FREEMEM( *ptr );
		return FALSE;
	}

	memmove( *ptr, &(*ptr)[FI_SIZE], len-FI_SIZE );

	if( ( SPRITESIZE(*ptr) + FI_SIZE ) != len ) {
		FREEMEM( *ptr );
		return FALSE;
	}

	REALLOCMEM( *ptr, len - FI_SIZE );

	return TRUE;
}






//
//
//
void GetSpriteDim( char *name, int *w, int *h ) {

	UCHAR *s = NULL;

	LoadSprite( name, &s );

	*w = SPRITEW(s);
	*h = SPRITEH(s);

	FREEMEM( s );

	return;
}






//
//
//
void DupSprite( UCHAR *s1, memptr_ptr ptr ) {

	if( !s1 || !ptr )
		return;

	ALLOCMEM( *ptr, SPRITESIZE(s1) );
	memcpy( *ptr, s1, SPRITESIZE(s1) );

	return;
}





//
//
//
void MungeSprite( UCHAR *s ) {

	int x,y,p,offs;
	UCHAR *buf;

	/***
	if((SPRITEW(s)%4)!=0) {
		Quit("MungeSprite(): not 4!");
	}
	***/

	ALLOCMEM(buf,SPRITEW(s));

	for(y=0,offs=0;y<SPRITEH(s);y++,offs=0) {
		for(p=0;p<4;p++)
			for(x=p;x<SPRITEW(s);(x+=4),offs++)
				buf[offs] = s[SPRITEW(s)*y+x+SPRITEHEADER];

		memcpy(&s[SPRITEW(s)*y+SPRITEHEADER],buf,SPRITEW(s));
	}

	FREEMEM( buf );

	return;
}



#if 1
//
// this is the fixed-point version of the scaling/rotation routine.
// since we want maximum speed while still remaining in C parts of this
// may be hard to read compared to the general rotate/scale routine.
// It is similiar to the mathematical version except it uses 16.16
// fixed-point and computes an initial texture vector then does something
// similiar to a two-level line drawing routine, ok?
// x = X*cos(a)-Y*sin(a)
// y = X*sin(a)+Y*cos(a)
//
// input: the scaling factor, the rotation angle
//
void RotateScaleSprite( UCHAR *spr1, FLOAT scale, FLOAT angle, UCHAR *spr2 ) {

	int sina = ftoi((FLOAT)sin(angle)*65536.0f*scale);
	int cosa = ftoi((FLOAT)cos(angle)*65536.0f*scale);
	int tempx,tempy,x,y;
	UCHAR *screen = &spr2[SPRITEHEADER];

	for( y=0; y<SPRITEH(spr2); y++ ) {

		for( x=0; x<SPRITEW(spr2); x++ ) {

			tempx = (x-(SPRITEW(spr2)/2))*cosa - (y-(SPRITEH(spr2)/2))*sina;
			tempy = (x-(SPRITEW(spr2)/2))*sina + (y-(SPRITEH(spr2)/2))*cosa;

			tempx >>= 16;
			tempy >>= 16;

			tempx += (SPRITEW(spr1)/2);
			tempy += (SPRITEH(spr1)/2);

			if( (tempx  < 0)	     || // clip
			    (tempx >= SPRITEW(spr1)) ||
			    (tempy  < 0)	     ||
			    (tempy >= SPRITEH(spr1)) )

				screen[x] = NOCOLOR;  // clip to black
			else

				screen[x] = SPR(spr1,tempx,tempy); //draw texel

		}

		screen += SPRITEW(spr2);
	}

	return;
}


#else
void RotateScaleSprite( UCHAR *spr1, FLOAT scale, FLOAT angle, UCHAR *spr2 ) {

	int sinas = sin(angle)*65536*scale;
	int cosas = cos(angle)*65536*scale;

	// x' = cos(-angle)+sin(-angle)
	// y' = cos(-angle)-sin(-angle)
	int xc = ((SPRITEW(spr1)/2)*65536) - ((SPRITEH(spr2)/2)*(cosas+sinas));
	int yc = ((SPRITEH(spr1)/2)*65536) - ((SPRITEW(spr2)/2)*(cosas-sinas));

	int tx,ty;
	int x,y;
	int tempx,tempy;
	UCHAR *screen = &spr2[SPRITEHEADER];

	for( y=0; y<SPRITEH(spr2); y++ ) {

		tx = xc;
		ty = yc;

		for( x=0; x<SPRITEW(spr2); x++ ) {

			tempx = (tx>>16);
			tempy = (ty>>16);

			if( (tempx  < 0)	     || // clip
			    (tempx >= SPRITEW(spr1)) ||
			    (tempy  < 0)	     ||
			    (tempy >= SPRITEH(spr1)) )

				screen[x] = NOCOLOR;  // clip to black
			else

				screen[x] = spr1[SPRITEHEADER+tempx+tempy*SPRITEW(spr1)]; //draw texel

			tx += cosas;
			ty -= sinas;
		}

		screen += SPRITEW(spr2);
		xc += sinas;
		yc += cosas;
	}

	return;
}
#endif



//
//
//
void ColumnMajorTransformSprite( UCHAR *spr ) {

	int x,y;
	UCHAR *dest;

	ALLOCMEM( dest, SPRITEW(spr)*SPRITEH(spr) );

	for( y=0; y<SPRITEH(spr); y++ )
	for( x=0; x<SPRITEW(spr); x++ )
		dest[x*SPRITEH(spr)+y] = spr[SPRITEHEADER+(y*SPRITEW(spr))+x];

	memcpy( &spr[SPRITEHEADER], dest, SPRITEW(spr)*SPRITEH(spr) );

	FREEMEM( dest );

	return;
}



//
//
//
void FlipHorizontalSprite( UCHAR *b ) {

	int x,y;
	UCHAR *p;


	ALLOCMEM( p, SPRITEW(b) );

	for( y=0; y<SPRITEH(b); y++ ) {
		for( x=0; x<SPRITEW(b); x++ )
			p[x] = b[SPRITEHEADER+y*SPRITEW(b)+SPRITEW(b)-(x+1)];

		memcpy(&b[SPRITEHEADER+y*SPRITEW(b)],&p[0],SPRITEW(b));
	}

	FREEMEM( p );

	return;
}






//
//
//
void FlipVerticalSprite( UCHAR *buf ) {

	UCHAR *top;
	UCHAR *bottom;
	UCHAR *temp;
	int i, x, y;;

	x = SPRITEW(buf);
	y = SPRITEH(buf);

	ALLOCMEM( temp, x );

	top = &buf[SPRITEHEADER];
	bottom = &buf[SPRITEHEADER] + (x * (y-1));

	i = (y >> 1);
	while( i-- ) {
		memcpy( temp, top, x );
		memcpy( top, bottom, x );
		memcpy( bottom, temp, x );
		top += x;
		bottom -= x;
	}

	FREEMEM( temp );

	return;
}





//
// True High Normal
//
void ScaleSprite( int dest_x, int dest_y, UCHAR *buf1, memptr_ptr buf2 ) {

	int oldx, oldy, newx, newy;
	int i, j, count, k,x,y;
	UCHAR *src_base;
	UCHAR *src_ptr;
	UCHAR *dest_ptr;
	UCHAR *newbuf;
	int bpp = SPRPIXELLEN(buf1);  // byte per pixel
	FLOAT fw,fh;

	xprintf( "(bpp = %d) ", bpp );

	oldx = SPRITEW(buf1);
	oldy = SPRITEH(buf1);
	newx = dest_x;
	newy = dest_y;

	ALLOCMEM( *buf2, SPRITEHEADER + (newx*newy*bpp) );
	MKSPRW( *buf2, newx );
	MKSPRH( *buf2, newy );
	     if( bpp == 4 ) { MKSPR32( *buf2 ); }
	else if( bpp == 3 ) { MKSPR24( *buf2 ); }
	else if( bpp == 2 ) { MKSPR16( *buf2 ); }
	else 		    { MKSPR8( *buf2 ); }

	newbuf = &((*buf2)[SPRITEHEADER]);

	src_base = &buf1[SPRITEHEADER];
	dest_ptr = newbuf;

	fw = (FLOAT)oldx / (FLOAT)newx;
	fh = (FLOAT)oldy / (FLOAT)newy;
	xprintf( "(%f, %f) ", fw, fh );

	for( y=0; y<newy; y++ )
	for( x=0; x<newx; x++ ) {

		j = ftoi( (FLOAT)y * fh );
		k = ftoi( (FLOAT)x * fw );

		CLAMPMINMAX( j, 0, oldy-1 );
		CLAMPMINMAX( k, 0, oldx-1 );

		src_ptr = src_base + (j * oldx + k) * bpp;

		for( i=0; i<bpp; i++ )
			*(dest_ptr+i) = *(src_ptr+i);
		dest_ptr += bpp;
	}

#if 0
	static int cnt = 1;
	char filename[256];
	sprintf( filename, "old_scalesprite_%d.pcx", cnt++ );
	SavePcx( filename, buf1, NULL );
	sprintf( filename, "new_scalesprite_%d.pcx", cnt++ );
	SavePcx( filename, *buf2, NULL );
#endif

/***
	ULONG ErrorAccX, ErrorAccY, ErrorAdjX, ErrorAdjY;

	dest_ptr = newbuf;

	// My bitmap scaling routine.  As you probably noticed, it's
	// pretty Bresenhammy!

	ErrorAccY = 0x8000;

	if( newx > oldx ) {

		// Biggering

		ErrorAdjX = ((((ULONG)newx) << 16) / (((ULONG)oldx)));
		ErrorAdjY = ((((ULONG)newy) << 16) / (((ULONG)oldy)));

		i=oldy;
		while( i-- ) {

			ErrorAccX = 0x8000;
			src_ptr = src_base;

			j=oldx;
			while( j-- ) {

				ErrorAccX += ErrorAdjX;
				if( (count = (ErrorAccX >> 16)) ) {

					ErrorAccX &= 0xFFFFL;
					while( count-- ) {
						for( k=0; k<bpp; k++ )
							*(dest_ptr+k) = *(src_ptr+k);
						dest_ptr += bpp;
					}
				}

				src_ptr += bpp;
			}

			ErrorAccY += ErrorAdjY;
			count = (ErrorAccY >> 16) - 1;

			while( count-- ) {
				memmove( dest_ptr, dest_ptr - (newx*bpp), newx*bpp );
				dest_ptr += (newx*bpp);
			}

			ErrorAccY &= 0xFFFFL;
			src_base += (oldx*bpp);
		}

	}
	else {

		// Smallering

		ErrorAdjX = ((((ULONG)oldx) << 16) / (((ULONG)newx)));
		ErrorAdjY = ((((ULONG)oldy) << 16) / (((ULONG)newy)));

		i=newy;
		while( i-- ) {

			ErrorAccX = 0x8000;
			src_ptr = src_base;

			j=newx;
			while( j-- ) {

				for( k=0; k<bpp; k++ )
					*(dest_ptr+k) = *(src_ptr+k);
				dest_ptr += bpp;

				ErrorAccX += ErrorAdjX;

				src_ptr += ((ErrorAccX >> 16) * bpp);

				ErrorAccX &= 0xFFFFL;
			}

			ErrorAccY += ErrorAdjY;

			src_base += ((oldx*bpp) * (ErrorAccY >> 16));

			ErrorAccY &= 0xFFFFL;
		}

	}
***/

	return;
}










//
//
//
void VerticalScaleSprite( int dest_y, UCHAR *buf1, memptr_ptr buf2 ) {

	ULONG ErrorAccY, ErrorAdjY;
	int xsize, oldy, newy;
	int i, count;
	UCHAR *src_ptr;
	UCHAR *dest_ptr;
	UCHAR *newbuf;

	xsize = SPRITEW(buf1);
	oldy = SPRITEH(buf1);
	newy = dest_y;

	ALLOCMEM( *buf2, SPRITEHEADER + xsize*newy );

	newbuf = &(*buf2)[SPRITEHEADER];

	src_ptr = &buf1[SPRITEHEADER];
	dest_ptr = newbuf;

	// My bitmap scaling routine.  As you probably noticed, it's
	// pretty Bresenhammy!

	ErrorAccY = 0x8000;
	ErrorAdjY = ((((ULONG)newy) << 16) /
		     (((ULONG)oldy)));

	if( newy>=oldy ) {

		// Biggering

		i=oldy;
		while( i-- ) {

			ErrorAccY += ErrorAdjY;
			if( (count = (ErrorAccY >> 16)) ) {

				ErrorAccY &= 0xFFFFL;

				while( count-- ) {

					memmove(dest_ptr, src_ptr, xsize);
					dest_ptr += xsize;
				}
			}

			src_ptr += xsize;
		}
	}
	else {

		// Smallering

		i=oldy;
		while( i-- ) {

			ErrorAccY += ErrorAdjY;
			if( ErrorAccY & ~0xFFFFL ) {

				ErrorAccY &= 0xFFFFL;

				memmove(dest_ptr, src_ptr, xsize);
				dest_ptr += xsize;
			}

			src_ptr += xsize;
		}
	}

	return;
}








//
//
//
void StretchSprite( int factor, UCHAR *buf ) {

	UCHAR *buf_ptr;
	int i,scratch;

	buf_ptr = &buf[SPRITEHEADER];

	for( i = (SPRITEW(buf) * SPRITEH(buf)); i; i-- ) {

		scratch = ((((*buf_ptr - 32) * factor) + 8) >> 4) + 32;

		if( scratch <= 0 ) {
			*buf_ptr++ = 0;
		}
		else
		if( scratch >= 63 ) {
			*buf_ptr++ = 63;
		}
		else {
			*buf_ptr++ = scratch;
		}
	}

	return;
}







//
//
//
void ScaleScanline( UCHAR *source, UCHAR *dest, int smap_size, int dmap_size, int dline_size) {

	ULONG ErrorAcc, ErrorAdj;
	int i, temp, invert;

	ErrorAcc = 0x8000;

	// Prepare for backwards scanlines

	if( dline_size >= 0) {
		invert = 0;
	} else {
		invert = 1;
		dline_size = -dline_size;
	}

	if( dline_size > smap_size ) {

		// Biggering

		if( smap_size == 0 ) {
			return;
		}

		ErrorAdj = ((((ULONG)dline_size) << 16) /
			    (((ULONG)smap_size)));

		i=smap_size;
		while( i-- ) {

			ErrorAcc += ErrorAdj;
			temp = (ErrorAcc >> 16);
			ErrorAcc &= 0xFFFFL;

			while( temp-- ) {
				*dest++ = *source;
			}
			source++;
		}

	}
	else {

		// Smallering

		if( dline_size == 0 ) {
			memset(dest, 0, dmap_size);
		}
		else {
			temp = dmap_size - dline_size;
			i = temp >> 1;
			temp -= i;
			while( i-- ) {
				*dest++ = 0;
			}

			ErrorAdj = ((((ULONG)smap_size) << 16) /
				    (((ULONG)dline_size)));

			i=dline_size;

			while( i-- ) {
				*dest++ = *source;
				ErrorAcc += ErrorAdj;
				source += (ErrorAcc >> 16);
				ErrorAcc &= 0xFFFFL;
			}

			while( temp-- ) {
				*dest++ = 0;
			}
		}
	}

	return;
}





//
//
//
void CompileSprite( UCHAR *pal, UCHAR *spr1, UCHAR *spr2 ) {

	int i,w,h;
	UCHAR col[256];

	CompileRgb( pal, col );

	w = SPRITEW(spr1);
	h = SPRITEH(spr1);
	MKSPRW( spr2, w );
	MKSPRH( spr2, h );
	MKSPR8( spr2 );

	for( i=0; i<(w*h); i++ ) {

		spr2[SPRITEHEADER+i] = col[ spr1[SPRITEHEADER+i] ];
		//spr1[SPRITEHEADER+i] = col[ spr1[SPRITEHEADER+i] ];

	}

	return;
}



//
//
//
void SystemSprite( UCHAR *pal, UCHAR *spr ) {

	int i,w,h;
	UCHAR col[256];

	CompileRgb( pal, col );

	w = SPRITEW(spr);
	h = SPRITEH(spr);

	for( i=0; i<(w*h); i++ )
		spr[SPRITEHEADER+i] = col[ spr[SPRITEHEADER+i] ];

	return;
}





//
// post bilinear antialiasing
//
void BilinearAliasing( UCHAR *spr ) {

	int x,y,width,height;
	UCHAR *p;

	width  = SPRITEW(spr);
	height = SPRITEH(spr);
	p = &spr[SPRITEHEADER]+width+1;

	for( y=0; y<height-2; y++ ) {
		for( x=0; x<width-2; x++ ) {
			*p++ = (( (*p) + ( ( *(p+1)+*(p-1)+*(p-width)+*(p+width) ) >>2 ) )>>1);
		}
		p += 2;
	}

	return;
}




//
// post trilinear antialiasing
//
void TrilinearAliasing( UCHAR *spr ) {

	int x,y,width,height;
	UCHAR *p;

	width  = SPRITEW(spr);
	height = SPRITEH(spr);
	p = &spr[SPRITEHEADER]+width+1;

	for( y=0; y<height-2; y++ ) {
		for( x=0; x<width-2; x++ ) {
			*p++ = (( (*p) + ( (*(p+1)+*(p-1)+*(p-width)+*(p+width)+*(p-width-1)+*(p-width+1)+*(p+width-1)+*(p+width+1)) >>3 ) )>>1);
		}
		p += 2;
	}

	return;
}




//
// post hyperlinear? antialiasing. This is just an experiment.
//
void HyperlinearAliasing( UCHAR *spr ) {

	int x,y,width,height;
	UCHAR c,*p;

	width  = SPRITEW(spr);
	height = SPRITEH(spr);
	p = &spr[SPRITEHEADER]+width+1;

	for( y=0; y<height-2; y++ ) {
		for( x=0; x<width-2; x++ ) {
			c = (( (*p) + ( (*(p+1)+*(p-1)+*(p-width)+*(p+width)+*(p-width-1)+*(p-width+1)+*(p+width-1)+*(p+width+1)) >>3 ) )>>1);
			*(p-width-1) = c;
			*(p-width+1) = c;
			*(p+width-1) = c;
			*(p+width+1) = c;
			++p;
		}
		p += 2;
	}

	return;
}





//
// post bilinear antialiasing
//
void ColorBilinearAliasing( UCHAR *spr ) {

	int x,y,width,height;
	UCHAR c,*p;

	width  = SPRITEW(spr);
	height = SPRITEH(spr);
	p = &spr[SPRITEHEADER]+width+1;

	for( y=0; y<height-2; y++ ) {
		for( x=0; x<width-2; x++ ) {
			c = *p;
			c = aliasing_rgb[ c + ((*(p-1))<<8)	];
			c = aliasing_rgb[ c + ((*(p+1))<<8)	];
			c = aliasing_rgb[ c + ((*(p-width))<<8) ];
			c = aliasing_rgb[ c + ((*(p+width))<<8) ];
			*(p++) = c;
		}
		p += 2;
	}

	return;
}




//
// this is the original color bilinear antialiasing routine.
// it works but is very slow.
//
void AccurateColorBilinearAliasing( UCHAR *spr ) {

#define RED(i)	 system_rgb[(i)*3+0]
#define GREEN(i) system_rgb[(i)*3+1]
#define BLUE(i)  system_rgb[(i)*3+2]

	int x,y,width,height;
	int red,green,blue;
	UCHAR c,*p;

	width  = SPRITEW(spr);
	height = SPRITEH(spr);
	p = &spr[SPRITEHEADER]+width+1;

	for( y=0; y<height-2; y++ ) {
		for( x=0; x<width-2; x++ ) {

			c     = *(p-1);
			red   = RED(c);
			green = GREEN(c);
			blue  = BLUE(c);

			c      = *(p+1);
			red   += RED(c);
			green += GREEN(c);
			blue  += BLUE(c);

			c      = *(p-width);
			red   += RED(c);
			green += GREEN(c);
			blue  += BLUE(c);

			c      = *(p+width);
			red   += RED(c);
			green += GREEN(c);
			blue  += BLUE(c);

			red   >>= 2;
			green >>= 2;
			blue  >>=2;

			c=*p;
			red   += RED(c);
			green += GREEN(c);
			blue  += BLUE(c);

			red   >>= 1;
			green >>= 1;
			blue  >>= 1;

			*p = FindRgb( red, green, blue );
		}
		p += 2;
	}

	return;
}





//
//
//
void HighSprite( memptr_ptr ptr, UCHAR *pal ) {

	int x,y,w,h,word;
	UCHAR *work,*pwork,*spr,*pspr;
	BOOL cheat = FALSE;

	spr = *ptr;

	if( bpp == 0 ) {

		bpp = 16;

		rsize = 5;
		gsize = 6;
		bsize = 5;

		cheat = TRUE;
	}

	switch( SPRPIXELLEN( spr ) ) {

		case 1:
			w = SPRITEW(spr);
			h = SPRITEH(spr);

			ALLOCMEM( work, w*h*2 );

			pspr = &spr[SPRITEHEADER];
			pwork = work;

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				// 6 bites paletta -> 5:6:5
				//word	= (((ULONG)(pal[ ((*pspr)*3) + 0]) >> 1) & 0x1f) << 11;
				//word |= (((ULONG)(pal[ ((*pspr)*3) + 1]) >> 0) & 0x3f) << 5;
				//word |= (((ULONG)(pal[ ((*pspr)*3) + 2]) >> 1) & 0x1f) << 0;

				word  = (((ULONG)(pal[ ((*pspr)*3) + 0]) >> (6-rsize)) & rmask) << rshift;
				word |= (((ULONG)(pal[ ((*pspr)*3) + 1]) >> (6-gsize)) & gmask) << gshift;
				word |= (((ULONG)(pal[ ((*pspr)*3) + 2]) >> (6-bsize)) & bmask) << bshift;

				*pwork++ = LOBYTE(word);
				*pwork++ = HIBYTE(word);

				++pspr;
			}

			REALLOCMEM( *ptr, SPRITEHEADER+(w*h*2) );
			memmove( &(*ptr)[SPRITEHEADER], work, w*h*2 );
			MKSPR16( *ptr );

			FREEMEM( work );

			break;


		case 2:
			break;

		case 3:
		case 4:
			w = SPRITEW(spr);
			h = SPRITEH(spr);

			ALLOCMEM( work, w*h*2 );

			pspr = &spr[SPRITEHEADER];
			pwork = work;

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				//word	= (((ULONG)(*pspr++) >> 3) & 0x1f) << 0;   // blue
				//word |= (((ULONG)(*pspr++) >> 2) & 0x3f) << 5;   // green
				//word |= (((ULONG)(*pspr++) >> 3) & 0x1f) << 11;  // red

				word  = (((ULONG)(*pspr++) >> (8-rsize)) & rmask) << rshift;
				word |= (((ULONG)(*pspr++) >> (8-gsize)) & gmask) << gshift;
				word |= (((ULONG)(*pspr++) >> (8-bsize)) & bmask) << bshift;

				*pwork++ = LOBYTE(word);
				*pwork++ = HIBYTE(word);

				// RGBA
				pspr += (SPRPIXELLEN( spr ) - 3);
			}

			REALLOCMEM( *ptr, SPRITEHEADER+(w*h*2) );
			memmove( &(*ptr)[SPRITEHEADER], work, w*h*2 );
			MKSPR16( *ptr );

			FREEMEM( work );

			break;
	}

	if( cheat == TRUE ) {

		bpp = 0;

		rsize = 0;
		gsize = 0;
		bsize = 0;
	}

	return;
}





//
//
//
void TrueSprite( memptr_ptr ptr, UCHAR *pal ) {

	int x,y,w,h,word;
	UCHAR *work,*pwork,*spr,*pspr;
	BOOL cheat = FALSE;

	spr = *ptr;

	if( bpp == 0 ) {

		bpp = 16;

		rsize = 5;
		gsize = 6;
		bsize = 5;

		cheat = TRUE;
	}

	switch( SPRPIXELLEN( spr ) ) {

		case 1:
			w = SPRITEW(spr);
			h = SPRITEH(spr);

			ALLOCMEM( work, w*h*3 );

			pspr = &spr[SPRITEHEADER];
			pwork = work;

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				*pwork++ = (UCHAR)pal[ ((*pspr)*3) + 0 ];
				*pwork++ = (UCHAR)pal[ ((*pspr)*3) + 1 ];
				*pwork++ = (UCHAR)pal[ ((*pspr)*3) + 2 ];

				++pspr;
			}

			REALLOCMEM( *ptr, SPRITEHEADER+(w*h*3) );
			memmove( &(*ptr)[SPRITEHEADER], work, w*h*3 );
			MKSPR24( *ptr );

			FREEMEM( work );

			break;

		case 2:
			w = SPRITEW(spr);
			h = SPRITEH(spr);

			ALLOCMEM( work, w*h*3 );

			pspr = &spr[SPRITEHEADER];
			pwork = work;

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				word = *(USHORT*)pspr;
				pspr += 2;

				*pwork++ = (UCHAR)INTRED16(word) << (8-rsize);
				*pwork++ = (UCHAR)INTGREEN16(word) << (8-gsize);
				*pwork++ = (UCHAR)INTBLUE16(word) << (8-bsize);
			}

			REALLOCMEM( *ptr, SPRITEHEADER+(w*h*3) );
			memmove( &(*ptr)[SPRITEHEADER], work, w*h*3 );
			MKSPR24( *ptr );

			FREEMEM( work );

			break;

		case 3:
			break;

		case 4:
			// alphát kidobni
			w = SPRITEW(spr);
			h = SPRITEH(spr);

			ALLOCMEM( work, w*h*3 );

			pwork = work;
			pspr = &spr[SPRITEHEADER];

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {
				*pwork++ = *pspr++;
				*pwork++ = *pspr++;
				*pwork++ = *pspr++;
				++pspr;			// alpha skip
			}

			REALLOCMEM( *ptr, SPRITEHEADER+(w*h*3) );
			memmove( &(*ptr)[SPRITEHEADER], work, w*h*3 );
			MKSPR24( *ptr );

			FREEMEM( work );

			break;

	}

	if( cheat == TRUE ) {

		bpp = 0;

		rsize = 0;
		gsize = 0;
		bsize = 0;
	}

	return;
}




//
// PalSprite: Popcap Quantize.cpp
// 	mColorIndices = new uchar[mWidth*mHeight];
// 	mColorTable = new ulong[256];
// TODO: ez nem az amire gondolok
//
void PalSprite( UCHAR *theSrcBits, UCHAR *theDestColorIndices, ULONG *theDestColorTable ) {

	int theWidth = SPRITEW( theSrcBits );
	int theHeight = SPRITEH( theSrcBits );
	int aSize = theWidth*theHeight;

	int aColorTableSize = 0;

	ULONG aSearchTable[256];
	UCHAR aTranslationTable[256]; // From search table to color table

	if( aSize > 0 ) {
		aSearchTable[0] = theSrcBits[0];
		theDestColorTable[0] = theSrcBits[0];
		aTranslationTable[0] = 0;
		theDestColorIndices[0] = 0;
		aColorTableSize++;
	}

	for( int anIdx = 1; anIdx < aSize; anIdx++ ) {

		ULONG aColor = theSrcBits[anIdx];

		int aLeftPos = 0;
		int aRightPos = aColorTableSize-1;
		int aMiddlePos = (aLeftPos+aRightPos)/2;

		for( ;; ) {

			ULONG aCheckColor = aSearchTable[aMiddlePos];

			if( aColor < aCheckColor )
				aRightPos = aMiddlePos - 1;
			else
			if( aColor > aCheckColor )
				aLeftPos = aMiddlePos + 1;
			else {
				theDestColorIndices[anIdx] = aTranslationTable[aMiddlePos];
				break;
			}

			if( aLeftPos > aRightPos ) {

				if( aColorTableSize >= 256 )
					return;

				int anInsertPos = aLeftPos;
				if( (anInsertPos < aColorTableSize) && (aColor > aSearchTable[anInsertPos]) )
					anInsertPos++;

				// Insert color into the table
				memmove( aSearchTable+anInsertPos+1, aSearchTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(ULONG) );
				aSearchTable[anInsertPos] = aColor;

				memmove( aTranslationTable+anInsertPos+1, aTranslationTable+anInsertPos, (aColorTableSize-anInsertPos) * sizeof(UCHAR) );
				aTranslationTable[anInsertPos] = aColorTableSize;

				theDestColorTable[aColorTableSize] = aColor;

				theDestColorIndices[anIdx] = aColorTableSize;

				aColorTableSize++;

				break;
			}

			aMiddlePos = (aLeftPos+aRightPos)/2;
		}
	}

	return;
}



//
//
//
void OutlineSprite( UCHAR *ptr, UCHAR *pal ) {

	int x,y,w,h,word,red,green,blue,dr,dg,db,dm;
	UCHAR *spr = ptr;
	UCHAR *temp = NULL;

	if( SPRPIXELLEN( spr ) == 1 && pal == NULL ) {
		xprintf("OutlineSprite: need pal for 8 bit.\n");
		return;
	}

	ALLOCMEM( temp, SPRITEW(spr)*SPRITEH(spr) );
	memset(temp,0L,SPRITEW(spr)*SPRITEH(spr));

	for( y=0; y<SPRITEH(spr); y++ )
	for( x=0; x<SPRITEW(spr); x++ ) {

		UCHAR *pspr = SPRPTR(spr,x,y);

		red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
		green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
		blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;

		// ha színes pixel akkor tovább
		if( red>11 || green>11 || blue>11 )
			continue;

		int n = 0;
		if( y>0 ) {
			UCHAR *pspr = SPRPTR(spr,x,y-1);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			n += red;
			n += green;
			n += blue;
		}

		int nw = 0;
		if( y>0 && x>0 ) {
			UCHAR *pspr = SPRPTR(spr,x-1,y-1);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			nw += red;
			nw += green;
			nw += blue;
		}

		int w = 0;
		if( x>0 ) {
			UCHAR *pspr = SPRPTR(spr,x-1,y);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			w += red;
			w += green;
			w += blue;
		}

		int sw = 0;
		if( x>0 && y<SPRITEH(spr)-1 ) {
			UCHAR *pspr = SPRPTR(spr,x-1,y+1);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			sw += red;
			sw += green;
			sw += blue;
		}

		int s = 0;
		if( y<SPRITEH(spr)-1 ) {
			UCHAR *pspr = SPRPTR(spr,x,y+1);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			s += red;
			s += green;
			s += blue;
		}

		int se = 0;
		if( x<SPRITEW(spr)-1 && y<SPRITEH(spr)-1 ) {
			UCHAR *pspr = SPRPTR(spr,x+1,y+1);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			se += red;
			se += green;
			se += blue;
		}

		int e = 0;
		if( x<SPRITEW(spr)-1 ) {
			UCHAR *pspr = SPRPTR(spr,x+1,y);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			e += red;
			e += green;
			e += blue;
		}

		int ne = 0;
		if( x<SPRITEW(spr)-1 && y>0 ) {
			UCHAR *pspr = SPRPTR(spr,x+1,y-1);
			red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
			green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
			blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;
			ne += red;
			ne += green;
			ne += blue;
		}

		if( n+nw+w+sw+s+se+e+ne < 11 )
			continue;

		temp[y*SPRITEH(spr)+x] = 20;
	}

	// szélek, nem biztos hogy kellenek
	for( x=0; x<SPRITEW(spr); x++ ) {

		UCHAR *pspr = SPRPTR(spr,x,0);

		red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
		green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
		blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;

		if( red>11 || green>11 || blue>11 )
			temp[x] = 20;

		pspr = SPRPTR(spr,x,SPRITEH(spr)-1);

		red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
		green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
		blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;

		if( red>11 || green>11 || blue>11 )
			temp[(SPRITEH(spr)-1)*SPRITEW(spr)+x] = 20;
	}

	for( y=0; y<SPRITEH(spr); y++ ) {

		UCHAR *pspr = SPRPTR(spr,0,y);

		red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
		green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
		blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;

		if( red>11 || green>11 || blue>11 )
			temp[y*SPRITEH(spr)] = 20;

		pspr = SPRPTR(spr,SPRITEW(spr)-1,y);

		red = (SPRPIXELLEN( spr )>1)? pspr[0] : pal[ pspr[0]*3 ] + pal[ pspr[0]*3+1 ] + pal[ pspr[0]*3+2 ];
		green = (SPRPIXELLEN( spr )>1)? pspr[1] : 0;
		blue = (SPRPIXELLEN( spr )>2)? pspr[2] : 0;

		if( red>11 || green>11 || blue>11 )
			temp[y*SPRITEH(spr)+SPRITEW(spr)-1] = 20;
	}

	if( SPRPIXELLEN( spr ) == 1 ) {
		pal[20*3+0] = 20;
		pal[20*3+1] = 20;
		pal[20*3+2] = 20;
	}

	for( y=0; y<SPRITEH(spr); y++ )
	for( x=0; x<SPRITEW(spr); x++ ) {
		if( temp[y*SPRITEH(spr)+x] == 0 )
			continue;
		if( SPRPIXELLEN( spr ) == 1 )
			SPR(spr,x,y) = 20;
		if( SPRPIXELLEN( spr ) == 2 )
			;
		if( SPRPIXELLEN( spr ) > 2 ) {
			UCHAR *pspr = SPRPTR(spr,x,y);
			pspr[0] = 20;
			pspr[1] = 20;
			pspr[2] = 20;
		}
	}

	SAFE_FREEMEM(temp);

	return;
}




//
//
//
int Rgb2Gray( int red, int green, int blue ) {
	return ftoi( (FLOAT)(76 * red + 148 * green + 31 * blue) / 255.0f );
}


/*If the requested RGB shade contains a negative weight for
 * one of the primaries, it lies outside the color gamut
 * accessible from the given triple of primaries.  Desaturate
 * it by adding white, equal quantities of R, G, and B, enough
 * to make RGB all positive.  The function returns 1 if the
 * components were modified, zero otherwise.*/
int constrain_rgb( int *r, int *g, int *b ) {

	int w;

	/* Amount of white needed is w = - min(0, *r, *g, *b) */

	w = (0 < *r) ? 0 : *r;
	w = (w < *g) ? w : *g;
	w = (w < *b) ? w : *b;
	w = -w;

	/* Add just enough white to make r, g, b all positive. */

	if( w > 0 ) {
		*r += w;
		*g += w;
		*b += w;
		return 1; /* Color modified to fit RGB gamut */
	}

	return 0; /* Color within RGB gamut */
}




//
//
//
unsigned char RoundToByte( FLOAT value ) {

	if( value < 0.0 ) {
		value = 0.0;
	}
	else
	if( value > 255.0 ) {
		value = 255.0;
	}
	else {
		value += 0.5;
	}

	return (UCHAR)value;
}


//
//
//
void Rgb2Hsb( int red, int green, int blue, FLOAT *hue, FLOAT *saturation, FLOAT *brightness ) {

	int max = red > green ? red : green;
	if( blue > max ) {
		max = blue;
	}

	int min = red < green ? red : green;
	if( blue < min ) {
		min = blue;
	}

	*brightness = (FLOAT)max/255.0;

	FLOAT delta =(FLOAT)(max-min);
	if( delta == 0.0 ) {
		*saturation = 0.0;
		*hue = 0.0;
		return;
	}

	*saturation = 1.0-((FLOAT)min/(FLOAT)max);

	if( red == max) {
		*hue = ( (FLOAT)green-(FLOAT)blue ) / delta;
	}
	else {
		if( green == max) {
			*hue = 2.0 + ( (FLOAT)blue - (FLOAT)red )  / delta;
		}
		else {
			*hue = 4.0 + ( (FLOAT)red - (FLOAT)green ) / delta;
		}
	}

	*hue/=6.0;
	if( *hue < 0.0 ) {
		*hue+=1.0;
	}

	return;
}


//
//
//
void Hsb2Rgb( FLOAT hue, FLOAT saturation, FLOAT brightness, int *red,int *green,int *blue ) {

	if( saturation == 0.0 ) {
		*red=(int)(255.0*brightness);
		*green=(*red);
		*blue=(*red);
		return;
	}

	FLOAT h = 6.0*(hue-floor(hue));
	FLOAT f = h-floor((FLOAT) h);
	FLOAT p = brightness*(1.0-saturation);
	FLOAT q = brightness*(1.0-saturation*f);
	FLOAT t = brightness*(1.0-(saturation*(1.0-f)));

	switch( (int) h ) {
		case 0:
		default:
			*red=RoundToByte(255.0*brightness);
			*green=RoundToByte(255.0*t);
			*blue=RoundToByte(255.0*p);
			break;
		case 1:
			*red=RoundToByte(255.0*q);
			*green=RoundToByte(255.0*brightness);
			*blue=RoundToByte(255.0*p);
			break;
		case 2:
			*red=RoundToByte(255.0*p);
			*green=RoundToByte(255.0*brightness);
			*blue=RoundToByte(255.0*t);
			break;
		case 3:
			*red=RoundToByte(255.0*p);
			*green=RoundToByte(255.0*q);
			*blue=RoundToByte(255.0*brightness);
			break;
		case 4:
			*red=RoundToByte(255.0*t);
			*green=RoundToByte(255.0*p);
			*blue=RoundToByte(255.0*brightness);
			break;
		case 5:
			*red=RoundToByte(255.0*brightness);
			*green=RoundToByte(255.0*p);
			*blue=RoundToByte(255.0*q);
			break;
	}

	return;
}





//
//
//
void GammaSprite( UCHAR *ptr, UCHAR *pal ) {

	int x,y,w,h,word,red,green,blue,dr,dg,db,dm;
	UCHAR *spr,*pspr,*ppal;
	// int gamma = (int)(100.0f * gamma_corr.value / 5.0f);

	spr = ptr;

	if( gamma_corr.value == 100.0f )
		return;

	FLOAT gamma = gamma_corr.value / 100.0f;

	switch( SPRPIXELLEN( spr ) ) {

		case 1:

			// FIXME: paletta

			if( pal == NULL ) {
				xprintf("GammaSprite: need palette for 8 bit sprites.\n" );
				return;
			}

			ppal = pal;

			for( x=0; x<256; x++ ) {

				red   = ppal[0];
				green = ppal[1];
				blue  = ppal[2];

				if( !((red == 0) && (green == 0) && (blue == 0)) ) {

					red   = ftoi((FLOAT)red * gamma);
					green = ftoi((FLOAT)green * gamma);
					blue  = ftoi((FLOAT)blue * gamma);

					CLAMPMINMAX( red, 0, 255 );
					CLAMPMINMAX( green, 0, 255 );
					CLAMPMINMAX( blue, 0, 255 );

					dr = red - 63;
					dg = green - 63;
					db = blue - 63;

					dm = MAX3(dr,dg,db);

					if( dm > 0 ) {
						red -= dm;
						green -= dm;
						blue -= dm;
					}
				}

				ppal[0] = red;
				ppal[1] = green;
				ppal[2] = blue;

				ppal += 3;
			}

			break;


		case 2:

			w = SPRITEW(spr);
			h = SPRITEH(spr);

			pspr = &spr[SPRITEHEADER];

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				word = *(USHORT*)pspr;

				red   = (UCHAR)INTRED16(word) << (8-rsize);
				green = (UCHAR)INTGREEN16(word) << (8-gsize);
				blue  = (UCHAR)INTBLUE16(word) << (8-bsize);

				if( !((red < 4) && (green < 7) && (blue < 4)) ) {

					red   = ftoi((FLOAT)red * gamma);
					green = ftoi((FLOAT)green * gamma);
					blue  = ftoi((FLOAT)blue * gamma);

			// TODO: a colorkey miatt ne legyen 11-mél kisebb ha nem volt azelőtt se
			// illetve ne legyen nagyobb se

					CLAMPMINMAX( red, 0, 255 );
					CLAMPMINMAX( green, 0, 255 );
					CLAMPMINMAX( blue, 0, 255 );

					dr = red - 255;
					dg = green - 255;
					db = blue - 255;

					dm = MAX3(dr,dg,db);

					if( dm > 0 ) {
						red -= dm;
						green -= dm;
						blue -= dm;
					}
				}

				word  = (((ULONG)(red)   >> (8-rsize)) & rmask) << rshift;
				word |= (((ULONG)(green) >> (8-gsize)) & gmask) << gshift;
				word |= (((ULONG)(blue)  >> (8-bsize)) & bmask) << bshift;

				*pspr++ = LOBYTE(word);
				*pspr++ = HIBYTE(word);
			}

			break;

		case 3:
		case 4: {

			w = SPRITEW(spr);
			h = SPRITEH(spr);

			/***
			pspr = &spr[SPRITEHEADER];
			// find avrage color of image
			rgb_t avg = {0,0,0,0};
			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				avg.r += pspr[0];
				avg.g += pspr[1];
				avg.b += pspr[2];

				pspr += 3;

				// RGBA -> alpha
				if( SPRPIXELLEN( spr ) == 4 )
					pspr += 1;
			}
			avg.r /= (w*h);
			avg.g /= (w*h);
			avg.b /= (w*h);
			// xprintf("avg: %d, %d, %d\n",avg.r,avg.g,avg.b);
			***/

			pspr = &spr[SPRITEHEADER];

			for( y=0; y<h; y++ )
			for( x=0; x<w; x++ ) {

				red   = pspr[0];
				green = pspr[1];
				blue  = pspr[2];

				if( !((red == 0) && (green == 0) && (blue == 0)) ) {

					// http://www.gamedev.net/topic/313181-adjust-contrast-rgb-image/
					FLOAT brightness = 0.0; // ranging from 0.0 to 1.0
					FLOAT hue = 0.0; // ranging from 0.0 to 1.0 (actually not a good representation)
					FLOAT saturation = 0.0; // ranging from 0.0 to 1.0
					Rgb2Hsb( red, green, blue, &hue, &saturation, &brightness );
					// brightness += 0.5 * contrast * (0.5*(sin(M_PI*(brightness-0.5))+1.0)-brightness);
					brightness *= gamma;
					Hsb2Rgb( hue, saturation, brightness, &red,&green,&blue );

					/***
					// What I did for contrast in an application I was working on
					// (not exactly images, but stored and manipulated as images) was
					// to first find the average color of the image, then for each pixel
					// set R = (R - AverageR) * Contrast + AverageR, G = ...
					FLOAT contrast = 1 + (gamma_corr.value-100) / 200.0f;
					red = (red-avg.r) * contrast + avg.r;
					green = (green-avg.g) * contrast + avg.g;
					blue = (blue-avg.b) * contrast + avg.b;
					***/

					/***
					red   = ftoi((FLOAT)red * gamma);
					green = ftoi((FLOAT)green * gamma);
					blue  = ftoi((FLOAT)blue * gamma);
					***/

					constrain_rgb( &red, &green, &blue );

			// TODO: a colorkey miatt ne legyen 11-mél kisebb ha nem volt azelőtt se
			// illetve ne legyen nagyobb se

					CLAMPMINMAX( red, 11, 255 );
					CLAMPMINMAX( green, 11, 255 );
					CLAMPMINMAX( blue, 11, 255 );
				}

				pspr[0] = red;
				pspr[1] = green;
				pspr[2] = blue;

				pspr += 3;

				// RGBA -> alpha
				if( SPRPIXELLEN( spr ) == 4 )
					pspr += 1;
			}

			break;
		}
	}

	// xprintf( "gamma corrected " );

	return;
}



//
//
//
void GreySprite( UCHAR *spr ) {

	UCHAR *ptr;

	if( !ISSPR24( spr ) )
		return;

	ptr = &spr[SPRITEHEADER];

	for( int y=0; y<SPRITEH(spr); y++ )
	for( int x=0; x<SPRITEW(spr); x++ ) {

		/***
		sum  = *(ptr+0);
		sum += *(ptr+1);
		sum += *(ptr+2);

		sum <<= 4;
		sum /= 3;
		sum >>= 4;
		***/

		int sum = Rgb2Gray( *(ptr+0), *(ptr+1), *(ptr+2) );

		int r = 255;
		int g = 255;
		int b = 255;

		//if( sum > MIN3(r,g,b) )
		  //	sum = MIN3(r,g,b);

		*(ptr+0) = MIN( sum, r );
		*(ptr+1) = MIN( sum, g );
		*(ptr+2) = MIN( sum, b );

		ptr += 3;
	}

	return;
}




//
//
//
int NumSpriteFrame( UCHAR *spr ) {

	int num,len;
	UCHAR *ptr;

	if( spr == NULL )
		return 0;

	num = 1;
	ptr = spr;

	while( 1 ) {

		len = SPRITESIZE( ptr );

		if( !ISANIMSPR( ptr ) )
			return num;

		num += 1;
		ptr += len;
	}

	return 0;
}






//
//
//
void AddAnimFrame( memptr_ptr animsprptr, UCHAR *frame ) {

	UCHAR *animspr;
	int num;

	animspr = (UCHAR*)(*animsprptr);
	num = 1;




	return;
}





//
//
//
void FillSprite( UCHAR *spr, UCHAR *pal, rgb_t rgb ) {

	UCHAR *ptr;

	if( spr == NULL )
		return;

	switch( SPRPIXELLEN( spr ) ) {

		case 1:
			if( pal == NULL ) {
				xprintf("FillSprite: pal == NULL.\n" );
				return;
			}

			xprintf( "FillSprite: NYI for 8 bit sprites.\n");

			break;

		case 2:

			xprintf( "FillSprite: NYI for 16 bit sprites.\n");

			break;

		case 3:

			int i,w,h;

			w = SPRITEW(spr);
			h = SPRITEH(spr);

			ptr = &spr[SPRITEHEADER];

			for( i=0; i<w*h; i++ ) {
				*ptr++ = rgb.r;
				*ptr++ = rgb.g;
				*ptr++ = rgb.b;
			}

			break;
	}

	return;
}




//
// [x1, y1] [x2, y2] INCLUSIVE
//
void FillSpriteRect( UCHAR *spr, UCHAR *pal, int x1,int y1, int x2, int y2, rgb_t rgb ) {

	UCHAR *ptr;

	if( spr == NULL )
		return;

	switch( SPRPIXELLEN( spr ) ) {

		case 1:
			if( pal == NULL ) {
				xprintf("FillSpriteRect: pal == NULL.\n" );
				return;
			}

			xprintf( "FillSpriteRect: NYI for 8 bit sprites.\n");

			break;

		case 2:

			xprintf( "FillSpriteRect: NYI for 16 bit sprites.\n");

			break;

		case 3:

			int w,h,x,y;

			w = SPRITEW(spr);
			h = SPRITEH(spr);

			for( y=y1; y<=y2; y++ )
			for( x=x1; x<=x2; x++ ) {

				ptr = &spr[SPRITEHEADER + 3*(y*w + x)];

				ptr[0] = rgb.r;
				ptr[1] = rgb.g;
				ptr[2] = rgb.b;
			}

			break;
	}

	return;
}



//
//
//
void CompositeSprite( UCHAR *dst, UCHAR *src, int x, int y ) {

	if( !dst || !src )
		return;

	if( SPRPIXELLEN(dst)<3 || SPRPIXELLEN(src)<3 )
		return;

	UCHAR *p = &src[SPRITEHEADER];

	for( int i=0; i<SPRITEH(src); i++ )
	for( int j=0; j<SPRITEW(src); j++ ) {
		int r = *p++;
		int g = *p++;
		int b = *p++;
		// if( SPRPIXELLEN(src) == 3 && (r<11) && (g<11) && (b<11) )
		//	continue;

		int a = 255;
		if( SPRPIXELLEN(src) == 4 ) {
			a = *p++;
			// if( a < 127 )
			//	continue;
		}

		int dst_x = x + j;
		int dst_y = y + i;

		if( (dst_x<0) || (dst_x>=SPRITEW(dst)) ||
		    (dst_y<0) || (dst_y>=SPRITEH(dst)) )
			continue;

		UCHAR *s = SPRPTR(dst,dst_x,dst_y);
		*s++ = r;
		*s++ = g;
		*s++ = b;

		if( SPRPIXELLEN(dst) == 4 )
			*s++ = a;
	}

	return;
}





//
//
//
void CheckerTexture( UCHAR **orig_spr, int width, int height ) {

	UCHAR *spr,pal[768];
	int x,y,col,u,v,w,h;

	ALLOCMEM( *orig_spr, SPRITEHEADER + (width*height) );
	memset( *orig_spr, 0L, SPRITEHEADER + (width*height) );
	MKSPRW( *orig_spr, width );
	MKSPRH( *orig_spr, height );
	MKSPR8( *orig_spr );

	spr = *orig_spr;

	memset( pal, 0L, 768 );

	pal[0] = 20;
	pal[1] = 20;
	pal[2] = 20;

	pal[3] = 230;
	pal[4] = 230;
	pal[5] = 230;

	w = width / 4;
	h = height / 4;

	for( y=0; y<height; y++ )
	for( x=0; x<width; x++ ) {

		u = ((int)((y-(y%h)) / h)) & 1;
		v = ((int)((x-(x%w)) / w)) & 1;

		col = ( u ^ v ) ? 1 : 0;

		SPR(spr,x,y) = 	col;
	}

	HighSprite( orig_spr, pal );

	return;
}





//
//
//
int AddCheckerTexmap( void ) {

	int flag,color;
	UCHAR pal[768],*spr;
	texmap_t *tex;
	int texid;
	char filename[XMAX_PATH];
	char *FXNAME = "INVALIDTEX_CHECKER_TEXTURE1";

	if( (texid = TexNumForName( FXNAME )) != (-1) )
		return texid;

	CheckerTexture( &spr, 256,256 );

	flag = 0L;
	color = 0L;

	AddTexMapHigh( spr, pal, FXNAME, FXNAME, flag, color );

	if( (texid = TexNumForName( FXNAME )) == (-1) ) {
		FREEMEM( spr );
		xprintf("AddCheckerTexmap: no \"%s\" texture.", FXNAME );
		return texid;
	}

	tex = TexForNum( texid );

	int w = tex->width1;
	int h = tex->height1;

	FREEMEM( spr );

	return texid;
}




//
//
//
void ColorTexture( UCHAR **orig_spr, int width, int height, int r, int g, int b ) {

	UCHAR *spr,pal[768];
	int x,y,col,w,h;

	ALLOCMEM( *orig_spr, SPRITEHEADER + (width*height)*3 );
	memset( *orig_spr, 0L, SPRITEHEADER + (width*height)*3 );
	MKSPRW( *orig_spr, width );
	MKSPRH( *orig_spr, height );
	MKSPR24( *orig_spr );

	spr = &(*orig_spr)[SPRITEHEADER];

	memset( pal, 0L, 768 );

	pal[0] = 20;
	pal[1] = 20;
	pal[2] = 20;

	pal[3] = 230;
	pal[4] = 230;
	pal[5] = 230;

	for( y=0; y<height; y++ )
	for( x=0; x<width; x++ ) {

		spr[0] = r;
		spr[1] = g;
		spr[2] = b;

		spr += 3;
	}

	HighSprite( orig_spr, pal );

	return;
}




//
//
//
void DrawChecker( void ) {

	int load_texmapid;
	char *FXNAME = "BKG_CHECKER_TEXTURE1";
	polygon_t poly;
	point_t point[4];
	point3_t fekete_st[4] = { {0.0f,0.0f,0.0f},
				{0.0f,255.0f,0.0f},
				{31.0f,255.0f,0.0f},
				{31.0f,0.0f,0.0f} };
	rgb_t rgb[4] = { {255,255,255,255},
			{255,255,255,255},
			{255,255,255,255},
			{255,255,255,255} };

	if( (load_texmapid = TexNumForName( FXNAME )) == (-1) ) {

		UCHAR *spr, pal[768];
		int flag,color;
		rgb_t rgb;

		xprintf("DrawChecker: creating \"%s\" texture...\n", FXNAME );

		ALLOCMEM( spr, SPRITEHEADER + (64*64*3) );
		memset( spr, 0L, SPRITEHEADER + (64*64*3) );
		MKSPRW( spr, 64 );
		MKSPRH( spr, 64 );
		MKSPR24( spr );

		rgb.r = 255;
		rgb.g = 255;
		rgb.b = 255;
		rgb.a = 255;

		memset( pal, 0L, 768 );

		FillSprite( spr, pal, rgb );

		HighSprite( PTR(spr), pal );

		flag = 0L;
		color = 0L;

		// SETFLAG( flag, TF_CHROMA );
		// SETFLAG( flag, TF_ALPHA );

		AddTexMapHigh( spr, pal, FXNAME, FXNAME, flag, color );

		FREEMEM( spr );

		if( (load_texmapid = TexNumForName( FXNAME )) == (-1) ) {
			xprintf("can't.\n" );
			return;
		}

		xprintf("done.\n");
	}

	poly.npoints = 4;
	poly.point = point;

	int a,b,x,y;
	int c,i;

	a = b = 1;

#define STEP (16*3)

	for( x=0; x<SCREENW; x += STEP ) {

		b ^= 1;
		a = b;

		for( y=0; y<SCREENH; y += STEP ) {

			a ^= 1;

			c = 25-a*15;

			for( i=0; i<4; i++ ) {
				rgb[i].r = c;
				rgb[i].g = c;
				rgb[i].b = c;
				rgb[i].a = 255;
			}

			point[0][0] = x;	point[0][1] = y;
			point[1][0] = x;	point[1][1] = y + STEP;
			point[2][0] = x+STEP;	point[2][1] = y + STEP;
			point[3][0] = x+STEP;	point[3][1] = y;

			PutSpritePoly( poly, fekete_st, load_texmapid, rgb );
		}
	}

#undef STEP

	return;
}





static	point3_t fekete_st[4] = {
	{0.0f,0.0f,0.0f},
	{31.0f,0.0f,0.0f},
	{31.0f,127.0f,0.0f},
	{0.0f,127.0f,0.0f},
};

static	point3_t feher_st[4] = {
	{32.0f,0.0f,0.0f},
	{62.0f,0.0f,0.0f},
	{62.0f,127.0f,0.0f},
	{32.0f,127.0f,0.0f},
};

static	point3_t red_st[4] = {
	{64.0f,0.0f,0.0f},
	{95.0f,0.0f,0.0f},
	{95.0f,127.0f,0.0f},
	{64.0f,127.0f,0.0f},
};

static	point3_t green_st[4] = {
	{96.0f,0.0f,0.0f},
	{126.0f,0.0f,0.0f},
	{126.0f,127.0f,0.0f},
	{96.0f,127.0f,0.0f},
};

static	point3_t blue_st[4] = {
	{128.0f,0.0f,0.0f},
	{159.0f,0.0f,0.0f},
	{159.0f,127.0f,0.0f},
	{128.0f,127.0f,0.0f},
};

static	point3_t dark_blue_st[4] = {
	{161.0f,0.0f,0.0f},
	{191.0f,0.0f,0.0f},
	{191.0f,127.0f,0.0f},
	{161.0f,127.0f,0.0f},
};

static	point3_t darkest_blue_st[4] = {
	{193.0f,0.0f,0.0f},
	{223.0f,0.0f,0.0f},
	{223.0f,127.0f,0.0f},
	{193.0f,127.0f,0.0f},
};


static	point3_t full_st[4] = {
	{0.0f,0.0f,0.0f},
	{255.0f,0.0f,0.0f},
	{255.0f,127.0f,0.0f},
	{0.0f,127.0f,0.0f},
};

static rgb_t rgb[4] = {
	{255,255,255,255},
	{255,255,255,255},
	{255,255,255,255},
	{255,255,255,255},
};

static rgb_t spectrum_rgb[4] = {
	{255,0,0,255},
	{255,0,0,255},
	{0,255,0,255},
	{0,255,0,255},
};


#define TEXLINEDIM 64


//
//
//
static int CreateDrawTex( void ) {

	int load_texmapid = -1;
	char *FXNAME = "TEXLINE_TEXTURE1";
	int dx, dy;

	if( (load_texmapid = TexNumForName( FXNAME )) == (-1) ) {

		UCHAR *spr, pal[768], *p;
		int flag,color;
		rgb_t rgb;
		int w,h,i;

		xprintf("CreateDrawTex: creating \"%s\" texture...\n", FXNAME );

		w = TEXLINEDIM;
		h = TEXLINEDIM;

		ALLOCMEM( spr, SPRITEHEADER + (w*h*3) );
		memset( spr, 0L, SPRITEHEADER + (w*h*3) );
		MKSPRW( spr, w );
		MKSPRH( spr, h );
		MKSPR24( spr );

		memset( pal, 0L, 768 );

		p = &spr[SPRITEHEADER];

		for( i=0; i<w*h; i++ ) {
			*p++ = rand() % 255;	// rgb
			*p++ = rand() % 255;
			*p++ = rand() % 255;
		}

		// fekete
		rgb.r = 0;
		rgb.g = 0;
		rgb.b = 0;
		rgb.a = 0;

		FillSpriteRect( spr,pal, 0,0, TEXLINEDIM/8, (TEXLINEDIM/2)-1, rgb );

		// feher
		rgb.r = 255;
		rgb.g = 255;
		rgb.b = 255;
		rgb.a = 255;

		FillSpriteRect( spr,pal, TEXLINEDIM/8,0, (TEXLINEDIM/8)*2, (TEXLINEDIM/2)-1, rgb );

		// red
		rgb.r = 255;
		rgb.g = 0;
		rgb.b = 0;
		rgb.a = 0;

		FillSpriteRect( spr,pal, (TEXLINEDIM/8)*2,0, (TEXLINEDIM/8)*3, (TEXLINEDIM/2)-1, rgb );

		// green
		rgb.r = 0;
		rgb.g = 255;
		rgb.b = 0;
		rgb.a = 0;

		FillSpriteRect( spr,pal, (TEXLINEDIM/8)*3,0, (TEXLINEDIM/8)*4, (TEXLINEDIM/2)-1, rgb );

		// blue
		rgb.r = 0;
		rgb.g = 0;
		rgb.b = 255;
		rgb.a = 0;

		FillSpriteRect( spr,pal, (TEXLINEDIM/8)*4,0, (TEXLINEDIM/8)*5, (TEXLINEDIM/2)-1, rgb );

		// blue1
		rgb.r = 48;
		rgb.g = 100;
		rgb.b = 150;
		rgb.a = 255;

		FillSpriteRect( spr,pal, (TEXLINEDIM/8)*5,0, (TEXLINEDIM/8)*6, (TEXLINEDIM/2)-1, rgb );

		// blue2
		rgb.r = 26;
		rgb.g = 63;
		rgb.b = 107;
		rgb.a = 255;

		FillSpriteRect( spr,pal, (TEXLINEDIM/8)*6,0, (TEXLINEDIM/8)*7, (TEXLINEDIM/2)-1, rgb );

		// SavePcx( "drawtexline.pcx", spr, pal );

		HighSprite( PTR(spr), pal );

		flag = 0L;
		color = 0L;

		// SETFLAG( flag, TF_CHROMA );
		// SETFLAG( flag, TF_ALPHA );

		AddTexMapHigh( spr, pal, FXNAME, FXNAME, flag, color );

		FREEMEM( spr );

		if( (load_texmapid = TexNumForName( FXNAME )) == (-1) ) {
			xprintf("can't.\n" );
			return -1;
		}

		xprintf("done.\n");
	}

	return load_texmapid;
}




//
// ez Software render-rel nem mûködik
//
void DrawTexRect( int x1, int y1, int x2, int y2, int color, int alpha ) {

	polygon_t poly;
	point_t point[4];
	void *tex_st = NULL;
	int load_texmapid = -1;

	if( (load_texmapid = CreateDrawTex()) == (-1) ) {
		xprintf("DrawTexRect: WTF?!\n");
		return;
	}

	poly.npoints = 4;
	poly.point = point;

	// if( x2 < x1 ) { SWAPL( x1, x2); SWAPL( y1, y2); }
	// if( y2 < y1 ) { SWAPL( y1, y2); SWAPL( x1, x2); }

	// x2 = x1 + 50;
	// y2 = y1 + 50;

	point[0][0] = x1; point[0][1] = y1;	// 0
	point[1][0] = x2; point[1][1] = y1;	// 3
	point[2][0] = x2; point[2][1] = y2;	// 2
	point[3][0] = x1; point[3][1] = y2;	// 1

	switch( color ) {
		case 0: tex_st = fekete_st; break;
		case 1: tex_st = feher_st; break;
		case 2: tex_st = red_st; break;
		case 3: tex_st = green_st; break;
		case 4: tex_st = blue_st; break;
		case 5: break;
		case 6: tex_st = dark_blue_st; break;
		case 7: tex_st = darkest_blue_st; break;
		default: tex_st = feher_st; break;
	}

	if( alpha != rgb[0].a ) {
		rgb[0].a = spectrum_rgb[0].a = alpha;
		rgb[1].a = spectrum_rgb[1].a = alpha;
		rgb[2].a = spectrum_rgb[2].a = alpha;
		rgb[3].a = spectrum_rgb[3].a = alpha;
	}

	if( color == 5 )
		PutSpritePoly( poly, (point3_t *)tex_st, load_texmapid, spectrum_rgb );
	else
		PutSpritePoly( poly, (point3_t *)tex_st, load_texmapid, rgb );

	return;
}



//
// ez Software render-rel nem mûködik
//
void DrawTexLine( int line_x, int line_y, int line_x1, int line_y1, int color, int border, int alpha ) {

	polygon_t poly;
	point_t point[4];
	void *tex_st = NULL;
	int load_texmapid = -1;
	int dx,dy,adx,ady;

	if( (load_texmapid = CreateDrawTex()) == (-1) )
		return;

	poly.npoints = 4;
	poly.point = point;

	// border = DRAWTEXLINE_BORDER;

	if( line_x1 < line_x ) { SWAPL( line_x, line_x1); SWAPL( line_y, line_y1); border = -border; }
	if( line_y1 < line_y ) { SWAPL( line_y, line_y1); SWAPL( line_x, line_x1); border = -border; }

	if( line_x1 < 0 ) return;
	if( line_y1 < 0 ) return;

	dx = line_x1 - line_x;
	dy = line_y1 - line_y;

	adx = ABS(dx);
	ady = ABS(dy);

	if( adx > ady ) {
		point[0][0] = line_x;	point[0][1] = line_y;			// 0
		point[1][0] = line_x1;	point[1][1] = line_y1;			// 3
		point[2][0] = line_x1;	point[2][1] = line_y1 + border;		// 2
		point[3][0] = line_x;	point[3][1] = line_y + border;		// 1
	}
	else {
		point[0][0] = line_x;		point[0][1] = line_y;		// 0
		point[1][0] = line_x +border;	point[1][1] = line_y;		// 3
		point[2][0] = line_x1 +border;	point[2][1] = line_y1;		// 2
		point[3][0] = line_x1;		point[3][1] = line_y1;		// 1
	}

	switch( color ) {
		case 0: tex_st = fekete_st; break;
		case 1: tex_st = feher_st;  break;
		case 2: tex_st = red_st;  break;
		case 3: tex_st = green_st;  break;
		case 4: tex_st = blue_st;  break;
		default: tex_st = feher_st;  break;
	}

	if( color == 5 ) {

		static rgb_t rgb[4] = {
			{255,0,0,155},
			{255,0,0,155},
			{0,255,0,155},
			{0,255,0,155},
		};
		/*
		rgb[0].r = rgb[1].r = (spectrum_rgb[0].r * ABS(line_y-line_y1) ) / SCREENH;
		rgb[0].g = rgb[1].g = (spectrum_rgb[0].g * ABS(line_y-line_y1) ) / SCREENH;

		rgb[2].r = rgb[3].r = spectrum_rgb[2].r;
		rgb[2].g = rgb[3].g = spectrum_rgb[2].g;
		*/
		PutSpritePoly( poly, (point3_t *)tex_st, load_texmapid, rgb );
	}
	else
		PutSpritePoly( poly, (point3_t *)tex_st, load_texmapid, rgb );

	return;
}







/************************
 *
 * BigSprite
 *
 ************************/


// #define TILEW 256;
// #define TILEH 256


typedef enum {

	ST_PICTURE = 0,
	ST_ANIM,

} spritetype_e;


typedef struct bigsprite_s {

	int handle;
	spritetype_e type;
	char filename[XMAX_PATH];

	int width,height;		// az origw, origh
	int tilew,tileh;		// ennyi tileból áll
	int bpp;
	int pages;			// anim .GIF
	int tic;			// animhoz
	int delay;			// frameTime
	UCHAR *spr;			// az original sprite

	int numtexmapids;
	int *texmapids;			// a tileok texmapid-je
	UCHAR **working;		// a szétdarabolt sprite
	int curr_page;			// ami fel van töltve a membe, avagy ami kinn van a képernyőn
	int *tex_flag;			// amibe belelóg háttér

	BOOL isAlpha;
	BOOL isChroma;
	int alpha;
	FLOAT fok;			// forgatás ennyi fokkal

	int TILEW;
	int TILEH;

	rgb_t avg;			// avarage color kontraszthoz

} bigsprite_t, *bigsprite_ptr;



static bigsprite_t *bigsprites = NULL;
static int numbigsprites = 0;

static int id_cnt = 1213;

#define BIGSPRITE bigsprites[numbigsprites]

#define PAGEONSCREEN (TICKBASE / 10)


//
//
//
static void CopySpritePart( UCHAR *hova, UCHAR *honnan, int honnanw, int honnanh, int bpp, int startx,int starty,int width, int height, BOOL munge=FALSE, BOOL force_keret = FALSE ) {

	int x,y,x0,y0,i;
	UCHAR *black,*notblack;
	BOOL bMunged = FALSE;
	BOOL bit24 = FALSE;
	int mincolor;

	// if( munge && bpp != 3 ) xprintf("CopySpritePart: munge inacurate when bpp=%d.\n", bpp );

	ALLOCMEM( black, bpp );
	memset( black, 0L, bpp );

	// 16 biten fenntebb van a chroma key
	// 24-tõl 1-est is érzékeli

	if( GFXDLL_GetData ) {
		bit24 = GFXDLL_TEX24BIT;
		GFXDLL_GetData( &bit24, sizeof(BOOL) );
	}

	if( bit24 == TRUE )
		mincolor = 1L;
	else
		mincolor = 8L;		// 8-bitrõl 5-re már jó

	ALLOCMEM( notblack, bpp );
	memset( notblack, mincolor, bpp );
	if( bpp >= 4 )
		notblack[3] = 255;

	// xprintf("bpp: %d, bit24 = %d, mincolor = %d\n", bpp, bit24, mincolor );

	for( y=starty,y0=0; y<starty+height; y++,y0++ )
	for( x=startx,x0=0; x<startx+width; x++,x0++ ) {

		UCHAR *p;

		if( (x < honnanw) && (y < honnanh) ) {

			p = &honnan[ (y*honnanw + x)*bpp ];

			// fekete-e a pixel
			if( munge ) {
				if( bpp == 3 && p[0] < mincolor && p[1] < mincolor && p[2] < mincolor ) {
					p = notblack;
					bMunged = TRUE;
				}
			}

		}
		else {
			// zöld lesz a kiegészítés
			// black[1] = x0;

			p = black;
		}

		// keret a darabok köré
		UCHAR keret[4] = { 200,200,200, 255 };

		if( force_keret &&
		    ((x0 < 5) || (x0 > (SPRITEW(hova) - 5)) ||
		     (y0 < 5) || (y0 > (SPRITEH(hova) - 5))) )
		    p = keret;

		memcpy( &hova[ SPRITEHEADER + (y0*SPRITEW(hova) + x0)*bpp ], p, bpp );
	}

	FREEMEM( black );
	FREEMEM( notblack );

	// if( bMunged == TRUE ) xprintf("CopySpritePart: munged good.\n" );

	return;
}




//
//
//
BOOL Collision( int x, int y, int w, int h, int x1, int y1, int w1, int h1 ) {
	return
		y + h > y1  	&&
		y <= y1 + h1 	&&
		x + w > x1 	&&
		x <= x1 + w1;
}




//
// SDL pixel perfect collision test
//
BOOL PixelCollide( int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh ) {

	/*Box A;
	Box B;*/

	/*a - bottom right co-ordinates*/
	int ax1 = ax + aw - 1;
	int ay1 = ay + ah - 1;

	/*b - bottom right co-ordinates*/
	int bx1 = bx + bw - 1;
	int by1 = by + bh - 1;

	/*check if bounding boxes intersect*/
	if( (bx1 < ax) || (ax1 < bx) )
		return FALSE;
	if( (by1 < ay) || (ay1 < by) )
		return FALSE;

	/*Now lets make the bouding box for which we check for a pixel collision*/

	/*To get the bounding box we do
	    Ax1,Ay1_____________
		|		|
		|		|
		|		|
		|    Bx1,By1_____________
		|	|	|	|
		|	|	|	|
		|_______|_______|	|
			|    Ax2,Ay2	|
			|		|
			|		|
			|____________Bx2,By2

	To find that overlap we find the biggest left hand cordinate
	AND the smallest right hand co-ordinate

	To find it for y we do the biggest top y value
	AND the smallest bottom y value

	Therefore the overlap here is Bx1,By1 --> Ax2,Ay2

	Remember	Ax2 = Ax1 + SA->w
			Bx2 = Bx1 + SB->w

			Ay2 = Ay1 + SA->h
			By2 = By1 + SB->h
	*/

	/*now we loop round every pixel in area of
	intersection
		if 2 pixels alpha values on 2 surfaces at the
		same place != 0 then we have a collision*/
	int inter_x0 = MAX(ax,bx);
	int inter_x1 = MIN(ax1,bx1);

	int inter_y0 = MAX(ay,by);
	int inter_y1 = MIN(ay1,by1);

	for( int y=inter_y0; y<=inter_y1; y++ ) {
		for( int x=inter_x0; x<=inter_x1 ; x++ ) {
			// SDL_TransparentPixel: returns 1 if offsets are a transparent pixel
			// if( SDL_TransparentPixel(as , x-ax , y-ay) && /*compute offsets for surface*/
			//    SDL_TransparentPixel(bs , x-bx , y-by) )/*before pass to SDL_TransparentPixel*/
			//	return TRUE;
		}
	}

	// ha van transparens check akkor FALSE
	return TRUE;
}



//
// return: 0 == fail
//	   0< == okés
//
int LoadBigSprite( char *in_filename, BOOL alpha, BOOL chroma, BOOL force_reload, int load_flag, int cut_x, int cut_y, int cut_w, int cut_h, FLOAT fok, BOOL fromSprite ) {

	UCHAR *spr=NULL,*working=NULL;
	UCHAR pal[768];
	int flag = 0L;
	int color = 0L;
	int x,y,i,origbpp,delay;
	char filename[XMAX_PATH];

	if( in_filename == NULL )
		return 0;

	if( fromSprite == TRUE ) {
		sprintf( filename, "fromSprite_%d", id_cnt);
	}
	else
	if( load_flag == BIGSPR_NOFLAG )
		strcpy( filename, in_filename );
	else
		sprintf( filename, "%s_%d", in_filename, load_flag );

	// be van-e már töltve
	for( i=0; i<numbigsprites; i++ )
		if( !stricmp( bigsprites[i].filename, filename ) )
			break;

	// igen
	if( i < numbigsprites ) {

		if( force_reload == FALSE ) {
			// xprintf("LoadBigSprite: already loaded \"%s\" (id = %d, idx = %d).\n",filename, bigsprites[i].handle, i );
			return bigsprites[i].handle;
		}

		FreeBigSprite( bigsprites[i].handle );
	}

	xprintf("LoadBigSprite: loading \"%s\" (alpha: %s, chroma: %s, flag = %d).\n",filename, alpha?"On":"Off",chroma?"On":"Off", load_flag );

	if( fromSprite ) {
		if( !in_filename )
			return 0;
		DupSprite( (UCHAR *)in_filename, PTR(spr) );
		origbpp = SPRPIXELLEN(in_filename) * 8;
		delay = 100;
	}
	else
	if( LoadPicture( in_filename, PTR(spr), &x,&y,&origbpp, &delay, TRUE ) == FALSE ) {
		xprintf("LoadBigSprite: can't load %s.\n",filename );
		return 0;
	}

	GetRgbPicture( pal );

	// TrueSprite( PTR(spr), pal );		// ez ne kelljen
	int pixellen = SPRPIXELLEN(spr);

	// flip left - right
	if( load_flag & BIGSPR_FLIPHOR ) {

		xprintf("LoadBigSprite: flipping left to right.\n" );

		for( int y=0; y<SPRITEH(spr); y++ )
		for( int x=0; x<SPRITEW(spr)/2; x++ ) {
			int r = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 0 ];
			int g = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 1 ];
			int b = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 2 ];

			spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 0 ] = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 0 ];
			spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 1 ] = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 1 ];
			spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 2 ] = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 2 ];

			spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 0 ] = r;
			spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 1 ] = g;
			spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 2 ] = b;

			if( pixellen==4 ) {
				int a = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 3 ];
				spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + x * pixellen + 3 ] = spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 3 ];
				spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen + ((SPRITEW(spr)-1) - x) * pixellen + 3 ] = a;
			}
		}
	}

	// flip up - down
	if( load_flag & BIGSPR_FLIPVERT ) {

		xprintf("LoadBigSprite: flipping up and down.\n" );

		for( int y=0; y<SPRITEH(spr); y++ )
			memmove( &spr[SPRITEHEADER + y * SPRITEW(spr) * pixellen ], &spr[SPRITEHEADER + ((SPRITEH(spr)-1) - y) * SPRITEW(spr) * pixellen ], SPRITEW(spr) );

	}

	// a képnek csak bizonyos része érdekel
	if( (SPRITEF(spr) == 1) &&
	    (SPRPIXELLEN(spr) == 3 || SPRPIXELLEN(spr) == 4) &&
	     cut_x > 0 && cut_y > 0 &&				// default: -1, mi legyen -100,-100 vágsokkal
	     cut_w > 0 && cut_w <= SPRITEW(spr) &&
	     cut_h > 0 && cut_h <= SPRITEH(spr) ) {		// hogy ne kelljen reallokálni

		int x,y;
		UCHAR *tmp,*p;

		ALLOCMEM( tmp, cut_w * cut_h * 3 );
		p = tmp;

		for( y=0; y<cut_h; y++ )
		for( x=0; x<cut_w; x++ ) {

			int sx = cut_x + x;
			int sy = cut_y + y;

			if( sy > 0 && sy < SPRITEH(spr) &&
			    sx > 0 && sy < SPRITEW(spr) ) {
				p[0] = spr[ SPRITEHEADER + sy*SPRITEW(spr)*pixellen + sx*pixellen + 0 ];
				p[1] = spr[ SPRITEHEADER + sy*SPRITEW(spr)*pixellen + sx*pixellen + 1 ];
				p[2] = spr[ SPRITEHEADER + sy*SPRITEW(spr)*pixellen + sx*pixellen + 2 ];
				if( pixellen == 4 )
					p[3] = spr[ SPRITEHEADER + sy*SPRITEW(spr)*pixellen + sx*pixellen + 3 ];
			}
			else {
				p[0] = 0;
				p[1] = 0;
				p[2] = 0;
				if( pixellen == 4 )
					p[3] = 0;
			}

			p += pixellen;
		}

		MKSPRW( spr, cut_w );
		MKSPRH( spr, cut_h );

		memcpy( &spr[SPRITEHEADER], p, cut_w * cut_h * pixellen );

		FREEMEM( tmp );
	}

	REALLOCMEM( bigsprites, (numbigsprites+1) * sizeof(bigsprite_t) );

	BIGSPRITE.handle = id_cnt++;
	strcpy( BIGSPRITE.filename, filename );

	BIGSPRITE.width = SPRITEW(spr);
	BIGSPRITE.height = SPRITEH(spr);
	BIGSPRITE.bpp = origbpp;
	BIGSPRITE.pages = ISANIMSPR(spr) ? ( (SPRITEF(spr) > 1) ? SPRITEF(spr) : 1 ) : 1;
	BIGSPRITE.curr_page = 0;
	BIGSPRITE.tic = -1;
	BIGSPRITE.delay = (delay == -1) ? PAGEONSCREEN : delay;

	DupSprite( spr, &BIGSPRITE.spr );

	BIGSPRITE.isAlpha = alpha;
	BIGSPRITE.isChroma = chroma;

	BIGSPRITE.fok = fok;

	BIGSPRITE.type = (BIGSPRITE.pages > 1) ? ST_ANIM : ST_PICTURE;

	int TILEW = SPRITEW(spr);
	int TILEH = SPRITEH(spr);

	// kettõ hatványai legyen a w és h
	if( TILEW > TILEH ) {
		TILEW = (int)powf( 2, ceil( log(TILEW) / log(2) ) );
		TILEH = TILEW;
	}
	else {
		TILEH = (int)powf( 2, ceil( log(TILEH) / log(2) ) );
		TILEW = TILEH;
	}

	int maxTexSize = GFXDLL_MAXTEXSIZE;
	if( GFXDLL_GetData ) GFXDLL_GetData( &maxTexSize, sizeof(int) );
	else maxTexSize = 256;

	int minTexSize = GFXDLL_MINTEXSIZE;
	if( GFXDLL_GetData ) GFXDLL_GetData( &minTexSize, sizeof(int) );
	else minTexSize = 64;

	CLAMPMAX( maxTexSize, 1024 );

	CLAMPMINMAX( TILEW, minTexSize, maxTexSize );
	TILEH = TILEW;

	BIGSPRITE.TILEW = TILEW;
	BIGSPRITE.TILEH = TILEH;

	BIGSPRITE.tilew = BIGSPRITE.width / TILEW;
	if( BIGSPRITE.width % TILEW )	// be fog-e lógni
		++BIGSPRITE.tilew;

	BIGSPRITE.tileh = BIGSPRITE.height / TILEH;
	if( BIGSPRITE.height % TILEH )	// be fog-e lógni
		++BIGSPRITE.tileh;

	// Grafikus módba van-e
	if( GFXDLL_GetData ) {
		BIGSPRITE.numtexmapids = BIGSPRITE.tilew * BIGSPRITE.tileh;
		ALLOCMEM( BIGSPRITE.texmapids, BIGSPRITE.numtexmapids * sizeof(int) );
	}
	else {
		BIGSPRITE.numtexmapids = 0;
		BIGSPRITE.texmapids = NULL;
	}

#define WORKING BIGSPRITE.working[texid_offset]

	ALLOCMEM( BIGSPRITE.working, BIGSPRITE.tilew * BIGSPRITE.tileh * BIGSPRITE.pages * sizeof(UCHAR *) );
	ALLOCMEM( BIGSPRITE.tex_flag, BIGSPRITE.tilew * BIGSPRITE.tileh * BIGSPRITE.pages * sizeof(int) );

	// Grafikus módba van-e
	for( int texid_offset=0,i=0; GFXDLL_GetData && i<BIGSPRITE.pages; i++ ) {

		if( BIGSPRITE.pages > 1 )
			winSetPanel(NULL,(BIGSPRITE.pages+1)*2+i+1);

		///////////////////////////////////////// int texid_offset = BIGSPRITE.tilew * BIGSPRITE.tileh * i;
		UCHAR *spr_page = &spr[ SPRITEHEADER + SPRITEW(spr) * SPRITEH(spr) * pixellen * i];

		for( y=0; y<BIGSPRITE.tileh; y++ )
		for( x=0; x<BIGSPRITE.tilew; x++, texid_offset++ ) {

			ALLOCMEM( WORKING, (TILEW*TILEH*pixellen)+SPRITEHEADER );
			memset( WORKING, 0, (TILEW*TILEH*pixellen)+SPRITEHEADER );
			MKSPRW( WORKING, TILEW);
			MKSPRH( WORKING, TILEH);
			if( ISSPR24(spr) ) MKSPR24( WORKING );
			if( ISSPR32(spr) ) MKSPR32( WORKING );

			BOOL munge = FALSE;
			char extfilename[XMAX_PATH*2];

			// ha anim akkor más neve lesz
			if( BIGSPRITE.pages > 1 )
				sprintf( extfilename, "%d_%dx%d_%d_%s", BIGSPRITE.handle, x+1, y+1, i+1, filename );
			else
				sprintf( extfilename, "%d_%dx%d_%s", BIGSPRITE.handle, x+1, y+1, filename );

			memset( WORKING + SPRITEHEADER, 0L, (TILEW*TILEH*pixellen) );

			flag = 0L;
			color = 0L;

			if( alpha ) SETFLAG( flag, TF_ALPHA );
			if( chroma ) SETFLAG( flag, TF_CHROMA );

			// ha kilóg vége akkor
			// ha chroma kell neki akkor nem kell munge
			if( (( (x+1)*TILEW > BIGSPRITE.width ) ||
			     ( (y+1)*TILEH > BIGSPRITE.height ) ) &&
			     chroma == FALSE ) {
				munge = TRUE;
				SETFLAG( flag, TF_CHROMA );
			}

			CopySpritePart( WORKING, spr_page, SPRITEW(spr), SPRITEH(spr), pixellen, x*TILEW, y*TILEH, TILEW, TILEH, munge );

			BIGSPRITE.tex_flag[texid_offset] = flag;

			// debug Save
			if( 0 ) {
				char extfilename[XMAX_PATH];
				sprintf( extfilename, "%d_%dx%d_%d_%s.jpg", BIGSPRITE.handle, x+1, y+1, i+1, GetFilenameNoPath(filename) );
				SaveFree( extfilename, WORKING );
			}

			// csak az első framet kell betölteni, a többi modify
			if( i == 0 )
				BIGSPRITE.texmapids[texid_offset] = AddTexMapHigh( WORKING, pal, extfilename, extfilename, flag, color );
		}
	}

#undef WORKING

	{
	// find average color of image
	UCHAR *pspr = &spr[SPRITEHEADER];
	double red = 0;
	double green = 0;
	double blue = 0;
	for( int y=0; y<BIGSPRITE.height; y++ )
	for( int x=0; x<BIGSPRITE.width; x++ ) {

		red += pspr[0];
		green += pspr[1];
		blue += pspr[2];

		pspr += 3;

		// RGBA -> alpha
		if( SPRPIXELLEN( spr ) == 4 )
			pspr += 1;
	}
	BIGSPRITE.avg.r = ftoi(red / (BIGSPRITE.width*BIGSPRITE.height));
	BIGSPRITE.avg.g = ftoi(green / (BIGSPRITE.width*BIGSPRITE.height));
	BIGSPRITE.avg.b = ftoi(blue / (BIGSPRITE.width*BIGSPRITE.height));
	// xprintf("avg: %d, %d, %d\n",BIGSPRITE.avg.r,BIGSPRITE.avg.g,BIGSPRITE.avg.b);
	}

	if( spr ) FREEMEM( spr );

	int handle = BIGSPRITE.handle;

	char anim_str[100] = "";

	if( BIGSPRITE.type == ST_ANIM )
		sprintf( anim_str," / %d pages (%d ms)", BIGSPRITE.pages, BIGSPRITE.delay );

	xprintf("LoadBigSprite: loaded [%d, %d  %d bits%s] (%d, %d) (tile size: %d x %d).\n", BIGSPRITE.width, BIGSPRITE.height,BIGSPRITE.bpp, anim_str, BIGSPRITE.tilew, BIGSPRITE.tileh,BIGSPRITE.TILEW, BIGSPRITE.TILEH );

	if( BIGSPRITE.pages > 1 )
		winHidePanel();

	++numbigsprites;

	return handle;
}



//
//
//
int ReloadBigSprite( int handle, int load_flag ) {

	int i;

#undef BIGSPRITE
#define BIGSPRITE bigsprites[i]

	for( i=0; i<numbigsprites; i++ )
		if( BIGSPRITE.handle == handle )
			break;

	if( i >= numbigsprites )
		// nincs ilyen
		return handle;

	for( int j=0; j<BIGSPRITE.tileh*BIGSPRITE.tilew; j++ )
		ReloadTexMap( BIGSPRITE.texmapids[j] );

	return BIGSPRITE.handle;
}



//
//
//
void PutBigSprite( int handle, int screenx, int screeny, int width, int height, int alpha, BOOL chroma ) {

	int i,j,x,y;
	polygon_t poly;
	rgb_t rgb[4];
	point_t point[4];
	point3_t st[4];

	// vicces de hátha
	if( alpha == 0 )
		return;

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return;

	poly.npoints = 4;
	poly.point = point;

	st[0][0] = 0.0f;	st[0][1] = 0.0f;
	st[1][0] = 0.0f;	st[1][1] = 255.0f;
	st[2][0] = 255.0f;	st[2][1] = 255.0f;
	st[3][0] = 255.0f;	st[3][1] = 0.0f;

	for( j=0; j<4; j++ ) {
		rgb[j].r = 255;
		rgb[j].g = 255;
		rgb[j].b = 255;
		rgb[j].a = alpha>0?alpha:255;
		CLAMPMAX( rgb[j].a, 255 );
	}

#undef BIGSPRITE
#define BIGSPRITE bigsprites[i]

	// ha anim
	if( BIGSPRITE.type == ST_ANIM && BIGSPRITE.pages > 1 ) {

		int tic = GetTic();

		if( BIGSPRITE.tic == -1 )
			BIGSPRITE.tic = tic;

		int page = ftoi( (FLOAT)((tic - BIGSPRITE.tic) % (BIGSPRITE.pages * BIGSPRITE.delay)) / (FLOAT)BIGSPRITE.delay );

		CLAMPMINMAX( page, 0, BIGSPRITE.pages-1 );

		if( BIGSPRITE.curr_page != page ) {

#define WORKING BIGSPRITE.working[ page * BIGSPRITE.tilew * BIGSPRITE.tileh + y * BIGSPRITE.tilew + x ]

			int k = 0;
			for( y=0; y<BIGSPRITE.tileh; y++ )
			for( x=0; x<BIGSPRITE.tilew; x++, k++ ) {

				if( 0 ) {
					char extfilename[XMAX_PATH];
					sprintf( extfilename, "%d_%dx%d_%d_%s.jpg", BIGSPRITE.handle, x+1, y+1, page+1, GetFilenameNoPath(BIGSPRITE.filename) );
					SaveFree( extfilename, WORKING );
				}

				// if( BIGSPRITE.tex_flag[k] ) xprintf("need munge %d\n",GetTic());

				ModifyTexMap( BIGSPRITE.texmapids[k], WORKING, NULL, BIGSPRITE.tex_flag[k] );
			}

			BIGSPRITE.curr_page = page;
#undef WORKING
		}
	}

	for( y=0; y<BIGSPRITE.tileh; y++ )
	for( x=0; x<BIGSPRITE.tilew; x++ ) {

		FLOAT scalex = 1.0f, scaley = 1.0f;

		if( width > 0 && width != BIGSPRITE.width ) {

			// scale X

			scalex =  (float)width / (float)BIGSPRITE.width;
		}


		if( height > 0 && height != BIGSPRITE.height ) {

			// scale Y

			scaley =  (float)height / (float)BIGSPRITE.height;
		}

		// kell-e forgatás?
		if( BIGSPRITE.fok != 0.0f  ) {

			FLOAT centerx,centery;
			FLOAT x1,y1;

			centerx = BIGSPRITE.width * scalex / 2;
			centery = BIGSPRITE.height * scaley / 2;

			// forgatjuk a körül
			// RotatePoint( point_t x0, point_t x1, FLOAT fok, FLOAT centerx=0.0f, FLOAT centery=0.0f );

			point[0][0] = x*BIGSPRITE.TILEW*scalex;
			point[0][1] = y*BIGSPRITE.TILEH*scaley;

			point[1][0] = x*BIGSPRITE.TILEW*scalex;
			point[1][1] = (y+1)*BIGSPRITE.TILEH*scaley;

			point[2][0] = (x+1)*BIGSPRITE.TILEW*scalex;
			point[2][1] = (y+1)*BIGSPRITE.TILEH*scaley;

			point[3][0] = (x+1)*BIGSPRITE.TILEW*scalex;
			point[3][1] = y*BIGSPRITE.TILEH*scaley;

			for( j=0; j<4; j++ ) {
				RotatePoint( point[j][0], point[j][1], &x1,&y1, BIGSPRITE.fok, centerx, centery );
				point[j][0] = screenx + x1;
				point[j][1] = screeny + y1;
			}
		}
		else {
			point[0][0] = screenx + x*BIGSPRITE.TILEW*scalex;
			point[0][1] = screeny + y*BIGSPRITE.TILEH*scaley;

			point[1][0] = screenx + x*BIGSPRITE.TILEW*scalex;
			point[1][1] = screeny + (y+1)*BIGSPRITE.TILEH*scaley;

			point[2][0] = screenx + (x+1)*BIGSPRITE.TILEW*scalex;
			point[2][1] = screeny + (y+1)*BIGSPRITE.TILEH*scaley;

			point[3][0] = screenx + (x+1)*BIGSPRITE.TILEW*scalex;
			point[3][1] = screeny + y*BIGSPRITE.TILEH*scaley;
		}

		// 2 rectangle intersection
		// http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
		FLOAT minx,miny;
		FLOAT maxx,maxy;

		minx = maxx = point[0][0];
		miny = maxy = point[0][1];

		for( j=0; j<4; j++ ) {
			if( point[j][0] < minx ) minx = point[j][0];
			if( point[j][1] < miny ) miny = point[j][1];
			if( point[j][0] > maxx ) maxx = point[j][0];
			if( point[j][1] > maxy ) maxy = point[j][1];
		}

		BOOL visible = !(minx > SCREENW || 0 > maxx ||
				 miny > SCREENH || maxy < 0);

		/***
		bool noOverlap = r1.x1 > r2.x2 ||
				 r2.x1 > r1.x2 ||
				 r1.y1 > r2.y2 ||
				 r2.y1 > r1.y2;
		return !noOverlap;
		***/

		/***
		// van-e valamelyik a screenen belül
		for( j=0; j<4; j++ )
			if( point[j][0] <= SCREENW && point[j][0] >= 0 &&
			    point[j][1] <= SCREENH && point[j][1] >= 0 )
			    visible = TRUE;

		// esetleg a screennél nagyobb-e
		if( point[0][0] <= 0 && point[2][0] >= SCREENW &&
		    point[0][1] <= 0 && point[1][1] >= SCREENH )
			visible = TRUE;


		// bal él metszi-e a screent
		if( point[0][0] >= 0 && point[0][0] <= SCREENW &&
		    point[0][1] <= 0 && point[1][1] >= SCREENH )
			visible = TRUE;

		// jobb él metszi-e a screent
		if( point[2][0] >= 0 && point[2][0] <= SCREENW &&
		    point[3][1] <= 0 && point[2][1] >= SCREENH )
			visible = TRUE;

		// felsõ él metszi-e a screent
		if( point[0][1] >= 0 && point[0][1] <= SCREENH &&
		    point[0][0] <= 0 && point[2][0] >= SCREENW )
			visible = TRUE;

		// alsó él metszi-e a screent
		if( point[1][1] >= 0 && point[1][1] <= SCREENH &&
		    point[0][0] <= 0 && point[2][0] >= SCREENW )
			visible = TRUE;
		***/

		if( visible )
			PutSpritePoly( poly, st, BIGSPRITE.texmapids[y * BIGSPRITE.tilew + x], rgb );
	}

	return;
}


//
// kitesz egy darabot a spriteból
// TODO: sprite scale, rotate
//
void PutBigSpritePart( int handle, int screenx, int screeny, int width, int height, int sx, int sy, int partw, int parth, int alpha ) {

#undef BIGSPRITE
#define BIGSPRITE bigsprites[i]

	int i,j;
	polygon_t poly;
	rgb_t rgb[4];
	point_t point[4];
	point3_t st[4];

	if( alpha == 0 )
		return;

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return;

	poly.npoints = 4;
	poly.point = point;

	st[0][0] = 0.0f;	st[0][1] = 0.0f;
	st[1][0] = 0.0f;	st[1][1] = 255.0f;
	st[2][0] = 255.0f;	st[2][1] = 255.0f;
	st[3][0] = 255.0f;	st[3][1] = 0.0f;

	for( j=0; j<4; j++ ) {
		rgb[j].r = 255;
		rgb[j].g = 255;
		rgb[j].b = 255;
		rgb[j].a = alpha>0?alpha:255;
		CLAMPMAX( rgb[j].a, 255 );
	}

	if( sx == -1 ) sx = 0;
	if( sy == -1 ) sy = 0;
	if( partw == -1 ) partw = width;
	if( parth == -1 ) parth = height;

	if( sx < 0 ) {
		partw += sx;
		screenx -= sx;
		width += sx;
		sx = 0;
	}
	if( sy < 0 ) {
		parth += sy;
		screeny -= sy;
		height += sy;
		sy = 0;
	}

	if( width <= 0 || height <= 0 || partw <= 0 ||parth <= 0 )
		return;

	if( (screenx + width < 0) || (screeny + height < 0) || screenx >= SCREENW || screeny > SCREENH )
		return;

	// xprintf("PutBigSpritePart: %d %d %d %d %d %d %d %d %d %d\n",handle, screenx, screeny, width, height, sx, sy, partw, parth, alpha);

	int rh = parth;		// remaining height
	for( int y=sy; y<sy+parth; /*y+=BIGSPRITE.TILEH,rh-=BIGSPRITE.TILEH*/ ) {

		int dy = y % BIGSPRITE.TILEH;
		int h = (dy + rh < BIGSPRITE.TILEH) ? rh : (BIGSPRITE.TILEH - dy);

		int rw = partw;
		for( int x=sx; x<sx+partw; /*x+=BIGSPRITE.TILEW,rw-=BIGSPRITE.TILEW*/ ) {

			int tilex = ftoi( (FLOAT)x / (FLOAT)BIGSPRITE.TILEW );
			int tiley = ftoi( (FLOAT)y / (FLOAT)BIGSPRITE.TILEH );

			int dx = x % BIGSPRITE.TILEW;
			int w = (dx + rw < BIGSPRITE.TILEW) ? rw : (BIGSPRITE.TILEW - dx);

			// xprintf("x,y: %d %d   dx,dy: %d %d   w,h: %d %d\n",x,y,dx,dy,w,h);

			st[0][0] = (FLOAT)dx * 256.0f / (FLOAT)BIGSPRITE.TILEW;
			st[0][1] = (FLOAT)dy * 256.0f / (FLOAT)BIGSPRITE.TILEH;

			st[1][0] = st[0][0];
			st[1][1] = (FLOAT)(dy+h) * 256.0f / (FLOAT)BIGSPRITE.TILEH;

			st[2][0] = (FLOAT)(dx+w) * 256.0f / (FLOAT)BIGSPRITE.TILEW;
			st[2][1] = st[1][1];

			st[3][0] = st[2][0];
			st[3][1] = st[0][1];

			int ox = screenx + x - sx;
			int oy = screeny + y - sy;

			point[0][0] = ox; point[0][1] = oy;
			point[1][0] = ox; point[1][1] = oy+h;
			point[2][0] = ox+w; point[2][1] = oy+h;
			point[3][0] = ox+w; point[3][1] = oy;

			// DrawTexRect( point[0][0],point[0][1], point[2][0],point[2][1],3);

			PutSpritePoly( poly, st, bigsprites[i].texmapids[tiley * bigsprites[i].tilew + tilex], rgb );

			x += w;
			rw -= w;
		}

		y += h;
		rh -= h;
	}

	return;
}



//
//
//
BOOL ModifyBigSprite( int handle, UCHAR *spr, UCHAR *pal, int flag ) {

	int i;

#undef BIGSPRITE
#define BIGSPRITE bigsprites[i]

	for( i=0; i<numbigsprites; i++ )
		if( BIGSPRITE.handle == handle )
			break;

	if( i >= numbigsprites )
		// nincs ilyen
		return FALSE;

	if( spr == NULL ||
	    BIGSPRITE.width != SPRITEW(spr) ||
	    BIGSPRITE.height != SPRITEH(spr) ||
	    SPRPIXELLEN(BIGSPRITE.working[0]) != SPRPIXELLEN(spr) )
		return FALSE;

	int pixellen = SPRPIXELLEN(spr);

#define WORKING BIGSPRITE.working[texid_offset]

	// UCHAR *tmp;
	// ALLOCMEM( tmp, SPRITEHEADER + SPRITEW(WORKING) * SPRITEH(WORKING) * pixellen );

	int TILEW = BIGSPRITE.TILEW;
	int TILEH = BIGSPRITE.TILEH;

	int texid_offset = 0;
	for( int y=0; y<BIGSPRITE.tileh; y++ )
	for( int x=0; x<BIGSPRITE.tilew; x++, texid_offset++ ) {

		if( 1 ) {
			// retkes lassú
			BOOL munge = FALSE;
			if( (( (x+1)*TILEW > BIGSPRITE.width ) ||
			     ( (y+1)*TILEH > BIGSPRITE.height ) ) )
				munge = TRUE;

			CopySpritePart( WORKING, &spr[SPRITEHEADER], SPRITEW(spr), SPRITEH(spr), pixellen, x*TILEW, y*TILEH, TILEW, TILEH, munge, FALSE );
		}
		else {
			// alig gyorsabb, de atlátszóságot nem csinál
			int wline = SPRITEW(WORKING) * pixellen;
			int line = wline;
			int h = SPRITEH(WORKING);

			if( (x+1)*TILEW > BIGSPRITE.width )
				line = (BIGSPRITE.width - x*TILEW) * pixellen;

			if( (y+1)*TILEH > BIGSPRITE.height )
				h = BIGSPRITE.height - y*TILEH;

			for( int j=0; j<h; j++ )
				memcpy( &WORKING[SPRITEHEADER + j*wline],
					&spr[ SPRITEHEADER + (y * TILEH + j) * BIGSPRITE.width * pixellen + x * TILEW * pixellen ],
					line );
		}

		ModifyTexMap( BIGSPRITE.texmapids[texid_offset], WORKING, NULL, BIGSPRITE.tex_flag[texid_offset]  );
	}

	// FREEMEM( tmp );

	return TRUE;
}



//
//
//
BOOL GetBigSpriteParam( int handle, int *w, int *h, BOOL *alpha, BOOL *chroma, int *bpp, int *pages, int *delay, FLOAT *fok, point_t *bbox, int *curr_page ) {

	int i;

	if( w ) *w = -1;
	if( h ) *h = -1;
	if( alpha ) *alpha = -1;
	if( chroma ) *chroma = -1;
	if( fok ) *fok = 0.0f;

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return FALSE;

	if( w ) *w = bigsprites[i].width;
	if( h ) *h = bigsprites[i].height;
	if( bpp ) *bpp = bigsprites[i].bpp;
	if( alpha ) *alpha = bigsprites[i].isAlpha;
	if( chroma ) *chroma = bigsprites[i].isChroma;
	if( pages ) *pages = bigsprites[i].pages;
	if( delay ) *delay = bigsprites[i].delay;
	if( fok ) *fok = bigsprites[i].fok;

	if( bbox ) {
		// TODO: a befoglaló keret
		if( bigsprites[i].fok == 0.0f || bigsprites[i].fok == 360.0f ) {
			bbox[0][0] = 0;
			bbox[0][1] = 0;

			bbox[1][0] = bigsprites[i].width;
			bbox[1][1] = bigsprites[i].height;
		}
		else {
			FLOAT centerx = bigsprites[i].width / 2;
			FLOAT centery = bigsprites[i].height / 2;
		}
	}

	if( curr_page ) *curr_page = bigsprites[i].curr_page;

	return TRUE;
}




//
//
//
BOOL SetBigSpriteParam( int handle, int alpha, int chroma, int delay, FLOAT fok, int page ) {

	int i;

	// xprintf("SetBigSpriteParam: called with %d, %d, %d, %.2f\n", handle, alpha, chroma, fok );

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return FALSE;

	if( alpha > -1 ) {
		bigsprites[i].isAlpha = TRUE;
		bigsprites[i].alpha = alpha;
	}

	if( chroma > -1 ) {
		bigsprites[i].isChroma = chroma;
		bigsprites[i].alpha = alpha;
	}

	if( delay > 0 ) {
		bigsprites[i].delay = delay;
	}

	bigsprites[i].fok = fok;
	while( bigsprites[i].fok >= 360.0f ) bigsprites[i].fok -= 360.0f;
	while( bigsprites[i].fok < 0.0f ) bigsprites[i].fok += 360.0f;

	// animgif
	if( bigsprites[i].type == ST_ANIM && page > -1 && page < bigsprites[i].pages ) {

		int tic = GetTic();

		if( bigsprites[i].tic == -1 )
			bigsprites[i].tic = tic;

		bigsprites[i].tic = tic - page * bigsprites[i].delay;
	}

	return TRUE;
}




//
//
//
BOOL GetBigSpritePixel( int handle, int x, int y, int *r, int *g, int *b ) {

	int i;

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return FALSE;

	if( r ) *r = *((UCHAR*)(bigsprites[i].spr) + SPRITEHEADER + (SPRITEW(bigsprites[i].spr)*(y)*SPRPIXELLEN(bigsprites[i].spr)) + ((x)*SPRPIXELLEN(bigsprites[i].spr)) +0);
	if( g ) *g = *((UCHAR*)(bigsprites[i].spr) + SPRITEHEADER + (SPRITEW(bigsprites[i].spr)*(y)*SPRPIXELLEN(bigsprites[i].spr)) + ((x)*SPRPIXELLEN(bigsprites[i].spr)) +1);
	if( b ) *b = *((UCHAR*)(bigsprites[i].spr) + SPRITEHEADER + (SPRITEW(bigsprites[i].spr)*(y)*SPRPIXELLEN(bigsprites[i].spr)) + ((x)*SPRPIXELLEN(bigsprites[i].spr)) +2);

	return TRUE;
}


//
//
//
UCHAR *GetBigSpriteData( int handle ) {

	int i;

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return NULL;

	return bigsprites[i].spr;
}




//
//
//
int IsBigSpriteLoaded( char *filename ) {

	int i;

	if( filename == NULL )
		return 0;

	// be van-e már töltve
	for( i=0; i<numbigsprites; i++ )
		if( !stricmp( bigsprites[i].filename, filename ) )
			break;

	// igen
	if( i < numbigsprites )
		return bigsprites[i].handle;

	return 0;
}



//
//
//
void FreeBigSprite( int handle ) {

	int i,j;

	for( i=0; i<numbigsprites; i++ )
		if( bigsprites[i].handle == handle )
			break;

	if( i >= numbigsprites )
		return;

	/***
	winShowPanel(bigsprites[i].filename, "Freeing");
	winSetPanel(NULL,0,bigsprites[i].pages/2);
	***/

	if( GFXDLL_GetData ) {
		for( j=0; j<bigsprites[i].pages * bigsprites[i].tilew * bigsprites[i].tileh; j++ ) {
			FREEMEM( bigsprites[i].working[j] );
		}
	}

	FREEMEM( bigsprites[i].working );
	FREEMEM( bigsprites[i].tex_flag );

	for( j=0; j<bigsprites[i].numtexmapids; j++ ) {
		DiscardTexmap( bigsprites[i].texmapids[j] );
		/***
		if( (bigsprites[i].pages > 1) && !(j%(bigsprites[i].numtexmapids/(bigsprites[i].pages/2))) )
			winSetPanel(NULL,j/(bigsprites[i].numtexmapids/(bigsprites[i].pages/2)));
		***/
	}

	if( GFXDLL_GetData )
		FREEMEM( bigsprites[i].texmapids );
	FREEMEM( bigsprites[i].spr );

	/***
	winHidePanel();
	***/

	// ha csak egy volt
	if( numbigsprites == 1 ) {
		numbigsprites = 0;
		FREEMEM( bigsprites );
		bigsprites = NULL;
		return;
	}

	// nem az utsó volt
	if( i < (numbigsprites-1) )
		memmove( &bigsprites[i],
			 &bigsprites[i+1],
			 sizeof(bigsprite_t)*(numbigsprites-(i+1)) );

	// ha az utsó volt leszarjuk

	--numbigsprites;

	return;
}



//
//
//
void DiscardAllBigSprite( void ) {

	while( numbigsprites > 0 )
		FreeBigSprite( bigsprites[numbigsprites-1].handle );

	return;
}




#if 0

Image Scaling With Bresenham
by Thiadmer Riemersma

Listing One
void ScaleLine(PIXEL *Target, PIXEL *Source, int SrcWidth, int TgtWidth)
{
  int NumPixels = TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;
  while (NumPixels-- > 0) {
    *Target++ = *Source;
    Source += IntPart;
    E += FractPart;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    } /* if */
  } /* while */
}

Listing Two
#define average(a, b)   (PIXEL)(( (int)(a) + (int)(b) ) >> 1)
void ScaleLineAvg(PIXEL *Target, PIXEL *Source, int SrcWidth, int TgtWidth)
{
  int NumPixels = TgtWidth;
  int Mid = TgtWidth / 2;
  int E = 0;
  PIXEL p;
  if (TgtWidth > SrcWidth)
    NumPixels--;
  while (NumPixels-- > 0) {
    p = *Source;
    if (E >= Mid)
     p = average(p, *(Source+1));
    *Target++ = p;
    E += SrcWidth;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    } /* if */
  } /* while */
  if (TgtWidth > SrcWidth)
    *Target = *Source;
}

Listing Three
void ScaleRect(PIXEL *Target, PIXEL *Source, int SrcWidth, int SrcHeight,
               int TgtWidth, int TgtHeight)
{
  int NumPixels = TgtHeight;
  int IntPart = (SrcHeight / TgtHeight) * SrcWidth;
  int FractPart = SrcHeight % TgtHeight;
  int E = 0;
  PIXEL *PrevSource = NULL;
  while (NumPixels-- > 0) {
    if (Source == PrevSource) {
      memcpy(Target, Target-TgtWidth, TgtWidth*sizeof(*Target));
    } else {
      ScaleLine(Target, Source, SrcWidth, TgtWidth);
      PrevSource = Source;
    } /* if */
    Target += TgtWidth;
    Source += IntPart;
    E += FractPart;
    if (E >= TgtHeight) {
      E -= TgtHeight;
      Source += SrcWidth;
    } /* if */
  } /* while */
}


#endif







