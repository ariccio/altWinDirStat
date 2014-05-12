// ------------------------------------------------------------------------------------------------
// Simple pattern matching class.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------


#pragma once

#include <windows.h>
#include <vector>
#include <ctype.h>

class Pattern
{
public:
    /// Compare simple pattern against string.
    /// Patterns supported:
    ///          ?        ; any single character
    ///          *        ; zero or more characters
    static bool CompareCase(const wchar_t* pattern, const wchar_t* str)
    {
        // ToDo - make case comparison an argument to Compare.
        return Compare(pattern, 0, str, 0);
    }

    /// Compare simple pattern against string.
    /// Patterns supported:
    ///          ?        ; any single character
    ///          *        ; zero or more characters
    static bool CompareNoCase(const wchar_t* pattern, const wchar_t* str)
    {
        // ToDo - make case comparison an argument to Compare.
        return Compare(pattern, 0, str, 0);
    }

    // Text comparison functions.
    static bool YCaseChrCmp(wchar_t c1, wchar_t c2) { return c1 == c2; }
    static bool NCaseChrCmp(wchar_t c1, wchar_t c2) { return tolower(c1) == tolower(c2); }

private:
    static bool (*ChrCmp)(wchar_t c1, wchar_t c2);
    static bool Compare(const wchar_t* wildStr, int wildOff, const wchar_t* rawStr, int rawOff);
};

