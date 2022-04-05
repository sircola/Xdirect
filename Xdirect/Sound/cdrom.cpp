
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <direct.h>


#include <xlib.h>

RCSID( "$Id: cdrom.cpp,v 1.2 2003/09/22 14:00:01 bernie Exp $" )


#define MINE ( 0xdeadbeef )


int StartTrack = 0,EndTrack = 0;
static UINT wDeviceID = MINE;


//
//
//
void CD_Eject( void ) {
	
	DWORD dwReturn;

	if( wDeviceID == MINE )
		if( InitCdrom() == FALSE )
			return;

    if( (dwReturn = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD)NULL)) )
		xprintf("CD_Eject failed (%i).\n", dwReturn);

	DeinitCdrom();

	return;
}



//
//
//
void CD_Close( void ) {
	
	DWORD dwReturn;

    if( (dwReturn = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD)NULL)) )
		xprintf("CD_Close failed (%i).\n", dwReturn);

	WaitTimer( TICKBASE*3 );

	return;
}




//
//
//
int CD_GetInfo( void ) {

	DWORD				dwReturn;
	MCI_STATUS_PARMS	mciStatusParms;
	int first = 0;

reget_cd:

	mciStatusParms.dwItem = MCI_STATUS_READY;
    dwReturn = mciSendCommand( wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD) (LPVOID) &mciStatusParms );
	
	if( dwReturn ) {
		xprintf("CD_GetInfo: drive ready test - get status failed\n");
		return -1;
	}

	if( !mciStatusParms.dwReturn ) {
		if( first++ < 2 ) {
			CD_Close();
			goto reget_cd;
		}
		xprintf("CD_GetInfo: drive not ready\n");
		return -1;
	}

	mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD) (LPVOID) &mciStatusParms);
	
	if( dwReturn ) {
		xprintf("CD_GetInfo: get tracks - status failed\n");
		return -1;
	}
	
	if( mciStatusParms.dwReturn < 1 ) {
		xprintf("CD_GetInfo: no music tracks\n");
		return -1;
	}

	StartTrack = 1;
	EndTrack = mciStatusParms.dwReturn;

	return 0;
}




//
//
//
static BOOL PlayReal( int track, int endtrack ) {

	char postxt[16];
	char command[256];
	MCIERROR error;

	UnlockSurface();

	if( track < 1 || track > EndTrack ) {
		xprintf("CDAudio: Bad track number %u.\n", track);
		return FALSE;
	}

	mciSendString("stop cdaudio",NULL,0,0);

	sprintf(command,"status cdaudio position track %d",track);
	error = mciSendString( command, postxt, 16, 0 );

	if( !error ) {
		sprintf(command,"play cdaudio from %s",postxt);
		error = mciSendString( command, NULL, 0, 0 );
	}

	return !error;
}




//
//
//
static BOOL StopReal( void ) {

	MCIERROR error;

	UnlockSurface();

	error = mciSendString("stop cdaudio",NULL,0,0);

	return !error;
}





/*
 * CD-ROM general Interface
 *
 */

#define CDC_NONE 0
#define CDC_PLAY 1
#define CDC_LOOP 2
#define CDC_RAND 4


static int From=0, To=0, Actual=0;
static int PlayMode = CDC_NONE;





//
//
//
BOOL InitCdrom( void ) {

	DWORD	dwReturn;
	MCI_OPEN_PARMS	mciOpenParms;
    MCI_SET_PARMS	mciSetParms;

	mciOpenParms.lpstrDeviceType = "cdaudio";
	if( (dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE, (DWORD) (LPVOID) &mciOpenParms)) ) {
		xprintf("CDAudio_Init: MCI_OPEN failed (%i)\n", dwReturn);
		return FALSE;
	}
	
	wDeviceID = mciOpenParms.wDeviceID;

    // Set the time format to track/minute/second/frame (TMSF).
    mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
    if( (dwReturn = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &mciSetParms)) ) {
		xprintf("MCI_SET_TIME_FORMAT failed (%i)\n", dwReturn);
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, (DWORD)NULL);
		return FALSE;
    }

	PlayMode = CDC_NONE;

	CD_GetInfo();

	return TRUE;
}




//
//
//
BOOL DeinitCdrom( void ) {

	StopReal();

	if( wDeviceID == MINE )
		return TRUE;

	CD_Stop();

	if( mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD)NULL) )
		xprintf("CDAudio_Shutdown: MCI_CLOSE failed\n");

	wDeviceID = MINE;

	return TRUE;
}




//
//
//
void CD_PlayRange( int track, int endtrack ) {

	PlayMode |= CDC_PLAY;

	Actual = From = track;
	To = endtrack;

	PlayReal( From, To );

	return;
}



//
//
//
void CD_PlayTrack( int track ) {

	if( wDeviceID == MINE )
		if( InitCdrom() == FALSE )
			return;

	PlayMode |= CDC_PLAY;

	Actual = From = track;
	To = EndTrack;

	PlayReal( From, To );

	return;
}



//
//
//
void CD_PlayAll( void ) {

	PlayMode |= CDC_PLAY;

	Actual = From = StartTrack;
	To = EndTrack;

	PlayReal( From, To );

	return;
}





//
//
//
void CD_Stop( void ) {

	if( PlayMode & CDC_PLAY ) {

		PlayMode &= ~CDC_PLAY;

		StopReal();
	}

	return;
}





//
//
//
void CD_NextTrack( void ) {

	if( (PlayMode & CDC_PLAY) && (Actual < EndTrack) ) {

		Actual++;

		PlayReal( Actual, To );
	}

	return;
}




//
//
//
void CD_PreviousTrack( void ) {

	if( (PlayMode & CDC_PLAY) && (Actual > StartTrack) ) {

		Actual--;

		PlayReal( Actual, To );
	}

	return;
}





/***
//
//Check for presence of cd
//
bool CMusCDAudio::IsCDInserted()
{
	DWORD dwRet=0;
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwItem = MCI_STATUS_MEDIA_PRESENT;

    if (dwRet = mciSendCommand(m_mciDevice, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms)) 
    {
		PrintErrorMessage(dwRet,"CMusCDAudio::IsCDInserted:");
		return false;
    }
	if(mciStatusParms.dwReturn == FALSE)
	{
		m_cdInserted = false;
		m_numTracks = 0;
		m_curTrack = 0;
		m_curPos = 0;
		return false;
	}
	m_cdInserted = true;
	return true;
}
***/






