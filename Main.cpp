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
		printf("CoInitializeEx(); FATAL ERROR: %x\r\n", result );
	}

	WindowsImagingComponent wic;
	wic.Initialize();

	DWORD time = GetTickCount();
	wic.CaptureScreenRegion( 0, 0, 500, 500 );
	time = GetTickCount() - time;

	printf("Capture Time: %d\r\n", time );

	

	Sleep( INFINITE );
}