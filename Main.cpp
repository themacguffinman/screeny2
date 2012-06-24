#include "Main.h"
#include "Imaging.h"
#include "Errors.h"


void main()
{
	HRESULT result;

	//Logger ALWAYS STARTS FIRST
	logger.Initialize(); //logger declared as extern global variable in Error.h/Error.cpp






	result = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("CoInitializeEx(); FATAL ERROR: %x\r\n"), result );
	}

	WindowsImagingComponent wic;
	wic.Initialize();




	DWORD time = GetTickCount();

	HDC desktop_capture_dc;
	HBITMAP desktop_capture_bitmap;

	CaptureScreen( &desktop_capture_dc, &desktop_capture_bitmap );
	wic.CaptureDCRegion( desktop_capture_dc, 30, 50, 1200, 800 );

	DeleteObject( desktop_capture_bitmap );
	DeleteDC( desktop_capture_dc );

	time = GetTickCount() - time;

	logger.printf( _T("Capture Time: %d\r\n"), time );

	

	Sleep( INFINITE );
}