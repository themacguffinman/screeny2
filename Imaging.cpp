#include "Imaging.h"
#include "Errors.h"

bool WindowsImagingComponent::Initialize()
{
	HRESULT result;

	result = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*) &this->pFactory);
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::Initialize()::CoCreateInstance(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	return true;
}

bool WindowsImagingComponent::ConvertBitmapToPng( HBITMAP hbmp, unsigned int width, unsigned int height )
{
	HRESULT result;

	if( !this->pFactory )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng() error: IWICImagingFactory is missing\r\n") );
		return false;
	}

	result = this->pFactory->CreateStream( &this->pStream );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::CreateStream(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pStream->InitializeFromFilename( TEXT("captureoutput.png"), GENERIC_WRITE );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::InitializeFromFilename(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pFactory->CreateEncoder( GUID_ContainerFormatPng, NULL, &this->pBitmapEncoder );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::CreateEncoder(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pBitmapEncoder->Initialize( this->pStream, WICBitmapEncoderNoCache );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::Initialize(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pBitmapEncoder->CreateNewFrame( &this->pBitmapFrame, NULL ); //propertybag is NULL because it's complex
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::CreateNewFrame(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pBitmapFrame->Initialize( NULL ); //initializing with no propertybag
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::Initialize( NULL ); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pFactory->CreateBitmapFromHBITMAP( hbmp, NULL, WICBitmapIgnoreAlpha, &this->pWicBitmap );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::CreateBitmapFromHBITMAP(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	WICRect wicrect;
	wicrect.X = 0;
	wicrect.Y = 0;
	wicrect.Width = width;
	wicrect.Height = height;
	result = this->pBitmapFrame->WriteSource( this->pWicBitmap, &wicrect );
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::WriteSource(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pBitmapFrame->Commit();
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::pBItmapFrame->Commit(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	result = this->pBitmapEncoder->Commit();
	if( !SUCCEEDED(result) )
	{
		logger.printf( _T("WindowsImagingComponent::ConvertBitmapToPng()::pBitmapEncoder->Commit(); FATAL ERROR: %x\r\n"), result );
		return false;
	}

	if( this->pWicBitmap )
		this->pWicBitmap->Release();
	this->pWicBitmap = NULL;

	if ( this->pBitmapFrame )
		this->pBitmapFrame->Release();
	this->pBitmapFrame = NULL;

	if ( this->pBitmapEncoder )
		this->pBitmapEncoder->Release();
	this->pBitmapEncoder = NULL;

	if ( this->pStream )
		this->pStream->Release();
	this->pStream = NULL;

	return true;
}

bool WindowsImagingComponent::CaptureScreenRegion( unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
	int errmsg;

	HWND desktop_window;
	HDC desktop_dc;
	HDC capture_dc;
	HBITMAP hbmp;

	desktop_window = GetDesktopWindow();

	desktop_dc = GetDC( desktop_window );
	if( !desktop_dc )
	{
		logger.printf( _T("Capture()::GetDC( desktop_window ); FATAL ERROR\r\n"));
		return false;
	}

	capture_dc = CreateCompatibleDC( desktop_dc );
	if( !capture_dc )
	{
		logger.printf( _T("Capture()::CreateCompatibleDC( desktop_dc ); FATAL ERROR\r\n"));
		return false;
	}

	hbmp = CreateCompatibleBitmap( desktop_dc, width, height );
	if( !hbmp )
	{
		logger.printf( _T("Capture()::CreateCompatibleBitmap( desktop_dc, width, height ); FATAL ERROR\r\n"));
		return false;
	}

	HGDIOBJ hgdiobj_return = SelectObject( capture_dc, hbmp );
	if( hgdiobj_return == NULL || hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("Capture()::SelectObject( capture_dc, hbmp ); FATAL ERROR\r\n"));
		return false;
	}

	errmsg = BitBlt( capture_dc, 0, 0, width, height, desktop_dc, x, y, SRCCOPY|CAPTUREBLT );
	if( !errmsg )
	{
		logger.printf( _T("Capture()::BitBlt( capture_dc, 0, 0, width, height, desktop_dc, x, y, SRCCOPY|CAPTUREBLT ); FATAL ERROR: %x\r\n"), GetLastError() );
		return false;
	}

	this->ConvertBitmapToPng( hbmp, width, height );

	//RELEASE THINGS
	DeleteObject( hbmp );
	DeleteDC( capture_dc );
	ReleaseDC( desktop_window, desktop_dc );

	return true;
}