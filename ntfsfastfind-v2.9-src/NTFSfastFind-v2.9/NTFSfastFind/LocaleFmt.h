// ------------------------------------------------------------------------------------------------
// Handle Locale (Language and Country) specific formatting.
// This was derived from publicly available source code XFormatNumber 
// http://www.codeproject.com/script/Articles/ViewDownloads.aspx?aid=2492
//
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <winnls.h>

namespace LocaleFmt
{
// Format number and automatically add thousand separators.
//
//  Ex:
//      wchar_t str[20];    // big enough for final value with commas.
//      float    fValue;
//      int      dValue;
//      LONGLONG llvalue;
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%f", fValue);
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%.2f", fValue);
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%d", dValue);
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%lld", llValue);

extern wchar_t* snprintf(wchar_t* str, unsigned maxChar, wchar_t* fmt, ...);

// Get Locale number format (used internally by snprintf).
extern const NUMBERFMT& GetNumberFormat();
}

