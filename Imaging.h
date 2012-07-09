#include "Main.h"
#include <wincodec.h>
#include <wincodecsdk.h>

#define PNG_BUFFER_SIZE 10*1024*1024

bool Whiten( HDC source_dc, HBITMAP source_bitmap, unsigned int width, unsigned int height, HDC *pwhitened_dc, HBITMAP *pwhitened_bitmap, HGDIOBJ* pwhitened_dc_deselectobj, BYTE **ppwhitened_data );
bool CaptureScreen( HDC *pcapture_dc, HBITMAP *phbmp, HGDIOBJ *pcapture_dc_deselectobj );

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

		//Unfortunately, the destructor is called after COM is uninitialized in the main method
		//IWICFactory cannot be released in the destructor
		/*if( this->pFactory )
			this->pFactory->Release();*/
	}

	bool Initialize();
	bool WindowsImagingComponent::ConvertBitmapToPng( HBITMAP hbmp, unsigned int width, unsigned int height, BYTE **ppimage_buffer, ULARGE_INTEGER *pimgbuf_real_len );
	bool WindowsImagingComponent::CaptureDCRegion( HDC source_dc, HBITMAP source_bitmap, unsigned int x, unsigned int y, unsigned int width, unsigned int height, BYTE **ppimage_buffer, ULARGE_INTEGER *pimgbuf_real_len );
};

extern WindowsImagingComponent wic;