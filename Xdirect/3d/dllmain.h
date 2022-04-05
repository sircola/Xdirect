/* Copyright (C) 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */



#ifdef __cplusplus
extern "C" {
#endif


extern void dllQuit( char *s, ... );
extern void dllDeinit( void );               // = GFXDLL_Deinit
extern int dll_init( void );
extern void dll_deinit( void );
extern char *GFXDLL_NAME;
extern void (*dprintf)( char *s, ... );
extern unsigned long (*dGetBackHwnd)( void );
extern unsigned long dllGetHinst( void );
extern BOOL (*dwinQuit)( char *s, ... );
extern void *(*dmalloc)( int );
extern int (*dfree)( void * );
extern void *(*drealloc)( void *, int );




#ifdef __cplusplus
}
#endif


extern BOOL ChangeResolution( int w, int h, int bitdepth=-1 );
extern void RestoreResoltion( void );
extern void dibdetect( void );
