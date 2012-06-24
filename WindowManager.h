#include "Main.h"

struct WindowManager
{
	WNDCLASSEX wndclass;
	HWND main_window;
	HFONT primary_font;

	WindowManager()
	{
		memset( &this->wndclass, NULL, sizeof( WNDCLASSEX ) );
		main_window = NULL;
		primary_font = NULL;
	}

	bool Initialize( WNDPROC windowproc );
};