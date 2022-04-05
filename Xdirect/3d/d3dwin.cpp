
#ifndef XLIB
#define XLIB
#endif

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <ddraw.h>
#include <dinput.h>
#include <d3d.h>


#include <xlib.h>

#include "xd3d.h"

#include "../resource.h"



RCSID( "$Id: d3dwin.cpp,v 1.2 2003/09/22 13:59:59 bernie Exp $" )



static D3DDEVICEDESC mydesc[MAX_DESC];
D3DDEVICEDESC *global_mydesc = NULL;
static int ndescs = 0;

static DDCAPS ddcaps[MAX_DESC];
DDCAPS *global_ddcaps = NULL;
static int nddcaps = 0;

static dd_info_t dd_info[MAX_DESC];
DDCAPS *global_dd_info = NULL;
static int nddinfos = 0;




static combobox_t *combobox_data;

static HWND hwndcombo = NULL, choosehwnd = NULL, edithwnd = NULL;


#define YESNO( value ) (((value))?"Yes":"No")

//
//
//
static void PrintCaps( int sel, BOOL log ) {

	// string_t<512> str;
	char str[1024];

	if( !log ) D3D_ClearLogWindow();

	if( log ) dprintf("PrintCaps: -------------------\n");

	sprintf(str,"Memory: %.2f Mb\n", (FLOAT)ddcaps[sel].dwVidMemTotal/(1024.0f*1024.0f) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"3d hardware: %s\n", YESNO(ddcaps[sel].dwCaps & DDCAPS_3D) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"Bankswitched: %s\n", YESNO(ddcaps[sel].dwCaps & DDCAPS_BANKSWITCHED) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"Colorkey: %s\n", YESNO(ddcaps[sel].dwCKeyCaps & DDCKEYCAPS_SRCBLT ) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"Z buffer: %s\n", YESNO(ddcaps[sel].ddsCaps.dwCaps & DDSCAPS_ZBUFFER) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"Overlay: %s\n", YESNO(ddcaps[sel].ddsCaps.dwCaps & DDCAPS_OVERLAY) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"DrawPrimitive: %s\n", YESNO(mydesc[sel].dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);
/*
	sprintf(str,"HW acceleration: %s\n", YESNO(mydesc[sel].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	sprintf(str,"HW T&&L: %s\n", YESNO(mydesc[sel].dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);
*/

	sprintf(str,"Texturing: %s (from %dx%d to %dx%d)\n", YESNO(ddcaps[sel].ddsCaps.dwCaps & DDSCAPS_TEXTURE), mydesc[sel].dwMinTextureWidth, mydesc[sel].dwMinTextureHeight, mydesc[sel].dwMaxTextureWidth, mydesc[sel].dwMaxTextureHeight );

	if( mydesc[sel].dwMaxTextureHeight == 0 || mydesc[sel].dwMaxTextureWidth == 0 )
		strcat( str, "S3 Virge drivers sometimes lies about max texture width or height.\n");

	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);


	if( mydesc[sel].dwFlags & D3DDD_TRICAPS ) {

		sprintf(str,"Non square texture support: %s\n",YESNO(mydesc[sel].dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY));
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Gouraud shading: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Fog: ");

		if( mydesc[sel].dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE )
			strcat(str, "fogtable ");

		if( mydesc[sel].dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE )
			strcat(str, "fogrange ");

		if( mydesc[sel].dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX )
			strcat(str, "fogvertex ");

		strcat(str,"\n");
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Source blend mode SRCALPHA: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Source blend mode ONE: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ONE) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Source blend mode ZERO: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Destination blend mode INVSRCALPHA: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Destination blend mode ONE: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);

		sprintf(str,"Destination blend mode SRCCOLOR: %s\n", YESNO(mydesc[sel].dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR) );
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);
	}
	else {
		sprintf(str,"No TriCaps!\n");
		if( !log ) D3D_AddLogWindow(str);
		if( log ) dprintf(str);
	}

	sprintf(str,"MS certified: %s\n", YESNO(ddcaps[sel].dwCaps2 & DDCAPS2_CERTIFIED) );
	if( !log ) D3D_AddLogWindow(str);
	if( log ) dprintf(str);

	if( edithwnd )
		SendMessage( edithwnd, EM_LINESCROLL, 0, -SendMessage( edithwnd, EM_GETLINECOUNT, 0, 0 ) );

	if( log == FALSE ) {
		global_mydesc = &mydesc[sel];
		global_ddcaps = &ddcaps[sel];
	}

	if( log ) dprintf("---------------------\n");

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

					if( SendDlgItemMessage( choosehwnd, IDC_RADIO1, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) == BST_CHECKED )
						strcpy( d3dName, "HAL" );
					else
					if( SendDlgItemMessage( choosehwnd, IDC_RADIO2, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) == BST_CHECKED )
						strcpy( d3dName, "RGB" );
					else
					if( SendDlgItemMessage( choosehwnd, IDC_RADIO3, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) == BST_CHECKED )
						strcpy( d3dName, "Ref" );

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
int D3D_winComboBox( combobox_t *cb_data ) {

	combobox_data = cb_data;

	if( combobox_data->nstrs == 0 ) {
		combobox_data->cursel = -1;
		return combobox_data->cursel;
	}

	// ha csak egy device van
	if( combobox_data->nstrs < 2 ) {
		combobox_data->cursel = 0;
		choosehwnd = NULL;
		dprintf("D3D_winComboBox: %s\n", combobox_data->str[combobox_data->cursel][0] );
		PrintCaps( combobox_data->cursel, TRUE );
		global_mydesc = &mydesc[combobox_data->cursel];
		global_ddcaps = &ddcaps[combobox_data->cursel];
		return combobox_data->cursel;
	}

	if( (choosehwnd = CreateDialog( hw_state.hInst, MAKEINTRESOURCE(IDD_D3D), NULL, (DLGPROC)MyDlgProc)) == NULL )
		return (-1);

	SetWindowText( choosehwnd, "Direct3D devices" );
	ShowWindow( choosehwnd, SW_NORMAL );

	SendMessage( hwndcombo, CB_SETCURSEL, combobox_data->cursel, 0 );
	SetFocus( hwndcombo );

	// drive checkbox

	SendDlgItemMessage( choosehwnd, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, (LPARAM)0 );
	SendDlgItemMessage( choosehwnd, IDC_RADIO2, BM_SETCHECK, BST_UNCHECKED, (LPARAM)0 );
	SendDlgItemMessage( choosehwnd, IDC_RADIO3, BM_SETCHECK, BST_UNCHECKED, (LPARAM)0 );

	EnableWindow(GetDlgItem(choosehwnd, IDC_RADIO2), FALSE);
	EnableWindow(GetDlgItem(choosehwnd, IDC_RADIO3), FALSE);

	// edit field

	SendDlgItemMessage( choosehwnd, IDC_EDIT1, EM_FMTLINES, TRUE, (LPARAM)0 );

	edithwnd = GetDlgItem( choosehwnd, IDC_EDIT1 );

	PrintCaps( combobox_data->cursel, FALSE );

	while( choosehwnd != NULL ) {

		MSG msg;

		if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
			if( !IsDialogMessage( choosehwnd, &msg ) ) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	if( choosehwnd ) DestroyWindow( choosehwnd );
	choosehwnd = NULL;

	dprintf("log: %s\n", combobox_data->str[combobox_data->cursel][0] );
	PrintCaps( combobox_data->cursel, TRUE );

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
void D3D_AddLogWindow( char *str ) {

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
void D3D_ClearLogWindow( void ) {

	if( edithwnd == NULL )
		return;

	Edit_SetSel( edithwnd, 0, INT_MAX );
	Edit_ReplaceSel( edithwnd, "" );
	Edit_SetSel( edithwnd, INT_MAX, INT_MAX );

	return;
}




//
//
//
DDCAPS *D3D_GetCaps( void ) {

	return &ddcaps[ nddcaps++ ];
}


//
//
//
D3DDEVICEDESC *D3D_GetDesc( void ) {

	return &mydesc[ ndescs++ ];
}



//
//
//
dd_info_t *D3D_GetDdinfo( void ) {

	return &dd_info[ nddinfos++ ];
}

