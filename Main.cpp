#include "Main.h"
#include "Imaging.h"
#include "Errors.h"


void main()
{
	HRESULT result;

	logger.Initialize(); //logger declared as extern global variable in Error.h/Error.cpp

	result = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("CoInitializeEx(); FATAL ERROR: %x\r\n"), result );
	}

	WindowsImagingComponent wic;
	wic.Initialize();

	DWORD time = GetTickCount();
	wic.CaptureScreenRegion( 0, 0, 500, 500 );
	time = GetTickCount() - time;

	logger.printf( _T("Capture Time: %d\r\n"), time );

	

	Sleep( INFINITE );
}