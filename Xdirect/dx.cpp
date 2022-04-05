
//#define WIN32_LEAN_AND_MEAN
#define INITGUID

#define DIRECTINPUT_VERSION	0x0500
#define REQ_DIRECTDRAW_VERSION	0x0600

#include <windows.h>
#include <mmsystem.h>
// #include <multimon.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <direct.h>
#include <zmouse.h>
#include <dinput.h>
#include <io.h>
#include <shlobj.h>

#include <xlib.h>
#include <xinner.h>

// #include <DebugHlp.h>


#include "dderr.h"
#include "resource.h"
#include "resrc1.h"
#include "dppal.h"

RCSID( "$Id: dx.cpp,v 1.2 2003/09/22 13:59:57 bernie Exp $" )


int CLIPMINX;
int CLIPMINY;
int CLIPMAXX;
int CLIPMAXY;
int SCREENW;
int SCREENH;

__int64 gfxmem;


int bpp=0,pixel_len;
int rshift,gshift,bshift;
int rmask,gmask,bmask;
int rsize,gsize,bsize;

cvar_t zclipnear = {"zclipnear","0.0f"};
cvar_t zclipfar = {"zclipfar","25.0f"};
cvar_t gamma_corr = {"gamma_corr","100.0f"};

volatile int TimeCount;
volatile int didupdate;

BOOL display_zprintf = FALSE;
BOOL bZBuffer = FALSE;

static LPDIRECTINPUT DirectInput = NULL;
static LPDIRECTINPUTDEVICE KeyboardDevice = NULL;
static LPDIRECTINPUTDEVICE MouseDevice = NULL;
static int ntJoystickDevice = -1;

signed int joyx,joyy,joyz,joydx,joydy,joydz,joypov,maxjoyx,maxjoyy,maxjoyz;
BOOL joyb[32];
BOOL joybd[32];
static BOOL joyHasZ = FALSE;

static HHOOK hMouseHook = NULL;
static int win32_lastMouseX = 0, win32_mousecx = 0, win32_mousedx = 0;
static int win32_lastMouseY = 0, win32_mousecy = 0, win32_mousedy = 0, mouse_zgran = WHEEL_DELTA;
static BOOL win32_mousebl=FALSE,win32_mousebm=0,win32_mousebr=0,win32_mouseb4=FALSE,win32_mouseb5=FALSE;
static int lowlevel_mouseb4 = 0;
static int lowlevel_mouseb5 = 0;

#define VK_MAX 0x1FF
#define VK_XBUTTON1	5
#define VK_XBUTTON2	6
UCHAR *hardware_keys = NULL;
DWORD last_hardware_key = 0;

static BOOL blockmousemove = FALSE;
static FLOAT mousespeed_percent = 100.0f;

HWND hwnd = NULL;
HINSTANCE hInst = NULL;

#define MOUSECUR "mouse1.bmp"

#define DEVICEBUFFERSIZE 128

int active_flag = 1;	// 1: active  0: deactivate/background  -1: minimized

#define CHECKACTIVE	{ if( active_flag < 0 ) return; }
#define CHECKACTIVEN	{ if( active_flag < 0 ) return FALSE; }

// FIXME: találj ki jobbat
static int setx_flag = 0;
int winSetxFlag( void ) { return setx_flag; }

void (GFXDLLCALLCONV *GFXDLL_GetInfo)( GFXDLLinfo_t * ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_SetupCulling)( point3_t,point3_t, point3_t, FLOAT zn, FLOAT zf ) = NULL;
BOOL (GFXDLLCALLCONV *GFXDLL_AddToPipeline)( trans2pipeline_t ) = NULL;
int (GFXDLLCALLCONV *GFXDLL_AddTexMapHigh)( texmap_t *texmap ) = NULL;
int (GFXDLLCALLCONV *GFXDLL_ReloadTexMapHigh)( texmap_t *texmap ) = NULL;
int (GFXDLLCALLCONV *GFXDLL_ModifyTexMapHigh)( texmap_t *texmap ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_PutSpritePoly)( polygon_t ppoly, point3_t *inst, int texmapid, rgb_t *rgb ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_PutPoly)( polygon_t ppoly, rgb_t *rgb ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_DiscardAllTexture)( void ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_DiscardTexture)( int handler ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_Line)( int x0, int y0, int x1, int y1, rgb_t color ) = NULL; // DrawLineRgb
void (GFXDLLCALLCONV *GFXDLL_PutPixel)( int x, int y, rgb_t color ) = NULL;	// PutPixelRgb
void (GFXDLLCALLCONV *GFXDLL_BeginScene)( void ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_EndScene)( void ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_FlushScene)( void ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_GetDescription)( char * ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_GetData)( void *, int ) = NULL;
BOOL (GFXDLLCALLCONV *GFXDLL_LockLfb)( __int64 *, int ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_UnlockLfb)( void ) = NULL;
BOOL (GFXDLLCALLCONV *GFXDLL_Init)( gfxdlldesc_t *desc ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_Deinit)( void ) = NULL;
ULONG (GFXDLLCALLCONV *GFXDLL_GetPitch)( void ) = NULL;
BOOL (GFXDLLCALLCONV *GFXDLL_Activate)( int flag ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_FlipPage)( void ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_Clear)( ULONG color, ULONG depth, int flag ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_Fog)( FLOAT fognear, FLOAT fogfar, ULONG fogcolor ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_InitPalette)( UCHAR *pal ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_SetRgb)( int color, int red, int green, int blue ) = NULL;
void (GFXDLLCALLCONV *GFXDLL_GetRgb)( int color, int *red, int *green, int *blue ) = NULL;
BOOL (GFXDLLCALLCONV *GFXDLL_Fullscreen)( int flag ) = NULL;

typedef HRESULT(WINAPI *DIRECTINPUTCREATE)( HINSTANCE, DWORD, LPDIRECTINPUT* ,IUnknown* );
static DIRECTINPUTCREATE DICreate = NULL;


#ifdef _DEBUG
#define XD3D_DRV_FILENAME "DEBUG_xd3d.drv"
#define XOGL_DRV_FILENAME "DEBUG_xopengl.drv"
#define XSFT_DRV_FILENAME "DEBUG_xsoft.drv"
#define XGLD_DRV_FILENAME "DEBUG_xglide.drv"
#define XSDL_DRV_FILENAME "DEBUG_xsdl.drv"
#else
#define XD3D_DRV_FILENAME "xd3d.drv"
#define XOGL_DRV_FILENAME "xopengl.drv"
#define XSFT_DRV_FILENAME "xsoft.drv"
#define XGLD_DRV_FILENAME "xglide.drv"
#define XSDL_DRV_FILENAME "xsdl.drv"
#endif



//
// Alt-F4, ablak bezárás, azonnali kilépés
//
BOOL winWantToQuit( int flag ) {

	static BOOL wantToQuit = FALSE;

	if( flag != -1 )
		wantToQuit = flag;

	return wantToQuit;
}




#if _WIN32_WINNT >= 0x0400

#define KEY_IGNORE 0xFFC3D44F
#define KEY_PHYS_IGNORE (KEY_IGNORE - 1)  // Same as above but marked as physical for other instances of the hook.
#define KEY_IGNORE_ALL_EXCEPT_MODIFIER (KEY_IGNORE - 2)  // Non-physical and ignored only if it's not a modifier.

//
// low-level keyboard to detect ALT-TAB
// csak win2k-tól mûködik
//

static HHOOK ExistingKeyboardProc = NULL;
static int kbd_check_tic;

//
// Mumble: GlobalShortcut_win.cpp van egy ilyen Hook
// AHK: hook.cpp
//
static LRESULT CALLBACK KeyboardProcLowLevel( int nCode, WPARAM wParam, LPARAM lParam ) {

	// MSDN docs specify that both LL keybd & mouse hook should return in this case.
	if( nCode != HC_ACTION )
		return CallNextHookEx( ExistingKeyboardProc, nCode, wParam, lParam);

	KBDLLHOOKSTRUCT *hookstruct = (KBDLLHOOKSTRUCT *)(lParam);

	if( hookstruct->dwExtraInfo == KEY_PHYS_IGNORE )
		hookstruct->flags &= ~LLKHF_INJECTED;

	int vk = hookstruct->vkCode;
	int sc = hookstruct->scanCode;

	// check hogy mi vagyunk-e az elsők
	if( vk == 0xff ) {
		kbd_check_tic = GetTic();
		return CallNextHookEx( ExistingKeyboardProc, nCode, wParam, lParam);
	}

	if( vk && !sc ) // Might happen if another app calls keybd_event with a zero scan code.
		sc = vk2sc(vk);

	sc &= 0xFF;
	if( hookstruct->flags & LLKHF_EXTENDED )
		sc |= 0x100;

	BOOL is_artificial = hookstruct->flags & LLKHF_INJECTED; // LLKHF vs. LLMHF

	if( !is_artificial && hardware_keys ) {

		if( wParam == WM_KEYUP || wParam == WM_SYSKEYUP )
			hardware_keys[vk] = FALSE;
		else
			hardware_keys[vk] = TRUE;

		last_hardware_key = GetTickCount();

		// xprintf("real: %d, %d %s\n",vk,sc,hardware_keys[vk]?"down":"up");
	}

	// if( active_flag == FALSE )
	switch( wParam ) {

		case WM_KEYDOWN:
			break;

		case WM_KEYUP:
			break;

		case WM_SYSKEYDOWN:

			// Take no Action, Signal app to take action in main loop

			if( (((hookstruct->flags)>>5)&1) )  {

				// ALT +

				switch( hookstruct->vkCode ) {


					case VK_TAB : // ALT+TAB

						// cWinBase::SignalKeysAltTab();

						break;

					case VK_RETURN : // ALT+ENTER

						// cWinBase::SignalKeysAltEnter();

						// cWinBase::SignalKeysAltEnter();

						break;

					case VK_ESCAPE : // ALT+ESC

						// cWinBase::SignalKeysAltEsc();

						break;

					case VK_DELETE : // ALT+DEL

						// cWinBase::SignalKeysCtrAltDel();

						break;

				} //switch

			} // if alt+

			break;

		case WM_SYSKEYUP:

			break;
	}

	return CallNextHookEx( ExistingKeyboardProc, nCode, wParam, lParam);
}



//
//
//
void XLIB_CheckKeyboardHook( void ) {

	// nem is volt, nem is lesz
	if( ExistingKeyboardProc == NULL )
		return;

	// send vkff
	kbd_check_tic = 0;
	XLIB_SendKey( 0xFF, 0, -1 );

	Delay(200);

	if( GetTic() - kbd_check_tic > 1500 ) {
		XLIB_UnHookKeyboardProc();
		XLIB_HookKeyboardProc();
		// xprintf("rehooked kbd.\n");
	}

	return;
}




//
//
//
BOOL XLIB_HookKeyboardProc( void ) {

	if( ExistingKeyboardProc )
		return TRUE;

	REALLOCMEM( hardware_keys, VK_MAX );
	memset( hardware_keys, 0L, VK_MAX );

	ExistingKeyboardProc = SetWindowsHookEx( WH_KEYBOARD_LL, // int idHook,
						 KeyboardProcLowLevel, //HOOKPROC lpfn,
						 (HINSTANCE)GetHinstance(), // HINSTANCE hMod,
						 0 );// DWORD dwThreadId
	if( !ExistingKeyboardProc ) {
		//  Failed
		return FALSE;
	}

	// Succeeded.
	return TRUE;
}




//
//
//
void XLIB_UnHookKeyboardProc( void ) {

	if( ExistingKeyboardProc ) {

		BOOL retcode = UnhookWindowsHookEx((HHOOK) KeyboardProcLowLevel);

		if( retcode ) {
			// Successfully Un Hooked keyboard routine.
		}
		else {
			//Error Keyboard Not successfully Un hooked!
			// UnhookWindowsHookEx() returned failure!
		}

		ExistingKeyboardProc = 0;

		SAFE_FREEMEM(hardware_keys);

		return;
	}

	//Error Keyboard Not successfully hooked!
	// Could not unhook procedure!

	return;
}



#define LLMHF_INJECTED		1
#define WH_MOUSE_LL 		14
#define WM_XBUTTONDOWN 		523
#define WM_XBUTTONUP 		524
// #define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#define VK_WHEEL_LEFT      0x9C
#define VK_WHEEL_RIGHT     0x9D
#define VK_WHEEL_DOWN      0x9E
#define VK_WHEEL_UP        0x9F

static HHOOK ExistingMouseProc = NULL;

static int prev_mousexll = 0;
static int prev_mouseyll = 0;

//
//
//
static LRESULT CALLBACK MouseProcLowLevel( int nCode, WPARAM wParam, LPARAM lParam ) {

	if( nCode != HC_ACTION )
		return CallNextHookEx( ExistingMouseProc, nCode, wParam, lParam);

	MSLLHOOKSTRUCT *hookstruct = (MSLLHOOKSTRUCT *)(lParam);

	BOOL is_artificial = hookstruct->flags & LLMHF_INJECTED; // LLKHF vs. LLMHF

	// Physical mouse movement or button action (uses LLMHF vs. LLKHF).
	if( !is_artificial )
		last_hardware_key = GetTickCount();

	if( wParam == WM_MOUSEMOVE ) {
		if( !is_artificial ) {
			if( blockmousemove )
				return 1;
			if( FABS(mousespeed_percent - 100.0f) > FLOAT_EPSILON ) {
				int dx = hookstruct->pt.x - prev_mousexll;
				int dy = hookstruct->pt.y - prev_mouseyll;
				hookstruct->pt.x = ftoi((FLOAT)prev_mousexll + (FLOAT)dx * mousespeed_percent / 100.0f);
				hookstruct->pt.y = ftoi((FLOAT)prev_mouseyll + (FLOAT)dy * mousespeed_percent / 100.0f);
				SetCursorPos(hookstruct->pt.x,hookstruct->pt.y);
				// xprintf("mouse: %d, %d\n",dx,dy);
				prev_mousexll = hookstruct->pt.x;
				prev_mouseyll = hookstruct->pt.y;
				return 1;
			}
			prev_mousexll = hookstruct->pt.x;
			prev_mouseyll = hookstruct->pt.y;
		}
		return CallNextHookEx(ExistingMouseProc, nCode, wParam, lParam);
	}

	// xprintf("is_artificial: %d\n",is_artificial);

	int vk = 0;
	int sc = 0; // To be overriden if this even is a wheel turn.
	short wheel_delta;

	if( !is_artificial && hardware_keys )
		switch( wParam ) {

			case WM_XBUTTONDOWN:
				if( HIWORD(hookstruct->mouseData) == 1 ) {
					lowlevel_mouseb4 = 1;
					hardware_keys[VK_XBUTTON1] = TRUE;
				}
				else
				if( HIWORD(hookstruct->mouseData) == 2 ) {
					lowlevel_mouseb5 = 1;
					hardware_keys[VK_XBUTTON2] = TRUE;
				}
				break;

			case WM_XBUTTONUP:
				if( HIWORD(hookstruct->mouseData) == 1 ) {
					lowlevel_mouseb4 = -1;
					hardware_keys[VK_XBUTTON1] = FALSE;
				}
				else
				if( HIWORD(hookstruct->mouseData) == 2 ) {
					lowlevel_mouseb5 = -1;
					hardware_keys[VK_XBUTTON2] = FALSE;
				}
				break;

			case WM_MOUSEWHEEL:
				wheel_delta = GET_WHEEL_DELTA_WPARAM(hookstruct->mouseData); // Must typecast to short (not int) via macro, otherwise the conversion to negative/positive number won't be correct.
				if( wParam == WM_MOUSEWHEEL )
					vk = wheel_delta < 0 ? VK_WHEEL_DOWN : VK_WHEEL_UP;
				else
					vk = wheel_delta < 0 ? VK_WHEEL_LEFT : VK_WHEEL_RIGHT;
				sc = (wheel_delta > 0 ? wheel_delta : -wheel_delta) / WHEEL_DELTA; // See above. Note that sc is unsigned.
				break;

			case WM_LBUTTONUP: vk = VK_LBUTTON; hardware_keys[VK_LBUTTON] = FALSE; break;
			case WM_RBUTTONUP: vk = VK_RBUTTON; hardware_keys[VK_RBUTTON] = FALSE; break;
			case WM_MBUTTONUP: vk = VK_MBUTTON; hardware_keys[VK_MBUTTON] = FALSE; break;
			case WM_LBUTTONDOWN: vk = VK_LBUTTON; hardware_keys[VK_LBUTTON] = TRUE; break;
			case WM_RBUTTONDOWN: vk = VK_RBUTTON; hardware_keys[VK_RBUTTON] = TRUE; break;
			case WM_MBUTTONDOWN: vk = VK_MBUTTON; hardware_keys[VK_MBUTTON] = TRUE; break;
		}

	return CallNextHookEx( ExistingMouseProc, nCode, wParam, lParam);
}




//
//
//
BOOL XLIB_HookMouseProcLL( void ) {

	// már hookolva van
	if( ExistingMouseProc )
		return TRUE;

	POINT pt;
	GetCursorPos(&pt);
	prev_mousexll = pt.x;
	prev_mouseyll = pt.y;

	REALLOCMEM( hardware_keys, VK_MAX );
	memset( hardware_keys, 0L, VK_MAX );

	lowlevel_mouseb4 = 0;
	lowlevel_mouseb5 = 0;

	ExistingMouseProc = SetWindowsHookEx( WH_MOUSE_LL, // int idHook,
					      MouseProcLowLevel, //HOOKPROC lpfn,
					      (HINSTANCE)GetHinstance(), // HINSTANCE hMod,
					      0 ); // DWORD dwThreadId
	if( !ExistingMouseProc ) {

		//  Failed
		return FALSE;
	}

	// xprintf("HookMouseProc: done.\n");

	return TRUE;
}




//
//
//
void XLIB_UnHookMouseProcLL( void ) {

	if( ExistingMouseProc ) {

		BOOL retcode = UnhookWindowsHookEx((HHOOK) MouseProcLowLevel);

		if( retcode ) {
			// Successfully Un Hooked keyboard routine.
		}
		else {
			//Error Keyboard Not successfully Un hooked!
			// UnhookWindowsHookEx() returned failure!
		}

		ExistingMouseProc = 0;

		SAFE_FREEMEM(hardware_keys);

		// xprintf("UnHookMouseProc: undone.\n");

		return;
	}

	//Error Keyboard Not successfully hooked!
	// Could not unhook procedure!

	return;
}

#endif



//
//
//
void XLIB_SetMouseSpeed( FLOAT value ) {

	mousespeed_percent = value * 100.0f;

	if( FABS(value) <= FLOAT_EPSILON )
		blockmousemove = TRUE;
	else
		blockmousemove = FALSE;

	// xprintf("speed: %d%%\n",ftoi(mousespeed_percent));

	return;
}



//
// Mouse hook
//
static LRESULT CALLBACK MouseProc( int nCode, WPARAM wParam, LPARAM lParam ) {

	// MOUSEHOOKSTRUCTEX
	// WM_MOUSEWHEEL
	// dz = HIWORD(d.mouseData);
	// WM_XBUTTONDOWN, WM_XBUTTONUP

	if( nCode < 0 )
		return CallNextHookEx( hMouseHook, nCode, wParam, lParam );

	if( nCode == HC_ACTION ) {

		MOUSEHOOKSTRUCT *pmhs = (MOUSEHOOKSTRUCT *)lParam;

		switch( wParam ) {

			case WM_MOUSEMOVE:

				/***
				if( winFullscreen() == TRUE ) {

					mousedx = win32_mousedx - (win32_lastMouseX - pmhs->pt.x);
					mousedy = win32_mousedy - (win32_lastMouseY - pmhs->pt.y);
					win32_lastMouseX = pmhs->pt.x;
					win32_lastMouseY = pmhs->pt.y;

					win32_mousedx = 0;
					win32_mousedy = 0;
				}
				else {
				***/
					// static int cnt = 0;
					// xprintf("mmove %d %d %d %d\n",++cnt,pmhs->pt.x,win32_mousecx,win32_mousecy);

					mousex = pmhs->pt.x - win32_mousecx;
					mousey = pmhs->pt.y - win32_mousecy;

					mousedx = -(win32_lastMouseX - pmhs->pt.x);
					mousedy = -(win32_lastMouseY - pmhs->pt.y);

					win32_lastMouseX = pmhs->pt.x;
					win32_lastMouseY = pmhs->pt.y;
				/// }

				break;

			case WM_LBUTTONDOWN:

				win32_mousebl = TRUE;

				break;

			case WM_LBUTTONUP:

				win32_mousebl = FALSE;

				break;


			case WM_MBUTTONDOWN:

				win32_mousebm = TRUE;

				break;

			case WM_MBUTTONUP:

				win32_mousebm = FALSE;

				break;


			case WM_RBUTTONDOWN:

				win32_mousebr = TRUE;

				break;

			case WM_RBUTTONUP:

				win32_mousebr = FALSE;

				break;

			case WM_XBUTTONDOWN:

				// HIWORD(pmhs.mouseData)

				break;

			case WM_XBUTTONUP:

				// HIWORD(pmhs.mouseData)

				break;

			case WM_MOUSEWHEEL:

				// mousedz = HIWORD(pmhs.mouseData);

				break;
		}
	}

	return 0;
}



//
//
//
BOOL XLIB_HookMouseProc( void ) {

	win32_mousebl = FALSE;
	win32_mousebm = FALSE;
	win32_mousebr = FALSE;
	win32_mouseb4 = FALSE;
	win32_mouseb5 = FALSE;

	hMouseHook = SetWindowsHookEx( WH_MOUSE, (HOOKPROC)MouseProc, NULL, GetCurrentThreadId() );

	xprintf("init: win32 mouse started.\n");

	return TRUE;
}




//
//
//
void XLIB_UnHookMouseProc( void ) {

	if( hMouseHook )
		UnhookWindowsHookEx( hMouseHook );

	hMouseHook = NULL;

	return;
}



//
// winfosban viszi a mouset
//
void winMoveCursor( int x, int y ) {

	POINT point;
	point.x = x;
	point.y = y;

	ClientToScreen( (HWND)GetHwnd(), &point );

	SetCursorPos( point.x, point.y );

	return;
}





//
//
//
void winClipMouse( void ) {

	RECT rc;
	POINT point;

	if( /*(winFullscreen() == TRUE) || */(winNoDinput() == FALSE) ) {

		GetWindowRect( hwnd, &rc );

		// xprintf("MOUSE window: %d %d, %d %d\n",rc.left,rc.top,rc.right,rc.bottom);

		win32_mousecx = (rc.left + rc.right) / 2;
		win32_mousecy = (rc.top + rc.bottom) / 2;

		// freebasic forrásból
		GetClientRect(hwnd, &rc);
		point.x = rc.left;
		point.y = rc.top;

		ClientToScreen(hwnd, &point);
		rc.left = point.x;
		rc.top = point.y;
		point.x = rc.right;
		point.y = rc.bottom;

		ClientToScreen(hwnd, &point);
		rc.right = point.x;
		rc.bottom = point.y;

		ClipCursor(&rc);
	}
	else {

		// xprintf("ClipMouse: nem fullscreenes\n");

		GetWindowRect( hwnd, &rc );

		win32_mousecx = rc.left;
		win32_mousecy = rc.top;

		// xprintf("MOUSE rec1: %d %d, %d %d\n",rc.left,rc.top,rc.right,rc.bottom);

		GetClientRect( hwnd, &rc );

		win32_mousecx += rc.left;
		win32_mousecy += rc.top;

		win32_mousecy += (GetSystemMetrics( SM_CYCAPTION ) + GetSystemMetrics( SM_CYFRAME ));

		/*
		xprintf("MOUSE rec2: %d %d, %d %d\n",rc.left,rc.top,rc.right,rc.bottom);
		xprintf("m: %d %d\n",win32_mousecx,win32_mousecy);
		*/

		/***
		RECT rcScreenRect;

		GetClientRect( hwnd, &rcScreenRect );
		ClientToScreen( hwnd, (POINT*)&rcScreenRect.left );
		ClientToScreen( hwnd, (POINT*)&rcScreenRect.right );

		xprintf("rc: %d,%d  %d,%d\n",rcScreenRect.left, rcScreenRect.top, rcScreenRect.right, rcScreenRect.bottom);
		***/
	}

	return;
}



//
// http://www.ucancode.net/CPP_Library_Control_Tool/Create-display-convert-bitmap-cursor-CreateIconIndirect-DeleteDC-ICONINFO-VC.htm
//
static HCURSOR MakeHcursor( char *filename ) {

	HBITMAP hSourceBitmap;
	COLORREF clrTransparent = RGB(0,0,0);
	HBITMAP hAndMaskBitmap;
	HBITMAP hXorMaskBitmap;
	static HCURSOR hCursor = NULL;

	extern HBITMAP ConvertHbitmap( UCHAR *spr );

	if( filename == NULL ) {
		if( hCursor ) DestroyIcon(hCursor);
		hCursor = NULL;
		return NULL;
	}

	if( hCursor != NULL )
		return hCursor;

	if( FileExist(filename) == FALSE )
		return NULL;

	UCHAR *spr;
	if( LoadPicture(filename,PTR(spr)) == FALSE )
		return NULL;
	TrueSprite(PTR(spr));
	hSourceBitmap = ConvertHbitmap(spr);
	FREEMEM(spr);

	HDC hDC        = GetDC(NULL);
	HDC hMainDC    = CreateCompatibleDC(hDC);
	HDC hAndMaskDC = CreateCompatibleDC(hDC);
	HDC hXorMaskDC = CreateCompatibleDC(hDC);

	// Get the dimensions of the source bitmap
	BITMAP bm;
	GetObject(hSourceBitmap,sizeof(BITMAP),&bm);

	hAndMaskBitmap = CreateCompatibleBitmap(hDC,bm.bmWidth,bm.bmHeight);
	hXorMaskBitmap = CreateCompatibleBitmap(hDC,bm.bmWidth,bm.bmHeight);

	//Select the bitmaps to DC
	HBITMAP hOldMainBitmap    = (HBITMAP)SelectObject(hMainDC,hSourceBitmap);
	HBITMAP hOldAndMaskBitmap = (HBITMAP)SelectObject(hAndMaskDC,hAndMaskBitmap);
	HBITMAP hOldXorMaskBitmap = (HBITMAP)SelectObject(hXorMaskDC,hXorMaskBitmap);

	//Scan each pixel of the souce bitmap and create the masks
	COLORREF MainBitPixel;
	for( int x=0; x<bm.bmWidth; ++x )
	for( int y=0; y<bm.bmHeight; ++y ) {
		MainBitPixel = GetPixel(hMainDC,x,y);
		if( MainBitPixel == clrTransparent ) {
			SetPixel(hAndMaskDC,x,y,RGB(255,255,255));
			SetPixel(hXorMaskDC,x,y,RGB(0,0,0));
		}
		else {
			SetPixel(hAndMaskDC,x,y,RGB(0,0,0));
			SetPixel(hXorMaskDC,x,y,MainBitPixel);
		}
	}

	SelectObject(hMainDC,hOldMainBitmap);
	SelectObject(hAndMaskDC,hOldAndMaskBitmap);
	SelectObject(hXorMaskDC,hOldXorMaskBitmap);

	DeleteDC(hXorMaskDC);
	DeleteDC(hAndMaskDC);
	DeleteDC(hMainDC);

	ReleaseDC(NULL,hDC);

	ICONINFO iconinfo       = {0};
	iconinfo.fIcon          = FALSE;
	iconinfo.xHotspot       = 0;
	iconinfo.yHotspot       = 0;
	iconinfo.hbmMask        = hAndMaskBitmap;
	iconinfo.hbmColor       = hXorMaskBitmap;

	hCursor = CreateIconIndirect(&iconinfo);

	return hCursor;
}


static HCURSOR hCursor = NULL;
static BOOL hourglass_mouse = FALSE;

//
//
//
void winNormalCursor( void ) {

	hourglass_mouse = FALSE;

	HCURSOR hc = MakeHcursor( MOUSECUR );
	SetCursor( hc?hc:LoadCursor(NULL, IDC_ARROW) );

	return;
}


//
//
//
void winWaitCursor( void ) {

	hourglass_mouse = TRUE;

	hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );

	return;
}



static char draganddrop_name[XMAX_PATH] = "My body is ready.";

//
//
//
char *winGetDragAndDrop( char *filename ) {

	// van-e droppolt file
	static BOOL bDragAndDrop = FALSE;

	// query
	if( filename == NULL ) {
		if( bDragAndDrop == TRUE ) {
			bDragAndDrop = FALSE;
			return draganddrop_name;
		}
		else
			return NULL;
	}

	bDragAndDrop = TRUE;
	draganddrop_name[0] = 0;
	strcpy( draganddrop_name, filename );

	RemoveChar( draganddrop_name, '\"' );

	return NULL;
}





//
// inactivá teszi az alkalmazást
//
void winMinimize( int flag ) {

	HWND hwnd = (HWND)GetHwnd();

	if( hwnd )
		ShowWindow( hwnd, SW_MINIMIZE );

	HWND backhwnd = (HWND)GetBackHwnd();

	if( backhwnd != NULL )
		ShowWindow( backhwnd, SW_MINIMIZE );

	return;
}






// Menu

#define CMD_OPENFILE	101
#define CMD_EXIT	199
#define CMD_ABOUT	201
#define CMD_TOGGLE	211
#define CMD_LOG		212


//
//
//
static HMENU DX_CreateMenu( void ) {

	HMENU menubar = CreateMenu();
	HMENU file = CreateMenu();
	HMENU context = CreateMenu();
	HMENU help = CreateMenu();

	AppendMenu(menubar, MF_POPUP|MF_STRING, (UINT)file, "&File");
	AppendMenu(file, MF_STRING, CMD_OPENFILE, "&Open...");
	AppendMenu(file, MF_SEPARATOR, 0, NULL);
	AppendMenu(file, MF_STRING, CMD_EXIT, "E&xit");

	AppendMenu(menubar, MF_POPUP|MF_STRING, (UINT)context, "&Context");
	AppendMenu(context, MF_STRING, CMD_TOGGLE, "&Toggle Window");
	AppendMenu(context, MF_STRING, CMD_OPENFILE, "&Open...");
	AppendMenu(context, MF_SEPARATOR, 0, NULL);
	AppendMenu(context, MF_STRING, CMD_LOG, "Show &Log");
	AppendMenu(context, MF_STRING, CMD_ABOUT, "&About...");
	AppendMenu(context, MF_STRING, CMD_EXIT, "E&xit");

	AppendMenu(menubar, MF_POPUP|MF_STRING, (UINT)help, "&Help");
	AppendMenu(help, MF_STRING, CMD_ABOUT, "&About...");

	return menubar;
}

static void restoreFromTray( void );

//
//
//
static void DX_Menu( int cmd ) {

	switch( cmd ) {

		case CMD_OPENFILE:

			// drag and drop-ba kell beleírni
			// FIXME
			winOpenFile( "All Files (*.*)\0*.*\0");

			break;

		case CMD_EXIT:

			winWantToQuit( TRUE );

        		break;

		case CMD_ABOUT: {

			static BOOL flag = FALSE;

			if( flag == FALSE ) {

				flag = TRUE;

				char str[XMAX_PATH*2];

				sprintf( str,"%s\n\n%s", XLIB_winSetProgramDesc(NULL), getXlibVersion() );

				MessageBox(hwnd, str, "About", MB_OK|MB_ICONINFORMATION);

				flag = FALSE;
			}

			break;
		}

		case CMD_TOGGLE:

			if( active_flag <= 0 )
				restoreFromTray();
			else
				winMinimize();

			break;

		case CMD_LOG: {

			// toggle log window

			winSetNoLog( FALSE );

			winToggleLog();
			// winShowLog();

			break;
		}
	}

	return;
}




#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 13)
static NOTIFYICONDATA mIconData;
static BOOL bTrayInited = FALSE;
static BOOL bIsTray = FALSE;		// lehet-e system trayre menni

static HMENU tray_menu = NULL;

static HINSTANCE hWingyogy = NULL;
static BOOL (__cdecl *pRemoveTaskbarIcon)( HWND hwnd, int status ) = NULL;



//
//
//
static BOOL RemoveTaskbarIcon( HWND hwnd, int status = 0 ) {

	if( (hwnd == NULL) || (bIsTray == FALSE) || (bTrayInited == FALSE) )
		return FALSE;

	if( hWingyogy == NULL ) {

		char bassdllname[XMAX_PATH];
		sprintf( bassdllname, "%s%c%s", GetExeDir(), PATHDELIM, XWINGYOGYDLLNAME );

		if( (hWingyogy = LoadLibrary(bassdllname)) == NULL ) {
			static BOOL sema = FALSE;
			if( sema == FALSE )
				xprintf("RemoveTaskbarIcon: couldn't load \"%s\" library.\n",bassdllname);
			sema = TRUE;
			return FALSE;
		}

		if( (pRemoveTaskbarIcon = (BOOL (__cdecl*)(HWND,int))GetProcAddress( hWingyogy, "RemoveTaskbarIcon" )) == NULL ) {
			xprintf("no RemoveTaskbarIcon\n" );
			// html is ott van
			// FreeLibrary( hSpeech );
			hWingyogy = NULL;
			return FALSE;
		}

		xprintf("RemoveTaskbarIcon: loaded and inited. (%s).\n",bassdllname);
	}

	if( pRemoveTaskbarIcon )
		pRemoveTaskbarIcon(hwnd, status);

	return TRUE;
}




//
// maximize from tray
//
static void restoreFromTray( void ) {

	HWND backhwnd = (HWND)GetBackHwnd();

	if( bIsTray ) {

		if( backhwnd != NULL )
			ShowWindow( backhwnd, SW_SHOW );

		ShowWindow( hwnd, SW_SHOW );
	}

	if( (backhwnd != NULL) )
		ShowWindow(backhwnd,SW_RESTORE);

	ShowWindow(hwnd,SW_RESTORE);

	SetActiveWindow(hwnd);
	SetForegroundWindow(hwnd);

	return;
}



#define IDT_TIMER1 1002

static UINT taskbar_timer = 0;


//
//
//
BOOL winSystemTray( int flag, char *tooltip ) {

	if( flag != -1 )
		bIsTray = flag;

	if( bIsTray && (bTrayInited == FALSE) && hwnd ) {

		mIconData.cbSize		= sizeof(NOTIFYICONDATA);
		mIconData.hWnd			= hwnd;
		mIconData.uID			= 1;
		mIconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
		mIconData.uFlags		= NIF_MESSAGE;

		HICON hIcon = 0;
		hIcon = LoadIcon( hInst, MAKEINTRESOURCE(IDI_ICON1) );
		if( hIcon ) {
			mIconData.hIcon = hIcon;
			mIconData.uFlags |= NIF_ICON;
		}

		strcpy( mIconData.szTip, XLIB_winSetProgramDesc(NULL) );
		mIconData.uFlags |= NIF_TIP;

		if( Shell_NotifyIcon( NIM_ADD, &mIconData ) == FALSE )
			xprintf("winSystemTray: Shell_NotifyIcon phailed.\n" );

		tray_menu = DX_CreateMenu();

		taskbar_timer = SetTimer( hwnd,		// handle to main window
				    IDT_TIMER1,         // timer identifier
				    200,                // 10-second interval
				    (TIMERPROC)NULL );   // no timer callback

		bTrayInited = TRUE;
	}

	if( bTrayInited && tooltip && strcmp( mIconData.szTip, tooltip ) ) {

		strcpy( mIconData.szTip, tooltip );

		Shell_NotifyIcon( NIM_MODIFY, &mIconData );

		xprintf("winSystemTray: %d, %s\n", flag, tooltip );
	}

	return bIsTray;
}



//
//
//
LRESULT APIENTRY DX_WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {

	switch( msg ) {

		case WM_CREATE:

			break;

		case WM_SETCURSOR:

			if( hourglass_mouse == TRUE ) {
				winWaitCursor();
				return TRUE;
			}

			// return TRUE;

			break;

		case WM_MOVE:
		case WM_SIZE:

			break;

		case WM_ERASEBKGND:

			break;


		case WM_PAINT: {

			PAINTSTRUCT ps;

			BeginPaint( hwnd, &ps );
			EndPaint( hwnd, &ps );

			// return 0;
			}
			break;

		case WM_SYSCOMMAND:

			switch( wparam & 0xFFF0 ) {	// ~0x0F

				case SC_TASKLIST:
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:

					/* Ignore screensaver requests in fullscreen modes */

					// return 0;
					break;

				case SC_MINIMIZE: {

					HWND backhwnd = (HWND)GetBackHwnd();

					if( backhwnd )
						ShowWindow( backhwnd, SW_MINIMIZE );

					break;
				}

				case SC_RESTORE: {

					HWND backhwnd = (HWND)GetBackHwnd();

					if( backhwnd  )
						ShowWindow( backhwnd, SW_SHOWNORMAL );

					break;
				}

				case SC_MAXIMIZE:

					if( winFullscreen() == FALSE )
						if( ToggleFullscreen() == TRUE )
							StatusText( ST_YELLOW, "Alt-Enter to switch back" );
					break;
			}
			break;

		case WM_KEYDOWN:

			if( KeyboardDevice == NULL ) {

				int scan = (lparam >> 16) & 0xff;

				//xprintf("scan: %x, %x\n",scan,wparam);

				if( !(lparam & (1<<30)) )
					WIN32_keyboard_handler( scan, 0x80 );
			}

			break;

		case WM_KEYUP:

			if( KeyboardDevice == NULL ) {

				int scan = (lparam >> 16) & 0xff;

				WIN32_keyboard_handler( scan, 0x00 );
			}

			break;

#ifdef WM_MOUSEWHEEL
		case WM_MOUSEWHEEL:

			if( MouseDevice == NULL ) {

				signed short word;
				unsigned short uword = HIWORD(wparam);

				memcpy( &word, &uword, 2 );

				mousedz = word / mouse_zgran;
			}

			break;
#endif

		case WM_DISPLAYCHANGE:

			// g_pVoid->OnDisplayChange(wParam, LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_ENTERSIZEMOVE:
		case WM_ENTERMENULOOP:
		case WM_SYSCHAR:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:

			/* Stop Alt-Space from pausing our game */

			// handle Alt-F4
			if( (msg == WM_SYSKEYDOWN) && (wparam == VK_F4) && (lparam & (1<<29)) )
				// Quit( NULL );
				// PostQuitMessage( 0 );
				// exit(12);
				winWantToQuit( TRUE );

			// handle Alt-TAB
			// if( (msg == WM_SYSKEYDOWN) && (wparam == VK_TAB) && (lparam & (1<<29)) )
			//	xprintf("alt-tab: itt van.\n");

			return 0;

			break;

		case WM_EXITSIZEMOVE: {

			// Message("exit size move");

			int flag = GFXDLL_RESIZE;

			if( GFXDLL_GetData )
				GFXDLL_GetData( &flag, sizeof(int) );

			if( GetMouseClip() )
				winClipMouse();

			DX_SaveWindowPos();

			}

			break;


		case WM_CHAR:


			break;

		case WM_SETFOCUS: {

			// xprintf("setfocus\n");
			// winSetBackImage( 2 );

			break;
		}

		case WM_ACTIVATE: {

			if( winWantToQuit() )
				break;

			int fActive = LOWORD(wparam);
			int fMinimized = HIWORD(wparam);

			if( (fActive == WA_INACTIVE) || fMinimized ) {

				active_flag = 0;

				// minize to tray
				if( fMinimized && bIsTray && bTrayInited ) {

					HWND backhwnd = (HWND)GetBackHwnd();

					if( backhwnd != NULL )
						ShowWindow( backhwnd, SW_HIDE );

					ShowWindow( hwnd, SW_HIDE );

					XLIB_winWriteProfileInt( "xlib_minimized", 1 );

					active_flag = -1;
				}
			}
			else {
				// ha előzőleg minimalizált volt, mert induláskor is kap ilyet
				if( active_flag <= 0 )
					XLIB_winWriteProfileInt( "xlib_minimized", 0 );

				active_flag = 1;

				restoreFromTray();


				/***
				// TODO: remove mouse clicks & mouse moves
				// Consume all existing mouse events and those posted to our own dispatch queue
				MSG msg;
				PeekMessage( &msg, 0,WM_MOUSEFIRST,WM_MOUSELAST , PM_QS_POSTMESSAGE | PM_NOYIELD | PM_REMOVE );
				RemoveMessages( NULL, WM_MOUSEMOVE, WM_MOUSEMOVE );
				***/
			}

			if( GFXDLL_Activate )
				GFXDLL_Activate( active_flag );

			ClearKey();
			ClearMouse();

			xprintf( "DX_WndProc: active_flag = %s.\n", active_flag==1?"TRUE":active_flag==0?"FALSE":"minimized" );

			break;
		}


		case SERVER_SOCKET_EVENT_HAS_HAPPENED:

			// NetworkServerCallback( wparam, lparam );

			break;


		case CLIENT_SOCKET_EVENT_HAS_HAPPENED:

			// NetworkClientCallback( wparam, lparam );

			break;


		case WM_CLOSE:
		case WM_DESTROY:

			// PostQuitMessage( 0 );

			// FIXME
			// Quit(NULL);
			// exit(11);
			winWantToQuit( TRUE );

			if( GFXDLL_Activate )
				GFXDLL_Activate( FALSE );

			RemoveTaskbarIcon( hwnd, -1 );
			// return 0;

			break;

		// drag and drop
		case WM_DROPFILES: {

			HDROP hDrop = (HDROP)wparam;
			POINT pt;

			DragQueryPoint( hDrop, &pt);

			char filename[MAX_PATH];

			UINT count = DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 );

			xprintf( "Drag and drop: " );

			if( count > 0 ) {

				DragQueryFile( hDrop, 0, filename, MAX_PATH );

				DragFinish( hDrop );

				winGetDragAndDrop( filename );

				xprintf( "counted %d at [%d, %d] with \"%s\" first.\n", count, pt.x, pt.y, draganddrop_name );
			}
			else
				xprintf( "no files WTF?!\n" );


			return 0;

			break;
		}

		//
		// tray icon message
		//
		case WM_TRAY_ICON_NOTIFY_MESSAGE: {

			int uID = (int)wparam;
			int uMsg = (int)lparam;

			if( uID != 1 )
				break;

			POINT pt;

			switch( uMsg ) {

				case WM_MOUSEMOVE:
					GetCursorPos(&pt);
					ClientToScreen(hwnd,&pt);
					// OnTrayMouseMove(pt);
					break;

				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
					GetCursorPos(&pt);
					ClientToScreen(hwnd,&pt);
					// OnTrayLButtonDown(pt);
					if( active_flag <= 0 )
						restoreFromTray();
					else
						ShowWindow( hwnd, SW_MINIMIZE );
					break;

				/***
				case WM_LBUTTONDBLCLK:
					GetCursorPos(&pt);
					ClientToScreen(hwnd,&pt);
					// OnTrayLButtonDblClk(pt);
					restoreFromTray();
					break;
				***/

				case WM_RBUTTONDOWN:
				case WM_CONTEXTMENU:
				case WM_RBUTTONDBLCLK:

					// jobb klik a tray ikonon

					GetCursorPos(&pt);

					if( tray_menu ) {

						// toggle window a default
						SetMenuDefaultItem( GetSubMenu( tray_menu, 1 ), 0, TRUE );

						SetActiveWindow(hwnd);
						SetForegroundWindow(hwnd);

						char str[MAX_PATH] = "derp";

						// mute - unmute kellene talán

						if( IsIconic(hwnd) )
							// "Restore Window"
							LoadString( (HINSTANCE)GetHinstance(), IDS_STRING40023, str, MAX_PATH );
						else
							// "Hide Window"
							LoadString( (HINSTANCE)GetHinstance(), IDS_STRING40022, str, MAX_PATH );

						if( !ModifyMenu( GetSubMenu( tray_menu, 1 ), 0, MF_BYPOSITION|MF_STRING, CMD_TOGGLE, str ) )
							xprintf( "DX_WndProc: ModifyMenu for Window phailed. (%s)\n", XLIB_GetWinError() );

						// Log window
						if( winLogState() == FALSE )
							// "Show Log"
							LoadString( (HINSTANCE)GetHinstance(), IDS_STRING40024, str, MAX_PATH );
						else
							// "Hide Log"
							LoadString( (HINSTANCE)GetHinstance(), IDS_STRING40025, str, MAX_PATH );

						if( !ModifyMenu( GetSubMenu( tray_menu, 1 ), 3, MF_BYPOSITION|MF_STRING, CMD_LOG, str ) )
							xprintf( "DX_WndProc: ModifyMenu for Log phailed. (%s)\n", XLIB_GetWinError() );

						TrackPopupMenu( GetSubMenu( tray_menu, 1 ),
								TPM_BOTTOMALIGN |
								TPM_LEFTBUTTON |
								TPM_RIGHTBUTTON,
								pt.x, pt.y, 0, hwnd, NULL);
					}

					break;
			}

			break;
		}

		// menü kezelés a tray iconból
		case WM_COMMAND:

			if( !HIWORD(wparam) )
				DX_Menu( LOWORD(wparam) );

			break;


		case WM_TIMER:

			switch( wparam ) {

				case IDT_TIMER1:

					RemoveTaskbarIcon( hwnd );
					RemoveTaskbarIcon( (HWND)GetBackHwnd() );

					break;
			}

			break;

		/***
		case WM_CLOSE:
			// Don't let the window be closed, just hide it
			ShowWindow(hWnd, SW_HIDE);
			return 0;

		case WM_CTLCOLORSTATIC:
			// Select the correct font for the text box
			hdc = (HDC) wParam;
			SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
			return 0;
		***/
	}

	return DefWindowProc( hwnd, msg, wparam, lparam );
}






//
//
//
void DX_CleanupWindow( void ) {

	if( bTrayInited ) {
		Shell_NotifyIcon( NIM_DELETE, &mIconData );
		KillTimer( hwnd, taskbar_timer );
		taskbar_timer = 0;
		if( tray_menu ) DestroyMenu( tray_menu );
		tray_menu = NULL;
		bTrayInited = FALSE;
	}

	//SendMessage(hwnd,WM_CLOSE,0,0);
	if( hwnd ) DestroyWindow( hwnd );
	// hwnd = NULL;

	InvalidateRect( GetTopWindow(NULL), NULL, TRUE );

	UnregisterClass( XLIBWNDCLASSNAME, hInst );

	// SetErrorMode( SEM_NOGPFAULTERRORBOX );
	// VERIFY( SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS ) );

	MakeHcursor(NULL);

	return;
}



//
//
//
void DX_SetupWindow( int width, int height, int depth, int flag ) {

	WNDCLASS wndclass;
	int x,y,w=0,h=0;
	RECT rc;
	int win_flag;

	if( !GetClassInfo( hInst, XLIBWNDCLASSNAME, &wndclass ) ) {

		memset( &wndclass, 0, sizeof(wndclass) );

		// wndclass.cbSize = sizeof(WNDCLASS);
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = DX_WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInst;
		wndclass.hIcon = LoadIcon( hInst, MAKEINTRESOURCE(IDI_ICON1) );
		if( (wndclass.hCursor = MakeHcursor( MOUSECUR )) == NULL )
			wndclass.hCursor = NULL; // LoadCursor( NULL, IDC_ARROW ) = hidemouse nem működik e miatt da fuq iz dat?
		wndclass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = XLIBWNDCLASSNAME;
		// wndclass.hIconSm = LoadIcon( hInst, MAKEINTRESOURCE(IDI_ICON1) );

		// XLIB_winDeleteFile( str );

		if( !RegisterClass( &wndclass ) ) {
			Quit("DX_SetupWindow: RegisterClass phailed. (%s)\n", XLIB_GetWinError() );
			return;
		}
	}

	SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 );

	x = (flag==SETX_JUSTWINDOW) ? CW_USEDEFAULT : ((rc.right - rc.left - width ) / 2);
	y = (flag==SETX_JUSTWINDOW) ? CW_USEDEFAULT : ((rc.bottom - rc.top - height ) / 2);
	w = (flag==SETX_JUSTWINDOW) ? CW_USEDEFAULT : 0; // GetSystemMetrics( SM_CXSCREEN ); // (width + 2*GetSystemMetrics( SM_CXFRAME )); // GetSystemMetrics( SM_CXSCREEN );
	h = (flag==SETX_JUSTWINDOW) ? CW_USEDEFAULT : 0; //GetSystemMetrics( SM_CYSCREEN ); // (height + GetSystemMetrics( SM_CYCAPTION ) + 2*GetSystemMetrics( SM_CYFRAME ));

	/***
	WS_POPUPWINDOW | // WS_MINIMIZE |
	// WS_OVERLAPPEDWINDOW,
	WS_VISIBLE,
	// ((flag!=SETX_JUSTWINDOW)?WS_POPUP:0) |
	//WS_CAPTION,
	//WS_SYSMENU,
	***/

	if( winFullscreen() || flag == SETX_NOBORDER ) {

		// a háttér ablak nem kell
		winHideBackWindow();

		win_flag = 0; // WS_POPUP;
	}
	else {
		win_flag = WS_OVERLAPPED |
				WS_SYSMENU |
				WS_CAPTION |
				WS_MINIMIZEBOX;

		// ilyenkor gyorsan indul, nem standard felbontással
		if( flag != SETX_FORCERND )
			win_flag |= WS_MAXIMIZEBOX;

	}

	if( (hwnd = CreateWindowEx( /*WS_EX_TRANSPARENT*/ 0,
					XLIBWNDCLASSNAME,
					XLIB_winSetProgramDesc(NULL),
					win_flag,
					/***
					((width==SETX_JUSTWINDOW)?CW_USEDEFAULT:0),
					((width==SETX_JUSTWINDOW)?CW_USEDEFAULT:0),
					((width==SETX_JUSTWINDOW)?CW_USEDEFAULT:GetSystemMetrics( SM_CXSCREEN )),
					((width==SETX_JUSTWINDOW)?CW_USEDEFAULT:GetSystemMetrics( SM_CYSCREEN )),
					****/
					x,y,w,h,
					winFullscreen()?NULL:(HWND)GetBackHwnd(),
					NULL /*DX_CreateMenu()*/,
					hInst,
					NULL ) ) == NULL )
		Quit("DX_SetupWindow: CreateWindowEx failed. (%s)",XLIB_GetWinError());

	if( winFullscreen() || flag == SETX_NOBORDER ) {
		LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
		lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
		SetWindowLong(hwnd, GWL_STYLE, lStyle);
		SetWindowPos(hwnd, NULL, x,y,w,h, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}

	DragAcceptFiles( hwnd, TRUE );

	atexit( DX_CleanupWindow ); // ez meghivja még1szer a quitot

	return;
}




static BOOL timer_inited = FALSE;
static UINT TimerId = 0;

//
//
//
void CALLBACK MyTimerProc( UINT IDEvent, UINT uReserved, DWORD dwUser, DWORD dwReserved1, DWORD dwReserved2 ) {

	static volatile int timersema=0;

	if( ++timersema == 1 ) {

		++TimeCount;

	}

	timersema--;

	return;
}




//
//
//
void SetupTimer( void ) {

	return;

	if( timer_inited )
		return;

	timer_inited = TRUE;

	TimeCount = 0L;

	// timeBeginPeriod(20);	  /* set the minimum resolution */

	// Set up the callback event.	The callback function
	// MUST be in a FIXED CODE DLL!!! -> not in Win95
	TimerId = timeSetEvent( 40,		/* how often			*/
				40,		/* timer resolution		*/
				MyTimerProc,	/* callback function		*/
				0,		/* info to pass to callback	*/
				TIME_PERIODIC); /* oneshot or periodic?		*/

	didupdate = 1;

	xprintf("init: timer started.\n");

	return;
}





//
//
//
void CleanupTimer( void ) {

	if( !timer_inited )
		return;

	timer_inited = FALSE;

	timeKillEvent(TimerId);

	// timeEndPeriod(20);

	return;
}






//
// Set a DWORD property on a DirectInputDevice.
//
static HRESULT SetDIDwordProperty( LPDIRECTINPUTDEVICE pdev, REFGUID guidProperty,
				DWORD dwObject, DWORD dwHow, DWORD dwValue ) {

	DIPROPDWORD dipdw;

	dipdw.diph.dwSize	= sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj	= dwObject;
	dipdw.diph.dwHow	= dwHow;
	dipdw.dwData		= dwValue;

	return pdev->SetProperty( guidProperty, &dipdw.diph );
}




//
//
//
BOOL CALLBACK InitKeyboardInput( LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef ) {

	HRESULT error;

	if( (error=DirectInput->CreateDevice(pdinst->guidInstance, &KeyboardDevice, NULL)) )
		return DIENUM_CONTINUE;

	if( (error=KeyboardDevice->SetDataFormat(&c_dfDIKeyboard)) ) {
		KeyboardDevice->Release();
		KeyboardDevice = NULL;
		return DIENUM_CONTINUE;
	}

	if( (error=SetDIDwordProperty(KeyboardDevice, DIPROP_BUFFERSIZE, 0, DIPH_DEVICE, DEVICEBUFFERSIZE)) ) {
		KeyboardDevice->Release();
		KeyboardDevice = NULL;
		return DIENUM_CONTINUE;
	}

	if( (error=KeyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE)) ) {
		KeyboardDevice->Release();
		KeyboardDevice = NULL;
		return DIENUM_CONTINUE;
	}

	if( (error=KeyboardDevice->Acquire()) ) {
		KeyboardDevice->Release();
		KeyboardDevice = NULL;
		return DIENUM_CONTINUE;
	}

	xprintf("init: \"%s\" directx keyboard.\n", pdinst->tszProductName );

	return DIENUM_STOP;
}





//
//
//
BOOL CALLBACK InitMouseInput( LPCDIDEVICEINSTANCE pdinst, LPVOID pvRef ) {

	DIPROPDWORD dipdw;
	HRESULT error;

	if( (error = DirectInput->CreateDevice(pdinst->guidInstance, &MouseDevice, NULL)) )
		return DIENUM_CONTINUE;

	if( (error = MouseDevice->SetDataFormat(&c_dfDIMouse)) ) {
		MouseDevice->Release();
		MouseDevice = NULL;
		return DIENUM_CONTINUE;
	}

	if( (error = SetDIDwordProperty(MouseDevice, DIPROP_BUFFERSIZE, 0, DIPH_DEVICE, DEVICEBUFFERSIZE)) ) {
		MouseDevice->Release();
		MouseDevice = NULL;
		return DIENUM_CONTINUE;
	}

	memset( &dipdw.diph, 0, sizeof(dipdw.diph) );
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0; // device property
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData = 120;

	if( SUCCEEDED(error = MouseDevice->GetProperty(DIPROP_GRANULARITY, &dipdw.diph) ) ) {
		mouse_zgran = dipdw.dwData;
		xprintf("InitMouseInput: mouse_zgran = %d.\n",mouse_zgran);
	}

	// if( SUCCEDED(error=MouseDevice->GetProperty(DIPROP_PRODUCTNAME, &dipdw.diph) ) )
	//	xprintf("mouse: %S\n",mouse_zgran);

	if( (error = MouseDevice->SetCooperativeLevel(hwnd,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE)) ) {
		MouseDevice->Release();
		MouseDevice = NULL;
		return DIENUM_CONTINUE;
	}

	if( (error = MouseDevice->Acquire()) ) {
		MouseDevice->Release();
		MouseDevice = NULL;
		return DIENUM_CONTINUE;
	}

	xprintf("init: \"%s\" directx mouse.\n", pdinst->tszProductName );

	return DIENUM_STOP;
}





static HINSTANCE DIHinst = NULL;


//
//
//
void DX_Setup( void ) {

	HRESULT error;
	int i;

	// FIXME
	if( !winNoDinput() ) {

		if( (DIHinst = LoadLibrary( "DINPUT.DLL" )) == NULL ) {
			// No DInput... must be DX2 on NT 4 pre-SP3
			xprintf( "DX_Setup: Couldn't LoadLibrary dinput.dll.\n" );
		}
		else
		if( (DICreate = (DIRECTINPUTCREATE)GetProcAddress( DIHinst, "DirectInputCreateA" )) == NULL ) {
			// No DInput... must be pre-SP3 DX2
			xprintf( "DX_Setup: Couldn't GetProcAddress DInputCreate.\n" );
		}
		else
		if( (error = DICreate( hInst, DIRECTINPUT_VERSION, &DirectInput, NULL )) )
			xprintf("DX_Setup: no directx input v%d.%02d.\n",(DIRECTINPUT_VERSION>>8)&0xff,DIRECTINPUT_VERSION&0xff);
	}

	// Keyboard Init

	if( !winNoDinput() && DirectInput )
		DirectInput->EnumDevices( DIDEVTYPE_KEYBOARD, InitKeyboardInput, DirectInput, DIEDFL_ATTACHEDONLY );

	if( KeyboardDevice == NULL )
		xprintf("init: win32 keyboard started.\n");

	extern BOOL CapsLock;
	extern BOOL NumLock;
	extern BOOL ScrollLock;

	NumLock = BOOLEAN( GetKeyState( VK_NUMLOCK ) );
	ScrollLock = BOOLEAN( GetKeyState( VK_SCROLL ) );
	CapsLock = BOOLEAN( GetKeyState( VK_CAPITAL ) );

	/***
	// force numlock on
	if( GetKeyState(VK_NUMLOCK) == 0 ) {
		INPUT input;
		size_t sz = sizeof(INPUT);
		memset(&input, 0, sz);
		input.type   = INPUT_KEYBOARD;
		input.ki.wVk = VK_NUMLOCK;
		SendInput(1, &input, sz);
	}
	***/

	// Mouse Init

	if( !winNoDinput() && DirectInput )
		DirectInput->EnumDevices( DIDEVTYPE_MOUSE, InitMouseInput, DirectInput, DIEDFL_ATTACHEDONLY );

	if( MouseDevice == NULL )
		XLIB_HookMouseProc();

	// Joystick Init
	if( winJoystick() == TRUE )
		XLIB_RefreshJoystick();

	joyHasZ = FALSE;

	maxjoyx = maxjoyy = maxjoyz = 1000;
	joyx = joyy = joyz = joydx = joydy = joydz = maxjoyx/2;
	joypov = 0;

	for( i=0; i<32; i++ ) {
		joyb[i] = FALSE;
		joybd[i] = FALSE;
	}
	// SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

	// HookKeyboardProc( (HINSTANCE)GetHinstance() );
	// HookMouseProc( (HINSTANCE)GetHinstance() );

	return;
}





//
//
//
void DX_Cleanup( void ) {

	HRESULT error;

	CleanupTimer();

	if( KeyboardDevice ) {

		if( (error=KeyboardDevice->Unacquire()) )
			xprintf("DX_Cleanup: Keyboard->Unacquire failed (%s).\n",DX_Error(error));
		if( (error=KeyboardDevice->Release()) )
			xprintf("DX_Cleanup: Keyboard->Release failed (%s).\n",DX_Error(error));

		KeyboardDevice = NULL;
	}

	// XLIB_UnHookKeyboardProc();
	XLIB_UnHookMouseProcLL();

	XLIB_UnHookMouseProc();

	if( MouseDevice ) {

		if( (error=MouseDevice->Unacquire()) )
			xprintf("DX_Cleanup: Mouse->Unacquire failed (%s).\n",DX_Error(error));
		if( (error=MouseDevice->Release()) )
			xprintf("DX_Cleanup: Mouse->Release failed (%s).\n",DX_Error(error));

		MouseDevice = NULL;
	}

	if( DirectInput ) {

		if( (error=DirectInput->Release()) )
			xprintf("DX_Cleanup: DirectInput->Release failed (%s).\n",DX_Error(error));

		DirectInput = NULL;
	}

	if( DIHinst ) VAPI( FreeLibrary( DIHinst ) );
	DIHinst = NULL;

	return;
}




static HINSTANCE hDll = NULL;



//
//
//
void UnloadGFXDLL( void ) {

	if( hDll ) VAPI( FreeLibrary( hDll ) );
	hDll = NULL;

	GFXDLL_GetInfo = NULL;
	GFXDLL_SetupCulling = NULL;
	GFXDLL_AddToPipeline = NULL;
	GFXDLL_AddTexMapHigh = NULL;
	GFXDLL_ReloadTexMapHigh = NULL;
	GFXDLL_ModifyTexMapHigh = NULL;
	GFXDLL_PutSpritePoly = NULL;
	GFXDLL_Line = NULL;
	GFXDLL_PutPixel = NULL;
	GFXDLL_PutPoly = NULL;
	GFXDLL_DiscardAllTexture = NULL;
	GFXDLL_DiscardTexture = NULL;
	GFXDLL_BeginScene = NULL;
	GFXDLL_EndScene = NULL;
	GFXDLL_FlushScene = NULL;
	GFXDLL_GetDescription = NULL;
	GFXDLL_GetData = NULL;
	GFXDLL_LockLfb = NULL;
	GFXDLL_UnlockLfb = NULL;
	GFXDLL_Init = NULL;
	GFXDLL_Deinit = NULL;
	GFXDLL_GetPitch = NULL;
	GFXDLL_Activate = NULL;
	GFXDLL_FlipPage = NULL;
	GFXDLL_Fullscreen = NULL;
	GFXDLL_Clear = NULL;
	GFXDLL_Fog = NULL;
	GFXDLL_InitPalette = NULL;
	GFXDLL_SetRgb = NULL;
	GFXDLL_GetRgb = NULL;

	return;
}


//
//
//
BOOL LoadGFXDLL( char *dllname ) {

	GFXDLLinfo_t info;

	if( (hDll = LoadLibraryEx( dllname, NULL, 0L )) == NULL )
		return FALSE;

	if( (GFXDLL_GetInfo = (void (GFXDLLCALLCONV*)(GFXDLLinfo_t*))GetProcAddress( hDll, "GFXDRV_GetInfo" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		return FALSE;
	}

	GFXDLL_GetInfo( &info );

	GFXDLL_Init = (BOOL (GFXDLLCALLCONV *)(gfxdlldesc_t*))info.GFXDLL_Init;
	GFXDLL_Deinit = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_Deinit;
	GFXDLL_GetDescription = (void (GFXDLLCALLCONV *)(char *))info.GFXDLL_GetDescription;
	GFXDLL_GetData = (void (GFXDLLCALLCONV *)(void *, int))info.GFXDLL_GetData;
	GFXDLL_SetupCulling = (void (GFXDLLCALLCONV *)(point3_t,point3_t,point3_t,FLOAT,FLOAT))info.GFXDLL_SetupCulling;
	GFXDLL_AddToPipeline = (int (GFXDLLCALLCONV *)( trans2pipeline_t ))info.GFXDLL_AddToPipeline;
	GFXDLL_AddTexMapHigh = (int (GFXDLLCALLCONV *)(texmap_t*))info.GFXDLL_AddTexMapHigh;
	GFXDLL_ReloadTexMapHigh = (int (GFXDLLCALLCONV *)(texmap_t*))info.GFXDLL_ReloadTexMapHigh;
	GFXDLL_ModifyTexMapHigh = (int (GFXDLLCALLCONV *)(texmap_t*))info.GFXDLL_ModifyTexMapHigh;
	GFXDLL_PutSpritePoly = (void (GFXDLLCALLCONV *)(polygon_t, point3_t *, int , rgb_t *))info.GFXDLL_PutSpritePoly;
	GFXDLL_PutPoly = (void (GFXDLLCALLCONV *)(polygon_t, rgb_t *))info.GFXDLL_PutPoly;
	GFXDLL_Line = (void (GFXDLLCALLCONV *)(int, int, int, int, rgb_t))info.GFXDLL_Line;
	GFXDLL_PutPixel = (void (GFXDLLCALLCONV *)(int, int, rgb_t))info.GFXDLL_PutPixel;
	GFXDLL_DiscardAllTexture = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_DiscardAllTexture;
	GFXDLL_DiscardTexture = (void (GFXDLLCALLCONV *)(int))info.GFXDLL_DiscardTexture;
	GFXDLL_BeginScene = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_BeginScene;
	GFXDLL_EndScene = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_EndScene;
	GFXDLL_FlushScene = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_FlushScene;
	GFXDLL_LockLfb = (BOOL (GFXDLLCALLCONV *)( __int64 *, int))info.GFXDLL_LockLfb;
	GFXDLL_UnlockLfb = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_UnlockLfb;
	GFXDLL_GetPitch = (ULONG (GFXDLLCALLCONV *)(void))info.GFXDLL_GetPitch;
	GFXDLL_Activate = (BOOL (GFXDLLCALLCONV *)(int))info.GFXDLL_Activate;
	GFXDLL_FlipPage = (void (GFXDLLCALLCONV *)(void))info.GFXDLL_FlipPage;
	GFXDLL_Fullscreen = (BOOL (GFXDLLCALLCONV *)(int))info.GFXDLL_Fullscreen;
	GFXDLL_Clear = (void (GFXDLLCALLCONV *)(ULONG,ULONG,int))info.GFXDLL_Clear;
	GFXDLL_Fog = (void (GFXDLLCALLCONV *)(FLOAT, FLOAT , ULONG ))info.GFXDLL_Fog;
	GFXDLL_InitPalette = (void (GFXDLLCALLCONV *)(UCHAR*))info.GFXDLL_InitPalette;
	GFXDLL_SetRgb = (void (GFXDLLCALLCONV *)(int,int,int,int))info.GFXDLL_SetRgb;
	GFXDLL_GetRgb = (void (GFXDLLCALLCONV *)(int,int*,int*,int*))info.GFXDLL_GetRgb;

	if( GFXDLL_GetData == NULL ) {
		UnloadGFXDLL();
		return FALSE;
	}

	int i = GFXDLL_ISDEBUG;

	GFXDLL_GetData( &i, sizeof(int) );

#ifdef _DEBUG
	if( i == FALSE ) {
		UnloadGFXDLL();
		return FALSE;
	}
#else
	if( i == TRUE ) {
		UnloadGFXDLL();
		return FALSE;
	}
#endif

	i = GFXDLL_ENABLED;

	GFXDLL_GetData( &i, sizeof(int) );

	if( i == FALSE ) {
		UnloadGFXDLL();
		return FALSE;
	}

/*
	if( (GFXDLL_Init = (BOOL (GFXDLLCALLCONV *)(gfxdlldesc_t*))GetProcAddress( hDll, "GFXDLL_GetInfo" )) == NULL ) {
		VAPI( FreeLibrary( hDll ) );
		return FALSE;
	}
*/

	return TRUE;
}






//
//
//
static void DX_Chooser( char *dll_name, int *width, int *height, int *depth, int *flag ) {

	int i,num,cursel;
	combobox_t combobox;
	findfile_t data;
	char str[MAX_PATH],dirname[MAX_PATH] = ".";
	BOOL mustinit, dll_exist;

	winSetSplashText( "Graphics driver..." );

	// ablak nélküi DirectX választás
	if( *flag == SETX_FORCEDX ) {

		sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XD3D_DRV_FILENAME );

		xprintf("DX_Chooser: forced \"%s\" driver.\n", GetFilenameNoPath(dll_name) );

		if( access(dll_name,F_OK) )
			winHideSplash();
		else
			return;
	}

	if( *flag == SETX_FORCEOGL ) {

		sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XOGL_DRV_FILENAME );

		xprintf("DX_Chooser: forced \"%s\" driver.\n", GetFilenameNoPath(dll_name) );

		if( access(dll_name,F_OK) )
			winHideSplash();
		else
			return;
	}

	if( *flag == SETX_FORCESFT ) {

		sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XSFT_DRV_FILENAME );

		xprintf("DX_Chooser: forced \"%s\" driver.\n", GetFilenameNoPath(dll_name) );

		if( access(dll_name,F_OK) )
			winHideSplash();
		else
			return;
	}

	if( *flag == SETX_FORCESDL ) {

		sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XSDL_DRV_FILENAME );

		xprintf("DX_Chooser: forced \"%s\" driver.\n", GetFilenameNoPath(dll_name) );

		if( access(dll_name,F_OK) )
			winHideSplash();
		else
			return;
	}

	if( *flag == SETX_FORCEGLD ) {

		sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XGLD_DRV_FILENAME );

		xprintf("DX_Chooser: forced \"%s\" driver.\n", GetFilenameNoPath(dll_name) );

		if( access(dll_name,F_OK) )
			winHideSplash();
		else {
			winSetFullscreen( TRUE );	// glide csak fullscreenbe megy
			return;
		}
	}


	// ha random legyen, FUNKY STUFF
	if( *flag == SETX_FORCERND || *flag == SETX_NOBORDER ) {

		int rnd = GetSec() % 2;		// only DX, OGL
		// int rnd = GetSec() % 5;

		     if( rnd == 0 ) sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XD3D_DRV_FILENAME );
		else if( rnd == 1 ) sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XOGL_DRV_FILENAME );
		else if( rnd == 2 ) sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XSFT_DRV_FILENAME );
		else if( rnd == 3 ) sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XSDL_DRV_FILENAME );
		else if( rnd == 4 ) sprintf( dll_name, "%s%c%s", GetExeDir(), PATHDELIM, XGLD_DRV_FILENAME );
		else Quit("WTF?!");

		xprintf("DX_Chooser: forced random \"%s\" driver.\n", GetFilenameNoPath(dll_name) );

		if( access(dll_name,F_OK) )
			winHideSplash();
		else
			return;
	}

	winWaitCursor();

	num = cursel = 0;

	getcwd( dirname, MAX_PATH );

	chdir( GetExeDir() );

	if( FindFirst( "*.drv", &data ) ) {

		winInitComboBox();

		do {
			if( LoadGFXDLL( data.name ) ) {
				strlwr(data.name);
				GFXDLL_GetDescription(str);
				sprintf( combobox.str[num][0], "%s (%s)", str, data.name );
				sprintf( combobox.str[num][1], "%s%c%s", GetExeDir(), PATHDELIM, data.name );
				UnloadGFXDLL();
				winComboBoxAddString( combobox.str[num][0] );
				if( !stricmp( data.name, XD3D_DRV_FILENAME) )
					cursel = num;
				++num;
			}
		} while( FindNext( &data ) == TRUE );

		MyFindClose();
	}

	chdir( dirname );

	if( !num ) {
		winHideSplash();
		Quit("No graphics drivers found!\n\nPlease reinstall the application!\n\nThank you!");
	}

	XLIB_winReadProfileString( "xlib_gfxdll", "fIRESTARTER", str, XMAX_PATH );

	mustinit = TRUE;

	if( XLIB_winReadProfileInt( "xlib_nosetup", 0 ) > 0 )
		mustinit = FALSE;

	// ha -nosetup hívta meg
	if( CheckParm("nosetup") > 0 )
		mustinit = FALSE;

	// ha -setup hívta meg
	if( CheckParm("setup") > 0 ) {
		XLIB_winWriteProfileInt( "xlib_nosetup", 0 );
		mustinit = TRUE;
	}

	dll_exist = FALSE;


	// ha az elsõ indítás
	if( !stricmp( str, "fIRESTARTER" ) ) {
		strcpy( str, combobox.str[cursel][1] );
		mustinit = TRUE;
	}
	else
		for( i=0; i<num; i++ )
			if( !stricmp( combobox.str[i][1], str ) ) {
				cursel = i;
				dll_exist = TRUE;
				break;
			}

	if( (dll_exist == FALSE) || (mustinit == TRUE) ) {
		winHideSplash();
		if( (cursel = winComboBox( cursel, width, height, depth, flag )) == -1 )
			Quit( NULL );
	}
	else {
		if( (*width == -1) || (*height == -1) ) {
			int b;
			winCurrScreenRes( width, height, &b );
			CLAMPMINMAX( b, 16, 24 );
			*depth = -b;
		}
	}

	strcpy( dll_name, combobox.str[cursel][1] );

	XLIB_winWriteProfileString( "xlib_gfxdll", combobox.str[cursel][1] );

	winNormalCursor();

	// glide csak fullscreenbe megy
	if( stristr( dll_name, XGLD_DRV_FILENAME ) )
		winSetFullscreen( TRUE );

	return;
}


extern HWND loghwnd;
extern HWND backhwnd;


//
//
//
__int64 GetHwnd( void ) {

	if( !hwnd ) {
		__int64 back = GetBackHwnd();
		return back?back:(__int64)loghwnd;
	}

	return (__int64)hwnd;
}


//
//
//
__int64 GetHinstance( void ) {

	return (__int64)hInst;
}



//
// TRUE: savelhet
// FALSE: nem
//
static BOOL bSavePos = FALSE;


//
//
//
BOOL DX_SaveWindowPos( void ) {

	// lehet-e savelni
	if( bSavePos == FALSE )
		return TRUE;

	int maxx = XLIB_winGetMaxWidth();
	int maxy = XLIB_winGetMaxHeight();

	// saving
	if( (SCREENW < maxx) && (SCREENH < maxy) ) {

		RECT rc;
		POINT point;

		if( !GetClientRect( hwnd, &rc ) )
			return FALSE;

		point.x = rc.left;
		point.y = rc.top;

		if( !ClientToScreen(hwnd, &point) )
			return FALSE;

		rc.left = point.x;
		rc.top = point.y;

		XLIB_winWriteProfileInt( "xlib_window_x", rc.left );
		XLIB_winWriteProfileInt( "xlib_window_y", rc.top );

		XLIB_winWriteProfileInt( "xlib_window_w", SCREENW );
		XLIB_winWriteProfileInt( "xlib_window_h", SCREENH );
	}

	return TRUE;
}




static void HLineX( int,int,int,ULONG );
static void VLineX( int,int,int,ULONG );

static void PutPixelHigh(int,int,ULONG);
static ULONG GetPixelHigh(int,int);
static void PutSpriteHigh( int x1, int y1, UCHAR *spr );
static void GetSpriteHigh( memptr_ptr spr, int x1, int y1, int w, int h );


static void PutPixelX(int,int,ULONG);
static ULONG GetPixelX(int,int);
static void PutSpriteX( int x1, int y1, UCHAR *spr );
static void GetSpriteX( memptr_ptr spr, int x1, int y1, int w, int h );

// typedef HRESULT (*funcGetDXVersion)( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );
static HRESULT (GFXDLLCALLCONV *pGetDXVersion)( DWORD* pdwDirectXVersion, char *strDirectXVersion, int cchDirectXVersion ) = NULL;

// if (!(fnglAlphaFunc     = (void*)GetProcAddress(openglInst, "glAlphaFunc"))) /* return */ errstr("glAlphaFunc");


//
//
//
BOOL XLIB_SetX( int width, int height, int depth, int flag ) {

	int i = 0;
	gfxdlldesc_t desc;
	char dll_name[MAX_PATH];
	HINSTANCE HGetDxver = NULL;
	char *dxhelper = "DXHELPER.DLL";
	char strDXVersion[256] = "\"hope for the best\"";

	if( flag == SETX_FORCERND || flag == SETX_NOBORDER ) {
		bSavePos = TRUE;
		winSetFullscreen( FALSE );
	}
	else
		bSavePos = FALSE;

	winSetSplashText( "Registering variables..." );

	CV_RegisterVariable( &zclipnear );
	CV_RegisterVariable( &zclipfar );
	CV_RegisterVariable( &gamma_corr );

	gamma_corr.value = (FLOAT)XLIB_winReadProfileInt( "xlib_gamma", 100 );

	setx_flag = flag;

	DWORD dwDXVersion = DIRECTDRAW_VERSION;
	int req_dx_version = DIRECTDRAW_VERSION;

	winSetSplashText( "DX version..." );

	if( (HGetDxver = LoadLibraryEx( dxhelper, NULL, 0L )) != NULL ) {
		if( (pGetDXVersion = (HRESULT (GFXDLLCALLCONV *)(DWORD* pdwDirectXVersion, char *strDirectXVersion, int cchDirectXVersion))GetProcAddress( HGetDxver, "GetDXVersion" )) != NULL ) {

			(*pGetDXVersion)( &dwDXVersion, strDXVersion, sizeof(strDXVersion) );
			pGetDXVersion = NULL;

			VAPI( FreeLibrary( HGetDxver ) );

			// xprintf("log: DirectX v%d.%d detected.\n",HIBYTE(dwDXVersion),LOBYTE(dwDXVersion));
			xprintf("log: DirectX v%s or greater detected.\n",strDXVersion);
		}
		else
			xprintf( "SetX: couldn't GetProcAddress GetDXVersion().\n" );
	}
	else
		xprintf( "SetX: couldn't LoadLibrary %s.", dxhelper );

	if( dwDXVersion < (DWORD)req_dx_version ) {
		// Message("This application runs better on DirectX v%d.%d or later.", HIBYTE(req_dx_version), LOBYTE(req_dx_version) );
		xprintf("log: compiled for DirectX v%d.%d version.\n", HIBYTE(req_dx_version), LOBYTE(req_dx_version) );
	}

	dll_name[0] = 0;
	DX_Chooser( dll_name, &width, &height, &depth, &flag );

	// XLIB_winWriteProfileInt( "width", width );
	// XLIB_winWriteProfileInt( "height", height );

	SCREENW = width;
	SCREENH = height;

	CLIPMINX = 0;
	CLIPMINY = 0;
	CLIPMAXX = SCREENW-1;
	CLIPMAXY = SCREENH-1;

	bpp = 16;

	if( !LoadGFXDLL( dll_name ) ) {
		char str[1024];
		sprintf( str, "This application is not properly installed or damaged.\n\nThe Error is:\ncan't load \"%s\" graphics driver.\n\nPlease reinstall the application! Or contact us at Gamesfrom21.webs.com.\n\nThank you!",dll_name );
		MessageBox( NULL, str, "Overly Serious Error", MB_OK|MB_ICONERROR );
		exit(13);
		// Quit(NULL);
	}

	DX_SetupWindow( width, height, depth, flag );
	SetupTimer();

	while( active_flag <= 0 ) {

		MSG msg;

		GetMessage( &msg, hwnd, 0, 0 );
		DispatchMessage( &msg );

		Sleep( 1 );
	}

	if( flag == SETX_JUSTWINDOW )
		return TRUE;

	// xprintf("req: %d,%d, %d bit\n", width, height, depth );

// if( depth < 0 ) depth *= -1;
	// -1 -nél nem kapcsol más felbontásba

	desc.hwnd = (void *)hwnd;
	desc.hInst = (void *)hInst;
	desc.width = width;
	desc.height = height;
	desc.bpp = depth;		// FIXME
	desc.flag = flag;

	desc.x = (GetSystemMetrics( SM_CXSCREEN ) - SCREENW) / 2;
	desc.y = (GetSystemMetrics( SM_CYSCREEN ) - SCREENH) / 2;

	// window pozicó visszatöltése
	// pölö: utils.dps
	if( bSavePos ) {

		int maxx = XLIB_winGetMaxWidth();
		int maxy = XLIB_winGetMaxHeight();

		int savedx = XLIB_winReadProfileInt( "xlib_window_x", -1 );
		int savedy = XLIB_winReadProfileInt( "xlib_window_y", -1 );

		int savedw = XLIB_winReadProfileInt( "xlib_window_w", -1 );
		int savedh = XLIB_winReadProfileInt( "xlib_window_h", -1 );

		// loading
		if( (SCREENW < maxx) && (SCREENH < maxy) &&
		    (savedx != -1) && (savedy != -1) &&
		    (savedw == SCREENW) && (savedh == SCREENH) ) {

			desc.x = savedx;
			desc.y = savedy;
		}
	}


	desc.xprintf			= (void *)xprintf;
	desc.malloc			= (void *)AllocMemNoPtr;
	desc.realloc			= (void *)ReallocMemNoPtr;
	desc.free			= (void *)FreeMemNoPtr;
	desc.Quit			= (void *)winShowQuitDlg;
	desc.GetBackHwnd		= (void *)GetBackHwnd;
	desc.CheckParm			= (void *)CheckParm;
	desc.TexForNum			= (void *)TexForNum;
	desc.CopyVector			= (void *)CopyVector;
	desc.DotProduct			= (void *)DotProduct;
	desc.winFullscreen		= (void *)winFullscreen;
	desc.winReadProfileString	= (void *)XLIB_winReadProfileString;
	desc.winWriteProfileString	= (void *)XLIB_winWriteProfileString;
	desc.winReadProfileInt		= (void *)XLIB_winReadProfileInt;
	desc.winWriteProfileInt		= (void *)XLIB_winWriteProfileInt;
	desc.winOpengl			= (void *)winOpengl;
	desc.GetHwnd			= (void *)GetHwnd;
	desc.GetHinstance		= (void *)GetHinstance;

	winHideSplash();

	if( !GFXDLL_Init( &desc ) ) {

		char errstr[512];

		*(ULONG *)errstr = GFXDLL_GETERROR;

		if( GFXDLL_GetData )
			GFXDLL_GetData( errstr, sizeof(errstr) );
		else
			errstr[0] = 0;

		if( errstr[0] == 0 )
			Quit("It seems there's no %dx%d %dbpp mode for your display.",SCREENW,SCREENH,desc.bpp);
		else
			Quit( errstr );
	}

	// MoveWindow( hwnd, 0,0, GetSystemMetrics( SM_CXSCREEN ),GetSystemMetrics( SM_CYSCREEN ), TRUE );

	DX_Setup();

	bZBuffer = desc.bZBuffer;

	rmask = desc.rmask;
	gmask = desc.gmask;
	bmask = desc.bmask;

	rsize = desc.rsize;
	gsize = desc.gsize;
	bsize = desc.bsize;

	rshift = desc.rshift;
	gshift = desc.gshift;
	bshift = desc.bshift;

	// xprintf("res: %dx%d %dbpp\nrgb: %d:%d:%d (%d:%d:%d)\nmask: %d:%d:%d\n",SCREENW,SCREENH,bpp,rsize,gsize,bsize,rshift,gshift,bshift,rmask,gmask,bmask);
	// Message("res: %dx%d %dbpp\nrgb: %d:%d:%d (%d:%d:%d)\nmask: %d:%d:%d",SCREENW,SCREENH,bpp,rsize,gsize,bsize,rshift,gshift,bshift,rmask,gmask,bmask);

	pixel_len = bpp >> 3;

	memcpy( system_rgb, GetDPpal(), 768 );

	if( bpp == 8 ) GFXDLL_InitPalette( GetDPpal() );

	switch( bpp ) {

		case 16:

			XL_GetPixel	= GetPixelHigh;
			PutSprite	= PutSpriteHigh;
			GetSprite	= GetSpriteHigh;
			PutSpriteNC	= PutSpriteHigh;
			GetSpriteNC	= GetSpriteHigh;

			break;

		case 8:

			XL_GetPixel	= GetPixelX;
			PutSprite	= PutSpriteX;
			GetSprite	= GetSpriteX;
			PutSpriteNC	= PutSpriteX;
			GetSpriteNC	= GetSpriteX;

			break;
	}

	if( line_offset ) FREEMEM( line_offset );
	ALLOCMEM( line_offset, SCREENH * sizeof(__int64) );

	for( i=0; i<SCREENH; i++ )
		line_offset[i] = i * GFXDLL_GetPitch();

	renderinfo.xformframe_cnt = 0;

	// SetWindowText( hwnd, "XLIB_APPLICATION" );

	consoleResize();

	SendMessage( (HWND)GetBackHwnd(), WM_PAINT, 0,0 );

	return TRUE;
}





//
//
//
void XLIB_ResetX( void ) {

	// XLIB_winWriteProfileInt( "xlib_gamma", (int)gamma_corr.value );

	DeinitCdrom();

	DiscardAllTexmap();

	if( GFXDLL_Deinit ) GFXDLL_Deinit();
	DX_Cleanup();
	ResetGraphics();

	UnloadGFXDLL();

	ClipCursor( NULL );
////////	ShowCursor( TRUE );

	// DX_CleanupWindow();

	DeinitTransColor();

	SAFE_FREEMEM( line_offset );

	return;
}






//
//
//
void FrameRateLimiter( int fps ) {

	BOOL slept = FALSE;

	static long fpsStartTime = 0;
	static long fpsFrameCount = 0;

	int freq = TICKBASE;
	int frame = GetTic();

	while( (frame - fpsStartTime) * fps < freq * fpsFrameCount ) {

		int sleepTime = (int)((fpsStartTime * fps + freq * fpsFrameCount - frame * fps) * 1000 / (freq * fps));

		if( sleepTime > 0 ) {
			sleepTime %= (TICKBASE/60);
			Sleep( sleepTime );
			slept = TRUE;
		}

		frame = GetTic();
	}

	if( (++fpsFrameCount > fps) || (fpsStartTime == 0) ) {
		fpsFrameCount = 1;
		fpsStartTime = frame;
	}

	if( slept == FALSE )
		Sleep(1);

	return;
}




//
//
//
void XLIB_MinimalMessagePump( void ) {

	MSG msg;

	Sleep(1);

	if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
		if( GetMessage( &msg, NULL, 0, 0 ) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return;
}



// TODO: ezt egy kicsit kulturáltabban
static BOOL need_toggle = FALSE;


//
// http://www.gamedev.net/page/resources/_/reference/programming/platform-specific/windows/getting-rid-of-the-windows-message-pump-s-r1249
//
void DX_GetMessage( void ) {

	MSG msg;
	int endtic = GetTic() + TICKBASE / 10;

/*	// régi
	// while( PeekMessage( &msg, hwnd, 0, 0, (active_flag?PM_NOYIELD:0) | PM_REMOVE ) ) {
	while( PeekMessage( &msg, hwnd, 0, 0, PM_REMOVE ) ) {
		TranslateMessage(&msg);

		if( msg.message == WM_QUIT )
			Quit( NULL );

		DispatchMessage(&msg);
	}
*/

	// új
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
		if( GetMessage(&msg, NULL, 0, 0) ) {

			TranslateMessage(&msg);
			/***
			// Luarunnernél csak ha log window van
			if( hwnd == NULL && backhwnd == NULL && loghwnd &&
			    msg.message == WM_KEYDOWN &&
			    GetFocus() != loghwnd ) {
				// xprintf("log: %d   active: %d  focus: %d\n",loghwnd,GetForegroundWindow(),GetFocus());
				// ha nem a focusolt a loghwnd
				SendMessage( loghwnd, WM_KEYDOWN, msg.wParam, msg.lParam);
			}
			***/
			DispatchMessage(&msg);

		}

		if( GetTic() > endtic )	break;
	}

	winBrowserCallback();

	if( loghwnd ) UpdateWindow( loghwnd );

	if( active_flag > 0 ) {

		DX_RefreshKey();
		// DX_RefreshMouse();
		// FIXME: ezt ki kell még találni
		// if( (winJoystick() == TRUE) && (ntJoystickDevice > 0) ) XLIB_RefreshJoystick();

		// fullscreen toggle
		// jó-e ha ez itt van?
		if( (keys[sc_Enter] && keys[sc_Alt]) || need_toggle  ) {
			if( ToggleFullscreen() == TRUE )
				StatusText( ST_YELLOW, "Alt-Enter to switch back" );
			ClearKey();
			need_toggle = FALSE;
		}

		/***
		// screenshot a temp-be
		if( keys[sc_PrintScreen] ) {
			ScreenShot();
			ClearKey();
		}
		***/
	}

	FrameRateLimiter( 60 );

	return;
}



//
//
//
void FlipPage( BOOL noflip ) {

	/*
	LockSurface( LS_WRITE );

	if( display_zprintf == TRUE )
		refresh_zprintf( TRUE );

	UnlockSurface();
	*/

	if( active_flag >= 0 ) {

		// PostProcess();

		if( GFXDLL_FlipPage /*&& noflip == FALSE*/ ) GFXDLL_FlipPage();

		// FIXME: jó helyen van itt ez?
		AVI_Frame();
	}

	// FIXME:
	if( noflip == FALSE )
		DX_GetMessage();

	return;
}



//
//
//
BOOL ToggleFullscreen( int flag ) {

	if( flag == -3 ) {
		need_toggle = TRUE;
		return TRUE;
	}

	if( (active_flag == 1) && GFXDLL_Fullscreen ) {

		winSetFullscreen( winFullscreen() ? FALSE : TRUE );

		if( GFXDLL_Fullscreen( winFullscreen() ) == FALSE )
			Quit("Switching screenmode failed.\n\nProgram is unstable. Exiting.");

		Sleep(1);

		SetMouseClip( winFullscreen() );

		// ReloadAllTexMap();
	}
	else
		return -1;

	return TRUE;
}





//
//
//
void win_WaitRetrace( int i ) {

	// HRESULT error;

	while( i-- ) {

		Sleep( i * ( 1000000/70 ) );

		//if( (error=DirectDraw->WaitForVerticalBlank( DDWAITVB_BLOCKBEGIN, 0 )) )
		//	Quit("WaitRetrace: VerticalBlank failed.\nError: %s",DX_Error(error));
	}

	return;
}


//
// Oops!
//
void win_WaitVerticalRetraceStart(void) { return; }
void win_WaitVerticalRetraceEnd(void) { return; }
void win_WaitHorizontalRetraceStart(void) { return; }
void win_WaitHorizontalRetraceEnd(void) { return; }


#define INPB _inp
#define STATUS_REGISTER_1 0x3da


//
// vertical
//
void WaitRetrace( int i ) {
/*
#ifdef __GNUC__
	asm("   movw $0x3da,%%dx
		l1:
		inb %%dx,%%al
		andb $1,%%al
		jnz l1
		l2:
		inb %%dx,%%al
		andb $1,%%al
		jz l2
		loop l1"
		:
		: "c" (i)
		: "ax","dx"
	);
#else
  	while( i-- ) {
		while((INPB(STATUS_REGISTER_1)&8)!=0);	       // amig display
		while((INPB(STATUS_REGISTER_1)&8)==0);	       // retrace kezdet
	}
#endif
*/
	return;
}




//
// Vertical Start!
//
void WaitVerticalRetraceStart( void ) {
/*
#ifdef __GNUC__
	asm("   movw $0x3da,%%dx
		l3:
		inb %%dx,%%al
		andb $1,%%al
		jnz l3
		l4:
		inb %%dx,%%al
		andb $1,%%al
		jz l4"
		:
		:
		: "ax","dx"
	);
#else
	while((INPB(STATUS_REGISTER_1)&8)!=0);
	while((INPB(STATUS_REGISTER_1)&8)==0);
#endif
*/
	return;
}


//
// Vertical End!
//
void WaitVerticalRetraceEnd( void ) {
/*
#ifdef __GNUC__
	asm("   movw $0x3da,%%dx
		l5:
		inb %%dx,%%al
		andb $1,%%al
		jz l5
		l6:
		inb %%dx,%%al
		andb $1,%%al
		jnz l6"
		:
		:
		: "ax","dx"
	);
#else
	while((INPB(STATUS_REGISTER_1)&8)==0);
	while((INPB(STATUS_REGISTER_1)&8)!=0);
#endif
*/
	return;
}



//
// horizontal Start!
//
void WaitHorizontalRetraceStart( void ) {

#ifndef __GNUC__
	while((INPB(STATUS_REGISTER_1)&1)!=0);
	while((INPB(STATUS_REGISTER_1)&1)==0);
#endif

	return;
}


//
// Horizontal End!
//
void WaitHorizontalRetraceEnd( void ) {

#ifndef __GNUC__
	while((INPB(STATUS_REGISTER_1)&1)==0);
	while((INPB(STATUS_REGISTER_1)&1)!=0);
#endif

	return;
}






//
// use the blter to do a color fill to clear the back buffer
//
void Clear( ULONG color ) {

	// UnlockSurface(); ez kell-e?

	if( GFXDLL_Clear && (active_flag>=0) ) GFXDLL_Clear( color, 0xffff, CF_TARGET | CF_ZBUFFER );

	return;
}



//
//
//
void ClearDepth( ULONG depth ) {

	if( GFXDLL_Clear && (active_flag>=0) ) GFXDLL_Clear( 0, depth, CF_ZBUFFER );

	return;
}



//
//
//
void *GetVideoPtr( int x, int y ) {

	if( active_flag < 0 ) return NULL;

	UCHAR *video = (UCHAR*)gfxmem;

	return &video[ line_offset[y] +  (x *  pixel_len) ];
}


/************************************************
 *						*
 *	256 Color				*
 *	Interface				*
 *						*
 ************************************************/



//
//
//
static void PutPixelX( int x, int y, ULONG c ) {

	CHECKACTIVE;

	CLIPPANIC(x,y);

	*(UCHAR*)( gfxmem + line_offset[y]+x) = (UCHAR)c;

	return;
}



//
//
//
static ULONG GetPixelX( int x, int y ) {

	ULONG color = 0x0000;

	if( active_flag < 0 ) return color;

	CLIPPANIC0(x,y);

	color = *(UCHAR*)( gfxmem + line_offset[y]+x);

	return color;
}



//
//
//
static void PutSpriteX( int x1, int y1, UCHAR *spr ) {

	int y2,w,h,cw;
	int cim;
	UCHAR *ptr = &spr[SPRITEHEADER];

	CHECKACTIVE;

	if( !ISSPR8(spr) ) {
		GenPutSprite( x1,y1, spr );
		return;
	}

	cw = w = SPRITEW( spr );	// cw clipped width
	h = SPRITEH( spr );

	++CLIPMAXX;
	++CLIPMAXY;
#define BACK { --CLIPMAXX; --CLIPMAXY; return; }


	if( x1<CLIPMINX ) {
		if( (x1+w)<CLIPMINX )
			BACK
		else {
			ptr += (CLIPMINX-x1);
			cw -= (CLIPMINX-x1);
			x1 = CLIPMINX;
		}
	}

	if( (x1+cw)>CLIPMAXX ) {
		if( x1>CLIPMAXX )
			BACK
		else
			cw -= ( (x1+cw) - CLIPMAXX );
	}


	if( y1<CLIPMINY ) {
		if( (y1+h)<CLIPMINY )
			BACK
		else {
			ptr += ( w*(CLIPMINY-y1) );
			h -= (CLIPMINY-y1);
			y1 = CLIPMINY;
		}
	}

	y2 = y1 + h;
	if( y2>CLIPMAXY ) {
		if( y1>CLIPMAXY )
			BACK
		else
			y2 = CLIPMAXY;
	}

	--CLIPMAXX;
	--CLIPMAXY;
#undef BACK

	LockSurface( LS_WRITE );

	cim = gfxmem + line_offset[y1] + x1;

	if( GFXDLL_GetPitch )
	for( ; y1<y2; y1++ ) {

		memcpy( (UCHAR*)cim, ptr, cw );
		cim += GFXDLL_GetPitch();
		ptr += w;
	}

	UnlockSurface();

	return;
}




//
//
//
static void GetSpriteX( memptr_ptr spr, int x1, int y1, int w, int h ) {

	int y2,cw;
	int cim;
	UCHAR *ptr;

	CHECKACTIVE;

	ALLOCMEM( *spr, (w*h)+SPRITEHEADER );
	MKSPRW( *spr, w );
	MKSPRH( *spr, h );
	MKSPR8( *spr );

	ptr = &(*spr)[SPRITEHEADER];
	memset( ptr, 0L, w*h );
	cw = w;

	++CLIPMAXX;
	++CLIPMAXY;

#define BACK { --CLIPMAXX; --CLIPMAXY; return; }


	if( x1<CLIPMINX ) {
		if( (x1+w)<CLIPMINX )
			BACK
		else {
			ptr += (CLIPMINX-x1);
			cw -= (CLIPMINX-x1);
			x1 = CLIPMINX;
		}
	}

	if( (x1+cw)>CLIPMAXX ) {
		if( x1>CLIPMAXX )
			BACK
		else
			cw -= ( (x1+cw) - CLIPMAXX );
	}


	if( y1<CLIPMINY ) {
		if( (y1+h)<CLIPMINY )
			BACK
		else {
			ptr += ( w*(CLIPMINY-y1) );
			h -= (CLIPMINY-y1);
			y1 = CLIPMINY;
		}
	}

	y2 = y1 + h;
	if( y2>CLIPMAXY ) {
		if( y1>CLIPMAXY )
			BACK
		else
			y2 = CLIPMAXY;
	}

	--CLIPMAXX;
	--CLIPMAXY;
#undef BACK

	LockSurface( LS_READ );

	cim = gfxmem + line_offset[y1] + x1;

	if( GFXDLL_GetPitch )
	for( ; y1<y2; y1++ ) {

		memcpy( ptr, (UCHAR*)cim, cw );
		cim += GFXDLL_GetPitch();
		ptr += w;
	}

	UnlockSurface();

	return;
}




//
//
//
static void HLineX( int x1, int len, int y1, ULONG color ) {

	int cim;

	CHECKACTIVE;

	if( x1<CLIPMINX ) {
		if( (x1+len)<CLIPMINX )
			return;
		else {
			len -= (CLIPMINX-x1);
			x1 = CLIPMINX;
		}
	}

	if( (x1+len)>CLIPMAXX ) {
		if( x1>CLIPMAXX )
			return;
		else
			len -= ( (x1+len) - CLIPMAXX );
	}


	if( y1<CLIPMINY || y1>CLIPMAXY )
		return;

	LockSurface( LS_WRITE );

	cim = gfxmem + line_offset[y1] + x1;

	memset( (UCHAR*)cim, color, len );

	UnlockSurface();

	return;
}




//
//
//
static void VLineX( int x1, int y1, int len, ULONG color ) {

	int y2;
	int cim;

	CHECKACTIVE;

	if( x1<CLIPMINX || x1>CLIPMAXX )
		return;


	if( y1<CLIPMINY ) {
		if( (y1+len)<CLIPMINY )
			return;
		else {
			len -= (CLIPMINY-y1);
			y1 = CLIPMINY;
		}
	}

	y2 = y1 + len;
	if( y2>CLIPMAXY ) {
		if( y1>CLIPMAXY )
			return;
		else
			y2 = CLIPMAXY;
	}

	LockSurface( LS_WRITE );

	cim = gfxmem + line_offset[y1] + x1;

	if( GFXDLL_GetPitch )
	for( ; y1<y2; y1++ ) {

		*(UCHAR*)cim = (UCHAR)color;
		cim += GFXDLL_GetPitch();

	}

	UnlockSurface();

	return;
}










/****************************************************
 *													*
 *	HIGH Color										*
 *	Interface										*
 *													*
 ****************************************************/


//
//
//
static void PutPixelHigh( int x, int y, ULONG color ) {

	ULONG cim;

	CHECKACTIVE;

	CLIPPANIC(x,y);

	// cim = gfxmem + line_offset[y] + ( pixel_len * x );
	cim = gfxmem + GFXDLL_GetPitch()*y + ( pixel_len * x );

	*(USHORT*)cim = (USHORT)color;

	return;
}





//
//
//
static ULONG GetPixelHigh( int x, int y ) {

	ULONG cim;

	if( active_flag < 0 ) return NOCOLOR;

	CLIPPANIC0(x,y);

	// cim = gfxmem + line_offset[y] + ( pixel_len * x );
	cim = gfxmem + GFXDLL_GetPitch()*y + ( pixel_len * x );

	return (ULONG)(*((USHORT*)cim));
}




//
//
//
static void PutSpriteHigh( int x1, int y1, UCHAR *spr ) {

	int y2,w,h,cw;
	int cim,pitch;
	UCHAR *ptr = &spr[SPRITEHEADER];

	CHECKACTIVE;

	if( !ISSPR16(spr) ) {
		GenPutSprite( x1,y1, spr );
		return;
	}


#define BACK { --CLIPMAXX; --CLIPMAXY; return; }

	cw = w = SPRITEW( spr );	// cw clipped width
	h = SPRITEH( spr );

	++CLIPMAXX;
	++CLIPMAXY;


	if( x1<CLIPMINX ) {
		if( (x1+w)<CLIPMINX )
			BACK
		else {
			ptr += (CLIPMINX-x1)*pixel_len;
			cw -= (CLIPMINX-x1);
			x1 = CLIPMINX;
		}
	}

	if( (x1+cw)>CLIPMAXX ) {
		if( x1>CLIPMAXX )
			BACK
		else
			cw -= ( (x1+cw) - CLIPMAXX );
	}


	if( y1<CLIPMINY ) {
		if( (y1+h)<CLIPMINY )
			BACK
		else {
			ptr += ( w*(CLIPMINY-y1)*pixel_len );
			h -= (CLIPMINY-y1);
			y1 = CLIPMINY;
		}
	}

	y2 = y1 + h;
	if( y2>CLIPMAXY ) {
		if( y1>CLIPMAXY )
			BACK
		else
			y2 = CLIPMAXY;
	}

#undef BACK

	--CLIPMAXX;
	--CLIPMAXY;

	LockSurface( LS_WRITE );

	cim = gfxmem + line_offset[y1] + x1*pixel_len;
	pitch = GFXDLL_GetPitch();

	cw *= pixel_len;
	w *= pixel_len;

	for( ; y1<y2; y1++ ) {

		memmove( (UCHAR*)(cim), ptr, cw );
		cim += pitch;
		ptr += w;
	}

	UnlockSurface();

	return;
}






//
//
//
static void GetSpriteHigh( memptr_ptr spr, int x,int y,int w,int h) {

	int x1,y1,y2,cw;
	int cim;
	UCHAR *ptr;

	CHECKACTIVE;

#define BACK { --CLIPMAXX; --CLIPMAXY; return; }

	ALLOCMEM( *spr, SPRITEHEADER+(w*h*pixel_len) );
	MKSPRW(*spr,w);
	MKSPRH(*spr,h);
	MKSPR16( *spr );

	ptr = &(*spr)[SPRITEHEADER];

	cw = w;        // cw clipped width

	++CLIPMAXX;
	++CLIPMAXY;

	x1 = x;
	y1 = y;

	if( x1<CLIPMINX ) {
		if( (x1+w)<CLIPMINX )
			BACK
		else {
			ptr += (CLIPMINX-x1)*pixel_len;
			cw -= (CLIPMINX-x1);
			x1 = CLIPMINX;
		}
	}

	if( (x1+cw)>CLIPMAXX ) {
		if( x1>CLIPMAXX )
			BACK
		else
			cw -= ( (x1+cw) - CLIPMAXX );
	}


	if( y1<CLIPMINY ) {
		if( (y1+h)<CLIPMINY )
			BACK
		else {
			ptr += ( w*(CLIPMINY-y1)*pixel_len );
			h -= (CLIPMINY-y1);
			y1 = CLIPMINY;
		}
	}

	y2 = y1 + h;
	if( y2>CLIPMAXY ) {
		if( y1>CLIPMAXY )
			BACK
		else
			y2 = CLIPMAXY;
	}

#undef BACK

	--CLIPMAXX;
	--CLIPMAXY;

	LockSurface( LS_READ );

	cim = gfxmem + line_offset[y1] + x1*pixel_len;

	cw *= pixel_len;
	w *= pixel_len;

	if( GFXDLL_GetPitch )
	for( ; y1<y2; y1++ ) {

		memmove( ptr, (UCHAR*)(cim), cw );
		cim += GFXDLL_GetPitch();
		ptr += w;
	}

	UnlockSurface();

	return;
}





//
//
//
long GetKeyDX( void ) {

	DIDEVICEOBJECTDATA data;
	DWORD amount = 1;
	HRESULT error;

	// if( FAILED(KeyboardDevice->Poll()) )
	//	Quit("Poll failed");

	if( (error=KeyboardDevice->GetDeviceData(sizeof(data),&data,&amount,0)) != DD_OK ) {
		xprintf("GetKeyDX: GetDeviceData failed (%s).\n",DX_Error(error));
		return 0;
	}

	if( amount && data.dwData&0x80 )
		return data.dwOfs;

	return 0;
}








//
//
//
int translateWin32Text( int kc ) {

	typedef int (WINAPI *LPFN_FOLDSTRING)( DWORD,LPCWSTR,int,LPWSTR,int );
	static LPFN_FOLDSTRING pFoldString = (LPFN_FOLDSTRING)GetProcAddress(GetModuleHandle("kernel32"),"FoldStringW");

	typedef int (WINAPI *LPFN_PTOASCII)( UINT,UINT,PBYTE,LPWORD,UINT,HKL );
	static LPFN_PTOASCII pToAscii = (LPFN_PTOASCII)GetProcAddress(GetModuleHandle("user32"),"ToAsciiEx");

	if( pFoldString == NULL || pToAscii == NULL )
		return kc;

	static WCHAR deadKey = 0;

	BYTE keyState[256];
	HKL  layout = GetKeyboardLayout(0);
	if( GetKeyboardState(keyState) == 0 )
		return 0;

	int code = *((int*)&kc);
	unsigned int vk = MapVirtualKeyEx( (UINT)code, 3, layout );
	if( vk == 0 )
		return 0;

	// WCHAR buff[3] = { 0, 0, 0 };
	// int ascii = pToUnicodeEx(vk, (UINT)code, keyState, buff, 3, 0, layout);

	WORD buff[3] = { 0, 0, 0 };
	int ascii = pToAscii( vk, (UINT)code, keyState, (LPWORD)buff,0,layout);

	// xprintf("vk: %d  ascii: %d  buf: %d  sizeof(buf): %d\n", vk, ascii, buff[0], sizeof(buff[0]) );

	if( ascii == 1 && deadKey != '\0' ) {
		// A dead key is stored and we have just converted a character key
		// Combine the two into a single character
		WCHAR wcBuff[3] = { buff[0], deadKey, '\0' };
		WCHAR out[3];

		deadKey = '\0';
		if( pFoldString(MAP_PRECOMPOSED, (LPWSTR)wcBuff, 3, (LPWSTR)out, 3) )
			return out[0];
	}
	else
	if( ascii == 1 ) {
		// We have a single character
		deadKey = '\0';
		return buff[0];
	}
	else
	if( ascii == 2 ) {
		// Convert a non-combining diacritical mark into a combining diacritical mark
		// Combining versions range from 0x300 to 0x36F; only 5 (for French) have been mapped below
		// http://www.fileformat.info/info/unicode/block/combining_diacritical_marks/images.htm
		switch( buff[0] ) {
			case 0x5E: // Circumflex accent: в
				deadKey = 0x302; break;
			case 0x60: // Grave accent: а
				deadKey = 0x300; break;
			case 0xA8: // Diaeresis: ь
				deadKey = 0x308; break;
			case 0xB4: // Acute accent: й
				deadKey = 0x301; break;
			case 0xB8: // Cedilla: з
				deadKey = 0x327; break;
			default:
				deadKey = buff[0]; break;
		}
	}

	return 0;
}





//
//
//
void DX_RefreshKey( void ) {

	if( KeyboardDevice ) {

		DIDEVICEOBJECTDATA rgdod[DEVICEBUFFERSIZE];
		DWORD dwItems = DEVICEBUFFERSIZE;
		HRESULT error;

		if( (error=KeyboardDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0 )) != DD_OK ) {

			if( error == DIERR_INPUTLOST )
				error = KeyboardDevice->Acquire();

			// xprintf("DX_RefreshKey: GetDeviceData failed (%s).\n",DX_Error(error));

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of
			// switching, so just try again later
		}
		else {
			for( int i=0; i<(int)dwItems; i++ )
				DX_keyboard_handler( rgdod[i].dwOfs, rgdod[i].dwData, translateWin32Text(rgdod[i].dwOfs) );

			if( dwItems == 0 )
				XLIB_FeedKey();
		}
	}

	return;
}





//
//
//
void DX_RefreshMouse( void ) {

	mousedx = 0;
	mousedy = 0;
	mousedz = 0;

	// a mousebl és a mousebld kezelés miatt kell
	if( win32_mousebl ) {
		if( mousebld == TRUE ) {
			mousebl = FALSE;
			mousebld = TRUE;
		}
		else {
			mousebl = TRUE;
			mousebld = TRUE;
		}
	}
	else {
		mousebl = FALSE;
		mousebld = FALSE;
	}

	if( win32_mousebm ) {
		if( mousebmd == TRUE ) {
			mousebm = FALSE;
			mousebmd = TRUE;
		}
		else {
			mousebm = TRUE;
			mousebmd = TRUE;
		}
	}
	else {
		mousebm = FALSE;
		mousebmd = FALSE;
	}

	if( win32_mousebr ) {
		if( mousebrd == TRUE ) {
			mousebr = FALSE;
			mousebrd = TRUE;
		}
		else {
			// xprintf("-->hoppá.\n");
			mousebr = TRUE;
			mousebrd = TRUE;
		}
	}
	else {
		mousebr = FALSE;
		mousebrd = FALSE;
	}

	// xprintf("m4: %d, m4d: %d         m5: %d, m5d: %d\n",mouseb4,mouseb4d,mouseb5,mouseb5d);

	// lowlevel 4th
	if( lowlevel_mouseb4 == 1 ) {
		if( mouseb4d == TRUE ) {
			mouseb4 = FALSE;
			mouseb4d = TRUE;
			lowlevel_mouseb4 = 0;
		}
		else {
			mouseb4 = TRUE;
			mouseb4d = TRUE;
		}
	}
	else
	if( lowlevel_mouseb4 == -1 ) {
		mouseb4 = FALSE;
		mouseb4d = FALSE;
	}

	// lowlevel 5th
	if( lowlevel_mouseb5 == 1 ) {
		if( mouseb5d == TRUE ) {
			mouseb5 = FALSE;
			mouseb5d = TRUE;
			lowlevel_mouseb5 = 0;
		}
		else {
			mouseb5 = TRUE;
			mouseb5d = TRUE;
		}
	}
	else
	if( lowlevel_mouseb5 == -1 ) {
		mouseb5 = FALSE;
		mouseb5d = FALSE;
	}

	if( MouseDevice ) {

		DIDEVICEOBJECTDATA rgdod[DEVICEBUFFERSIZE];
		DWORD dwItems=DEVICEBUFFERSIZE;
		HRESULT error;

		if( (error = MouseDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), rgdod, &dwItems, 0 )) != DD_OK ) {

			int i = 0;
			do {
				error = MouseDevice->Acquire();
				if( i++ > 10 )
					break;
			} while( error == DIERR_INPUTLOST );

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of
			// switching, so just try again later

			// xprintf("DX_RefreshMouse: GetDeviceData failed (%s).\n",DX_Error(error));
		}
		else
			for( int i=0; i<(int)dwItems; i++ ) {

				if( rgdod[i].dwOfs == DIMOFS_BUTTON0 )
					if( (win32_mousebl = BOOLEAN(rgdod[i].dwData & 0x80)) == FALSE ) {
						mousebl = FALSE;
						mousebld = FALSE;
					}
					else {
						mousebl = TRUE;
						mousebld = TRUE;
					}

				if( rgdod[i].dwOfs == DIMOFS_BUTTON1 )
					if( (win32_mousebr = BOOLEAN(rgdod[i].dwData & 0x80)) == FALSE ) {
						mousebr = FALSE;
						mousebrd = FALSE;
					}
					else {
						mousebr = TRUE;
						mousebrd = TRUE;
						// xprintf("itten vagyunk.\n");
					}

				if( rgdod[i].dwOfs == DIMOFS_BUTTON2 )
					if( (win32_mousebm = BOOLEAN(rgdod[i].dwData & 0x80)) == FALSE ) {
						mousebm = FALSE;
						mousebmd = FALSE;
					}
					else {
						mousebm = TRUE;
						mousebmd = TRUE;
					}
/*
				if( rgdod[i].dwOfs == DIMOFS_BUTTON3 )
					if( (win32_mousebm = BOOLEAN(rgdod[i].dwData & 0x80)) == FALSE ) {
						mouseb4 = FALSE;
						mouseb4d = FALSE;
					}
					else {
						mouseb4 = TRUE;
						mouseb4d = TRUE;
					}
*/
				// dx6.1 nem támogatja
				/***
				if( rgdod[i].dwOfs == DIMOFS_BUTTON4 )
					if( (win32_mousebm = BOOLEAN(rgdod[i].dwData & 0x80)) == FALSE ) {
						mouseb5 = FALSE;
						mouseb5d = FALSE;
					}
					else {
						mouseb5 = TRUE;
						mouseb5d = TRUE;
					}
				***/

				if( rgdod[i].dwOfs == DIMOFS_X )
					mousedx += rgdod[i].dwData;

				if( rgdod[i].dwOfs == DIMOFS_Y )
					mousedy += rgdod[i].dwData;

				if( rgdod[i].dwOfs == DIMOFS_Z ) {

					signed int integer;

					memcpy( &integer, &rgdod[i].dwData, 4 );

					mousedz += (integer / mouse_zgran);
				}
			}
	}

	/***
	if( MouseDevice ) {

		DIMOUSESTATE MouseState;

		if( FAILED(MouseDevice->GetDeviceState(sizeof(MouseState),&MouseState)) )
			Quit("GetDeviceState failed (mouse)");

		mousedx = MouseState.lX;
		mousedy = MouseState.lY;

		mousebl = MouseState.rgbButtons[0] & 0x80;
		mousebr = MouseState.rgbButtons[1] & 0x80;
		mousebm = MouseState.rgbButtons[2] & 0x80;
	}
	***/


	return;
}



//
//
//
BOOL XLIB_RefreshJoystick( void ) {

	// még nem volt initelve
	if( ntJoystickDevice == -1 && (ntJoystickDevice = joyGetNumDevs()) > 0 ) {

		JOYCAPS jc;

		memset( &jc, 0L, sizeof(JOYCAPS) );

		if( joyGetDevCaps( JOYSTICKID1, &jc, sizeof(JOYCAPS) ) == JOYERR_NOERROR )
			xprintf("init: \"%s\" joystick (%d buttons, %s Z-axis, %s POV).\n",
					jc.szPname,
					jc.wNumButtons,
					jc.wCaps & JOYCAPS_HASZ ? "has" : "No",
					jc.wCaps & JOYCAPS_HASPOV ? "has" : "No" );
		else {
			xprintf("init: no joystick.\n");
			// ne probálkozzon többet
			// ntJoystickDevice = -2;
		}

		joyHasZ = jc.wCaps & JOYCAPS_HASZ ? TRUE : FALSE;

		maxjoyx = maxjoyy = maxjoyz = 1000;
		joyx = joyy = joyz = joydx = joydy = joydz = maxjoyx/2;
		joypov = 0;

		for( int i=0; i<32; i++ ) {
			joyb[i] = FALSE;
			joybd[i] = FALSE;
		}
	}

	// van-e win32-es joy?
	if( ntJoystickDevice <= 0 )
		return FALSE;

	JOYINFOEX ji;

	memset( &ji, 0L, sizeof(JOYINFOEX) );

	ji.dwSize = sizeof(JOYINFOEX);
	ji.dwFlags |= JOY_RETURNALL;

	if( joyGetPosEx( JOYSTICKID1, &ji) == JOYERR_NOERROR ) {

		if( (ji.dwFlags | JOY_RETURNX) )
			joyx = ji.dwXpos * maxjoyx / 0xffff;

		if( (ji.dwFlags | JOY_RETURNY) )
			joyy = ji.dwYpos * maxjoyy / 0xffff;

		if( joyHasZ && (ji.dwFlags | JOY_RETURNZ) )
			joyz = maxjoyz - ji.dwZpos * maxjoyz / 0xffff;

		if( (ji.dwFlags | JOY_RETURNBUTTONS) )
			for( int i=0; i<32; i++ )
				if( BOOLEAN(ji.dwButtons & (1<<i)) ) {
					if( joybd[i] ) {
						joyb[i] = FALSE;
						joybd[i] = TRUE;
					}
					else {
						joyb[i] = TRUE;
						joybd[i] = TRUE;
					}
				}
				else {
					joyb[i] = FALSE;
					joybd[i] = FALSE;
				}

		if( (ji.dwFlags | JOY_RETURNPOV) ) {
			switch( ji.dwPOV ) {
				case JOY_POVBACKWARD: joypov = 4; break;
				default:
				case JOY_POVCENTERED: joypov = 0; break;
				case JOY_POVFORWARD: joypov = 2; break;
				case JOY_POVLEFT: joypov = 3; break;
				case JOY_POVRIGHT: joypov = 1; break;
			}
		}
	}

	return TRUE;
}





#define QUITSTRLEN (2*1024+1)

//
//
//
void Quit( const char *s, ... ) {

	static BOOL sema = FALSE;

	// MessageBox( NULL, "Into Quit()", "Yes", MB_OK | MB_ICONERROR | MB_TOPMOST  );

	if( sema == FALSE ) {

		sema = TRUE;

		char text[QUITSTRLEN],text2[QUITSTRLEN];
		char title[QUITSTRLEN];
		va_list args;

		ClipCursor( NULL );
		ShowCursor( TRUE );

		if( s ) {
			va_start(args, s);
			_vsnprintf(text2,QUITSTRLEN-1, s, args);
			va_end(args);
		}
		else
			strcpy( text2, "Quit." );

		xprintf("*** %s ***\n",text2);

		LoadString( hInst, 106, title, 256 );

		//sprintf(text, "%s\n\n%s", title, text2 );
		sprintf(text, "%s", text2 );

		if( s != NULL ) {
			DB_InitDebug();
			DB_StackWalk();
			DB_DeinitDebug();
		}

		DEINITSOUND;

		DeinitMouse();
		DeinitKeyboard();

		XLIB_ResetX();

		LoadString( hInst, 104, title, 256 );

		ClipCursor( NULL );
		ShowCursor( TRUE );

		if( s ) {
			winHideSplash();
			if( winShowQuitDlg( text2 ) == FALSE )
				MessageBox( GetDesktopWindow(), text, title, MB_OK | /* MB_SYSTEMMODAL | */ MB_ICONERROR | MB_TOPMOST  );
		}
	}
	else {
		char text[QUITSTRLEN];
		va_list args;

		if( s ) {
			va_start(args, s);
			_vsnprintf(text,QUITSTRLEN-1, s, args);
			va_end(args);
		}
		else
			strcpy( text, "bye." );

		MessageBox(NULL, text, "Emergency exit:", MB_OK);
	}

	DeinitLogWindow();

	//FatalAppExit( 0, text );  // gazos

	//FatalExit(1); 	// to debugger

	winWantToQuit( TRUE );

	exit(14);

	return;
}


