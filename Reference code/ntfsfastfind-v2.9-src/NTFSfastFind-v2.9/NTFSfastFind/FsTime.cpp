// ------------------------------------------------------------------------------------------------
// FileSystem Time class used to manipulate time.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------


#include "FsTime.h"

#include <iostream>
#include <iomanip>

//-----------------------------------------------------------------------------
FILETIME FsTime::UnixTimeToFileTime(time_t unixTime)
{
    // UnixTime seconds since midnight January 1, 1970 UTC 
    // FILETIME 100-nanosecond intervals since January 1, 1601 UTC 

    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(unixTime, 10000000) + 116444736000000000;
    FILETIME fileTime;
    fileTime.dwLowDateTime = (DWORD)ll;
    fileTime.dwHighDateTime = ll >> 32;
    return fileTime;
}

//-----------------------------------------------------------------------------
FILETIME FsTime::SecondsToFileTime(time_t unixTime)
{
    // UnixTime seconds since midnight January 1, 1970 UTC 
    // FILETIME 100-nanosecond intervals since January 1, 1601 UTC 

    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(unixTime, 10000000);
    FILETIME fileTime;
    fileTime.dwLowDateTime = (DWORD)ll;
    fileTime.dwHighDateTime = ll >> 32;
    return fileTime;
}

// ---------------------------------------------------------------------------
std::wostream& operator<<(std::wostream& out, const FILETIME& utcFT)
{
    FILETIME   ltzFT;
    SYSTEMTIME sysTime;

	sysTime.wDay = 0;
	sysTime.wDayOfWeek = 0;
	sysTime.wHour = 0;
	sysTime.wMilliseconds = 0;
	sysTime.wMinute = 0;
	sysTime.wMonth = 0;
	sysTime.wSecond = 0;
	sysTime.wYear = 0;

    FileTimeToLocalFileTime(&utcFT, &ltzFT);    // convert UTC to local Timezone
    FileTimeToSystemTime(&ltzFT, &sysTime);
 
    wchar_t szLocalDate[255], szLocalTime[255];
    szLocalDate[0] = szLocalTime[0] = '\0';

    // GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, szLocalDate, sizeof(szLocalDate) );
    GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &sysTime, L"MM'/'dd'/'yyyy", szLocalDate, ARRAYSIZE(szLocalDate) );
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, &sysTime, NULL, szLocalTime, ARRAYSIZE(szLocalTime) );

    out << std::setw(10) << szLocalDate << ' ' << std::setw(9) << szLocalTime;

    return out;
}

