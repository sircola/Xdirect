
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include <glide.h>

#include <xlib.h>

#include "xglide.h"

#include "../resource.h"



RCSID( "$Id: Glidewin.cpp,v 1.1.1.1 2003/08/19 17:44:50 bernie Exp $" )


static combobox_t *combobox_data;

static HWND hwndcombo = NULL, choosehwnd = NULL, edithwnd = NULL;




//
//
//
static void PrintCaps( int sel, BOOL log ) {

	int numfb,fbram,numtmu,tmuram,revtmu;
	// string_t<512> str;
	char str[512];

	Glide_ClearLogWindow();

	sel = atoi( combobox_data->str[ sel ][1] );

	pGrSstSelect( sel );

	pGrGet( GR_MEMORY_FB, sizeof(fbram), (long*)&fbram );
	pGrGet( GR_NUM_FB, sizeof(numfb), (long*)&numfb );
	pGrGet( GR_MEMORY_TMU, sizeof(tmuram), (long*)&tmuram );
	pGrGet( GR_NUM_TMU, sizeof(numtmu), (long*)&numtmu );
	pGrGet( GR_REVISION_TMU, sizeof(revtmu), (long*)&revtmu );

	sprintf(str,"frame buffer: %dKb\n", (numfb*fbram)/1024 );
	Glide_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"TMU: %d, revision %d\n", numtmu, revtmu );
	Glide_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"texture ram: %dKb\n", (numtmu*tmuram)/1024 );
	Glide_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"extensions: %s\n", pGrGetString(GR_EXTENSION) );
	Glide_AddLogWindow(str);
	if( log ) dprintf(str);

	if( edithwnd )
		SendMessage( edithwnd, EM_LINESCROLL, 0, -SendMessage( edithwnd, EM_GETLINECOUNT, 0, 0 ) );

	return;
}





//
//
//
BOOL CALLBACK MyDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch( msg ) {

		case WM_KEYDOWN:


			break;


		case WM_COMMAND:

			switch( LOWORD(wParam) ) {

				case IDC_COMBO1:

					if( HIWORD(wParam) == CBN_SELCHANGE ) {

						int sel;

						sel = SendDlgItemMessage( choosehwnd, IDC_COMBO1, CB_GETCURSEL, (WPARAM)0, (LPARAM)0 );

						PrintCaps( sel, FALSE );
					}

					break;



				case IDOK:

					combobox_data->cursel = SendMessage( hwndcombo, CB_GETCURSEL, 0, 0 );

					DestroyWindow( hwnd );

					choosehwnd = NULL;

					return TRUE;

				case IDCANCEL:

					combobox_data->cursel = -1;

					DestroyWindow( hwnd );

					choosehwnd = NULL;

					return TRUE;
			}
			break;


		case WM_INITDIALOG:

			int i;

			hwndcombo = GetDlgItem( hwnd, IDC_COMBO1 );

			for( i=0; i<combobox_data->nstrs; i++ )
				SendMessage( hwndcombo, CB_ADDSTRING, 0, (DWORD)combobox_data->str[i][0] );

			SendMessage( hwndcombo, CB_SETCURSEL, combobox_data->cursel, 0 );

			break;

		case WM_DESTROY:

			choosehwnd = NULL;

			break;
	}

	return FALSE;
}







//
//
//
int Glide_winComboBox( combobox_t *cb_data ) {

	int i;

	combobox_data = cb_data;

	if( combobox_data->nstrs == 0 ) {
		combobox_data->cursel = -1;
		return combobox_data->cursel;
	}

	// log
	for( i=0; i<combobox_data->nstrs; i++ ) {
		dprintf("log: %s\n", combobox_data->str );
		PrintCaps( i, TRUE );
	}

	if( combobox_data->nstrs == 1 ) {
		combobox_data->cursel = 0;
		return combobox_data->cursel;
	}

	if( (choosehwnd = CreateDialog( (HINSTANCE)hw_state.hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)MyDlgProc)) == NULL )
		return (-1);

	SetWindowText( choosehwnd, "Glide devices" );
	ShowWindow( choosehwnd, SW_NORMAL );

	SendMessage( hwndcombo, CB_SETCURSEL, combobox_data->cursel, 0 );
	SetFocus( hwndcombo );

	// drive checkbox

	ShowWindow( GetDlgItem( choosehwnd, IDC_FRAME ), SW_HIDE );
	ShowWindow( GetDlgItem( choosehwnd, IDC_RADIO1 ), SW_HIDE );
	ShowWindow( GetDlgItem( choosehwnd, IDC_RADIO2 ), SW_HIDE );
	ShowWindow( GetDlgItem( choosehwnd, IDC_RADIO3 ), SW_HIDE );

	// edit field

	SendDlgItemMessage( choosehwnd, IDC_EDIT1, EM_FMTLINES, TRUE, (LPARAM)0 );

	edithwnd = GetDlgItem( choosehwnd, IDC_EDIT1 );

	PrintCaps( combobox_data->cursel, FALSE );

	while( choosehwnd != NULL ) {

		MSG msg;

		//if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
		if( GetMessage(&msg, NULL, 0, 0) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	if( choosehwnd ) DestroyWindow( choosehwnd );
	choosehwnd = NULL;

	return combobox_data->cursel;
}







//
//
//
char *FixNewLines( char *lpcszText ) {

	char *lpszScratch;
	char *lpszNewLine;

	if( !lpcszText )
		return NULL;

	// allocate a buffer twice as big in case we need to
	// add any \r's to solo \n's

	lpszScratch = (char*)dmalloc( (strlen(lpcszText) * 2) + 1 );
	if( !lpszScratch )
		return NULL;

	/* make a copy of the passed text */
	strcpy(lpszScratch, lpcszText);

	/* replace \n\r with \r\n */
	lpszNewLine = strstr(lpszScratch, "\n\r");
	while ( lpszNewLine ) {
		*lpszNewLine = '\r';
		*(lpszNewLine + 1) = '\n';

		// skip our changes
		lpszNewLine += 2;

		lpszNewLine = strstr(lpszNewLine, "\n\r");
	}

	/* replace solo \n's with \r\n */
	lpszNewLine = lpszScratch;
	while( *lpszNewLine ) {

		if( *lpszNewLine == '\n' ) {
			/* handle the case of \n being the 1st char */
			if( (lpszNewLine == lpszScratch) ) {
				MoveMemory(lpszScratch + 1, lpszScratch, strlen(lpszScratch) + 1);
				*lpszScratch = '\r';
				lpszNewLine++;
			}
			else
			if( *(lpszNewLine - 1) != '\r' ) {
				MoveMemory(lpszNewLine + 1, lpszNewLine, strlen(lpszNewLine) + 1);
				*lpszNewLine = '\r';
				lpszNewLine++;
			}
		}

		lpszNewLine++;
	}

	return lpszScratch;
}





//
//
//
void Glide_AddLogWindow( char *str ) {

	if( edithwnd == NULL )
		return;

	char *lpszDebugText = FixNewLines( str );

	Edit_SetSel( edithwnd, INT_MAX, INT_MAX );
	Edit_ReplaceSel( edithwnd, lpszDebugText );
	Edit_SetSel( edithwnd, INT_MAX, INT_MAX );

	// it's our responsibility to delete the memory
	dfree(lpszDebugText);

	return;
}




//
//
//
void Glide_ClearLogWindow( void ) {

	if( edithwnd == NULL )
		return;

	Edit_SetSel( edithwnd, 0, INT_MAX );
	Edit_ReplaceSel( edithwnd, "" );
	Edit_SetSel( edithwnd, INT_MAX, INT_MAX );

	return;
}




