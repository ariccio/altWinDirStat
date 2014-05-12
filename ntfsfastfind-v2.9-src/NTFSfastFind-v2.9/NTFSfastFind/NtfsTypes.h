// ------------------------------------------------------------------------------------------------
// NTFS Master File Table structures.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>

#pragma pack(push, curAlignment)
#pragma pack(1)
// ------------------------------------------------------------------------------------------------
// MFT record header and attribute header  
struct MFT_FILE_HEADER
{
	char		szSignature[4];		// Signature "FILE"
	WORD		wFixupOffset;		// offset to fixup pattern
	WORD		wFixupSize;			// Size of fixup-list +1
	LONGLONG	n64LogSeqNumber;	// log file seq number
	WORD		wSequence;			// sequence nr in MFT
	WORD		wHardLinks;			// Hard-link count
	WORD		wAttribOffset;		// Offset to seq of Attributes
	WORD		wFlags;				// 0x01 = NonRes; 0x02 = Dir
	DWORD		dwRecLength;		// Real size of the record
	DWORD		dwAllLength;		// Allocated size of the record
	LONGLONG	n64BaseMftRec;		// ptr to base MFT rec or 0
	WORD		wNextAttrID;		// Minimum Identificator +1
	WORD		wFixupPattern;		// Current fixup pattern
	DWORD		dwMFTRecNumber;		// Number of this MFT Record
								// followed by resident and
								// part of non-res attributes
};

// ------------------------------------------------------------------------------------------------
struct	NTFS_ATTRIBUTE   
{					     
	DWORD	dwType;
	WORD	wFullLength;
	WORD	wPad;
	BYTE	uchNonResFlag;
	BYTE	uchNameLength;
	WORD	wNameOffset;
	WORD	wFlags;
	WORD	wID;

	union ATTR
	{
		struct RESIDENT
		{
			DWORD	dwLength;
			WORD	wAttrOffset;
			BYTE	uchIndexedTag;
			BYTE	uchPadding;
		} Resident;

		struct NONRESIDENT
		{
			LONGLONG	n64StartVCN;
			LONGLONG	n64EndVCN;
			WORD		wDatarunOffset;
			WORD		wCompressionSize; // compression unit size
			BYTE		uchPadding[4];
			LONGLONG	n64AllocSize;
			LONGLONG	n64RealSize;
			LONGLONG	n64StreamSize;
			// data runs...
		}NonResident;
	}Attr;
} ;

// ------------------------------------------------------------------------------------------------
//  Attributes 
struct MFT_STANDARD 
{
	LONGLONG	n64Create;		    // Creation time
	LONGLONG	n64Modify;		    // Last Modify time
	LONGLONG	n64Modfil;		    // Last modify of MFT record
	LONGLONG	n64Access;		    // Last Access time
	DWORD		dwFATAttributes;    // As FAT + 0x800 = compressed
	DWORD		dwMaxNumVersions;	   
	DWORD		dwVersionNum;	
    DWORD       dwClassId;
#if 0
    DWORD       dwOwnerId;          // Win2k
    DWORD       dwSecurityId;       // Win2k
    LONGLONG    n64QutoaCharged;    // Win2k
    LONGLONG    n64UpdateSeqNum;    // Win2k  
#endif
};   
  
// ------------------------------------------------------------------------------------------------
struct MFT_FILEINFO
{
	LONGLONG	dwMftParentDir;         // Seq[2] parent-dir[6] MFT entry
	LONGLONG	n64Create;              // Creation time
	LONGLONG	n64Modify;              // Last Modify time
	LONGLONG	n64Modfil;              // Last modify of MFT record
	LONGLONG	n64Access;              // Last Access time
	LONGLONG	n64Allocated;           // Allocated disk space
	LONGLONG	n64RealSize;            // Size of the file
	DWORD		dwFlags;				// attribute
	DWORD		dwEAsReparsTag;			// Used by EAs and Reparse
	BYTE		chFileNameLength;
	BYTE		chFileNameType;         // 8.3 / Unicode
	wchar_t		wFilename[256];         // Name (in Unicode ?)
}; 

const LONGLONG sMaxFileSize = 0xffffffffffff;
const LONGLONG sParentMask  = 0xffffffffff;

// http://inform.pucp.edu.pe/~inf232/Ntfs/ntfs_doc_v0.5/attributes/file_name.html
enum MFTFileInfoFlags  // dwFlags
{
    eReadOnly    = 0x00000001,
    eHidden      = 0x00000002, 
    eSystem      = 0x00000004, 
    eArchive     = 0x00000020,
    eDevice      = 0x00000040,
    eTemporary   = 0x00000100,
    eSparseFile  = 0x00000200,
    eReparsePoint= 0x00000400,
    eCompressed  = 0x00000800,
    eOffLine     = 0x00001000,
    eNotContent  = 0x00002000,
    eEncrypted   = 0x00004000,
    eDirectory   = 0x10000000,
    eIndexView   = 0x20000000,
};

enum MFTFileInfoTypes   // chFileNameType
{
    ePOSIX  = 0,
    eUnicode= 1,
    eDOS    = 2,
    eBoth   = 3
};

struct MFT_INDEX_HEADER
{
    DWORD   offsetEntry;        // always 0x10 (16)
    DWORD   totalSizeEntries;   // sizeofEntries + 16     
    DWORD   allocSizeEntries;      
    WORD    hasLargeIndex;      // 0=fits in Index, 1=Large Index
    WORD    pad;
};

struct MFT_INDEX_ENTRY
{
    LONGLONG    fileRef;
    WORD        size;           // Sizeof entry (rounded up to 8)
    WORD        fileInfoSize;   // sizeof(MFT_FILEINFO) + fileInfo.wFilename.length
    BYTE        flags;          // 1=subnodes, 2=last
    BYTE        pad[3];
    MFT_FILEINFO fileInfo;      // 66 bytes + wFilename length
};

struct MFT_INDEX_ROOT
{
    DWORD   attribute;      // always 0x30 ? or is 0x90
    DWORD   collation;      // always 1
    DWORD   size;           // size of index record
    DWORD   numCperIdx;     // number of clusters per index record.

    MFT_INDEX_HEADER header;
    MFT_INDEX_ENTRY  entries[1];
};

struct MFT_INDEX_ALLOCATION
{ 
    char        magicNumber[4];    // "INDX"
    WORD        updateSeqOffs; 
    WORD        sizeOfUpdateSequenceNumberInWords; 
    LONGLONG    logFileSeqNum;      // LSN
    LONGLONG    vcnOfINDX; 
    DWORD       indexEntryOffs;     // entries[] at 24 + indexEntryOffs
    DWORD       sizeOFEntries; 
    DWORD       sizeOfEntryAlloc; 
    BYTE        flags; 
    BYTE        padding[3]; 
    // WORD        updateSeq; 
};

#pragma pack(pop, curAlignment)