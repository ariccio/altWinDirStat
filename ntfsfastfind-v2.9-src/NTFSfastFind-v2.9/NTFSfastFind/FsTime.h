// ------------------------------------------------------------------------------------------------
// FileSystem Time class used to manipulate time.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>

#include <iostream>
#include <time.h>
#include <assert.h>

class FsTime
{
public:
    FsTime(void) {}

    static FILETIME TodayUTC()
    {
        return UnixTimeToFileTime(time(0));
    }

    /// Convert Unix time to FILETIME.
    ///   UnixTime seconds since midnight January 1, 1970 UTC 
    ///   FILETIME  100-nanosecond intervals since January 1, 1601 UTC 
    static FILETIME UnixTimeToFileTime(time_t t);

    /// Convert seconds to nanoseconds.
    static FILETIME SecondsToFileTime(time_t t);

    class TimeSpan
    {
    public:
        TimeSpan(double seconds) : m_seconds(seconds)
        {  }

        operator FILETIME () const
        {
            return SecondsToFileTime(time_t(m_seconds));   
        }

#if 0
        operator LARGE_INTEGER () const
        {
            return *(LARGE_INTEGER*)& SecondsToFileTime(time_t(m_seconds));  
        }
#endif

        static TimeSpan Days(double days)
        { return TimeSpan(days * sSecondsPerDay); }

        static const size_t sSecondsPerDay = 24 * 60 * 60;

    private:
        double m_seconds;
    };
};

inline LONGLONG Quad(const LARGE_INTEGER& li)
{ return li.QuadPart; }
inline LONGLONG Quad(const FILETIME& li)
{ return ((LARGE_INTEGER*)&li)->QuadPart; }

//
// Math operators
//
inline FILETIME operator+(const FILETIME& left, const FsTime::TimeSpan& right)
{
    assert(sizeof(LARGE_INTEGER) == sizeof(FILETIME));

    LARGE_INTEGER result;
    result.QuadPart= Quad(left) + Quad(right);
    return *((FILETIME*)&result);
}

inline FILETIME operator-(const FILETIME& left, const FsTime::TimeSpan& right)
{
    assert(sizeof(LARGE_INTEGER) == sizeof(FILETIME));

    LARGE_INTEGER result;
    result.QuadPart= Quad(left) - Quad(right);
    return *((FILETIME*)&result);
}

//
//  Pretty print time to stream.
//
extern std::wostream& operator<<(std::wostream& out, const FILETIME& utcFT);
