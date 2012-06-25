#include "Main.h"
#include "Imaging.h"
#include "Errors.h"
#include "WindowManager.h"

LRESULT CALLBACK MainWindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
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





	//Initialize Main Window
	WNDCLASSEX screeny_wnd_class = {0};
	RegisterWindowClass( &screeny_wnd_class, MainWindowProc );
	main_window.Initialize( screeny_wnd_class );

	//Initialize COM
	result = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("CoInitializeEx(); FATAL ERROR: %x\r\n"), result );
	}

	//Initialize Windows Imaging Component (COM)
	wic.Initialize();



	//Capture screen
	DWORD time = GetTickCount();

	HDC desktop_capture_dc;
	HBITMAP desktop_capture_bitmap;

	CaptureScreen( &desktop_capture_dc, &desktop_capture_bitmap );
	wic.CaptureDCRegion( desktop_capture_dc, 30, 50, 1200, 800 );

	DeleteObject( desktop_capture_bitmap );
	DeleteDC( desktop_capture_dc );

	time = GetTickCount() - time;

	logger.printf( _T("Capture Time: %d\r\n"), time );


	//Message loop
	while( GetMessage( &wnd_msg, NULL, 0, 0 ) )
	{
		//IsDialogMessage()


		TranslateMessage(&wnd_msg);
		DispatchMessage(&wnd_msg);
	}


	//CLEANUP
	if( wic.pFactory )
		wic.pFactory->Release();
	CoUninitialize();
	UnregisterClass( screeny_wnd_class.lpszClassName, GetModuleHandle(NULL) );

	//Sleep( INFINITE );
}