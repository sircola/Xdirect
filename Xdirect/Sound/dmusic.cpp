
#ifndef __MINGW32__
// mingw-be szarok a header fileok wip rofl

#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <stdio.h>
#include <dshow.h>
#include <tchar.h>

#include <xlib.h>


// #pragma comment( lib, "strmiids.lib" )
// #pragma comment( lib, "quartz.lib" )
// #pragma comment( lib, "strmbase.lib" )

static IGraphBuilder *pGraph = NULL;
static IMediaControl *pMediaControl = NULL;
static IGraphBuilder *pGraphBuilder = NULL;
static IMediaEventEx *pEvent = NULL;
static IMediaSeeking *pMediaSeeking = NULL;
static IBasicAudio   *pAudio = NULL;



//
//
//
BOOL DM_Init( void ) {

#define WM_GRAPHNOTIFY  WM_APP + 1

	if( pGraph )
		return TRUE;

	CoInitialize( NULL );

	if( FAILED(CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph)) ) {
		xprintf("DM_Init: no dmusic.\n" );
		return FALSE;
	}

	pGraph->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
	pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pMediaSeeking);
	pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
	pGraph->QueryInterface(IID_IGraphBuilder, (void **)&pGraphBuilder);
	pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pEvent);
	pEvent->SetNotifyWindow( (OAHWND)GetHwnd(), WM_GRAPHNOTIFY, 0);

	return TRUE;
}

 
//
//
//
void DM_Deinit( void ) {

	if( pMediaControl ) pMediaControl->Stop();

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

	SAFE_RELEASE(pGraphBuilder);
	SAFE_RELEASE(pAudio);
	SAFE_RELEASE(pMediaSeeking);
	SAFE_RELEASE(pMediaControl);
	SAFE_RELEASE(pEvent);
	SAFE_RELEASE(pGraph);

	CoUninitialize();

	return;
}



//
//
//
BOOL DM_Load( char *filename ) {

	HRESULT hr;

	if( !pMediaControl ) 
		return FALSE;

	xprintf("DM_Load: loading \"%s\" file...\n", filename );
	
	pMediaControl->Stop();
	
	// enumerat filters
	
	IEnumFilters *pFilterEnum = NULL;
	IBaseFilter  *pFilterTemp = NULL;
	
	hr = pGraphBuilder->EnumFilters(&pFilterEnum);
		
	int iFiltCount = 0;
	int iPos = 0;
	
	if( SUCCEEDED(hr) )  {
						
		while( S_OK == pFilterEnum->Skip(1) ) { iFiltCount++; }
						
		IBaseFilter **ppFilters = reinterpret_cast<IBaseFilter **>(_alloca( sizeof(IBaseFilter *) * iFiltCount) );
				
		pFilterEnum->Reset();

		while( S_OK == pFilterEnum->Next(1, &(ppFilters[iPos++]), NULL) );
		
		SAFE_RELEASE(pFilterEnum);

		for( iPos=0; iPos<iFiltCount; iPos++ ) {
							
			pGraphBuilder->RemoveFilter(ppFilters[iPos]);
			
			// Put the filter back, unless it is the old source
			// if (ppFilters[iPos] != g_pSourceCurrent) {
					//g_pGraphBuilder->AddFilter(ppFilters[iPos], NULL);
			
			SAFE_RELEASE(ppFilters[iPos]);
		}
	}
									
	WCHAR longfilename[MAX_PATH] = { 0 };
					
	MultiByteToWideChar( CP_ACP, 0, filename, -1, longfilename, MAX_PATH );
	
	pGraph->RenderFile(longfilename, NULL);
	
	return TRUE;
}


//
//
//
void DM_Pause( void ) {

	if( !pMediaControl ) 
		return;
	
	pMediaControl->Pause();

	return;
}



//
//
//
void DM_Play( void ) {

	if( !pMediaControl ) 
		return;
	
	pMediaControl->Run();

	return;
}
					


//
//
//
void DM_Stop( void ) {

	if( !pMediaControl ) 
		return;

	pMediaControl->Stop();
	
	LONGLONG llPos = 0;
	pMediaSeeking->SetPositions( &llPos, AM_SEEKING_AbsolutePositioning,&llPos, AM_SEEKING_NoPositioning );
			
	return;
}

	
#define VOLUME_FULL     0L
#define VOLUME_SILENCE  (-3000L)		// -10000L

//
//
//
void DM_SetVol( int vol ) {

	if( !pMediaControl ) 
		return;
				
	CLAMPMINMAX( vol, 0, MAXVOL );

	long volume;
	
	// pAudio->get_Volume(&volume);
	// xprintf("curr: %d\n", volume );

	volume = VOLUME_SILENCE + (long)((VOLUME_FULL - VOLUME_SILENCE) * vol / MAXVOL);
	// xprintf("next: %d\n", volume );

	pAudio->put_Volume( volume );

	return;
}

#endif
