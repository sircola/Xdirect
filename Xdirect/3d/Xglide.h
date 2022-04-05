
#ifdef __cplusplus
extern "C" {
#endif

// #undef FX_CALL
// #define FX_CALL __stdcall


extern BOOL LoadGlide( void );
extern void FreeGlide( void );


extern FxU32 ( FX_CALL *pGrTexMinAddress)( GrChipID_t tmu );
extern FxU32 ( FX_CALL *pGrTexMaxAddress)( GrChipID_t tmu );
extern void ( FX_CALL *pGrColorCombine)( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert );
extern FxU32 ( FX_CALL *pGrTexTextureMemRequired)( FxU32 evenOdd, GrTexInfo *info );
extern void ( FX_CALL *pGrTexDownloadMipMap)( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
extern void ( FX_CALL *pGrTexSource)( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
extern void ( FX_CALL *pGrTexCombine)( GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert );
extern void ( FX_CALL *pGrChromakeyMode)( GrChromakeyMode_t mode );
extern void ( FX_CALL *pGrChromakeyValue)( GrColor_t value );
extern void ( FX_CALL *pGrAlphaBlendFunction)( GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df );
extern void ( FX_CALL *pGrDepthMask)( FxBool mask );
extern void ( FX_CALL *pGrAlphaCombine)( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert );
extern void ( FX_CALL *pGrCullMode)( GrCullMode_t mode );
extern void ( FX_CALL *pGrDepthBufferFunction)( GrCmpFnc_t function );
extern void ( FX_CALL *pGrDepthBufferMode)( GrDepthBufferMode_t mode );
extern void ( FX_CALL *pGrDrawVertexArrayContiguous)( FxU32 mode, FxU32 Count, void *pointers, FxU32 stride );
extern void ( FX_CALL *pGrFinish)(void);
extern void (FX_CALL *pGrFlush)(void);
extern FxBool ( FX_CALL *pGrLfbLock)( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t *info );
extern FxBool ( FX_CALL *pGrLfbUnlock)( GrLock_t type, GrBuffer_t buffer );
extern void ( FX_CALL *pGrGlideInit)( void );
extern void ( FX_CALL *pGrGlideShutdown)( void );
extern FxU32 ( FX_CALL *pGrGet)( FxU32 pname, FxU32 plength, FxI32 *params );
extern const char * ( FX_CALL *pGrGetString)( FxU32 pname );
extern void ( FX_CALL *pGrSstSelect)( int which_sst );
extern void ( FX_CALL *pGrConstantColorValue)( GrColor_t value );
extern void ( FX_CALL *pGrLfbConstantDepth)( FxU32 depth );
extern GrContext_t ( FX_CALL *pGrSstWinOpen)( FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers );
extern FxBool ( FX_CALL *pGrSstWinClose)( GrContext_t context );
extern void ( FX_CALL *pGrRenderBuffer)( GrBuffer_t buffer );
extern void ( FX_CALL *pGrCoordinateSpace)( GrCoordinateSpaceMode_t mode );
extern void ( FX_CALL *pGrClipWindow)( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy );
extern void ( FX_CALL *pGrVertexLayout)(FxU32 param, FxI32 offset, FxU32 mode);
extern void ( FX_CALL *pGrDitherMode)( GrDitherMode_t mode );
extern void ( FX_CALL *pGrTexMipMapMode)( GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend );
extern void ( FX_CALL *pGrTexFilterMode)( GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode );
extern void ( FX_CALL *pGrTexClampMode)( GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode );
extern void ( FX_CALL *pGrEnable)( GrEnableMode_t mode );
extern void ( FX_CALL *pGrDisable)( GrEnableMode_t mode );
extern void ( FX_CALL *pGrSplash)(float x, float y, float width, float height, FxU32 frame);
extern void ( FX_CALL *pGrBufferSwap)( FxU32 swap_interval );
extern void ( FX_CALL *pGrBufferClear)( GrColor_t color, GrAlpha_t alpha, FxU32 depth );
extern void ( FX_CALL *pGrColorMask)( FxBool rgb, FxBool a );
extern void ( FX_CALL *pGrFogColorValue)( GrColor_t fogcolor );
extern void ( FX_CALL *pGrFogMode)( GrFogMode_t mode );
extern void ( FX_CALL *pGrFogTable)( const GrFog_t ft[] );


extern int Glide_winComboBox( combobox_t *combobox_data );
extern void Glide_ClearLogWindow( void );
extern void Glide_AddLogWindow( char *str );

extern void dllQuit( char *s, ... );

extern int dll_init( void );
extern void dll_deinit( void );
extern void (*dprintf)( char *s, ... );
extern texmap_t *(*dTexForNum)( int texmapid );
extern "C" ULONG (*dGetBackHwnd)( void );
extern "C" BOOL (*dwinQuit)( char *s, ... );


#define SNAP_BIAS ((float)(3L<<18))

typedef struct {
	float  sow;	/* s texture ordinate (s over w) */
	float  tow;	/* t texture ordinate (t over w) */
	float  oow;	/* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;


typedef struct {
	float x, y;		/* X and Y in screen space */
	float ooz;		/* 65535/Z (used for Z-buffering) */
	float oow;		/* 1/W (used for W-buffering, texturing) */
	float r, g, b, a;	/* R, G, B, A [0..255.0] */
	float z;		/* Z is ignored */
	GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} GrVertex;


#define GR_VERTEX_X_OFFSET		0
#define GR_VERTEX_Y_OFFSET		1
#define GR_VERTEX_OOZ_OFFSET		2
#define GR_VERTEX_OOW_OFFSET		3
#define GR_VERTEX_R_OFFSET		4
#define GR_VERTEX_G_OFFSET		5
#define GR_VERTEX_B_OFFSET		6
#define GR_VERTEX_A_OFFSET		7
#define GR_VERTEX_Z_OFFSET		8
#define GR_VERTEX_SOW_TMU0_OFFSET	9
#define GR_VERTEX_TOW_TMU0_OFFSET	10
#define GR_VERTEX_OOW_TMU0_OFFSET	11
#define GR_VERTEX_SOW_TMU1_OFFSET	12
#define GR_VERTEX_TOW_TMU1_OFFSET	13
#define GR_VERTEX_OOW_TMU1_OFFSET	14
#if (GLIDE_NUM_TMU > 2)
#define GR_VERTEX_SOW_TMU2_OFFSET	15
#define GR_VERTEX_TOW_TMU2_OFFSET	16
#define GR_VERTEX_OOW_TMU2_OFFSET	17
#endif




#define MAXTEXTURE 40


typedef struct glidetexture_s {

	FxU32 textureSize;
	FxU32 startAddress;
	GrTexInfo info;

	int texmapid;

	UCHAR *mipmap;
	int mipmap_size;

} glidetexture_t, *glidetexture_ptr;





#define HM_UNINIT	0
#define HM_TEXTURE	1
#define HM_COLOR	2

typedef struct hardwarestate_s {

	BOOL tex24bit;

	int texmap;
	int mode;
	int cullmode;
	BOOL zbuffermode;
	BOOL bSurfaceOn;

	int SCREENW,SCREENH,bpp;
	__int64 hwnd;
	__int64 hInst;
	BOOL bZBuffer;
	FLOAT zmax;

	GrContext_t context;

} hardwarestate_t;


extern hardwarestate_t hw_state;



//
// function to initialise the FPU to 24-bit precision
// _inline
static void init_fpu( void ) {

	unsigned long memvar;

#ifdef __GNUC__
	dprintf("init_fpu: not implemented.\n");
#else
	__asm {
		finit
		fwait
		fnstcw	[memvar]
		fwait
		mov	eax, [memvar]
		and	eax, 0fffffcffh
		mov	[memvar], eax
		fldcw	[memvar]
		fwait
	}
#endif

	return;
}


#define MAX_GCACHE_LUMPS	1024
#define TEXMEM_2MB_EDGE		2097152


//
// Doubly linked-list lump structure
//
typedef struct gcachelump_s {

	BOOL free;
	BOOL assigned;
	int start;
	char key[PATH_MAX];
	int size;
	int access_num;

	struct gcachelump_s *previous;
	struct gcachelump_s *next;

} gcachelump_t, *gcachelump_ptr;




//
//
//
typedef struct gcache_s {

	gcachelump_t gcache_lumps[ MAX_GCACHE_LUMPS ];

	int free_space;
	int total_space;
	int num_lumps;

	gcachelump_t *first_lump;

} gcache_t, *gcache_ptr;



extern void glide_cache_init( void );
extern void glide_cache_uninit( void );
extern int glide_cache_item_check( char * );
extern int glide_cache_item_space( char *, int );


extern "C" void *(*dmalloc)( int );
extern "C" int (*dfree)( void * );



#ifdef __cplusplus
}
#endif


