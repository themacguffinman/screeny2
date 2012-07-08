#include "Main.h"
#include "Imaging.h"
#include "Errors.h"
#include "WindowManager.h"
#include "resource.h"

#pragma comment(lib, "Comctl32.lib")

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int result;
RECT desktop_rect = {0};

NOTIFYICONDATA main_nid = {0};

HICON systray_icon = NULL;
HICON imgtype_icon = NULL;

HDC backbuffer_dc = NULL;
HBITMAP backbuffer_bitmap = NULL;
HDC desktop_capture_dc = NULL;
HBITMAP desktop_capture_bitmap = NULL;
HDC overlay_dc = NULL;
HBITMAP overlay_bitmap = NULL;
bool click_selection = false;
unsigned int box_x1 = 0;
unsigned int box_y1 = 0;
unsigned int box_x2 = 0;
unsigned int box_y2 = 0;

LRESULT CALLBACK MainWindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_KILLFOCUS:
		printf("killfocus\r\n");
		break;

	case WM_LBUTTONUP:
		/*selection_box.left = box_x1 < box_x2 ? box_x1 : box_x2;
		selection_box.top = box_y1 < box_y2 ? box_y1 : box_y2;
		selection_box.right = box_x1 > box_x2 ? box_x1 : box_x2;
		selection_box.bottom = box_y1 > box_y2 ? box_y1 : box_y2;*/

		printf("Start: %d, %d \t End: %d, %d\r\n", box_x1, box_y1, box_x2, box_y2 );
		click_selection = false;
		break;

	case WM_MOUSEMOVE:
		if( wParam == MK_LBUTTON )
		{
			//set starting point
			if( click_selection == false )
			{
				click_selection = true;
				box_x1 = LOWORD(lParam);
				box_y1 = HIWORD(lParam);
			}

			//set ending point
			box_x2 = LOWORD(lParam);
			box_y2 = HIWORD(lParam);

			InvalidateRect( hwnd, NULL, FALSE );

			printf("mousedown\r\n");
		}
		break;

	case WM_HOTKEY:
		switch( wParam )
		{
		case 0:
			printf("WM_HOTKEY\r\n");

			
			ShowWindow( hwnd, SW_SHOW );

			if( desktop_capture_bitmap )
			{
				DeleteObject( desktop_capture_bitmap );
				desktop_capture_bitmap = NULL;
			}
			if( desktop_capture_dc )
			{
				DeleteDC( desktop_capture_dc );
				desktop_capture_dc = NULL;
			}

			CaptureScreen( &desktop_capture_dc, &desktop_capture_bitmap );

			Whiten( desktop_capture_dc, desktop_capture_bitmap, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top, &overlay_bitmap );

			overlay_dc = CreateCompatibleDC( desktop_capture_dc );
			if( !overlay_dc )
			{
				logger.printf( _T("WM_HOTKEY::CreateCompatibleDC(desktop_capture_dc); FATAL ERROR\r\n"));
				Sleep(INFINITE);
			}
			HGDIOBJ hgdiobj_return = SelectObject( overlay_dc, overlay_bitmap );
			if( hgdiobj_return == NULL || hgdiobj_return == HGDI_ERROR )
			{
				logger.printf( _T("SelectObject( backbuffer_dc, backbuffer_bitmap ); FATAL ERROR\r\n"));
				Sleep(INFINITE);
			}

			InvalidateRect( hwnd, NULL, FALSE );

			//Need to make CaptureDCRegion return a pointer
			//wic.CaptureDCRegion( desktop_capture_dc, desktop_capture_bitmap, 30, 50, 1200, 800 );

			

			break;
		}
		break;
	case WM_PAINT:
		printf("wm_paint\r\n");

		//Draw background
		result = BitBlt( backbuffer_dc, 0, 0, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top, overlay_dc, 0, 0, SRCCOPY );
		if( !result )
		{
			logger.printf( _T("WM_PAINT::BitBlt(overlay_dc->backbuffer_dc); FATAL ERROR: %x\r\n"), GetLastError() );
			return false;
		}

		//Draw overlay
		result = BitBlt( backbuffer_dc,
			box_x1 < box_x2 ? box_x1 : box_x2, //dest x-coordinates
			box_y1 < box_y2 ? box_y1 : box_y2, //dest y-coordinates
			box_x1 > box_x2 ? box_x1 - box_x2 : box_x2 - box_x1, //width
			box_y1 > box_y2 ? box_y1 - box_y2 : box_y2 - box_y1, //height
			desktop_capture_dc,
			box_x1 < box_x2 ? box_x1 : box_x2, //source x-coordinates
			box_y1 < box_y2 ? box_y1 : box_y2, //source y-coordinates
			SRCCOPY );
		if( !result )
		{
			logger.printf( _T("WM_PAINT::BitBlt(desktop_capture_dc->backbuffer_dc); FATAL ERROR: %x\r\n"), GetLastError() );
			return false;
		}

		result = BitBlt( GetDC(hwnd), 0, 0, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top, backbuffer_dc, 0, 0, SRCCOPY );
		if( !result )
		{
			logger.printf( _T("WM_PAINT::BitBlt(backbuffer_dc->GetDC(hwnd)); FATAL ERROR: %x\r\n"), GetLastError() );
			return false;
		}
		break;
	case WM_CLOSE:
		if( DestroyWindow(hwnd) == false )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("MainWindowProc()::DestroyWindow() FATAL ERROR: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return false;
		}
		break;

	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &main_nid);
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

void main()
{
	HRESULT result;
	MSG wnd_msg;

	//Logger ALWAYS STARTS FIRST
	logger.Initialize(); //logger declared as extern global variable in Error.h/Error.cpp




	GetWindowRect( GetDesktopWindow(), &desktop_rect );
	
	//Load Icons
	result = LoadIconMetric( GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SYSTRAY_NORMAL), LIM_SMALL, &systray_icon );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("LoadIconMetric() FATAL ERROR: %d\r\n"), result );
		Sleep( INFINITE );
	}
	result = LoadIconMetric( GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_GENERIC_PICTURE), LIM_LARGE, &imgtype_icon );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("LoadIconMetric() FATAL ERROR: %d\r\n"), result );
		Sleep( INFINITE );
	}

	//Initialize Main Window
	WNDCLASSEX screeny_wnd_class = {0};
	RegisterWindowClass( &screeny_wnd_class, MainWindowProc );
	main_window.Initialize( screeny_wnd_class, desktop_rect.left, desktop_rect.top, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top );

	//Create a Backbuffer
	backbuffer_dc = CreateCompatibleDC( GetDC(main_window.window) );
	if( !backbuffer_dc )
	{
		logger.printf( _T("CreateCompatibleDC(); FATAL ERROR\r\n"));
		Sleep(INFINITE);
	}
	backbuffer_bitmap = CreateCompatibleBitmap( GetDC(main_window.window), desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top );
	if( !backbuffer_bitmap )
	{
		logger.printf( _T("CreateCompatibleBitmap(); FATAL ERROR\r\n"));
		Sleep(INFINITE);
	}
	HGDIOBJ hgdiobj_return = SelectObject( backbuffer_dc, backbuffer_bitmap );
	if( hgdiobj_return == NULL || hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("SelectObject( backbuffer_dc, backbuffer_bitmap ); FATAL ERROR\r\n"));
		Sleep(INFINITE);
	}

	//Register hotkey
	if( RegisterHotKey( main_window.window, 0, MOD_CONTROL, VK_F12 ) == NULL )
	{
		logger.printf( _T("RegisterHotKey() error: %x\r\n"), GetLastError() );
		Sleep( INFINITE );
	}

	//Register Tray Icon
	RegisterTrayIcon( main_window.window, systray_icon, _T("Screeny"), &main_nid );
	//ShowBalloon( main_nid, imgtype_icon, _T("title is awesome"), _T("the awesome message is awesome and yes shit") );

	//Initialize COM
	result = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("CoInitializeEx(); FATAL ERROR: %x\r\n"), result );
		Sleep( INFINITE );
	}

	//Initialize Windows Imaging Component (COM)
	wic.Initialize();


	//Message loop
	while( GetMessage( &wnd_msg, NULL, 0, 0 ) )
	{
		//IsDialogMessage()


		TranslateMessage(&wnd_msg);
		DispatchMessage(&wnd_msg);
	}


	//CLEANUP
	if( backbuffer_dc )
		DeleteDC( backbuffer_dc );
	if( backbuffer_bitmap )
		DeleteObject( backbuffer_bitmap );
	if( desktop_capture_bitmap )
		DeleteObject( desktop_capture_bitmap );
	if( desktop_capture_dc )
		DeleteDC( desktop_capture_dc );
	if( wic.pFactory )
		wic.pFactory->Release();
	CoUninitialize();
	UnregisterClass( screeny_wnd_class.lpszClassName, GetModuleHandle(NULL) );
	if( imgtype_icon )
		DestroyIcon( imgtype_icon );
	if( systray_icon )
		DestroyIcon( systray_icon );

	//Sleep( INFINITE );
}