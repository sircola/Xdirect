
#include <windows.h>
#include <malloc.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: sdlmixershell.cpp,v 1.0 2012/08/30 06:34:00 bernie Exp $" )

#include "SDL.h"
#include "c:\My Projects\SDL_mixer-2.0\Include\SDL2\SDL_mixer.h"


typedef struct wave_s {

   	int      	dwFreq; 	// Frequency.
   	int      	dwPan;		// Panning info.
   	int		dwVol;		// Total volume.
   	BOOL		fLooped;	// Looped?

	Mix_Chunk	*chunk;

	int		v;
	int 	   	handler;
	int 		tic;

	char		szFileName[XMAX_PATH];	// MAX_PATH

	int 		libsnd_chn;
	int 		libsnd_bits;
	int 		libsnd_freq;

	struct wave_s *pNext; 		// Pointer to next file.

} wave_t, *wave_ptr;


#define SDLMAXVOL 128
#define SDLMAXPAN (PAN_RIGHT - PAN_LEFT)
#define SDLMAXVOLWAV (MIKMODMAXVOL*2)


#define NUMMODCHANNEL 32
#define NUMSFXCHANNEL 32


static wave_t WaveFirst = {0,0,0,0, NULL, 0,0,0, "",  0,0,0, NULL };

static Mix_Music *mixer_music = NULL;


#define SDLMIXERDLLNAME  "SDL2_mixer.dll"

static HINSTANCE hSdlMixer = 0;		// bass handle

static int SDLCALL (*pMix_OpenAudio)(int frequency, Uint16 format, int channels, int chunksize) = NULL;
static void SDLCALL (*pMix_CloseAudio)(void) = NULL;
static int SDLCALL (*pMix_AllocateChannels)(int numchans) = NULL;
static int SDLCALL (*pMix_Volume)(int channel, int volume) = NULL;
static int SDLCALL (*pMix_VolumeMusic)(int volume) = NULL;
static Mix_Music * SDLCALL (*pMix_LoadMUS)(const char *file) = NULL;
static int SDLCALL (*pMix_PlayMusic)(Mix_Music *music, int loops) = NULL;
static void SDLCALL (*pMix_FreeMusic)(Mix_Music *music) = NULL;
static int SDLCALL (*pMix_PlayChannelTimed)(int channel, Mix_Chunk *chunk, int loops, int ticks) = NULL;
static int SDLCALL (*pMix_VolumeChunk)(Mix_Chunk *chunk, int volume) = NULL;
static int SDLCALL (*pMix_QuerySpec)(int *frequency,Uint16 *format,int *channels) = NULL;
static void SDLCALL (*pMix_FreeChunk)(Mix_Chunk *chunk) = NULL;
static Mix_Chunk * SDLCALL (*pMix_QuickLoad_WAV)(Uint8 *mem) = NULL;


#define LOADSDLMIXERFUNCTION(f,g) { 						\
	*((void**)&g)=(void*)GetProcAddress(hSdlMixer,#f); 			\
	if( (g) == NULL ) { 							\
		if( hSdlMixer ) FreeLibrary( hSdlMixer ); 			\
		hSdlMixer = NULL; 						\
		xprintf("LoadSdlMixer: can't load \"%s\" function.\n", #f ); 	\
		return FALSE; 							\
	} };


#define SDLDLLNAME  "SDL2.dll"

static HINSTANCE hSdl = 0;		// bass handle

typedef int (SDLCALL *LPFN_SDLINIT)( Uint32 flags );
static LPFN_SDLINIT pSDL_Init = NULL;

typedef void (SDLCALL *LPFN_SDLQUIT)( void );
static LPFN_SDLQUIT pSDL_Quit = NULL;

typedef char * (SDLCALL *LPFN_SDLGETERROR)( void );
static LPFN_SDLGETERROR pSDL_GetError = NULL;

typedef SDL_RWops * (SDLCALL *LPFN_SDLRWFROMFILE)(const char *file, const char *mode);
static LPFN_SDLRWFROMFILE pSDL_RWFromFile = NULL;


//
//
//
static void UnloadSdlMixer( void ) {

	if( hSdlMixer ) VAPI( FreeLibrary( hSdlMixer ) );
	hSdlMixer = NULL;

	pMix_OpenAudio = NULL;
	pMix_CloseAudio = NULL;

	if( hSdl ) VAPI( FreeLibrary( hSdl ) );
	hSdl = NULL;

	pSDL_Init = NULL;
	pSDL_Quit = NULL;

	return;
}



//
//
//
static BOOL LoadSdlMixer( void ) {

	if( hSdlMixer != NULL )
		return TRUE;

	char mixerdllname[XMAX_PATH];

	sprintf( mixerdllname, "%s%c%s", GetExeDir(), PATHDELIM, SDLMIXERDLLNAME );

	if( (hSdlMixer = LoadLibrary(mixerdllname)) == NULL ) {
		xprintf("LoadSdlMixer: couldn't load \"%s\" library.\n",mixerdllname);
		return FALSE;
	}

	LOADSDLMIXERFUNCTION( Mix_OpenAudio, pMix_OpenAudio );
	LOADSDLMIXERFUNCTION( Mix_CloseAudio, pMix_CloseAudio );
	LOADSDLMIXERFUNCTION( Mix_AllocateChannels, pMix_AllocateChannels );
	LOADSDLMIXERFUNCTION( Mix_Volume, pMix_Volume );
	LOADSDLMIXERFUNCTION( Mix_VolumeMusic, pMix_VolumeMusic );
	LOADSDLMIXERFUNCTION( Mix_LoadMUS, pMix_LoadMUS );
	LOADSDLMIXERFUNCTION( Mix_PlayMusic, pMix_PlayMusic );
	LOADSDLMIXERFUNCTION( Mix_FreeMusic, pMix_FreeMusic );
	LOADSDLMIXERFUNCTION( Mix_QuickLoad_WAV, pMix_QuickLoad_WAV );
	LOADSDLMIXERFUNCTION( Mix_PlayChannelTimed, pMix_PlayChannelTimed );
	LOADSDLMIXERFUNCTION( Mix_VolumeChunk, pMix_VolumeChunk );
	LOADSDLMIXERFUNCTION( Mix_QuerySpec, pMix_QuerySpec );
	LOADSDLMIXERFUNCTION( Mix_FreeChunk, pMix_FreeChunk );

	xprintf("LoadSdlMixer: %s loaded.\n", mixerdllname);

	sprintf( mixerdllname, "%s%c%s", GetExeDir(), PATHDELIM, SDLDLLNAME );

	if( (hSdl = LoadLibrary(mixerdllname)) == NULL ) {
		hSdlMixer = NULL;
		xprintf("LoadSdlMixer: couldn't load \"%s\" library.\n",mixerdllname);
		return FALSE;
	}

	if( (pSDL_Init = (LPFN_SDLINIT)GetProcAddress( hSdl, "SDL_Init" )) == NULL ) {
		hSdlMixer = NULL;
		hSdl = NULL;
		xprintf("LoadSdlMixer: couldn't find proc.\n");
		return FALSE;
	}

	if( (pSDL_Quit = (LPFN_SDLQUIT)GetProcAddress( hSdl, "SDL_Quit" )) == NULL ) {
		hSdlMixer = NULL;
		hSdl = NULL;
		xprintf("LoadSdlMixer: couldn't find proc.\n");
		return FALSE;
	}

	if( (pSDL_GetError = (LPFN_SDLGETERROR)GetProcAddress( hSdl, "SDL_GetError" )) == NULL ) {
		hSdlMixer = NULL;
		hSdl = NULL;
		xprintf("LoadSdlMixer: couldn't find proc.\n");
		return FALSE;
	}

	if( (pSDL_RWFromFile = (LPFN_SDLRWFROMFILE)GetProcAddress( hSdl, "SDL_RWFromFile" )) == NULL ) {
		hSdlMixer = NULL;
		hSdl = NULL;
		xprintf("LoadSdlMixer: couldn't find proc.\n");
		return FALSE;
	}

	return TRUE;
}


//
//
//
BOOL MIX_Init( void ) {

	if( hSdlMixer )
		return TRUE;

	if( LoadSdlMixer() == FALSE )
		return FALSE;

	mixer_music = NULL;
	WaveFirst.pNext = NULL;

	int audio_rate = 22050;
        Uint16 audio_format = AUDIO_S16;
        int audio_channels = NUMSFXCHANNEL;
        int audio_buffers = 1024;
	int MusicVolume = 64;
	int SoundVolume = 64;

	if( pSDL_Init(SDL_INIT_AUDIO) != 0 ) {
		xprintf("ERROR: Unable to initialize SDL: %s\n", pSDL_GetError() );
		return FALSE;
	}

        if( pMix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) ) {
		xprintf("MIX_Init: Unable to open audio.\n");
		return FALSE;
	}

	pMix_AllocateChannels(NUMSFXCHANNEL);
	for( int channel = 0; channel < audio_channels; channel++ ) {
		pMix_Volume( channel, SoundVolume );
	}

	pMix_VolumeMusic(MusicVolume);

	// pMix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

	xprintf("MIX_Init: inited.\n");

	return TRUE;
}



//
// Free all stuff
//
void MIX_Deinit( void ) {

	if( hSdlMixer == NULL )
		return;

	pMix_CloseAudio();
	pSDL_Quit();

	UnloadSdlMixer();

	return;
}


//
//
//
BOOL MIX_PlayMod( char *filename ) {

	int mixer_loop = 1;

	if( filename == NULL || MIX_Init() == FALSE )
		return FALSE;

	mixer_music = pMix_LoadMUS( filename );

	if( pMix_PlayMusic( mixer_music, mixer_loop ) == -1 ) {
		xprintf("MIX_PlayMod: %s\n", pSDL_GetError());
	}

	xprintf("MIX_PlayMod: playing \"%s\".\n",filename);

	return TRUE;
}


//
//
//
void MIX_StopMod( void ) {

	pMix_FreeMusic( mixer_music );

	mixer_music = NULL;

	return;
}



//
//
//
BOOL MIX_SetModuleParam( int vol, int order, int row, int loop ) {

	if( mixer_music == NULL )
		return TRUE;

	xprintf("MIX_SetModuleParam: called with %d, %d, %d\n", vol, order, row );

	return TRUE;
}



//
//
//
BOOL MIX_GetModuleParam( int *vol, int *ord, int *row, int *max_order, int *percent, char **song_name, int *max_chan ) {

	if( mixer_music == NULL )
		return TRUE;



	return TRUE;
}




//
// Is the player playing?
//
BOOL MIX_IsModulePlaying( void ) {

	if( mixer_music == NULL )
		return FALSE;

	return TRUE; // pPlayer_Active();
}





//
//
//
static int AddToList( wave_t *pWaveHead, wave_t *pWave ) {

	pWave->pNext = NULL;

	while( pWaveHead->pNext != NULL ) {
		pWaveHead = pWaveHead->pNext;
	}

	pWaveHead->pNext = pWave;

	return TRUE;
}



//
//
//
static wave_t *GetWave( int handler=-1, const char *filename=NULL ) {

	wave_t *pWaveFirst;

	pWaveFirst = WaveFirst.pNext;

	// handlerre keresünk
	while( handler != -1 && pWaveFirst != NULL ) {
		if( pWaveFirst->handler == handler )
			return pWaveFirst;
		pWaveFirst = pWaveFirst->pNext;
	}

	pWaveFirst = WaveFirst.pNext;

	// filenévre
	while( filename != NULL && pWaveFirst != NULL ) {
		if( !stricmp( pWaveFirst->szFileName, filename ) )
			return pWaveFirst;
		pWaveFirst = pWaveFirst->pNext;
	}

	return NULL;
}



//
//
//
int MIX_LoadWave( const char *inFileName, int id ) {

	// int idx = Mix_LoadWAV_RW( SDL_RWFromFile(filename, "rb"), 1 );

	wave_t *wave = NULL;
	char *loaded_file = NULL;
	int size;

	if( winNoSound() )
		return id;

	if( inFileName == NULL || MIX_Init() == FALSE )
		return -1;

	xprintf("\rMIX_LoadWave: ");

	if( GetWave( id ) )
		xprintf("Warning: there is already %d handler. ", id );

	ALLOCMEM( wave, sizeof(wave_t) );

	memset( wave, 0L, sizeof(wave_t) );

	wave->handler = id;
	strcpy( wave->szFileName, inFileName );
	strlwr( wave->szFileName );

	wave->chunk = NULL;

	wave->libsnd_chn = -1;
	wave->libsnd_bits = -1;
	wave->libsnd_freq = -1;

	if( (size = LoadFile( inFileName, PTR(loaded_file))) == 0 ) {
		xprintf("\"%s\" can't LoadFile.", inFileName );
		return -1;
	}

	if( IsSnd( loaded_file, size) == TRUE ) {

		char *snd = NULL;
		int snd_size = 0;

		if( DecodeSnd( (UCHAR *)loaded_file, size, PTR(snd), &snd_size, &wave->libsnd_chn, &wave->libsnd_bits, &wave->libsnd_freq ) == FALSE ) {

			FREEMEM( loaded_file );
			FREEMEM( wave );

			xprintf("\"%s\" can't load thru libsndfile.", inFileName );

			return -1;
		}

		REALLOCMEM( loaded_file, snd_size );

		memcpy( loaded_file, snd, snd_size );
		size = snd_size;

		FREEMEM(snd);
	}
	else
	if( IsFlac( loaded_file, size) == TRUE ) {

		// FLAC

		char *flac = NULL;
		int flac_size = 0;

		if( DecodeFlac( (UCHAR *)loaded_file, size, PTR(flac), &flac_size ) == FALSE ) {

			FREEMEM( loaded_file );
			FREEMEM( wave );

			xprintf("\"%s\" can't decode as FLAC.",inFileName );

			return -1;
		}

		REALLOCMEM( loaded_file, flac_size );

		memcpy( loaded_file, flac, flac_size );
		size = flac_size;

		FREEMEM(flac);

		wave->libsnd_chn = 2;
		wave->libsnd_bits = 16;
		wave->libsnd_freq = 22050;
	}

	if( (wave->chunk = pMix_QuickLoad_WAV((UCHAR *)loaded_file)) != 0 ) {

		if( AddToList( &WaveFirst, wave ) ) {

			wave->dwFreq = 22050;
			wave->dwPan = MIDPAN;
			wave->dwVol = MAXVOL;
			wave->fLooped = FALSE;

			xprintf("\"%s\" loaded (%d).",inFileName,wave->handler);
		}
		else
			xprintf("addtolist failed.");
	}
	else {
		FREEMEM( loaded_file );
		FREEMEM( wave );

		xprintf("\"%s\" NOT loaded. (%s)",inFileName,pSDL_GetError());

		return -1;
	}

	FREEMEM( loaded_file );

	return wave->handler;
}


//
//
//
BOOL MIX_IsPlaying( int handler ) {
	return FALSE;
}



//
//
//
void MIX_PlayWave( int handler ) {

	// for (int index = 1; index < 16; index++) Mix_Volume( index, SoundVolume );

	// for (int indexTwo = 0; indexTwo < SoundTotal; indexTwo++) Mix_VolumeChunk(SoundFX[indexTwo], SoundVolume);

	wave_t *pWave = GetWave( handler );

	if( (pWave = GetWave( handler ) ) == NULL ) {
		xprintf("MIX_PlayWave: can't find %d handler.\n",handler);
		return;
	}

	int channel = -1;
	int loops = 0;
	int ticks = -1;

	if( pMix_PlayChannelTimed(channel,pWave->chunk,loops,ticks) == -1 )
		xprintf("MIX_PlayWave: %s\n",pSDL_GetError());

	return;
}




//
//
//
void MIX_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped, int percent ) {


	return;
}


//
//
//
BOOL MIX_GetWaveParam( int handler, int *percent, int *time, int *maxTime, int *mypos, int *myMaxPos, char **name ) {


	return TRUE;
}


//
//
//
void MIX_StopWave( int handler ) {
	return;
}


//
//
//
void MIX_FreeWave( int handler ) {

	wave_t *pWave = NULL;
	wave_t *pPrev = NULL;

	if( (pWave = GetWave( handler ) ) != NULL ) {

		pPrev = &WaveFirst;

		while( pPrev != NULL ) {

			if( pPrev->pNext == pWave )
				break;

			pPrev = pPrev->pNext;
		}

		xprintf("MIX_FreeWave: \"%s\".\n",pWave->szFileName);

		if( pWave->chunk ) {
			// BASS_SampleStop( pWave->samp );
			// BASS_SampleFree( pWave->samp );
			pMix_FreeChunk( pWave->chunk );
		}

		if( pPrev ) pPrev->pNext = pWave->pNext;

		xprintf("pWave:\n");

		FREEMEM( pWave );
	}


	return;
}

//
//
//
void MIX_FreeAllWave( void ) {
	return;
}
