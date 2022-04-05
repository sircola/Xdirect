
#include <windows.h>

#include <al.h>
#include <alc.h>
#include <altypes.h>
#include <alctypes.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <xlib.h>

RCSID( "$Id: openalshell.cpp,v 1.0 2011/08/26 23:55:30 bernie Exp $" )



static UINT TimerId = 0;


// AL 1.0 did not define the ALchar and ALCchar types, so define them here
// if they don't exist

#ifndef ALchar
#define ALchar char
#endif

#ifndef ALCchar
#define ALCchar char
#endif

#define ALAPIENTRY __cdecl
#define AL_CALLBACK

#define ALCAPIENTRY __cdecl

// Complete list of functions available in AL 1.0 implementations

typedef void           (ALAPIENTRY *LPALENABLE)( ALenum capability );
typedef void           (ALAPIENTRY *LPALDISABLE)( ALenum capability );
typedef ALboolean      (ALAPIENTRY *LPALISENABLED)( ALenum capability );
typedef const ALchar*  (ALAPIENTRY *LPALGETSTRING)( ALenum param );
typedef void           (ALAPIENTRY *LPALGETBOOLEANV)( ALenum param, ALboolean* data );
typedef void           (ALAPIENTRY *LPALGETINTEGERV)( ALenum param, ALint* data );
typedef void           (ALAPIENTRY *LPALGETFLOATV)( ALenum param, ALfloat* data );
typedef void           (ALAPIENTRY *LPALGETDOUBLEV)( ALenum param, ALdouble* data );
typedef ALboolean      (ALAPIENTRY *LPALGETBOOLEAN)( ALenum param );
typedef ALint          (ALAPIENTRY *LPALGETINTEGER)( ALenum param );
typedef ALfloat        (ALAPIENTRY *LPALGETFLOAT)( ALenum param );
typedef ALdouble       (ALAPIENTRY *LPALGETDOUBLE)( ALenum param );
typedef ALenum         (ALAPIENTRY *LPALGETERROR)( void );
typedef ALboolean      (ALAPIENTRY *LPALISEXTENSIONPRESENT)(const ALchar* extname );
typedef void*          (ALAPIENTRY *LPALGETPROCADDRESS)( const ALchar* fname );
typedef ALenum         (ALAPIENTRY *LPALGETENUMVALUE)( const ALchar* ename );
typedef void           (ALAPIENTRY *LPALLISTENERF)( ALenum param, ALfloat value );
typedef void           (ALAPIENTRY *LPALLISTENER3F)( ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void           (ALAPIENTRY *LPALLISTENERFV)( ALenum param, const ALfloat* values );
typedef void           (ALAPIENTRY *LPALLISTENERI)( ALenum param, ALint value );
typedef void           (ALAPIENTRY *LPALGETLISTENERF)( ALenum param, ALfloat* value );
typedef void           (ALAPIENTRY *LPALGETLISTENER3F)( ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3 );
typedef void           (ALAPIENTRY *LPALGETLISTENERFV)( ALenum param, ALfloat* values );
typedef void           (ALAPIENTRY *LPALGETLISTENERI)( ALenum param, ALint* value );
typedef void           (ALAPIENTRY *LPALGENSOURCES)( ALsizei n, ALuint* sources );
typedef void           (ALAPIENTRY *LPALDELETESOURCES)( ALsizei n, const ALuint* sources );
typedef ALboolean      (ALAPIENTRY *LPALISSOURCE)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEF)( ALuint sid, ALenum param, ALfloat value);
typedef void           (ALAPIENTRY *LPALSOURCE3F)( ALuint sid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 );
typedef void           (ALAPIENTRY *LPALSOURCEFV)( ALuint sid, ALenum param, const ALfloat* values );
typedef void           (ALAPIENTRY *LPALSOURCEI)( ALuint sid, ALenum param, ALint value);
typedef void           (ALAPIENTRY *LPALGETSOURCEF)( ALuint sid, ALenum param, ALfloat* value );
typedef void           (ALAPIENTRY *LPALGETSOURCE3F)( ALuint sid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
typedef void           (ALAPIENTRY *LPALGETSOURCEFV)( ALuint sid, ALenum param, ALfloat* values );
typedef void           (ALAPIENTRY *LPALGETSOURCEI)( ALuint sid, ALenum param, ALint* value );
typedef void           (ALAPIENTRY *LPALSOURCEPLAYV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCESTOPV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCEREWINDV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCEPAUSEV)( ALsizei ns, const ALuint *sids );
typedef void           (ALAPIENTRY *LPALSOURCEPLAY)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCESTOP)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEREWIND)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEPAUSE)( ALuint sid );
typedef void           (ALAPIENTRY *LPALSOURCEQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, const ALuint *bids );
typedef void           (ALAPIENTRY *LPALSOURCEUNQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, ALuint *bids );
typedef void           (ALAPIENTRY *LPALGENBUFFERS)( ALsizei n, ALuint* buffers );
typedef void           (ALAPIENTRY *LPALDELETEBUFFERS)( ALsizei n, const ALuint* buffers );
typedef ALboolean      (ALAPIENTRY *LPALISBUFFER)( ALuint bid );
typedef void           (ALAPIENTRY *LPALBUFFERDATA)( ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq );
typedef void           (ALAPIENTRY *LPALGETBUFFERF)( ALuint bid, ALenum param, ALfloat* value );
typedef void           (ALAPIENTRY *LPALGETBUFFERI)( ALuint bid, ALenum param, ALint* value );
typedef void           (ALAPIENTRY *LPALDOPPLERFACTOR)( ALfloat value );
typedef void           (ALAPIENTRY *LPALDOPPLERVELOCITY)( ALfloat value );
typedef void           (ALAPIENTRY *LPALDISTANCEMODEL)( ALenum distanceModel );
typedef ALCcontext *   (ALCAPIENTRY *LPALCCREATECONTEXT) (ALCdevice *device, const ALCint *attrlist);
typedef ALCboolean     (ALCAPIENTRY *LPALCMAKECONTEXTCURRENT)( ALCcontext *context );
typedef void           (ALCAPIENTRY *LPALCPROCESSCONTEXT)( ALCcontext *context );
typedef void           (ALCAPIENTRY *LPALCSUSPENDCONTEXT)( ALCcontext *context );
typedef void           (ALCAPIENTRY *LPALCDESTROYCONTEXT)( ALCcontext *context );
typedef ALCcontext *   (ALCAPIENTRY *LPALCGETCURRENTCONTEXT)( ALvoid );
typedef ALCdevice *    (ALCAPIENTRY *LPALCGETCONTEXTSDEVICE)( ALCcontext *context );
typedef ALCdevice *    (ALCAPIENTRY *LPALCOPENDEVICE)( const ALCchar *devicename );
typedef ALCboolean     (ALCAPIENTRY *LPALCCLOSEDEVICE)( ALCdevice *device );
typedef ALCenum        (ALCAPIENTRY *LPALCGETERROR)( ALCdevice *device );
typedef ALCboolean     (ALCAPIENTRY *LPALCISEXTENSIONPRESENT)( ALCdevice *device, const ALCchar *extname );
typedef void *         (ALCAPIENTRY *LPALCGETPROCADDRESS)(ALCdevice *device, const ALCchar *funcname );
typedef ALCenum        (ALCAPIENTRY *LPALCGETENUMVALUE)(ALCdevice *device, const ALCchar *enumname );
typedef const ALCchar* (ALCAPIENTRY *LPALCGETSTRING)( ALCdevice *device, ALCenum param );
typedef void           (ALCAPIENTRY *LPALCGETINTEGERV)( ALCdevice *device, ALCenum param, ALCsizei size, ALCint *dest );


static LPALENABLE			_alEnable;
static LPALDISABLE                      _alDisable;
static LPALISENABLED                    _alIsEnabled;
static LPALGETBOOLEAN                   _alGetBoolean;
static LPALGETINTEGER                   _alGetInteger;
static LPALGETFLOAT                     _alGetFloat;
static LPALGETDOUBLE                    _alGetDouble;
static LPALGETBOOLEANV                  _alGetBooleanv;
static LPALGETINTEGERV                  _alGetIntegerv;
static LPALGETFLOATV                    _alGetFloatv;
static LPALGETDOUBLEV                   _alGetDoublev;
static LPALGETSTRING                    _alGetString;
static LPALGETERROR                     _alGetError;
static LPALISEXTENSIONPRESENT		_alIsExtensionPresent;
static LPALGETPROCADDRESS               _alGetProcAddress;
static LPALGETENUMVALUE                 _alGetEnumValue;
static LPALLISTENERI                    _alListeneri;
static LPALLISTENERF                    _alListenerf;
static LPALLISTENER3F                   _alListener3f;
static LPALLISTENERFV                   _alListenerfv;
static LPALGETLISTENERI                 _alGetListeneri;
static LPALGETLISTENERF                 _alGetListenerf;
static LPALGETLISTENER3F                _alGetListener3f;
static LPALGETLISTENERFV                _alGetListenerfv;
static LPALGENSOURCES                   _alGenSources;
static LPALDELETESOURCES                _alDeleteSources;
static LPALISSOURCE                     _alIsSource;
static LPALSOURCEI                      _alSourcei;
static LPALSOURCEF                      _alSourcef;
static LPALSOURCE3F                     _alSource3f;
static LPALSOURCEFV                     _alSourcefv;
static LPALGETSOURCEI                   _alGetSourcei;
static LPALGETSOURCEF                   _alGetSourcef;
static LPALGETSOURCEFV                  _alGetSourcefv;
static LPALSOURCEPLAYV                  _alSourcePlayv;
static LPALSOURCESTOPV                  _alSourceStopv;
static LPALSOURCEPLAY                   _alSourcePlay;
static LPALSOURCEPAUSE                  _alSourcePause;
static LPALSOURCESTOP                   _alSourceStop;
static LPALGENBUFFERS                   _alGenBuffers;
static LPALDELETEBUFFERS                _alDeleteBuffers;
static LPALISBUFFER                     _alIsBuffer;
static LPALBUFFERDATA                   _alBufferData;
static LPALGETBUFFERI                   _alGetBufferi;
static LPALGETBUFFERF                   _alGetBufferf;
static LPALSOURCEQUEUEBUFFERS          	_alSourceQueueBuffers;
static LPALSOURCEUNQUEUEBUFFERS        	_alSourceUnqueueBuffers;
static LPALDISTANCEMODEL                _alDistanceModel;
static LPALDOPPLERFACTOR                _alDopplerFactor;
static LPALDOPPLERVELOCITY              _alDopplerVelocity;
static LPALCGETSTRING                   _alcGetString;
static LPALCGETINTEGERV                 _alcGetIntegerv;
static LPALCOPENDEVICE                  _alcOpenDevice;
static LPALCCLOSEDEVICE                 _alcCloseDevice;
static LPALCCREATECONTEXT               _alcCreateContext;
static LPALCMAKECONTEXTCURRENT         	_alcMakeContextCurrent;
static LPALCPROCESSCONTEXT              _alcProcessContext;
static LPALCGETCURRENTCONTEXT          	_alcGetCurrentContext;
static LPALCGETCONTEXTSDEVICE          	_alcGetContextsDevice;
static LPALCSUSPENDCONTEXT              _alcSuspendContext;
static LPALCDESTROYCONTEXT              _alcDestroyContext;
static LPALCGETERROR                    _alcGetError;
static LPALCISEXTENSIONPRESENT         	_alcIsExtensionPresent;
static LPALCGETPROCADDRESS              _alcGetProcAddress;
static LPALCGETENUMVALUE                _alcGetEnumValue;




#define OPENALDLL "OPENAL32.DLL"

static HINSTANCE hOpenALDLL = NULL;

#define LOADOPENALFUNC(func) { 								\
*((void**)&_##func)=(void*)GetProcAddress(hOpenALDLL,#func); 				\
if( _##func == NULL ) { 								\
	xprintf("LoadOpenAL: failed to retrieve '%s' function address.\n",#func); 	\
	if( hOpenALDLL ) FreeLibrary( hOpenALDLL ); 					\
	hOpenALDLL = NULL; 								\
	return FALSE; 									\
} }



//
//
//
static BOOL LoadOpenAL( void ) {

	if( hOpenALDLL )
		return TRUE;

        if( (hOpenALDLL = LoadLibrary(OPENALDLL)) == NULL ) {
		xprintf( "LoadOpenAL: can't load %s.\n", OPENALDLL );
		return FALSE;
	}

        // Get function pointers
	LOADOPENALFUNC(alEnable);
	LOADOPENALFUNC(alDisable);
	LOADOPENALFUNC(alIsEnabled);
	LOADOPENALFUNC(alGetBoolean);
	LOADOPENALFUNC(alGetInteger);
	LOADOPENALFUNC(alGetFloat);
	LOADOPENALFUNC(alGetDouble);
	LOADOPENALFUNC(alGetBooleanv);
	LOADOPENALFUNC(alGetIntegerv);
	LOADOPENALFUNC(alGetFloatv);
	LOADOPENALFUNC(alGetDoublev);
	LOADOPENALFUNC(alGetString);
	LOADOPENALFUNC(alGetError);
	LOADOPENALFUNC(alIsExtensionPresent);
	LOADOPENALFUNC(alGetProcAddress);
	LOADOPENALFUNC(alGetEnumValue);
	LOADOPENALFUNC(alListeneri);
	LOADOPENALFUNC(alListenerf);
	LOADOPENALFUNC(alListener3f);
	LOADOPENALFUNC(alListenerfv);
	LOADOPENALFUNC(alGetListeneri);
	LOADOPENALFUNC(alGetListenerf);
	LOADOPENALFUNC(alGetListener3f);
	LOADOPENALFUNC(alGetListenerfv);
	LOADOPENALFUNC(alGenSources);
	LOADOPENALFUNC(alDeleteSources);
	LOADOPENALFUNC(alIsSource);
	LOADOPENALFUNC(alSourcei);
	LOADOPENALFUNC(alSourcef);
	LOADOPENALFUNC(alSource3f);
	LOADOPENALFUNC(alSourcefv);
	LOADOPENALFUNC(alGetSourcei);
	LOADOPENALFUNC(alGetSourcef);
	LOADOPENALFUNC(alGetSourcefv);
	LOADOPENALFUNC(alSourcePlayv);
	LOADOPENALFUNC(alSourceStopv);
	LOADOPENALFUNC(alSourcePlay);
	LOADOPENALFUNC(alSourcePause);
	LOADOPENALFUNC(alSourceStop);
	LOADOPENALFUNC(alGenBuffers);
	LOADOPENALFUNC(alDeleteBuffers);
	LOADOPENALFUNC(alIsBuffer);
	LOADOPENALFUNC(alBufferData);
	LOADOPENALFUNC(alGetBufferi);
	LOADOPENALFUNC(alGetBufferf);
	LOADOPENALFUNC(alSourceQueueBuffers);
	LOADOPENALFUNC(alSourceUnqueueBuffers);
	LOADOPENALFUNC(alDistanceModel);
	LOADOPENALFUNC(alDopplerFactor);
	LOADOPENALFUNC(alDopplerVelocity);
	LOADOPENALFUNC(alcGetString);
	LOADOPENALFUNC(alcGetIntegerv);
	LOADOPENALFUNC(alcOpenDevice);
	LOADOPENALFUNC(alcCloseDevice);
	LOADOPENALFUNC(alcCreateContext);
	LOADOPENALFUNC(alcMakeContextCurrent);
	LOADOPENALFUNC(alcProcessContext);
	LOADOPENALFUNC(alcGetCurrentContext);
	LOADOPENALFUNC(alcGetContextsDevice);
	LOADOPENALFUNC(alcSuspendContext);
	LOADOPENALFUNC(alcDestroyContext);
	LOADOPENALFUNC(alcGetError);
	LOADOPENALFUNC(alcIsExtensionPresent);
	LOADOPENALFUNC(alcGetProcAddress);
	LOADOPENALFUNC(alcGetEnumValue);

	xprintf("LoadOpenAL: %s loaded.\n", OPENALDLL);

	return TRUE;
}



//
//
//
static void UnloadOpenal( void ) {

	// Unload the dll
        if( hOpenALDLL ) {
		FreeLibrary( hOpenALDLL );
                hOpenALDLL = NULL;
        }

	return;
}



//
//
//
static const char *GetALErrorStr( const ALenum error ) {

	static const char*ALNoErrorStr = "No error occurred";
	static const char*ALInvalidNameStr = "a bad name (ID) was passed to an OpenAL function";
	static const char*ALInvalidEnumStr = "an invalid enum value was passed to an OpenAL function";
	static const char*ALInvalidValueStr = "an invalid value was passed to an OpenAL function";
	static const char*ALInvalidOpStr = "the requested operation is not valid";
	static const char*ALOutOfMemoryStr = "the requested operation resulted in OpenAL running out of memory";
	static const char*ALOtherErrorStr = "unknown error";

	switch( error ) {
		case AL_NO_ERROR:
			return ALNoErrorStr;
		break;
		case AL_INVALID_NAME:
			return ALInvalidNameStr;
		break;
		case AL_INVALID_ENUM:
			return ALInvalidEnumStr;
		break;
		case AL_INVALID_VALUE:
			return ALInvalidValueStr;
		break;
		case AL_INVALID_OPERATION:
			return ALInvalidOpStr;
		break;
		case AL_OUT_OF_MEMORY:
			return ALOutOfMemoryStr;
		break;
		default:
			return ALOtherErrorStr;
		break;
 	}

	return ALOtherErrorStr;
}





typedef enum {

	WT_WAVE = 0,
	WT_STREAM,

} wavetype_e;

#define MAXBUFFERS	10			// stream buffers
#define BUFFERSIZE 	(4096 * 4)		// streaming buffer size

typedef struct wavelist_s {

	int handle;
	wavetype_e type;
	char filename[XMAX_PATH];

	ALenum format;
	ALuint buffers[MAXBUFFERS];

	int freq;
	int pan;
	int vol;
	BOOL loop;

	int size;
	BOOL bit16;
	BOOL stereo;
	int tic;

	// Ogg Stream

	BOOL playing;

	OggVorbis_File oggfile;
	ov_callbacks vcb;
	vorbis_info *info;
	vorbis_comment *comment;

	UCHAR *raw_file;
	int raw_file_size;
	int raw_file_cur;
	int cur_source;

} wavelist_t, *wavelist_ptr;


static wavelist_t *wavelist = NULL;
static int numwavelists = 0;

static int wave_id_cnt = 1213;

#define WAVELIST wavelist[numwavelists]

static BOOL oggStream( wavelist_t *wave, ALuint buffer );


// ezek a channelek
#define MAXSOURCES	16

static ALuint oal_sources[MAXSOURCES];	//
static int oal_handles[MAXSOURCES];		// default: -1



//
//
//
void OAL_Deinit( void ) {

	if( hOpenALDLL == NULL )
		return;

	if( TimerId ) {
		timeKillEvent( TimerId );
		TimerId = 0;
	}

	for( int i=0; i<MAXSOURCES; i++ )
		_alDeleteSources( 1, &oal_sources[i] );

	_alGetError();

	ALCcontext *context = _alcGetCurrentContext();
 	if( context != NULL ) {

#define PCCE_AL_CHECK() { ALenum error = _alGetError(); if(AL_NO_ERROR != error ) xprintf( "OpenAL error:  %s (%s %d).\n", GetALErrorStr(error), __FILE__, __LINE__); }

		ALCdevice* device = _alcGetContextsDevice(context);
		PCCE_AL_CHECK();

		_alcSuspendContext(context);
		PCCE_AL_CHECK();

		_alcDestroyContext(context);
		PCCE_AL_CHECK();

		if( device != NULL ) {
			_alcCloseDevice(device);
			PCCE_AL_CHECK();
		}
	}

	UnloadOgglib();
	UnloadOpenal();

	return;
}




//
//
//
BOOL OAL_Init( void ) {

	if( hOpenALDLL != NULL )
		return TRUE;

	if( LoadOpenAL() == FALSE ) {
		xprintf("OAL_Init: failed to init OpenAL.\n");
		return FALSE;
	}

	if( _alcGetCurrentContext() != NULL ) {
		xprintf("OAL_Init: OpenAL already initialized.\n");
		return TRUE;
	}

	_alGetError();

 	ALCdevice *device = _alcOpenDevice(NULL); // select the "preferred device"
 	if( device == NULL ) {
		xprintf("OAL_Init: Cannot open preferred device.\n");
		UnloadOpenal();
		return FALSE;
	}

	if( _alcGetError(device) != ALC_NO_ERROR ) {
		_alcCloseDevice(device);
		xprintf("OAL_Init: Could not open device (alc error).\n");
		UnloadOpenal();
		return FALSE;
 	}

 	// TODO: check the context attributes, maybe something is useful:
 	// http://www.openal.org/openal_webstf/specs/oal11spec_html/oal11spec6.html
 	// 6.2.1. Context Attributes
 	// my bet is on ALC_STEREO_SOURCES ;-)
 	ALCcontext *context = _alcCreateContext(device, NULL);
 	if( context == NULL ) {
		_alcCloseDevice(device);
		xprintf("OAL_Init: Could not create context");
		UnloadOpenal();
		return FALSE;
 	}

 	if( _alcGetError(device) != ALC_NO_ERROR ) {
		_alcDestroyContext(context);
		_alcCloseDevice(device);
		xprintf("OAL_Init: Could not open device (alc error)");
		UnloadOpenal();
		return FALSE;
 	}

 	if( _alcMakeContextCurrent(context) != ALC_TRUE ) {
		_alcDestroyContext(context);
		_alcCloseDevice(device);
		xprintf("OAL_Init: Could not make context current");
		UnloadOpenal();
		return FALSE;
 	}

 	if( _alcGetError(device) != ALC_NO_ERROR ) {
		_alcMakeContextCurrent(NULL);
	 	_alcDestroyContext(context);
		_alcCloseDevice(device);
		xprintf("OAL_Init: Could not make context current (alc error)");
		UnloadOpenal();
		return FALSE;
 	}

	_alGetError();

 	// Should the listener be set here? It seems to be set to a default, and
 	// eventually I'd like to have a SetALListener method that takes pcce
 	// vectors (once we get vectors done).

 	//--- listener config -------------------------------------
 	ALfloat listenerPos[] = { 0.0, 0.0, 0.0 };
 	ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };

 	// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
 	ALfloat listenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };

 	_alListenerfv(AL_POSITION, listenerPos);
 	_alListenerfv(AL_VELOCITY, listenerVel);
 	_alListenerfv(AL_ORIENTATION, listenerOri);

	if( _alGetError() != AL_NO_ERROR ) {
		OAL_Deinit();
		xprintf("OAL_Init: could not set Listener attributes.\n");
		UnloadOpenal();
		return FALSE;
 	}

	for( int i=0; i<MAXSOURCES; i++ ) {
		oal_handles[i] = -1;
		_alGetError();
		_alGenSources( 1, &oal_sources[i] );
	}

	if( 0 ) {
		// xprintf("OpenAL version: %s\n", alGetString(AL_VERSION) );
		// xprintf("OpenAL renderer: %s\n", alGetString(AL_RENDERER) );
		// xprintf("OpenAL vendor: %s\n", alGetString(AL_VENDOR) );
		// xprintf("OpenAL extensions: %s\n", alGetString(AL_EXTENSIONS) );

		// Enumerate OpenAL devices
		if( _alcIsExtensionPresent( NULL, (const ALCchar *)"ALC_ENUMERATION_EXT") == AL_TRUE ) {

			const char *s = (const char *) _alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			while( *s != '\0' ) {
				xprintf("OpenAL available device: %s\n", s );
				while( *s++ != '\0' );
			}
		}
		else
			xprintf("OpenAL device enumeration isn't available.\n" );

		// Print default device name
		xprintf("OpenAL default device: %s\n", (const char *)_alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER) );
	}

	// Print current device name
	xprintf("OAL_Init: using \"%s\" device.\n", (const char *)_alcGetString(device, ALC_DEVICE_SPECIFIER) );

	return TRUE;
}



//
//
//
int OAL_IsLoaded( char *filename ) {

	int i;

	if( filename == NULL )
		return 0;

	// be van-e már töltve
	for( i=0; i<numwavelists; i++ )
		if( !stricmp(filename, wavelist[i].filename ) )
			break;

	// igen
	if( i < numwavelists )
		return wavelist[i].handle;

	return 0;
}




//
//
//
int OAL_IsPlaying( int handle ) {

	int i;

	// be van-e már töltve
	for( i=0; i<numwavelists; i++ )
		if( wavelist[i].handle == handle )
			break;

	// igen
	if( i < numwavelists && OAL_GetWaveParam( handle ) )
		return wavelist[i].playing;

	return FALSE;
}




//
//
//
void OAL_FreeWave( int handle ) {

	int i,j;

	for( i=0; i<numwavelists; i++ )
		if( wavelist[i].handle == handle )
			break;

	if( i >= numwavelists )
		return;

	// stream
	if( wavelist[i].type == WT_STREAM ) {

		wavelist[i].playing = FALSE;

		// ha megvan még a csatornája
		if( oal_handles[wavelist[i].cur_source] == wavelist[i].handle ) {

			int queued;

			_alGetSourcei( oal_sources[wavelist[i].cur_source], AL_BUFFERS_QUEUED, &queued );

			_alGetError();

			while( queued-- ) {
				ALuint buffer;
				_alSourceUnqueueBuffers( oal_sources[wavelist[i].cur_source], 1, &buffer);
				_alGetError();
			}
		}

		_alDeleteBuffers( MAXBUFFERS, wavelist[i].buffers );
		wavelist[i].buffers[0] = AL_NONE;

		_ov_clear( &wavelist[i].oggfile );

		SAFE_FREEMEM( wavelist[i].raw_file );
	}

	// Wave
	if( wavelist[i].buffers[0] != AL_NONE )
		_alDeleteBuffers( 1, wavelist[i].buffers );

	if( oal_handles[wavelist[i].cur_source] == wavelist[i].handle ) {
		oal_handles[wavelist[i].cur_source] = -1;
		// source stop
	}

	if( wavelist[i].raw_file ) FREEMEM( wavelist[i].raw_file );

	// ha csak egy volt
	if( numwavelists == 1 ) {
		numwavelists = 0;
		FREEMEM( wavelist );
		wavelist = NULL;
		return;
	}

	// nem az utsó volt
	if( i < (numwavelists-1) )
		memmove( &wavelist[i],
			 &wavelist[i+1],
			 sizeof(wavelist_t)*(numwavelists-(i+1)) );

	// ha az utsó volt leszarjuk

	--numwavelists;

	return;
}



//
//
//
void OAL_FreeAllWave( void ) {

	while( numwavelists > 0 )
		OAL_FreeWave( wavelist[numwavelists-1].handle );

	return;
}





//
//
//
int OAL_LoadWave( char *filename, BOOL force_reload ) {

	int i,j;

	if( filename == NULL )
		return -1;

	if( stristr( filename, ".ogg") && FileLong( filename ) > 50000 )
		return OAL_LoadStream( filename, force_reload );

	if( OAL_Init() == FALSE )
		return -1;

	// be van-e már töltve
	for( i=0; i<numwavelists; i++ )
		if( !stricmp( wavelist[i].filename, filename ) )
			break;

	// igen
	if( i < numwavelists ) {

		if( force_reload == FALSE )
			return wavelist[i].handle;

		OAL_FreeWave( wavelist[i].handle );
	}

	xprintf("OAL_LoadWave: loading \"%s\"...\n",filename );

	REALLOCMEM( wavelist, (numwavelists+1) * sizeof(wavelist_t) );

	WAVELIST.handle = wave_id_cnt++;
	WAVELIST.type = WT_WAVE;
	strcpy( WAVELIST.filename, filename );

	WAVELIST.cur_source = 0;
	WAVELIST.pan = MIDPAN;
	WAVELIST.vol = MIDVOL;
	WAVELIST.loop = FALSE;
	WAVELIST.buffers[0] = AL_NONE;

	ALenum format = AL_FORMAT_MONO8;
	ALsizei size = 0;
	int channel = 0;
	int bps = 0;
	char *loaded_file = NULL;

	size = LoadFile( WAVELIST.filename, PTR(loaded_file) );

	if( IsSnd( loaded_file, size) == TRUE ) {

		char *snd = NULL;
		int snd_size = 0;

		if( DecodeSnd( (UCHAR *)loaded_file, size, PTR(snd), &snd_size, &channel, &bps, &WAVELIST.freq ) == FALSE ) {
			if( loaded_file ) FREEMEM( loaded_file );
			xprintf("OAL_LoadWave: DecodeSnd phailed.\n");
			return -1;
		}

		REALLOCMEM( loaded_file, snd_size );

		memcpy( loaded_file, snd, snd_size );
		size = snd_size;

		FREEMEM(snd);
	}
	else {
		if( loaded_file ) FREEMEM( loaded_file );
		xprintf("OAL_LoadWave: %s not a sound file?\n",WAVELIST.filename);
		return -1;
	}

	// dump
	if( 0 ) {
		xprintf("OAL_LoadWave: size = %d, chan = %d, bps = %d bit, freq = %d\n",size,channel,bps,WAVELIST.freq);
		char str[XMAX_PATH];
		sprintf( str, "oal_dump_%d.wav", WAVELIST.handle );
		PushTomFlag();
		TomFlag( NOFLAG );
		XL_WriteFile( str, loaded_file, size );
		PopTomFlag();
	}

	if( channel == 1 )
		format = (bps == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
 	else
		format = (bps == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

	WAVELIST.stereo = channel == 1 ? FALSE : TRUE;
	WAVELIST.bit16 = bps == 8 ? FALSE : TRUE;
	WAVELIST.size = size-44;
	WAVELIST.tic = -TICKBASE*10;

	_alGetError();

 	_alGenBuffers( 1, WAVELIST.buffers );

 	if( _alGetError() != AL_NO_ERROR || WAVELIST.buffers[0] == AL_NONE ) {
		if( loaded_file ) FREEMEM( loaded_file );
		xprintf("OAL_LoadWave: Could not generate buffer.\n");
		return -1;
 	}

	// a WAV header 44 bytos
 	_alBufferData( WAVELIST.buffers[0], format, &loaded_file[44], size-44, WAVELIST.freq );

	if( _alGetError() != AL_NO_ERROR ) {
		if( loaded_file ) FREEMEM( loaded_file );
		xprintf( "OAL_LoadWave: Could not load buffer data.\n");
		return -1;
	}

	if( loaded_file ) FREEMEM( loaded_file );

	int handle = WAVELIST.handle;

	xprintf("OAL_LoadWave: handle = %d.\n", handle );

	++numwavelists;

	return handle;
}



//
//
//
void OAL_PlayWave( int handle ) {

	static volatile int timersema = 0;

	if( timersema > 0 )
		return;

	timersema = 1;

	int i,j;

	for( i=0; i<numwavelists; i++ )
		if( wavelist[i].handle == handle )
			break;

	if( i >= numwavelists ) {
		timersema = 0;
		return;
	}

#define SOURCE oal_sources[j]
#define HANDLE oal_handles[j]

	for( j=0; j<MAXSOURCES; j++ ) {

		// van-e olyan aminek vége van?
		ALenum state;

		_alGetSourcei( oal_sources[j], AL_SOURCE_STATE, &state );

		if( state == AL_STOPPED || state == AL_INITIAL )
			break;
	}

	// debug: force next channel
	if( 0 ) {
		static int cnt = 0;
		j = cnt++;
		if( cnt >= MAXSOURCES ) cnt = 0;
	}

	// Wave: nincs szabad csatorna
	if( j >= MAXSOURCES ) {

		// TODO: find first non-Stream channel

		j = 0;
	}

	wavelist[i].playing = FALSE;	// StreamUpdater miatt

	_alSourceStop(SOURCE);

	// FIXME: ne csak egy szóljon!!!!
	// stream buffer leállít csak egy szólhat
	if( wavelist[i].type == WT_STREAM && wavelist[i].cur_source != -1 ) {

		_alSourceStop(oal_sources[wavelist[i].cur_source]);

		// stop lejáccás
		int queued;

		_alGetSourcei( oal_sources[wavelist[i].cur_source], AL_BUFFERS_QUEUED, &queued );

		while( queued-- ) {
			ALuint buffer;
			_alSourceUnqueueBuffers( oal_sources[wavelist[i].cur_source], 1, &buffer);
		}

		_alGetError();
	}

	// stop lejáccás a csatornán amin le akarunk jáccani
	int queued;

	_alGetSourcei( SOURCE, AL_BUFFERS_QUEUED, &queued );

	while( queued-- ) {
		ALuint buffer;
		_alSourceUnqueueBuffers( SOURCE, 1, &buffer);
	}

	_alGetError();


	wavelist[i].cur_source = j;
	HANDLE = wavelist[i].handle;

	// Wave
	if( wavelist[i].type == WT_WAVE ) {

		_alGetError();

		ALint curBuffer;

		_alGetSourcei( SOURCE, AL_BUFFER, &curBuffer );

		if( curBuffer != wavelist[i].buffers[0] )
			_alSourcei( SOURCE, AL_BUFFER, wavelist[i].buffers[0] );

		if( _alGetError() != AL_NO_ERROR )
			xprintf("OAL_PlayWave: Could not set source buffer.\n");
	}

	// oggPlayback
	if( wavelist[i].type == WT_STREAM ) {

		// rewind és play
		_ov_time_seek( &wavelist[i].oggfile, 0 );

		for( int k=0; k<MAXBUFFERS; k++ )
			if( !oggStream( &wavelist[i] ,wavelist[i].buffers[k] ) ) {
				xprintf( "OAL_PlayWave: oggStream (buffer = %d) phailed.\n",k);
				timersema = 0;
				return;
			}

		_alSourceQueueBuffers( SOURCE, MAXBUFFERS, wavelist[i].buffers );
	}

	_alGetError();

	_alSourcef( SOURCE, AL_PITCH, 1.0f );
	_alSourcef( SOURCE, AL_GAIN, (FLOAT)wavelist[i].vol / (FLOAT)MAXVOL );

 	if( _alGetError() != AL_NO_ERROR )
		xprintf("OAL_PlayWave: can't set volume.\n");

	// xprintf( "j = %d vol = %d\n", j, wavelist[i].vol );

 	ALfloat sourcePos[] = {0.0, 0.0, 0.0};
 	ALfloat sourceVel[] = {0.0, 0.0, 0.0};

	// FIXME: most [-1 ... +1]  0 közép
	sourcePos[0] = (FLOAT)(MIDPAN - wavelist[i].pan) / (FLOAT)-MIDPAN;

	// xprintf("OAL_PlayWave: pan = %f\n", sourcePos[0]);

	_alSourcefv( SOURCE, AL_POSITION, sourcePos );
	_alSourcefv( SOURCE, AL_VELOCITY, sourceVel );
	// streamnél nem jó a loop
	_alSourcei( SOURCE, AL_LOOPING, (wavelist[i].type == WT_WAVE)?wavelist[i].loop:AL_FALSE );

 	if( _alGetError() != AL_NO_ERROR ) {
		xprintf("OAL_PlayWave: could not set Source attributes.\n");
		timersema = 0;
		return;
	}

	_alSourcePlay( SOURCE );

	if( _alGetError() != AL_NO_ERROR )
		xprintf("OAL_PlayWave: can't play source.\n");

	wavelist[i].tic = GetTic();
	wavelist[i].playing = TRUE;

	// vol dump
	if( 0 ) {
		ALfloat Gain;
		_alGetSourcef( SOURCE, AL_GAIN, &Gain );
		xprintf( "chn: %d  vol: %.1f shoud be: %.1f\n", j, Gain * 100.f, ((FLOAT)wavelist[i].vol / (FLOAT)MAXVOL)* 100.f );
	}

	timersema = 0;

	return;
}




//
//
//
void OAL_StopWave( int handle ) {

	int i;

	// be van-e már töltve
	for( i=0; i<numwavelists; i++ )
		if( wavelist[i].handle == handle )
			break;

	// igen
	if( i < numwavelists )
		return;

	// wavelist[i].playing = FALSE;
	// wavelist[i].tic = -TICKBASE;

	return;
}



//
//
//
void OAL_SetWaveParam( int handle, int dwFreq, int dwVol, int dwPan, int fLooped ) {

	int i;

	for( i=0; i<numwavelists; i++ )
		if( wavelist[i].handle == handle )
			break;

	if( i >= numwavelists ) {
		xprintf("OAL_SetWaveParam: can't find %d handler.\n",handle);
		return;
	}

	if( dwFreq != -1 )
		wavelist[i].freq = dwFreq;

	if( dwPan != -1 ) {
		wavelist[i].pan = dwPan;
		CLAMPMINMAX( wavelist[i].pan, MINPAN, MAXPAN );
	}

	if( dwVol != -1 ) {
		wavelist[i].vol = dwVol;
		CLAMPMINMAX( wavelist[i].vol, MINVOL, MAXVOL );
	}

	if( fLooped != -1 )
		wavelist[i].loop = fLooped;

	// menet közbeni állítás
	if( wavelist[i].playing == TRUE ) {

		// volume
		if( dwVol != -1 ) {

			_alGetError();

			_alSourcef( oal_sources[wavelist[i].cur_source], AL_GAIN, (FLOAT)wavelist[i].vol / (FLOAT)MAXVOL );

			if( _alGetError() != AL_NO_ERROR )
				xprintf("OAL_SetWaveParam: can't set volume.\n");
		}

		// panning
		if( dwPan != -1 ) {

		}

		if( fLooped != -1 )
			_alSourcei( oal_sources[wavelist[i].cur_source], AL_LOOPING, (wavelist[i].type == WT_WAVE)?wavelist[i].loop:AL_FALSE );
	}

	// xprintf( "OAL_SetWaveParam: %d, %d, %d, %d\n", dwFreq,dwVol,dwPan,fLooped );

	return;
}





//
// printf(" %u:%02u\n",time/60,time%60);
// printf("pos %09I64u",pos);
//
BOOL OAL_GetWaveParam( int handler, int *percent, int *time, int *maxTime, int *bytePos, int *byteMaxPos, char **name ) {

	int i;

	for( i=0; i<numwavelists; i++ )
		if( wavelist[i].handle == handler )
			break;

	if( i >= numwavelists ) {
		xprintf("OAL_GetWaveParam: can't find %d handler.\n",handler);
		return FALSE;
	}

	int sampleslen = wavelist[i].bit16 ? 2 : 1;
	sampleslen *= (wavelist[i].stereo ? 2 : 1);

	int len = (wavelist[i].size / sampleslen) / wavelist[i].freq;	// in sec
	int pos = 0, pos_ms = 0;

	// FIXME: a loopot nem támogatja
	if( wavelist[i].tic >= 0 ) {
		int c = GetTic();
		int t = wavelist[i].tic + len*TICKBASE;
		pos_ms = c < t ? c - wavelist[i].tic : 0;
		pos = pos_ms / TICKBASE;
		if( pos_ms == 0 ) {
			wavelist[i].playing = FALSE;
			wavelist[i].tic = -TICKBASE;
		}
	}
	else {
		wavelist[i].playing = FALSE;
		wavelist[i].tic = -TICKBASE;
	}

	if( time ) *time = pos;
	if( maxTime ) *maxTime = len;

	if( bytePos ) *bytePos = 0;
	if( byteMaxPos ) *byteMaxPos = wavelist[i].size;

	if( percent ) *percent = ( pos_ms*100 / len ) / TICKBASE;

	if( name != NULL )
		*name = wavelist[i].filename;

	return TRUE;
}






/**********************************************
 *
 * Ogg Vorbis
 *
 **********************************************/


static int ogg_log = 0;

//
//
//
static BOOL oggPlaying( wavelist_t *wave ) {

	ALenum state;

	_alGetSourcei( oal_sources[wave->cur_source], AL_SOURCE_STATE, &state );

	return (state == AL_PLAYING);
}



static BOOL vege = FALSE;

//
//
//
static BOOL oggStream( wavelist_t *wave, ALuint buffer ) {

	char data[BUFFERSIZE];
	int  size = 0;
	int  section;
	int  result;
	int error;

	while( size < BUFFERSIZE ) {

		result = _ov_read( &wave->oggfile, data + size, BUFFERSIZE - size, 0, 2, 1, &section );

		if( result > 0 )
			size += result;
		else
		if( result < 0 ) {
			xprintf("oggStream: %s. (%d)\n", Ogg_ErrorString(result), result );
			return FALSE;
		}
		else
			break;
	}

	vege = (result == 0)?TRUE:FALSE;

	if( size == 0 )
		return FALSE;

	if( ogg_log ) xprintf("oggStream: streaming %d bytes.\n", size );

	_alGetError();

	_alBufferData( buffer, wave->format, data, size, wave->info->rate );

	if( (error = _alGetError()) != AL_NO_ERROR )
		xprintf("oggStream: alBufferData error. (%d)\n", error );

	return TRUE;
}







//
//
//
static void CALLBACK StreamUpdater( UINT IDEvent, UINT uReserved, DWORD dwUser, DWORD dwReserved1, DWORD dwReserved2 ) {

	static volatile int timersema = 0;

	if( timersema > 0 )
		return;

	timersema = 1;

	int i;

	for( i=0; i<numwavelists; i++ ) {

		// játsza-e le
		if( wavelist[i].playing == FALSE )
			continue;

		if( wavelist[i].type == WT_STREAM ) {

			int processed;

			_alGetError();

			// lejátszotta-e már a buffert
			ALenum state;

			_alGetSourcei( oal_sources[wavelist[i].cur_source], AL_SOURCE_STATE, &state );

			if( state == AL_STOPPED ) {
				_alSourcePlay( oal_sources[wavelist[i].cur_source] );
				// xprintf("StreamUpdater: replay (%d).\n" ,wavelist[i].cur_source);
			}

			_alGetError();

			_alGetSourcei( oal_sources[wavelist[i].cur_source], AL_BUFFERS_PROCESSED, &processed );

			while( wavelist[i].playing == TRUE && processed-- ) {

				ALuint buffer;
				int error;

				_alGetError();

				_alSourceUnqueueBuffers( oal_sources[wavelist[i].cur_source], 1, &buffer );

				if( (error = _alGetError()) != AL_NO_ERROR ) {
					xprintf("StreamUpdater: alSourceUnqueueBuffers error. (%d)\n", error );
					break;
				}

				BOOL active = oggStream( &wavelist[i], buffer );

				_alGetError();

				_alSourceQueueBuffers( oal_sources[wavelist[i].cur_source], 1, &buffer );

				if( (error = _alGetError()) != AL_NO_ERROR ) {
					xprintf("StreamUpdater: alSourceQueueBuffers error. (%d)\n", error );
					break;
				}
			}

			// ha loop van és már vége lett
			if( wavelist[i].playing == TRUE && wavelist[i].loop == TRUE && vege ) {
				OAL_PlayWave( wavelist[i].handle );
				vege = FALSE;
			}
		}
	}

	timersema = 0;

	return;
}





//
//
//
static size_t VorbisRead( void *ptr, size_t byteSize, size_t sizeToRead, void *datasource ) {

	size_t spaceToEOF;
	size_t actualSizeToRead;
	wavelist_t *wave = (wavelist_t *)datasource;

	spaceToEOF = wave->raw_file_size - wave->raw_file_cur;
	if( (sizeToRead*byteSize) < spaceToEOF ) {
		actualSizeToRead = (sizeToRead * byteSize);
	}
	else {
		actualSizeToRead = spaceToEOF;
	}

	if( actualSizeToRead ) {
		memcpy( ptr, (char *)&wave->raw_file[wave->raw_file_cur], actualSizeToRead );
		wave->raw_file_cur += (actualSizeToRead);
	}

	return actualSizeToRead;
}



//
//
//
static int VorbisSeek( void *datasource, ogg_int64_t offset, int whence ) {

	size_t spaceToEOF;
	ogg_int64_t actualOffset;
	wavelist_t *wave = (wavelist_t *)datasource;

	switch( whence ) {

		case SEEK_SET:

			if( wave->raw_file_size >= offset ) {
				actualOffset = offset;
			}
			else {
				actualOffset = wave->raw_file_size;
			}

			wave->raw_file_cur = (int)actualOffset;

			break;


		case SEEK_CUR:

			spaceToEOF = wave->raw_file_size - wave->raw_file_cur;

			if( offset < spaceToEOF ) {
				actualOffset = (offset);
			}
			else {
				actualOffset = spaceToEOF;
			}

			wave->raw_file_cur += actualOffset;

			break;

		case SEEK_END:

			wave->raw_file_cur = wave->raw_file_size+1;

			break;

		default:
			xprintf("VorbisSeek: error cant seek.\n");
	}

	return 0;
}




//
//
//
static int VorbisClose( void *datasource ) {

	wavelist_t *wave = (wavelist_t *)datasource;

	return 1;
}


//
//
//
static long VorbisTell( void *datasource ) {

	wavelist_t *wave = (wavelist_t *)datasource;

	return wave->raw_file_cur;
}



//
//
//
int OAL_LoadStream( char *filename, BOOL force_reload ) {

	int i,j;

	if( filename == NULL )
		return -1;

	if( OAL_Init() == FALSE || LoadOgglib() == FALSE )
		return -1;

	// be van-e már töltve
	for( i=0; i<numwavelists; i++ )
		if( !stricmp( wavelist[i].filename, filename ) )
			break;

	// igen
	if( i < numwavelists ) {

		if( force_reload == FALSE )
			return wavelist[i].handle;

		OAL_FreeWave( wavelist[i].handle );
	}

	xprintf("OAL_LoadStream: loading \"%s\"...\n",filename );

	REALLOCMEM( wavelist, (numwavelists+1) * sizeof(wavelist_t) );

	WAVELIST.handle = wave_id_cnt++;
	WAVELIST.playing = FALSE;
	WAVELIST.type = WT_STREAM;
	WAVELIST.cur_source = -1;
	strcpy( WAVELIST.filename, filename );

	WAVELIST.pan = MIDPAN;
	WAVELIST.vol = MIDVOL;
	WAVELIST.loop = FALSE;
	WAVELIST.buffers[0] = AL_NONE;

	WAVELIST.format = AL_FORMAT_MONO8;

	WAVELIST.raw_file_size = LoadFile( filename, PTR(WAVELIST.raw_file) );
	WAVELIST.raw_file_cur = 0;

	WAVELIST.vcb.read_func = VorbisRead;
	WAVELIST.vcb.close_func = VorbisClose;
	WAVELIST.vcb.seek_func = VorbisSeek;
	WAVELIST.vcb.tell_func = VorbisTell;

	if( _ov_open_callbacks( &WAVELIST, &WAVELIST.oggfile, NULL, 0, WAVELIST.vcb ) ) {
		xprintf("OAL_LoadStream: ov_open_callbacks failed.\n");
		return -1;
	}

	WAVELIST.info = _ov_info( &WAVELIST.oggfile, -1);
	WAVELIST.comment = _ov_comment( &WAVELIST.oggfile, -1);

	switch( WAVELIST.info->channels ) {
		case 1:	WAVELIST.format = AL_FORMAT_MONO16; break;
		case 2: WAVELIST.format = AL_FORMAT_STEREO16; break;
		case 4: WAVELIST.format = _alGetEnumValue("AL_FORMAT_QUAD16"); break;
		case 6: WAVELIST.format = _alGetEnumValue("AL_FORMAT_51CHN16"); break;
		case 7: WAVELIST.format = _alGetEnumValue("AL_FORMAT_61CHN16"); break;
		case 8: WAVELIST.format = _alGetEnumValue("AL_FORMAT_71CHN16"); break;
		default: WAVELIST.format = 0; break;
	}

	WAVELIST.stereo = WAVELIST.info->channels == 1 ? FALSE : TRUE;
	WAVELIST.bit16 = TRUE;
	WAVELIST.size = WAVELIST.raw_file_size;

	if( ogg_log > 0 ) {
		xprintf("Ogg info:\n" );
		xprintf("version:         %d\n", WAVELIST.info->version );
		xprintf("channels:        %d\n", WAVELIST.info->channels );
		xprintf("rate (hz):       %d\n", WAVELIST.info->rate );
		xprintf("bitrate upper:   %d\n", WAVELIST.info->bitrate_upper );
		xprintf("bitrate nominal: %d\n", WAVELIST.info->bitrate_nominal );
		xprintf("bitrate lower:   %d\n", WAVELIST.info->bitrate_lower );
		xprintf("bitrate window:  %d\n", WAVELIST.info->bitrate_window  );
		xprintf("vendor:          %s\n", WAVELIST.comment->vendor );

		for( int i=0; i<WAVELIST.comment->comments; i++ )
			xprintf("    %s\n", WAVELIST.comment->user_comments[i] );
	}

	_alGetError();

	_alGenBuffers( MAXBUFFERS, WAVELIST.buffers );
	if( _alGetError() != AL_NO_ERROR ) {
		xprintf( "OAL_LoadStream: alGenBuffers phailed.\n");
		return -1;
	}

	if( TimerId == 0 ) {
		// Set up the callback event.	The callback function
		// MUST be in a FIXED CODE DLL!!! -> not in Win95
		TimerId = timeSetEvent( 500,		 /* how often			*/
					500,		 /* timer resolution		*/
					StreamUpdater,	 /* callback function		*/
					0,		 /* info to pass to callback	*/
					TIME_PERIODIC ); /* oneshot or periodic?	*/
		xprintf("OAL_LoadStream: timer started.\n");
	}

	int handle = WAVELIST.handle;

	xprintf("OAL_LoadStream: handle = %d.\n", handle );

	++numwavelists;

	return handle;
}


