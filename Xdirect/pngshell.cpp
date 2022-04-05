
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <conio.h>
#include <malloc.h>
#include <windows.h>

#include <xlib.h>

RCSID( "$Id: pngshell.cpp,v 1.1.1.1 2003/08/19 17:44:46 bernie Exp $" )


#define PNG_ASSEMBLER_CODE_SUPPORTED
#define PNG_BYTES_TO_CHECK  8

#include <png.h>

#ifndef png_bytepp_NULL
#define png_bytepp_NULL NULL
#endif

#undef png_jmpbuf
#define png_jmpbuf(png_ptr) (*lib.png_set_longjmp_fn((png_ptr), longjmp, sizeof (jmp_buf)))


#define PNGDLLNAME "libpng15-15.dll"


static struct {
	int loaded;
	HINSTANCE handle;
	png_infop (*png_create_info_struct) (png_structp png_ptr);
	png_structp (*png_create_read_struct) (png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
	void (*png_destroy_read_struct) (png_structpp png_ptr_ptr, png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr);
	png_uint_32 (*png_get_IHDR) (png_structp png_ptr, png_infop info_ptr, png_uint_32 *width, png_uint_32 *height, int *bit_depth, int *color_type, int *interlace_method, int *compression_method, int *filter_method);
	png_voidp (*png_get_io_ptr) (png_structp png_ptr);
	png_uint_32 (*png_get_tRNS) (png_structp png_ptr, png_infop info_ptr, png_bytep *trans, int *num_trans, png_color_16p *trans_values);
	png_uint_32 (*png_get_valid) (png_structp png_ptr, png_infop info_ptr, png_uint_32 flag);
	void (*png_read_image) (png_structp png_ptr, png_bytepp image);
	void (*png_read_info) (png_structp png_ptr, png_infop info_ptr);
	void (*png_read_update_info) (png_structp png_ptr, png_infop info_ptr);
	void (*png_set_expand) (png_structp png_ptr);
	void (*png_set_gray_to_rgb) (png_structp png_ptr);
	void (*png_set_packing) (png_structp png_ptr);
	void (*png_set_read_fn) (png_structp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn);
	void (*png_set_strip_16) (png_structp png_ptr);
	int (*png_sig_cmp) (png_bytep sig, png_size_t start, png_size_t num_to_check);
	void (*png_set_sig_bytes)(png_structp png_ptr,int num_bytes);
	void (*png_set_background)(png_structp png_ptr,png_color_16p background_color, int background_gamma_code, int need_expand, double background_gamma);
	png_uint_32 (*png_get_gAMA)(png_structp png_ptr, png_infop info_ptr, double *file_gamma);
	void (*png_set_gamma)(png_structp png_ptr,double screen_gamma, double default_file_gamma);
	void (*png_set_invert_alpha)(png_structp png_ptr);
	png_uint_32 (*png_get_PLTE)(png_structp png_ptr,png_infop info_ptr, png_colorp *palette, int *num_palette);
	void (*png_read_rows)(png_structp png_ptr,png_bytepp row, png_bytepp display_row, png_uint_32 num_rows);
	void (*png_read_end)(png_structp png_ptr,png_infop info_ptr);
	png_uint_32 (*png_get_bKGD)(png_structp png_ptr,png_infop info_ptr, png_color_16p *background);
	void (*png_write_end)(png_structp png_ptr,png_infop info_ptr);
	void (*png_destroy_write_struct)(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr);
	void (*png_free)(png_structp png_ptr, png_voidp ptr);
	void (*png_write_row)(png_structp png_ptr,png_bytep row);
	void (*png_write_info)(png_structp png_ptr,png_infop info_ptr);
	void (*png_set_PLTE)(png_structp png_ptr,png_infop info_ptr, png_colorp palette, int num_palette);
	png_voidp (*png_malloc)(png_structp png_ptr,png_uint_32 size);
	void (*png_set_IHDR)(png_structp png_ptr,png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,int color_type, int interlace_method, int compression_method,int filter_method);
	void (*png_set_write_fn)(png_structp png_ptr,png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn);
	png_structp (*png_create_write_struct)(png_const_charp user_png_ver, png_voidp error_ptr,png_error_ptr error_fn, png_error_ptr warn_fn);
	jmp_buf* (*png_set_longjmp_fn)(png_structp png_ptr, png_longjmp_ptr longjmp_fn, size_t jmp_buf_size);
} lib;


//
//
//
BOOL LoadPNGDll( void ) {

	if( lib.loaded == 0 ) {


		char bassdllname[PATH_MAX];

		sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, PNGDLLNAME );


		// load the temporary BASS.DLL library
		if( (lib.handle = LoadLibrary(bassdllname)) == NULL ) {
			xprintf("LoadPNGDll: couldn't load \"%s\" library.\n",bassdllname);
			return 3;
		}

		lib.png_create_info_struct =
			(png_infop (*) (png_structp))
			GetProcAddress(lib.handle, "png_create_info_struct");
		if ( lib.png_create_info_struct == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_create_read_struct =
			(png_structp (*) (png_const_charp, png_voidp, png_error_ptr, png_error_ptr))
			GetProcAddress(lib.handle, "png_create_read_struct");
		if ( lib.png_create_read_struct == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_destroy_read_struct =
			(void (*) (png_structpp, png_infopp, png_infopp))
			GetProcAddress(lib.handle, "png_destroy_read_struct");
		if ( lib.png_destroy_read_struct == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_get_IHDR =
			(png_uint_32 (*) (png_structp, png_infop, png_uint_32 *, png_uint_32 *, int *, int *, int *, int *, int *))
			GetProcAddress(lib.handle, "png_get_IHDR");
		if ( lib.png_get_IHDR == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_get_io_ptr =
			(png_voidp (*) (png_structp))
			GetProcAddress(lib.handle, "png_get_io_ptr");
		if ( lib.png_get_io_ptr == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_get_tRNS =
			(png_uint_32 (*) (png_structp, png_infop, png_bytep *, int *, png_color_16p *))
			GetProcAddress(lib.handle, "png_get_tRNS");
		if ( lib.png_get_tRNS == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_get_valid =
			(png_uint_32 (*) (png_structp, png_infop, png_uint_32))
			GetProcAddress(lib.handle, "png_get_valid");
		if ( lib.png_get_valid == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_read_image =
			(void (*) (png_structp, png_bytepp))
			GetProcAddress(lib.handle, "png_read_image");
		if ( lib.png_read_image == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_read_info =
			(void (*) (png_structp, png_infop))
			GetProcAddress(lib.handle, "png_read_info");
		if ( lib.png_read_info == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_read_update_info =
			(void (*) (png_structp, png_infop))
			GetProcAddress(lib.handle, "png_read_update_info");
		if ( lib.png_read_update_info == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_set_expand =
			(void (*) (png_structp))
			GetProcAddress(lib.handle, "png_set_expand");
		if ( lib.png_set_expand == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_set_gray_to_rgb =
			(void (*) (png_structp))
			GetProcAddress(lib.handle, "png_set_gray_to_rgb");
		if ( lib.png_set_gray_to_rgb == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_set_packing =
			(void (*) (png_structp))
			GetProcAddress(lib.handle, "png_set_packing");
		if ( lib.png_set_packing == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_set_read_fn =
			(void (*) (png_structp, png_voidp, png_rw_ptr))
			GetProcAddress(lib.handle, "png_set_read_fn");
		if ( lib.png_set_read_fn == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_set_strip_16 =
			(void (*) (png_structp))
			GetProcAddress(lib.handle, "png_set_strip_16");
		if ( lib.png_set_strip_16 == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}
		lib.png_sig_cmp =
			(int (*) (png_bytep, png_size_t, png_size_t))
			GetProcAddress(lib.handle, "png_sig_cmp");
		if ( lib.png_sig_cmp == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}

		lib.png_set_longjmp_fn = (jmp_buf* (*) (png_structp, png_longjmp_ptr, size_t))GetProcAddress(lib.handle, "png_set_longjmp_fn");
		if ( lib.png_set_longjmp_fn == NULL ) {
			FreeLibrary(lib.handle);
			return -1;
		}

		xprintf("LoadPNGDll: libpng (%s) loaded.\n",bassdllname);
	}

	++lib.loaded;

	return 0;
}



//
//
//
void FreePNGDll() {

	if ( lib.loaded == 0 ) {
		return;
	}

	if ( lib.loaded == 1 ) {
		FreeLibrary(lib.handle);
	}

	--lib.loaded;

	return;
}




//
//
//
void GetRgbPng( UCHAR *pal ) {

	int i;

	for(i=0;i<768;i++)
		pal[i] = pic_pal[i];

	return;
}



//
//
//
void SetRgbPng( void ) {

	UCHAR *pal;

	ALLOCMEM( pal, 768 );

	GetRgbPcx(pal); 			// 6 bit miatt
	SetRgbBuf(pal);

	FREEMEM( pal );

	return;
}





//
//
//
BOOL IsPng( char *name ) {

	FILE *f = NULL;
	UCHAR magic[4] = "DRP";
	BOOL is_PNG = FALSE;

	if( (f = ffopen(name,"rb")) == NULL )
		return FALSE;

	ffread( magic, 4, 1, f );

	ffclose( f );

	if ( magic[0] == 0x89 &&
	     magic[1] == 'P' &&
	     magic[2] == 'N' &&
	     magic[3] == 'G' ) {
		is_PNG = 1;
	}

	return is_PNG;
}





static FILE *s_handle = NULL;

//
//
//
void ReadProc( struct png_struct_def *b, png_bytep data, png_size_t size ) {

	fread( data, size, 1, s_handle );

	return;
}


//
//
//
void WriteProc( struct png_struct_def *b, png_bytep data, png_size_t size ) {

	fwrite( data, size, 1, s_handle );

	return;
}



//
//
//
void FlushProc( png_structp png_ptr ) {

	// empty flush implementation

	return;
}


//
//
//
void error_handler( struct png_struct_def *png_ptr, const char *error ) {

	xprintf("LoadPng(): %s.\n", error );

	// Return control to the setjmp point

	longjmp( png_jmpbuf(png_ptr), 1 );

	return;
}


//
// warnings disabled
//
void warning_handler( struct png_struct_def *b, const char *warning ) {

	xprintf("LoadPng() Warning: %s.\n", warning );

	return;
}




//
//
//
BOOL LoadPng( char *fname, memptr_ptr image ) {

	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	png_colorp png_palette;
	double gamma = 0;
	double screen_gamma = 2.2;
	int bpp, color_type, palette_entries;
	int i;

	// Quit( png_get_copyright(NULL) );

	if( LoadPNGDll() )
		return FALSE;


	if( (s_handle = fopen(fname,"rb")) == NULL )
		return FALSE;

	unsigned char png_check[PNG_BYTES_TO_CHECK];

	fread( png_check, PNG_BYTES_TO_CHECK, 1, s_handle );

	if( lib.png_sig_cmp(png_check, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0)
			goto png_read_error_label; // Bad signature

	// create the chunk manage structure

	png_ptr = lib.png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)error_handler, error_handler, warning_handler);

	if( !png_ptr )
		goto png_read_error_label;

	// create the info structure

	if( (info_ptr = lib.png_create_info_struct(png_ptr)) == NULL ) {
		lib.png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		goto png_read_error_label;
	}

	// init the IO

	lib.png_set_read_fn( png_ptr, info_ptr, ReadProc );

	if( setjmp(png_jmpbuf(png_ptr)) ) {
		lib.png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		goto png_read_error_label;
	}

	// Because we have already read the signature...

	lib.png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

	// read the IHDR chunk

	lib.png_read_info(png_ptr, info_ptr);
	lib.png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpp, &color_type, NULL, NULL, NULL);

	// DIB's don't support >8 bits per sample
	// => tell libpng to strip 16 bit/color files down to 8 bits/color

	if( bpp == 16 ) {
		lib.png_set_strip_16(png_ptr);
		bpp = 8;
	}

	// Set some additional flags

	switch(color_type) {

		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:

			// Flip the RGB pixels to BGR (or RGBA to BGRA)

			// png_set_bgr(png_ptr);

			break;

		case PNG_COLOR_TYPE_PALETTE:

			// Expand palette images to the full 8 bits from 2 or 4 bits/pixel

			if( (bpp == 2) || (bpp == 4) ) {
				lib.png_set_packing(png_ptr);
				bpp = 8;
			}

			break;

		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_GRAY_ALPHA:

			// Expand grayscale images to the full 8 bits from 2 or 4 bits/pixel

			if( (bpp == 2) || (bpp == 4) ) {
				lib.png_set_expand(png_ptr);
				bpp = 8;
			}

			break;

		default:

			xprintf("PNG format not supported\n");
			goto png_read_error_label;

			break;
	}

	// Set the background color to draw transparent and alpha images over.
	// It is possible to set the red, green, and blue components directly
	// for paletted images instead of supplying a palette index.  Note that
	// even if the PNG file supplies a background, you are not required to
	// use it - you should use the (solid) application background if it has one.

	if( color_type != PNG_COLOR_TYPE_RGB_ALPHA ) {

		png_color_16 my_background= { 0, 255, 255, 255, 0 };
		png_color_16 *image_background;

		if( lib.png_get_bKGD(png_ptr, info_ptr, &image_background) )
			lib.png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
		else
			lib.png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	}

	if( lib.png_get_gAMA(png_ptr, info_ptr, &gamma) )
		lib.png_set_gamma(png_ptr, screen_gamma, gamma);

	// All transformations have been registered; now update info_ptr data

	lib.png_read_update_info(png_ptr, info_ptr);

	// Create a DIB and write the bitmap header
	// set up the DIB palette, if needed

	switch( color_type ) {

		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:

			lib.png_set_invert_alpha(png_ptr);
			// dib = freeimage->allocate_proc(width, height, 24, 0xFF, 0xFF00, 0xFF0000);

			ALLOCMEM( *image, (width*height*3)+SPRITEHEADER );

			MKSPRW( *image, width);
			MKSPRH( *image, height);
			MKSPR24( *image );

			break;

		case PNG_COLOR_TYPE_PALETTE :

			// dib = freeimage->allocate_proc(width, height, bpp);

			ALLOCMEM( *image, (width*height)+SPRITEHEADER );

			MKSPRW( *image, width);
			MKSPRH( *image, height);
			MKSPR8( *image );

			lib.png_get_PLTE(png_ptr,info_ptr, &png_palette,&palette_entries);

			// palette = freeimage->get_palette_proc(dib);

			// store the palette

			CLAMPMINMAX( palette_entries, 0, 256 );

			for( i=0; i<palette_entries; i++ ) {
				pic_pal[(i*3)+0] = png_palette[i].red >> 2;
				pic_pal[(i*3)+1] = png_palette[i].green >> 2;
				pic_pal[(i*3)+2] = png_palette[i].blue >> 2;
			}

			// store the transparency table

			// if( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )
			//	freeimage->set_transparency_table_proc(dib, (BYTE *)trans, num_trans);

			break;

		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_GRAY_ALPHA:

			// dib = freeimage->allocate_proc(width, height, bpp);

			ALLOCMEM( *image, (width*height)+SPRITEHEADER );

			MKSPRW( *image, width);
			MKSPRH( *image, height);
			MKSPR8( *image );

			// palette = freeimage->get_palette_proc(dib);
			palette_entries = 1 << bpp;

			for( i=0; i<palette_entries; i++ ) {
				pic_pal[(i*3)+0] =
				pic_pal[(i*3)+1] =
				pic_pal[(i*3)+2] = (i * 255) / (palette_entries - 1);
			}

			// store the transparency table

			// if( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )
			//	freeimage->set_transparency_table_proc(dib, (BYTE *)trans, num_trans);

			break;
	}

	for( i=0; i<height; i++ ) {
		UCHAR *p = SPRLINE(*image, i);
		lib.png_read_rows(png_ptr, &p, png_bytepp_NULL, 1);
	}

	// cleanup

	lib.png_read_end( png_ptr, info_ptr );

	if( png_ptr )
		lib.png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return TRUE;

png_read_error_label:;

	if( png_ptr )
		lib.png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return FALSE;
}





//
// Save(FreeImage *freeimage, FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data)
//
BOOL SavePng( char *name, UCHAR *s, UCHAR *p ) {

	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette = NULL;
	png_uint_32 width, height, bpp;
	BOOL has_alpha_channel = FALSE;
	int bit_depth;
	int palette_entries;
	int interlace_type;
	int i;

	PushTomFlag();
	TomFlag( NOFLAG );

	if( (s_handle = fopen(name,"wb")) == NULL ) {
		PopTomFlag();
		xprintf("SavePng: can't open \"%s\" file.\n", name);
		return FALSE;
	}

	// create the chunk manage structure

	if( (png_ptr = lib.png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)error_handler, error_handler, warning_handler)) == NULL )
		goto png_write_error_label;

	// Allocate/initialize the image information data.

	if( (info_ptr = lib.png_create_info_struct(png_ptr)) == NULL ) {
		lib.png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
		goto png_write_error_label;
	}

	// Set error handling.  REQUIRED if you aren't supplying your own
	// error handling functions in the png_create_write_struct() call.

	if (setjmp(png_jmpbuf(png_ptr)))  {
		lib.png_destroy_write_struct(&png_ptr, &info_ptr);
		goto png_write_error_label;
	}

	// init the IO

	lib.png_set_write_fn(png_ptr, info_ptr, WriteProc, FlushProc);

	// Set the image information here.  Width and height are up to 2^31,
	// bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	// the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	// PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	// or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	// PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	// currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED

	width = SPRITEW(s);
	height = SPRITEH(s);
	bpp = bit_depth = (SPRPIXELLEN(s) * 8);

	if( ISSPR16(s) ) {
		bpp = 24;
		bit_depth = 24;
	}

	bit_depth = (bit_depth > 8) ? 8 : bit_depth;

	interlace_type = PNG_INTERLACE_NONE;	// Default value

	if( ISSPR8(s) ) {

		lib.png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
						PNG_COLOR_TYPE_PALETTE, interlace_type,
						PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		// set the palette

		palette_entries = 1 << bit_depth;
		palette = (png_colorp)lib.png_malloc(png_ptr, palette_entries * sizeof (png_color));

		for( i=0; i<palette_entries; i++) {
			palette[i].red   = p[(i*3)+0] << 2;
			palette[i].green = p[(i*3)+1] << 2;
			palette[i].blue  = p[(i*3)+2] << 2;
		}

		// You must not free palette here, because png_set_PLTE only makes a link to
		// the palette that you malloced.  Wait until you are about to destroy
		// the png structure.

		lib.png_set_PLTE(png_ptr, info_ptr, palette, palette_entries);
	}

	if( ISSPR24(s) || ISSPR16(s) ) {

		lib.png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
						PNG_COLOR_TYPE_RGB, interlace_type,
						PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		// png_set_bgr(png_ptr); // flip BGR pixels to RGB
	}

	lib.png_write_info(png_ptr, info_ptr);

	// write out the image data

	if( ISSPR8(s) || ISSPR24(s) )
		for( i=0; i<height; i++ ) {
			UCHAR *p = SPRLINE(s, i);
			lib.png_write_row(png_ptr, p);
		}
	else
	if( ISSPR16(s) ) {

		UCHAR *work,*pwork,*pspr;
		int x,w,word;

		w = SPRITEW(s);

		pspr = &s[SPRITEHEADER];

		ALLOCMEM( work, w*3 );

		for( i=0; i<height; i++ ) {

			pwork = work;

			for( x=0; x<w; x++ ) {

				word = *(USHORT*)pspr;
				pspr += 2;

				*pwork++ = (UCHAR)INTRED16(word) << (8-rsize);
				*pwork++ = (UCHAR)INTGREEN16(word) << (8-gsize);
				*pwork++ = (UCHAR)INTBLUE16(word) << (8-bsize);
			}

			lib.png_write_row(png_ptr, p);
		}

		FREEMEM( work );
	}


	// It is REQUIRED to call this to finish writing the rest of the file
	// Bug with png_flush

	lib.png_write_end(png_ptr, info_ptr);

	// clean up after the write, and free any memory allocated

	lib.png_destroy_write_struct(&png_ptr, &info_ptr);

	if( palette )
		lib.png_free(png_ptr, palette);

	fclose( s_handle );

	PopTomFlag();

	return TRUE;

png_write_error_label:;

	if( palette )
		lib.png_free(png_ptr, palette);

	fclose( s_handle );

	PopTomFlag();

	return FALSE;
}




