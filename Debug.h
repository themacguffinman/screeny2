//------------------------------------------------------------------------------------------------
//
// Written by Lucky K.
//
//------------------------------------------------------------------------------------------------


#include <tchar.h>
#include <stdio.h>
#include <Windows.h>

#ifdef _DEBUG

#define DMESG( msg, ... ) Debug::Error( _T(msg), ##__VA_ARGS__ )
#define FNT( functionName ) Debug::FT asdfasdhyg8665gkhg8gh8gf( _T(functionName) )

#else

#define DMESG( msg, ... )
#define FNT( functionName )

#endif


namespace Debug
{
	//------------------------------------------------------------------------------------------------
	//
	// Exposed Methods
	//
	//------------------------------------------------------------------------------------------------
	bool Initialize();
	void _cdecl Error( const TCHAR *str, ... );
	void _cdecl TMSG( const TCHAR *str, ... );

	void StartTime();
	void StopTime();

	//------------------------------------------------------------------------------------------------
	//
	// Exposed Types
	//
	//------------------------------------------------------------------------------------------------
	static const int depthLimit = 5;
	static int t = 0;
	static const TCHAR tabs[] = _T("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
	
	struct FT
	{
		LARGE_INTEGER timeStart;
		LARGE_INTEGER timeStop;
		LARGE_INTEGER timeFreq;
		TCHAR strName[32];

		FT( TCHAR *functionName )
		{
			_tcscpy( strName, functionName );
			
			QueryPerformanceFrequency( &timeFreq );
			QueryPerformanceCounter( &timeStart );
			if ( t < depthLimit )
				TMSG( _T("%.*s<%s t=\"%f\">\r\n"), t, tabs, strName, (float)timeStart.QuadPart / (float)timeFreq.QuadPart );
			t++;
		}

		~FT()
		{
			QueryPerformanceCounter( &timeStop );
			t--;
			if ( t < depthLimit )
				TMSG( _T("%.*s</%s>\r\n%.*s<%f/>\r\n"), t, tabs, strName, t, tabs, (float)(timeStop.QuadPart - timeStart.QuadPart) / (float)timeFreq.QuadPart );
		}
	};
}