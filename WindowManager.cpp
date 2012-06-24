#include "WindowManager.h"
#include "Errors.h"

bool WindowManager::Initialize( WNDPROC windowproc )
{
	this->wndclass.cbSize = sizeof( WNDCLASSEX );
	this->wndclass.lpszClassName = _T("MainWClass");
	this->wndclass.lpfnWndProc = windowproc;
	this->wndclass.hInstance = GetModuleHandle( NULL );
	//this->wndclass.style = CS_DROPSHADOW;
	//LoadIconMetric( GetModuleHandle(NULL), MAKEINTRESOURCEW( IDI_ICON1 ), LIM_LARGE, &(wnd->hIcon) );

	if( NULL == RegisterClassEx(&this->wndclass) )
	{
		logger.printf( _T("WindowManager::Initialize()::CreateMainWindow()::RegisterClassex() error: %d\r\n"), GetLastError() );
		return false;
	}

	this->main_window = CreateWindowEx
		(
		WS_EX_TOPMOST, //extended styles
		this->wndclass.lpszClassName, //class name
		_T("MainWindow"), //window name
		WS_POPUP, //style tags
		100, //horizontal position
		100, //vertical position
		900, //width
		600, //height
		NULL, //parent window
		NULL, //class menu
		GetModuleHandle(NULL), //some HINSTANCE pointer
		NULL //Create Window Data?
		);

	if( this->main_window == NULL )
	{
		logger.printf( _T("WindowManager::Initialize()::CreateMainWindow()::CreateWindowEx() error: %d\r\n"), GetLastError() );
		return false;
	}

	//SendMessage( this->main_window, WM_SETFONT, (WPARAM)this->primary_font, (LPARAM)TRUE );

	return true;
}