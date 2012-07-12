#include "Main.h"
#include "Imaging.h"
#include "Errors.h"
#include "WindowManager.h"
#include "resource.h"
#include "Network.h"
#include <curl\curl.h>

int result;
RECT desktop_rect = {0};

NOTIFYICONDATA main_nid = {0};

HICON systray_icon = NULL;
HICON imgtype_icon = NULL;
HICON error_icon = NULL;

HCURSOR crosshair_cursor = NULL;

BYTE *pimage_buffer = NULL;
ULARGE_INTEGER image_size = {0};

CURL *pcurl_handle;

HDC backbuffer_dc = NULL;
HGDIOBJ backbuffer_dc_deselectobj = NULL;
HBITMAP backbuffer_bitmap = NULL;
HDC desktop_capture_dc = NULL;
HGDIOBJ desktop_capture_dc_deselectobj = NULL;
HBITMAP desktop_capture_bitmap = NULL;
HDC overlay_dc = NULL;
HGDIOBJ overlay_dc_deselectobj = NULL;
HBITMAP overlay_bitmap = NULL;
BYTE *overlay_bitmap_data = NULL;
bool pressed_hotkey = false;
bool click_selection = false;
unsigned int box_x1 = 0;
unsigned int box_y1 = 0;
unsigned int box_x2 = 0;
unsigned int box_y2 = 0;

bool SetClipboardText( char *str )
{
	HGLOBAL globalmemory;
	void *cliptext = 0;

	if( !OpenClipboard( main_window.window ) )
	{
		logger.printf( _T("Open Clipboard failed") );
		return false;
	}

	if( !EmptyClipboard() )
	{
		logger.printf( _T("Empty Clipboard failed") );
		return false;
	}

	globalmemory = GlobalAlloc( GMEM_MOVEABLE, strlen(str) + 1 );

	cliptext = GlobalLock( globalmemory );

	if ( 0 == cliptext )
	{
		GlobalFree( globalmemory );
		CloseClipboard();
		logger.printf( _T("failed to GlobalLock") );
		return false;
	}

	memcpy( cliptext, str, strlen(str) + 1 );

	HANDLE clipdata = SetClipboardData( CF_TEXT, cliptext );

	GlobalUnlock( cliptext );

	if( !CloseClipboard() )
	{
		logger.printf( _T("Close Clipboard failed") );
		return false;
	}

	return true;
}

CaptureScreenThread_in *CreateCaptureScreenThread_in( CURL *pcurl_handle, BYTE *pimage_buffer, ULONGLONG image_size )
{
	CaptureScreenThread_in *cst_in = new CaptureScreenThread_in();
	cst_in->pcurl_handle = pcurl_handle;
	cst_in->pimage_buffer = pimage_buffer;
	cst_in->image_buffer_len = image_size;
	
	return cst_in;
}

DWORD WINAPI CaptureScreenThreadProc( LPVOID lpParam )
{
	CaptureScreenThread_in *input = (CaptureScreenThread_in *) lpParam;

	if( false == ImgurUpload( input->pcurl_handle, input->pimage_buffer, input->image_buffer_len ) )
	{
		logger.printf( _T("CaptureScreenThreadProc(): Screenshot upload failure\r\n") );
		ShowBalloon( main_nid, error_icon, _T("Unexpected error in uploading image to Imgur"), _T("Please consult the log for more details") );
	} else {
		ShowBalloon( main_nid, imgtype_icon, _T("Image successfully uploaded to Imgur"), _T("A hyperlink to the image has been copied to your clipboard") );
	}

	VirtualFree( input->pimage_buffer, 0, MEM_RELEASE );

	delete lpParam;

	SetClipboardText( imgur_uploads.back().upload.links.original );
	//printf("%s\r\n", imgur_uploads[0].upload.links.original );

	return 0;
}

LRESULT CALLBACK MainWindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_KEYDOWN:
		switch( wParam )
		{
		case VK_ESCAPE:
			ShowWindow( hwnd, SW_HIDE );
			break;
		}
		break;

	case WM_ACTIVATE:
		switch( wParam )
		{
		case WA_CLICKACTIVE:
		case WA_ACTIVE:
			SetCursor( crosshair_cursor );
			ShowWindow( hwnd, SW_SHOW );
			BringWindowToTop( hwnd );
			break;
		case WA_INACTIVE:
			printf("inactive\r\n");
			//Reset overlay and desktop_capture
			if( overlay_dc )
			{
				SelectObject( overlay_dc, overlay_dc_deselectobj );
				DeleteDC( overlay_dc );
				overlay_dc = NULL;
			}
			if( overlay_bitmap )
			{
				VirtualFree( overlay_bitmap_data, 0, MEM_RELEASE );
				DeleteObject( overlay_bitmap );
				overlay_bitmap = NULL;
			}
			if( desktop_capture_dc )
			{
				SelectObject( desktop_capture_dc, desktop_capture_dc_deselectobj );
				DeleteDC( desktop_capture_dc );
				desktop_capture_dc = NULL;
			}
			if( desktop_capture_bitmap )
			{
				DeleteObject( desktop_capture_bitmap );
				desktop_capture_bitmap = NULL;
			}

			box_x1 = 0;
			box_y1 = 0;
			box_x2 = 0;
			box_y2 = 0;
			click_selection = false;
			pressed_hotkey = false;
			SetCursor( NULL );
			break;
		}
		break;

	case WM_LBUTTONUP:
		if( !(pressed_hotkey&&click_selection) )
			break;

		//Need to make CaptureDCRegion return a pointer
		wic.CaptureDCRegion( desktop_capture_dc, desktop_capture_bitmap,
			box_x1 < box_x2 ? box_x1 : box_x2, //x
			box_y1 < box_y2 ? box_y1 : box_y2, //y
			box_x1 > box_x2 ? box_x1 - box_x2 : box_x2 - box_x1, //width
			box_y1 > box_y2 ? box_y1 - box_y2 : box_y2 - box_y1, //height
			&pimage_buffer, &image_size );

		CaptureScreenThread_in *cst_in;
		cst_in = CreateCaptureScreenThread_in( pcurl_handle, pimage_buffer, image_size.QuadPart );

		if( NULL == CreateThread( NULL, NULL, CaptureScreenThreadProc, cst_in, 0, NULL ) )
			logger.printf( _T("WM_LBUTTONUP::CreateThread() FATAL ERROR: %d\r\n"), GetLastError() );

		ShowWindow( hwnd, SW_HIDE );
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
		}
		break;

	case WM_HOTKEY:
		switch( wParam )
		{
		case 0:
			printf("WM_HOTKEY\r\n");

			if( pressed_hotkey )
				break;
			
			pressed_hotkey = true;

			//Reset overlay and desktop_capture
			if( overlay_dc )
			{
				SelectObject( overlay_dc, overlay_dc_deselectobj );
				DeleteDC( overlay_dc );
				overlay_dc = NULL;
			}
			if( overlay_bitmap )
			{
				VirtualFree( overlay_bitmap_data, 0, MEM_RELEASE );
				DeleteObject( overlay_bitmap );
				overlay_bitmap = NULL;
			}
			if( desktop_capture_dc )
			{
				SelectObject( desktop_capture_dc, desktop_capture_dc_deselectobj );
				DeleteDC( desktop_capture_dc );
				desktop_capture_dc = NULL;
			}
			if( desktop_capture_bitmap )
			{
				DeleteObject( desktop_capture_bitmap );
				desktop_capture_bitmap = NULL;
			}

			CaptureScreen( &desktop_capture_dc, &desktop_capture_bitmap, &desktop_capture_dc_deselectobj );

			Whiten( desktop_capture_dc, desktop_capture_bitmap, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top, &overlay_dc, &overlay_bitmap, &overlay_dc_deselectobj, &overlay_bitmap_data );

			InvalidateRect( hwnd, NULL, FALSE );
			ShowWindow( hwnd, SW_SHOW );
			SetCursor( crosshair_cursor );

			break;
		}
		break;
	case WM_PAINT:
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

		//Flip backbuffer
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


	//FreeConsole();

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
	
	SHSTOCKICONINFO shsii;
	shsii.cbSize = sizeof(SHSTOCKICONINFO);
	result = SHGetStockIconInfo( SIID_ERROR, SHGSI_ICON|SHGSI_LARGEICON, &shsii );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("SHGetStockIconInfo(SIID_ERROR) FATAL ERROR: %d\r\n"), result );
		Sleep( INFINITE );
	}
	error_icon = shsii.hIcon;

	//Load Cursors
	crosshair_cursor = LoadCursor( NULL, MAKEINTRESOURCE(IDC_CROSS) );
	if( crosshair_cursor == NULL )
	{
		logger.printf( _T("LoadCursor(IDC_CROSS) FATAL ERROR: %d\r\n"), GetLastError() );
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
	backbuffer_dc_deselectobj = SelectObject( backbuffer_dc, backbuffer_bitmap );
	if( backbuffer_dc_deselectobj == NULL || backbuffer_dc_deselectobj == HGDI_ERROR )
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

	//Initialize libcurl
	CURLcode libcurl_result = curl_global_init( CURL_GLOBAL_WIN32 );
	if( libcurl_result )
	{
		logger.printf( _T("curl_global_init(CURL_GLOBAL_WIN32); FATAL ERROR: %d\r\n"), libcurl_result );
		Sleep( INFINITE );
	}
	pcurl_handle = curl_easy_init();
	if( pcurl_handle == NULL )
	{
		logger.printf( _T("curl_easy_init(); FATAL ERROR\r\n") );
		Sleep( INFINITE );
	}


	//Message loop
	while( GetMessage( &wnd_msg, NULL, 0, 0 ) )
	{
		//IsDialogMessage()


		TranslateMessage(&wnd_msg);
		DispatchMessage(&wnd_msg);
	}


	//CLEANUP
	curl_global_cleanup();

	if( overlay_dc )
	{
		SelectObject( overlay_dc, overlay_dc_deselectobj );
		DeleteDC( overlay_dc );
		overlay_dc = NULL;
	}
	if( overlay_bitmap )
	{
		VirtualFree( overlay_bitmap_data, 0, MEM_RELEASE );
		DeleteObject( overlay_bitmap );
	}
	if( desktop_capture_dc )
	{
		SelectObject( desktop_capture_dc, desktop_capture_dc_deselectobj );
		DeleteDC( desktop_capture_dc );
	}
	if( desktop_capture_bitmap )
		DeleteObject( desktop_capture_bitmap );
	if( backbuffer_dc )
	{
		SelectObject( backbuffer_dc, backbuffer_dc_deselectobj );
		DeleteDC( backbuffer_dc );
	}
	if( backbuffer_bitmap )
		DeleteObject( backbuffer_bitmap );

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