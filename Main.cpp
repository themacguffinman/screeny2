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

RECT desktop_rect = {0};

NOTIFYICONDATA main_nid = {0};

HICON systray_icon = NULL;
HICON imgtype_icon = NULL;

HDC desktop_capture_dc = NULL;
HBITMAP desktop_capture_bitmap = NULL;
HBRUSH main_window_brush = NULL;

LRESULT CALLBACK MainWindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_HOTKEY:
		switch( wParam )
		{
		case 0:
			printf("WM_HOTKEY\r\n");

			if( main_window_brush )
			{
				DeleteObject( main_window_brush );
				main_window_brush = NULL;
			}
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

			HBITMAP overlay_bitmap;

			CaptureScreen( &desktop_capture_dc, &desktop_capture_bitmap );

			Whiten( desktop_capture_dc, desktop_capture_bitmap, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top, &overlay_bitmap );
			
			main_window_brush = CreatePatternBrush( desktop_capture_bitmap );
			if( main_window_brush == NULL )
			{
				logger.printf( _T("CreatePatternBrush() error: %x\r\n"), GetLastError() );
				Sleep( INFINITE );
			}
			InvalidateRect( hwnd, NULL, FALSE );
			UpdateWindow(hwnd);

			//Need to make CaptureDCRegion return a pointer
			wic.CaptureDCRegion( desktop_capture_dc, desktop_capture_bitmap, 30, 50, 1200, 800 );

			

			break;
		}
		break;
	case WM_PAINT:
		printf("wm_paint\r\n");

		if( FillRect( GetDC(hwnd), &desktop_rect, main_window_brush ) == false )
		{
			logger.printf( _T("MainWindowProc()::FillRect(); failed\r\n") );
			break;
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

	main_window_brush = GetSysColorBrush( COLOR_GRAYTEXT );
	if( main_window_brush == NULL )
	{
		logger.printf( _T("GetSysColorBrush() FATAL ERROR\r\n") );
		Sleep( INFINITE );
	}
	
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

	if( RegisterHotKey( main_window.window, 0, MOD_CONTROL, VK_F12 ) == NULL )
	{
		logger.printf( _T("RegisterHotKey() error: %x\r\n"), GetLastError() );
		Sleep( INFINITE );
	}

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
	if( main_window_brush )
		DeleteObject( main_window_brush );
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