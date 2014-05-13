// ------------------------------------------------------------------------------------------------
// Simple pattern matching class.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include "Pattern.h"
#include <iostream>
#include "std_pre.h"
// Initialize static members
//
bool (*Pattern::ChrCmp)(wchar_t c1, wchar_t c2) = Pattern::NCaseChrCmp;
static wchar_t sDirChr = L'\\';

//-----------------------------------------------------------------------------
// Simple wildcard matcher, used by directory file scanner.
// Patterns supported:
//          ?        ; any single character
//          *        ; zero or more characters

bool Pattern::Compare(const wchar_t* wildStr, int wildOff, const wchar_t* rawStr, int rawOff)
{
#ifdef TRACING
	std::cout << std::endl << "\tCompare: " << TRACE_OUT(wildStr) << TRACE_OUT(wildOff) << TRACE_OUT(rawStr) << TRACE_OUT(rawOff) << std::endl;
#endif

    const wchar_t EOS = L'\0';
    while (wildStr[wildOff])
    {
        if (rawStr[rawOff] == EOS)
            return (wcscmp(wildStr+wildOff, L"*") == 0);

        if (wildStr[wildOff] == L'*')
        {
            if (wildStr[wildOff + 1] == EOS)
                return true;

            do
            {
                // Find match with char after '*'
                while (rawStr[rawOff] && 
                    !Pattern::ChrCmp(rawStr[rawOff], wildStr[wildOff+1]))
                    rawOff++;
                if (rawStr[rawOff] &&
                    Compare(wildStr, wildOff + 1, rawStr, rawOff))
                        return true;
                if (rawStr[rawOff])
                    ++rawOff;
            } while (rawStr[rawOff]);

            if (rawStr[rawOff] == EOS)
                return (wcscmp(wildStr+wildOff+1,  L"*") == 0);
        }
        else if (wildStr[wildOff] == L'?')
        {
            if (rawStr[rawOff] == EOS)
                return false;
            rawOff++;
        }
        else
        {
            if (!Pattern::ChrCmp(rawStr[rawOff], wildStr[wildOff]))
                return false;
            if (wildStr[wildOff] == EOS)
                return true;
            ++rawOff;
        }

        ++wildOff;
    }

    return (wildStr[wildOff] == rawStr[rawOff]);
}
