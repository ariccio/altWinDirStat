// ------------------------------------------------------------------------------------------------
// Read Master File Table from Windows NTFS file system.
//
// Original code from T.YogaRamanan's Undelete project posted to CodeProject 13-Jan-2005.
// http://www.codeproject.com/KB/files/NTFSUndelete.aspx
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------


#pragma once

#include "BaseTypes.h"
#include "FsFilter.h"
#include "NtfsTypes.h"

#include <map>
#include <string>

#define TRACE_OUT(x) std::endl << "\t" << #x << " = `" << x << "`"

namespace MFTconst
{
const unsigned sSTANDARD_INFORMATION = 0x10;  //  Contains timestamps for the time when the file was created, last modified and last access. Also holds a flag telling if it is a directory, if the file is read-only, if it should be hidden and all the other traditional MS-DOS attributes. [DUC98 /standard.html]  
const unsigned sATTRIBUTE_LIST       = 0x20;  //  This attribute is used to find the extension file record and to see what attributes are stored in it when all attributes cannot fit into one file record. It is very rarely used and is mostly observed in heavily fragmented files. [CUS94, 23]  
const unsigned sFILE_NAME            = 0x30;  //  Holds the file name of the file record. A file record can have multiple file name attributes to support MS-DOS based file names. [CUS94, 23]  
const unsigned sVOLUME_VERSION       = 0x40;  //  This have I not ever encountered, [DUC98 /volumever.html] states that it could have something to do with what version of NTFS that created the volume.  
const unsigned sSECURITY_DESCRIPTOR  = 0x50;  //  Holds information on: who owns the file, who can access it and what the different users can do with it. [CUS94, 23]  
const unsigned sVOLUME_NAME          = 0x60;  //  Just the name of the volume. [DUC98 /volumename.html]  
const unsigned sVOLUME_INFORMATION   = 0x70;  //  Stores the volume dirty flag. [DUC98 /volumeinf.html]  
const unsigned sDATA                 = 0x80;  //  The usual content of a data file. [DUC98 /data.html]  
const unsigned sINDEX_ROOT           = 0x90;  //  These three attributes together store the content of a directory. [CUS94, 23]  
const unsigned sINDEX_ALLOCATION     = 0xa0;  //
const unsigned sBITMAP               = 0xb0;  //
const unsigned sSYMBOLIC_LINK        = 0xc0;  //  Unused. [RUS98Ja]  
const unsigned sEA_INFORMATION       = 0xd0;  //  Used to implement HPFS style extended attribute for OS/2 subsystems and for OS/2 clients of Windows NT file servers. [CUS94, 23]  
const unsigned sEA_DATA              = 0xe0;  //  ??
const unsigned sEND                  = 0xf0;  //  ??

const unsigned sMaxSizeAny = (unsigned)-1;
};

// ------------------------------------------------------------------------------------------------

class MFTRecord  
{
public:
	MFTRecord();
	virtual ~MFTRecord();

	int SetRecordInfo(LONGLONG n64StartPos, DWORD dwRecSize, DWORD dwBytesPerCluster);

	void SetDriveHandle(HANDLE hDrive)
    {  m_hDrive = hDrive; }

	int ExtractFile(const Block& inMFTBlock, bool loadData=false, size_t maxDataSize=0xffffffff)
    { return ExtractFileOrMFT(inMFTBlock, loadData, maxDataSize); }

    // Call if you want to see the stream names and m_streamCnt != 0
    int ExtractStream(const Block& inMFTBlock, StreamFilter* pStreamFilter)
    { return ExtractFileOrMFT(inMFTBlock, false, MFTconst::sMaxSizeAny, NULL, pStreamFilter); }

    int ExtractMFT(const Block& inMFTBlock, const FsFilter& filter, size_t maxDataSize=0xffffffff)
    { return ExtractFileOrMFT(inMFTBlock, true, maxDataSize, &filter); }

  
    struct MFTitem
    {
        DWORD                   type;
        const NTFS_ATTRIBUTE*   pNTFSAttribute;
        Block                   data;
    };
    typedef std::vector<MFTitem> ItemList;

    int ExtractItems(const Block& inMFTBlock, ItemList& itemList, size_t maxDataSize=0xffffffff);

	int ReadRaw(LONGLONG n64LCN, Buffer& chData, DWORD64 dwLen, const FsFilter* pMFTFilter=NULL);
    
public:
    //  attributes  
	MFT_STANDARD    m_attrStandard;
	MFT_FILEINFO    m_attrFilename;

    // List of (disk_LCN, disk_byte_length)
    typedef std::vector<std::pair<LONGLONG,LONGLONG>> FileOnDiskList;
    FileOnDiskList  m_fileOnDisk;
	Buffer          m_outFileData;  // Raw data of file loaded.
	bool            m_bInUse;       // false = deleted
    bool            m_bSparse;
    unsigned        m_nameCnt;      // number of name attributes found.
    unsigned        m_streamCnt;    // number of data streams found.
    unsigned        m_fragCnt;      // number of allocation fragments. 

    static char*    sMFTRecordTypeStr[];

protected:
	HANDLE	        m_hDrive;   // Does not own handle, shares it with parent.
 	Block           m_MFTBlock;
	DWORD           m_dwMFTRecSize;
	DWORD           m_dwCurPos;
	DWORD           m_dwBytesPerCluster;
	LONGLONG        m_n64StartPos;

    int ExtractFileOrMFT(const Block& inMFTBlock, 
            bool loadData=false, size_t maxFile=0xfffffff, 
            const FsFilter* pMFTFilter=NULL,
            const StreamFilter* pStreamFilter=NULL);

    int ExtractData(const NTFS_ATTRIBUTE& ntfsAttr, 
            Buffer& outBuffer, size_t maxSize, const FsFilter* pMFTFilter=NULL);

    int ExtractDataPos(const NTFS_ATTRIBUTE& ntfsAttr, 
            Buffer& outBuffer, size_t maxSize, const FsFilter* pMFTFilter=NULL);

public:
    typedef DWORD   TypeCnt[16];
    const TypeCnt& GetTypeCnts() const
    { return m_typeCnt; }

protected:
    // Query information
    TypeCnt   m_typeCnt;    
};
