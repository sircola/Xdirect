
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "opengl.h"

static HINSTANCE openglInst = NULL;
BOOL openglBypassGDI = FALSE;

void            (APIENTRY *fnglAlphaFunc)(GLenum func, GLclampf ref) = NULL;
void            (APIENTRY *fnglBegin)(GLenum mode) = NULL;
void            (APIENTRY *fnglBindTexture)(GLenum target, GLuint texture) = NULL;
void            (APIENTRY *fnglBlendFunc)(GLenum sfactor, GLenum dfactor) = NULL;
void            (APIENTRY *fnglClear)(GLbitfield mask) = NULL;
void            (APIENTRY *fnglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = NULL;
void            (APIENTRY *fnglClearDepth)(GLclampd depth) = NULL;
void            (APIENTRY *fnglColor3f)(GLfloat red, GLfloat green, GLfloat blue) = NULL;
void            (APIENTRY *fnglColor3fv)(const GLfloat *v) = NULL;
void            (APIENTRY *fnglColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void            (APIENTRY *fnglColor4fv)(const GLfloat *v) = NULL;
void            (APIENTRY *fnglCullFace)(GLenum mode) = NULL;
void            (APIENTRY *fnglDepthFunc)(GLenum func) = NULL;
void            (APIENTRY *fnglDepthMask)(GLboolean flag) = NULL;
void            (APIENTRY *fnglDepthRange)(GLclampd zNear, GLclampd zFar) = NULL;
void            (APIENTRY *fnglDeleteTextures)(GLsizei n, const GLuint *textures) = NULL;
void            (APIENTRY *fnglDisable)(GLenum cap) = NULL;
void		(APIENTRY *fnglDrawPixels)(GLsizei width,GLsizei height,GLenum format,GLenum type, GLvoid *pixels ) = NULL;
void            (APIENTRY *fnglEnable)(GLenum cap) = NULL;
void            (APIENTRY *fnglEnd)(void) = NULL;
void            (APIENTRY *fnglFinish)(void) = NULL;
void            (APIENTRY *fnglFlush)(void) = NULL;
void		(APIENTRY *fnglFogf)( GLenum pname, GLfloat param) = NULL;
void		(APIENTRY *fnglFogfv)(GLenum pname, const GLfloat *params) = NULL;
void		(APIENTRY *fnglFogi)(GLenum pname, GLint param) = NULL;
void            (APIENTRY *fnglFrontFace)(GLenum mode) = NULL;
void            (APIENTRY *fnglFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) = NULL;
GLenum          (APIENTRY *fnglGetError)(void) = NULL;
void            (APIENTRY *fnglGetFloatv)(GLenum pname, GLfloat *params) = NULL;
void            (APIENTRY *fnglGetIntegerv)(GLenum pname, GLint *params) = NULL;
const GLubyte * (APIENTRY *fnglGetString)(GLenum name) = NULL;
void            (APIENTRY *fnglGenTextures)(GLsizei n, GLuint *textures) = NULL;
void            (APIENTRY *fnglHint)(GLenum target, GLenum mode) = NULL;
GLboolean       (APIENTRY *fnglIsEnabled)(GLenum cap) = NULL;
void            (APIENTRY *fnglLoadIdentity)(void) = NULL;
void            (APIENTRY *fnglLoadMatrixf)(const GLfloat *m) = NULL;
void            (APIENTRY *fnglMatrixMode)(GLenum mode) = NULL;
void            (APIENTRY *fnglMultMatrixf)(const GLfloat *m) = NULL;
void            (APIENTRY *fnglOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) = NULL;
void            (APIENTRY *fnglPixelStorei)(GLenum pname, GLint param) = NULL;
void            (APIENTRY *fnglPolygonOffset)(GLfloat factor, GLfloat units) = NULL;
void            (APIENTRY *fnglPolygonMode)(GLenum face, GLenum mode) = NULL;
void            (APIENTRY *fnglPopMatrix)(void) = NULL;
void            (APIENTRY *fnglPushMatrix)(void) = NULL;
void            (APIENTRY *fnglRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) = NULL;
void            (APIENTRY *fnglReadBuffer)(GLenum mode) = NULL;
void		(APIENTRY *fnglReadPixels)(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,GLvoid *pixels) = NULL;
void            (APIENTRY *fnglScalef)(GLfloat x, GLfloat y, GLfloat z) = NULL;
void            (APIENTRY *fnglShadeModel)(GLenum mode) = NULL;
void            (APIENTRY *fnglTexCoord2f)(GLfloat s, GLfloat t) = NULL;
void            (APIENTRY *fnglTexCoord2fv)(const GLfloat *v) = NULL;
void            (APIENTRY *fnglTexCoord3f)(GLfloat s, GLfloat t, GLfloat r) = NULL;
void            (APIENTRY *fnglTexCoord3fv)(const GLfloat *v) = NULL;
void            (APIENTRY *fnglTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q) = NULL;
void            (APIENTRY *fnglTexEnvf)(GLenum target, GLenum pname, GLfloat param) = NULL;
void            (APIENTRY *fnglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) = NULL;
void            (APIENTRY *fnglTexParameterf)(GLenum target, GLenum pname, GLfloat param) = NULL;
void            (APIENTRY *fnglTranslatef)(GLfloat x, GLfloat y, GLfloat z) = NULL;
void            (APIENTRY *fnglVertex2f)(GLfloat x, GLfloat y) = NULL;
void            (APIENTRY *fnglVertex3f)(GLfloat x, GLfloat y, GLfloat z) = NULL;
void            (APIENTRY *fnglVertex3fv)(const GLfloat *v) = NULL;
void            (APIENTRY *fnglVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w) = NULL;
void            (APIENTRY *fnglViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void		(APIENTRY *fnglScissor)( GLint x, GLint y, GLsizei width, GLsizei height ) = NULL;
void		(APIENTRY *fnglEnableClientState)( GLenum array ) = NULL;
void		(APIENTRY *fnglDisableClientState)( GLenum array ) = NULL;
BOOL		(APIENTRY *fnSwapBuffers)(HDC hdc) = NULL;


//
//
//
static BOOL OpenGLFindDriver( char *openglLib ) {

	// HINSTANCE found;
	// BOOL ext;

	openglLib[0] = 0;

	/***
	ext = BOOLEAN(GL_winReadProfileInt("gl_minigl", 1));
	GL_winWriteProfileInt( "gl_minigl", temp );

	if( ext == TRUE ) {

		// attempt to autodetermine opengl driver
		// Voodoo Rush, Voodoo1, Voodoo2

		if( !*openglLib ) {

			if( (found = LoadLibrary("glide2x.dll")) ) {
				strcpy(openglLib, "3dfxogl.dll");
				FreeLibrary( found );
			}
		}

		// PowerVR? Will this work

		if( !*openglLib ) {

			if( (found = LoadLibrary("pvrhal32.dll")) ) {
				strcpy(openglLib, "pvrgl.dll");
				FreeLibrary( found );
			}
		}
	}
	***/

	if( dwinOpengl() == TRUE ) {

#define OFN_FILELEN (256)

		char fileTitle[OFN_FILELEN],currdir[OFN_FILELEN],init_dir[OFN_FILELEN];
		char filter[] = "Executable Files (.exe;.dll;.ocx)\0*.exe;*.dll;*.ocx\0All Files (*.*)\0*.*\0";
		char filename[OFN_FILELEN] = {0,0};
		char text[1024];
		OPENFILENAME ofn;

		GL_winReadProfileString( "gl_icdname", "\0", filename, sizeof(filename) );

		sprintf( text, "Use \"%s\" driver?", filename );

		if( (strlen(filename) < 2) || (MessageBox( (HWND)dGetHwnd(), text , "OpenGL ICD driver", MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION | MB_TOPMOST ) == IDNO) ) {

			memset( filename, 0, sizeof(filename) );
			lstrcpy( fileTitle, "");
			GL_winWriteProfileString( "gl_icdname", filename );

			GetCurrentDirectory( sizeof(currdir), currdir );
			GetSystemDirectory( init_dir, sizeof(init_dir) );

			ofn.lStructSize       = sizeof(OPENFILENAME);
			ofn.hwndOwner	      = (HWND)dGetHwnd();
			ofn.hInstance	      = (HINSTANCE)dGetHinstance();
			ofn.lpstrFilter       = filter;
			ofn.lpstrCustomFilter = (LPSTR)NULL;
			ofn.nMaxCustFilter    = 0L;
			ofn.nFilterIndex      = 1L;
			ofn.lpstrFile	      = filename;
			ofn.nMaxFile	      = sizeof(filename);
			ofn.lpstrFileTitle    = fileTitle;
			ofn.nMaxFileTitle     = sizeof(fileTitle);
			ofn.lpstrInitialDir   = init_dir;
			ofn.lpstrTitle	      = "Select OpenGL ICD driver";
			ofn.nFileOffset       = 0;
			ofn.nFileExtension    = 0;
			ofn.lpstrDefExt       = "*.*";
			ofn.lCustData	      = 0;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;

			if( GetOpenFileName(&ofn) ) {

				SetCurrentDirectory( currdir );

				GL_winWriteProfileString( "gl_icdname", filename );

				strcpy( openglLib, filename );
			}
			else {

				// cancel-t nyomott

				SetCurrentDirectory( currdir );

				GL_winWriteProfileString( "gl_icdname", "" );
				GL_winWriteProfileInt( "xlib_opengl", 0 );
			}
		}
		else
			strcpy( openglLib, filename );

	}

	// Fallback to system opengl/ICD driver
	if( !*openglLib )
		strcpy(openglLib, "opengl32.dll");

	return TRUE;
}



//
//
//
static int errstr( char *funcName ) {

	dprintf("log: no \"%s\" function.\n",funcName);

	dllQuit("OpenGL driver: no \"%s\" function!\n\nSelect another driver please!",funcName);

	return -1;
}


//
//
//
int OpenGLInit( char *lib ) {

	if( OpenGLFindDriver( lib ) == FALSE ) {
		openglInst = NULL;
		return -1;
	}

	/* load dll */

	if( (openglInst = LoadLibrary(lib)) == NULL ) {

		// MessageBox(GetFocus(), "Could not open OpenGL Library", "Error", MB_OK);

		dprintf( "init: Could not open %s library.\n", lib );

		strcpy( lib, "opengl32.dll" );

		if( (openglInst = LoadLibrary(lib)) == NULL ) {
			openglInst = NULL;
			return -1;
		}
	}

	/* load all the function pointers */

	if(!(fnglAlphaFunc	= (void (APIENTRY *)( GLenum, GLclampf ))GetProcAddress(openglInst, "glAlphaFunc"))) /* return */ errstr("glAlphaFunc");
	if(!(fnglBegin		= (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glBegin"))) /* return */ errstr("glBegin");
	if(!(fnglBindTexture	= (void (APIENTRY *)( GLenum, GLuint ))GetProcAddress(openglInst, "glBindTexture"))) /* return */ errstr("glBindTexture");
	if(!(fnglBlendFunc	= (void (APIENTRY *)( GLenum, GLenum ))GetProcAddress(openglInst, "glBlendFunc"))) /* return */ errstr("glBlendFunc");
	if(!(fnglClear		= (void (APIENTRY *)( GLbitfield ))GetProcAddress(openglInst, "glClear"))) /* return */ errstr("glClear");
	if(!(fnglClearColor	= (void (APIENTRY *)( GLclampf, GLclampf, GLclampf, GLclampf ))GetProcAddress(openglInst, "glClearColor"))) /* return */ errstr("glClearColor");
	if(!(fnglClearDepth	= (void (APIENTRY *)( GLclampd ))GetProcAddress(openglInst, "glClearDepth"))) /* return */ errstr("glClearDepth");
	if(!(fnglColor3f	= (void (APIENTRY *)( GLfloat, GLfloat, GLfloat ))GetProcAddress(openglInst, "glColor3f"))) /* return */ errstr("glColor3f");
	if(!(fnglColor3fv	= (void (APIENTRY *)( const GLfloat *))GetProcAddress(openglInst, "glColor3fv"))) /* return */ errstr("glColor3fv");
	if(!(fnglColor4f	= (void (APIENTRY *)( GLfloat, GLfloat, GLfloat, GLfloat ))GetProcAddress(openglInst, "glColor4f"))) /* return */ errstr("glColor4f");
	if(!(fnglColor4fv	= (void (APIENTRY *)( const GLfloat * ))GetProcAddress(openglInst, "glColor4fv"))) /* return */ errstr("glColor4fv");
	if(!(fnglCullFace	= (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glCullFace"))) /* return */ errstr("glCullFace");
	if(!(fnglDepthFunc	= (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glDepthFunc"))) /* return */ errstr("glDepthFunc");
	if(!(fnglDepthMask	= (void (APIENTRY *)( GLboolean ))GetProcAddress(openglInst, "glDepthMask"))) /* return */ errstr("glDepthMask");
	if(!(fnglDepthRange	= (void (APIENTRY *)( GLclampd, GLclampd ))GetProcAddress(openglInst, "glDepthRange"))) /* return */ errstr("glDepthRange");
	if(!(fnglDeleteTextures	= (void (APIENTRY *)( GLsizei, const GLuint * ))GetProcAddress(openglInst, "glDeleteTextures"))) /* return */ errstr("glDeleteTextures");
	if(!(fnglDisable	= (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glDisable"))) /* return */ errstr("glDisable");
	if(!(fnglDrawPixels	= (void (APIENTRY *)( GLsizei,GLsizei,GLenum,GLenum,GLvoid * ))GetProcAddress(openglInst, "glDrawPixels"))) /* return */ errstr("glDrawPixels");
	if(!(fnglEnable		= (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glEnable"))) /* return */ errstr("glEnable");
	if(!(fnglEnd		= (void (APIENTRY *)( void ))GetProcAddress(openglInst, "glEnd"))) /* return */ errstr("glEnd");
	if(!(fnglFinish		= (void (APIENTRY *)( void ))GetProcAddress(openglInst, "glFinish"))) /* return */ errstr("glFinish");
	if(!(fnglFlush		= (void (APIENTRY *)( void ))GetProcAddress(openglInst, "glFlush"))) /* return */ errstr("glFlush");
	if(!(fnglFogf		= (void (APIENTRY *)( GLenum, GLfloat ))GetProcAddress(openglInst, "glFogf"))) /* return */ errstr("glFogf");
	if(!(fnglFogfv		= (void (APIENTRY *)( GLenum, const GLfloat * ))GetProcAddress(openglInst, "glFogfv"))) /* return */ errstr("glFogfv");
	if(!(fnglFogi		= (void (APIENTRY *)( GLenum, GLint ))GetProcAddress(openglInst, "glFogi"))) /* return */ errstr("glFogi");
	if(!(fnglFrontFace	= (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glFrontFace"))) /* return */ errstr("glFrontFace");
	if(!(fnglFrustum	= (void (APIENTRY *)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble))GetProcAddress(openglInst, "glFrustum"))) /* return */ errstr("glFrustum");
	if(!(fnglGetError	= (GLenum (APIENTRY *)( void ))GetProcAddress(openglInst, "glGetError"))) /* return */ errstr("glGetError");
	if(!(fnglGetFloatv	= (void (APIENTRY *)( GLenum, GLfloat * ))GetProcAddress(openglInst, "glGetFloatv"))) /* return */ errstr("glGetFloatv");
	if(!(fnglGetIntegerv   = (void (APIENTRY *)( GLenum, GLint * ))GetProcAddress(openglInst, "glGetIntegerv"))) /* return */ errstr("glGetIntegerv");
	if(!(fnglGetString     = (const GLubyte * (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glGetString"))) /* return */ errstr("glGetString");
	if(!(fnglGenTextures   = (void (APIENTRY *)( GLsizei, GLuint * ))GetProcAddress(openglInst, "glGenTextures"))) /* return */ errstr("glGenTextures");
	if(!(fnglHint          = (void (APIENTRY *)( GLenum, GLenum ))GetProcAddress(openglInst, "glHint"))) /* return */ errstr("glHint");
	if(!(fnglIsEnabled     = (GLboolean (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glIsEnabled"))) /* return */ errstr("glIsEnabled");
	if(!(fnglLoadIdentity  = (void (APIENTRY *)( void ))GetProcAddress(openglInst, "glLoadIdentity"))) /* return */ errstr("glLoadIdentity");
	if(!(fnglLoadMatrixf   = (void (APIENTRY *)( const GLfloat * ))GetProcAddress(openglInst, "glLoadMatrixf"))) /* return */ errstr("glLoadMatrixf");
	if(!(fnglMatrixMode    = (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glMatrixMode"))) /* return */ errstr("glMatrixMode");
	if(!(fnglMultMatrixf   = (void (APIENTRY *)( const GLfloat * ))GetProcAddress(openglInst, "glMultMatrixf"))) /* return */ errstr("glMultMatrixf");
	if(!(fnglOrtho         = (void (APIENTRY *)( GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble ))GetProcAddress(openglInst, "glOrtho"))) /* return */ errstr("glOrtho");
	if(!(fnglPixelStorei   = (void (APIENTRY *)( GLenum , GLint ))GetProcAddress(openglInst, "glPixelStorei"))) /* return */ errstr("glPixelStorei");
	if(!(fnglPolygonOffset = (void (APIENTRY *)( GLfloat, GLfloat ))GetProcAddress(openglInst, "glPolygonOffset"))) /* return */ errstr("glPolygonOffset");
	if(!(fnglPolygonMode   = (void (APIENTRY *)( GLenum, GLenum ))GetProcAddress(openglInst, "glPolygonMode"))) /* return */ errstr("glPolygonMode");
	if(!(fnglPopMatrix     = (void (APIENTRY *)( void ))GetProcAddress(openglInst, "glPopMatrix"))) /* return */ errstr("glPopMatrix");
	if(!(fnglPushMatrix    = (void (APIENTRY *)( void ))GetProcAddress(openglInst, "glPushMatrix"))) /* return */ errstr("glPushMatrix");
	if(!(fnglRotatef       = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat, GLfloat))GetProcAddress(openglInst, "glRotatef"))) /* return */ errstr("glRotatef");
	if(!(fnglReadBuffer    = (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glReadBuffer"))) /* return */ errstr("glReadBuffer");
	if(!(fnglReadPixels    = (void (APIENTRY *)( GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,GLvoid *))GetProcAddress(openglInst, "glReadPixels"))) /* return */ errstr("glReadPixels");
	if(!(fnglScalef        = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat))GetProcAddress(openglInst, "glScalef"))) /* return */ errstr("glScalef");
	if(!(fnglShadeModel    = (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glShadeModel"))) /* return */ errstr("glShadeModel");
	if(!(fnglTexCoord2f    = (void (APIENTRY *)( GLfloat, GLfloat))GetProcAddress(openglInst, "glTexCoord2f"))) /* return */ errstr("glTexCoord2f");
	if(!(fnglTexCoord2fv   = (void (APIENTRY *)( const GLfloat *))GetProcAddress(openglInst, "glTexCoord2fv"))) /* return */ errstr("glTexCoord2fv");
	if(!(fnglTexCoord3f    = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat))GetProcAddress(openglInst, "glTexCoord3f"))) /* return */ errstr("glTexCoord3f");
	if(!(fnglTexCoord3fv   = (void (APIENTRY *)( const GLfloat *))GetProcAddress(openglInst, "glTexCoord3fv"))) /* return */ errstr("glTexCoord3fv");
	if(!(fnglTexCoord4f    = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat, GLfloat ))GetProcAddress(openglInst, "glTexCoord4f"))) /* return */ errstr("glTexCoord4f");
	if(!(fnglTexEnvf       = (void (APIENTRY *)( GLenum , GLenum , GLfloat ))GetProcAddress(openglInst, "glTexEnvf"))) /* return */ errstr("glTexEnvf");
	if(!(fnglTexImage2D    = (void (APIENTRY *)( GLenum, GLint, GLint, GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid * ))GetProcAddress(openglInst, "glTexImage2D"))) /* return */ errstr("glTexImage2D");
	if(!(fnglTexParameterf = (void (APIENTRY *)( GLenum, GLenum, GLfloat ))GetProcAddress(openglInst, "glTexParameterf"))) /* return */ errstr("glTexParameterf");
	if(!(fnglTranslatef    = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat))GetProcAddress(openglInst, "glTranslatef"))) /* return */ errstr("glTranslatef");
	if(!(fnglVertex2f      = (void (APIENTRY *)( GLfloat, GLfloat ))GetProcAddress(openglInst, "glVertex2f"))) /* return */ errstr("glVertex2f");
	if(!(fnglVertex3f      = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat))GetProcAddress(openglInst, "glVertex3f"))) /* return */ errstr("glVertex3f");
	if(!(fnglVertex3fv     = (void (APIENTRY *)( const GLfloat * ))GetProcAddress(openglInst, "glVertex3fv"))) /* return */ errstr("glVertex3fv");
	if(!(fnglVertex4f      = (void (APIENTRY *)( GLfloat, GLfloat, GLfloat, GLfloat ))GetProcAddress(openglInst, "glVertex4f"))) /* return */ errstr("glVertex4f");
	if(!(fnglViewport      = (void (APIENTRY *)( GLint, GLint, GLsizei, GLsizei ))GetProcAddress(openglInst, "glViewport"))) /* return */ errstr("glViewport");
	if(!(fnglScissor       = (void (APIENTRY *)( GLint, GLint, GLsizei , GLsizei ))GetProcAddress(openglInst, "glScissor"))) /* return */ errstr("glScissor");
	if(!(fnglEnableClientState = (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glEnableClientState"))) /* return */ errstr("glEnableClientState");
	if(!(fnglDisableClientState = (void (APIENTRY *)( GLenum ))GetProcAddress(openglInst, "glDisableClientState"))) /* return */ errstr("glDisableClientState");

	/*
	 * The 3dfxgl.dll minidriver bypasses the GDI entirely
	 * I do a proper failover, but the GDI failover call for
	 * SwapBuffers is REALLY bloody slow, so best to avoid it
	 * alltogether. (when using winglide for debugging I was
	 * going from 8 fps to 5fps, thats 37.5%!!!!)
	 */

	if( strcmp(lib, "opengl32.dll") )
		openglBypassGDI = TRUE;

	if( openglInst )
		fnSwapBuffers = (BOOL (APIENTRY *)(HDC))GetProcAddress(openglInst, "wglSwapBuffers");

	/*
	 * The Matrox D3D wrapper fails completely unless the GDI
	 * is bypassed? but glGetString doesnt work until after
	 * MakeCurrentContext, how do I detect?
	 */

	/* Cant do anything here, couldnt get the call to work
	 * in MakeCurrentContext
	 */

	/* going to try remembering the results of the pixel
	 * format calls
	 */

	return 0;
}


//
//
//
void OpenGLUnInit( void ) {

	if( openglInst != NULL )
		FreeLibrary(openglInst);

	openglInst = NULL;

	return;
}


//
//
//
void APIENTRY glAlphaFunc( GLenum func, GLclampf ref ) {

	if( fnglAlphaFunc )
		(*fnglAlphaFunc)(func,ref);

	return;
}


//
//
//
void APIENTRY glBegin( GLenum mode ) {

	if( fnglBegin )
		(*fnglBegin)(mode);

	return;
}


//
//
//
void APIENTRY glBindTexture( GLenum target, GLuint texture ) {

	if( fnglBindTexture )
		(*fnglBindTexture)(target,texture);

	return;
}



//
//
//
void APIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor ) {

	if( fnglBlendFunc )
		(*fnglBlendFunc)(sfactor,dfactor);

	return;
}


//
//
//
void APIENTRY glClear( GLbitfield mask ) {

	if( fnglClear )
		(*fnglClear)(mask);

	return;
}


//
//
//
void APIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {

	if( fnglClearColor )
		(*fnglClearColor)(red,green,blue,alpha);

	return;
}


//
//
//
void APIENTRY glClearDepth( GLclampd depth ) {

	if( fnglClearDepth )
		(*fnglClearDepth)(depth);

	return;
}


//
//
//
void APIENTRY glColor3f( GLfloat red, GLfloat green, GLfloat blue ) {

	if( fnglColor3f )
		(*fnglColor3f)(red,green,blue);

	return;
}


//
//
//
void APIENTRY glColor3fv( const GLfloat *v ) {

	if( fnglColor3fv )
		(*fnglColor3fv)(v);

	return;
}


//
//
//
void APIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {

	if( fnglColor4f )
		(*fnglColor4f)(red,green,blue,alpha);

	return;
}



//
//
//
void APIENTRY glColor4fv( const GLfloat *v ) {

	if( fnglColor4fv )
		(*fnglColor4fv)(v);

	return;
}


//
//
//
void APIENTRY glCullFace( GLenum mode ) {

	if( fnglCullFace )
		(*fnglCullFace)(mode);

	return;
}


//
//
//
void APIENTRY glDepthFunc( GLenum func ) {

	if( fnglDepthFunc )
		(*fnglDepthFunc)(func);

	return;
}


//
//
//
void APIENTRY glDepthMask( GLboolean flag ) {

	if( fnglDepthMask )
		(*fnglDepthMask)(flag);

	return;
}


//
//
//
void APIENTRY glDepthRange( GLclampd zNear, GLclampd zFar ) {

	if( fnglDepthRange )
		(*fnglDepthRange)(zNear,zFar);

	return;
}


//
//
//
void APIENTRY glDeleteTextures( GLsizei n, const GLuint *textures ) {

	if( fnglDeleteTextures )
		(*fnglDeleteTextures)(n,textures);

	return;
}


//
//
//
void APIENTRY glDisable( GLenum cap ) {

	if( fnglDisable )
		(*fnglDisable)( cap );

	return;
}


//
//
//
void APIENTRY glDrawPixels( GLsizei width,GLsizei height,GLenum format,GLenum type, const GLvoid *pixels ) {

	if( fnglDrawPixels )
		(*fnglDrawPixels)( width,height, format, type, pixels );

	return;
}


//
//
//
void APIENTRY glEnable( GLenum cap ) {

	if( fnglEnable )
		(*fnglEnable)( cap );

	return;
}


//
//
//
void APIENTRY glEnd( void ) {

	if( fnglEnd )
		(*fnglEnd)();

	return;
}


//
//
//
void APIENTRY glFinish( void ) {

	if( fnglFinish )
		(*fnglFinish)();

	return;
}


//
//
//
void APIENTRY glFlush( void ) {

	if( fnglFlush )
		(*fnglFlush)();

	return;
}


//
//
//
void APIENTRY glFogfv( GLenum pname, const GLfloat *params ) {

	if( fnglFogfv )
		(*fnglFogfv)(pname,params);

	return;
}


//
//
//
void APIENTRY glFogf( GLenum pname, GLfloat param ) {

	if( fnglFogf )
		(*fnglFogf)(pname,param);

	return;
}


//
//
//
void APIENTRY glFogi( GLenum pname, GLint param ) {

	if( fnglFogi )
		(*fnglFogi)( pname, param );

	return;
}


//
//
//
void APIENTRY glFrontFace( GLenum mode ) {

	if( fnglFrontFace )
		(*fnglFrontFace)(mode);

	return;
}


//
//
//
void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {

	if( fnglFrustum )
		(*fnglFrustum)(left,right,bottom,top,zNear,zFar);

	return;
}


//
//
//
GLenum APIENTRY glGetError( void ) {

	if( fnglGetError )
		return (*fnglGetError)();

	return 0;
}



//
//
//
void APIENTRY glGetFloatv( GLenum pname, GLfloat *params ) {

	if( fnglGetFloatv )
		(*fnglGetFloatv)( pname, params );

	return;
}


//
//
//
void APIENTRY glGetIntegerv( GLenum pname, GLint *params ) {

	if( fnglGetIntegerv )
		(*fnglGetIntegerv)( pname, params );

	return;
}


//
//
//
const GLubyte * APIENTRY glGetString( GLenum name ) {

  	static char *null_string = "null";

	if( fnglGetString )
		return (*fnglGetString)( name );

	return (const GLubyte *)null_string;
}


//
//
//
void APIENTRY glGenTextures( GLsizei n, GLuint *textures ) {

	if( fnglGenTextures )
		(*fnglGenTextures)( n, textures );

	return;
}


//
//
//
void APIENTRY glHint( GLenum target, GLenum mode ) {

	if( fnglHint )
		(*fnglHint)( target, mode );

	return;
}



//
//
//
GLboolean APIENTRY glIsEnabled( GLenum cap ) {

	if( fnglIsEnabled )
		return (*fnglIsEnabled)( cap );

	return 0;
}


//
//
//
void APIENTRY glLoadIdentity( void ) {

	if( fnglLoadIdentity )
		(*fnglLoadIdentity)();

	return;
}


//
//
//
void APIENTRY glLoadMatrixf( const GLfloat *m ) {

	if( fnglLoadMatrixf )
		(*fnglLoadMatrixf)( m );

	return;
}


//
//
//
void APIENTRY glMatrixMode( GLenum mode ) {

	if( fnglMatrixMode )
		(*fnglMatrixMode)(mode);

	return;
}


//
//
//
void APIENTRY glMultMatrixf( const GLfloat *m ) {

	if( fnglMultMatrixf )
		(*fnglMultMatrixf)(m);

	return;
}


//
//
//
void APIENTRY glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar ) {

	if( fnglOrtho )
		(*fnglOrtho)(left,right,bottom,top,zNear,zFar);

	return;
}


//
//
//
void APIENTRY glPixelStorei( GLenum pname, GLint param ) {

	if( fnglPixelStorei )
		(*fnglPixelStorei)( pname, param );

	return;
}


//
//
//
void APIENTRY glPolygonOffset( GLfloat factor, GLfloat units ) {

	if( fnglPolygonOffset )
		(*fnglPolygonOffset)( factor, units );

	return;
}


//
//
//
void APIENTRY glPolygonMode( GLenum face, GLenum mode ) {

	if( fnglPolygonMode )
		(*fnglPolygonMode)( face, mode );

	return;
}


//
//
//
void APIENTRY glPopMatrix( void ) {

	if( fnglPopMatrix )
		(*fnglPopMatrix)();

	return;
}


//
//
//
void APIENTRY glPushMatrix( void ) {

	if( fnglPushMatrix )
		(*fnglPushMatrix)();

	return;
}


//
//
//
void APIENTRY glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {

	if( fnglRotatef )
		(*fnglRotatef)(angle,x,y,z);

	return;
}


//
//
//
void APIENTRY glReadBuffer( GLenum mode ) {

	if( fnglReadBuffer )
		(*fnglReadBuffer)(mode);

	return;
}


//
//
//
void APIENTRY glReadPixels( GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,GLvoid *pixels ) {

	if( fnglReadPixels )
		(*fnglReadPixels)( x,y, width,height, format, type, pixels );

	return;
}


//
//
//
void APIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z ) {

	if( fnglScalef )
		(*fnglScalef)(x,y,z);

	return;
}


//
//
//
void APIENTRY glShadeModel( GLenum mode ) {

	if( fnglShadeModel )
		(*fnglShadeModel)(mode);

	return;
}


//
//
//
void APIENTRY glTexCoord2f( GLfloat s, GLfloat t ) {

	if( fnglTexCoord2f )
		(*fnglTexCoord2f)(s,t);

	return;
}


//
//
//
void APIENTRY glTexCoord2fv( const GLfloat *v ) {

	if( fnglTexCoord2fv )
		(*fnglTexCoord2fv)(v);

	return;
}



//
//
//
void APIENTRY glTexCoord3f( GLfloat s, GLfloat t, GLfloat r ) {

	if( fnglTexCoord3f )
		(*fnglTexCoord3f)(s,t,r);

	return;
}



//
//
//
void APIENTRY glTexCoord3fv( const GLfloat *v ) {

	if( fnglTexCoord3fv )
		(*fnglTexCoord3fv)(v);

	return;
}


//
//
//
void APIENTRY glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) {

	if( fnglTexCoord4f )
		(*fnglTexCoord4f)(s,t,r,q);

	return;
}



//
//
//
void APIENTRY glTexEnvf( GLenum target, GLenum pname, GLfloat param ) {

	if( fnglTexEnvf )
		(*fnglTexEnvf)(target,pname,param);

	return;
}


//
//
//
void APIENTRY glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {

	if( fnglTexImage2D )
		(*fnglTexImage2D)( target,level,internalformat,width,height,border,format,type,pixels );

	return;
}


//
//
//
void APIENTRY glTexParameterf( GLenum target, GLenum pname, GLfloat param ) {

	if( fnglTexParameterf )
		(*fnglTexParameterf)(target,pname,param);

	return;
}


//
//
//
void APIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z ) {

	if( fnglTranslatef )
		(*fnglTranslatef)(x,y,z);

	return;
}


//
//
//
void APIENTRY glVertex2f( GLfloat x, GLfloat y ) {

	if( fnglVertex2f )
		(*fnglVertex2f)(x,y);

	return;
}



//
//
//
void APIENTRY glVertex3f( GLfloat x, GLfloat y, GLfloat z ) {

	if( fnglVertex3f )
		(*fnglVertex3f)(x,y,z);

	return;
}


//
//
//
void APIENTRY glVertex3fv( const GLfloat *v ) {

	if( fnglVertex3fv )
		(*fnglVertex3fv)(v);

	return;
}


//
//
//
void APIENTRY glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) {

	if( fnglVertex4f )
		(*fnglVertex4f)(x,y,z,w);

	return;
}


//
//
//
void APIENTRY glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {

	if( fnglViewport )
		(*fnglViewport)(x,y,width,height);

	return;
}



//
//
//
void APIENTRY glScissor( GLint x, GLint y, GLsizei width, GLsizei height ) {

	if( fnglScissor )
		(*fnglScissor)(x,y,width,height);

	return;
}


//
//
//
void APIENTRY glEnableClientState( GLenum array ) {

	if( fnglEnableClientState )
		(*fnglEnableClientState)(array);

	return;
}

//
//
//
void APIENTRY glDisableClientState( GLenum array ) {

	if( fnglDisableClientState )
		(*fnglDisableClientState)(array);

	return;
}



/******************************************************************************/
/* replaced WGL* GDI functions */

/*
 * These cant? be fixed like the above functions becuase they
 * are declared in gdi.h and for some reason the compiler will
 * insist on declaring them as EXPORTED dll functions... wierd
 */
HGLRC APIENTRY OpenGLCreateContext( HDC hdc ) {

	HGLRC (APIENTRY *fnwglCreateContext)(HDC hdc);

	if( !openglInst )
		return NULL;

	fnwglCreateContext = (HGLRC (APIENTRY *)(HDC))GetProcAddress(openglInst, "wglCreateContext");

	if( !fnwglCreateContext )
		return NULL;

	return (*fnwglCreateContext)(hdc);
}



//
//
//
BOOL APIENTRY OpenGLDeleteContext(HGLRC hglrc) {

	BOOL (APIENTRY *fnwglDeleteContext)(HGLRC hglrc);

	if( !openglInst )
		return FALSE;

	fnwglDeleteContext = (BOOL (APIENTRY *)(HGLRC))GetProcAddress(openglInst, "wglDeleteContext");

	if( !fnwglDeleteContext )
		return FALSE;

	return (*fnwglDeleteContext)( hglrc );
}



//
//
//
BOOL APIENTRY OpenGLMakeCurrent(HDC hdc, HGLRC hglrc) {

	BOOL (APIENTRY *fnwglMakeCurrent)(HDC hdc, HGLRC hglrc);

	if( !openglInst )
		return FALSE;

	fnwglMakeCurrent = (BOOL (APIENTRY *)(HDC, HGLRC))GetProcAddress(openglInst, "wglMakeCurrent");

	if( !fnwglMakeCurrent )
		return FALSE;

	return (*fnwglMakeCurrent)( hdc, hglrc );
}



//
//
//
BOOL APIENTRY OpenGLSwapLayerBuffers( HDC hdc, UINT fuPlanes ) {

	BOOL (APIENTRY *fnwglSwapLayerBuffers)(HDC hdc, UINT fuPlanes);

  	if( !openglInst )
		return FALSE;

	fnwglSwapLayerBuffers = (BOOL (APIENTRY *)(HDC, UINT))GetProcAddress(openglInst, "wglSwapLayerBuffers");

	if( !fnwglSwapLayerBuffers )
		return FALSE;

	return (*fnwglSwapLayerBuffers)( hdc, fuPlanes );
}



//
//
//
BOOL APIENTRY OpenGLSwapBuffers( HDC hdc ) {

  	if( !openglBypassGDI ) {

		// Try GDI version first

		int retValue;

		retValue = SwapBuffers(hdc);

		if( retValue )
			return retValue;
	}

	if( !fnSwapBuffers )
		return FALSE;

	return (*fnSwapBuffers)( hdc );
}



//
//
//
int APIENTRY OpenGLChoosePixelFormat( HDC hdc, CONST PIXELFORMATDESCRIPTOR * ppfd ) {

	int (APIENTRY *fnChoosePixelFormat)(HDC hdc, CONST PIXELFORMATDESCRIPTOR * ppfd);

	if( !openglBypassGDI ) {

		// Try GDI version first

		int retValue;

		retValue = ChoosePixelFormat(hdc, ppfd);

		if( retValue )
			return retValue;
	}

	// Bypass GDI entirely
	if( !openglInst )
		return FALSE;

	fnChoosePixelFormat = (int (APIENTRY *)(HDC, CONST PIXELFORMATDESCRIPTOR *))GetProcAddress(openglInst, "wglChoosePixelFormat");

	if( !fnChoosePixelFormat )
		return FALSE;

	openglBypassGDI = TRUE;

	return (*fnChoosePixelFormat)(hdc, ppfd);
}


//
//
//
int APIENTRY OpenGLDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd) {

	int (APIENTRY *fnDescribePixelFormat)(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);

	if( !openglBypassGDI ) {

		// Try GDI version first

		int retValue;

		retValue = DescribePixelFormat(hdc, iPixelFormat, nBytes, ppfd);

		if( retValue )
			return retValue;
	}

	// Bypass GDI entirely

	if( !openglInst )
		return FALSE;

	fnDescribePixelFormat = (int (APIENTRY *)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR))GetProcAddress(openglInst, "wglDescribePixelFormat");

	if( !fnDescribePixelFormat )
		return FALSE;

	openglBypassGDI = TRUE;

	return (*fnDescribePixelFormat)(hdc, iPixelFormat, nBytes, ppfd);
}


//
//
//
BOOL  APIENTRY OpenGLSetPixelFormat(HDC hdc, int iPixelFormat, CONST PIXELFORMATDESCRIPTOR * ppfd) {

	BOOL (APIENTRY *fnSetPixelFormat)(HDC hdc, int iPixelFormat, CONST PIXELFORMATDESCRIPTOR * ppfd);

	if( !openglBypassGDI ) {

		// Try GDI version first

		int retValue;

		retValue = SetPixelFormat(hdc, iPixelFormat, ppfd);

		if( retValue )
			return retValue;
	}

	// Bypass GDI entirely

	if( !openglInst )
		return FALSE;

	fnSetPixelFormat = (BOOL (APIENTRY *)(HDC, int, CONST PIXELFORMATDESCRIPTOR *))GetProcAddress(openglInst, "wglSetPixelFormat");

	if( !fnSetPixelFormat )
		return FALSE;

	openglBypassGDI = TRUE;

	return (*fnSetPixelFormat)( hdc, iPixelFormat, ppfd );
}

