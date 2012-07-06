#include "WindowManager.h"
#include "Errors.h"

WindowManager main_window;

bool RegisterWindowClass( WNDCLASSEX *wndclass, WNDPROC window_procedure )
{
	memset( wndclass, NULL, sizeof( WNDCLASSEX ) );

	wndclass->cbSize = sizeof( WNDCLASSEX );
	wndclass->lpszClassName = _T("ScreenyWindowClass");
	wndclass->lpfnWndProc = window_procedure;
	wndclass->hInstance = GetModuleHandle( NULL );
	//wndclass->style = CS_DROPSHADOW;
	//LoadIconMetric( GetModuleHandle(NULL), MAKEINTRESOURCEW( IDI_ICON1 ), LIM_LARGE, &(wnd->hIcon) );

	if( RegisterClassEx( wndclass ) == NULL )
	{
		logger.printf( _T("RegisterWindowClass()::RegisterClassEx() error: %d\r\n"), GetLastError() );
		return false;
	}

	return true;
}

bool RegisterTrayIcon( HWND hwnd, HICON tray_icon, TCHAR *tooltip_msg, NOTIFYICONDATA *nid )
{
	nid->cbSize = sizeof(NOTIFYICONDATA);
	nid->hWnd = hwnd;
	nid->uID = 0;
	nid->uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP|NIF_SHOWTIP;
	nid->uCallbackMessage = 0;
	nid->hIcon = tray_icon;
	_tcscpy( nid->szTip, tooltip_msg );
	nid->uVersion = NOTIFYICON_VERSION_4;

	if( Shell_NotifyIcon(NIM_ADD, nid) == false )
	{
		logger.printf( _T("RegisterTrayIcon()::Shell_NotifyIcon(NIM_ADD) error\r\n") );
		return false;
	}

	if( Shell_NotifyIcon(NIM_SETVERSION, nid) == false )
	{
		logger.printf( _T("RegisterTrayIcon()::Shell_NotifyIcon(NIM_SETVERSION) error\r\n") );
		return false;
	}

	return true;
}

bool ShowBalloon( NOTIFYICONDATA nid, HICON balloon_icon, TCHAR *title, TCHAR *msg )
{
	nid.uFlags = NIF_INFO;
	nid.dwInfoFlags = NIIF_USER|NIIF_LARGE_ICON;
	_tcscpy( nid.szInfoTitle, title );
	_tcscpy( nid.szInfo, msg );
	nid.hBalloonIcon = balloon_icon;

	if( Shell_NotifyIcon(NIM_SETVERSION, &nid) == false )
	{
		logger.printf( _T("ShowBalloon()::Shell_NotifyIcon(NIM_SETVERSION) error\r\n") );
		return false;
	}

	if( Shell_NotifyIcon(NIM_MODIFY, &nid) == false )
	{
		logger.printf( _T("ShowBalloon()::Shell_NotifyIcon(NIM_MODIFY) error\r\n") );
		return false;
	}
}

bool WindowManager::Initialize( WNDCLASSEX wndclass_in, unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
	WNDCLASSEX temp;
	if( GetClassInfoEx( GetModuleHandle(NULL), wndclass_in.lpszClassName, &temp ) == false )
	{
		logger.printf( _T("WindowManager::Initialize() error: Window Class is not registered!\r\n") );
		return false;
	}
	this->wndclass = wndclass_in;

	this->window = CreateWindowEx
		(
		/*WS_EX_TOPMOST*/NULL, //extended styles
		this->wndclass.lpszClassName, //class name
		_T("MainWindow"), //window name
		WS_POPUP, //style tags
		x, //horizontal position
		y, //vertical position
		width, //width
		height, //height
		NULL, //parent window
		NULL, //class menu
		GetModuleHandle(NULL), //some HINSTANCE pointer
		NULL //Create Window Data?
		);
	if( this->window == NULL )
	{
		logger.printf( _T("WindowManager::Initialize()::CreateMainWindow()::CreateWindowEx() error: %d\r\n"), GetLastError() );
		return false;
	}

	//ShowWindow( this->window, SW_SHOW );

	//SendMessage( this->main_window, WM_SETFONT, (WPARAM)this->primary_font, (LPARAM)TRUE );

	return true;
}