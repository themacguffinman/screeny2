//------------------------------------------------------------------------------------------------
//
// Written by Lucky K.
//
//------------------------------------------------------------------------------------------------

#include "Debug.h"

namespace Debug
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Internal Globals
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hTmsgFile = INVALID_HANDLE_VALUE;
	TCHAR temp[1024];
	LARGE_INTEGER start;
	LARGE_INTEGER freq;
	LARGE_INTEGER finish;

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Methods
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Initialize()
	{
		if ( (hFile = CreateFile( _T("log6572.log"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL )) == INVALID_HANDLE_VALUE )
			goto Fail;

		if ( (hTmsgFile = CreateFile( _T("Hellfish.xml"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL )) == INVALID_HANDLE_VALUE )
			goto Fail;

		QueryPerformanceFrequency( &freq );

		freq.QuadPart /= 1000;

		return true;

Fail:
		return false;
	}

	void _cdecl Error( const TCHAR *str, ... )
	{
		DWORD dwBytesWritten = 0;

		va_list arglist;

		va_start( arglist, str );

		dwBytesWritten = _vstprintf( temp, str, arglist );

		WriteFile( hFile, temp, _tcslen(temp), &dwBytesWritten, NULL );

		//_tprintf( temp );

		//FlushFileBuffers( hFile );

		va_end( arglist );
	}

	void _cdecl TMSG( const TCHAR *str, ... )
	{
		DWORD dwBytesWritten = 0;

		va_list arglist;

		va_start( arglist, str );

		dwBytesWritten = _vstprintf( temp, str, arglist );

		WriteFile( hTmsgFile, temp, _tcslen(temp), &dwBytesWritten, NULL );

		va_end( arglist );
	}

	void StartTime()
	{
		QueryPerformanceCounter( &start );
	}

	void StopTime()
	{
		QueryPerformanceCounter( &finish );
		Error( _T("Time: %f\n"), ((float)(finish.QuadPart - start.QuadPart) / (float)freq.QuadPart)  );
	}
}