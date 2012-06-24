#include "Main.h"
#include "Imaging.h"



void main()
{
	HRESULT result;
	result = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY );
	if( !SUCCEEDED(result) )
	{
		printf("CoInitializeEx(); FATAL ERROR: %x\r\n", result );
	}

	WindowsImagingComponent wic;
	wic.Initialize();
	wic.CaptureScreenRegion( 0, 0, 500, 500 );

	printf("asdgagraewrg\r\n");
	Sleep( INFINITE );
}