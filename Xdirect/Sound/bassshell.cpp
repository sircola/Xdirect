
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>


#define LOADBASSFUNCTION(f) *((void**)&f)=(void*)GetProcAddress(hBass,#f)
#define BASSDEF(f) (WINAPI *f)	// define the functions as pointers

#define LOADBASSMIDIFUNCTION(f) *((void**)&f)=(void*)GetProcAddress(hBassMidi,#f)
#define BASSMIDIDEF(f) (WINAPI *f)	// define the functions as pointers

#define LOADBASSENCFUNCTION(f) *((void**)&f)=(void*)GetProcAddress(hBassEnc,#f)
#define BASSENCDEF(f) (WINAPI *f)	// define the functions as pointers

#define LOADBASSAACFUNCTION(f) *((void**)&f)=(void*)GetProcAddress(hBassAac,#f)
#define BASSAACDEF(f) (WINAPI *f)	// define the functions as pointers


#include <c:\\My Projects\\Bass\\c\\bass.h>
#include <c:\\My Projects\\Bass\\Bassmidi\\c\\bassmidi.h>
#include <c:\\My Projects\\Bass\\Enc\\c\\bassenc.h>
#include <c:\\My Projects\\Bass\\Aac\\c\\bass_aac.h>

#include <xlib.h>

#ifdef BASS_LIB

RCSID( "$Id: bassshell.cpp,v 1.0 2011/05/25 20:53:50 bernie Exp $" )



// http://www.un4seen.com/

#define BASSDLLNAME  "BASS.DLL"



//
//
//
typedef struct wave_s {

   	int      	dwFreq; 		// Frequency.

						// pan The pan position... -1 (full left) to +1 (full right), 0 = centre.
	int      	dwPan;			// Panning info.   MAXPAN
   	int		dwVol;			// Total volume.
   	BOOL		fLooped;		// Looped?

	HSAMPLE		samp;
	HSTREAM		stream;
	HCHANNEL 	channel;

	UCHAR		*stream_buf;
	int		size;

	int 	   	handler;

	int 		nFileName;		// Index to filename, not including dir.
	char		szFileName[XMAX_PATH];	// MAX_PATH

	HDSP		ampdsp;
	FLOAT 		gain;			// amplification level
	int 		delay; 			// delay before increasing level
	int 		count; 			// count of sequential samples below target level
	FLOAT 		high; 			// the highest in that period
	int 		quiet; 			// count of sequential samples below quiet level

	int 		libsnd_chn;
	int 		libsnd_bits;
	int 		libsnd_freq;

	struct wave_s	*pNext; 		// Pointer to next file.

} wave_t, *wave_ptr;

#define WAVAMPVAL 	28000  // 28000
#define MIDIAMPVAL 	10000
#define MODAMPVAL 	3100

static BOOL bIsInited = FALSE;


// BASS_ATTRIB_MUSIC_VOL_GLOBAL
// The global volume level... 0 (min) to 64 (max, 128 for IT format). This will be rounded down to a whole number
// csak music
#define BASSMAXVOL 64

static wave_t WaveFirst = {0, 0,0,0, 0,0,0, 0,0, 0, 0,"", 0,0,0,0,0,0, 0,0,0, NULL };


static HMUSIC lpModule = 0;


static HINSTANCE hBass = 0;		// bass handle


static ULONG lastPlayedChannel = 0;	// hogy legyen mirõl infót kérni;
static FLOAT *fft = NULL;
static int size_fft = 0;

static BOOL bCheckvol = FALSE;
static int touch_ord,touch_row,touch_tic;
static BOOL music_loop = FALSE;
static int music_volume = 0;

static int normalize_sec = 1;		// ennyi másodpercig dolgoz fel max


//
// load BASS and the required functions
//
static int LoadBASS( void ) {

	/***
	// BYTE *data;
	// HANDLE hres,hfile;
	// DWORD len;
	// char temppath[XMAX_PATH];

	// get the BASS.DLL resource
	if (!(hres=FindResource(GetModuleHandle(NULL),"BASS_DLL",RT_RCDATA))
		|| !(len=SizeofResource(NULL,hres))
		|| !(hres=LoadResource(NULL,hres))
		|| !(data=LockResource(hres))) {
		xprintf("Error: Can't get the BASS.DLL resource\n");
		return 1;
	}

	// get a temporary filename
	GetTempPath(MAX_PATH,temppath);
	GetTempFileName(temppath,"bas",0,tempfile);

	// write BASS.DLL to the temporary file
	if( INVALID_HANDLE_VALUE==(hfile=CreateFile(tempfile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL)) ) {
		xprintf("Error: Can't write BASS.DLL\n");
		return 2;
	}

	WriteFile(hfile,data,len,&c,NULL);
	CloseHandle(hfile);
	***/

	char bassdllname[XMAX_PATH];

	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, BASSDLLNAME );

	// load the temporary BASS.DLL library
	if( (hBass = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadBASS: couldn't load \"%s\" library.\n",bassdllname);
		return 3;
	}

	{
	// crc check
	char *buf;
	int size = LoadFile(bassdllname,PTR(buf));
	UINT crc = 0;
	CalcCrc( buf, size, &crc);
	if( crc != 2604408541UL ) {
		// Quit( "Install is damaged.\n\nPlease, reinstall application.\n\nThank You!" );
		VAPI( FreeLibrary( hBass ) );
		hBass = NULL;
		return 2;
	}
	FREEMEM(buf);
	}

	LOADBASSFUNCTION(BASS_GetVersion);

	LOADBASSFUNCTION( BASS_ErrorGetCode );
	LOADBASSFUNCTION( BASS_Init );
	LOADBASSFUNCTION( BASS_Free );
	LOADBASSFUNCTION( BASS_GetCPU);
	LOADBASSFUNCTION( BASS_Start );
	LOADBASSFUNCTION( BASS_SetVolume );
	LOADBASSFUNCTION( BASS_MusicLoad );
	LOADBASSFUNCTION( BASS_ChannelPlay );
	LOADBASSFUNCTION( BASS_StreamCreateFile );
	LOADBASSFUNCTION( BASS_StreamCreate );
	LOADBASSFUNCTION( BASS_ChannelIsActive );
	LOADBASSFUNCTION( BASS_ChannelGetInfo);
	LOADBASSFUNCTION( BASS_ChannelSetAttribute);
	LOADBASSFUNCTION( BASS_ChannelGetAttribute);
	LOADBASSFUNCTION( BASS_ChannelGetPosition);
	LOADBASSFUNCTION( BASS_ChannelGetLevel);
	LOADBASSFUNCTION( BASS_ChannelSetSync );
	LOADBASSFUNCTION( BASS_GetVersion );
	LOADBASSFUNCTION( BASS_GetDeviceInfo );
	LOADBASSFUNCTION( BASS_SampleLoad );
	LOADBASSFUNCTION( BASS_SampleStop );
	LOADBASSFUNCTION( BASS_SampleFree );
	LOADBASSFUNCTION( BASS_SampleGetInfo );
	LOADBASSFUNCTION( BASS_SampleSetInfo);
	LOADBASSFUNCTION( BASS_ChannelGetTags);
	LOADBASSFUNCTION( BASS_ChannelSetPosition );
	LOADBASSFUNCTION( BASS_ChannelStop );
	LOADBASSFUNCTION( BASS_MusicFree);
	LOADBASSFUNCTION( BASS_Stop );
	LOADBASSFUNCTION( BASS_Pause );
	LOADBASSFUNCTION( BASS_Start );
	LOADBASSFUNCTION( BASS_ChannelSlideAttribute );
	LOADBASSFUNCTION( BASS_SampleGetChannel );
	LOADBASSFUNCTION( BASS_ChannelGetData );
	LOADBASSFUNCTION( BASS_ChannelGetLength );
	LOADBASSFUNCTION( BASS_ChannelBytes2Seconds );
	LOADBASSFUNCTION( BASS_ChannelLock );
	LOADBASSFUNCTION( BASS_ChannelFlags );
	LOADBASSFUNCTION( BASS_ChannelPause );
	LOADBASSFUNCTION( BASS_StreamCreateURL );
	LOADBASSFUNCTION( BASS_StreamGetFilePosition );
	LOADBASSFUNCTION( BASS_SetConfig );
	LOADBASSFUNCTION( BASS_SetConfigPtr );
	LOADBASSFUNCTION( BASS_StreamFree );
	LOADBASSFUNCTION( BASS_ChannelSetDSP );
	LOADBASSFUNCTION( BASS_ChannelRemoveDSP );
	LOADBASSFUNCTION( BASS_ChannelSeconds2Bytes );
	LOADBASSFUNCTION( BASS_StreamPutData );

	LOADBASSFUNCTION( BASS_SetVolume );
	LOADBASSFUNCTION( BASS_GetVolume );

	xprintf("LoadBASS: BASS (%s) loaded.\n",bassdllname);

	return 0;
}



//
// free the BASS library from memory and delete the temporary file
//
static void FreeBASS( void ) {

	if( !hBass )
		return;

	FreeLibrary(hBass);
	hBass=0;

	// DeleteFile(tempfile);

	return;
}



//
//
//
BOOL BLIB_Init( int freq ) {

	if( bIsInited == TRUE )
		return TRUE;

	if( winNoSound() && winNoMusic() )
		return TRUE;

	if( LoadBASS() )
		return FALSE;

	memset( &WaveFirst, 0L, sizeof(wave_t) );
	WaveFirst.handler = -1;
	WaveFirst.pNext = NULL;

	int ver = BASS_GetVersion();
	xprintf("BASS_Init: BASS v%x.%x.%x.%x detected (v%x.%x compiled).\n", HIBYTE(HIWORD(ver)), LOBYTE(HIWORD(ver)), HIBYTE(LOWORD(ver)), LOBYTE(LOWORD(ver)), HIBYTE(BASSVERSION), LOBYTE(BASSVERSION) );

	int dev = -1;
	if( freq < 1 )
		freq = 22050; // 48000 44100 22050 11025 8000
	int flag = 0; // BASS_DEVICE_LATENCY;

	// if (!BASS_Init(-1,44100,0,win,NULL))

	// Initialize digital sound - default device, 44100hz, stereo, 16 bits, no syncs used
	if( !BASS_Init( dev, freq, flag,(HWND)GetHwnd(), NULL ) ) {
		xprintf("BASS_Init: init failed (error = %d).\n", BASS_ErrorGetCode() );
		FreeBASS();
		return FALSE;
	}

	// xprintf("BASS_Init: \"%s\" device.\n", BASS_GetDeviceDescription(dev) );

	int a, count=0;
	BASS_DEVICEINFO info;
	for( a=0; BASS_GetDeviceInfo(a, &info); a++ )
		if( info.flags & BASS_DEVICE_ENABLED ) { // device is enabled
			xprintf("BASS_Init: \"%s\" (%s) device.\n", info.name, info.driver );
			count++; // count it
		}
	/***
	// Initialize CD
	if( !BASS_CDInit(("E:",0) )
		xprintf("BASS_Init: Can't initialize CD system.\n");
	***/

	/* Use meters as distance unit, real world rolloff, real doppler effect */
	// BASS_Set3DFactors(1.0, 1.0, 1.0);

	/* Turn EAX off (volume=0.0), if error then EAX is not supported */
	// BASS_SetEAXParameters(-1, 0.0, -1.0, -1.0);

	BASS_Start();	/* Start digital output */

	bIsInited = TRUE;

	return TRUE;
}



//
//
//
BOOL BLIB_Deinit( void ) {

	if( !bIsInited )
		return TRUE;

	BLIB_StopMod();
	BLIB_FreeAllWave();

	BASS_Stop();		/* Stop digital output */

	BASS_Free();		/* Close digital sound system */
	// BASS_CDFree();	/* Close CD system */

	FreeBASS();

	if( fft ) FREEMEM( fft );
	fft = NULL;

	bIsInited = FALSE;

	return TRUE;
}



// 0 - 65535.0
static int volSteps[26] = {         0,   2621,   5242,   7863,
				10484,  13105,  15726,  18347,
				20968,  23589,  26210,  28831,
				31452,  34073,  36694,  39315,
				41936,  44557,  47178,  49799,
				52420,  55041,  57662,  60283,
				62904,  65535 };


//
// 0-BASSMAXVOL = 0-64
//
static FLOAT getLogVol( int vol ) {
/*
	xprintf( "getLogVol: in = %d\n", vol );

	int v = volSteps[ ftoi( (FLOAT)vol / ((FLOAT)BASSMAXVOL / 26.0f) ) ];

	vol = v * BASSMAXVOL / 65535.0f;

	xprintf( "getLogVol: out = %d\n", vol );
*/
	return vol;
}


//
//
//
static FLOAT getUnLogVol( int vol ) {
/*
	int v = vol * 65535.0f / BASSMAXVOL;

	int diff = 999999999;
	int index = -1;

	for( int i=0; i<26; i++ ) {

		if( ABS( volSteps[i] - v ) < diff )
			index = i;
	}

	vol = index * ((FLOAT)BASSMAXVOL / 26.0f);
*/
	return vol;
}





#define MUSICTARGET 	13000 		// target level
#define MUSICQUIET 	3000 		// quiet level
#define MUSICRATE 	0.02f 		// amp adjustment rate

static FLOAT music_gain = 1.0f;	// amplification level
static int music_delay = 1; 	// delay before increasing level
static int music_count = 0;	// count of sequential samples below target level
static int music_high = 0;	// the highest in that period
static int music_quiet = 0;	// count of sequential samples below quiet level

static HDSP music_dsp = 0;


//
// Dinamikus normalizálicó
//
static void CALLBACK musicDSP( HDSP handle, DWORD channel, void *buffer, DWORD length, void *user ) {

	short *data = (short *)buffer;
	DWORD c;

	if( length == 0 || buffer == NULL )
		return;

	for( c=0; c<length/2; c++ ) {

		short s = (FLOAT)data[c] * music_gain; // amplify sample
		short sa = abs(s);

		if( abs(data[c]) < MUSICQUIET )
			music_quiet++; // sample is below quiet level
		else
			music_quiet = 0;

		if( sa < MUSICTARGET ) { // amplified level is below target

			if( sa > music_high )
				music_high = sa;

			music_count++;

			if( music_count >= music_delay ) { // been below target for a while

				if( music_quiet > music_delay )
					// it's quiet, go back towards normal level
					music_gain += 10 * MUSICRATE * (1-music_gain);
				else
					// increase amp
					music_gain += MUSICRATE * MUSICTARGET / (FLOAT)music_high;

				music_high = music_count = 0; // reset counts
			}
		}
		else { // amplified level is above target
			if( s < -32768 ) s = -32768;
			else
			if( s > 32767 ) s = 32767;

			music_gain -= 2 * MUSICRATE * (FLOAT)sa / MUSICTARGET; // decrease amp
			music_high = music_count = 0;
		}

		data[c] = s; // replace original sample with amplified version
	}

	// biztonsági szelep, de mi a picsának kell
	if( music_gain <= MUSICRATE || music_gain > 5.0f )
		music_gain = 1.0f;

	return;
}




//
// getAmp
//
static int MOD_getGain( HMUSIC lpm, char *loaded_file, int size, BOOL log = FALSE ) {

	HMUSIC chan = 0;
	int p, pos;
	short buf[10000];
	int amp = 50;
	int iter;
	BOOL hasLoop = FALSE;

	chan = BASS_MusicLoad(TRUE,loaded_file,0,size,BASS_MUSIC_DECODE|BASS_MUSIC_PRESCAN,0);

	if( chan == 0 ) {
		xprintf("MOD_getGain: Error loading (%d)\n", BASS_ErrorGetCode() );
		return amp;
	}

	BASS_ChannelSetAttribute( chan, BASS_ATTRIB_MUSIC_AMPLIFY, amp );
	BASS_ChannelSetAttribute( chan, BASS_ATTRIB_MUSIC_VOL_GLOBAL, (FLOAT)MAXVOL );

	if( log ) {
		// count channels

		float dummy;
		for( p=0; BASS_ChannelGetAttribute(chan,BASS_ATTRIB_MUSIC_VOL_CHAN+p,&dummy); p++ );

		BASS_CHANNELINFO info;
		BASS_ChannelGetInfo(chan,&info);

		xprintf("MOD_getGain: \"%s\" [%u chans, %u orders]\n",
				BASS_ChannelGetTags(chan,BASS_TAG_MUSIC_NAME),
				p,
				(DWORD)BASS_ChannelGetLength(chan,BASS_POS_MUSIC_ORDER));
	}

	for( iter=0; iter<5; iter++ ) {

		ULONG end_tic = GetTic() + TICKBASE * normalize_sec;

		__int64 max_avg = 0;
		ULONG max_cnt = 0;
		__int64 min_avg = 0;
		ULONG min_cnt = 0;

		while( BASS_ChannelIsActive(chan) ) {

			int c = BASS_ChannelGetData( chan, buf, 20000 );

			// xprintf("c = %d\n", c );

			for( int i=0; i<c/2; i++ ) {
				if( buf[i] > 500 ) {
					max_avg += buf[i];
					++max_cnt;
				}
				else
				if( buf[i] < -500 ) {
					min_avg += buf[i];
					++min_cnt;
				}
				else {  // == 0
					// ++max_cnt;
					// ++min_cnt;
				}
			}

			// amit 3 másodperc alatt fel tud dolgozni
			if( end_tic < GetTic() ) {
				if( log ) xprintf("getAmp: time is up.\n" );
				hasLoop = TRUE;
				break;
			}
		}

		max_avg /= (!max_cnt?1:max_cnt);
		min_avg /= (!min_cnt?1:min_cnt);

		if( log ) {
			xprintf("max_avg = %d, cnt = %d\n", (int)max_avg, max_cnt );
			xprintf("min_avg = %d, cnt = %d\n", (int)min_avg, min_cnt );
		}

		if( (min_avg > -MODAMPVAL) && (max_avg < MODAMPVAL) )
			break;
/***
		// ha közte van
		if( min > -32768 && min < -AMPVAL &&
		    max < 32767 && max > AMPVAL ) {

			if( ABS(min) < max )
				amp = ftoi( (FLOAT)AMPVAL * (FLOAT)amp / (FLOAT)max );
			else
				amp = ftoi( (FLOAT)-AMPVAL * (FLOAT)amp / (FLOAT)min );
		}
		else
			// teljesen el van szállva a volume
			amp /= 2;
***/
		amp /= 2;

		if( amp <= 1 )
			break;

		if( log ) xprintf("MOD_getGain: reamp = %d.\n", amp );
		BASS_ChannelSetPosition( chan, 0, BASS_POS_BYTE );
		BASS_ChannelSetAttribute( chan, BASS_ATTRIB_MUSIC_AMPLIFY, amp );
		if( lpm ) BASS_ChannelSetAttribute( lpm, BASS_ATTRIB_MUSIC_AMPLIFY, amp );
	}

	DWORD q = (DWORD)BASS_ChannelBytes2Seconds(chan,BASS_ChannelGetPosition(chan,BASS_POS_BYTE));
	xprintf("MOD_getGain: amp = %d in %d steps, playtime = %d:%02d min%s\n", amp, !iter?1:iter+1, q/60,q%60,hasLoop?", probably looped.":"." );

	BASS_ChannelStop( chan );
	BASS_MusicFree( chan );

	return amp;
}



static int *instr_tic = NULL;
static int max_instr = 0;

//
//
//
void CALLBACK InstrSync( HSYNC handle, DWORD channel, DWORD data, void *user ) {

	int chn = (int)user;

	// xprintf( "user = %d, max_instr = %d\n", chn, max_instr );

	instr_tic[ chn ] = GetTic();

	return;
}


// ha már egyszer volt vol állítás akkor mindig erre lövi be
static FLOAT prev_volume = -1;


//
//
//
BOOL BLIB_PlayMod( const char *txt ) {

	int flag = BASS_MUSIC_LOOP;
	char *loaded_file = NULL;
	int size = 0;

	bCheckvol = FALSE;

	if( winNoMusic() )
		return TRUE;

	if( BLIB_Init() == FALSE )
		return TRUE;

	if( lpModule ) {
		BASS_ChannelStop( lpModule );
		BASS_MusicFree( lpModule );
		lpModule = 0;
	}

	if( (size = LoadFile( txt, PTR(loaded_file) )) == 0 ) {
		xprintf("BASS_PlayMod: LoadFile \"%s\" error.\n", txt );
		return FALSE;
	}

	if( (lpModule = BASS_MusicLoad(TRUE,loaded_file,0,size,flag,0)) == 0 ) {
		FREEMEM( loaded_file );
		lpModule = 0;
		xprintf("BASS_PlayMod: Error loading \"%s\". (%d)\n", txt, BASS_ErrorGetCode() );
		return FALSE;
	}

	FLOAT vol;

	BASS_ChannelGetAttribute( lpModule, BASS_ATTRIB_MUSIC_VOL_GLOBAL, &vol );
	BASS_ChannelSetAttribute( lpModule, BASS_ATTRIB_MUSIC_VOL_GLOBAL, 0.0f );

	if( prev_volume > 0 )
		BASS_ChannelSlideAttribute( lpModule, BASS_ATTRIB_MUSIC_VOL_GLOBAL, prev_volume, prev_volume * 1000 / BASSMAXVOL );

	// dynamic amplify
	if( 0 ) {
		music_delay = BASS_ChannelSeconds2Bytes( lpModule, 1 ) / 4;
		music_gain = 1;
		music_count = 0;
		music_high = 0;
		music_quiet = 0;
		music_dsp = BASS_ChannelSetDSP( lpModule, (DSPPROC*)&musicDSP, NULL, 0 );
	}

	BASS_ChannelPlay(lpModule,FALSE);

	// egyszeri amplify
	if( 1 ) {
		BASS_ChannelSetAttribute( lpModule, BASS_ATTRIB_MUSIC_AMPLIFY, 1 );
		BASS_ChannelSetAttribute( lpModule, BASS_ATTRIB_MUSIC_AMPLIFY, MOD_getGain(lpModule,loaded_file,size) ); // 50 = default
	}

	FREEMEM( loaded_file );

	{
		float dummy;
		int max_chan=0;
		for( ; BASS_ChannelGetAttribute(lpModule,BASS_ATTRIB_MUSIC_VOL_CHAN+max_chan,&dummy); max_chan++ );

		// instruments
		max_instr = 0;
		while( BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_INST + max_instr ) ) ++max_instr;

		// samples
		int max_samp = 0;
		while( BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_SAMPLE + max_samp ) ) ++max_samp;

		// xprintf( "BASS_PlayMod: chan: %d, instr: %d, samp: %d\n", max_chan, max_instr, max_samp );

		if( max_instr <= 0 )
			max_instr = max_samp;

		if( instr_tic )
			FREEMEM( instr_tic );
		ALLOCMEM( instr_tic, max_instr * sizeof(int) );
		memset( instr_tic, 0L, max_instr * sizeof(int) );

		for( int i=1; i<=max_instr; i++ )
			BASS_ChannelSetSync( lpModule, BASS_SYNC_MUSICINST, 0xffff0000 + i, InstrSync, (void *)(i-1) );
	}

	lastPlayedChannel = lpModule;

	touch_ord = 0;
	touch_row = 0;
	touch_tic = GetTic();
	bCheckvol = TRUE;

	xprintf("BASS_PlayMod: playing \"%s\".\n",BASS_ChannelGetTags(lpModule, BASS_TAG_MUSIC_NAME));

	return TRUE;
}




//
//
//
BOOL BLIB_StopMod( void ) {

	bCheckvol = FALSE;

	if( lpModule == 0 )
		return TRUE;

	// BASS_ChannelSlideAttribute( lpModule, -1, -2, -1, 1000 );

	if( music_dsp )
		BASS_ChannelRemoveDSP( lpModule, music_dsp );

	BASS_ChannelStop( lpModule );
	BASS_MusicFree( lpModule );

	if( instr_tic )
		FREEMEM( instr_tic );
	max_instr = 0;

	lpModule = 0;

	return TRUE;
}




//
//
//
BOOL BLIB_SetModuleParam( int vol, int nOrder, int nRow, int loop ) {

	if( lpModule == 0 )
		return TRUE;

	xprintf("BLIB_SetModuleParam: called with %d, %d, %d\n", vol, nOrder, nRow );

	if( vol > -1 ) {
		vol = getLogVol( vol );
		// 0 - 64
		float v = (FLOAT)vol * (FLOAT)BASSMAXVOL / (FLOAT)MAXVOL;
		BASS_ChannelSetAttribute( lpModule, BASS_ATTRIB_MUSIC_VOL_GLOBAL, v );
		// BASS_ChannelSlideAttribute( lpModule, BASS_ATTRIB_MUSIC_VOL_GLOBAL, v, 1000 );
		prev_volume = v;
	}

	if( (nOrder != -1) || (nRow != -1) ) {
		DWORD p;
		p = (DWORD)BASS_ChannelGetPosition( lpModule, BASS_POS_MUSIC_ORDER );
		if(nOrder == -1) nOrder = LOWORD(p);
		if(nRow == -1) nRow = HIWORD(p);
		BASS_ChannelSetPosition(lpModule, (QWORD)MAKELONG(nOrder, nRow),BASS_POS_MUSIC_ORDER);
	}

	if( loop != -1 ) {

		if( loop )
			BASS_ChannelFlags( lpModule, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
		else
			BASS_ChannelFlags(lpModule, 0, BASS_SAMPLE_LOOP);

	}

	BASS_ChannelPlay(lpModule, FALSE);

	return TRUE;
}




//
//
//
int BLIB_GetInfo( int flag ) {

	if( lpModule == 0 )
		return 0;

	int num = 0;

	// instruments
	if( flag == 0 )
		while( BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_INST + num ) ) ++num;

	// samples
	if( flag == 1 )
		while( BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_SAMPLE + num ) ) ++num;

	if( flag >= 100 && (flag - 100) < max_instr && instr_tic != NULL )
		num = instr_tic[ flag - 100 ];

	return num;
}


//
//
//
char *BLIB_GetStrings( int num, int flag ) {

	if( lpModule == 0 )
		return NULL;

	// MOD instrument name. Only available with formats that have instruments, eg. IT and XM (and MO3).
	if( flag == 0 ) {
		char *str = (char *)BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_INST + num );
		// if( str == NULL || str[0] == 0 )
		//	return NULL;
		return str;
	}

	if( flag == 1 ) {
		char *str = (char *)BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_SAMPLE + num );
		// if( str == NULL || str[0] == 0 )
		//	return NULL;
		return str;
	}

	if( flag == 2 ) {
		char *str = (char *)BASS_ChannelGetTags( lpModule, BASS_TAG_MUSIC_MESSAGE );
		// if( str == NULL || str[0] == 0 )
		//	return NULL;
		return str;
	}

	return NULL;
}




//
//
//
BOOL BLIB_GetModuleParam( int *vol, int *ord, int *row, int *max_order, int *percent, char **song_name, int *max_chan ) {

	if( lpModule == 0 )
		return TRUE;

	// xprintf("music: gain = %.2f, count = %d, high = %d, quiet = %d\n", music_gain,music_count,music_high,music_quiet );

	if( vol ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( lpModule, BASS_ATTRIB_MUSIC_VOL_GLOBAL, &fvol );
		*vol = getUnLogVol( ftoi( (FLOAT)(fvol) * (FLOAT)MAXVOL / (FLOAT)BASSMAXVOL ) );
	}

	DWORD p;
	p = (DWORD)BASS_ChannelGetPosition( lpModule, BASS_POS_MUSIC_ORDER );

	if( ord ) *ord = LOWORD( p );
	if( row ) *row = HIWORD( p );

	int mod_max_order = (int)BASS_ChannelGetLength( lpModule, BASS_POS_MUSIC_ORDER );

	if( max_order )
		*max_order = mod_max_order;

	if( song_name )
 		*song_name = (char *)BASS_ChannelGetTags( lpModule,BASS_TAG_MUSIC_NAME );

	// percent
	if( percent ) {

		int maxPos = mod_max_order;
		int pos = LOWORD( p ) + 1;

		if( maxPos )
			*percent = (int)(pos * 100 / maxPos);
		else
			*percent = 0;

		if( pos > maxPos )
			*percent = 0;
	}

	// count channels
	if( max_chan ) {
		float dummy;
		for( (*max_chan)=0; BASS_ChannelGetAttribute(lpModule,BASS_ATTRIB_MUSIC_VOL_CHAN+(*max_chan),&dummy); (*max_chan)++ );
	}

	return TRUE;
}



//
// volume checker
// meg loop checker
// TODO: az lz.cpp-ben van benne, ide is kellene
//
static void modChecker( void ) {

	if( lpModule == 0 || bCheckvol == FALSE )
		return;

	int vol,ord,row;

	BLIB_GetModuleParam(&vol,&ord,&row);

	if( vol > music_volume )
		BLIB_SetModuleParam( music_volume );

	if( ord > touch_ord ) {
		// ha a következö patternre ugrott
		touch_ord = ord;
		touch_row = row;
		touch_tic = GetTic();
	}
	else
	if( (ord < touch_ord) && music_loop == FALSE ) {
		// vissza ugrott patternt
		// force_next = TRUE;
		// xprintf("modChecker: pattern loop detected, forcing next.\n");
	}
	else
	if( ord == touch_ord && row < touch_row && music_loop == FALSE ) {
		// ugyanaz a pattern, de a visszaugrott az elejére
		// force_next = TRUE;
		// xprintf("modChecker: looping same pattern, forcing next.\n");
	}
	if( ord == touch_ord && row == touch_row && music_loop == FALSE ) {
		// egy soron idözik már egy ideje, esetleg megállt
		// tempo = 0;
		if( GetTic() - touch_tic > TICKBASE*10 ) {
			// force_next = TRUE;
			// xprintf("modChecker: snail tempo detected WTF!?, forcing next.\n");
		}
	}
	else {
		// egyik sem, mehet tova
		touch_row = row;
		touch_tic = GetTic();
	}

	return;
}



//
//
//
BOOL BLIB_IsModulePlaying( void ) {

	if( lpModule == 0 )
		return FALSE;

	/* check if the module is stopped */
	BOOL stopped = BASS_ChannelIsActive(lpModule);

	return BOOLEAN(stopped);
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





#define amptarget 30000 // target level
#define ampquiet 800 // quiet level
#define amprate 0.02f // amp adjustment rate

//
// FIXME:
// Dinamikus normalizálicó
//
static void CALLBACK autoampDSP( HDSP handle, DWORD channel, void *buffer, DWORD length, void *user ) {

	wave_t *wave = (wave_t *)user;
	FLOAT *data = (FLOAT *)buffer;
	DWORD c;

	if( length == 0 || buffer == NULL )
		return;

	for( c=0; c<length/4; c++ ) {

		FLOAT s = data[c] * wave->gain; // amplify sample
		FLOAT sa = fabs(s);

		if( fabs(data[c]) < ampquiet )
			wave->quiet++; // sample is below quiet level
		else
			wave->quiet = 0;

		if( sa < amptarget ) { // amplified level is below target

			if( sa > wave->high )
				wave->high = sa;

			wave->count++;

			if( wave->count == wave->delay ) { // been below target for a while

				if( wave->quiet>wave->delay )
					// it's quiet, go back towards normal level
					wave->gain += 10 * amprate * (1-wave->gain);
				else
					wave->gain += amprate * amptarget / wave->high; // increase amp

				wave->high = wave->count = 0; // reset counts
			}
		}
		else { // amplified level is above target
			if( s < -32768 ) s = -32768;
			else
			if( s > 32767 ) s = 32767;

			wave->gain -= 2 * amprate * sa / amptarget; // decrease amp
			wave->high = wave->count = 0;
		}

		data[c] = s; // replace original sample with amplified version
	}

	return;
}





//
//
//
static void CALLBACK AmplifyDSP( HDSP handle, DWORD channel, void *void_buffer, DWORD length, void *user ) {

	wave_t *wave = (wave_t *)user;
	FLOAT *buffer = (FLOAT *)void_buffer; // the MIDI channel is floating-point

	if( wave->gain == 0.0f || length == 0 || buffer == NULL )
		return;

	for( DWORD a=0; a<length/4; a++ )
		buffer[a] *= wave->gain; // pow( 10.0, wave->dBgain/20.0 );

	return;
}



//
// getAmp for Wave
//
static void WAV_getGain( wave_t *wave ) {

	if( wave == NULL || wave->stream_buf == NULL || wave->size == 0 )
		return;

	wave->gain = 0;

	BOOL tooLong = FALSE;

	DWORD peak=0;
	HSTREAM channel = BASS_StreamCreateFile(TRUE,wave->stream_buf,0,wave->size,BASS_STREAM_DECODE);

	if( !channel ) {
		xprintf("WAV_getGain: Error loading (%d).\n", BASS_ErrorGetCode() );
		return;
	}

	int start_tic = GetTic();

	while( BASS_ChannelIsActive(channel) ) {
		// not reached end, keep scanning...

		DWORD level = BASS_ChannelGetLevel(channel);

		if( peak < LOWORD(level) ) peak=LOWORD(level);
		if( peak < HIWORD(level) ) peak=HIWORD(level);

		// túl hangos
		/* if( peak >= 32767 )
			break;
		*/

		// amit 3 másodperc alatt fel tud dolgozni
		if( start_tic + (TICKBASE*normalize_sec) < GetTic() ) {
			tooLong = TRUE;
			break;
		}

	}

	/* if( peak > 32767 )
		peak = 32767;
	*/

	DWORD q = (DWORD)BASS_ChannelBytes2Seconds(channel,BASS_ChannelGetPosition(channel,BASS_POS_BYTE));

	BASS_StreamFree(channel);

	wave->gain = (FLOAT)WAVAMPVAL/(FLOAT)peak;

	xprintf("\nWAV_getGain: gain = %.2f, scanned %u:%02u min%s\n", wave->gain, q/60,q%60,tooLong?", TLDR.":"." );

	return;
}


//
//
//
int BLIB_LoadWave( const char *inFileName, int id ) {

	wave_t *wave = NULL;
	char *loaded_file = NULL;
	int size;

	if( winNoSound() )
		return id;

	if( inFileName == NULL || BLIB_Init() == FALSE )
		return -1;

	xprintf("\rBASS_LoadWave: ");

	if( GetWave( id ) )
		xprintf("Warning: there is already %d handler. ", id );

	ALLOCMEM( wave, sizeof(wave_t) );

	memset( wave, 0L, sizeof(wave_t) );

	wave->handler  = id;
	strcpy( wave->szFileName, inFileName );
	strlwr( wave->szFileName );

	wave->samp = 0;
	wave->stream = 0;
	wave->channel = -1;
	wave->stream_buf = NULL;

	wave->libsnd_chn = -1;
	wave->libsnd_bits = -1;
	wave->libsnd_freq = -1;

	if( stristr(wave->szFileName, ".mp3") || stristr(wave->szFileName, ".ogg") ) {

		xprintf("stream ");

		if( (wave->size = LoadFile( inFileName, PTR(wave->stream_buf))) == 0 ) {
			xprintf("\"%s\" can't LoadFile.", inFileName );
			return -1;
		}

		if( (wave->stream = BASS_StreamCreateFile( TRUE, wave->stream_buf, 0,wave->size, BASS_SAMPLE_FLOAT)) != 0 ) {

			if( AddToList( &WaveFirst, wave ) ) {

				WAV_getGain( wave );

				wave->dwFreq = 22050;
				wave->dwPan = MIDPAN;
				wave->dwVol = MAXVOL;
				wave->fLooped = FALSE;

				// TODO: valós info kellene
				wave->libsnd_chn = 2;
				wave->libsnd_bits = 16;
				wave->libsnd_freq = 22050;

				xprintf("\"%s\" loaded (%d).",inFileName,wave->handler);
			}
			else
				xprintf("addtolist failed.");
		}
		else {
			FREEMEM(wave->stream_buf);
			FREEMEM( wave );

			xprintf("\"%s\" NOT loaded. (%d)",inFileName,BASS_ErrorGetCode());

			return -1;
		}
	}
	else {
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

		if( (wave->samp = BASS_SampleLoad(TRUE,loaded_file,0,size,5,0)) != 0 ) {

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

			xprintf("\"%s\" NOT loaded. (%d)",inFileName,BASS_ErrorGetCode());

			return -1;
		}

		FREEMEM( loaded_file );
	}


	return wave->handler;
}



//
//
//
void BLIB_PlayWave( int handler ) {

	wave_t *pWave = GetWave( handler );

	if( (pWave = GetWave( handler ) ) == NULL ) {
		xprintf("BLIB_PlayWave: can't find %d handler.\n",handler);
		return;
	}

	if( pWave->samp ) {

		// play the sample (at default rate, volume=50%, random pan position)
		pWave->channel = BASS_SampleGetChannel( pWave->samp, 0 );

		if( pWave->channel ) {

			// 0 - 1
			BASS_ChannelSetAttribute(pWave->channel,BASS_ATTRIB_VOL, (FLOAT)pWave->dwVol / (FLOAT)MAXVOL );
			BASS_ChannelSetAttribute(pWave->channel,BASS_ATTRIB_PAN, (FLOAT)pWave->dwPan*2 / (FLOAT)MAXPAN - 1);

			// info.pan = ftoi( ((FLOAT)dwPan * (FLOAT)BASSMAXPAN / (FLOAT)MAXPAN) - 100.0f );
			// info.volume = ftoi( (FLOAT)dwVol * (FLOAT)BASSMAXVOL / (FLOAT)MAXVOL );

			if(!BASS_ChannelPlay(pWave->channel,FALSE))
				xprintf("BLIB_PlayWave: Can't play sample (error = %d).\n", BASS_ErrorGetCode());

			// ezt nem memóból jácca le
			// lastPlayedChannel = pWave->channel;
		}
		else
			xprintf("BLIB_PlayWave: BASS_SampleGetChannel failed. (error = %d)\n", BASS_ErrorGetCode() );
	}

	if( pWave->stream ) {

		// 0 - 1
		BASS_ChannelSetAttribute(pWave->stream,BASS_ATTRIB_VOL, (FLOAT)pWave->dwVol / (FLOAT)MAXVOL );
		BASS_ChannelSetAttribute(pWave->stream,BASS_ATTRIB_PAN, (FLOAT)pWave->dwPan*2 / (FLOAT)MAXPAN - 1);

		// play the stream (continue from current position)
		if( !BASS_ChannelPlay(pWave->stream,FALSE) )
			xprintf("BLIB_PlayWave: Can't play stream\n");
		else
			lastPlayedChannel = pWave->stream;

		if( 1 )
			// pre-scan
			pWave->ampdsp = BASS_ChannelSetDSP( pWave->stream, AmplifyDSP, pWave, 0); // set level checking DSP on it
		else {
			// dynamic
			pWave->gain = 1;
			pWave->delay = BASS_ChannelSeconds2Bytes(pWave->stream,1)/4;
			pWave->count = pWave->high = pWave->quiet = 0;
			pWave->ampdsp = BASS_ChannelSetDSP( pWave->stream, (DSPPROC*)&autoampDSP, pWave, 0 );
		}

	}

	return;
}




//
//
//
void BLIB_FreeWave( int handler ) {

	wave_t *pWave = NULL;
	wave_t *pPrev = NULL;

	if( (pWave = GetWave( handler ) ) != NULL ) {

		pPrev = &WaveFirst;

		while( pPrev != NULL ) {

			if( pPrev->pNext == pWave )
				break;

			pPrev = pPrev->pNext;
		}

		xprintf("BLIB_FreeWave: \"%s\".\n",pWave->szFileName);

		if( pWave->samp ) {
			BASS_SampleStop( pWave->samp );
			BASS_SampleFree( pWave->samp );
		}

		xprintf("stream:\n");

		if( pWave->stream ) {
			if( pWave->ampdsp ) BASS_ChannelRemoveDSP( pWave->stream, pWave->ampdsp );
			BASS_ChannelStop( pWave->stream );
			// BASS_StreamFree( pWave->stream );
		}

		xprintf("stream_buf:\n");

		SAFE_FREEMEM( pWave->stream_buf );

		if( pPrev ) pPrev->pNext = pWave->pNext;

		xprintf("pWave:\n");

		FREEMEM( pWave );
	}

	return;
}






//
//
//
void BLIB_FreeAllWave( void ) {

	/* release the waveform file */

	wave_t *pWave, *pWaveNext;

	pWave = WaveFirst.pNext;

	while( pWave != NULL ) {

		xprintf("FreeAllList: \"%s\".\n",pWave->szFileName);

		if( pWave->samp ) {
			BASS_SampleStop( pWave->samp );
			BASS_SampleFree( pWave->samp );
		}

		xprintf("stream:\n");

		if( pWave->stream ) {
			BASS_ChannelStop( pWave->stream );
			// BASS_StreamFree( pWave->stream );
		}

		xprintf("stream_buf:\n");

		if( pWave->stream_buf )
			FREEMEM( pWave->stream_buf );
		pWave->stream_buf = NULL;

		pWaveNext = pWave->pNext;

		xprintf("pWave:\n");

		FREEMEM( pWave );

		pWave = pWaveNext;
	}

	WaveFirst.pNext = NULL;

	return;
}




//
//
//
void BLIB_SetWaveParam( int handler, int dwFreq, int dwVol, int dwPan, int fLooped, int percent ) {

	wave_t *pWave;
	int channel = -1;

	if( (pWave = GetWave( handler ) ) != NULL ) {

		if( dwFreq != -1 )
			pWave->dwFreq = dwFreq;

		if( dwPan != -1 ) {
			pWave->dwPan = dwPan;
			CLAMPMINMAX( pWave->dwPan, MINPAN, MAXPAN );
		}

		if( dwVol != -1 ) {
			pWave->dwVol = dwVol;
			CLAMPMINMAX( pWave->dwVol, MINVOL, MAXVOL );
		}

		if( fLooped != -1 )
			pWave->fLooped = fLooped;

		if( pWave->stream )
			channel = pWave->stream;

		if( pWave->samp )
			channel = pWave->channel;

		// menet közbeni állítás, amit PlayWave-vel már elindítottak
		if( (channel != -1) && (BASS_ChannelIsActive( channel ) != BASS_ACTIVE_STOPPED) ) {

			if( dwVol != -1 ) BASS_ChannelSetAttribute(channel,BASS_ATTRIB_VOL, (FLOAT)pWave->dwVol / (FLOAT)MAXVOL );
			if( dwPan != -1 ) BASS_ChannelSetAttribute(channel,BASS_ATTRIB_PAN, (FLOAT)pWave->dwPan*2 / (FLOAT)MAXPAN - 1);
			if( fLooped != -1 ) BASS_ChannelFlags( channel, (pWave->fLooped == TRUE) ? BASS_SAMPLE_LOOP : 0, BASS_SAMPLE_LOOP);

			// pozicíó
			if( percent != -1 ) {

				QWORD pos,maxPos;

				CLAMPMINMAX( percent, 0, 100 );

				maxPos = BASS_ChannelGetLength( channel, BASS_POS_BYTE );

				pos = maxPos*percent / 100;

				// little hackerance
				if( percent == 100 )
					pos = maxPos - 1;

				CLAMPMINMAX( pos, 0, maxPos );

				BASS_ChannelSetPosition( channel, pos, BASS_POS_BYTE );
			}
		}
	}
	else
		xprintf("BASS_SetWaveParam: can't find %d handler.\n",handler);

	return;
}




//
// printf(" %u:%02u\n",time/60,time%60);
// printf("pos %09I64u",pos);
//
BOOL BLIB_GetWaveParam( int handler, int *percent, int *time, int *maxTime, int *bytePos, int *byteMaxPos, char **name, int *freq, int *bits, int *chn ) {

	wave_t *pWave;
	QWORD pos,maxPos;
	int channel;

	if( (pWave = GetWave( handler ) ) == NULL ) {
		xprintf("BLIB_GetWaveParam: can't find %d handler.\n",handler);
		return FALSE;
	}

	if( pWave->samp ) {
		if( pWave->channel != (-1) )
			channel = pWave->channel;
		else
			return FALSE;
	}
	else
	if( pWave->stream )
		channel = pWave->stream;
	else
		return FALSE;

	pos = BASS_ChannelGetPosition( channel, BASS_POS_BYTE );

	if( pos != -1 ) {
		if( time ) *time = (int)BASS_ChannelBytes2Seconds( channel, pos );
	}
	else {
		if( time ) *time = 0;
		pos = 0;
	}

	maxPos = BASS_ChannelGetLength( channel, BASS_POS_BYTE );

	if( maxPos != -1 ) {
		if( maxTime ) *maxTime = (int)BASS_ChannelBytes2Seconds( channel, maxPos );
	}
	else {
		if( maxTime ) *maxTime = 0;
		maxPos = 0;
	}

	if( bytePos )
		*bytePos = (int)pos;

	if( byteMaxPos )
		*byteMaxPos = (int)maxPos;

	if( percent ) {

		if( maxPos != 0 )
			*percent = (int)(pos*100 / maxPos);

		// streamnél a pos 100%-nál marad, pWave->stream &&
		if( pos >= maxPos )
			*percent = 0;
	}

	if( name != NULL ) {

		TAG_ID3 *id3 = (TAG_ID3*)BASS_ChannelGetTags( channel, BASS_TAG_ID3 );

		if( id3 )
			*name = id3->title;
		else
			*name = pWave->szFileName;
	}

	if( pWave->samp ) {

		BASS_SAMPLE info;

		BASS_SampleGetInfo(pWave->samp, &info);

		if( freq ) *freq = pWave->libsnd_freq != -1 ? pWave->libsnd_freq : info.freq;

		if( bits ) {
			if( pWave->libsnd_bits != -1 )
				*bits = pWave->libsnd_bits;
			else
			if( info.flags & BASS_SAMPLE_FLOAT )
				*bits = 32;
			else
			if( info.flags & BASS_SAMPLE_8BITS )
				*bits = 8;
			else
				*bits = 16;
		}

		if( chn ) *chn = pWave->libsnd_chn != -1 ? pWave->libsnd_chn : ((info.flags & BASS_SAMPLE_MONO)? 1: 2);
	}

	if( pWave->stream ) {
		if( freq ) *freq = pWave->libsnd_freq;
		if( bits ) *bits = pWave->libsnd_bits;
		if( chn ) *chn = pWave->libsnd_chn;
	}

	return TRUE;
}




//
//
//
int BLIB_IsLoaded( const char *name ) {

	wave_t *pWaveFirst;

	pWaveFirst = WaveFirst.pNext;

	while( pWaveFirst != NULL ) {

		if( name && !stricmp( pWaveFirst->szFileName, name ) )
			return pWaveFirst->handler;

		pWaveFirst = pWaveFirst->pNext;
	}

	return 0;
}




//
// BASS_ACTIVE_STOPPED The channel is not active, or handle is not a valid channel.
// BASS_ACTIVE_PLAYING The channel is playing (or recording).
// BASS_ACTIVE_PAUSED The channel is paused.
// BASS_ACTIVE_STALLED Playback of the stream has been stalled due to a lack of sample data. The playback will automatically resume once there is sufficient data to do so.
//
int BLIB_IsPlaying( int handler ) {

	wave_t *pWave;

	if( (pWave = GetWave( handler ) ) == NULL ) {
		// xprintf("BASS_IsPlaying: can't find %d handler.\n",handler);
		return FALSE;
	}

	if( pWave->samp ) {
		if( pWave->channel == (-1) )
			return FALSE;

		return BASS_ChannelIsActive(pWave->channel) == BASS_ACTIVE_PLAYING;
	}

	if( pWave->stream ) {

		BOOL isPlaying = BASS_ChannelIsActive(pWave->stream) == BASS_ACTIVE_PLAYING;

		// FIXME: ha végigjáccotta egyszer akkor kell ezt csinálni
		if( isPlaying == FALSE && pWave->gain != 1.0f ) {
				pWave->gain = 1.0f;
				xprintf("BASS_IsPlaying: gain reset here.\n");
		}

		return isPlaying;
	}

	return FALSE;
}



//
//
//
void BLIB_StopWave( int handler ) {

	wave_t *pWave;

	if( (pWave = GetWave( handler ) ) == NULL )
		return;

	if( pWave->samp )
		BASS_SampleStop( pWave->samp );

	if( pWave->stream )
		BASS_ChannelStop( pWave->stream );

	return;
}


#define PLAYSOUND_MIDI	(10000)
#define PLAYSOUND_SID	(11000)
#define PLAYSOUND_RADIO	(12000)
#define PLAYSOUND_YM	(13000)
#define PLAYSOUND_MOD	(14000)
#define PLAYSOUND_AD	(15000)



//
// ez lesz a buta PLAYSOUND( "valami.wav" );
//
int BLIB_PlaySound( const char *filename, int vol, int pan, BOOL loop ) {

	wave_t *pWave;
	int id;
	char str[XMAX_PATH],*p;

	if( filename == NULL || winNoSound() || BLIB_Init() == FALSE )
		return FALSE;

	strcpy( str, filename );
	strlwr( str );

	// ha nincs extension legyen Wave
	if( (p = GetExtension( str )) != NULL ) {

		--p;

		// mid, midi
		const char *mid_pattern = "*.mid;*.midi";

		if( strstr( mid_pattern, p ) ) {
			BLIB_StopMidi();
			int rez = BLIB_PlayMidi( filename );
			BLIB_SetMidiParam( vol==-1?MIDVOL:vol, -1, -1, loop==-1?TRUE:loop );
			return rez ? 0 : PLAYSOUND_MIDI;	// 0 a jó
		}

		// sid
		const char *sid_pattern = "*.sid;*.mus";

		if( strstr( sid_pattern, p ) ) {
			SID_StopSid();
			int rez = SID_PlaySid(filename);
			SID_SetParam( -1, vol==-1?MIDVOL:vol );
			return rez ? 0 : PLAYSOUND_SID;	// 0 a jó
		}

		// pls, m3u
		const char *radio_pattern = "*.pls;*.m3u";

		if( strstr( radio_pattern, p ) ) {
			NR_Stop();
			int rez = NR_Play(filename);
			NR_SetParam( vol==-1?MIDVOL:vol );
			return rez ? 0 : PLAYSOUND_RADIO;	// 0 a jó
		}

		// YM
		const char *ym_pattern = "*.ym;*.y5";

		if( strstr( ym_pattern, p ) ) {
			YM_Stop();
			int rez = YM_Play(filename);
			YM_SetParam( vol==-1?MIDVOL:vol );
			return rez ? 0 : PLAYSOUND_YM;	// 0 a jó
		}


		// s3m, xm, mod
		const char *mod_pattern = "*.s3m;*.stm;*.xm;*.mod;*.it;*.mtm;*.mo3";

		if( strstr( mod_pattern, p ) ) {
			BLIB_StopMod();
			int rez = BLIB_PlayMod(filename);
			BLIB_SetModuleParam( vol==-1?MIDVOL:vol, -1, -1, loop==-1?TRUE:loop );
			return rez ? 0 : PLAYSOUND_MOD;	// 0 a jó
		}

		// Adplug (az s3m miatt a BASS mögé)
		const char *adplug_pattern = "*.s3m;*.s3ma;*.a2m;*.adl;*.amd;*.bam;*.cff;*.cmf;*.d00;*.dfm;*.dmo;*.dro;*.dtm;*.hsc;*.hsp;*.imf;*.ksm;*.laa;*.lds;*.m;*.mad;*.mkj;*.msc;*.mtk;*.rad;*.raw;*.rix;*.rol;*.sa2;*.sat;*.sci;*.sng;*.xad;*.xms;*.xsm";

		if( strstr( adplug_pattern, p ) ) {
			AD_Stop();
			int rez = AD_Play(filename);
			AD_SetParam( -1, vol==-1?MIDVOL:vol );
			return rez ? 0 : PLAYSOUND_AD;	// 0 a jó
		}
	}


	// wav, etc.
	if( (pWave = GetWave( -1, filename ) ) == NULL ) {

		// keres egy üres handlert
		id = 1213;

		wave_t *pWaveFirst = WaveFirst.pNext;

		while( pWaveFirst != NULL ) {
			if( pWaveFirst->handler == id ) {
				// már volt ilyen
				++id;
				pWaveFirst = WaveFirst.pNext;
			}
			pWaveFirst = pWaveFirst->pNext;
		}


		if( (id = BLIB_LoadWave( filename, id )) == -1 )
			return 0;


		if( (pWave = GetWave( id ) ) == NULL )
			return 0;
	}

	// wave volt, vol == 0 tehát csak cachelt
	if( vol == 0 )
		return id;

	id = pWave->handler;

	BLIB_SetWaveParam( id, -1, vol==-1?MIDVOL:vol, pan==-1?MIDPAN:pan, loop==-1?FALSE:loop );

	BLIB_PlayWave( id );

	BLIB_SetWaveParam( id, -1, vol==-1?MIDVOL:vol, pan==-1?MIDPAN:pan, loop==-1?FALSE:loop );

	return id;
}


//
//
//
void BLIB_StopSound( int id ) {

	if( id >= PLAYSOUND_AD ) {
		AD_Stop();
	}
	else
	if( id >= PLAYSOUND_MOD ) {
		BLIB_StopMod();
	}
	else
	if( id >= PLAYSOUND_YM ) {
		YM_Stop();
	}
	else
	if( id >= PLAYSOUND_RADIO ) {
		NR_Stop();
	}
	else
	if( id >= PLAYSOUND_SID ) {
		SID_StopSid();
	}
	else
	if( id >= PLAYSOUND_MIDI ) {
		BLIB_StopMidi();
	}
	else
		BLIB_StopWave(id);

	return;
}




//
//  float fft[1024];
//
void *BLIB_ChanGetData( void ) {

	if( lastPlayedChannel == 0 || BASS_ChannelIsActive(lastPlayedChannel) != BASS_ACTIVE_PLAYING )
		return NULL;

	if( fft == NULL ) ALLOCMEM( fft, 2048 * sizeof(FLOAT) );

	if( fft == NULL )
		return fft;

	// BASS_ChannelLock(lastPlayedChannel, TRUE);

	DWORD size = BASS_ChannelGetData( lastPlayedChannel, fft, BASS_DATA_FFT2048 ); // get the FFT data

	if( size == -1 )
		return NULL;

	if( size < 1024 )
		memset( &fft[ size ], 0L, (1024 - size) * sizeof(FLOAT) );

	// BASS_ChannelLock(lastPlayedChannel, FALSE);

	return fft;
}



#define BANDS 28


//
// spectrum
//
int BLIB_DrawSpectrum( int specmode,int origx, int origy, int origw, int origh ) {

	int x,y,y1;
	FLOAT *fft = NULL;

	if( specmode < 0 ) specmode = 4;
	if( specmode > 4 ) specmode = 0;

	FlushScene();

	if( origx == -1 ||
            origy == -1 ||
	    origw == -1 ||
	    origh == -1 ) {
		origx = origy = 0;
		origw = SCREENW;
		origh = SCREENH;
	}

	// "normal" FFT
	if( specmode == 0 ) {

		if( (fft = (FLOAT *)BLIB_ChanGetData()) == NULL ) // get the FFT data
			return specmode;

		for( x=0; x<origw/2; x++) {

			// scale it (sqrt to make low values more visible)
			y = sqrt( fft[ (x+1) * 1024 / origw ] ) * 5 * origh-4;
			// y = fft[x+1]*10*SCREENH; // scale it (linearly)

			CLAMPMAX( y, origh );

			// interpolate from previous to make the display smoother
			if( x && (y1=(y+y1)/2) )
				// while (--y1>=0) specbuf[y1*SPECWIDTH+x*2-1]=y1+1;
				DrawTexLine( origx+x*2-1, origy+origh, origx+x*2, (origy+origh)-y1, 5, 2 );
			y1=y;
			// while (--y>=0) specbuf[y*SPECWIDTH+x*2]=y+1; // draw level
			DrawTexLine( origx+x*2, origy+origh, origx+x*2+1, (origy+origh)-y, 5, 2 );
		}
	}


	// logarithmic, acumulate & average bins
	if( specmode == 1 ) {

		int b0=0;

		if( (fft = (FLOAT *)BLIB_ChanGetData()) == NULL ) // get the FFT data
			return specmode;

		for( x=0; x<BANDS; x++ ) {

			FLOAT peak = 0.0f;
			int b1 = pow( 2, x*10.0/(BANDS-1) );

			if( b1 > 1023 )
				b1 = 1023;

			if( b1 <= b0 )
				b1 = b0 + 1; // make sure it uses at least 1 FFT bin

			for( ; b0 < b1; b0++ )
				if( peak < fft[1+b0] )
					peak = fft[1+b0];

			y = sqrt(peak) * 5 * origh-4; // scale it (sqrt to make low values more visible)

			CLAMPMAX( y, origh );

			/*
			// FIXME: BANDS-1 hackeransz
			DrawTexRect( origx + x*(origw/(BANDS-1)), origy+(origh-y),
				     origx + x*(origw/(BANDS-1)) + (origw/(BANDS-1)-2), origy+origh , 5 );
			*/
		}
	}

	// waveform
	if( specmode == 2 ) {

		int c;
		FLOAT *buf;
		BASS_CHANNELINFO ci;
		rgb_t red = { 255,0,0, 255 };
		rgb_t green = { 0,255,0, 255 };

		if( lastPlayedChannel == 0 )
			return specmode;

		BASS_ChannelGetInfo( lastPlayedChannel, &ci ); // get number of channels

		if( ci.chans < 1 )
			return specmode;

		ALLOCMEM( buf, (ci.chans * origw * sizeof(FLOAT)) ); // allocate buffer for data

		// get the sample data (floating-point to avoid 8 & 16 bit processing)
		BASS_ChannelGetData( lastPlayedChannel, buf, (ci.chans * origw * sizeof(FLOAT)) | BASS_DATA_FLOAT );

		for( c=0; c<(int)ci.chans; c++ ) {

			for( x=0; x<origw; x++ ) {

				int v = (1-buf[x*ci.chans+c]) * origh/2; // invert and scale to fit display

				CLAMPMINMAX( v, 0, origh );

				if( !x )
					y = v;

				/*
				if( c&1 ) Line( x,y, x,v, red );
				else	  Line( x,y, x,v, green );
				*/

				// draw line from previous sample...
				do {
					if( y < v )
						y++;
					else
					if( y > v )
						y--;

					// left=green, right=red (could add more colours to palette for more chans)
					if( c&1 )
						PutPixel( origx+x, origy+y, red );
					else
						PutPixel( origx+x, origy+y, green );

				} while( y!=v );
			}
		}

		FREEMEM( buf );
	}

	// "3D"
	if( specmode == 3 ) {

		static UCHAR *specbuf = NULL;
		static rgb_t *pal = NULL;
		static int specpos = 0;
		static int allocw = 0;
		static int alloch = 0;

		const int SPECHEIGHT = origh / 3;
		const int SPECWIDTH = origw / 3;

		if( (fft = (FLOAT *)BLIB_ChanGetData()) == NULL ) // get the FFT data
			return specmode;

		if( (allocw != SPECWIDTH || alloch != SPECHEIGHT) && specbuf ) {
			FREEMEM( specbuf );
			specbuf = NULL;
		}

		if( specbuf == NULL ) {
			ALLOCMEM( specbuf, SPECWIDTH*SPECHEIGHT );
			memset( specbuf, 0L, SPECWIDTH*SPECHEIGHT );
			allocw = SPECWIDTH;
			alloch = SPECHEIGHT;
		}

		// setup palette
		if( pal == NULL ) {

			ALLOCMEM( pal, 256*sizeof(rgb_t) );
			memset( pal, 0L, 256*sizeof(rgb_t) );

			int a;

			for( a=0; a<256; a++ )
				pal[a].a = 255;

			pal[0].r = 0;
			pal[0].g = 0;
			pal[0].b = 0;

			pal[255].r = 255;
			pal[255].g = 255;
			pal[255].b = 255;

			for( a=1; a<128; a++ ) {
				pal[a].g = 256-2*a;
				pal[a].r = 2*a;
			}

			for( a=0; a<32; a++ ) {
				pal[128+a].b = 8*a;
				pal[128+32+a].b = 255;
				pal[128+32+a].r = 8*a;
				pal[128+64+a].r = 255;
				pal[128+64+a].b = 8*(31-a);
				pal[128+64+a].g = 8*a;
				pal[128+96+a].r = 255;
				pal[128+96+a].g = 255;
				pal[128+96+a].b = 8*a;
			}
		}

		for( x=0; x<SPECHEIGHT; x++ ) {
			y = sqrt( fft[x+1] ) * 3 * 127; // scale it (sqrt to make low values more visible)
			if( y > 127 )
				y = 127; // cap it
			specbuf[ x*SPECWIDTH + specpos ] = 128+y; // plot it
		}

		// move marker onto next position
		specpos = (specpos+1)%SPECWIDTH;
		for( x=0; x<SPECHEIGHT; x++ )
			specbuf[ x*SPECWIDTH + specpos] = 255;

		int px = (origw - SPECWIDTH) / 2;
		int py = (origh - SPECHEIGHT) / 2;

		for( y=0; y<SPECHEIGHT; y++ )
		for( x=0; x<SPECWIDTH; x++ )
			PutPixel( origx + px+x, origy + py+SPECHEIGHT-y, pal[specbuf[y*SPECWIDTH+x]] );
	}

	// VUmeter
	if( specmode == 4 ) {

		if( lastPlayedChannel == 0 || BASS_ChannelIsActive(lastPlayedChannel) == BASS_ACTIVE_STOPPED )
			return specmode;

		DWORD level = BASS_ChannelGetLevel(lastPlayedChannel);

		int left = LOWORD(level);
		int right = HIWORD(level);

		if( left > 32767 ) left = 32767;
		if( right > 32767 ) right = 32767;

		int w1 = sqrt((FLOAT)left/32767.0f) * 3 * origw-4; // scale it (sqrt to make low values more visible)

		CLAMPMAX( w1, origw );

		if( w1 > 0 )
			DrawTexRect( origx, origy+origh/5,
				     origx + w1, origy+origh*2/5, 3 );

		int w2 = sqrt((FLOAT)right/32767.0f) * 3 * origw-4; // scale it (sqrt to make low values more visible)

		CLAMPMAX( w2, origw );

		if( w2 > 0 )
			DrawTexRect( origx, origy+origh*3/5,
				     origx + w2, origy+origh*4/5, 3 );
	}


	if( specmode == 5 ) {
		/***
		ZGameEditor / Projects\ModPlay\ModPlay.zgeproj

		int level=BASS_ChannelGetLevel(MusicHandle);

		int left=level >> 16;
		int right=level & 0xffff;

		s=intToStr( left );
		Rtext1.Text=s;

		Rbeams1.Length = (left/32768.0) * 20;]]>
		<RenderBeams Name="Rbeams1" Count="10" Length="0.2856" Width="0.1"/>

TRenderBeams = class(TRenderCommand)
  public
    Count : integer;
    Length : zptFloat;
    Width : zptFloat;
    Speed : zptFloat;



procedure TRenderBeams.Execute;
var
  I : integer;
  B : TBeam;
  Angle,X,Y,C,S : single;
begin
  {$ifndef minimal}
  AssertRenderMode;
  {$endif}

  glBegin(GL_TRIANGLES);

  for I := 0 to Beams.Count-1 do
  begin
    B := TBeam(Beams[I]);
    glTexCoord2f(0.52,0.52);
    glVertex3f(0,0,0);

    Angle := B.Angle-B.Width/2;
    C := cos(Angle);
    S := sin(Angle);
    X := C * Length;
    Y := S * Length;
    glTexCoord2f( 0.5 + C/2 , 0.5 + S/2);
    glVertex3f(X,Y,0);

    Angle := B.Angle+B.Width/2;
    C := cos(Angle);
    S := sin(Angle);
    X := C * Length;
    Y := S * Length;
    glTexCoord2f( 0.5 + C/2 , 0.5 + S/2);
    glVertex3f(X,Y,0);
  end;

  glEnd;
end;

procedure TRenderBeams.Update;
var
  B : TBeam;
  I : integer;
  WRange : single;
begin
  {$ifndef minimal}
  if IsChanged then
  begin
    //Nollställ ifall ändrad i designer
    Beams.Clear;
    IsChanged := False;
  end;
  {$endif}
  //Emit new beams
  while Beams.Count<Count do
  begin
    B := TBeam.Create;
    WRange := Width*0.1; //Use +/- 10 percent width diff
    B.Width := ZMath.Random(Width,WRange);
    B.Angle := System.Random * (2*PI);
    B.AngleInc := Random(Speed,Speed*0.5);
    if System.Random<0.5 then
      B.AngleInc := B.AngleInc * -1;
    Beams.Add(B);
  end;
  //Update beams
  for I := 0 to Beams.Count-1 do
  begin
    B := TBeam(Beams[I]);
    B.Angle := B.Angle + B.AngleInc * ZApp.DeltaTime;
  end;
end;

		***/
	}

	return specmode;
}




//
// DWORD start,end;
// GetSilenceLength(file,500,&start,&end);
//
void BLIB_GetSilenceLength( const char *file, int threshold, ULONG *start, ULONG *end ) {

	short buf[50000];
	ULONG count=0;
	QWORD pos;

	if( BLIB_Init() == FALSE )
		return;

	// create decoding channel
	HSTREAM chan=BASS_StreamCreateFile(FALSE,file,0,0,BASS_STREAM_DECODE);

	if( !chan )
		return;

	while( BASS_ChannelIsActive(chan) ) {

		int a,b=BASS_ChannelGetData(chan,buf,20000); // decode some data
		b/=2; // bytes -> samples

		for( a=0; a<b && abs(buf[a])<=threshold; a++ ); // count silent samples

		count+=a*2; // add number of silent bytes

		if( a<b ) { // sound has begun!
			// move back to a quieter sample (to avoid "click")
			for( ;a && abs(buf[a])>threshold/4; a--, count-=2 );
			break;
		}
	}

	*start = count;

	pos = BASS_ChannelGetLength( chan, BASS_POS_BYTE );

	while( pos > count ) {

		int a,b;

		pos = pos<100000 ? 0 : pos-100000; // step back a bit

		BASS_ChannelSetPosition( chan, pos, BASS_POS_BYTE );

		b = BASS_ChannelGetData( chan, buf, 100000 ); // decode some data
		b/=2; // bytes -> samples

		for( a=b; a>0 && abs(buf[a-1])<=threshold/2 ; a-- ); // count silent samples

		if( a > 0 ) { // sound has begun!
			count = (ULONG)pos+a*2; // silence begins here
			break;
		}
	}

	*end = count;

	BASS_StreamFree(chan);

	return;
}






//
// BASS_ChannelSetSync(chan, BASS_SYNC_POS,BASS_ChannelSeconds2Bytes(chan,180),MySyncProc, 0);
// set a POS sync there
//
void CALLBACK MySyncProc(HSYNC handle, DWORD channel, DWORD data, void *user) {

	//do something here, ex free the channel

	return;
}




/********************************
 *
 * MIDI stuff
 *
 ********************************/


#define BASSMIDIDLLNAME "BASSMIDI.DLL"

static HINSTANCE hBassMidi = NULL;

static HSTREAM midi_chan=0;	// channel handle
static HSOUNDFONT font=0;	// soundfont

static int miditempo;		// MIDI file tempo
static float temposcale=1;	// tempo adjustment
static BOOL midi_loop = TRUE;

static char lyrics[1000]; // lyrics buffer

static HDSP midi_dsp = 0;
static FLOAT midi_gain = 0;


//
// load BASS and the required functions
//
static int LoadBASSMidi( void ) {

	if( hBassMidi )
		return 0;

	if( BLIB_Init() == FALSE )
		return 5;

	char bassdllname[PATH_MAX];

	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, BASSMIDIDLLNAME );

	// load the temporary BASS.DLL library
	if( (hBassMidi = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadBASSMidi: can't load \"%s\" library.\n",bassdllname);
		return 3;
	}

	LOADBASSMIDIFUNCTION( BASS_MIDI_FontInit );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamSetFonts );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamGetFonts );
	LOADBASSMIDIFUNCTION( BASS_MIDI_FontFree );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamGetMark );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamGetEvent );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamEvent );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamCreateFile );
	LOADBASSMIDIFUNCTION( BASS_MIDI_StreamGetChannel );

	xprintf("LoadBASSMidi: BASSMIDI (%s) loaded.\n",bassdllname);

	return 0;
}




//
//
//
BOOL BLIB_LoadSoundFont( const char *filename ) {

	if( LoadBASSMidi() )
		return FALSE;

	// már van bennt egy
	if( filename == NULL && font )
		return TRUE;

	char file[XMAX_PATH];

	if( filename == NULL ) {

		char str[XMAX_PATH];

		XLIB_winReadProfileString( (char *)"xlib_soundfont", (char *)"ChoriumRevA.SF2", str, XMAX_PATH );

		sprintf( file, "%s%c%s", GetDataDir(), PATHDELIM, str );
	}
	else
		strcpy( file, filename );

	FILE *f=fopen(file,"rb");

	if( f == NULL ) {
		char path[XMAX_PATH];
		sprintf( path, "%s%c%s", GetDataDir(), PATHDELIM, file );
		strcpy( file, path );
		f=fopen(file,"rb");
		if( f == NULL ) {
			xprintf("BLIB_LoadSoundFont: can't find %s soundfont.\n", file );
			return FALSE;
		}
	}

	fclose(f);

	HSOUNDFONT newfont;
	if( (newfont=BASS_MIDI_FontInit(file,0)) ) {

		BASS_MIDI_FONT sf;
		sf.font = newfont;
		sf.preset = -1;		// use all presets
		sf.bank=0;		// use default bank(s)

		BASS_MIDI_StreamSetFonts(0,&sf,1);		// set default soundfont
		BASS_MIDI_StreamSetFonts(midi_chan,&sf,1);	// set for current stream too
		BASS_MIDI_FontFree(font);			// free old soundfont

		font = newfont;
	}
	else {
		xprintf("BLIB_LoadSoundFont: can't load %s soundfont.\n", file );
		return FALSE;
	}

	xprintf("BLIB_LoadSoundFont: soundfont (%s) loaded.\n", file );

	return TRUE;
}




//
//
//
void CALLBACK LyricSync(HSYNC handle, DWORD channel, DWORD data, void *user) {

	BASS_MIDI_MARK mark;
	const char *text;
	int lines;

	BASS_MIDI_StreamGetMark(channel,(DWORD)user,data,&mark); // get the lyric/text

	text=mark.text;

	if( text[0]=='@' )
		return; // skip info

	if( text[0]=='\\' ) // clear display
		strcpy(lyrics,text+1);
	else {
		if (text[0]=='/') { // new line
			strcat(lyrics,"\n");
			text++;
		}
		strcat(lyrics,text);
	}

	for( lines=1,text=lyrics; text=strchr(text,'\n');lines++,text++ ) ; // count lines

	if( lines>3 ) { // remove old lines so that new lines fit in display...
		int a;
		for( a=0,text=lyrics;a<lines-3;a++)
			text=strchr(text,'\n')+1;
		strcpy(lyrics,text);
	}

	return;
}




//
//
//
void CALLBACK EndSync(HSYNC handle, DWORD channel, DWORD data, void *user) {

	lyrics[0]=0; // clear lyrics

	return;
}



//
//
//
void SetTempo( BOOL reset ) {

	if( reset )
		miditempo=BASS_MIDI_StreamGetEvent(midi_chan,0,MIDI_EVENT_TEMPO); // get the file's tempo

	BASS_MIDI_StreamEvent(midi_chan,0,MIDI_EVENT_TEMPO,miditempo*temposcale); // set tempo

	return;
}



//
//
//
void CALLBACK TempoSync(HSYNC handle, DWORD channel, DWORD data, void *user) {

	SetTempo(TRUE); // override the tempo

	return;
}



// look for a marker (eg. loop points)
BOOL FindMarker( HSTREAM handle, const char *text, BASS_MIDI_MARK *mark ) {

	int a;

	for( a=0; BASS_MIDI_StreamGetMark(handle,BASS_MIDI_MARK_MARKER,a,mark); a++ ) {
		if( !stricmp(mark->text,text) )
			return TRUE; // found it
	}

	return FALSE;
}




//
//
//
void CALLBACK LoopSync( HSYNC handle, DWORD channel, DWORD data, void *user ) {

	BASS_MIDI_MARK mark;

	if( midi_loop == FALSE )
		return;

	// found a loop start point
	if( FindMarker(channel,"loopstart",&mark) )
		// rewind to it (and let old notes decay)
		BASS_ChannelSetPosition( channel, mark.pos, BASS_POS_BYTE|BASS_MIDI_DECAYSEEK );
	else
		// else rewind to the beginning instead
		BASS_ChannelSetPosition( channel, 0, BASS_POS_BYTE|BASS_MIDI_DECAYSEEK );

	return;
}



static float midi_levels[2]; // left/right peak levels

//
//
//
void CALLBACK LevelDSP( HDSP handle, DWORD channel, void *buffer, DWORD length, void *user ) {

	FLOAT *s = (FLOAT *)buffer; // the MIDI channel is floating-point

	if( length == 0 || buffer == NULL )
		return;

	midi_levels[0] = 0;
	midi_levels[1] = 0; // reset levels

	for( int a=0; a<length/sizeof(float); a+=2 ) {

		if( midi_levels[0] < fabs(s[a]) )
			midi_levels[0] = fabs(s[a]); // check left

		if( midi_levels[1] < fabs(s[a+1]) )
			midi_levels[1] = fabs(s[a+1]); // check right
	}

	return;
}



//
//
//
static void CALLBACK MidiDSP( HDSP handle, DWORD channel, void *void_buffer, DWORD length, void *user ) {

	FLOAT *buffer = (FLOAT *)void_buffer; // the MIDI channel is floating-point

	if( length == 0 || buffer == NULL )
		return;

	for( DWORD a=0; a<length/4; a++ )
		buffer[a] *= midi_gain; // pow( 10.0, wave->dBgain/20.0 );

	return;
}


//
//
//
static void MIDI_getGain( char *loaded_file, int size, BOOL log = FALSE ) {

	midi_gain = 0;

	BOOL tooLong = FALSE;

	DWORD peak=0;
	HSTREAM channel = BASS_MIDI_StreamCreateFile(TRUE,loaded_file,0,size,BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT,0);

	if( !channel ) {
		xprintf("MIDI_getGain: Error loading (%d).\n", BASS_ErrorGetCode() );
		return;
	}

	int start_tic = GetTic();

	while( BASS_ChannelIsActive(channel) ) {
		// not reached end, keep scanning...

		DWORD level = BASS_ChannelGetLevel(channel);

		if( peak < LOWORD(level) ) peak=LOWORD(level);
		if( peak < HIWORD(level) ) peak=HIWORD(level);

		// túl hangos
		/* if( peak >= 32767 )
			break;
		*/

		// amit 3 másodperc alatt fel tud dolgozni
		if( start_tic + (TICKBASE*normalize_sec) < GetTic() ) {
			tooLong = TRUE;
			break;
		}

	}

	/* if( peak > 32767 )
		peak = 32767;
	*/

	DWORD q = (DWORD)BASS_ChannelBytes2Seconds(channel,BASS_ChannelGetPosition(channel,BASS_POS_BYTE));

	BASS_StreamFree(channel);

	midi_gain = (FLOAT)MIDIAMPVAL/(FLOAT)peak;

	xprintf("MIDI_getGain: gain = %.2f, scanned %u:%02u min%s\n", midi_gain, q/60,q%60,tooLong?", TLDR.":"." );

	return;
}




//
// "MIDI files (mid/midi/rmi/kar)\0*.mid;*.midi;*.rmi;*.kar\0All files\0*.*\0\0";
//
BOOL BLIB_PlayMidi( const char *file ) {

	if( LoadBASSMidi() )
		return FALSE;

	BLIB_LoadSoundFont();

	BLIB_StopMidi();

	char *loaded_file;
	int size;

	if( (size = LoadFile( file, PTR(loaded_file) )) == 0 ) {
		xprintf("BLIB_PlayMidi: LoadFile \"%s\" error.\n", file );
		return FALSE;
	}

	int flag = /*BASS_STREAM_DECODE|BASS_MIDI_NOFX|*/ BASS_SAMPLE_LOOP | BASS_SAMPLE_FLOAT;
	if( !(midi_chan = BASS_MIDI_StreamCreateFile(TRUE,loaded_file,0,size,flag,0)) ) {
		FREEMEM( loaded_file );
		loaded_file = NULL;
		xprintf("BLIB_PlayMidi: can't play \"%s\" file.\n", file);
		return FALSE;
	}

	// set looping syncs
	BASS_MIDI_MARK mark;
	if( FindMarker(midi_chan,"loopend",&mark) ) // found a loop end point
		BASS_ChannelSetSync(midi_chan,BASS_SYNC_POS|BASS_SYNC_MIXTIME,mark.pos,LoopSync,0); // set a sync there
	// set one at the end too (eg. in case of seeking past the loop point)
	BASS_ChannelSetSync(midi_chan,BASS_SYNC_END|BASS_SYNC_MIXTIME,0,LoopSync,0);

	midi_loop = TRUE;

	// clear lyrics buffer and set lyrics syncs
	lyrics[0]=0;
	if( BASS_MIDI_StreamGetMark(midi_chan,BASS_MIDI_MARK_LYRIC,0,&mark) ) // got lyrics
		BASS_ChannelSetSync(midi_chan,BASS_SYNC_MIDI_LYRIC,0,LyricSync,(void*)BASS_MIDI_MARK_LYRIC);
	else
	if( BASS_MIDI_StreamGetMark(midi_chan,BASS_MIDI_MARK_TEXT,20,&mark) ) // got text instead (over 20 of them)
		BASS_ChannelSetSync(midi_chan,BASS_SYNC_MIDI_TEXT,0,LyricSync,(void*)BASS_MIDI_MARK_TEXT);
	BASS_ChannelSetSync(midi_chan,BASS_SYNC_END,0,EndSync,0);


	// override the initial tempo, and set a sync to override tempo events and another to override after seeking
	SetTempo(TRUE);
	BASS_ChannelSetSync(midi_chan,BASS_SYNC_MIDI_EVENT|BASS_SYNC_MIXTIME,MIDI_EVENT_TEMPO,TempoSync,0);
	BASS_ChannelSetSync(midi_chan,BASS_SYNC_SETPOS|BASS_SYNC_MIXTIME,0,TempoSync,0);

	// get default soundfont in case of matching soundfont being used
	BASS_MIDI_FONT sf;
	BASS_MIDI_StreamGetFonts(midi_chan,&sf,1);
	font = sf.font;

	MIDI_getGain( loaded_file, size );

	FREEMEM( loaded_file );

	// HSTREAM midi_stream = BASS_MIDI_StreamGetChannel(midi_chan, 0); // get MIDI channel
	// BASS_ChannelSetDSP(midi_stream, LevelDSP, 0, 0); // set level checking DSP on it

	midi_dsp = BASS_ChannelSetDSP( midi_chan, MidiDSP, 0, 0); // set level checking DSP on it

	BASS_ChannelPlay(midi_chan,FALSE);

	lastPlayedChannel = midi_chan;

	xprintf("BLIB_PlayMidi: playing \"%s\" file.\n", file );

	return TRUE;
}




//
//
//
BOOL BLIB_StopMidi( void ) {

	if( midi_chan == 0 )
		return TRUE;

	if( midi_dsp ) BASS_ChannelRemoveDSP( midi_chan, midi_dsp );

	BASS_ChannelStop(midi_chan);

	midi_chan = 0;

	return TRUE;
}



//
//
//
BOOL BLIB_IsMidiPlaying( void ) {

	if( midi_chan == 0 )
		return FALSE;

	/* check if the module is stopped */
	BOOL stopped = BASS_ChannelIsActive(midi_chan);

	return BOOLEAN(stopped);
}



//
//
//
BOOL BLIB_GetMidiParam( int *vol, int *pos, int *len, int *percent, int *tempo, char **name, char **str_lyrics ) {

	if( !midi_chan )
		return FALSE;

	// xprintf("midi_level: %d,  %d\n", (int)(midi_levels[0]*100), (int)(midi_levels[1]*100) );

	if( vol ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( midi_chan, BASS_ATTRIB_VOL, &fvol );
		*vol =  (int)(fvol * MAXVOL);
	}

	QWORD max_pos = BASS_ChannelGetLength(midi_chan, BASS_POS_BYTE);
	QWORD cur_pos = BASS_ChannelGetPosition(midi_chan, BASS_POS_BYTE);

	if( pos )
		// *pos = (int)BASS_ChannelGetPosition(chan,BASS_POS_MIDI_TICK)/120; // update position
		*pos = (int)BASS_ChannelBytes2Seconds(midi_chan, cur_pos); // translate to seconds

	if( len )
		// *len = (int)BASS_ChannelGetLength( chan, BASS_POS_MIDI_TICK );
		*len = (int)BASS_ChannelBytes2Seconds(midi_chan, max_pos); // translate to seconds

	// percent
	if( percent ) {

		double maxPos = (signed __int64)max_pos;
		double pos = (signed __int64)cur_pos;

		if( maxPos )
			*percent = ftoi(pos * 100.0 / maxPos);
		else
			*percent = 0;

		if( pos > maxPos )
			*percent = 0;
	}

	if( str_lyrics )
		*str_lyrics = lyrics;

	// char text[10];
	// sprintf(text,"%.1f",60000000/(miditempo*temposcale)); // calculate bpm

	return TRUE;
}



//
//
//
BOOL BLIB_SetMidiParam( int vol, int percent, int tempo, int loop ) {

	if( !midi_chan )
		return FALSE;

	if( vol != -1 )
		BASS_ChannelSetAttribute(midi_chan,BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );

	// pozicíó
	if( percent != -1 ) {

		QWORD pos,maxPos;

		CLAMPMINMAX( percent, 0, 100 );

		maxPos = BASS_ChannelGetLength( midi_chan, BASS_POS_BYTE );

		pos = maxPos*percent / 100;

		// little hackerance
		if( percent == 100 )
			pos = maxPos - 1;

		CLAMPMINMAX( pos, 0, maxPos );

		// xprintf("BLIB_SetMidiParam: percent = %d, maxpos = %d, pos = %d\n", percent, (int)maxPos, (int)pos );

		BASS_ChannelSetPosition( midi_chan, pos, BASS_POS_BYTE );
	}

	if( loop != -1 ) {

		if( loop ) {
			BASS_ChannelFlags( midi_chan, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
			midi_loop = TRUE;
		}
		else {
			BASS_ChannelFlags( midi_chan, 0, BASS_SAMPLE_LOOP);
			midi_loop = FALSE;
		}

	}

	return TRUE;
}





/***************************************
 *
 * BASSEnc
 *
 ***************************************/



#define BASSENCDLLNAME "BASSENC.DLL"


static HINSTANCE hBassEnc = NULL;



//
// load BASS and the required functions
//
static int LoadBASSEnc( void ) {

	if( hBassEnc )
		return 0;

	if( BLIB_Init() == FALSE )
		return 0;

	char bassdllname[PATH_MAX];

	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, BASSENCDLLNAME );

	// load the temporary BASS.DLL library
	if( (hBassEnc = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadBASSEnc: can't load \"%s\" library.\n",bassdllname);
		return 3;
	}

	LOADBASSENCFUNCTION( BASS_Encode_Start );
	LOADBASSENCFUNCTION( BASS_Encode_Stop );
	LOADBASSENCFUNCTION( BASS_RecordInit );
	LOADBASSENCFUNCTION( BASS_RecordFree );
	LOADBASSENCFUNCTION( BASS_Encode_GetVersion );
	LOADBASSENCFUNCTION( BASS_Encode_IsActive );

	ULONG version = BASS_Encode_GetVersion();

	xprintf("LoadBASSEnc: BASSENC (%s) v%d.%d.%d.%d loaded.\n",bassdllname, (version>>24)&0xff, (version>>16)&0xff, (version>>8)&0xff, (version>>0)&0xff );

	return 0;
}





//
// BASSMidi and BASSenc
//
BOOL BLIB_Midi2Wav( const char *midiname, const char *wavname ) {

	xprintf("BLIB_Midi2Wav: writing \"%s\" file from %s.\n", wavname, midiname);

	if( LoadBASSMidi() )
		return FALSE;

	if( LoadBASSEnc() )
		return FALSE;

	BLIB_LoadSoundFont();

	BLIB_StopMidi();

	HSTREAM decoder = 0;		// channel handle
	char *loaded_file;
	int size;

	if( (size = LoadFile( midiname, PTR(loaded_file) )) == 0 ) {
		xprintf("BLIB_Midi2Wav: LoadFile \"%s\" error.\n", midiname );
		return FALSE;
	}

	// create decoding channel from MIDI file or MOD
	if( !(decoder = BASS_MIDI_StreamCreateFile(TRUE,loaded_file,0,size,BASS_STREAM_DECODE,44100)) &&
	    !(decoder = BASS_StreamCreateFile(TRUE,loaded_file,0,size,BASS_STREAM_DECODE)) ) {
		FREEMEM( loaded_file );
		xprintf("BLIB_Midi2Wav: can't play \"%s\" file.\n", midiname);
		return FALSE;
	}

	// set WAV encoder on it
	HENCODE encoder = BASS_Encode_Start(decoder, wavname, BASS_ENCODE_PCM | BASS_ENCODE_MONO, NULL, 0);

#define ENCBUFFSIZE (1024*1024)
	UCHAR *buf = NULL;

	ALLOCMEM( buf, ENCBUFFSIZE );

	// decoder is still active (not ended)
	while( BASS_ChannelIsActive(decoder) && BASS_Encode_IsActive(encoder) ) {

		winSetPanel( NULL, rand()%100, 100 );

		BASS_ChannelGetData( decoder, buf, ENCBUFFSIZE ); // decode & process (encode) data
	}

	BASS_StreamFree(decoder);
	BASS_Encode_Stop(decoder); // close encoder

	if( buf ) FREEMEM( buf );

	if( loaded_file ) FREEMEM( loaded_file );
	loaded_file = NULL;

	return TRUE;
}




//
// c:\MYPROJ~1\Bass\Enc\c\rectest\rectest.c
//
BOOL BLIB_Mic2Wav( int time, const char *wavname ) {

	if( LoadBASSEnc() )
		return FALSE;

	if( !BASS_RecordInit(-1) )
		return FALSE;






	BASS_RecordFree();
	BASS_Free();

	return TRUE;
}







/***************************************
 *
 * SID
 *
 ***************************************/




static HINSTANCE hSid = NULL;

static int (*pLoadSidFile)( char *buf, int len, int tune, char *filename ) = NULL;
static int (*pStopSid)( void ) = NULL;
static int (*pIsPlaying)( void ) = NULL;
static int (*pGetInfo)( int *cur_song, int *num_song, int *pos, int *max_pos, char *title, char *author, char *copyright, char *version ) = NULL;
static int (*pSetInfo)( int arg_cur_song, int pos, int loop );
static int (*pGetData)( UCHAR *buf, int len );
static char *(*pSidError)( char *str ) = NULL;

static HSTREAM sid_stream = 0;

#define XSIDDLLNAME  "XSIDPLAYER.DLL"


//
//
//
static int LoadSID( void ) {

	if( hSid )
		return 0;

	char bassdllname[PATH_MAX];
	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, XSIDDLLNAME );


	if( (hSid = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadSID: couldn't load \"%s\" library.\n",bassdllname);
		return 1;
	}

	if( (pLoadSidFile = (int (__cdecl*)(char*, int, int, char*))GetProcAddress( hSid, "LoadSidFile" )) == NULL ) {
		xprintf("no LoadSidFile\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 2;
	}

	if( (pStopSid = (int (__cdecl*)(void))GetProcAddress( hSid, "StopSid" )) == NULL ) {
		xprintf("no StopSid\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 3;
	}


	if( (pGetInfo = (int (__cdecl*)(int*,int*,int*,int*,char*,char*,char*,char*))GetProcAddress( hSid, "GetInfo" )) == NULL ) {
		xprintf("no GetInfo\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 4;
	}

	if( (pSetInfo = (int (__cdecl*)(int,int,int))GetProcAddress( hSid, "SetInfo" )) == NULL ) {
		xprintf("no SetInfo\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 5;
	}

	if( (pGetData = (int (__cdecl*)(UCHAR*,int))GetProcAddress( hSid, "GetData" )) == NULL ) {
		xprintf("no GetData\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 6;
	}

	if( (pIsPlaying = (int (__cdecl*)(void))GetProcAddress( hSid, "IsPlaying" )) == NULL ) {
		xprintf("no IsPlaying\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 7;
	}

	if( (pSidError = (char *(__cdecl*)(char*))GetProcAddress( hSid, "SidError" )) == NULL ) {
		xprintf("no SidError\n" );
		FreeLibrary( hSid );
		hSid = NULL;
		return 8;
	}

	char version[XMAX_PATH] = "My body is ready.";

	pGetInfo( NULL, NULL, NULL, NULL, NULL, NULL, NULL, version );

	xprintf("LoadSID: %s (%s) loaded.\n",version,bassdllname);

	return 0;
}





//
// stream writer
//
DWORD CALLBACK SID_WriteStream( HSTREAM handle, short *buffer, DWORD length, void *user ) {

	if( pGetData ) {
		// xprintf("pGetData: before len = %d\n", length );
		length = pGetData( (UCHAR *)buffer, length );
		// xprintf("pGetData: after len = %d\n", length );

		if( buffer && (length > 2) )
			musicDSP( 0, 0, (void *)buffer, length, NULL );
	}
	else
		length = 0;

	return length;
}




//
//
//
int SID_PlaySid( const char *filename, int tune ) {

	int result;

	if( filename == NULL )
		return 1;

	if( (result = LoadSID()) ) {
		xprintf("SID_LoadSid: init phailed. (error = %d)\n", result);
		return 2;
	}

	UCHAR *buf;
	int len;

	if( (len = LoadFile( filename, PTR(buf) )) == 0 ) {
		xprintf("SID_PlaySid: LoadFile \"%s\" error.\n", filename );
		return 5;
	}

	result = pLoadSidFile(  (char *)buf, len, tune, (char *)filename );

	if( buf ) FREEMEM( buf );

	if( result != 0 ) {
		xprintf("SID_PlaySid: error loading \"%s\" file. (error = %d)\n", filename, result );
		return result;
	}

	// BASS

	if( BLIB_Init() == FALSE )
		return 3;

	if( sid_stream ) {
		BASS_ChannelStop( sid_stream );
		sid_stream = 0;
	}

	if( (sid_stream = BASS_StreamCreate( 44100, 2, 0, (STREAMPROC*)SID_WriteStream, 0 )) == 0 ) {
		xprintf("SID_PlaySid: stream create phailed. (error = %d)\n", BASS_ErrorGetCode() );
		return 4;
	}

	music_delay = BASS_ChannelSeconds2Bytes( sid_stream, 1 ) / 4;
	music_gain = 1;
	music_count = 0;
	music_high = 0;
	music_quiet = 0;

	int vol = MAXVOL / 2;

	BASS_ChannelSetAttribute( sid_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );

	BASS_ChannelPlay( sid_stream, FALSE );

	lastPlayedChannel = sid_stream;

	xprintf("SID_PlaySid: playing \"%s\".\n", filename );

	return result;
}







//
//
//
BOOL SID_StopSid( void ) {

	if( sid_stream ) {
		BASS_ChannelStop( sid_stream );
		sid_stream = 0;
		lastPlayedChannel = 0;
	}

	if( pStopSid )
		pStopSid();

	return TRUE;
}



//
//
//
BOOL SID_IsPlaying( void ) {

	if( pIsPlaying )
		return pIsPlaying();

	return FALSE;
}



//
//
//
BOOL SID_SetParam( int tune, int vol, int pos, int loop ) {

	xprintf("SID_SetParam: called with %d, %d, %d, %d\n", tune, vol, pos, loop );

	if( (tune > -1) && pSetInfo && sid_stream ) {

		BASS_ChannelPause( sid_stream );

		pSetInfo( tune, -1, -1 );

		BASS_ChannelPlay( sid_stream, FALSE );
	}

	if( (vol > -1) && sid_stream ) {

		CLAMPMAX( vol, MAXVOL );

		BASS_ChannelSetAttribute( sid_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );
	}

	if( (loop > -1) && pSetInfo ) {

		pSetInfo( -1, -1, loop );
	}

	return TRUE;
}



//
//
//
int SID_GetParam( int *arg_cur_song, int *arg_num_song, int *vol,
		  int *pos, int *max_pos,
		  char *arg_title, char *arg_author, char *arg_copyright,
		  char *arg_version ) {

	if( vol && sid_stream ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( sid_stream, BASS_ATTRIB_VOL, &fvol );
		*vol =  (int)(fvol * MAXVOL);
	}

	if( pGetInfo == NULL )
		return 1;

	return pGetInfo( arg_cur_song, arg_num_song,
			 pos, max_pos,
			 arg_title,
			 arg_author,
			 arg_copyright,
			 arg_version );
}





/***************************************
 *
 * XAdPlug.dll
 *
 ***************************************/




static HINSTANCE hAdplug = NULL;

static int (*pAdplugPlay)( char *buf, int len, int tune, char *filename ) = NULL;
static void (*pAdplugStop)( void ) = NULL;
static int (*pAdplugIsPlaying)( void ) = NULL;
static int (*pAdplugGetInfo)( int *cur_song, int *num_song, int *pos, int *max_pos, char *title, char *author, char *desc, char *version ) = NULL;
static int (*pAdplugSetInfo)( int arg_cur_song, int pos, int loop );
static int (*pAdplugGetData)( char *buf, int len );
static char *(*pAdplugError)( char *str ) = NULL;

static HSTREAM adplug_stream = 0;

#define XADPLUGDLLNAME  "XADPLUG.DLL"



//
//
//
static int LoadADPLUG( void ) {

	if( hAdplug )
		return 0;

	char bassdllname[PATH_MAX];
	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, XADPLUGDLLNAME );
	// sprintf( bassdllname, "c:\\MYPROJ~1\\ADPLUG~1.1\\Xadplug.dll" );

	if( (hAdplug = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadADPLUG: couldn't load \"%s\" library.\n",bassdllname);
		return 1;
	}

	if( (pAdplugPlay = (int (__cdecl*)(char*, int, int, char*))GetProcAddress( hAdplug, "Play" )) == NULL ) {
		xprintf("no AdplugPlay\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 2;
	}

	if( (pAdplugStop = (void (__cdecl*)(void))GetProcAddress( hAdplug, "Stop" )) == NULL ) {
		xprintf("no AdplugStop\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 3;
	}


	if( (pAdplugGetInfo = (int (__cdecl*)(int*,int*,int*,int*,char*,char*,char*,char*))GetProcAddress( hAdplug, "GetInfo" )) == NULL ) {
		xprintf("no AdplugGetInfo\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 4;
	}

	if( (pAdplugSetInfo = (int (__cdecl*)(int,int,int))GetProcAddress( hAdplug, "SetInfo" )) == NULL ) {
		xprintf("no AdplugSetInfo\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 5;
	}

	if( (pAdplugGetData = (int (__cdecl*)(char*,int))GetProcAddress( hAdplug, "GetData" )) == NULL ) {
		xprintf("no AdplugGetData\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 6;
	}

	if( (pAdplugIsPlaying = (int (__cdecl*)(void))GetProcAddress( hAdplug, "IsPlaying" )) == NULL ) {
		xprintf("no AdplugIsPlaying\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 7;
	}

	if( (pAdplugError = (char *(__cdecl*)(char*))GetProcAddress( hAdplug, "Error" )) == NULL ) {
		xprintf("no AdplugError\n" );
		FreeLibrary( hAdplug );
		hAdplug = NULL;
		return 8;
	}

	char version[XMAX_PATH] = "My body is ready.";

	pAdplugGetInfo( NULL, NULL, NULL, NULL, NULL, NULL, NULL, version );

	xprintf("LoadADPLUG: %s (%s) loaded.\n",version,bassdllname);

	return 0;
}





//
// stream writer
//
DWORD CALLBACK Adplug_WriteStream( HSTREAM handle, short *buffer, DWORD length, void *user ) {

	if( pAdplugGetData ) {
		// xprintf("pGetData: before len = %d\n", length );
		length = pAdplugGetData( (char *)buffer, length );
		// xprintf("pGetData: after len = %d\n", length );

		if( buffer && (length > 2) )
			musicDSP( 0, 0, (void *)buffer, length, NULL );
	}
	else
		length = 0;

	return length;
}




//
//
//
int AD_Play( const char *filename, int tune ) {

	int result;

	if( filename == NULL )
		return 1;

	if( (result = LoadADPLUG()) ) {
		xprintf("AD_Play: init phailed. (error = %d)\n", result);
		return 2;
	}

	UCHAR *buf;
	int len;

	if( (len = LoadFile( filename, PTR(buf) )) == 0 ) {
		xprintf("AD_Play: LoadFile \"%s\" error.\n", filename );
		return 4;
	}

	char str[XMAX_PATH*2];

	PushTomFlag();
	TomFlag( NOFLAG );

	sprintf( str, "%s%c%s", GetRealTemp(), PATHDELIM, GetFilenameNoPath( (char *)filename ) );

	XL_WriteFile( str, (UCHAR *)buf, len );

	PopTomFlag();

	result = pAdplugPlay( (char *)buf, len, tune, str );

	if( buf ) FREEMEM( buf );

	UnlinkFile( str );

	if( result != 0 ) {
		xprintf("AD_Play: error loading \"%s\" file. (error = %d)\n", filename, result );
		return result;
	}

	// BASS
	if( BLIB_Init() == FALSE )
		return 3;

	if( adplug_stream ) {
		BASS_ChannelStop( adplug_stream );
		adplug_stream = 0;
	}

	if( (adplug_stream = BASS_StreamCreate( 44100, 1, 0, (STREAMPROC*)Adplug_WriteStream, 0 )) == 0 ) {
		xprintf("AD_Play: stream create phailed. (error = %d)\n", BASS_ErrorGetCode() );
		return 4;
	}

	int vol = MAXVOL / 2;

	BASS_ChannelSetAttribute( adplug_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );

	music_delay = BASS_ChannelSeconds2Bytes( adplug_stream, 1 ) / 4;
	music_gain = 1;
	music_count = 0;
	music_high = 0;
	music_quiet = 0;

	// xprintf("%.2f %d %d %d %d\n", music_gain, music_delay, music_count, music_high, music_quiet );

	BASS_ChannelPlay( adplug_stream, FALSE );

	lastPlayedChannel = adplug_stream;

	xprintf("AD_Play: playing \"%s\".\n", filename );

	return result;
}







//
//
//
BOOL AD_Stop( void ) {

	if( adplug_stream ) {
		BASS_ChannelStop( adplug_stream );
		adplug_stream = 0;
		lastPlayedChannel = 0;
	}

	if( pAdplugStop )
		pAdplugStop();

	return TRUE;
}



//
//
//
BOOL AD_IsPlaying( void ) {

	if( pAdplugIsPlaying )
		return pAdplugIsPlaying();

	return FALSE;
}



//
//
//
BOOL AD_SetParam( int tune, int vol, int pos, int loop ) {

	xprintf("AD_SetParam: called with %d, %d, %d, %d\n", tune, vol, pos, loop );

	if( (tune > -1) && pAdplugSetInfo && adplug_stream ) {

		BASS_ChannelPause( adplug_stream );

		pAdplugSetInfo( tune, -1, -1 );

		BASS_ChannelPlay( adplug_stream, FALSE );
	}

	if( (vol > -1) && adplug_stream ) {

		CLAMPMAX( vol, MAXVOL );

		BASS_ChannelSetAttribute( adplug_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );
	}

	if( (pos > -1) && pAdplugSetInfo ) {

		pAdplugSetInfo( -1, pos, -1 );
	}

	if( (loop > -1) && pAdplugSetInfo ) {

		pAdplugSetInfo( -1, -1, loop );
	}

	return TRUE;
}



//
//
//
int AD_GetParam( int *arg_cur_song, int *arg_num_song, int *vol,
		  int *pos, int *max_pos,
		  char *arg_title, char *arg_author, char *arg_copyright,
		  char *arg_version ) {

	if( vol && adplug_stream ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( adplug_stream, BASS_ATTRIB_VOL, &fvol );
		*vol = (int)(fvol * MAXVOL);
	}

	// if( pAdplugError ) xprintf("---------------------%s\n", pAdplugError( NULL ) );

	if( pAdplugGetInfo == NULL )
		return 1;

	// xprintf("%.2f %d %d %d %d\n", music_gain, music_delay, music_count, music_high, music_quiet );

	return pAdplugGetInfo( arg_cur_song, arg_num_song,
			       pos, max_pos,
			       arg_title,
			       arg_author,
			       arg_copyright,
			       arg_version );
}








/***************************************
 *
 * Xym.dll
 *
 *  Yamaha YM2149 SoundChip Processor emulation engine
 *  Specific ATARI-ST Sound programming emulation. (SID-Voice and DigiDrum emulation)
 *  AMIGA "PAULA" SoundChip Processor emulation engine.
 *   Universal Digital Sound Tracker engine.
 *
 ***************************************/




static HINSTANCE hYm = NULL;

static int (*pYmPlay)( char *buf, int len, char *filename ) = NULL;
static void (*pYmStop)( void ) = NULL;
static int (*pYmIsPlaying)( void ) = NULL;
static int (*pYmGetInfo)( int *pos, int *max_pos, char *title, char *author, char *desc, char *version ) = NULL;
static int (*pYmSetInfo)( int pos, int loop );
static int (*pYmGetData)( char *buf, int len );
static char *(*pYmError)( char *str ) = NULL;

static HSTREAM ym_stream = 0;

#define XYMDLLNAME  "XYM.DLL"



//
//
//
static int LoadYM( void ) {

	if( hYm )
		return 0;

	char bassdllname[PATH_MAX];
	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, XYMDLLNAME );

	if( (hYm = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadYM: couldn't load \"%s\" library.\n",bassdllname);
		return 1;
	}

	if( (pYmPlay = (int (__cdecl*)(char*, int, char*))GetProcAddress( hYm, "Play" )) == NULL ) {
		xprintf("no YmPlay\n" );
		FreeLibrary( hYm );
		hYm = NULL;
		return 2;
	}

	if( (pYmStop = (void (__cdecl*)(void))GetProcAddress( hYm, "Stop" )) == NULL ) {
		xprintf("no YmStop\n" );
		FreeLibrary( hYm );
		hYm = NULL;
		return 3;
	}

	if( (pYmGetInfo = (int (__cdecl*)(int*,int*,char*,char*,char*,char*))GetProcAddress( hYm, "GetInfo" )) == NULL ) {
		xprintf("no YmGetInfo\n" );
		FreeLibrary( hYm );
		hYm = NULL;
		return 4;
	}

	if( (pYmSetInfo = (int (__cdecl*)(int,int))GetProcAddress( hYm, "SetInfo" )) == NULL ) {
		xprintf("no YmSetInfo\n" );
		FreeLibrary( hAdplug );
		hYm = NULL;
		return 5;
	}

	if( (pYmGetData = (int (__cdecl*)(char*,int))GetProcAddress( hYm, "GetData" )) == NULL ) {
		xprintf("no YmGetData\n" );
		FreeLibrary( hYm );
		hYm = NULL;
		return 6;
	}

	if( (pYmIsPlaying = (int (__cdecl*)(void))GetProcAddress( hYm, "IsPlaying" )) == NULL ) {
		xprintf("no YmIsPlaying\n" );
		FreeLibrary( hYm );
		hYm = NULL;
		return 7;
	}

	if( (pYmError = (char *(__cdecl*)(char*))GetProcAddress( hYm, "Error" )) == NULL ) {
		xprintf("no YmError\n" );
		FreeLibrary( hYm );
		hYm = NULL;
		return 8;
	}

	char version[XMAX_PATH] = "My body is ready.";

	pYmGetInfo( NULL, NULL, NULL, NULL, NULL, version );

	xprintf("LoadYm: %s (%s) loaded.\n",version,bassdllname);

	return 0;
}





//
// stream writer
//
DWORD CALLBACK Ym_WriteStream( HSTREAM handle, short *buffer, DWORD length, void *user ) {

	if( pYmGetData ) {

		length = pYmGetData( (char *)buffer, length );

		if( buffer && (length > 2) )
			musicDSP( 0, 0, (void *)buffer, length, NULL );
	}
	else
		length = 0;

	return length;
}




//
//
//
int YM_Play( const char *filename ) {

	int result;

	if( filename == NULL )
		return 1;

	if( (result = LoadYM()) ) {
		xprintf("YM_Play: init phailed. (error = %d)\n", result);
		return 2;
	}

	UCHAR *buf;
	int len;

	if( (len = LoadFile( filename, PTR(buf) )) == 0 ) {
		xprintf("YM_Play: LoadFile \"%s\" error.\n", filename );
		return 5;
	}

	result = pYmPlay( (char *)buf, len, (char *)filename );

	if( buf ) FREEMEM( buf );

	if( result != 0 ) {
		xprintf("YM_Play: error loading \"%s\" file. (error = %d)\n", filename, result );
		return result;
	}

	// BASS
	if( BLIB_Init() == FALSE )
		return 3;

	if( ym_stream ) {
		BASS_ChannelStop( ym_stream );
		ym_stream = 0;
	}

	if( (ym_stream = BASS_StreamCreate( 44100, 1, 0, (STREAMPROC*)Ym_WriteStream, 0 )) == 0 ) {
		xprintf("YM_Play: stream create phailed. (error = %d)\n", BASS_ErrorGetCode() );
		return 4;
	}

	int vol = MAXVOL / 2;

	BASS_ChannelSetAttribute( ym_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );

	music_delay = BASS_ChannelSeconds2Bytes( ym_stream, 1 ) / 4;
	music_gain = 1;
	music_count = 0;
	music_high = 0;
	music_quiet = 0;

	// xprintf("%.2f %d %d %d %d\n", music_gain, music_delay, music_count, music_high, music_quiet );

	BASS_ChannelPlay( ym_stream, FALSE );

	lastPlayedChannel = ym_stream;

	xprintf("YM_Play: playing \"%s\".\n", filename );

	return result;
}







//
//
//
BOOL YM_Stop( void ) {

	if( ym_stream ) {
		BASS_ChannelStop( ym_stream );
		ym_stream = 0;
		lastPlayedChannel = 0;
	}

	if( pYmStop )
		pYmStop();

	return TRUE;
}



//
//
//
BOOL YM_IsPlaying( void ) {

	if( pYmIsPlaying )
		return pYmIsPlaying();

	return FALSE;
}



//
//
//
BOOL YM_SetParam( int vol, int pos, int loop ) {

	xprintf("YM_SetParam: called with %d, %d, %d\n", vol, pos, loop );

	if( (vol > -1) && ym_stream ) {

		CLAMPMAX( vol, MAXVOL );

		BASS_ChannelSetAttribute( ym_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );
	}

	if( (pos > -1) && pYmSetInfo ) {

		pYmSetInfo( pos, -1 );
	}

	if( (loop > -1) && pYmSetInfo ) {

		pYmSetInfo( -1, loop );
	}

	return TRUE;
}



//
//
//
int YM_GetParam(  int *vol,
		  int *pos, int *max_pos,
		  char *arg_title, char *arg_author, char *arg_desc,
		  char *arg_version ) {

	if( vol && ym_stream ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( ym_stream, BASS_ATTRIB_VOL, &fvol );
		*vol = (int)(fvol * MAXVOL);
	}

	// if( pAdplugError ) xprintf("---------------------%s\n", pAdplugError( NULL ) );

	if( pYmGetInfo == NULL )
		return 1;

	// xprintf("%.2f %d %d %d %d\n", music_gain, music_delay, music_count, music_high, music_quiet );

	return pYmGetInfo( pos, max_pos,
			   arg_title,
			   arg_author,
			   arg_desc,
			   arg_version );
}









/***************************************
 *
 * NSF
 *
 ***************************************/



#include "c:/MYPROJ~1/NOSEFA~1.7-M/src/winamp/winamp.h"


static HINSTANCE hNsf = NULL;

static In_Module *(*pNosefart_GetInfo)( void ) = NULL;

static In_Module *mod = NULL;

static HSTREAM nsf_stream = 0;


#define NSFDLLNAME  "XNOSEFART.DLL"




static Out_Module outmod = {
	OUT_VER,
	NULL,
	1,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};


//
//
//
static int LoadNSF( void ) {

	if( hNsf  )
		return 0;

	char bassdllname[PATH_MAX];
	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, NSFDLLNAME );


	if( (hNsf = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadNSF: couldn't load \"%s\" library.\n",bassdllname);
		return 1;
	}

	if( (pNosefart_GetInfo = (In_Module *(__cdecl*)(void))GetProcAddress( hNsf, "Nosefart_GetInfo" )) == NULL ) {
		xprintf("no Nosefart_GetInfo\n" );
		FreeLibrary( hNsf );
		hNsf = NULL;
		return 2;
	}

	if( (mod = pNosefart_GetInfo()) == NULL ) {
		xprintf("LoadNSF: Nosefart_GetInfo phailed.\n" );
		FreeLibrary( hNsf );
		hNsf = NULL;
		return 3;
	}

	mod->hMainWindow = (HWND)GetHwnd();
	mod->hDllInstance = hNsf;

	mod->outMod = &outmod;

	mod->Init();

	char version[XMAX_PATH] = "My body is ready.";

	xprintf("LoadNSF: %s (%s) loaded.\n",mod->description,bassdllname);

	// mod->Config( (HWND)GetHwnd() );

	return 0;
}



//
// stream writer
//
DWORD CALLBACK NSF_WriteStream( HSTREAM handle, short *buffer, DWORD length, void *user ) {

/*	if( pGetData ) {
		// xprintf("pGetData: before len = %d\n", length );
		length = pGetData( (UCHAR *)buffer, length );
		// xprintf("pGetData: after len = %d\n", length );
	}
	else
*/		length = 0;

	return length;
}





//
//
//
int NSF_Play( const char *filename, int tune ) {

	int result;

	if( filename == NULL )
		return 1;

	if( (result = LoadNSF()) ) {
		xprintf("LoadNsf: init phailed. (error = %d)\n", result);
		return 2;
	}

	// FIXME
	xprintf("NSF_Play: player is NYI.\n");
	return 10;

	// mod->InfoBox( filename, (HWND)GetHwnd() );

	UCHAR *buf;
	int len;

	if( (len = LoadFile( filename, PTR(buf) )) == 0 ) {
		xprintf("NSF_Play: LoadFile \"%s\" error.\n", filename );
		return 5;
	}

	result = mod->Play( (char *)filename, tune );

	if( buf ) FREEMEM( buf );

	if( result != 0 ) {
		xprintf("NSF_Play: error loading \"%s\" file. (error = %d)\n", filename, result );
		return result;
	}

	// BASS

	if( BLIB_Init() == FALSE )
		return 3;

	if( nsf_stream ) {
		BASS_ChannelStop( nsf_stream );
		nsf_stream = 0;
	}

	if( (nsf_stream = BASS_StreamCreate( 44100, 2, 0, (STREAMPROC*)NSF_WriteStream, 0 )) == 0 ) {
		xprintf("NSF_Play: stream create phailed. (error = %d)\n", BASS_ErrorGetCode() );
		return 4;
	}
	else {
		int vol = MAXVOL / 2;

		BASS_ChannelSetAttribute( nsf_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );

		BASS_ChannelPlay( nsf_stream, FALSE );

		lastPlayedChannel = nsf_stream;
	}

	xprintf("NSF_Play: playing \"%s\".\n", filename );

	return result;
}







//
//
//
BOOL NSF_Stop( void ) {

	if( nsf_stream ) {
		BASS_ChannelStop( nsf_stream );
		nsf_stream = 0;
		lastPlayedChannel = 0;
	}

	if( mod )
		mod->Stop();

	return TRUE;
}



//
//
//
BOOL NSF_IsPlaying( void ) {

	if( mod && mod->outMod )
		mod->outMod->IsPlaying();

	return FALSE;
}



//
//
//
BOOL NSF_SetParam( int tune, int vol, int pos, int loop ) {

	xprintf("NSF_SetParam: called with %d, %d, %d, %d\n", tune, vol, pos, loop );

	if( (tune > -1) && mod && nsf_stream ) {

		BASS_ChannelPause( nsf_stream );

		// pSetInfo( tune, -1, -1 );

		BASS_ChannelPlay( nsf_stream, FALSE );
	}

	if( (vol > -1) && nsf_stream ) {

		CLAMPMAX( vol, MAXVOL );

		BASS_ChannelSetAttribute( nsf_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );
	}

	if( (loop > -1) && mod ) {

		// pSetInfo( -1, -1, loop );
	}

	return TRUE;
}




//
//
//
int NSF_GetParam( int *arg_cur_song, int *arg_num_song, int *vol,
		  int *pos, int *max_pos,
		  char *arg_title, char *arg_author, char *arg_copyright,
		  char *arg_version ) {

	if( vol && nsf_stream ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( nsf_stream, BASS_ATTRIB_VOL, &fvol );
		*vol =  (int)(fvol * MAXVOL);
	}

	if( mod == NULL )
		return 1;

	return 1;
}





/***************************************
 *
 * Netradio
 *
 ***************************************/

/*
static const char *urls[10]={ // preset stream URLs
	"http://www.radioparadise.com/musiclinks/rp_128-9.m3u",
	"http://www.radioparadise.com/musiclinks/rp_32.m3u",
	"http://www.sky.fm/mp3/classical.pls",
	"http://www.sky.fm/mp3/classical_low.pls",
	"http://www.sky.fm/mp3/the80s.pls",
	"http://www.sky.fm/mp3/the80s_low.pls",
	"http://somafm.com/tags.pls",
	"http://somafm.com/tags32.pls",
	"http://somafm.com/secretagent.pls",
	"http://somafm.com/secretagent24.pls"
};
*/

#define NRSTR 512

static char nr_proxy[NRSTR] = "Hurr"; // proxy server


static DWORD cthread=0;
static HSTREAM netradio_channel = 0;
static char *netradio_url = NULL;
static int url_len = 0;
static BOOL bIsUrl = FALSE;		// a netradio_url:
					// FALSE: egy betöltött fájl
					// TRUE: "http://valami.hu/valami.pls"

static MMRESULT TimerId = 0;


static char nr_title[NRSTR] = "Herp";
static char nr_artist[NRSTR] = "Derp";
static char nr_status[NRSTR] = "Hurr";
static char nr_conn[NRSTR] = "Durr";

static ULONG start_tic = 0;	// amikor elkezdõdött a lejátszás
static BOOL netradio_playing = FALSE;


#define BASSAACDLLNAME "BASS_AAC.DLL"

static HINSTANCE hBassAac = NULL;



//
// load BASS and the required functions
//
static int LoadBASSAac( void ) {

	if( hBassAac )
		return 0;

	if( BLIB_Init() == FALSE )
		return 0;

	char bassdllname[PATH_MAX];

	sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, BASSAACDLLNAME );

	// load the temporary BASS.DLL library
	if( (hBassAac = LoadLibrary(bassdllname)) == NULL ) {
		xprintf("LoadBASSAac: can't load \"%s\" library.\n",bassdllname);
		return 3;
	}

	LOADBASSAACFUNCTION( BASS_AAC_StreamCreateFile );
	LOADBASSAACFUNCTION( BASS_AAC_StreamCreateURL );
	LOADBASSAACFUNCTION( BASS_AAC_StreamCreateFileUser );
	LOADBASSAACFUNCTION( BASS_MP4_StreamCreateFile );
	LOADBASSAACFUNCTION( BASS_MP4_StreamCreateFileUser );

	xprintf("LoadBASSAac: BASS_AAC (%s) loaded.\n",bassdllname);

	return 0;
}




//
// update stream title from metadata
//
static void NR_DoMeta( void ) {

	const char *meta = BASS_ChannelGetTags(netradio_channel,BASS_TAG_META);

	if( meta ) { // got Shoutcast metadata

		char *p = strstr(meta,"StreamTitle='");

		if( p ) {
			char *s = (char *)malloc( strlen(p+13) + 1 );
			strcpy( s, p+13 );
			strchr(s,';')[-1]=0;
			strncpy( nr_title, s, NRSTR-1 );
			free( s );
		}
	}
	else {

		meta = BASS_ChannelGetTags(netradio_channel,BASS_TAG_OGG);

		if( meta ) { // got Icecast/OGG tags

			const char *artist=NULL,*title=NULL,*p=meta;

			for( ; *p; p+=strlen(p)+1 ) {

				if( !strnicmp(p,"artist=",7) ) // found the artist
					artist = p+7;

				if( !strnicmp(p,"title=",6) ) // found the title
					title = p+6;
			}

			if( artist )
				strcpy( nr_artist, artist );

			if( title )
				strcpy( nr_title, title );
		}
	}

	return;
}



//
//
//
static void CALLBACK NR_MetaSync( HSYNC handle, DWORD channel, DWORD data, void *user ) {

	NR_DoMeta();

	return;
}


//
//
//
static void CALLBACK NR_EndSync( HSYNC handle, DWORD channel, DWORD data, void *user ) {

	strcpy( nr_status, "not playing" );
	nr_title[0] = 0;
	nr_artist[0] = 0;
	nr_conn[0] = 0;

	return;
}


//
//
//
static void CALLBACK NR_StatusProc( const void *buffer, DWORD length, void *user ) {

	// display connection status

	if( buffer && !length )
		strcpy( nr_conn, (char *)buffer );

	return;
}




//
//
//
void CALLBACK NR_TimerProc( UINT IDEvent, UINT uReserved, DWORD dwUser, DWORD dwReserved1, DWORD dwReserved2 ) {

	// monitor prebuffering progress
	DWORD progress = BASS_StreamGetFilePosition(netradio_channel,BASS_FILEPOS_BUFFER)
		*100/BASS_StreamGetFilePosition(netradio_channel,BASS_FILEPOS_END); // percentage of buffer filled

	if( progress>75 || !BASS_StreamGetFilePosition(netradio_channel,BASS_FILEPOS_CONNECTED)) { // over 75% full (or end of download)

		// finished prebuffering, stop monitoring
		timeKillEvent(TimerId);
		TimerId = 0;

		{ // get the broadcast name and URL
			const char *icy = BASS_ChannelGetTags(netradio_channel,BASS_TAG_ICY);

			if( !icy )
				icy = BASS_ChannelGetTags(netradio_channel,BASS_TAG_HTTP); // no ICY tags, try HTTP

			if( icy ) {
				for( ; *icy; icy+=strlen(icy)+1 ) {
					if( !strnicmp(icy,"icy-name:",9) )
						strcpy( nr_status, icy+9 );
					if( !strnicmp(icy,"icy-url:",8) )
						strcpy( nr_conn, icy+8 );
				}
			}
			else
				nr_status[0] = 0;
		}

		// get the stream title and set sync for subsequent titles
		NR_DoMeta();

		BASS_ChannelSetSync(netradio_channel,BASS_SYNC_META,0,&NR_MetaSync,0); // Shoutcast
		BASS_ChannelSetSync(netradio_channel,BASS_SYNC_OGG_CHANGE,0,&NR_MetaSync,0); // Icecast/OGG

		// set sync for end of stream
		BASS_ChannelSetSync(netradio_channel,BASS_SYNC_END,0,&NR_EndSync,0);

		// play it!
		BASS_ChannelPlay(netradio_channel,FALSE);
	}
	else
		sprintf( nr_status, "buffering... %d%%", progress );

	return;
}




//
//
//
static void __cdecl OpenURL( void *url ) {

	strcpy( nr_status, "connecting..." );
	nr_title[0] = 0;
	nr_artist[0] = 0;
	nr_conn[0] = 0;

	// char *s = "http://83.170.109.107:8000";  // AAC

	if( bIsUrl == TRUE ) {
		// ha URL
		netradio_channel = BASS_StreamCreateURL((char *)url,0,BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE,NR_StatusProc,0);

		// lehet AAC
		if( !netradio_channel && !LoadBASSAac() )
			netradio_channel = BASS_AAC_StreamCreateURL((char *)url,0,BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE,NR_StatusProc,0);
	}
	else {
		// ha sima file
		netradio_channel = BASS_StreamCreateFile( TRUE, (char *)url,0,url_len, BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE);

		// lehet AAC vagy MP4
		if( !netradio_channel && !LoadBASSAac() ) {

			netradio_channel = BASS_AAC_StreamCreateFile( TRUE, (char *)url,0,url_len, BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE);

			if( !netradio_channel )
				netradio_channel = BASS_MP4_StreamCreateFile( TRUE, (char *)url,0,url_len, BASS_STREAM_BLOCK|BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE);
		}
	}

	if( !netradio_channel ) {
		// failed to open
		strcpy( nr_status, "not playing" );
		xprintf("OpenURL: can't play. (%d).\n", BASS_ErrorGetCode() );
		netradio_playing = FALSE;
	}
	else {
		// start prebuffer monitoring

		TimerId = timeSetEvent( 50,		/* how often			*/
					50,		/* timer resolution		*/
					NR_TimerProc,	/* callback function		*/
					0,		/* info to pass to callback	*/
					TIME_PERIODIC); /* oneshot or periodic?		*/

		start_tic = GetTic();

		int vol = MAXVOL/2;

		BASS_ChannelSetAttribute( netradio_channel, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );

		lastPlayedChannel = netradio_channel;
	}

	cthread=0;

	return;
}






//
// NetRadio_Play
//
int NR_Play( const char *filename, int playlist_num, char *arg_proxy ) {

	if( cthread ) {
		xprintf("NR_Play: already connecting...\n");
		// MessageBeep(0);
		return 0;
	}

	if( filename == NULL )
		return 1;

	if( BLIB_Init() == FALSE )
		return 2;

	NR_Stop();

	if( (strlen(filename) >= 7) && !strncmp( filename, "http://", 7) ) {
		// ha URL
		ALLOCMEM( netradio_url, strlen(filename)+1 );
		strcpy( netradio_url, filename );
		bIsUrl = TRUE;
	}
	else {
		// akkor playlist: .m3u vagy .pls

		char *p = PLS_GetName(filename,playlist_num);

		if( p && (strlen(p) >= 7) && !strncmp( p, "http://", 7) ) {

			url_len = strlen(p)+1;
			ALLOCMEM( netradio_url, url_len );
			strcpy( netradio_url, p );

			bIsUrl = TRUE;
		}
		else {
			// ha sima file .ogg vagy .mp3

			if( (url_len = LoadFile( filename, PTR(netradio_url) )) == 0 ) {
				xprintf("NR_Play: LoadFile \"%s\" error.\n", filename );
				return 5;
			}

			bIsUrl = FALSE;
		}
	}

	BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST,2); // enable playlist processing
	BASS_SetConfig(BASS_CONFIG_NET_PREBUF,0); // minimize automatic pre-buffering, so we can do it (and display it) instead

	nr_proxy[0] = 0;
	if( arg_proxy )
		strcpy( nr_proxy, arg_proxy );

	BASS_SetConfigPtr(BASS_CONFIG_NET_PROXY,nr_proxy); // setup proxy server location

	// open URL in a new thread (so that main thread is free)
	if( (cthread = _beginthread(OpenURL,0,netradio_url)) < 1 ) {
		xprintf("NR_Play: beginthread phailed.\n");
		return 3;
	}

	netradio_playing = TRUE;

	xprintf("NR_Play: playing \"%s\".\n", filename );

	return 0;
}



//
//
//
BOOL NR_Stop( void ) {

	if( TimerId )
		timeKillEvent(TimerId);
	TimerId = 0;

	if( lastPlayedChannel == netradio_channel )
		lastPlayedChannel = 0;

	if( netradio_channel )
		BASS_StreamFree(netradio_channel); // close old stream
	netradio_channel = 0;

	if( netradio_url ) FREEMEM( netradio_url );
	netradio_url = NULL;
	url_len = 0;

	bIsUrl = FALSE;

	netradio_playing = FALSE;

	return TRUE;
}


//
//
//
BOOL NR_IsPlaying( void ) {

	/***
	if( TimerId && netradio_channel )
		return TRUE;

	if( cthread > 0 )
		return TRUE;
	***/

	return netradio_playing;
}


//
//
//
BOOL NR_Pause( void ) {

	if( netradio_playing == FALSE )
		return FALSE;

	if( netradio_channel )
		BASS_Pause();

	return TRUE;
}


//
//
//
BOOL NR_Resume( void ) {

	if( netradio_playing == FALSE )
		return FALSE;

	if( netradio_channel )
		BASS_Start();

	return TRUE;
}


//
//
//
BOOL NR_GetParam( int *vol, int *pos,
		  char *title, char *artist,
		  char *status, char *connection,
		  int *playlist_num, int *playlist_maxnum ) {

	if( vol && netradio_channel ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( netradio_channel, BASS_ATTRIB_VOL, &fvol );
		*vol = (int)(fvol * MAXVOL);
	}

	if( pos ) *pos = 0;

	if( pos && netradio_channel )
		*pos = (GetTic() - start_tic) / TICKBASE;

	if( title )
		strcpy( title, nr_title );

	if( artist  )
		strcpy( artist, nr_artist );

	if( status  )
		strcpy( status, nr_status );

	if( connection )
		strcpy( connection, nr_conn );

	if( playlist_num )
		*playlist_num = 0;

	if( playlist_maxnum )
		*playlist_maxnum = 0;

	return TRUE;
}



//
//
//
BOOL NR_SetParam( int vol, int pos, int playlist_num ) {

	if( (vol > -1) && netradio_channel ) {

		CLAMPMAX( vol, MAXVOL );

		BASS_ChannelSetAttribute( netradio_channel, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );
	}

	return TRUE;
}





///////////////////////////////////
//
// Generic PUT stream
//
///////////////////////////////////


static HSTREAM put_stream = 0;


//
//
//
void PUT_WriteStream( void *buffer, int length ) {

	// TODO: 8 bitest is
	if( !length || !put_stream || !buffer )
		return;

	// xprintf("len: %d\n", length );
	int len = BASS_StreamPutData( put_stream, buffer, length*2 );

	return;
}


//
//
//
int PUT_Init( int freq, int bits ) {

	int result = 0;

	// BASS
	if( BLIB_Init() == FALSE )
		return 1;

	if( put_stream ) {
		BASS_ChannelStop( put_stream );
		put_stream = 0;
	}

	if( freq == -1 )
		freq = 22050;

	if( (put_stream = BASS_StreamCreate( freq, 1, 0, STREAMPROC_PUSH, 0 )) == 0 ) {
		xprintf("PUT_Play: stream create phailed. (error = %d)\n", BASS_ErrorGetCode() );
		return 2;
	}

	BASS_ChannelSetAttribute( put_stream, BASS_ATTRIB_VOL, 1.0f );

	BASS_ChannelPlay( put_stream, FALSE );

	lastPlayedChannel = put_stream;

	xprintf("PUT_Init: inited at %d Hz.\n", freq );

	return result;
}




//
//
//
void PUT_Deinit( void ) {

	if( put_stream ) {
		BASS_ChannelStop( put_stream );
		put_stream = 0;
		lastPlayedChannel = 0;
	}

	return;
}



//
//
//
BOOL PUT_IsPlaying( void ) {

	if( put_stream )
		return TRUE;

	return FALSE;
}



//
//
//
BOOL PUT_SetParam( int vol, int pan ) {

	xprintf("PUT_SetParam: called with %d, %d\n", vol, pan );

	if( (vol > -1) && put_stream ) {

		CLAMPMAX( vol, MAXVOL );

		BASS_ChannelSetAttribute( put_stream, BASS_ATTRIB_VOL, (FLOAT)vol / (FLOAT)MAXVOL );
	}

	if( pan > -1 ) {
	}

	return TRUE;
}



//
//
//
int PUT_GetParam(  int *vol, int *pan ) {

	if( vol && put_stream ) {
		FLOAT fvol;
		BASS_ChannelGetAttribute( put_stream, BASS_ATTRIB_VOL, &fvol );
		*vol = (int)(fvol * MAXVOL);
	}

	if( pan && put_stream ) {

	}

	return 0;
}





#endif


