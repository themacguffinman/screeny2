#include "Main.h"

struct Logger
{
	HANDLE logfile;
	HANDLE tempfile;
	TCHAR temp_filepath[MAX_PATH];

	Logger()
	{
		DWORD result;
		TCHAR temp_path[MAX_PATH];

		this->logfile = CreateFile( _T("screeny.log"), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( this->logfile == INVALID_HANDLE_VALUE )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("Logger::Logger()::CreateFile( screeny.log ) error: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return;
		}
		
		result = GetTempPath( MAX_PATH, temp_path );
		if( result == 0 || result > MAX_PATH )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("Logger::Logger()::GetTempPath() error: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return;
		}

		result = GetTempFileName( temp_path, _T("screeny"), 74365, this->temp_filepath );
		if( result == 0 || result == ERROR_BUFFER_OVERFLOW )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("Logger::Logger()::GetTempFileName() error: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return;
		}

		this->tempfile = CreateFile( this->temp_filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( this->tempfile == INVALID_HANDLE_VALUE )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("Logger::Logger()::CreateFile( temp_filepath ) error: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return;
		}
	}
	~Logger()
	{
		if( logfile != NULL || logfile != INVALID_HANDLE_VALUE )
		{
			if( !FlushFileBuffers( logfile ) )
			{
				TCHAR errstr[256];
				_stprintf( errstr, _T("Logger::~Logger()::FlushFileBuffers( logfile ) error: %x"), GetLastError() );
				MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
				return;
			}
		}
		if( tempfile != NULL || tempfile != INVALID_HANDLE_VALUE )
		{
			if( !FlushFileBuffers( tempfile ) )
			{
				TCHAR errstr[256];
				_stprintf( errstr, _T("Logger::~Logger()::FlushFileBuffers( tempfile ) error: %x"), GetLastError() );
				MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
				return;
			}
		}

		
		if( !CloseHandle( logfile ) )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("Logger::~Logger()::CloseHandle( logfile ) error: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return;
		}
		if( !CloseHandle( tempfile ) )
		{
			TCHAR errstr[256];
			_stprintf( errstr, _T("Logger::~Logger()::CloseHandle( tempfile ) error: %x"), GetLastError() );
			MessageBox( NULL, errstr, _T("Screeny Error"), MB_ICONERROR|MB_SETFOREGROUND );
			return;
		}
	}

	bool Initialize();
	bool printf( const TCHAR *msg, ... );
};

extern Logger logger;