
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)


#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <direct.h>
#include <mmsystem.h>

#include <xlib.h>

RCSID( "$Id: midi.cpp,v 1.1.1.1 2003/08/19 17:48:47 bernie Exp $" )


extern "C" HWND hwnd;



//
//
//
BOOL MIDI_PlayFile( char *filename ) {

	char buffer[PATH_MAX + 100];
	int error;

	// MIDI_Stop();

	sprintf(buffer, "open \"%s\" type sequencer alias MUSIC", filename);

	if( (error = mciSendString(buffer, NULL, 0, NULL)) != 0 ) {
		xprintf("MIDI_PlayFile: open(\"%s\") returned %d.\n", filename, error );
		return FALSE;
	}


	if( (error = mciSendString("play MUSIC from 0 notify", NULL, 0, hwnd)) != 0 ) {
		xprintf("MIDI_PlayFile: play(\"%s\") returned %d.\n", filename, error );
		return FALSE;
	}

	xprintf("MIDI_PlayFile: playing \"%s\" file.\n", filename );

	return TRUE;
}




//
//
//
BOOL MIDI_Stop( void ) {

	int error;

	mciSendString("stop MUSIC",NULL,0,NULL);

	if( (error = mciSendString("close MUSIC", NULL, 0, NULL)) != 0 ) {
		xprintf("MIDI_Stop: close returned %d.\n", error );
		return FALSE;
	}

	return TRUE;
}




//
//
//
BOOL MIDI_Pause( void ) {

	// Pause if we're not already paused
	if( mciSendString("stop MUSIC", NULL, 0, NULL) != 0 )
		return FALSE ;

	return TRUE;
}




//
//
//
BOOL MIDI_Resume( void ) {

	// Resume midi
	if( mciSendString("play MUSIC notify", NULL, 0, hwnd) != 0 )
		return FALSE;

	return TRUE;
}




//
//
//
BOOL MIDI_Restart( void ) {

	// Replay midi
	if( mciSendString("play MUSIC from 0 notify", NULL, 0, hwnd) != 0 )
		return FALSE;

	return TRUE;
}




/***
//
// 0 - 1000
//
BOOL MIDI_SetVolume( int volume ) {

	char buffer[256];
	BOOL error;

	CLAMPMINMAX( volume, 0, 1000 );

	sprintf( buffer, "setaudio MUSIC volume to %d", volume );
        error = mciSendString(buffer, NULL, 0, hwnd);

	xprintf("MIDI_SetVolume: volume set to %d (%d).\n", volume, error );

        return error;
}
***/


//
//
//
int MIDI_GetLength( void ) {

	char retstr[200+1];
	long length;

	//                                    milliseconds
	mciSendStringA("set MUSIC time format milliseconds", retstr, 200, NULL);
	mciSendStringA("status MUSIC length", retstr, 200, NULL);
	length = atol(retstr);

	return length/1000;
}


//
//
//
int MIDI_GetPos( void ) {

	char retstr[200+1];
	long length;

	//                                    milliseconds
	mciSendStringA("set MUSIC time format milliseconds", retstr, 200, NULL);
	mciSendStringA("status MUSIC length", retstr, 200, NULL);
	length = atol(retstr);

	if( mciSendStringA("status MUSIC position", retstr, 200, NULL) == 0 ) {
		const long pos = atol(retstr);
		if( pos >= length ) {
			mciSendStringA("stop MUSIC", retstr, 200, NULL);  // We must stop ourselves
			mciSendStringA("close MUSIC", retstr, 200, NULL);
		}
		else
			return pos/1000;
	}

	return -1;
}



//
//
//
void MIDI_SetVolume( int left, int right ) {

	left = ftoi( (FLOAT)left * 0xffff / MAXVOL );

	if( right == -1 )
		right = left;

	// prissis short version
	long vol = (left<<0) | (right<<16);

	if( midiOutGetNumDevs() == 0 )
		return;

	midiOutSetVolume( 0, vol );

	xprintf("MIDI_SetVolume: volume set to %d (%d).\n", left, right );

	return;
}



//
// max = 0xFFFF
//
int MIDI_GetVolume( void ) {

	int OldMIDIVol[2] = {-1, -1};

	if( midiOutGetNumDevs() == 0 )
		return 0;

	DWORD old_volume;
	midiOutGetVolume( 0, &old_volume );
	OldMIDIVol[0] = old_volume>>16;
	OldMIDIVol[1] = old_volume&0xFFFF;

	return ftoi( (FLOAT)((OldMIDIVol[0]+OldMIDIVol[1])/2)*MAXVOL/0xffff);
}



//
//
//
BOOL MIDI_IsPlaying( void ) {

	if( MIDI_GetPos() > 0 )
		return TRUE;

	return FALSE;
}



//
//
//
void MIDI_PlayRandom( void ) {

	char dirname[XMAX_PATH];
	char *dlldir = "data";
	int i,num = 0;
	findfile_t data;

	getcwd( dirname, XMAX_PATH );

	chdir( dlldir );

	if( FindFirst( "*.mid", &data ) ) {

		do {
			++num;
		} while( FindNext( &data ) == TRUE );

		MyFindClose();

		srand( time(NULL) );
		num = rand() % num;

		i = 0;

		if( FindFirst( "*.mid", &data ) ) {

			while( i != num ) {

				FindNext( &data );
				++i;
			}

			MyFindClose();

			MIDI_PlayFile( data.name );
		}
	}

	chdir( dirname );

	return;
}




//
//
//
void valamiMixer( void ) {

	HMIXER hMixer;

	MMRESULT mmResult = mixerOpen(&hMixer, 0, (DWORD_PTR)GetHwnd(), 0, CALLBACK_WINDOW | MIXER_OBJECTF_MIXER);

	if( MMSYSERR_NOERROR == mmResult ) {
		MIXERLINE mxl;
		mxl.cbStruct = sizeof(MIXERLINE);
		mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
		MMRESULT mmResult = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE);
		if( mmResult == MMSYSERR_NOERROR ) {
			MIXERLINECONTROLS controls;
			memset(&controls, 0L, sizeof(controls));
			controls.cbStruct = sizeof(controls);
			controls.cControls = 1;
			controls.dwLineID = mxl.dwLineID;
			controls.cbmxctrl = sizeof(MIXERCONTROL); //one element only!!
			MIXERCONTROL controlsArray[2]; //First element: volume, second element: mixer
			memset(&controlsArray, 0L, sizeof(controlsArray));
			for (int i = 0; i < sizeof(controlsArray) / sizeof(*controlsArray); i++) {
				controlsArray[0].cbStruct = sizeof(controlsArray[0]);
			}

			controls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			controls.pamxctrl = &controlsArray[0]; //NOTE: this is ZERO
			mmResult = mixerGetLineControls((HMIXEROBJ)hMixer, &controls, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER);
			if( mmResult == MMSYSERR_NOERROR ) {
				controls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
				controls.pamxctrl = &controlsArray[1]; //NOTE: this is ONE
				mmResult = mixerGetLineControls((HMIXEROBJ)hMixer, &controls, MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER);
				if( mmResult == MMSYSERR_NOERROR ) {
					BOOL isVolume = FALSE; // controls[0].dwControlID == (DWORD)lParam;
					BOOL isMute = FALSE; // controls[1].dwControlID == (DWORD)lParam;
					if( isVolume | isMute ) {
						MIXERCONTROLDETAILS details;
						memset(&details, 0L, sizeof(details));
						details.cbStruct = sizeof(details);
						details.cChannels = 1;
						details.dwControlID = FALSE; // (DWORD)lParam;
						MIXERCONTROLDETAILS_UNSIGNED controlDetail;
						memset(&controlDetail, 0L, sizeof(controlDetail));
						details.paDetails = &controlDetail;
						details.cbDetails = sizeof(controlDetail);
						MMRESULT mmResult = mixerGetControlDetails((HMIXEROBJ)hMixer, &details, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
						if( mmResult == MMSYSERR_NOERROR ) {
						}
					}
				}
			}
		}
	}

	return;
}



//
//
//
void checkMixer( BOOL flag ) {

	MMRESULT result;
	HMIXER hMixer;
	result = mixerOpen(&hMixer, MIXER_OBJECTF_MIXER, 0, 0, 0);

	MIXERLINE ml = {0};
	ml.cbStruct = sizeof(MIXERLINE);
	ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	result = mixerGetLineInfo( (HMIXEROBJ)hMixer,  &ml, MIXER_GETLINEINFOF_COMPONENTTYPE );

	MIXERLINECONTROLS mlc = {0};
	MIXERCONTROL mc = {0};
	mlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mlc.dwLineID = ml.dwLineID;
	mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlc.cControls = 1;
	mlc.pamxctrl = &mc;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	result = mixerGetLineControls( (HMIXEROBJ)hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE );

	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_UNSIGNED mcdu = {0};
	mcdu.dwValue = 18500; // the volume is a number between 0 and 65535

	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.hwndOwner = 0;
	mcd.dwControlID = mc.dwControlID;
	mcd.paDetails = &mcdu;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.cChannels = 1;
	result = mixerGetControlDetails( (HMIXEROBJ)hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);

	int vol = ftoi((FLOAT)mcdu.dwValue*100.0f/65535.0f);

	// xprintf("checkMixer: master volume = %d%%\n",vol);

	// set master volume
	if( abs(vol - 50) > 1 && flag == TRUE ) {
		MIXERCONTROLDETAILS mcd = {0};
		MIXERCONTROLDETAILS_UNSIGNED mcdu = {0};
		mcdu.dwValue = 65536 / 2; // the volume is a number between 0 and 65535

		mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mcd.hwndOwner = 0;
		mcd.dwControlID = mc.dwControlID;
		mcd.paDetails = &mcdu;
		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
		mcd.cChannels = 1;

		xprintf("checkMixer: setting master volume to = %d%%.\n",mcdu.dwValue * 100 / 65535 );

		result = mixerSetControlDetails( (HMIXEROBJ)hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
	}

	// mute master volume
	if( 0 ) {
		mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		result = mixerGetLineControls( (HMIXEROBJ)hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

		MIXERCONTROLDETAILS_BOOLEAN mcb = {0};
		mcb.fValue    = TRUE;
		mcd.paDetails = &mcb;
		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		result = mixerSetControlDetails( (HMIXEROBJ)hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE );
	}

	mixerClose( hMixer );

	DWORD wav;
	if( waveOutGetVolume( 0, &wav ) == MMSYSERR_NOERROR ) {

		int left = wav & 0xffff;
		int right = (wav >> 16) & 0xffff;

		int vol = (int)((FLOAT)((left + right) / 2)*100.0f/(FLOAT)0xffff);

		// xprintf("checkMixer: wave = %d%%\n", vol);

		if( abs(vol - 50) > 1 && flag == TRUE ) {

			wav = ((0xffff >> 1) << 16) | (0xffff >> 1);

			left = wav & 0xffff;
			right = (wav >> 16) & 0xffff;

			vol = (int)((FLOAT)((left + right) / 2)*100.0f/(FLOAT)0xffff);

			xprintf("checkMixer: setting wave to = %d%%.\n", vol );

			if( waveOutSetVolume( 0, wav ) != MMSYSERR_NOERROR )
				xprintf("checkMixer: waveOutSetVolume phailed.\n");
		}
	}
	else
		xprintf("checkMixer: waveOutGetVolume phailed.\n");

	// MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT
	// bOK = GetMixerControl(hMixer, MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, wavCtrl, 1)

	return;
}


/***
void PauseWinamp( bool bMute )
{
    HWND hWndWinamp = NULL;
    hWndWinamp = ::FindWindow ( "Winamp v1.x", NULL );

    if( hWndWinamp )
    {
        char csWinampText[XMAX_PATH];

        ::GetWindowText( hWndWinamp, csWinampText, XMAX_PATH );

        if( bMute )
        {
            if( -1 == csWinampText.Find( "[Stopped]" ) &&
                -1 == csWinampText.Find( "[Paused]" ) &&
                -1 != csWinampText.Find( "- Winamp" ))
            {
                ::SendMessage( hWndWinamp, WM_KEYDOWN, ( WPARAM )0x43, 0 );

                m_bWasWinampMutedByUs = true;
            }
        }
        else
        {
            if( m_bWasWinampMutedByUs &&
                ( -1 != csWinampText.Find( "[Paused]" )))
            {
                ::SendMessage( hWndWinamp, WM_KEYDOWN, ( WPARAM )0x43, 0 );
            }

            m_bWasWinampMutedByUs = false;
        }
    }
}
***/


/***

static int OldMIDIVol[2] = {-1, -1};


#define __MIDI_VOL_SIMU    1  // 0-255
#define __MIDI_VOL_WIN32   2  // 0-65535


void __win32_set_midi_volume(int type, int left, int right);



// sets midi playback volume
void dr_set_midi_volume(int vol)
{
	__win32_set_midi_volume(__MIDI_VOL_SIMU, vol, vol);
}




// Midi shutdown/cleanup
void dr_destroy_midi(void)
{
	__win32_set_midi_volume(__MIDI_VOL_WIN32, OldMIDIVol[0], OldMIDIVol[1]);
}


// MIDI initialisation routines
bool dr_init_midi(void)
{
 #ifdef MIXER_VOLUME
	UINT nMIDIDevices;
	MIXERLINECONTROLS mlc;
	MIXERCONTROL MixControl;
	MIXERCONTROLDETAILS MixControlDetails;
	MIXERCONTROLDETAILS_UNSIGNED MixControlDetailsUnsigned[2];
	MIXERLINE MixerLine;
	HMIXER hMixer;
	MIXERCAPS DevCaps;

	// Reset MIDI volume

	nMIDIDevices = midiOutGetNumDevs();

	if (nMIDIDevices == 0)
		return;

	mixerOpen(&hMixer, 0, 0, 0, MIXER_OBJECTF_MIDIOUT);
	mixerGetDevCaps((UINT) hMixer, &DevCaps, sizeof(DevCaps));
	mixerClose(hMixer);

	MixerLine.cbStruct = sizeof(MixerLine);
	MixerLine.Target.dwType = MIXERLINE_TARGETTYPE_MIDIOUT;
	MixerLine.Target.wMid = DevCaps.wMid;
	MixerLine.Target.wPid = DevCaps.wPid;
	MixerLine.Target.vDriverVersion = DevCaps.vDriverVersion;
	strcpy(MixerLine.Target.szPname, DevCaps.szPname);

	mixerGetLineInfo(0, &MixerLine, MIXER_GETLINEINFOF_TARGETTYPE | MIXER_OBJECTF_MIDIOUT);

	mlc.cbStruct = sizeof(mlc);
	mlc.dwLineID = MixerLine.dwLineID;
	mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlc.cControls = 1;
	mlc.cbmxctrl = sizeof(MixControl);
	mlc.pamxctrl = &MixControl;

	MixControl.cbStruct = sizeof(MixControl);
	MixControl.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;

	mixerGetLineControls(0, &mlc, MIXER_OBJECTF_MIDIOUT | MIXER_GETLINECONTROLSF_ONEBYTYPE);

	MixControlDetails.cbStruct = sizeof(MixControlDetails);
	MixControlDetails.dwControlID = MixControl.dwControlID;
	MixControlDetails.cChannels = MixerLine.cChannels;
	MixControlDetails.hwndOwner = NULL;
	MixControlDetails.cMultipleItems = 0;
	MixControlDetails.cbDetails = sizeof(MixControlDetailsUnsigned[0])*2;
	MixControlDetails.paDetails = &MixControlDetailsUnsigned[0];

	mixerGetControlDetails(0, &MixControlDetails, MIXER_OBJECTF_MIDIOUT | MIXER_GETCONTROLDETAILSF_VALUE);

	OldMIDIVol[0] = MixControlDetailsUnsigned[0].dwValue;  // Save the old volume
	OldMIDIVol[1] = MixControlDetailsUnsigned[0].dwValue;

	sound_set_midi_volume_var(OldMIDIVol[0] >> 8); // Set the MIDI volume

	mixerSetControlDetails(0, &MixControlDetails, MIXER_OBJECTF_MIDIOUT | MIXER_SETCONTROLDETAILSF_VALUE);
#else
	if( midiOutGetNumDevs()== 0 ) {
		return false;
	}
	DWORD old_volume;
	midiOutGetVolume( 0, &old_volume );
	OldMIDIVol[0] = old_volume>>24;
	OldMIDIVol[1] = (old_volume&0x0000FF00)>>8;
#endif
	// Hajo: assuming if we got here, all is set up to work properly
	return true;
}



#ifdef MIXER_VOLUME
// Sets the MIDI volume - internal routine
void __win32_set_midi_volume(int type, int left, int right)
{
	UINT nMIDIDevices;
	MIXERLINECONTROLS mlc;
	MIXERCONTROL MixControl;
	MIXERCONTROLDETAILS MixControlDetails;
	MIXERCONTROLDETAILS_UNSIGNED MixControlDetailsUnsigned[2];
	MIXERLINE MixerLine;
	HMIXER hMixer;
	MIXERCAPS DevCaps;

	nMIDIDevices = midiOutGetNumDevs();
	if (nMIDIDevices == 0) {
		return;
	}

	mixerOpen(&hMixer, 0, 0, 0, MIXER_OBJECTF_MIDIOUT);
	mixerGetDevCaps((UINT) hMixer, &DevCaps, sizeof(DevCaps));
	mixerClose(hMixer);

	MixerLine.cbStruct = sizeof(MixerLine);
	MixerLine.Target.dwType = MIXERLINE_TARGETTYPE_MIDIOUT;
	MixerLine.Target.wMid = DevCaps.wMid;
	MixerLine.Target.wPid = DevCaps.wPid;
	MixerLine.Target.vDriverVersion = DevCaps.vDriverVersion;
	strcpy(MixerLine.Target.szPname, DevCaps.szPname);

	mixerGetLineInfo(0, &MixerLine, MIXER_GETLINEINFOF_TARGETTYPE | MIXER_OBJECTF_MIDIOUT);

	mlc.cbStruct = sizeof(mlc);
	mlc.dwLineID = MixerLine.dwLineID;
	mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlc.cControls = 1;
	mlc.cbmxctrl = sizeof(MixControl);
	mlc.pamxctrl = &MixControl;

	MixControl.cbStruct = sizeof(MixControl);
	MixControl.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;

	mixerGetLineControls(0, &mlc, MIXER_OBJECTF_MIDIOUT | MIXER_GETLINECONTROLSF_ONEBYTYPE);

	MixControlDetails.cbStruct = sizeof(MixControlDetails);
	MixControlDetails.dwControlID = MixControl.dwControlID;
	MixControlDetails.cChannels = MixerLine.cChannels;
	MixControlDetails.hwndOwner = NULL;
	MixControlDetails.cMultipleItems = 0;
	MixControlDetails.cbDetails = sizeof(MixControlDetailsUnsigned[0])*2;
	MixControlDetails.paDetails = &MixControlDetailsUnsigned[0];

	mixerGetControlDetails(0, &MixControlDetails, MIXER_OBJECTF_MIDIOUT | MIXER_GETCONTROLDETAILSF_VALUE);

	if (type == __MIDI_VOL_SIMU)
	{
		MixControlDetailsUnsigned[0].dwValue = (left << 8);
		MixControlDetailsUnsigned[1].dwValue = (right << 8);
	}
	else
	{
		MixControlDetailsUnsigned[0].dwValue = left;
		MixControlDetailsUnsigned[1].dwValue = right;
	}

	mixerSetControlDetails(0, &MixControlDetails, MIXER_OBJECTF_MIDIOUT | MIXER_SETCONTROLDETAILSF_VALUE);

	// Phew, I'm glad that's over! What a horrible API...
}
#else
// Sets the MIDI volume - internal routine
void __win32_set_midi_volume(int , int left, int right)
{
	// prissis short version
	long vol = (left<<24)|(right<<8);

	if( midiOutGetNumDevs()== 0 ) {
		return;
	}
	midiOutSetVolume( 0, vol );
}
#endif

***/