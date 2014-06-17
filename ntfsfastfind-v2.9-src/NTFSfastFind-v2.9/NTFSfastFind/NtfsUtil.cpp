// ------------------------------------------------------------------------------------------------
// Class to read NTFS Master File Table and scan for matching files.
//
// Original code from T.YogaRamanan's Undelete project posted to CodeProject 13-Jan-2005.
// http://www.codeproject.com/KB/files/NTFSUndelete.aspx
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include "NtfsUtil.h"
#include "FsUtil.h"
#include "Hnd.h"
#include "MFTRecord.h"
#include "LocaleFmt.h"
#include "oNullStream.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#define DUMP_DETAIL_MFT

#pragma pack(push, curAlignment)
#pragma pack(1)

// ------------------------------------------------------------------------------------------------
///   boot sector info  
struct NTFS_PART_BOOT_SEC
{
	char		chJumpInstruction[3];
	char		chOemID[4];
	char		chDummy[4];
	
	struct NTFS_BPB
	{
		WORD		wBytesPerSec;
		BYTE		uchSecPerClust;
		WORD		wReservedSec;
		BYTE		uchReserved[3];
		WORD		wUnused1;
		BYTE		uchMediaDescriptor;
		WORD		wUnused2;
		WORD		wSecPerTrack;
		WORD		wNumberOfHeads;
		DWORD		dwHiddenSec;
		DWORD		dwUnused3;
		DWORD		dwUnused4;
		LONGLONG	n64TotalSec;
		LONGLONG	n64MFTLogicalClustNum;
		LONGLONG	n64MFTMirrLogicalClustNum;
		int			nClustPerMFTRecord;
		int			nClustPerIndexRecord;
		LONGLONG	n64VolumeSerialNum;
		DWORD		dwChecksum;
	} bpb;

	char		chBootstrapCode[426];
	WORD		wSecMark;
};
#pragma pack(pop, curAlignment)

// ------------------------------------------------------------------------------------------------
static DWORD ReturnError(DWORD error)
{
	return error;   // handy place to set break point.
}

// ------------------------------------------------------------------------------------------------
NtfsUtil::NtfsUtil( void ) : m_error( 0 ), m_abort( false ), m_slash( '\\' ), m_bInitialized( false ), m_dwStartSector( 0 ), m_dwBytesPerCluster( 0 ), m_dwBytesPerSector( 0 ), m_dwMFTRecordSz( 0 ) { }

// ------------------------------------------------------------------------------------------------
void SumBitCounts( DWORD* outCnt, const DWORD* inCnt, unsigned bitCnt ) {
	unsigned bit = 1;
#ifdef TRACING
	std::wcout << std::endl << "\tSumming BitCounts..." << TRACE_OUT( *outCnt ) << TRACE_OUT( *inCnt ) << TRACE_OUT( bitCnt ) << std::endl;
#endif
	for ( unsigned bitIdx = 0; bitIdx <= bitCnt; bitIdx++ ) {
		if ( ( bitIdx & bit ) != 0 ) {
			outCnt[ bit ] += inCnt[ bitIdx ];
			}
		}
	}

// ------------------------------------------------------------------------------------------------
static void CountReport( const CountFilter::CountInfo& countInfo, std::wostream& wout ) {
	DWORD attrCnt[ 4 ];
	attrCnt[ 0 ] = 0;
	attrCnt[ 1 ] = 0;
	attrCnt[ 2 ] = 0;
	attrCnt[ 3 ] = 0;
	SumBitCounts(attrCnt, countInfo.m_attrCnt, 7);

	wout << "  --ATTRIBUTES (count)--"
		<< "\n              Normal:" << std::setw(15) << countInfo.m_attrCnt[0]
		<< "\n         ReadOnly(R):" << std::setw(15) << countInfo.m_attrCnt[1]
		<< "\n           Hidden(H):" << std::setw(15) << countInfo.m_attrCnt[2]
		<< "\n                 R&H:" << std::setw(15) << countInfo.m_attrCnt[3]
		<< "\n           System(S):" << std::setw(15) << countInfo.m_attrCnt[4]
		<< "\n                 S&R:" << std::setw(15) << countInfo.m_attrCnt[5]
		<< "\n                 S&H:" << std::setw(15) << countInfo.m_attrCnt[6]
		<< "\n               S&R&H:" << std::setw(15) << countInfo.m_attrCnt[7]
		<< "\n"
		<< "\n  --NAME TYPES (count)--"
		<< "\n               POSIX:" << std::setw(15) << countInfo.m_nameTypeCnt[0]  
		<< "\n             Unicode:" << std::setw(15) << countInfo.m_nameTypeCnt[1]   
		<< "\n                 DOS:" << std::setw(15) << countInfo.m_nameTypeCnt[2] 
		<< "\n         Unicode&DOS:" << std::setw(15) << countInfo.m_nameTypeCnt[3] 
		<< "\n"
		<< "\n  --TYPE (count)--"
		<< "\n                File:" << std::setw(15) << countInfo.m_fileCnt
		<< "\n           Directory:" << std::setw(15) << countInfo.m_dirCnt
		<< "\n"
		<< "\n  --SIZE--"
		<< "\n                Real:" << std::setw(15) << countInfo.m_realSize
		<< "\n           Allocated:" << std::setw(15) << countInfo.m_allocSize
		<< std::endl;
	}

#ifdef DUMP_DETAIL_MFT
// ------------------------------------------------------------------------------------------------
template <typename T>
T* MovePtr( T* pEntry, unsigned int off ) {
	return ( T* ) ( ( const char* ) pEntry + off );
	}

// ------------------------------------------------------------------------------------------------
inline bool isPrint( wchar_t w ) {
	// return iswprint(w);
	return w < 256 && isprint( w );
	}

// ------------------------------------------------------------------------------------------------
std::wstring Clean( const wchar_t* inStr, unsigned len ) {
	std::wstring outStr;
	len = min( len, 255 );
	outStr.reserve( len );

	while ( len-- != 0 ) {
		if ( isPrint( *inStr ) ) {
			outStr += *inStr;
			}
		else {
			outStr += L'~';
			}
		inStr++;
		}
#ifdef TRACING
	std::wcout << std::endl << "\tReturning cleaned string:" << TRACE_OUT(outStr) << std::endl;
#endif
	return outStr;
	}

// ------------------------------------------------------------------------------------------------
std::wostream& Format( const MFT_FILEINFO& fileInfo, const NtfsUtil::ReportCfg& reportCfg, std::wostream& wout ) {
	wchar_t numStr[ 30 ];

	if ( wout.bad( ) ) {
		wout.clear( );
		}
	if ( reportCfg.mftIndex ) {
		wout << std::setw( 6 ) << ( fileInfo.dwMftParentDir & sParentMask ) << reportCfg.separator;
		}
	// if (reportCfg.streamCnt)
	//    wout << std::setw(6) << stFInfo.streamCnt  << separator;

	if ( reportCfg.modifyTime ) {
		wout << *( FILETIME* ) &fileInfo.n64Modify << reportCfg.separator;
		}
	if ( reportCfg.size ) {
		wout << std::setw( 20 ) << LocaleFmt::snprintf( numStr, ARRAYSIZE( numStr ), L"%lld", fileInfo.n64RealSize & sMaxFileSize ) << reportCfg.separator;
		}
	if ( reportCfg.attribute ) {
		wout << ( ( eDirectory & fileInfo.dwFlags ) != 0 ? " Dir " : "     " ) << reportCfg.separator << std::setw( 8 ) << std::hex << fileInfo.dwFlags << std::dec << reportCfg.separator;
		}
	// if (reportCfg.nameCnt)
	//     wout << std::setw(6) << stFInfo.nameCnt  << separator;

	// wout << reportCfg.volume;
	// if (reportCfg.directory)
	//     wout << stFInfo.directory << m_slash;
	if ( fileInfo.chFileNameType == eDOS ) {
		wout << "[DOS]";
		}
	else if ( fileInfo.chFileNameType == ePOSIX ) {
		wout << "[POSIX]";
		}
	std::wstring cleanName = Clean(fileInfo.wFilename, fileInfo.chFileNameLength);
	wout << cleanName;
	wout << std::endl;

	if ( wout.bad( ) ) {
		wout.clear( );
		}
	return wout;
}

// ------------------------------------------------------------------------------------------------
void OutLL( std::wostream& wout, const char* label, LONGLONG ll ) {
	wchar_t numStr[ 30 ];
	wout << label << std::setw( 15 ) << LocaleFmt::snprintf( numStr, ARRAYSIZE( numStr ), L"%lld", ll ) << std::endl;
	}
#endif

// ------------------------------------------------------------------------------------------------
DWORD NtfsUtil::QueryMFT( const wchar_t* phyDrv, const DiskInfo& diskInfo, const ReportCfg& reportCfg, std::wostream& wout, StreamFilter* pStreamFilter ) {
#ifdef TRACING
	std::wcout << std::endl << "\tQueryMFT: " << TRACE_OUT(phyDrv) << std::endl;
#endif

	SharePtr<MultiFilter> countFilter = new CountFilter();

	// Return MFT Info

	wout << "\nMFT Information for volume " << reportCfg.volume << "\n\n";

	wout << "\n====System Files====\n";
	ReportCfg myReportCfg = reportCfg;
	myReportCfg.readFilter = countFilter;
	myReportCfg.attribute = myReportCfg.directory = myReportCfg.mftIndex = myReportCfg.modifyTime = myReportCfg.size = true;
	// wonullstream wnull;
	ScanFiles(phyDrv, diskInfo,  myReportCfg, wout, pStreamFilter);

	if ( reportCfg.showDetail ) {
#ifdef TRACING
		wout << std::endl << "\tshowing detail!" << std::endl;
#endif
		// read the only file detail not the file data
		MFTRecord mftRecord;
		mftRecord.SetDriveHandle(m_hDrive);
		mftRecord.SetRecordInfo((LONGLONG)m_dwStartSector * m_dwBytesPerSector, m_dwMFTRecordSz, m_dwBytesPerCluster);
		wout << "\n====MFT StartSector:" << m_dwStartSector << "====\n";

		for ( DWORD fileOff = 0; fileOff < m_copyOfMFT.size( ); fileOff += m_dwMFTRecordSz ) {
			if ( wout.bad( ) ) {
				wout.clear( );
				}
			if ( m_abort ) {
				return ( DWORD ) -2;
				}
			// point the record of the file in the MFT
			Block mftBlock(&m_copyOfMFT[fileOff], m_dwMFTRecordSz);
			MFTRecord::ItemList itemList;
			int nRet = mftRecord.ExtractItems(mftBlock, itemList);
			if ( nRet ) {
				break;
				}
			if ( mftRecord.m_bInUse ) {
				wout << "\n";
				for ( unsigned itemIdx = 0; itemIdx != itemList.size( ); itemIdx++ ) {
					const  MFTRecord::MFTitem& item = itemList[itemIdx];
					wout << "  Record(" << std::hex << item.type << std::dec << ") " << MFTRecord::sMFTRecordTypeStr[(item.type >> 4) & 0xf] << std::endl;
					if ( item.pNTFSAttribute->uchNonResFlag ) {
						OutLL(wout, "    StartVCN: ", item.pNTFSAttribute->Attr.NonResident.n64StartVCN);
						OutLL(wout, "    EndVCN:   ", item.pNTFSAttribute->Attr.NonResident.n64EndVCN);
						OutLL(wout, "    RealSize: ", item.pNTFSAttribute->Attr.NonResident.n64RealSize);
						OutLL(wout, "    AlloSize: ", item.pNTFSAttribute->Attr.NonResident.n64AllocSize);
						OutLL(wout, "    StreamSz: ", item.pNTFSAttribute->Attr.NonResident.n64StreamSize);
						}

					switch (item.type)
					{
					case 0x10: // STANDARD_INFORMATION
						{
						const MFT_STANDARD * pStandard = item.data.OutPtr<MFT_STANDARD >( 0, sizeof( MFT_STANDARD ) );
#ifdef TRACING
						wout << std::endl << "\t\tItem type `STANDARD_INFORMATION`, " << TRACE_OUT(pStandard->dwClassId) << TRACE_OUT(pStandard->dwFATAttributes) << TRACE_OUT(pStandard->dwMaxNumVersions) << TRACE_OUT(pStandard->dwVersionNum) << TRACE_OUT(pStandard->n64Access) << TRACE_OUT(pStandard->n64Create) << TRACE_OUT(pStandard->n64Modfil) << TRACE_OUT(pStandard->n64Modify) << std::endl;
#endif
						}
						break;
					case 0x30: // FILE_NAME
						{
#ifdef TRACING
							wout << std::endl << "\tFILE_NAME" << std::endl;
#endif
							const MFT_FILEINFO* pName = NULL;
							const unsigned FILEINFOsz = sizeof( MFT_FILEINFO ) - sizeof( pName->wFilename );
							if ( item.data.size( ) >= FILEINFOsz ) {
								pName = item.data.OutPtr<MFT_FILEINFO>( 0, FILEINFOsz );
#ifdef TRACING
								wout << std::endl << "\tFILE_NAME data:" << TRACE_OUT(pName->chFileNameLength) << TRACE_OUT(pName->chFileNameType) << TRACE_OUT(pName->dwEAsReparsTag) << TRACE_OUT(pName->dwFlags) << TRACE_OUT(pName->dwMftParentDir) << TRACE_OUT(pName->n64Access) << TRACE_OUT(pName->n64Allocated) << TRACE_OUT(pName->n64Create) << TRACE_OUT(pName->n64Create) << TRACE_OUT(pName->n64Modfil) << TRACE_OUT(pName->n64Modify) << TRACE_OUT(pName->n64RealSize) << TRACE_OUT(pName->wFilename) << std::endl;
#endif

								wout << "    Name:     " << Clean( pName->wFilename, pName->chFileNameLength ).c_str( ) << std::endl;
								OutLL( wout, "    RealSize: ", pName->n64RealSize );
								OutLL( wout, "    AlloSize: ", pName->n64Allocated );
								}
						}
						break;
					case 0x40: // OBJECT_ID
					case 0x50: // SECURITY_DESCRIPTOR
					case 0x60: // VOLUME_NAME
					case 0x70: // VOLUME_INFORMATION
					case 0x80: // DATA
					   wout << "    Location: " << ((item.pNTFSAttribute->uchNonResFlag == 0) ? "Resident" : "NonResident") << std::endl;
					   OutLL(wout, "    Size:     ",  
							  ((item.pNTFSAttribute->uchNonResFlag == 0) ? item.pNTFSAttribute->Attr.Resident.dwLength : item.pNTFSAttribute->Attr.NonResident.n64RealSize)
							  );
					   if ( item.pNTFSAttribute->uchNonResFlag ) {
						   if ( item.pNTFSAttribute->uchNameLength != 0 ) {
								if ( pStreamFilter ) {
									// Get Stream Name (currently not passed back to caller)
									std::vector<wchar_t> streamName( item.pNTFSAttribute->uchNameLength + 1 );
									memcpy( &streamName[ 0 ], ( char* ) item.pNTFSAttribute + item.pNTFSAttribute->wNameOffset, item.pNTFSAttribute->uchNameLength * 2 );
									streamName.back( ) = '\0';
									const wchar_t* pStreamName = &streamName[ 0 ];
									wout << " Stream " << pStreamName << " Size:" << item.pNTFSAttribute->wFullLength;
									// DWORD streamLength = 0; // where do we get the stream length from
									// pStreamFilter->IsMatch(pStreamName, pStreamName, streamLength);
									}
								}

						   if ( item.pNTFSAttribute->Attr.NonResident.wDatarunOffset != 0 ) {
								// Sparse file have data runs (I think)
								//
								// DataRuns    [[OL] [DataSize...] [Offset...] ]...
								//  First byte, low nibble is byte length of DataSize value
								//              high nibble is byte length of Offset value
								//  Repeat until OL is zero.
								// ( [LengthOfSizes] [DataSize] [Offset] ) ... repeat until LengthOfSizes is zero.
								const BYTE* pRunList = ( const BYTE* ) item.pNTFSAttribute + item.pNTFSAttribute->Attr.NonResident.wDatarunOffset;
								wout << " RunLength=" << std::hex << *pRunList << std::dec;
								}
							}
						break;
					case 0x90: //INDEX_ROOT
						{
							const unsigned INDEX_ROOTsz = 16; // sizeof(MFT_INDEX_ROOT) - sizeof(pIndex->entries[0].fileInfo.wFilename);
#ifdef TRACING
							wout << std::endl << "\t\tIndex root found!" << std::endl;
#endif
							if ( item.data.size( ) >= sizeof( INDEX_ROOTsz ) ) {
								const MFT_INDEX_ROOT* pIndex = item.data.OutPtr<MFT_INDEX_ROOT>(0, INDEX_ROOTsz); 
								const unsigned ENTRYsz = sizeof(MFT_INDEX_ENTRY) - sizeof(pIndex->entries[0].fileInfo.wFilename);
								assert(pIndex->header.offsetEntry == sizeof(MFT_INDEX_HEADER));
#ifdef TRACING
								wout << std::endl << "\t\tIndex root info:" << TRACE_OUT( pIndex->attribute) << TRACE_OUT( pIndex->collation) << TRACE_OUT( pIndex->entries) << TRACE_OUT( pIndex->header.allocSizeEntries) << TRACE_OUT( pIndex->header.hasLargeIndex ) << TRACE_OUT( pIndex->header.offsetEntry) << TRACE_OUT( pIndex->header.pad) << TRACE_OUT( pIndex->header.totalSizeEntries) << TRACE_OUT( pIndex->numCperIdx) << TRACE_OUT( pIndex->size) << std::endl;
#endif
								OutLL(wout, "    Size:     ", pIndex->size);
								OutLL(wout, "    EntrySize:", pIndex->header.totalSizeEntries);
								OutLL(wout, "    EntryOff: ", pIndex->header.offsetEntry);

								if ( mftRecord.m_bInUse ) {
									const MFT_INDEX_ENTRY* pEntry = pIndex->entries;
									unsigned entrySz = pIndex->header.totalSizeEntries - pIndex->header.offsetEntry;;
									const unsigned FILEINFOsz = sizeof( MFT_FILEINFO ) - sizeof( pEntry->fileInfo.wFilename );
									while ( entrySz > ENTRYsz && pEntry->fileInfoSize >= FILEINFOsz ) {
										wout << "    ";
										Format(pEntry->fileInfo, myReportCfg, wout);
										if ( pEntry->size > entrySz ) {
											entrySz = 0;
											}
										else {
											entrySz -= pEntry->size;
											}
										pEntry = MovePtr( pEntry, pEntry->size );
									}
								}
							}
						}
						break;
					case 0xa0: //INDEX_ALLOCATION
						{
							const MFT_INDEX_ALLOCATION* pIndexAlloc = NULL;
							const unsigned INDEX_ALLOCATIONsz = sizeof(MFT_INDEX_ALLOCATION);

							if ( item.data.size( ) >= sizeof( INDEX_ALLOCATIONsz ) ) {
#ifdef TRACING
								wout << std::endl << "\t\tIndex Allocation found!" << std::endl;
#endif

								pIndexAlloc = item.data.OutPtr<MFT_INDEX_ALLOCATION>(0, INDEX_ALLOCATIONsz); 
								//  const MFT_FILE_HEADER* pHeader = item.data.OutPtr<MFT_FILE_HEADER>(0, sizeof(MFT_FILE_HEADER)); 

								OutLL(wout, "    EntryOff: ", pIndexAlloc->indexEntryOffs);
								OutLL(wout, "    EntrySize:", pIndexAlloc->sizeOFEntries);
								OutLL(wout, "    EntryAllo:", pIndexAlloc->sizeOfEntryAlloc);

								if ( mftRecord.m_bInUse ) {
									const MFT_INDEX_ENTRY* pEntry = (const MFT_INDEX_ENTRY*)pIndexAlloc;
									pEntry = MovePtr(pEntry, INDEX_ALLOCATIONsz + pIndexAlloc->indexEntryOffs - 16);
									const unsigned ENTRYsz = sizeof(MFT_INDEX_ENTRY) - sizeof(pEntry->fileInfo.wFilename);
									const unsigned FILEINFOsz = sizeof(MFT_FILEINFO) - sizeof(pEntry->fileInfo.wFilename);
									unsigned entrySz = pIndexAlloc->sizeOFEntries; 
									while ( entrySz > ENTRYsz && pEntry->fileInfoSize >= FILEINFOsz ) {
										wout << "    ";
										Format(pEntry->fileInfo, myReportCfg, wout);
										if ( pEntry->size > entrySz ) {
											entrySz = 0;
											}
										else {
											entrySz -= pEntry->size;
											}
										pEntry = MovePtr(pEntry, pEntry->size);
										}
									}
								}
						}
						break;
					case 0xb0: //BITMAP
					case 0xc0: //REPARSE_POINT
					case 0xd0: //EA_INFORMATION
					case 0xe0: //EA
					case 0xf0: //PROPERTY_SET
					case 0x100: //LOGGED_UTILITY_STREAM
					case 0x1000: //FIRST_USER_DEFINED_ATTRIBUTE
					default:
						break;
					};
				}
			}
		}
		wout.flush();
	}

	const CountFilter* pCountFilter = ( CountFilter* ) countFilter.Ptr( );
	const CountFilter::CountInfo& activeInfo = pCountFilter->GetActiveInfo( );
	const CountFilter::CountInfo& deletedInfo = pCountFilter->GetDeletedInfo( );

	wout << "\n====Record Summary (Count)====";
	wout << "\n              Active:" << std::setw(15) << activeInfo.m_dirCnt + activeInfo.m_fileCnt;
	wout << "\n                Free:" << std::setw(15) << deletedInfo.m_dirCnt + deletedInfo.m_fileCnt;
	wout << "\n               Total:" << std::setw(15) << activeInfo.m_dirCnt + activeInfo.m_fileCnt + deletedInfo.m_dirCnt + deletedInfo.m_fileCnt;
	wout << "\n           Fragments:" << std::setw(15) << m_fileOnDisk.size();
	wout << "\n";

	wout << "\n====MFT Information (Record Count)====\n";
	for ( unsigned mftRecord = 1; mftRecord < ( MFTconst::sEND >> 4 ); mftRecord++ ) {
		wout << " " << std::setw( 20 ) << MFTRecord::sMFTRecordTypeStr[ mftRecord ] << std::setw( 15 ) << m_typeCnt[ mftRecord ] << std::endl;
		}

	wout << "\n====Active Records====\n";
	CountReport( activeInfo, wout );

	wout << "\n====Free(deleted) Records====\n";
	CountReport( deletedInfo, wout );
   
	return ERROR_SUCCESS;
	}

// ------------------------------------------------------------------------------------------------
template <typename TT>
bool HasBits(TT bits, TT mask)
{
	return (bits & mask) != 0;
}

// ------------------------------------------------------------------------------------------------
DWORD NtfsUtil::ScanFiles( const wchar_t* phyDrv, const DiskInfo& diskInfo, const ReportCfg& reportCfg, std::wostream& wout, StreamFilter* pStreamFilter ) {
#ifdef TRACING
	wout << std::endl << "\tScanFiles: " << TRACE_OUT(phyDrv) << TRACE_OUT(diskInfo.dwBytesPerSector) << TRACE_OUT(diskInfo.dwNTRelativeSector) << TRACE_OUT(diskInfo.dwNumSectors) << TRACE_OUT(diskInfo.dwRelativeSector) << TRACE_OUT(diskInfo.wCylinder) << TRACE_OUT(diskInfo.wHead) << TRACE_OUT(diskInfo.wSector) << TRACE_OUT(diskInfo.wType) << TRACE_OUT(reportCfg.attribute) << TRACE_OUT(reportCfg.attributes) << TRACE_OUT(reportCfg.directory) << TRACE_OUT(reportCfg.directoryFilter) << TRACE_OUT(reportCfg.mftIndex) << TRACE_OUT(reportCfg.modifyTime) << TRACE_OUT(reportCfg.name) << TRACE_OUT(reportCfg.nameCnt) << TRACE_OUT(reportCfg.queryInfo) << TRACE_OUT(reportCfg.separator) << TRACE_OUT(reportCfg.showDetail) << TRACE_OUT(reportCfg.showVcn) << TRACE_OUT(reportCfg.size) << TRACE_OUT(reportCfg.slash) << TRACE_OUT(reportCfg.streamCnt) << TRACE_OUT(reportCfg.volume) << std::endl;
#endif

	if ( !m_hDrive.IsValid( ) ) {
		m_hDrive = CreateFile(phyDrv, GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, NULL);
	   
		if ( !m_hDrive.IsValid( ) ) {
#ifdef TRACING 
			wout << std::endl << "\tHandle is invalid!" << std::endl;
#endif
			return ( m_error = GetLastError( ) );
			}
		}

	// ---- Set the starting sector of the NTFS
	m_dwStartSector     = diskInfo.dwNTRelativeSector;
	m_dwBytesPerSector  = 512;
	m_slash             = reportCfg.slash;

	// ---- Initialize, read all MFT in to the memory and optionally filter resuls.
	int nRet = Initialize( *reportCfg.readFilter );
	if ( nRet ) {
		return ( m_error == nRet );
		}

	wchar_t* separator = reportCfg.separator;
	bool drawHeader = true;
	std::wostringstream wHeading;
	if ( reportCfg.mftIndex ) {
		wHeading << std::setw( 6 ) << "Parent" << separator;
		}
	if ( reportCfg.streamCnt ) {
		wHeading << std::setw( 6 ) << "#Data" << separator;
		}
	if ( reportCfg.modifyTime ) {
		wHeading << "   Modified Date    " << separator;
		}
	if ( reportCfg.size ) {
		wHeading << std::setw( 20 ) << "Size" << separator;
		}
	if ( reportCfg.attribute ) {
		wHeading << " Dir" << separator << std::setw( 8 ) << "Attribute" << separator;
		}
	if ( reportCfg.nameCnt ) {
		wHeading << std::setw( 6 ) << "#Name" << separator;
		}
	wHeading << "Path\n";

	wchar_t numStr[20];
	m_abort = false;
	const DWORD sMaxFiles = (DWORD)-1;     // theoretical max file count is 0xFFFFFFFF
	for ( DWORD fileIdx = 0; fileIdx < sMaxFiles; fileIdx++ ) {
#ifdef TRACING
		wout << std::endl << "\tIteration..." << TRACE_OUT( fileIdx ) << std::endl;
#endif
		if ( m_abort ) {
			return ( DWORD ) -2;
			}
		// Get the file detail one by one.
		NtfsUtil::FileInfo stFInfo;
		StreamFilter streamFilter;      // TODO - fix this 
		nRet = GetSelectedFile(fileIdx, reportCfg.postFilter, stFInfo, reportCfg.directory | reportCfg.directoryFilter, &streamFilter); 
		if ( nRet == ERROR_NO_MORE_FILES ) {
			return 0;
			}
		if ( nRet ) {
			return ( m_error = nRet );
			}
		if ( !stFInfo.bDeleted && stFInfo.filename.length( ) != 0 ) {
			if ( reportCfg.directoryFilter ) {
				// Currently only the directory name is checked via the postFilter.
				static MFT_STANDARD sDummyAttr;
				static MFT_FILEINFO sDummyFileInfo;
				if ( !reportCfg.postFilter->IsMatch( sDummyAttr, sDummyFileInfo, &stFInfo ) ) {
					continue;
					}
				}

			bool goodFile = HasBits(stFInfo.dwAttributes, reportCfg.attributes);
			goodFile |= (stFInfo.dwAttributes == 0 && HasBits(reportCfg.attributes, (DWORD)eSystem));
			goodFile |= ((stFInfo.streamCnt > 1 || stFInfo.nameCnt > 1) && reportCfg.streamCnt);
			goodFile |= (stFInfo.bSparse && HasBits(reportCfg.attributes, (DWORD)eSystem));

			if ( !goodFile ) {
				continue;
				}
			if ( wout.bad( ) ) {
				wout.clear( );
				}
			if ( drawHeader ) {
				drawHeader = false;
				wout << wHeading.str( ).c_str( );
				}

			// wout  << std::setw(5) << fileIdx << separator;

			if ( reportCfg.mftIndex ) {
				wout << std::setw( 6 ) << stFInfo.parentSeq << separator;
				}
			if ( reportCfg.streamCnt ) {
				wout << std::setw( 6 ) << stFInfo.streamCnt << separator;
				}
			if ( reportCfg.modifyTime ) {
				wout << *( FILETIME* ) &stFInfo.n64Modify << separator;
				}
			if (reportCfg.size) {
				wout << std::setw( 19 ) << LocaleFmt::snprintf( numStr, ARRAYSIZE( numStr ), L"%lld", stFInfo.n64Size );
				wout << ( stFInfo.bSparse ? "%" : " " );
				wout << separator;
				}

			if ( reportCfg.attribute ) {
				wout << ( ( eDirectory & stFInfo.dwAttributes ) != 0 ? " Dir " : ( stFInfo.streamCnt > 1 ? " Aux " : "     " ) ) << separator << std::setw( 8 ) << std::hex << stFInfo.dwAttributes << std::dec << separator;
				}
			if ( reportCfg.showVcn )
				if ( stFInfo.m_fileOnDisk.size( ) ) {
					wout << " VCN(" << stFInfo.m_fileOnDisk.size() << ") ";
					for ( unsigned vcnIdx = 0; vcnIdx != stFInfo.m_fileOnDisk.size( ); ++vcnIdx ) {
						wout << stFInfo.m_fileOnDisk[ vcnIdx ].first << "#" << stFInfo.m_fileOnDisk[ vcnIdx ].second / m_dwBytesPerCluster << " ";
						}
					}

			if ( reportCfg.nameCnt ) {
				wout << std::setw( 6 ) << stFInfo.nameCnt << separator;
				}
			wout << reportCfg.volume;
			if ( reportCfg.directory ) {
				wout << stFInfo.directory << m_slash;
				}
			wout << stFInfo.filename;
			wout << std::endl;

		}
	}

	return ERROR_SUCCESS;
	}


// ------------------------------------------------------------------------------------------------
// Initialize will read the MFT entire MFT in to the memory.

int NtfsUtil::Initialize( const FsFilter& filter ) {
	LARGE_INTEGER n84StartPos;
	n84StartPos.QuadPart = ( LONGLONG ) m_dwBytesPerSector*m_dwStartSector;

	// Point to the starting NTFS volume sector in the physical drive
	SetFilePointer( m_hDrive, n84StartPos.LowPart, &n84StartPos.HighPart, FILE_BEGIN );
#ifdef TRACING
	std::wcout << std::endl << "\tSet file pointer to beginning of boot sector!" << TRACE_OUT( m_dwBytesPerSector ) << TRACE_OUT( m_dwStartSector ) << TRACE_OUT( m_hDrive )<< std::endl;
#endif
	m_error = GetLastError( );

	// Read the boot sector for the MFT infomation
	NTFS_PART_BOOT_SEC ntfsBS;
	DWORD dwBytes;
	int nRet = ReadFile( m_hDrive, &ntfsBS, sizeof( NTFS_PART_BOOT_SEC ), &dwBytes, NULL );
#ifdef TRACING
	std::wcout << std::endl << "\tRead boot sector!" << TRACE_OUT( dwBytes ) << TRACE_OUT( ntfsBS.bpb.dwChecksum ) << TRACE_OUT( ntfsBS.bpb.n64MFTLogicalClustNum ) << TRACE_OUT( ntfsBS.bpb.n64MFTMirrLogicalClustNum ) << TRACE_OUT( ntfsBS.bpb.n64TotalSec ) << TRACE_OUT( ntfsBS.bpb.n64TotalSec ) << TRACE_OUT( ntfsBS.bpb.n64VolumeSerialNum ) << TRACE_OUT( ntfsBS.bpb.nClustPerIndexRecord ) << TRACE_OUT( ntfsBS.bpb.nClustPerMFTRecord ) << TRACE_OUT( ntfsBS.bpb.uchMediaDescriptor ) << TRACE_OUT( ntfsBS.bpb.uchSecPerClust ) << TRACE_OUT( ntfsBS.bpb.wBytesPerSec ) << TRACE_OUT( ntfsBS.bpb.wNumberOfHeads ) << TRACE_OUT( ntfsBS.bpb.wSecPerTrack ) << TRACE_OUT( ntfsBS.chOemID ) << TRACE_OUT( ntfsBS.wSecMark ) << std::endl;
#endif
	if ( !nRet ) {
		return GetLastError( );
		}
	if ( memcmp( ntfsBS.chOemID, "NTFS", 4 ) != 0 ) {// Check whether it is realy ntfs
		return ReturnError( ERROR_INVALID_DRIVE );
		}
	/// Cluster is the logical entity
	///  which is made up of several sectors (a physical entity) 
	m_dwBytesPerCluster = ntfsBS.bpb.uchSecPerClust * ntfsBS.bpb.wBytesPerSec;
	m_dwMFTRecordSz = 0x01 << ( ( -1 )*( ( char ) ntfsBS.bpb.nClustPerMFTRecord ) );
#ifdef TRACING
	std::wcout << std::endl << "\tm_dwBytesPerCluster = ntfsBS.bpb.uchSecPerClust * ntfsBS.bpb.wBytesPerSec" << TRACE_OUT( m_dwBytesPerCluster ) << TRACE_OUT( ntfsBS.bpb.uchSecPerClust ) << TRACE_OUT( ntfsBS.bpb.wBytesPerSec ) << std::endl << std::endl;
	std::wcout << TRACE_OUT( m_dwMFTRecordSz ) << std::endl;
#endif
	
	m_oneMFTRecord.resize( m_dwMFTRecordSz );

	// Load entire MFT into m_copyOfMFT

	nRet = LoadMFT( ntfsBS.bpb.n64MFTLogicalClustNum, filter );
	if ( nRet ) {
		return nRet;
		}
	m_bInitialized = true;
	return ERROR_SUCCESS;
	}

// ------------------------------------------------------------------------------------------------
//// nStartCluster is the MFT table starting cluster
///    the first entry of record in MFT table will always have the MFT record of itself

int NtfsUtil::LoadMFT( LONGLONG nStartCluster, const FsFilter& filter ) {
#ifdef TRACING
	std::wcout << std::endl << "\tLoadMFT: " << TRACE_OUT(nStartCluster) << std::endl;
#endif

	int nRet;

	// NTFS starting point
	LARGE_INTEGER n64Pos;
	n64Pos.QuadPart = (LONGLONG)m_dwBytesPerSector*m_dwStartSector;
	
	// MFT starting point
	n64Pos.QuadPart += (LONGLONG)nStartCluster*m_dwBytesPerCluster;
#ifdef TRACING
	std::wcout << std::endl << "\tSetting file pointer to: " << TRACE_OUT(n64Pos.QuadPart) << std::endl;
	std::wcout << std::endl << "\tHigh and low parts:" << TRACE_OUT(n64Pos.HighPart) << TRACE_OUT(n64Pos.LowPart) << std::endl;
#endif
	//  Set the pointer to the MFT start
	nRet = SetFilePointer(m_hDrive, n64Pos.LowPart, &n64Pos.HighPart, FILE_BEGIN);
	if (nRet == 0xFFFFFFFF)
		return GetLastError();

#ifdef TRACING
	std::wcout << std::endl << "\tReading the first record in the NTFS table. The first record in NTFS is always the MFT record." << std::endl;
#endif
	// Reading the first record in the NTFS table.
	// The first record in the NTFS is always MFT record.
	DWORD dwBytes;
	BYTE* pMFTRecord = &m_oneMFTRecord[0];
	nRet = ReadFile( m_hDrive, pMFTRecord, m_dwMFTRecordSz, &dwBytes, NULL );
	if ( !nRet ) {
		return GetLastError( );
		}
#ifdef TRACING
	std::wcout << std::endl << "\tSuccessfully read an MFT record!" << TRACE_OUT( m_hDrive ) << TRACE_OUT( *pMFTRecord ) << TRACE_OUT( m_dwMFTRecordSz ) << TRACE_OUT( dwBytes ) << std::endl;
#endif
	assert( sizeof( MFT_FILE_HEADER ) <= m_dwMFTRecordSz );
	m_NtfsMFT = *( MFT_FILE_HEADER* ) pMFTRecord;

#ifdef TRACING
	std::wcout << std::endl << "\tGot MFT_FILE_HEADER!";
	std::wcout << TRACE_OUT( m_NtfsMFT.dwAllLength ) << TRACE_OUT( m_NtfsMFT.dwMFTRecNumber ) << TRACE_OUT( m_NtfsMFT.dwRecLength ) << TRACE_OUT( m_NtfsMFT.n64BaseMftRec ) << TRACE_OUT( m_NtfsMFT.n64LogSeqNumber ) << TRACE_OUT( m_NtfsMFT.szSignature ) << TRACE_OUT( m_NtfsMFT.wAttribOffset ) << TRACE_OUT( m_NtfsMFT.wFixupOffset ) << TRACE_OUT( m_NtfsMFT.wFixupPattern ) << TRACE_OUT( m_NtfsMFT.wFixupSize ) << TRACE_OUT( m_NtfsMFT.wFlags ) << TRACE_OUT( m_NtfsMFT.wHardLinks ) << TRACE_OUT( m_NtfsMFT.wNextAttrID ) << TRACE_OUT( m_NtfsMFT.wSequence ) << std::endl;
#endif
	// Now extract the MFT record just like the other MFT table records
	MFTRecord mftRecord;
	mftRecord.SetDriveHandle(m_hDrive);
	mftRecord.SetRecordInfo( ( LONGLONG ) m_dwStartSector*m_dwBytesPerSector, m_dwMFTRecordSz, m_dwBytesPerCluster );
#ifdef TRACING
	std::wcout << std::endl << "\tExtracted recordInfo: " << TRACE_OUT( m_dwStartSector*m_dwBytesPerSector ) << TRACE_OUT( m_dwMFTRecordSz ) << TRACE_OUT( m_dwBytesPerCluster ) << std::endl;
#endif
	nRet = mftRecord.ExtractMFT(m_oneMFTRecord, filter);
	if ( nRet ) {
		return nRet;
		}
	const wchar_t sMFTName[] = L"$MFT";
	if ( memcmp( mftRecord.m_attrFilename.wFilename, sMFTName, 8 ) ) {
#ifdef TRACING
		std::wcout << std::endl << "\t`no MFT file available`"<< std::endl;
#endif
		return ReturnError( ERROR_BAD_DEVICE );    // no MFT file available
		}
#ifdef TRACING
	std::wcout << std::endl << "\tGot MFTrecord!";
	std::wcout << std::endl << TRACE_OUT( mftRecord.m_attrFilename.chFileNameLength ) << TRACE_OUT( mftRecord.m_attrFilename.chFileNameType ) << TRACE_OUT( mftRecord.m_attrFilename.dwEAsReparsTag ) << TRACE_OUT( mftRecord.m_attrFilename.dwFlags ) << TRACE_OUT( mftRecord.m_attrFilename.dwMftParentDir ) << TRACE_OUT( mftRecord.m_attrFilename.n64Allocated ) << TRACE_OUT( mftRecord.m_attrFilename.n64RealSize ) << TRACE_OUT( mftRecord.m_attrFilename.wFilename ) << TRACE_OUT( mftRecord.m_attrStandard.dwClassId ) << TRACE_OUT( mftRecord.m_attrStandard.dwFATAttributes ) << TRACE_OUT( mftRecord.m_attrStandard.dwMaxNumVersions ) << TRACE_OUT( mftRecord.m_attrStandard.dwVersionNum ) /*<< TRACE_OUT(mftRecord.m_fileOnDisk.) */ << TRACE_OUT( mftRecord.m_fragCnt ) << TRACE_OUT( mftRecord.m_nameCnt ) /* << TRACE_OUT(mftRecord.m_outFileData) */ << TRACE_OUT( mftRecord.m_streamCnt ) << TRACE_OUT( mftRecord.sMFTRecordTypeStr ) << TRACE_OUT( mftRecord.GetTypeCnts( ) ) << std::endl;
#endif
	// Take data(m_outFileData) is special since it is the data of entire MFT file
	m_copyOfMFT.swap( mftRecord.m_outFileData );

	// Take file's on disk layout.
	m_fileOnDisk.swap( mftRecord.m_fileOnDisk );
	m_dirMap.clear( );

	// Copy MFT type count info.
	memcpy( m_typeCnt, mftRecord.GetTypeCnts( ), sizeof( m_typeCnt ) );

	return ERROR_SUCCESS;
	}

#if 0
// ------------------------------------------------------------------------------------------------
/// this function if succeeded it will allocate the buffer and passed to the caller
//  the caller's responsibility to free it

int NtfsUtil::Read_File(DWORD nFileSeq, Buffer& fileData)
{
	if (!m_bInitialized)
		return ReturnError(ERROR_INVALID_ACCESS);
	
	// point the record of the file in the MFT table
	Block mftBlock(&m_copyOfMFT[nFileSeq*m_dwMFTRecordSz], m_dwMFTRecordSz);

	// Then extract that file from the drive
	MFTRecord mftRecord;
	mftRecord.SetDriveHandle(m_hDrive);
	mftRecord.SetRecordInfo((LONGLONG)m_dwStartSector*m_dwBytesPerSector, m_dwMFTRecordSz, m_dwBytesPerCluster);
	int nRet = mftRecord.ExtractFile(mftBlock, true);
	if (nRet)
		return nRet;

	// pass the file data, It should be deallocated by the caller
	fileData.swap(mftRecord.m_outFileData);

	return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
int NtfsUtil::GetFileDetail(DWORD nFileSeq, FileInfo& stFileInfo)
{
	int nRet;

	if (!m_bInitialized)
		return ReturnError(ERROR_INVALID_ACCESS);

	if ((nFileSeq*m_dwMFTRecordSz+m_dwMFTRecordSz) >= m_copyOfMFT.size())
		return ReturnError(ERROR_NO_MORE_FILES);

	// point the record of the file in the MFT table
	Block mftBlock(&m_copyOfMFT[nFileSeq * m_dwMFTRecordSz], m_dwMFTRecordSz);

	// read the only file detail not the file data
	MFTRecord mftRecord;
	mftRecord.SetDriveHandle(m_hDrive);
	mftRecord.SetRecordInfo((LONGLONG)m_dwStartSector * m_dwBytesPerSector, m_dwMFTRecordSz, m_dwBytesPerCluster);
	nRet = mftRecord.ExtractFile(mftBlock, false);
	if (nRet)
		return nRet;

	// set the struct and pass the struct of file detail
	stFileInfo.filename = std::wstring(mftRecord.m_attrFilename.wFilename, mftRecord.m_attrFilename.chFileNameLength);
	
	stFileInfo.dwAttributes = mftRecord.m_attrFilename.dwFlags;

	stFileInfo.n64Create = mftRecord.m_attrStandard.n64Create;
	stFileInfo.n64Modify = mftRecord.m_attrStandard.n64Modify;
	stFileInfo.n64Access = mftRecord.m_attrStandard.n64Access;
	stFileInfo.n64Modfil = mftRecord.m_attrStandard.n64Modfil;

	stFileInfo.n64Size	 = mftRecord.m_attrFilename.n64Allocated;
	stFileInfo.n64Size	/= m_dwBytesPerCluster;
	stFileInfo.n64Size	 = (!stFileInfo.n64Size)?1:stFileInfo.n64Size;
	
	stFileInfo.bDeleted = !mftRecord.m_bInUse;
	stFileInfo.bSparse  = mftRecord.m_bSparse;

	return ERROR_SUCCESS;
}
#endif

// ------------------------------------------------------------------------------------------------
int NtfsUtil::GetSelectedFile( DWORD nFileSeq, const SharePtr<MultiFilter>& /* filter */, FileInfo& stFileInfo, bool getDir, StreamFilter* pStreamFilter ) {
#ifdef TRACING
	std::wcout << std::endl << "\tGetSelectedFile: " << TRACE_OUT(nFileSeq) << TRACE_OUT(getDir) << std::endl;
#endif

	int nRet;

	if (!m_bInitialized)
		return ReturnError(ERROR_INVALID_ACCESS);

	if ((nFileSeq * m_dwMFTRecordSz + m_dwMFTRecordSz) > m_copyOfMFT.size())
		return ERROR_NO_MORE_FILES;

	// Set mftBlock to point to the next mft record.
	Block mftBlock(&m_copyOfMFT[nFileSeq * m_dwMFTRecordSz], m_dwMFTRecordSz);

	// read the only file detail not the file data
	MFTRecord mftRecord;
	mftRecord.SetDriveHandle(m_hDrive);
	mftRecord.SetRecordInfo((LONGLONG)m_dwStartSector * m_dwBytesPerSector, m_dwMFTRecordSz, m_dwBytesPerCluster);
	nRet = mftRecord.ExtractStream(mftBlock, pStreamFilter);
	if ( nRet ) {

		return nRet;
		}
	// Store the file details in stFileInfo, extracting the info from the MFT.
	stFileInfo.filename = std::wstring(mftRecord.m_attrFilename.wFilename, mftRecord.m_attrFilename.chFileNameLength);
	
	stFileInfo.dwAttributes = mftRecord.m_attrFilename.dwFlags;

#if 1
	stFileInfo.n64Create = mftRecord.m_attrStandard.n64Create;
	stFileInfo.n64Modify = mftRecord.m_attrStandard.n64Modify;
	stFileInfo.n64Access = mftRecord.m_attrStandard.n64Access;
	stFileInfo.n64Modfil = mftRecord.m_attrStandard.n64Modfil;
#else
	// These dates are not accurate.
	stFileInfo.n64Create = mftRecord.m_attrFilename.n64Create;
	stFileInfo.n64Modify = mftRecord.m_attrFilename.n64Modify;
	stFileInfo.n64Access = mftRecord.m_attrFilename.n64Access;
	stFileInfo.n64Modfil = mftRecord.m_attrFilename.n64Modfil;
#endif
	stFileInfo.n64Size	 = mftRecord.m_attrFilename.n64RealSize & sMaxFileSize;
	stFileInfo.bDeleted  = !mftRecord.m_bInUse;
	stFileInfo.bSparse   = mftRecord.m_bSparse;
	stFileInfo.parentSeq = (DWORD)mftRecord.m_attrFilename.dwMftParentDir;

	stFileInfo.nameCnt   = mftRecord.m_nameCnt;
	stFileInfo.streamCnt = mftRecord.m_streamCnt;
	stFileInfo.m_fileOnDisk.swap(mftRecord.m_fileOnDisk);

	if (getDir && mftRecord.m_attrFilename.dwMftParentDir != 0)
	{
		GetDirectory(stFileInfo.directory, mftRecord.m_attrFilename.dwMftParentDir & sParentMask);
	}
	else
	{
		stFileInfo.directory.clear();
	}

	return ERROR_SUCCESS;
	}

// ------------------------------------------------------------------------------------------------
int NtfsUtil::GetDirectory( std::wstring& directory, LONGLONG mftIndex ) {
#ifdef TRACING
	std::wcout << std::endl << "\tGetDirectory: " << TRACE_OUT(&directory) << TRACE_OUT(mftIndex) << std::endl;
#endif

	DirMap::const_iterator dirIter = m_dirMap.find( mftIndex );
	if ( dirIter != m_dirMap.end( ) ) {
		directory = dirIter->second;
		return ERROR_SUCCESS;
		}

	LONGLONG n64LCN, n64Len = m_dwMFTRecordSz;
	if ( !GetDiskPosition( mftIndex * m_dwMFTRecordSz / m_dwBytesPerCluster, n64LCN, n64Len ) ) {
		return ReturnError( ERROR_INVALID_BLOCK );
		}
	unsigned MFTperCluster = m_dwBytesPerCluster / m_dwMFTRecordSz;
	unsigned bufferIdx = mftIndex % MFTperCluster;

	MFTRecord mftRecord;
	mftRecord.SetDriveHandle( m_hDrive );
	mftRecord.SetRecordInfo( ( LONGLONG ) m_dwStartSector * m_dwBytesPerSector, m_dwMFTRecordSz, m_dwBytesPerCluster );

	Buffer buffer;
	mftRecord.ReadRaw( n64LCN, buffer, m_dwBytesPerCluster );
	Buffer fileBuf = buffer.Region( bufferIdx * m_dwMFTRecordSz, m_dwMFTRecordSz );
	
	int nRet = mftRecord.ExtractFile( fileBuf, false );
	if ( nRet ) {
		return nRet;
		}
	LONGLONG parentIdx = mftRecord.m_attrFilename.dwMftParentDir & sParentMask;
	if ( parentIdx != mftIndex ) {
		nRet = GetDirectory( directory, parentIdx );
		directory += m_slash;
		directory += std::wstring( mftRecord.m_attrFilename.wFilename, mftRecord.m_attrFilename.chFileNameLength );
		}
	else {
		directory.clear( );
		}
	m_dirMap[ mftIndex ] = directory;
	return ERROR_SUCCESS;
	}

// ------------------------------------------------------------------------------------------------
int NtfsUtil::GetDiskPosition(LONGLONG findLCN, LONGLONG& outLCN, LONGLONG& inOutLen)  
{
#ifdef TRACING
	std::wcout << std::endl << "\tGetDiskPosition: " << TRACE_OUT(findLCN) << TRACE_OUT(outLCN) << TRACE_OUT(inOutLen) << std::endl;
#endif

	LONGLONG inLCNLen = inOutLen / m_dwBytesPerCluster;
	LONGLONG offLCN = 0;
	for ( unsigned idx = 0; idx != m_fileOnDisk.size( ); idx++ ) {
		if ( findLCN >= offLCN && findLCN + inLCNLen <= offLCN + m_fileOnDisk[ idx ].second / m_dwBytesPerCluster ) {
			outLCN = m_fileOnDisk[idx].first + (findLCN - offLCN);
			return true;
			}
		offLCN += m_fileOnDisk[idx].second / m_dwBytesPerCluster;
		}

	return false;
}



// ------------------------------------------------------------------------------------------------
// Custom filter to count NTFS inUse or deleted/free information.
// ------------------------------------------------------------------------------------------------

bool CountFilter::IsMatch( const MFT_STANDARD& /* attr */, const MFT_FILEINFO& name, const void* pData ) const {
#ifdef TRACING
	std::wcout << std::endl << "\tIsMatch: " << TRACE_OUT(name.wFilename) << std::endl;
#endif

	const MFTRecord* pMFTRecord = (const MFTRecord*)pData;
	bool inUse = pMFTRecord->m_bInUse;
	if ( inUse ) {
		m_activeInfo.Count( name );
		}
	else {
		m_deletedInfo.Count( name );
		}
#ifdef DUMP_DETAIL_MFT
	return true;    // keep all files.
#endif

	// Only keep system MFT hidden files.
	if ( inUse && name.chFileNameLength > 0 && name.wFilename[ 0 ] == '$' && ( name.dwFlags & eSystem ) != 0 && name.n64RealSize != 0 && ( name.dwMftParentDir & sParentMask ) < 16 ) {
		return true;
		}
	return false;   // don't need MFT file record anymore
	}

// ------------------------------------------------------------------------------------------------

void CountFilter::CountInfo::Count( const MFT_FILEINFO& name ) {
#ifdef TRACING
	std::wcout << std::endl << "\tCountInfo: " << TRACE_OUT( name.wFilename ) << std::endl;
#endif

	bool isDir = ( name.dwFlags & eDirectory ) == eDirectory;
	unsigned attrIdx = ( name.dwFlags & 7 );  // 1=Ronly, 2=hidden, 4=System
	   
	m_attrCnt[ attrIdx ]++;
	m_nameTypeCnt[ name.chFileNameType & 3 ]++;

	if ( isDir ) {
		m_dirCnt++;
		}
	else {
		m_fileCnt++;

		m_realSize += name.n64RealSize & sMaxFileSize;
		m_allocSize += name.n64Allocated & sMaxFileSize;
		}
	std::wcout << TRACE_OUT( m_fileCnt ) << TRACE_OUT( m_dirCnt ) << TRACE_OUT( m_realSize ) << TRACE_OUT( m_allocSize ) << std::endl;
	}


// ------------------------------------------------------------------------------------------------
// Custom Match filter to test 'count'.
// ------------------------------------------------------------------------------------------------

bool IsCntGreater( size_t inSize, size_t matchSize ) {
	return (inSize > matchSize);
	}

bool IsCntEqual( size_t inSize, size_t matchSize ) {
	return (inSize == matchSize);
	}

bool IsCntLess( size_t inSize, size_t matchSize ) {
	return (inSize < matchSize);
	}
