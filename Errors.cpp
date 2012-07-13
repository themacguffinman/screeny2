#include "Errors.h"

Logger logger;

bool Logger::Initialize()
{
	TCHAR output_buffer[256];
	SYSTEMTIME datetime;
	DWORD dwRetLen;

	GetSystemTime( &datetime );

	_stprintf( output_buffer, _T("----- Screeny Log [%d:%d:%d, %d/%d/%d UTC] -----\r\n"), datetime.wHour, datetime.wMinute, datetime.wSecond, datetime.wDay, datetime.wMonth, datetime.wYear );

	if( !WriteFile( this->logfile, output_buffer, _tcslen(output_buffer) * sizeof(TCHAR), &dwRetLen, NULL ) )
	{
		TCHAR errstr[256];
		_stprintf( errstr, _T("Logger::Initialize()::WriteFile() error: %x"), GetLastError() );
		MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
		return false;
	}

	return true;
}

bool _cdecl Logger::printf( const TCHAR *msg, ... )
{
	TCHAR formatted_str[512];
	va_list arglist;
	DWORD dwRetLen;

	va_start( arglist, msg );
	_vstprintf( formatted_str, msg, arglist );

	//temporary debugging measure
	WriteConsole( GetStdHandle(STD_OUTPUT_HANDLE), formatted_str, _tcslen(formatted_str), &dwRetLen, NULL );

	if( !WriteFile( this->logfile, formatted_str, _tcslen(formatted_str) * sizeof(TCHAR), &dwRetLen, NULL ) )
	{
		TCHAR errstr[256];
		_stprintf( errstr, _T("Logger::printf()::WriteFile() error: %x"), GetLastError() );
		MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
		return false;
	}

	va_end( arglist );

	return true;
}

bool _cdecl Logger::printfA( const char *msg, ... )
{
	char formatted_str[512];
	va_list arglist;
	DWORD dwRetLen;

	va_start( arglist, msg );
	vsprintf( formatted_str, msg, arglist );

	//temporary debugging measure
	WriteConsoleA( GetStdHandle(STD_OUTPUT_HANDLE), formatted_str, strlen(formatted_str), &dwRetLen, NULL );

	if( !WriteFile( this->logfile, formatted_str, strlen(formatted_str), &dwRetLen, NULL ) )
	{
		TCHAR errstr[256];
		_stprintf( errstr, _T("Logger::printf()::WriteFile() error: %x"), GetLastError() );
		MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
		return false;
	}

	va_end( arglist );

	return true;
}