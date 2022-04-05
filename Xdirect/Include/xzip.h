
#ifdef __cplusplus
extern "C" {
#endif
extern int (_stdcall *_deflateInit_)(z_streamp strm, int level, const char *version, int stream_size);
extern int (_stdcall *_deflate)(z_streamp strm, int flush);
extern int (_stdcall *_deflateEnd)(z_streamp strm);
extern int (_stdcall *_inflate)(z_streamp strm, int flush);
extern int (_stdcall *_inflateEnd)(z_streamp strm);
extern int (_stdcall *_inflateInit2_)(z_streamp strm, int  windowBits, const char *version, int stream_size);
extern uLong (_stdcall *_crc32)(uLong crc, const Bytef *buf, uInt len);
#ifdef __cplusplus
}
#endif

#undef inflateInit2
#define inflateInit2(strm, windowBits) \
        _inflateInit2_((strm), (windowBits), ZLIB_VERSION, sizeof(z_stream)) 

#undef deflateInit
#define deflateInit(strm, level) \
        _deflateInit_((strm), (level),       ZLIB_VERSION, sizeof(z_stream)) 
/***
int (_stdcall *pdeflateInit_)(z_streamp strm, int level, const char *version, int stream_size) = NULL;
int (_stdcall *pdeflate)(z_streamp strm, int flush) = NULL;
int (_stdcall *pdeflateEnd)(z_streamp strm) = NULL;
int (_stdcall *pinflate)(z_streamp strm, int flush) = NULL;
int (_stdcall *pinflateEnd)(z_streamp strm) = NULL;
int (_stdcall *pinflateInit2_)(z_streamp strm, int  windowBits, const char *version, int stream_size) = NULL;
uLong (_stdcall *pcrc32)(uLong crc, const Bytef *buf, uInt len) = NULL;
int (_stdcall *pcompress)(Bytef *dest,   uLongf *destLen, const Bytef *source, uLong sourceLen) = NULL;
int (_stdcall *puncompress)(Bytef *dest,   uLongf *destLen, const Bytef *source, uLong sourceLen) = NULL;
***/

typedef int (*UNCOMPRESSFN)(UCHAR *, ULONG *, UCHAR *, ULONG);

extern UNCOMPRESSFN _uncompress;
extern UNCOMPRESSFN _compress;