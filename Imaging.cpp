#include "Imaging.h"
#include "Errors.h"

WindowsImagingComponent wic;

bool Whiten( HDC source_dc, HBITMAP source_bitmap, unsigned int width, unsigned int height, HBITMAP *pwhitened_bitmap )
{
	int result;
	BYTE *pwhitened_data;
	BITMAPINFO whitened_bitmapinfo;

	if( pwhitened_bitmap == NULL )
	{
		logger.printf( _T("Whiten(); error: pwhitened_bitmap is NULL! \r\n") );
		return false;
	}

	pwhitened_data = (BYTE *) VirtualAlloc( NULL, width * 4 * height, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE );
	if( pwhitened_data == NULL )
	{
		logger.printf( _T("Whiten()::VirtualAlloc(); FATAL ERROR: %d\r\n"), GetLastError() );
		return false;
	}

	result = GetDIBits( source_dc, source_bitmap, 0, height, pwhitened_data, &whitened_bitmapinfo, DIB_RGB_COLORS );
	if( result == NULL )
	{
		logger.printf( _T("Whiten()::GetDIBits(); FATAL ERROR\r\n") );
		return false;
	}

	for( int i = 0; i < width * 4 * height; i++ )
	{
		pwhitened_data[i] = (pwhitened_data[i] + 255) / 2;
	}

	*pwhitened_bitmap = CreateBitmap( width, height, 1, 32, pwhitened_data );
	if( *pwhitened_bitmap == NULL )
	{
		logger.printf( _T("Whiten()::CreateBitmap(); FATAL ERROR\r\n") );
		return false;
	}

	return true;
}

bool CaptureScreen( HDC *pcapture_dc, HBITMAP *phbmp )
{
	int errmsg;

	HWND desktop_window;
	RECT desktop_rect;
	HDC desktop_dc;
	HDC capture_dc;
	HBITMAP hbmp;
	HGDIOBJ hgdiobj_return;

	desktop_window = GetDesktopWindow();
	GetWindowRect( desktop_window, &desktop_rect );

	desktop_dc = GetDC( desktop_window );
	if( !desktop_dc )
	{
		logger.printf( _T("CaptureScreen()::GetDC(); FATAL ERROR\r\n"));
		return false;
	}

	capture_dc = CreateCompatibleDC( desktop_dc );
	if( !capture_dc )
	{
		logger.printf( _T("CaptureScreen()::CreateCompatibleDC(); FATAL ERROR\r\n"));
		return false;
	}

	hbmp = CreateCompatibleBitmap( desktop_dc, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top );
	if( !hbmp )
	{
		logger.printf( _T("CaptureScreen()::CreateCompatibleBitmap(); FATAL ERROR\r\n"));
		return false;
	}

	hgdiobj_return = SelectObject( capture_dc, hbmp );
	if( hgdiobj_return == NULL || hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("CaptureScreen()::SelectObject( capture_dc, hbmp ); FATAL ERROR\r\n"));
		return false;
	}

	errmsg = BitBlt( capture_dc, 0, 0, desktop_rect.right-desktop_rect.left, desktop_rect.bottom-desktop_rect.top, desktop_dc, 0, 0, SRCCOPY|CAPTUREBLT );
	if( !errmsg )
	{
		logger.printf( _T("CaptureScreen()::BitBlt(); FATAL ERROR: %x\r\n"), GetLastError() );
		return false;
	}

	hgdiobj_return = SelectObject( capture_dc, hgdiobj_return );
	if( hgdiobj_return == NULL || hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("CaptureScreen()::SelectObject( capture_dc, hgdiobj_return ); FATAL ERROR\r\n"));
		return false;
	}

	*pcapture_dc = capture_dc;
	*phbmp = hbmp;
	ReleaseDC( desktop_window, desktop_dc );

	return true;
}

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

bool WindowsImagingComponent::CaptureDCRegion( HDC source_dc, HBITMAP source_bitmap, unsigned int x, unsigned int y, unsigned int width, unsigned int height )
{
	int errmsg;

	HDC capture_dc;
	HBITMAP hbmp;
	HGDIOBJ source_hgdiobj_return;
	HGDIOBJ capture_hgdiobj_return;

	source_hgdiobj_return = SelectObject( source_dc, source_bitmap );
	if( source_hgdiobj_return == NULL || source_hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("CaptureDCRegion()::SelectObject( source_dc, source_bitmap ); FATAL ERROR\r\n"));
		return false;
	}

	capture_dc = CreateCompatibleDC( source_dc );
	if( !capture_dc )
	{
		logger.printf( _T("CaptureDCRegion()::CreateCompatibleDC( source_dc ); FATAL ERROR\r\n"));
		return false;
	}

	hbmp = CreateCompatibleBitmap( source_dc, width, height );
	if( !hbmp )
	{
		logger.printf( _T("CaptureDCRegion()::CreateCompatibleBitmap( source_dc, width, height ); FATAL ERROR\r\n"));
		return false;
	}

	capture_hgdiobj_return = SelectObject( capture_dc, hbmp );
	if( capture_hgdiobj_return == NULL || capture_hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("CaptureDCRegion()::SelectObject( capture_dc, hbmp ); FATAL ERROR\r\n"));
		return false;
	}

	errmsg = BitBlt( capture_dc, 0, 0, width, height, source_dc, x, y, SRCCOPY|CAPTUREBLT );
	if( !errmsg )
	{
		logger.printf( _T("CaptureDCRegion()::BitBlt( capture_dc, 0, 0, width, height, desktop_dc, x, y, SRCCOPY|CAPTUREBLT ); FATAL ERROR: %x\r\n"), GetLastError() );
		return false;
	}

	this->ConvertBitmapToPng( hbmp, width, height );

	//RELEASE THINGS
	capture_hgdiobj_return = SelectObject( capture_dc, capture_hgdiobj_return );
	if( capture_hgdiobj_return == NULL || capture_hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("CaptureDCRegion()::SelectObject( capture_dc ) [deselecting object]; FATAL ERROR\r\n"));
		return false;
	}
	source_hgdiobj_return = SelectObject( source_dc, source_hgdiobj_return );
	if( source_hgdiobj_return == NULL || source_hgdiobj_return == HGDI_ERROR )
	{
		logger.printf( _T("CaptureDCRegion()::SelectObject( source_dc ) [deselecting object]; FATAL ERROR\r\n"));
		return false;
	}
	DeleteObject( hbmp );
	DeleteDC( capture_dc );

	return true;
}