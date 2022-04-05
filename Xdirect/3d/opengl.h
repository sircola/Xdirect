/* Important note: Do not include any of the gl includes yourself!
 * include only this file, it will load gl.h etc properly so that
 * the compiler wont think it is calling a staticly bound dll
 *
 * and make sure that you include windows.h before including this file
 */

#ifndef __PLASMA_OPENGL_H
#define __PLASMA_OPENGL_H

#include <windows.h>

#include <xdllconv.h>

/* do this so that it wont declare everything as dllimport */
#undef WINGDIAPI
#define WINGDIAPI

#ifdef __GL_H__
  #error Do not manually include gl.h or glu.h
#endif
#ifdef __gl_h__
  #error Do not manually include gl.h or glu.h
#endif

#include "gl.h"
#include <GL/glu.h>
#include "glext.h"


/* restore normal meaning of WINGDIAPI */
#undef WINGDIAPI
#define WINGDIAPI DECLSPEC_IMPORT

#ifdef __cplusplus
extern "C" {
#endif

/* Bypasses the GDI SwapBuffers call, calls the minidriver directly instead */
extern BOOL openglBypassGDI;

extern int OpenGLInit(char *lib);
extern void OpenGLUnInit(void);

/* Use these instead of the wgl* functions */
extern HGLRC APIENTRY OpenGLCreateContext(HDC hdc);
extern BOOL  APIENTRY OpenGLDeleteContext(HGLRC hglrc);
extern BOOL  APIENTRY OpenGLMakeCurrent(HDC hdc, HGLRC hglrc);
extern BOOL  APIENTRY OpenGLSwapLayerBuffers(HDC hdc, UINT fuPlanes);

/* use these functions instead of the GDI ones */
extern BOOL  APIENTRY OpenGLSwapBuffers(HDC hdc);
extern int   APIENTRY OpenGLChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR * ppfd);
extern int   APIENTRY OpenGLDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);
extern BOOL  APIENTRY OpenGLSetPixelFormat(HDC hdc, int iPixelFormat, CONST PIXELFORMATDESCRIPTOR * ppfd);

extern void (*dprintf)( char *s, ... );
extern BOOL (*dwinOpengl)( void );
extern int (*GL_winReadProfileInt)( char *, int );
extern void (*GL_winWriteProfileInt)( char *, int );
extern void (*GL_winReadProfileString)( char *key, char *default_string, char *string, int size );
extern void (*GL_winWriteProfileString)( char *key, char *string );
extern ULONG (*dGetHwnd)( void );
extern ULONG (*dGetHinstance)( void );

extern void dllQuit( char *s, ... );


#ifdef __cplusplus
}
#endif

#endif

