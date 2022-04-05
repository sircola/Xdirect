
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>

#define DIRECTSOUND_VERSION 0x0300

#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <dsound.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>


#include <xlib.h>

RCSID( "$Id: Sound.cpp,v 1.1.1.1 2003/08/19 17:48:47 bernie Exp $" )

#define DSOUNDDLL "DSOUND.DLL"

// To check for stopping of sounds, a timer is set...use this for the rate.
#define TIMERPERIOD	100	// In milliseconds

#define DS_MINVOL	( DSBVOLUME_MIN )
#define DS_MAXVOL	( DSBVOLUME_MAX )
#define DS_MIDPAN	( DSBPAN_CENTER )

#define BUFFERSIZE 	(4096 * 4)		// streaming buffer size


typedef struct fileinfo_s {

    	BYTE     	*pbData;	// Pointer to actual data of file.
    	UINT       	cbSize; 	// Size of data.
    	WAVEFORMATEX	*pwfx;		// Pointer to waveformatex structure.

    	DWORD      	dwFreq; 	// Frequency.
    	DWORD      	dwPan;		// Panning info.
    	DWORD		dwVol;		// Total volume.
    	BOOL		fLooped;	// Looped?

    	BOOL		fPlaying;	// Is this one playing?
    	BOOL		fLost;		// Is this one lost?
    	BOOL		fHardware;	// Is this a hardware buffer?
    	BOOL		fSticky;	// Is this a sticky buffer?

	int 	   	handler;

    	LPDIRECTSOUNDBUFFER pDSB;	// Pointer to direct sound buffer.

	// Ogg Stream
	BOOL		fOgg;

	OggVorbis_File oggfile;
	ov_callbacks vcb;
	vorbis_info *info;
	vorbis_comment *comment;

	UCHAR *raw_file;
	int raw_file_size;
	int raw_file_cur;
	int cur_source;

    	char szFileName[XMAX_PATH+1];

	struct fileinfo_s *pNext; 	// Pointer to next file.

} FILEINFO;



#define MAXCHANNEL	16

typedef struct channel_s {

	FILEINFO *pFileInfo;

	BOOL fPlaying;
	LPDIRECTSOUNDBUFFER pDSB;	// Pointer to direct sound buffer.

} channel_t, *channel_ptr;



// Start of linked list.
static FILEINFO FileInfoFirst;

static LPDIRECTSOUND gpds = NULL;

static UINT TimerId = 0;

static channel_t channel[MAXCHANNEL];

static LPDIRECTSOUND3DBUFFER   pDS3DBuffer = NULL;	// 3D sound buffer
static LPDIRECTSOUND3DLISTENER pDSListener = NULL;	// 3D listener object
static DS3DLISTENER            dsListenerParams;	// Listener properties


static HINSTANCE DSHinst = NULL;

typedef HRESULT (WINAPI *DIRECTSOUNDENUMERATE)( LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext );
static DIRECTSOUNDENUMERATE DSEnum = NULL;

typedef HRESULT (WINAPI *DIRECTSOUNDCREATE)( LPCGUID, LPDIRECTSOUND* ,IUnknown* );
static DIRECTSOUNDCREATE DSCreate = NULL;

#define DSMAXVOL ABS(DSBVOLUME_MAX - DSBVOLUME_MIN)

static int ds_id_cnt = 1214;

static int NewDirectSoundBuffer( FILEINFO *pFileInfo );
static int ReleaseDirectSoundBuffer( FILEINFO *pFileInfo );
static int CreateControl( FILEINFO *pFileInfo, DWORD dwFreq, DWORD dwPan, DWORD dwVol );
static FILEINFO *GetFileInfo( int handler=-1, char *szFileName=NULL );
static int AddToList( FILEINFO *pFileInfoHead, FILEINFO *pFileInfo );
static int FreeAllList( void );
static BOOL RemoveFromList( FILEINFO *pFileInfo );
static BOOL IsValidWave( LPSTR pszFileName );
static void CALLBACK DS_TimerHandler( UINT IDEvent, UINT uReserved, DWORD dwUser, DWORD dwReserved1, DWORD dwReserved2 );
static int StartDSound( FILEINFO *pFileInfo );
static int StopDSound( FILEINFO *pFileInfo );
static int StopAllDSounds( void );
static int ChangeOutputPan( FILEINFO *pFileInfo );
static int ChangeOutputFreq( FILEINFO *pFileInfo );
static int ChangeOutputVol( FILEINFO *pFileInfo );
static int FormatToIndex( FILEINFO *pFileInfo );
static int IndexToFormat( FILEINFO *pFileInfo, int index );
static char *TranslateDSError( HRESULT hr );




//
//
//
static void UpdateMainStatus() {

	DSCAPS	dsc;

	dsc.dwSize = sizeof(dsc);
	gpds->GetCaps(&dsc);

	xprintf("(remain: %d Kb)",(dsc.dwFreeHwMemBytes+512)/1024 );

	return;
}



static GUID sound_guid;
static char sound_str[128];

//
//
//
static BOOL CALLBACK DSEnumProc( LPGUID lpGUID, LPSTR lpszDesc, LPSTR lpszDrvName, LPVOID lpContext ) {

	if( lpGUID != NULL ) {

		sprintf( sound_str, "%s (%s)", lpszDesc, lpszDrvName );

		memcpy( &sound_guid, lpGUID, sizeof(GUID) );

		return FALSE;
	}

	return TRUE;
}


#define DSERROR { \
	if( DSHinst ) FreeLibrary( DSHinst ); \
	DSHinst = NULL;  }


//
//
//
BOOL DS_Init( void ) {

	DSBUFFERDESC dsbd;
	HRESULT hr;
	DWORD dw;
	int i;

	if( DSHinst != NULL )
		return TRUE;

	if( (DSHinst = LoadLibrary( DSOUNDDLL )) == NULL ) {
		xprintf( "DS_Init: Couldn't LoadLibrary %s.\n", DSOUNDDLL );
		DSERROR;
		return FALSE;
	}

	if( (DSCreate = (DIRECTSOUNDCREATE)GetProcAddress( DSHinst, "DirectSoundCreate" )) == NULL ) {
		xprintf( "DS_Init: Couldn't GetProcAddress DirectSoundCreate.\n" );
		DSERROR;
		return FALSE;
	}

	if( (DSEnum = (DIRECTSOUNDENUMERATE)GetProcAddress( DSHinst, "DirectSoundEnumerateA" )) == NULL ) {
		xprintf( "DS_Init: Couldn't GetProcAddress DirectSoundEnumerate.\n" );
		DSERROR;
		return FALSE;
	}

	memset( &FileInfoFirst, 0L, sizeof(FileInfoFirst) );

	if( FAILED(hr=DSEnum( (LPDSENUMCALLBACK)DSEnumProc, NULL )) ) {
		xprintf("DS_Init: DirectSoundEnumerate failed. (%s)\n",TranslateDSError(hr));
		DSERROR;
		return FALSE;
	}

	xprintf("DS_Init: %s\n",sound_str);

#ifdef __MINGW32__
	dw = sizeof(WAVEFORMATEX);
#else
	if( acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &dw) != MMSYSERR_NOERROR ) {
		xprintf("DS_Init: acmMetrics failed.\n");
		DSERROR;
		return FALSE;
	}
#endif

	ALLOCMEM( FileInfoFirst.pwfx, dw );

	FileInfoFirst.handler = -1;
	FileInfoFirst.pwfx->wFormatTag = WAVE_FORMAT_PCM;
	FileInfoFirst.pwfx->nChannels = 2;
	FileInfoFirst.pwfx->nSamplesPerSec = 22050;
	FileInfoFirst.pwfx->nAvgBytesPerSec = 22050*2*2;
	FileInfoFirst.pwfx->nBlockAlign = 4;
	FileInfoFirst.pwfx->wBitsPerSample = 16;
	FileInfoFirst.pwfx->cbSize = 0;

	if( (hr = DSCreate( &sound_guid, &gpds, NULL )) == DS_OK ) {
		if( gpds->SetCooperativeLevel( TOHWND(GetHwnd()), DSSCL_NORMAL) == DS_OK ) {

			ZeroMemory(&dsbd, sizeof(dsbd));
			dsbd.dwSize = sizeof(dsbd);
			dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;

			if( (hr = gpds->CreateSoundBuffer( &dsbd, &(FileInfoFirst.pDSB), NULL)) == DS_OK ) {

				/***
				if( FAILED( hr = gpds->QueryInterface( IID_IDirectSound3DListener, (void**)&pDSListener ) ) )
					xprintf("DS_Init: IID_IDirectSound3DListener. (%s)\n",TranslateDSError(hr));

				// Get listener parameters
				dsListenerParams.dwSize = sizeof(DS3DLISTENER);
				pDSListener->GetAllParameters( &g_dsListenerParams );
				***/

				if( (hr = FileInfoFirst.pDSB->Play( 0, 0, DSBPLAY_LOOPING)) != DS_OK ) {

					xprintf("DS_Init: can't play primary buffer. (%s)\n",TranslateDSError(hr));

					FileInfoFirst.pDSB->Release();
					FileInfoFirst.pDSB = NULL;

					gpds->Release();
					gpds = NULL;

					DSERROR;

					return FALSE;
				}
			}
			else {

				xprintf("DS_Init: can't create primary buffer. (%s)\n",TranslateDSError(hr));

				gpds->Release();
				gpds = NULL;

				DSERROR;

				return FALSE;
			}
		}
		else {
			xprintf("DS_Init: DirectSound SetCooperativeLevel failed. (%s)\n",TranslateDSError(hr));

			gpds->Release();
			gpds = NULL;

			DSERROR;

			return FALSE;
		}
	}
	else {
		xprintf("DS_Init: DirectSoundCreate failed. (%s)\n",TranslateDSError(hr));
		DSERROR;
		return FALSE;
	}

	for( i=0; i<MAXCHANNEL; i++ ) {
		channel[i].pFileInfo = NULL;
		channel[i].fPlaying = FALSE;
		channel[i].pDSB = NULL;
	}

	TimerId = timeSetEvent( TIMERPERIOD,	/* how often			*/
				TIMERPERIOD,	/* timer resolution		*/
				DS_TimerHandler,/* callback function		*/
				0,		/* info to pass to callback	*/
				TIME_PERIODIC); /* oneshot or periodic?		*/

	return TRUE;
}








//
//
//
void DS_Deinit( void ) {

	HRESULT hr = 0;

	StopAllDSounds();
	FreeAllList();

	if( TimerId )
		timeKillEvent(TimerId);
	TimerId = 0;

	// Destroy the direct sound buffer.
	if( FileInfoFirst.pDSB != NULL ) {
		FileInfoFirst.pDSB->Stop();
		FileInfoFirst.pDSB->Release();
		FileInfoFirst.pDSB = NULL;
	}

	if( pDSListener != NULL ) {
		pDSListener->Release();
		pDSListener = NULL;
	}

	if( pDS3DBuffer != NULL ) {
		pDS3DBuffer->Release();
		pDS3DBuffer = NULL;
	}

	// Destroy the direct sound object.
	if( gpds != NULL ) {
		gpds->Release();
		gpds = NULL;
	}

	SAFE_FREEMEM(FileInfoFirst.pwfx);
	SAFE_FREEMEM(FileInfoFirst.pbData);

	if( DSHinst ) FreeLibrary( DSHinst );
	DSHinst = NULL;

	return;
}




//
//
//
static size_t VorbisRead( void *ptr, size_t byteSize, size_t sizeToRead, void *datasource ) {

	size_t spaceToEOF;
	size_t actualSizeToRead;
	FILEINFO *wave = (FILEINFO *)datasource;

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
	FILEINFO *wave = (FILEINFO *)datasource;

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

	FILEINFO *wave = (FILEINFO *)datasource;

	return 1;
}


//
//
//
static long VorbisTell( void *datasource ) {

	FILEINFO *wave = (FILEINFO *)datasource;

	return wave->raw_file_cur;
}





//
//
//
int DS_LoadWave( char *szFileName ) {

	FILEINFO *pFileInfo;
	BOOL fSticky = FALSE;
	int hr;

	UCHAR *wav;
	int size;

	if( szFileName == NULL || DS_Init() == FALSE )
		return -1;

	if( (pFileInfo = GetFileInfo( -1, szFileName )) != NULL )
		return pFileInfo->handler;

	xprintf("DS_LoadWave: ");

	// stream-e?
	if( stristr( szFileName, ".ogg") && FileLong( szFileName ) > 50000 ) {

		if( LoadOgglib() == FALSE )
			return -1;

		ALLOCMEM( pFileInfo, sizeof(FILEINFO) );
		memset( pFileInfo, 0L, sizeof(FILEINFO) );

		pFileInfo->fOgg = TRUE;

		pFileInfo->raw_file_size = LoadFile( szFileName, PTR(pFileInfo->raw_file) );
		pFileInfo->raw_file_cur = 0;

		pFileInfo->vcb.read_func = VorbisRead;
		pFileInfo->vcb.close_func = VorbisClose;
		pFileInfo->vcb.seek_func = VorbisSeek;
		pFileInfo->vcb.tell_func = VorbisTell;

		if( _ov_open_callbacks( pFileInfo, &pFileInfo->oggfile, NULL, 0, pFileInfo->vcb ) ) {
			xprintf("ov_open_callbacks failed.\n");
			return -1;
		}

		pFileInfo->info = _ov_info( &pFileInfo->oggfile, -1);
		pFileInfo->comment = _ov_comment( &pFileInfo->oggfile, -1);

		/***
		xprintf("Ogg info:\n" );
		xprintf("version:         %d\n", pFileInfo->info->version );
		xprintf("channels:        %d\n", pFileInfo->info->channels );
		xprintf("rate (hz):       %d\n", pFileInfo->info->rate );
		xprintf("bitrate upper:   %d\n", pFileInfo->info->bitrate_upper );
		xprintf("bitrate nominal: %d\n", pFileInfo->info->bitrate_nominal );
		xprintf("bitrate lower:   %d\n", pFileInfo->info->bitrate_lower );
		xprintf("bitrate window:  %d\n", pFileInfo->info->bitrate_window  );
		xprintf("vendor:          %s\n", pFileInfo->comment->vendor );
		***/

		// TODO
		return -1;
	}

	size = LoadFile( szFileName, PTR(wav) );
	if( size < 0 ) {
		xprintf("can't load file.\n");
		return -1;
	}

	if( IsSnd( (char *)wav, size) == TRUE ) {

		char *snd = NULL;
		int snd_size = 0;

		if( DecodeSnd( (UCHAR *)wav, size, PTR(snd), &snd_size ) == FALSE ) {
			SAFE_FREEMEM( wav );
			xprintf("DecodeSnd phailed.\n");
			return -1;
		}

		REALLOCMEM( wav, snd_size );

		memcpy( wav, snd, snd_size );
		size = snd_size;

		FREEMEM(snd);
	}
	else {
		SAFE_FREEMEM( wav );
		xprintf("%s not a sound file?\n",szFileName);
		return -1;
	}

	// TODO: ez miért kell
	size = size - 44;

	ALLOCMEM( pFileInfo, sizeof(FILEINFO) );
	memset( pFileInfo, 0L, sizeof(FILEINFO) );

	pFileInfo->pbData   = NULL;
	pFileInfo->pwfx     = NULL;
	pFileInfo->pDSB     = NULL;
	pFileInfo->fSticky  = fSticky;
	pFileInfo->handler  = ++ds_id_cnt;
	strcpy( pFileInfo->szFileName, szFileName);

	pFileInfo->cbSize = size;

	ALLOCMEM( pFileInfo->pbData, size );

	memcpy( pFileInfo->pbData, wav + 44, size );

	ALLOCMEM( pFileInfo->pwfx, sizeof(WAVEFORMATEX) );

	memcpy( pFileInfo->pwfx, wav + 20, sizeof(WAVEFORMATEX) );
	pFileInfo->pwfx->cbSize = 0;

	FREEMEM( wav );

	if( (hr = NewDirectSoundBuffer( pFileInfo )) == 0 ) {

		if( AddToList( &FileInfoFirst, pFileInfo ) ) {

			CreateControl( pFileInfo, pFileInfo->pwfx->nSamplesPerSec, MIDPAN, MAXVOL );
			ChangeOutputVol(pFileInfo);
			ChangeOutputFreq(pFileInfo);
			ChangeOutputPan(pFileInfo);
			UpdateMainStatus();

			xprintf(" \"%s\" loaded (%d).\n",szFileName,pFileInfo->handler);
		}
		else
			xprintf("addtolist failed.\n");
	}
	else {
		FREEMEM( pFileInfo->pwfx );
		FREEMEM( pFileInfo->pbData );
		FREEMEM( pFileInfo );

		xprintf(" \"%s\" NOT loaded. (%s)\n",szFileName,TranslateDSError(hr));

		return -1;
	}

	return pFileInfo->handler;
}






//
//
//
int DS_NewSound( int id, UCHAR *header, int audiosize ) {

	FILEINFO *pFileInfo = NULL;
	BOOL fSticky = FALSE;
	int hr;

	// int size;

	if( DS_Init() == FALSE )
		return 0;

	xprintf("DS_NewWave: ");

	pFileInfo = (FILEINFO *)malloc( sizeof(FILEINFO) );

	memset( pFileInfo, 0L, sizeof(FILEINFO) );

	pFileInfo->pbData   = NULL;
	pFileInfo->pwfx     = NULL;
	pFileInfo->pDSB     = NULL;
	pFileInfo->fSticky  = fSticky;
	pFileInfo->handler  = id;
	strcpy( pFileInfo->szFileName, "custom sound");

	// audiosize = 0; // LoadFile( szFileName, PTR(wav) ) - 44;

	pFileInfo->cbSize = audiosize;
	ALLOCMEM( pFileInfo->pbData, audiosize );

	// memcpy( pFileInfo->pbData, wav + 44, size );

	ALLOCMEM( pFileInfo->pwfx, sizeof(WAVEFORMATEX) );

	memcpy( pFileInfo->pwfx, header, sizeof(WAVEFORMATEX) );
	pFileInfo->pwfx->cbSize = audiosize;

	if( (hr = NewDirectSoundBuffer( pFileInfo )) == 0 ) {

		if( AddToList( &FileInfoFirst, pFileInfo ) ) {

			CreateControl( pFileInfo, pFileInfo->pwfx->nSamplesPerSec, MIDPAN, MAXVOL );
			ChangeOutputVol(pFileInfo);
			ChangeOutputFreq(pFileInfo);
			ChangeOutputPan(pFileInfo);
			UpdateMainStatus();

			xprintf(" \"%s\" loaded (%d).\n",pFileInfo->szFileName,pFileInfo->handler);
		}
		else
			xprintf("addtolist failed.\n");
	}
	else {
		FREEMEM( pFileInfo->pwfx );
		FREEMEM( pFileInfo->pbData );
		FREEMEM( pFileInfo );

		xprintf(" \"%s\" NOT loaded. (%s)\n",pFileInfo->szFileName,TranslateDSError(hr));

		return -1;
	}

	return pFileInfo->handler;
}



//
//
//
BOOL DS_CopyWaveBuffer( int handle, UCHAR *data, int len ) {

	UCHAR *buf = NULL;

	return TRUE;
}




//
//
//
static int NewDirectSoundBuffer( FILEINFO *pFileInfo ) {

	DSBUFFERDESC	dsbd;
	DSBCAPS 	dsbc;
	HRESULT 	hr;
	BYTE		*pbData 	= NULL;
	BYTE		*pbData2	= NULL;
	DWORD		dwLength;
	DWORD		dwLength2;

	memset(&dsbd, 0L, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
	if( pFileInfo->fSticky )
		dsbd.dwFlags |= DSBCAPS_STICKYFOCUS;
	dsbd.dwBufferBytes = pFileInfo->cbSize;
	dsbd.lpwfxFormat= pFileInfo->pwfx;

	if( SUCCEEDED(hr = gpds->CreateSoundBuffer( &dsbd, &(pFileInfo->pDSB), NULL )) ) {

		if( (hr = pFileInfo->pDSB->Lock( 0, pFileInfo->cbSize,
							(void**)&pbData, &dwLength,
							(void**)&pbData2, &dwLength2,
							0L) ) == DSERR_BUFFERLOST ) {
			pFileInfo->pDSB->Restore();

			hr = pFileInfo->pDSB->Lock( 0, pFileInfo->cbSize,
							(void**)&pbData, &dwLength,
							(void**)&pbData2, &dwLength2,
							0L );
		}

		if( SUCCEEDED(hr) ) {

			CopyMemory( pbData, pFileInfo->pbData, MIN(dwLength, pFileInfo->cbSize) );

			if( pbData2 )
				CopyMemory( pbData2, pFileInfo->pbData+dwLength, MIN(dwLength2, pFileInfo->cbSize - dwLength) );

			if( SUCCEEDED(hr = pFileInfo->pDSB->Unlock( pbData, pFileInfo->cbSize, NULL, 0)) ) {

				pbData = NULL;

				pFileInfo->pDSB->SetVolume(DS_MAXVOL);
				pFileInfo->pDSB->SetPan(DS_MIDPAN);

				dsbc.dwSize = sizeof(dsbc);

				if( SUCCEEDED(hr = pFileInfo->pDSB->GetCaps(&dsbc)) ) {

					if( dsbc.dwFlags & DSBCAPS_LOCHARDWARE )
						pFileInfo->fHardware = TRUE;
					else
						pFileInfo->fHardware = FALSE;
				}
				else {
					pFileInfo->pDSB->Release();
					pFileInfo->pDSB = NULL;
				}
			}
			else {
				pFileInfo->pDSB->Release();
				pFileInfo->pDSB = NULL;
			}
		}
		else {
			pFileInfo->pDSB->Release();
			pFileInfo->pDSB = NULL;
		}
	}

	return hr;
}





//
//
//
static int ReleaseDirectSoundBuffer( FILEINFO *pFileInfo ) {

	if( pFileInfo->pDSB != NULL ) {
		pFileInfo->pDSB->Release();
		pFileInfo->pDSB = NULL;
	}

	return 0;
}




//
//
//
static int CreateControl( FILEINFO *pFileInfo, DWORD dwFreq, DWORD dwPan, DWORD dwVol ) {

	int nError = 0;
	DWORD dwMinFreq, dwMaxFreq;

	if( pFileInfo == NULL )
		return 1;

	// get the min and max range that the sound card supports.
	// If the buffer is in hardware query the card, else use
	// our ifdef'd values.
	if( pFileInfo->fHardware ) {
		DSCAPS dsc;
		memset(&dsc, 0, sizeof(DSCAPS));
		dsc.dwSize = sizeof(DSCAPS);
		if( (nError = gpds->GetCaps(&dsc)) != DS_OK );
		dwMinFreq = dsc.dwMinSecondarySampleRate;
		dwMaxFreq = dsc.dwMaxSecondarySampleRate;
	}
	else {
		dwMinFreq = DSBFREQUENCY_MIN;
		dwMaxFreq = DSBFREQUENCY_MAX;
	}

	// xprintf("freq: %d, %d\n",dwMinFreq,dwMaxFreq);

	pFileInfo->dwFreq = dwFreq;
	pFileInfo->dwPan = dwPan;
	pFileInfo->dwVol = dwVol;

	ChangeOutputFreq(pFileInfo);
	ChangeOutputPan(pFileInfo);
	ChangeOutputVol(pFileInfo);

	return nError;
}







//
//
//
static FILEINFO *GetFileInfo( int handler, char *szFileName ) {

	FILEINFO *pFileInfoFirst;

	pFileInfoFirst = FileInfoFirst.pNext;

	while( handler != -1 && pFileInfoFirst != NULL ) {
		if( pFileInfoFirst->handler == handler )
			return pFileInfoFirst;
		pFileInfoFirst = pFileInfoFirst->pNext;
	}

	pFileInfoFirst = FileInfoFirst.pNext;

	// filenévre
	while( szFileName != NULL && pFileInfoFirst != NULL ) {
		if( !stricmp( pFileInfoFirst->szFileName, szFileName ) )
			return pFileInfoFirst;
		pFileInfoFirst = pFileInfoFirst->pNext;
	}

	return NULL;
}





//
//
//
static int AddToList( FILEINFO *pFileInfoHead, FILEINFO *pFileInfo ) {

	pFileInfo->pNext = NULL;
	pFileInfo->fPlaying = FALSE;

	while( pFileInfoHead->pNext != NULL ) {
		pFileInfoHead = pFileInfoHead->pNext;
	}

	pFileInfoHead->pNext = pFileInfo;

	return TRUE;
}





//
//
//
static int FreeAllList( void ) {

	FILEINFO *pFileInfo, *pFileNext;

	if( DSHinst == NULL )
		return 0;

	pFileInfo = FileInfoFirst.pNext;

	while( pFileInfo != NULL ) {

		xprintf("DSFREEALL: \"%s\"\n",pFileInfo->szFileName);

		ReleaseDirectSoundBuffer( pFileInfo );

		FREEMEM(pFileInfo->pwfx);
		FREEMEM(pFileInfo->pbData);

		pFileNext = pFileInfo->pNext;

		FREEMEM( pFileInfo );
		pFileInfo = pFileNext;
	}

	FileInfoFirst.pNext = NULL;

	return 0;
}




//
//
//
static int RemoveFromList( FILEINFO *pFileInfo ) {

	FILEINFO *pFileNext, *pFileInfoRover;

	pFileInfoRover = &FileInfoFirst;

	while( pFileInfoRover->pNext != NULL ) {

		if( pFileInfoRover->pNext == pFileInfo ) {

			ReleaseDirectSoundBuffer( pFileInfoRover->pNext );

			FREEMEM(pFileInfoRover->pNext->pwfx);
			FREEMEM(pFileInfoRover->pNext->pbData);

			pFileNext = pFileInfoRover->pNext->pNext;

			// xprintf("FREE: \"%s\"\n",pFileInfoRover->pNext->szFileName);

			FREEMEM( pFileInfoRover->pNext );

			pFileInfoRover->pNext = pFileNext;

			break;
		}

		pFileInfoRover = pFileInfoRover->pNext;
	}

	return 0;
}




static BOOL timer_munging = FALSE;

//
//
static void CALLBACK DS_TimerHandler( UINT IDEvent, UINT uReserved, DWORD dwUser, DWORD dwReserved1, DWORD dwReserved2 ) {
//
// static void CALLBACK DS_TimerHandler( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime ) {

	HRESULT hr;
	DWORD	dwStatus = 0;
	int	i;

	// xprintf("timer\n");

	if( timer_munging == TRUE )
		return;

	for( i=0; i<MAXCHANNEL; i++ ) {

		if( channel[i].pDSB == NULL )
			continue;

		if( (hr = channel[i].pDSB->GetStatus( &dwStatus )) != DS_OK ) {
			channel[i].pDSB->Stop();
			channel[i].pDSB->Release();
			channel[i].pDSB = NULL;
			channel[i].pFileInfo = NULL;
			channel[i].fPlaying = FALSE;
			continue;
		}

		if( dwStatus & DSBSTATUS_BUFFERLOST ) {

			LPBYTE pbData, pbData2;
			DWORD  dwLength, dwLength2;

			//
			//  Restore the buffer, rewrite data, and play
			//
			if( (hr = channel[i].pDSB->Restore()) == DS_OK ) {

				hr = channel[i].pDSB->Lock( 0,
						channel[i].pFileInfo->cbSize,
						(void**)&pbData, &dwLength,
						(void**)&pbData2, &dwLength2,
						0 );

				if( hr == DS_OK ) {

					// if( (pbData == NULL) || (channel[i].pFileInfo->pbData == NULL) );

					memcpy( pbData, channel[i].pFileInfo->pbData, channel[i].pFileInfo->cbSize );

					hr = channel[i].pDSB->Unlock( pbData, dwLength, NULL, 0 );

					if( hr == DS_OK ) {

						if( channel[i].fPlaying )
							channel[i].pDSB->Play( 0, 0, channel[i].pFileInfo->fLooped ? DSBPLAY_LOOPING : 0 );

						channel[i].pDSB->GetStatus( &dwStatus );
					}
				}
			}
		}

		if( !(dwStatus & DSBSTATUS_BUFFERLOST) ) {
			if( !channel[i].fPlaying || (!(dwStatus & DSBSTATUS_PLAYING)) ) {
				channel[i].pDSB->Stop();
				channel[i].pDSB->Release();
				channel[i].pDSB = NULL;
				channel[i].pFileInfo = NULL;
				channel[i].fPlaying = FALSE;
			}
		}
	}

	return;
}




//
//
//
static int StartDSound( FILEINFO *pFileInfo ) {

	HRESULT hr = 0;
	DWORD	dwLooped;
	DWORD	dwStatus = 0;
	int	i;

	if( !pFileInfo )
		return FALSE;

	dwLooped = ( pFileInfo->fLooped ) ? DSBPLAY_LOOPING : 0;

	for( i=0; i<MAXCHANNEL; i++ )
		if( channel[i].pFileInfo == NULL )
			break;

	if( i >= MAXCHANNEL ) {
		xprintf("StartDSound: sound channel exceeded.\n");
		return 0;
	}

	timer_munging = TRUE;

	if( (hr = gpds->DuplicateSoundBuffer( pFileInfo->pDSB, &channel[i].pDSB )) == DS_OK ) {

		channel[i].pFileInfo = pFileInfo;

		if( (hr = channel[i].pDSB->Play( 0, 0, dwLooped)) != DS_OK )
			xprintf("StartDSound: cannot start playing.\n");
		else
			channel[i].fPlaying = TRUE;
	}
	else
		xprintf("StartDSound: duplicate failed. (%s)\n",TranslateDSError(hr));

	timer_munging = FALSE;

	return hr;
}





//
//
//
static int StopDSound( FILEINFO *pFileInfo ) {

	if( DSHinst == NULL )
		return 0;

	if( pFileInfo == NULL )
		return 0;

	timer_munging = TRUE;

	for( int i=0; i<MAXCHANNEL; i++ ) {
		if( channel[i].pFileInfo == pFileInfo ) {
			channel[i].fPlaying = FALSE;
			channel[i].pDSB->Stop();
			channel[i].pDSB->Release();
			channel[i].pDSB = NULL;
			channel[i].pFileInfo = NULL;
			break;
		}
	}

	timer_munging = FALSE;

	return 0;
}






//
//
//
static int StopAllDSounds( void ) {

	if( DSHinst == NULL )
		return 0;

	timer_munging = TRUE;

	for( int i=0; i<MAXCHANNEL; i++ ) {
		if( channel[i].pDSB ) {
			channel[i].fPlaying = FALSE;
			channel[i].pDSB->Stop();
			channel[i].pDSB->Release();
			channel[i].pDSB = NULL;
			channel[i].pFileInfo = NULL;
		}
	}

	timer_munging = FALSE;

	//timer_processed=0;
	//while( !timer_processed ) DX_GetMessage();

	return 0;
}



//
//
//
static int ChangeOutputPan( FILEINFO *pFileInfo ) {

	HRESULT hr = 0;

	if( !pFileInfo )
		return FALSE;

	int maxpan = ABS(DSBPAN_LEFT) + DSBPAN_RIGHT;
	int pan = DSBPAN_LEFT + ftoi( (FLOAT)pFileInfo->dwPan * (FLOAT)maxpan / (FLOAT)MAXPAN );

	if( (hr = pFileInfo->pDSB->SetPan(pan)) != DS_OK )
		xprintf("SetPan error: %d %d %s.\n",pan,pFileInfo->dwPan,TranslateDSError(hr));

	return hr;
}



//
//
//
static int ChangeOutputFreq( FILEINFO *pFileInfo ) {

	HRESULT hr = 0;

	if( !pFileInfo )
		return FALSE;

	if( (hr = pFileInfo->pDSB->SetFrequency(pFileInfo->dwFreq)) != 0 )
		xprintf("SetFrequency error: %s.\n",TranslateDSError(hr));

	return hr;
}



//
//
//
static int ChangeOutputVol( FILEINFO *pFileInfo ) {

	HRESULT hr = 0;
	int dwMinVol=DS_MINVOL,dwMaxVol=DS_MAXVOL;

	if( !pFileInfo )
		return FALSE;

	// int vol = DS_MINVOL + ftoi( (FLOAT)pFileInfo->dwVol * (FLOAT)(ABS(DS_MAXVOL-DS_MINVOL)) / (FLOAT)MAXVOL );

	// TODO: normális értékek DSound decibelel számol
	if( pFileInfo->dwVol > 0 )
		dwMinVol = -1700;

	int maxvol = ABS(dwMinVol) + dwMaxVol;
	int vol = dwMinVol + ftoi( (FLOAT)pFileInfo->dwVol * (FLOAT)maxvol / (FLOAT)MAXVOL );

	// xprintf("vol: %d\n",vol);

	if( (hr = pFileInfo->pDSB->SetVolume(vol)) != DS_OK )
		xprintf("SetVolume error: %s.\n",TranslateDSError(hr));

	return hr;
}




//
//
//
static int FormatToIndex( FILEINFO *pFileInfo ) {

	WAVEFORMATEX wfx;
	DWORD	    dwWaveStyle;
	DWORD	    dwSize;
	int	    nError = 0;

	// Get the format.
	if( (nError = pFileInfo->pDSB->GetFormat( &wfx, sizeof(wfx), &dwSize)) != 0 ) {
		goto ERROR_IN_ROUTINE;
	}

	if( dwSize > sizeof( wfx ) ) {
		nError = DSERR_GENERIC;
		goto ERROR_IN_ROUTINE;
	}

	// Change wfx to an integer.
	// Assume theres an error and check all parameters to
	// see if its valid.
	nError = LB_ERR;
	dwWaveStyle = 0;

	if( wfx.wFormatTag != WAVE_FORMAT_PCM )
		goto ERROR_IN_ROUTINE;

	// Check the channels
	switch( wfx.nChannels ) {

		case 1:
			break;

		case 2:
			dwWaveStyle |= 1;
			break;

		default:
			goto ERROR_IN_ROUTINE;
	}

	// Check the bits...
	switch( wfx.wBitsPerSample ) {

		case 8:
			break;

		case 16:
			dwWaveStyle |= 2;
			break;

		default:
			goto ERROR_IN_ROUTINE;
	}

	// Check the rate.
	switch( wfx.nSamplesPerSec ) {

		case 8000:
			break;

		case 11025:
			dwWaveStyle |= 4;
			break;

		case 22050:
			dwWaveStyle |= 8;
			break;

		case 44100:
			dwWaveStyle |= 12;
			break;

		default:
			goto ERROR_IN_ROUTINE;
	}

	nError = (int)dwWaveStyle;

	ERROR_IN_ROUTINE:

	return nError;
}




//
//
//
static int IndexToFormat( FILEINFO *pFileInfo, int index ) {

	int nError = 0;

	pFileInfo->pwfx->wFormatTag = WAVE_FORMAT_PCM;

	pFileInfo->pwfx->nChannels = 2; 	// Assume stereo.
	if( (index%2) == 0 )
		pFileInfo->pwfx->nChannels = 1; 	// Its mono.

	// Assume 16 bit
	pFileInfo->pwfx->nBlockAlign = 2*pFileInfo->pwfx->nChannels;
	pFileInfo->pwfx->wBitsPerSample = 16;
	if( (index%4) < 2 ) {
		// Its 8 bit.
		pFileInfo->pwfx->nBlockAlign = 1*pFileInfo->pwfx->nChannels;
		pFileInfo->pwfx->wBitsPerSample = 8;
	}

	pFileInfo->pwfx->nSamplesPerSec = 44100;    // Assume 44.1 kHz
	if( index < 4 )
		pFileInfo->pwfx->nSamplesPerSec = 8000;
	else
	if( index < 8 )
		pFileInfo->pwfx->nSamplesPerSec = 11025;
	else
	if( index < 12 )
		pFileInfo->pwfx->nSamplesPerSec = 22050;

	pFileInfo->pwfx->nAvgBytesPerSec = pFileInfo->pwfx->nSamplesPerSec *
					       pFileInfo->pwfx->nBlockAlign;
	pFileInfo->pwfx->cbSize = 0;

	if( (nError = pFileInfo->pDSB->SetFormat( pFileInfo->pwfx )) != DS_OK )
		xprintf("Cannot set format buffer. (%s)",TranslateDSError(nError));

	return nError;
}




//
//
//
void DS_FreeAllWave( void ) {

	StopAllDSounds();
	FreeAllList();

	return;
}



//
//
//
BOOL DS_IsLoaded( char *name ) {

	FILEINFO *pFileInfoFirst;

	pFileInfoFirst = FileInfoFirst.pNext;

	while( pFileInfoFirst != NULL ) {

		if( !stricmp( pFileInfoFirst->szFileName, name ) )
			return TRUE;

		pFileInfoFirst = pFileInfoFirst->pNext;
	}

	return FALSE;
}



//
//
//
void DS_PlayWave( int handler ) {

	FILEINFO *pFileInfo;

	if( (pFileInfo = GetFileInfo( handler ) ) != NULL )
		StartDSound( pFileInfo );
	else
		xprintf("DS_PlayWave: can't find %d handler.\n",handler);

	return;
}





//
//
//
void DS_StopWave( int handler ) {

	FILEINFO *pFileInfo;

	if( (pFileInfo = GetFileInfo( handler ) ) != NULL )
		StopDSound( pFileInfo );

	// xprintf("DS_StopWave: can't find %d handler.\n",handler);

	return;
}





//
//
//
int DS_IsPlaying( int handler ) {

	FILEINFO *pFileInfo;
	int i;

	if( (pFileInfo = GetFileInfo( handler ) ) != NULL ) {
		for( i=0; i<MAXCHANNEL; i++ ) {
			if( (channel[i].pFileInfo == pFileInfo) )
				return channel[i].fPlaying;
		}
	}
	else
		xprintf("DS_IsPlaying: can't find %d handler.\n",handler);

	return FALSE;
}






//
//
//
void DS_FreeWave( int handler ) {

	FILEINFO *pFileInfo;

	if( (pFileInfo = GetFileInfo( handler ) ) != NULL ) {
		StopDSound( pFileInfo );
		RemoveFromList( pFileInfo );
	}
	else
		xprintf("DS_FreeWave: can't find %d handler.\n",handler);

	return;
}






//
//
//
void DS_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped ) {

	FILEINFO *pFileInfo;

	if( (pFileInfo = GetFileInfo( handler ) ) != NULL ) {

		if( dwFreq != -1 ) pFileInfo->dwFreq = dwFreq;
		if( dwPan != -1 ) pFileInfo->dwPan = dwPan;
		if( dwVol != -1 ) pFileInfo->dwVol = dwVol;
		if( fLooped != -1 ) pFileInfo->fLooped = fLooped;

		ChangeOutputFreq( pFileInfo );
		ChangeOutputPan( pFileInfo );
		ChangeOutputVol( pFileInfo );
	}
	else
		xprintf("DS_SetWaveParam: can't find %d handler.\n",handler);

	return;
}






//
//
//
static char *TranslateDSError( HRESULT hr ) {

	switch( hr ) {
		case DSERR_ALLOCATED:
			return "DSERR_ALLOCATED";

		case DSERR_CONTROLUNAVAIL:
			return "DSERR_CONTROLUNAVAIL";

		case DSERR_INVALIDPARAM:
			return "DSERR_INVALIDPARAM";

		case DSERR_INVALIDCALL:
			return "DSERR_INVALIDCALL";

		case DSERR_GENERIC:
			return "DSERR_GENERIC";

		case DSERR_PRIOLEVELNEEDED:
			return "DSERR_PRIOLEVELNEEDED";

		case DSERR_OUTOFMEMORY:
			return "DSERR_OUTOFMEMORY";

		case DSERR_BADFORMAT:
			return "DSERR_BADFORMAT";

		case DSERR_UNSUPPORTED:
			return "DSERR_UNSUPPORTED";

		case DSERR_NODRIVER:
			return "DSERR_NODRIVER";

		case DSERR_ALREADYINITIALIZED:
			return "DSERR_ALREADYINITIALIZED";

		case DSERR_NOAGGREGATION:
			return "DSERR_NOAGGREGATION";

		case DSERR_BUFFERLOST:
			return "DSERR_BUFFERLOST";

		case DSERR_OTHERAPPHASPRIO:
			return "DSERR_OTHERAPPHASPRIO";

		case DSERR_UNINITIALIZED:
			return "DSERR_UNINITIALIZED";
	}

	return "Unknown HRESULT";
}



