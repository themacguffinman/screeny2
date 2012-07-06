#include "Main.h"

bool RegisterWindowClass( WNDCLASSEX *wndclass, WNDPROC window_procedure );
bool RegisterTrayIcon( HWND hwnd, HICON tray_icon, TCHAR *tooltip_msg, NOTIFYICONDATA *nid );

struct WindowManager
{
	WNDCLASSEX wndclass;
	HWND window;
	HFONT primary_font;

	WindowManager()
	{
		this->window = NULL;
		this->primary_font = NULL;
	}
	~WindowManager()
	{
		//UnregisterClass should be called in the main method

		//DestroyWindow should be called in WM_CLOSE in the windows procedure

		if( this->primary_font )
			DeleteObject( this->primary_font );
	}

	bool Initialize( WNDCLASSEX wndclass_in, unsigned int x, unsigned int y, unsigned int width, unsigned int height );
};

extern WindowManager main_window;