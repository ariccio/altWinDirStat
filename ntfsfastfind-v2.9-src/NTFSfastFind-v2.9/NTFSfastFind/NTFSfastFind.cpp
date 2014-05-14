// ------------------------------------------------------------------------------------------------
// Program to scan NTFS's Master File Table for matching file by name, date or size filters.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include <iostream>
#include <sys/stat.h>
#include <memory.h>
#include <memory>
#include <math.h>
#include <vector>
#include "BaseTypes.h"

#include "FsUtil.h"
#include "FsFilter.h"
#include "NtfsUtil.h"

#include "GetOpts.h"

#include "std_pre.h"

#define TRACING

#define _VERSION "v2.9"

char sUsage[] =
    "\n"
    "NTFS Fast File Find " _VERSION " - " __DATE__ "\n"
    "By: Dennis Lang\n"
    "https://home.comcast.net/~lang.dennis/\n"
    "\n"
    "Description:\n"
    "   NTFSfastFind searches NTFS Master File Table (MFT) rather then iterating across directories.\n"
    "   NTFSfastFind does not use or maintain an index database\n"
    "   By reading the MFT directly, NTFSfastFind can locate files anywhere on a disk quickly.\n"
    "   Note: Standard directory searching is faster if you know the directory to search.\n"
    "   If you don't know the directory and need to search the entire disk drive, NTFSfastFind is fast.\n"
    "\n"
    "   If you use the -z switch, it will iterate across the directories rather then using MFT.\n"
    "\n"
    "Use:\n"
    "   NTFSfastFind [options] <localNTFSdrivetoSearch>... \n"
    " Filter:\n"
    "   -d <count>                        ; Filter by data stream count  \n"
    "   -f <fileFilter>                   ; Filter by filename, use * or ? patterns \n"
    "   -s <size>                         ; Filter by file size  \n"
    "   -t <relativeModifyDate>           ; Filter by time modified, value is relative days \n"
    "   -z                                ; Force slow style directory search \n"
    " Report:\n"
    "   -A[=s|h|r|d|c]                    ; Include attributes, filter on attributes \n"
    "   -D                                ; Include directory \n"
    "   -I                                ; Include mft index \n"
    "   -S                                ; Include size \n"
    "   -T                                ; Include time \n"
    "   -V                                ; Include VCN array \n"
    "   -#                                ; Include stream and name counts \n"
    "\n"
    "   -Q                                ; Query / Display MFT information only \n"
    "\n"
    " Examples:\n"
    "  No filtering:\n"
    "    c:                 ; scan c drive, display filenames. \n"
    "    -ITSA  c:          ; scan c drive, display mft index, time, size, attributes, directory. \n"
    "  Filter examples (precede 'f' command letter with ! to invert rule):\n"
    "    -f *.txt d:        ; files ending in .txt on d: drive \n"
    "    -f \\*\\foo*\\*.txt d:; files ending in .txt on d: drive in directory starting with foo \n"
    "    -!f *.txt d:       ; files NOT ending in .txt on d: drive \n" 
    "    -t 2.5 -f *.log    ; modified more than 2.5 days and ending in .log on c drive \n"
    "    -t -7 e:           ; modified less than 7 days ago on e drive \n"
    "    -s 1000 d:         ; file size greater than 1000 bytes on d drive \n"
    "    -s -1000 d: e:     ; file size less than 1000 bytes on d and e drive \n"
    "    -f F* c: d:        ; limit scan to files starting with F on either C or D \n"
    "    -d 1 d:            ; files with more than 1 data stream on d: drive \n"
    "    -Q c:              ; Display special NTFS files\n"
    "    -z c:\\windows\\system32\\*.dll   ; Force slow directory search. \n"
    ;

// ------------------------------------------------------------------------------------------------
// Convert error number to semi-readable string.
std::wstring ErrorMsg(DWORD error)
{
	wchar_t *lpMsgBuf = NULL;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, 
        NULL);
	if ( lpMsgBuf != NULL ) {
		std::wstring msg( lpMsgBuf );
		LocalFree( lpMsgBuf );
		return msg;
		}
	else {
		return std::wstring(L"Failed to format string!");
		}
}

// ------------------------------------------------------------------------------------------------
int NTFSfastFind(
    const wchar_t* path, 
    NtfsUtil::ReportCfg& reportCfg, 
    std::wostream& wout,
    StreamFilter* pStreamFilter)
{
#ifdef TRACING
	std::cout << std::endl << "\tNTFSfastFind: " << TRACE_OUT(path) << std::endl;
#endif
    wchar_t driveLetter = FsUtil::GetDriveLetter(path);
    DWORD error;
    unsigned phyDrvNum = 0;
    unsigned partitionNum = 0;

    wchar_t volumePath[] = L"\\\\.\\C:";
    volumePath[4] = towupper(driveLetter);
    reportCfg.volume = volumePath+4;
    error = FsUtil::GetDriveAndPartitionNumber(volumePath, phyDrvNum, partitionNum);
#ifdef TRACING
	std::cout << TRACE_OUT( driveLetter ) << TRACE_OUT( volumePath ) << TRACE_OUT( reportCfg.volume ) << std::endl;
#endif

	if (error != ERROR_SUCCESS)
    {
        std::wcerr << "Error " << ErrorMsg(error).c_str() << std::endl;
        return error;
    }

    wchar_t physicalDrive[]= L"\\\\.\\PhysicalDrive0";
    FsUtil::DiskInfoList diskInfoList;
    
    // ARRAYSIZE includes string terminating null, so backup 2 characters.
    physicalDrive[ARRAYSIZE(physicalDrive)-2] += (char)phyDrvNum;

    error = FsUtil::GetLogicalDrives(physicalDrive, diskInfoList, FsUtil::eFsALL);
    if (error != 0)
    {
        std::wcerr << "Error " << ErrorMsg(error).c_str() << std::endl;
        return error;
    }
   
    NtfsUtil ntfsUtil;

    if (reportCfg.queryInfo)
        error = ntfsUtil.QueryMFT(physicalDrive, diskInfoList[partitionNum], reportCfg, wout, pStreamFilter);
    else
        error = ntfsUtil.ScanFiles(physicalDrive, diskInfoList[partitionNum], reportCfg, wout, pStreamFilter);
    if (error != 0)
    {
        std::wcerr << "Some Damn Error: " << ErrorMsg(error).c_str() << std::endl;
    }
    return error;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Scan file system using Directory API, which is slower than using NTFS but can be
// faster if limited to a subdirectory.
class DirSlowFind
{
public:
    DirSlowFind(NtfsUtil::ReportCfg& reportCfg, std::wostream& wout) :
        m_reportCfg(reportCfg),
        m_wout(wout),
        m_error(0)
    {
    }

    void ScanFiles();
    void ScanFiles(const wchar_t* path)
    {
        wcscpy_s(m_path, ARRAYSIZE(m_path), path);
        wchar_t* pSlash = wcsrchr(m_path, '\\');
        if (pSlash)
            *pSlash = '\0';
        ScanFiles();
    }

    NtfsUtil::ReportCfg& m_reportCfg;
    std::wostream&       m_wout;
    int                  m_error;

    // Dummy objects so we can call filters in ReportCfg.
    MFTRecord            m_mftRecord;
    NtfsUtil::FileInfo   m_fileInfo;

    wchar_t              m_path[MAX_PATH];
};

// ------------------------------------------------------------------------------------------------
void DirSlowFind::ScanFiles()
{
    WIN32_FIND_DATA FileData;    // Data structure describes the file found
    HANDLE     hSearch;          // Search handle returned by FindFirstFile

    m_fileInfo.directory = m_path+2;

    size_t dirLen = wcslen(m_path);
    wcscpy_s(m_path + dirLen, ARRAYSIZE(m_path) - dirLen, L"\\*");
    dirLen++;

    // Start searching for folder (directories), starting with srcdir directory.

    hSearch = FindFirstFile(m_path, &FileData);
    if (hSearch == INVALID_HANDLE_VALUE)
    {
        std::wcerr << "Error " << ErrorMsg(GetLastError()) 
            << "\nFailed to open directory " << m_path << std::endl;
        m_error = GetLastError();
        return;
    }

    bool isMore = true;
    while (isMore)
    {
        if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if (FileData.cFileName[0] != L'.' || isalnum(FileData.cFileName[1]))
            {
                wcscpy_s(m_path + dirLen, ARRAYSIZE(m_path) - dirLen, FileData.cFileName);
                ScanFiles();
                m_path[dirLen-1] = L'\0';
                m_fileInfo.directory = m_path+2;
                m_path[dirLen-1] = L'\\';
                m_path[dirLen] = L'\0';
            }
        }
        else
        {
            if (m_reportCfg.postFilter->IsMatch(
                    m_mftRecord.m_attrStandard, m_mftRecord.m_attrFilename, &m_fileInfo))
            {
                wcscpy_s(m_mftRecord.m_attrFilename.wFilename, 
                        ARRAYSIZE(m_mftRecord.m_attrFilename.wFilename),
                        FileData.cFileName);

                m_mftRecord.m_attrFilename.chFileNameLength = (BYTE)wcslen(FileData.cFileName);
                m_mftRecord.m_attrFilename.n64Modify =
                    m_mftRecord.m_attrStandard.n64Modify = *(LONGLONG*)&FileData.ftLastWriteTime;

                LARGE_INTEGER fileSize;
                fileSize.HighPart = FileData.nFileSizeHigh;
                fileSize.LowPart = FileData.nFileSizeLow;
                m_mftRecord.m_attrFilename.n64RealSize = fileSize.QuadPart;
                    
                if (m_reportCfg.readFilter->IsMatch(
                    m_mftRecord.m_attrStandard, m_mftRecord.m_attrFilename, &m_fileInfo))
                {
                    m_wout << m_path << "\\" <<  FileData.cFileName << std::endl;        
                }
            }
        }

        isMore = (FindNextFile(hSearch, &FileData) != 0) ? true : false;
    }

    // Close directory before calling callback incase client wants to delete dir.
    FindClose(hSearch);
}

// ------------------------------------------------------------------------------------------------
void AddFileFilter(const wchar_t* argv, NtfsUtil::ReportCfg& reportCfg, bool matchOn)
{
    const wchar_t* pName = wcsrchr(argv, reportCfg.slash);
    if (pName == NULL)
    {
        reportCfg.readFilter->List().push_back(new MatchName(argv, IsNameIcase, matchOn));
    }
    else
    {
        if (pName[1] != '\0' && pName[1] != '*')
            reportCfg.readFilter->List().push_back(new MatchName(pName+1, IsNameIcase, matchOn));
        std::wstring dirPat(argv, size_t(pName - argv));
        reportCfg.postFilter->List().push_back(new MatchDirectory(dirPat.c_str(), matchOn));
        reportCfg.directoryFilter = true;
    }
}

byte placeholder_func( ) {
	byte new_byte = 0;
	return new_byte;
	}



// ------------------------------------------------------------------------------------------------
int wmain(int argc, const wchar_t* argv[])
{
#ifdef TRACING
	std::cout << "argc: " << argc << ", argv: `";
	for ( auto i = 0; i < argc - 1; ++i ) {
		std::cout << "`, `" << argv[i];
		}
	std::cout << "`" << std::endl;
	std::cout << std::endl;
#endif

		{
		//const double the_val = pow( 2.00, 30.00 );
		auto zero = std::make_unique<std::vector<byte>>();
		zero->resize( 1040187392, 0 );
		//for ( auto aByte : *zero ) {
		//	
		//	}
		zero.reset( );
		byte placeholder = placeholder_func();


		}

	const wchar_t* path = L"c:\\";
    NtfsUtil::ReportCfg reportCfg;
    bool matchOn = true;
    bool doDirIterating = false;
    StreamFilter streamFilter;  // TODO - add members and logic to class



    if (argc == 1)
    {
        std::wcout << sUsage;
        return 0;
	}

    GetOpts<wchar_t> getOpts(argc, argv, L"!#A:DIQSTVd:f:s:t:z?");

    while (getOpts.GetOpt())
    {
        switch (getOpts.Opt())
        {
        case '!':   // not
            matchOn = false;
            break;
        case '#':   // number of names and streams
            reportCfg.nameCnt = true;
            reportCfg.streamCnt = true;
            break;
        case 'A':   // attributes
            reportCfg.attribute = !reportCfg.attribute;
            reportCfg.attributes = -1;
            if (getOpts.OptArg() != NULL && *getOpts.OptArg() == '=')
            {
                reportCfg.attributes = 0;
                for (const wchar_t* optStr = getOpts.OptArg()+1; *optStr != '\0'; optStr++)
                {
                    switch (tolower(*optStr))
                    {
                    case 'r':
                        reportCfg.attributes |= eReadOnly;
                    case 's': 
                        reportCfg.attributes |= eSystem;
                        break;
                    case 'h':   
                        reportCfg.attributes |= eHidden;
                        break;
                    case 'd':
                        reportCfg.attributes |= eDirectory;
                        break;
                    case 'f':   // files
                        reportCfg.attributes = ~eDirectory;
                        break;
                    case 'c':
                        reportCfg.attributes |= eCompressed;
                        break;
                    default:
                        std::wcerr << "Invalid attribute argument:" << getOpts.OptArg() << std::endl;
                        break;
                    }
                }
            }
            break;
        case 'D':   // directory path
            reportCfg.directory = !reportCfg.directory;
            break;
        case 'I':   // mft index
            reportCfg.mftIndex = !reportCfg.mftIndex;
            break;
        case 'Q':   // query info
#ifdef TRACING
			std::wcout << "argument: Query Info!" << std::endl;
#endif
            reportCfg.queryInfo = true;
            reportCfg.attributes = eSystem;
            break;
        case 'S':   // size
            reportCfg.size = !reportCfg.size;
            break;
        case 'T':   // modify time
            reportCfg.modifyTime = !reportCfg.modifyTime;
            break;
        case 'V':   // show VCN array
            reportCfg.showVcn = true;
            break;

        case 'd':   // data stream count
            {
                wchar_t* endPtr;
                long fileSize = wcstol(getOpts.OptArg(), &endPtr, 10);
                if (endPtr == getOpts.OptArg())
                {
                    std::wcerr << "Invalid Size argument:" << getOpts.OptArg() << std::endl;
                    return -1;
                }
                reportCfg.readFilter->List().push_back(new StreamCntMatch(labs(fileSize), fileSize > 0 ? IsCntGreater : IsCntLess, matchOn));
            }
            matchOn = true;
            break;

        case 'f':
            AddFileFilter(getOpts.OptArg(), reportCfg, matchOn);
            matchOn = true;
            break;

        case 's':   // size
            {
                wchar_t* endPtr;
                long fileSize = wcstol(getOpts.OptArg(), &endPtr, 10);
                if (endPtr == getOpts.OptArg())
                {
                    std::wcerr << "Invalid Size argument:" << getOpts.OptArg() << std::endl;
                    return -1;
                }
                reportCfg.readFilter->List().push_back(new MatchSize(labs(fileSize), fileSize > 0 ? IsSizeGreater : IsSizeLess, matchOn));
            }
            matchOn = true;
            break;

        case 't':
            {
                wchar_t* endPtr;
                double days = wcstod(getOpts.OptArg(), &endPtr);
                if (endPtr == getOpts.OptArg())
                {
                    std::wcerr << "Invalid Modify Days argument, expect floating point number\n";
                    return -1;
                }
                FILETIME  daysAgo = FsTime::TodayUTC() - FsTime::TimeSpan::Days(fabs(days));
                reportCfg.readFilter->List().push_back(new MatchDate(daysAgo, days < 0 ? IsDateModifyGreater : IsDateModifyLess, matchOn));
                // std::wcout << "Today      =" << FsTime::TodayUTC() << std::endl;
                // std::wcout << "Filter date=" << daysAgo << std::endl;
            }
            matchOn = true;
            break;

        case 'z':
            doDirIterating = true;
            break;

        default:
        case '?':
            std::wcout << sUsage;
            return 0;
        }
    }

    int error = 0;
    if (getOpts.NextIdx() < argc)
    {
        for (int optIdx = getOpts.NextIdx(); optIdx < argc; optIdx++)
        {
            reportCfg.PushFilter();
            reportCfg.directoryFilter = 
                    !reportCfg.postFilter.IsNull() && reportCfg.postFilter->List().size() != 0;

            const wchar_t* arg = argv[optIdx];
            if (wcslen(arg) > 3 && arg[1] == ':')
            {
                if (arg[2] == '\\')
                    AddFileFilter(arg + 3, reportCfg, true);
                else  
                    AddFileFilter(arg + 2, reportCfg, true);
            }

            if (doDirIterating)
            {
#ifdef TRACING
				std::wcout << "Iterating the slow way. " << TRACE_OUT( reportCfg.attribute ) << TRACE_OUT( reportCfg.attributes ) << TRACE_OUT( reportCfg.directory ) << TRACE_OUT( reportCfg.directoryFilter ) << TRACE_OUT( reportCfg.mftIndex ) << TRACE_OUT( reportCfg.modifyTime ) << TRACE_OUT( reportCfg.name ) << TRACE_OUT( reportCfg.nameCnt ) << TRACE_OUT( reportCfg.queryInfo ) << TRACE_OUT( reportCfg.showDetail ) << TRACE_OUT( reportCfg.showVcn ) << TRACE_OUT( reportCfg.size ) << TRACE_OUT( reportCfg.streamCnt ) << TRACE_OUT( reportCfg.separator ) << TRACE_OUT( reportCfg.slash ) << TRACE_OUT( reportCfg.volume ) << std::endl;
#endif
                DirSlowFind dirSlowFind(reportCfg, std::wcout);
                dirSlowFind.ScanFiles(argv[optIdx]);
                error |= dirSlowFind.m_error;
            }
            else
            {
				std::wcout << TRACE_OUT( argv[ optIdx ] ) << TRACE_OUT( reportCfg.attribute ) << TRACE_OUT( reportCfg.attributes ) << TRACE_OUT( reportCfg.directory ) << TRACE_OUT( reportCfg.directoryFilter ) << TRACE_OUT( reportCfg.mftIndex ) << TRACE_OUT( reportCfg.modifyTime ) << TRACE_OUT( reportCfg.name ) << TRACE_OUT( reportCfg.nameCnt ) << TRACE_OUT( reportCfg.queryInfo ) << TRACE_OUT( reportCfg.showDetail ) << TRACE_OUT( reportCfg.showVcn ) << TRACE_OUT( reportCfg.size ) << TRACE_OUT( reportCfg.streamCnt ) << TRACE_OUT( reportCfg.separator ) << TRACE_OUT( reportCfg.slash ) << TRACE_OUT( reportCfg.volume ) << std::endl;
                // ToDo - if multi files on same MFT, reuse previous scan !
                error |= NTFSfastFind(argv[optIdx], reportCfg, std::wcout, &streamFilter);
            }

            reportCfg.PopFilter();
        }
    }
    else
    {
        error = NTFSfastFind(path, reportCfg, std::wcout, &streamFilter);
    }

	return error;
}

