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

bool WindowManager::Initialize( WNDCLASSEX wndclass_in )
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
		WS_EX_TOPMOST, //extended styles
		this->wndclass.lpszClassName, //class name
		_T("MainWindow"), //window name
		WS_OVERLAPPED | WS_VISIBLE, //style tags
		100, //horizontal position
		100, //vertical position
		900, //width
		600, //height
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

	//SendMessage( this->main_window, WM_SETFONT, (WPARAM)this->primary_font, (LPARAM)TRUE );

	return true;
}