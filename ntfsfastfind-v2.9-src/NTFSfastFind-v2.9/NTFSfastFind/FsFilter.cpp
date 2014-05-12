// ------------------------------------------------------------------------------------------------
// FileSystem filter classes used to limit output of file system scan by filtering on 
// name, date or size.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include "FsFilter.h"
#include "Pattern.h"

#include <Windows.h>
#include <time.h>

#include <iostream>

// ------------------------------------------------------------------------------------------------

bool IsDateModifyGreater(const MFT_STANDARD & attr, const FILETIME& fileTime)
{
    return (CompareFileTime((const FILETIME*)&attr.n64Modify, &fileTime) > 0);
}

bool IsDateModifyEqual(const MFT_STANDARD & attr, const FILETIME& fileTime)
{
    return (CompareFileTime((const FILETIME*)&attr.n64Modify, &fileTime) == 0);
}

bool IsDateModifyLess(const MFT_STANDARD & attr, const FILETIME& fileTime)
{
    return (CompareFileTime((const FILETIME*)&attr.n64Modify, &fileTime) < 0);
}


// ------------------------------------------------------------------------------------------------

bool IsNameIcase(const MFT_FILEINFO& aName, const std::wstring& name)
{
    // return (wcsnicmp((wchar_t*)aName.wFilename, name.c_str(), aName.chFileNameLength) == 0);
    bool match = Pattern::CompareNoCase(name.c_str(), (wchar_t*)aName.wFilename);
    return match;
}

bool IsName(const MFT_FILEINFO& aName, const std::wstring& name)
{
    // return (wcsncmp((wchar_t*)aName.wFilename, name.c_str(), aName.chFileNameLength) == 0);
    bool match = Pattern::CompareCase(name.c_str(), (wchar_t*)aName.wFilename);
    return match;
}


// ------------------------------------------------------------------------------------------------

bool IsSizeGreater(const MFT_FILEINFO& aName, LONGLONG size)
{
    return (aName.n64RealSize > size);
}

bool IsSizeEqual(const MFT_FILEINFO& aName, LONGLONG size)
{
    return (aName.n64RealSize == size);
}

bool IsSizeLess(const MFT_FILEINFO& aName, LONGLONG size)
{
    return (aName.n64RealSize < size);
}

