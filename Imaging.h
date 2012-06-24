#include "Main.h"
#include <wincodec.h>
#include <wincodecsdk.h>
#pragma comment(lib, "WindowsCodecs.lib")

bool CaptureScreen( HDC *pcapture_dc, HBITMAP *phbmp );

struct WindowsImagingComponent
{
	IWICImagingFactory *pFactory;
	IWICBitmapEncoder *pBitmapEncoder;
	IWICBitmapFrameEncode *pBitmapFrame;
	IPropertyBag2 *pPropertybag;
	IWICStream *pStream;
	IWICBitmap *pWicBitmap;

	WindowsImagingComponent()
	{
		this->pFactory = NULL;
		this->pBitmapEncoder = NULL;
		this->pBitmapFrame = NULL;
		this->pPropertybag = NULL;
		this->pStream = NULL;
		this->pWicBitmap;
	}
	~WindowsImagingComponent()
	{
		if( this->pWicBitmap )
			this->pWicBitmap->Release();

		if( this->pStream )
			this->pStream->Release();

		if( this->pPropertybag )
			this->pPropertybag->Release();

		if( this->pBitmapFrame )
			this->pBitmapFrame->Release();

		if( this->pBitmapEncoder )
			this->pBitmapEncoder->Release();

		if( this->pFactory )
			this->pFactory->Release();
	}

	bool Initialize();
	bool ConvertBitmapToPng( HBITMAP hbmp, unsigned int width, unsigned int height );
	bool CaptureDCRegion( HDC source_dc, unsigned int x, unsigned int y, unsigned int width, unsigned int height );
};