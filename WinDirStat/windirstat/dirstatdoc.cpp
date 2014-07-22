// dirstatdoc.cpp: Implementation of CDirstatDoc
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"
#include "windirstat.h"
#include "item.h"
#include "mainframe.h"
#include "osspecific.h"
#include "deletewarningdlg.h"
#include "modalshellapi.h"
#include ".\dirstatdoc.h"
#include "dirstatview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	const COLORREF _cushionColors[] = {
		RGB(0, 0, 255),
		RGB(255, 0, 0),
		RGB(0, 255, 0),
		RGB(0, 255, 255),
		RGB(255, 0, 255),
		RGB(255, 255, 0),
		RGB(150, 150, 255),
		RGB(255, 150, 150),
		RGB(150, 255, 150),
		RGB(150, 255, 255),
		RGB(255, 150, 255),
		RGB(255, 255, 150),
		RGB(255, 255, 255)
	};
}

CDirstatDoc *_theDocument;

CDirstatDoc* GetDocument() {
	ASSERT( _theDocument != NULL );
	return _theDocument;
	}

IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)

CDirstatDoc::CDirstatDoc() {
	ASSERT(_theDocument == NULL);
	_theDocument               = this;
	m_rootItem                 = NULL;
	m_workingItem              = NULL;
	m_zoomItem                 = NULL;
	m_selectedItem             = NULL;
	m_rootItem                 = NULL;
	m_selectedItem             = NULL;
	m_zoomItem                 = NULL;
	m_workingItem              = NULL;
	m_searchStartTime.QuadPart = NULL;
	m_timerFrequency.QuadPart  = NULL;
	m_showFreeSpace            = CPersistence::GetShowFreeSpace( );
	m_showUnknown              = CPersistence::GetShowUnknown( );
	m_extensionDataValid       = false;
	m_timeTextWritten          = false;
	m_showMyComputer           = true;
	m_freeDiskSpace            = -1;
	m_searchTime               = -1;
	m_totalDiskSpace           = -1;
	}

CDirstatDoc::~CDirstatDoc( ) {
	/*
	  Pretty, isn't it?
	  Need to check if m_rootItem, m_zoomItem, m_selectedItem, are equal to any of the others, as they may have been at some point. Else, a memory leak, or (worse) an access violation.
	*/
	AfxCheckMemory( );
	if ( m_showFreeSpace != NULL ) {
		CPersistence::SetShowFreeSpace( m_showFreeSpace );
		}
	if ( m_showUnknown != NULL ) {
		CPersistence::SetShowUnknown( m_showUnknown );
		}
	if ( m_rootItem != NULL ) {
		delete m_rootItem;
		if ( m_rootItem    == m_zoomItem ) {
			m_zoomItem     = NULL;
			}
		if ( m_rootItem    == m_workingItem ) {
			m_workingItem  = NULL;
			}
		if ( m_rootItem    == m_selectedItem ) {
			m_selectedItem = NULL;
			}
		m_rootItem         = NULL;
		}
	if ( m_zoomItem != NULL ) {
		if ( m_zoomItem    ==  m_workingItem ) {
			m_workingItem  = NULL;
			}
		if ( m_zoomItem    == m_selectedItem ) {
			m_selectedItem = NULL;
			}
		delete m_zoomItem;
		m_zoomItem = NULL;
		}
	if ( m_workingItem != NULL ) {
		if ( m_workingItem == m_selectedItem ) {
			m_selectedItem = NULL;
			}
		delete m_workingItem;
		m_workingItem = NULL;
		}
	if ( m_selectedItem != NULL ) {
		delete m_selectedItem;
		m_selectedItem = NULL;
		}
	if ( _theDocument != NULL ) {
		_theDocument   = NULL;
		}
	AfxCheckMemory( );
	//CANNOT `delete _theDocument`, b/c infinite recursion
	}

void CDirstatDoc::clearZoomItem( ) {
	m_zoomItem = NULL;
	}

void CDirstatDoc::clearRootItem( ) {
	m_rootItem = NULL;
	}

void CDirstatDoc::clearSelection( ) {
	m_selectedItem = NULL;
	}

// Encodes a selection from the CSelectDrivesDlg into a string which can be routed as a pseudo document "path" through MFC and finally arrives in OnOpenDocument().
CString CDirstatDoc::EncodeSelection(_In_ const RADIO radio, _In_ const CString folder, _In_ const CStringArray& drives) {
	CString ret;
	TRACE( _T( "Encoding selection %s\r\n" ), folder );
	switch (radio)
	{
		case RADIO_ALLLOCALDRIVES:
		case RADIO_SOMEDRIVES:
			{
			for ( INT i = 0; i < drives.GetSize( ); i++ ) {
					if ( i > 0 ) {
						ret += CString( GetEncodingSeparator( ) );
						}
					ret     += drives[ i ];
					}
			}
			break;
		
		case RADIO_AFOLDER:
			ret.Format( _T( "%s" ), folder.GetString( ) );
			break;
	}
	TRACE( _T( "Selection encoded as '%s'\r\n" ), ret );
	return ret;
	}

void CDirstatDoc::experimentalFunc( ) {
	AfxCheckMemory( );
	HANDLE hVol = CreateFile( L"\\\\.\\C:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hVol == INVALID_HANDLE_VALUE ) {

		TRACE( _T( "CreateFile() failed\r\n" ) );
		DWORD numChar = 0;
		LPWSTR errStr = NULL;
		numChar = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError( ), 0, ( LPWSTR ) &errStr, 0, 0 );
		TRACE( _T( "Error message: `%s`\r\n" ), errStr );
		return;
		}
	auto mftData = zeroInitMFT_ENUM_DATA_V0( );
	auto UpdateSequenceNumberJournalData = zeroInitUSN_JOURNAL_DATA( );
	DWORD cb = 0;

	BOOL res = DeviceIoControl( hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &UpdateSequenceNumberJournalData, sizeof( UpdateSequenceNumberJournalData ), &cb, NULL );
	if ( res == 0 ) {
		return;
		}

	cb = 0;
	long long files = 0;
	long long dirs = 0;
	static_assert( sizeof( DWORDLONG ) == sizeof( long long ), "Format specifiers will fail!" );
	mftData.HighUsn = UpdateSequenceNumberJournalData.NextUsn;
	BYTE pData[ sizeof( DWORDLONG ) + 0x1000 ]; //2^28 = 0x10000000 (~268,435,456)

	while ( DeviceIoControl( hVol, FSCTL_ENUM_USN_DATA, &mftData, sizeof( mftData ), &pData, sizeof( pData ), &cb, NULL ) != FALSE ) {
		PUSN_RECORD pRecord = ( PUSN_RECORD ) &pData[ sizeof( USN ) ];
		//TRACE( _T( "found %s in USN journal, #: `%I64x`, parent #: `%I64x`\r\n" ), pRecord->FileName, pRecord->FileReferenceNumber, pRecord->ParentFileReferenceNumber );
		while ( ( PBYTE ) pRecord < ( pData + cb ) ) {
			if ( ( pRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) {
				++dirs;
				}
			else if ( ( pRecord->FileAttributes & ( FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL| FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_SYSTEM ) ) != 0 ) {
				++files;
				}
			relUSNInfo thisUSN;
			thisUSN.fileAttributes = pRecord->FileAttributes;
			thisUSN.fileName = pRecord->FileName;
			thisUSN.fileNameLength = pRecord->FileNameLength;
			thisUSN.fileNameOffset = pRecord->FileNameOffset;
			thisUSN.fileReferenceNumber = pRecord->FileReferenceNumber;
			thisUSN.fileUSN = pRecord->Usn;
			thisUSN.parentFileReferenceNumber = pRecord->ParentFileReferenceNumber;
			USNstructs.emplace_back( std::move( thisUSN ) );
			pRecord = ( PUSN_RECORD ) ( ( PBYTE ) pRecord + pRecord->RecordLength );
			}
		mftData.StartFileReferenceNumber = *( DWORDLONG* ) pData;
		}
	std::vector<DWORDLONG> referenceNumbers;
	std::vector<childrenOfUSN> childrenOfParents;
	for ( auto anItem : USNstructs ) {
		parentUSNs.emplace( anItem.fileReferenceNumber, std::move( anItem ) );
		}
#ifdef DEBUG
	for ( const auto& aChild : childrenOfParents ) {
		aChild.display( );
		}
#endif
	TRACE( _T( "Found %lld files, and %lld directories. Total: %lld\r\n" ), files, dirs, ( files + dirs ) );
	AfxCheckMemory( );
	}


std::int64_t CDirstatDoc::GetTotlDiskSpace( _In_ CString path ) {
	if ( ( m_freeDiskSpace == -1 ) || ( m_totalDiskSpace == -1 ) ) {
		MyGetDiskFreeSpace( path, m_totalDiskSpace, m_freeDiskSpace );
		}
	return m_totalDiskSpace;
	}

std::int64_t CDirstatDoc::GetFreeDiskSpace( _In_ CString path ) {
	if ( ( m_freeDiskSpace == -1 ) || ( m_totalDiskSpace == -1 ) ) {
		MyGetDiskFreeSpace( path, m_totalDiskSpace, m_freeDiskSpace );
		}
	return m_freeDiskSpace;
	}

void CDirstatDoc::experimentalSection( _In_ CStringArray& drives ) {
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------NTFS testing stuff
	TRACE( _T( "Entering experimental section! Number of drives: %i\r\n" ), ( INT ) ( drives.GetSize( ) ) );
	try {
		for ( auto k = 0; k < drives.GetSize( ); ++k ) {
			TCHAR volumeName[ MAX_PATH + 1 ] = { 0 };
			DWORD serialNumber = 0;
			TCHAR fileSystemName[ MAX_PATH + 1 ] = { 0 };
			DWORD maxComponentLen = 0;
			DWORD fileSystemFlags = 0;

			TRACE( _T( "Experimental section: drive #: %i\r\n" ), (INT)k );
			//Internally, GetVolumeInformation calls NtOpenFile, asking for access FILE_READ_ATTRIBUTES | SYNCHRONIZE, with an OBJECT_ATTRIBUTES struct. Then it calls NtDeviceIoControlFile with IOCTL_MOUNTDEV_QUERY_DEVICE_NAME. Then it calls RtlDosPathNameToRelativeNtPathName_U_WithStatus with ( "\\.\MountPointManager", 0x004ff784, NULL, 0x004ff7b8 ), before calling NtCreateFile with FILE_READ_ATTRIBUTES | SYNCHRONIZE, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT. Finally, it calls NtDeviceIoControlFile with IOCTL_MOUNTMGR_QUERY_POINTS.

			if ( GetVolumeInformation( L"C:\\", volumeName, MAX_PATH + 1, &serialNumber, &maxComponentLen, &fileSystemFlags, fileSystemName, MAX_PATH + 1 ) ) {
				TRACE( _T( "Volume name: `%s`, Serial #:`%lu`, FS name: `%s`, Max component length: `%lu`, FS flags: `0X%.08X` \r\n" ), volumeName, serialNumber, fileSystemName, maxComponentLen, fileSystemFlags );
				}
			else {
				TRACE( _T( "GetVolumeInformation failed!!!!!\r\n" ) );
				}

			check8Dot3NameCreationAndNotifyUser( );

			HANDLE hVol;
			auto UpdateSequenceNumber_JournalData = zeroInitUSN_JOURNAL_DATA( );

			DWORD dwBytes = 0;

			hVol = CreateFile( L"\\\\.\\C:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			
			if ( hVol == INVALID_HANDLE_VALUE ) {
				
				TRACE( _T( "CreateFile() failed\r\n" ) );
				DWORD numChar = 0;
				LPWSTR errStr = NULL;
				numChar = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError( ), 0, ( LPWSTR ) &errStr, 0, 0 );
				TRACE( _T( "Error message: `%s`\r\n" ), errStr );
				
				goto failed;
				}
			else {
				//MessageBox(NULL, TEXT("I'm alive!"), TEXT(""), MB_OK );
				TRACE( _T( "CreateFile succeeded!\r\n" ) );
				STORAGE_DEVICE_NUMBER driveNumber = zeroInitSTORAGE_DEVICE_NUMBER( );

				DWORD bytesReturned = NULL;

				if ( !DeviceIoControl( hVol, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &driveNumber, sizeof( driveNumber ), &bytesReturned, NULL ) ) {
					TRACE( _T( "IOCTL_STORAGE_GET_DEVICE_NUMBER failed!\r\n" ) );
					goto failed;
					}
				else {
					CString physDevNum;
					physDevNum.Format( _T( "\\\\.\\PhysicalDrive%lu" ), driveNumber.DeviceNumber );
					TRACE( _T( "Got device number/string `%s`\r\n" ), physDevNum );
					if ( !CreateFile( physDevNum, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) ) {
						TRACE( _T( "CreateFile( %s, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) failed!!!\r\n" ), physDevNum );
						goto failed;
						}
					else {

						}
					}

				if ( !DeviceIoControl( hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &UpdateSequenceNumber_JournalData, sizeof( UpdateSequenceNumber_JournalData ), &dwBytes, NULL ) ) {
					TRACE( _T( "DeviceIoControl() - Query journal failed\r\n" ) );
					DWORD numChar = 0;
					LPWSTR errStr = NULL;
					numChar = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError( ), 0, ( LPWSTR ) &errStr, 0, 0 );
					TRACE( _T( "Error message: `%s`\r\n" ), errStr );
					goto failed;
					}
				else {
					}
				}
			}
			return;
	failed://when you're doing low level programming, goto makes sense more often (which is still rarely).
		TRACE( _T( "Exiting Experimental section...\r\n" ) );
		displayWindowsMsgBoxWithError( );
		return;
		}//end try
		catch ( CException* anException ) {
			LPVOID lpMsgBuf;
			//LPVOID lpDisplayBuf;
			DWORD err = GetLastError( );
			TRACE( _T( "Error number: %llu\t\n" ), err );
			MessageBox(NULL, TEXT("Whoa! Error!"), (LPCWSTR)err, MB_OK );
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
			LPCTSTR msg = LPCTSTR( lpMsgBuf );
			MessageBox( NULL, LPCTSTR( lpMsgBuf ) , TEXT( "Error" ), MB_OK );
			TRACE( _T( "Error: %s\r\n" ), msg );
		}

	}

void addTokens( _In_ const CString& s, _Inout_ CStringArray& sa, _In_ INT& i, _In_ TCHAR EncodingSeparator ) {
	while ( i < s.GetLength( ) ) {
		CString token;
		while ( i < s.GetLength( ) && s[ i ] != EncodingSeparator ) {
			token += s[ i++ ];
			}
		
		token.TrimLeft( );
		token.TrimRight( );
		ASSERT( !token.IsEmpty( ) );
		sa.Add( token );

		if ( i < s.GetLength( ) ) {
			i++;
			}
		}
	}

// The inverse of EncodeSelection
void CDirstatDoc::DecodeSelection(_In_ const CString s, _Inout_ CString& folder, _Inout_ CStringArray& drives) {

	folder.Empty();
	drives.RemoveAll();

	// s is either something like "C:\programme" or something like "C:|D:|E:".

	CStringArray sa;
	INT i = 0;
	addTokens( s, sa, i, GetEncodingSeparator( ) );

	ASSERT( sa.GetSize( ) > 0 );
	if ( sa.GetSize( ) > 1 ) {
		for ( INT j = 0; j < sa.GetSize( ); j++ ) {
			CString d = sa[ j ];
			ASSERT( d.GetLength( ) == 2 );
			ASSERT( d[ 1 ] == _T( ':' ) );
			TRACE( _T( "Inserting drive: %s\r\n" ), ( d + _T( "\\" ) ) );
			drives.Add( d + _T( "\\" ) );
			}
		}
	else {
		CString f = sa[ 0 ];
		if ( f.GetLength( ) == 2 && f[ 1 ] == _T( ':' ) ) {
			TRACE( _T( "Inserting drive: %s\r\n" ), ( f + _T( "\\" ) ) );
			drives.Add( f + _T( "\\" ) );
			}
		else {
			// Remove trailing backslash, if any and not drive-root.
			if ( f.GetLength( ) > 0 && f.Right( 1 ) == _T( "\\" ) && ( f.GetLength( ) != 3 || f[ 1 ] != _T( ':' ) ) ) {
				f = f.Left( f.GetLength( ) - 1 );
				}
			TRACE( _T( "Whoops! %s is not a drive, it's a folder!\r\n" ), f );
			folder = f;
			}
		}
	}

TCHAR CDirstatDoc::GetEncodingSeparator() {
	return _T( '|' );// This character must be one, which is not allowed in file names.
	}

void CDirstatDoc::DeleteContents() {
	if ( m_rootItem != NULL ) {
		delete m_rootItem;
		if ( ( m_rootItem == m_zoomItem ) && ( m_rootItem == m_selectedItem ) ) {
			m_zoomItem = NULL;
			m_selectedItem = NULL;
			}
		if ( m_rootItem == m_zoomItem ) {
			m_zoomItem = NULL;
			}
		if ( m_rootItem == m_selectedItem ) {
			m_selectedItem = NULL;
			}
		m_rootItem   = NULL;
		}
	SetWorkingItem( NULL );
	if ( m_zoomItem != NULL ) {
		if ( m_zoomItem == m_selectedItem ) {
			m_selectedItem = NULL;
			}
		delete m_zoomItem;
		m_zoomItem   = NULL;
		}
	if ( m_selectedItem != NULL ) {
		delete m_selectedItem;
		m_selectedItem   = NULL;
		}
	GetApp( )->ReReadMountPoints( );
	AfxCheckMemory( );
	}

BOOL CDirstatDoc::OnNewDocument() {
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	UpdateAllViews(NULL, HINT_NEWROOT);
	return TRUE;
	}


void CDirstatDoc::buildDriveItems( _In_ CStringArray& rootFolders, _Inout_ std::vector<std::shared_ptr<CItem>>& smart_driveItems ) {
	if ( m_showMyComputer ) {
		m_rootItem = new CItem { ( ITEMTYPE ) ( IT_MYCOMPUTER | ITF_ROOTITEM ), LoadString( IDS_MYCOMPUTER ) };
		for ( INT i = 0; i < rootFolders.GetSize( ); i++ ) {
			CItem *drive = new CItem{ IT_DRIVE, rootFolders[ i ] };
			auto smart_drive = std::make_shared<CItem>( IT_DRIVE, rootFolders[ i ] );	
			smart_driveItems.emplace_back( std::move( smart_drive ) );
			m_rootItem->AddChild(drive);
			}
		}
	else {
		ITEMTYPE type = IsDrive( rootFolders[ 0 ] ) ? IT_DRIVE : IT_DIRECTORY;
		m_rootItem = new CItem { ( ITEMTYPE ) ( type | ITF_ROOTITEM ), rootFolders[ 0 ], false };
		if ( m_rootItem->GetType( ) == IT_DRIVE ) {
			smart_driveItems.emplace_back( std::make_shared<CItem>( ( ITEMTYPE ) ( type | ITF_ROOTITEM ), rootFolders[ 0 ], false ) );
			}
		m_rootItem->UpdateLastChange( );
		}
	}


void CDirstatDoc::buildRootFolders( _In_ CStringArray& drives, _In_ CString& folder, _Inout_ CStringArray& rootFolders ) {
	if ( drives.GetSize( ) > 0 ) {
		m_showMyComputer = ( drives.GetSize( ) > 1 );

		for ( INT i = 0; i < drives.GetSize( ); i++ ) {
			rootFolders.Add( drives[ i ] );
			}
		}
	else {
		ASSERT( !folder.IsEmpty( ) );
		m_showMyComputer = false;
		rootFolders.Add( folder );
		}
	}


void CDirstatDoc::CreateUnknownAndFreeSpaceItems( _Inout_ std::vector<std::shared_ptr<CItem>>& smart_driveItems ) {
	for ( auto& aDrive : smart_driveItems ) {
		if ( OptionShowFreeSpace( ) ) {
			aDrive->CreateFreeSpaceItem( );
			}
		if ( OptionShowUnknown( ) ) {
			aDrive->CreateUnknownItem( );
			}
		}
	}

BOOL CDirstatDoc::OnOpenDocument(_In_ LPCTSTR lpszPathName) {
	CDocument::OnNewDocument(); // --> DeleteContents()
	TRACE( _T( "Opening new document, path: %s\r\n" ), lpszPathName );
	CString spec = lpszPathName;
	CString folder;
	CStringArray drives;
	DecodeSelection(spec, folder, drives);
	//experimentalFunc( );
	//experimentalSection( drives );
	CStringArray rootFolders;
	buildRootFolders( drives, folder, rootFolders );
	std::vector<std::shared_ptr<CItem>> smart_driveItems;

	buildDriveItems( rootFolders, smart_driveItems );

	m_zoomItem = m_rootItem;

	CreateUnknownAndFreeSpaceItems( smart_driveItems );

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );
	BOOL behavedWell = QueryPerformanceCounter( &m_searchStartTime );
	if ( !behavedWell ) {
		std::wstring a;
		a += ( __FUNCTION__, __LINE__ );
		MessageBox(NULL, TEXT("QueryPerformanceCounter failed!!"), a.c_str( ), MB_OK );
		}
	behavedWell = QueryPerformanceFrequency( &m_timerFrequency );
	if ( !behavedWell ) {
		std::wstring a;
		a += ( __FUNCTION__, __LINE__ );
		MessageBox(NULL, TEXT("QueryPerformanceCounter failed!!"), a.c_str( ), MB_OK );
		}
	
	SetWorkingItem( m_rootItem );
	GetMainFrame( )->FirstUpdateProgress( );
	GetMainFrame( )->MinimizeGraphView( );
	GetMainFrame( )->MinimizeTypeView( );
	UpdateAllViews( NULL, HINT_NEWROOT );
	GetMainFrame( )->FirstUpdateProgress( );
	return true;
	}

void CDirstatDoc::SetPathName( _In_ LPCTSTR lpszPathName, BOOL /*bAddToMRU*/) {
	/*
	  We don't want MFCs AfxFullPath()-Logic, because lpszPathName is not a path. So we have overridden this.
	  MRU would be fine but is not implemented yet.
	*/
	m_strPathName = lpszPathName;
	ASSERT( !m_strPathName.IsEmpty( ) );       // must be set to something
	m_bEmbedded = FALSE;
	SetTitle( lpszPathName );
	ASSERT_VALID(this);
	}

void CDirstatDoc::Serialize(_In_ const CArchive& /*ar*/) { }

// Prefix the window title (with percentage or "Scanning")
void CDirstatDoc::SetTitlePrefix( const CString prefix ) {
	CString docName = prefix + GetTitle( );
	TRACE( _T( "Setting window title to '%s'\r\n" ), docName );
	GetMainFrame( )->UpdateFrameTitleForDocument( docName );
	}


COLORREF CDirstatDoc::GetCushionColor( _In_ LPCTSTR ext ) {
	CString ext_CS;
	std::vector<SExtensionRecord>::size_type position = m_extensionRecords.size();
	const std::vector<SExtensionRecord>::size_type vecSize = m_extensionRecords.size( );
	for ( size_t i = 0; i < vecSize; ++i ) {
		//if ( m_extensionRecords.at( i ).ext == ext ) {
		if ( m_extensionRecords[ i ].ext == ext ) {
			position = i;
			}
		}

	return ( m_extensionRecords.at( position ).color );
	}

COLORREF CDirstatDoc::GetZoomColor() const {
	return RGB(0,0,255);
	}

bool CDirstatDoc::OptionShowFreeSpace() const {
	return m_showFreeSpace;
	}

bool CDirstatDoc::OptionShowUnknown() const {
	return m_showUnknown;
	}

_Must_inspect_result_ std::vector<SExtensionRecord>* CDirstatDoc::GetExtensionRecords( ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
	return &m_extensionRecords;
	}

LONGLONG CDirstatDoc::GetRootSize() const {
	ASSERT( m_rootItem != NULL );
	ASSERT( IsRootDone( ) );
	return m_rootItem->GetSize( );
	}

void CDirstatDoc::ForgetItemTree( ) {
	AfxCheckMemory( );
	// The program is closing. As "delete m_rootItem" can last a long time (many minutes), if we have been paged out, we simply forget our item tree here and hope that the system will free all our memory anyway.
	//`delete m_rootItem` seems fine to me!
	if ( m_rootItem != NULL ) {
		//if ( ( m_rootItem == m_zoomItem )|| ( m_rootItem == m_selectedItem ) ) {
		if ( ( m_rootItem == m_zoomItem ) && ( m_rootItem == m_selectedItem ) ) {
			m_zoomItem = NULL;
			m_selectedItem = NULL;
			}
		if ( m_rootItem == m_zoomItem ) {
			m_zoomItem = NULL;
			}
		if ( m_rootItem == m_selectedItem ) {
			m_selectedItem = NULL;
			}
			delete m_rootItem;
			m_rootItem = NULL;
		}
	if ( m_zoomItem != NULL ) {
		if ( m_zoomItem == m_selectedItem ) {
			m_selectedItem = NULL;
			}
		delete m_zoomItem;
		m_zoomItem = NULL;
		}
	if ( m_selectedItem != NULL ) {
		delete m_selectedItem;
		m_selectedItem = NULL;
		}
	AfxCheckMemory( );
	}

bool CDirstatDoc::Work( _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t ticks ) {
	/*
	  This method does some work for ticks ms. 
	  return: true if done or suspended.
	*/
	if ( m_rootItem == NULL ) {
		/*
		  Bail out!
		*/
		TRACE( _T( "There's no work to do! (m_rootItem == NULL) - What the hell?\r\n" ) );
		//ASSERT( false );//valid condition - can occur if user clicks cancel in drive select box on first opening.
		return true;
		}

	if ( GetMainFrame( )->IsProgressSuspended( ) ) {
		return true;
		}

	if ( !m_rootItem->IsDone( ) ) {
		m_rootItem->DoSomeWork( ticks );
		if ( m_rootItem->IsDone( ) ) {
			TRACE( _T( "Finished walking tree...\r\n" ) );
			m_extensionDataValid = false;

			GetMainFrame( )->SetProgressPos100( );
			GetMainFrame( )->RestoreTypeView( );

			LARGE_INTEGER doneTime;
			BOOL behavedWell = QueryPerformanceCounter( &doneTime );
			if ( !behavedWell ) {
				doneTime.QuadPart = NULL;
				}
			const DOUBLE AdjustedTimerFrequency = ( DOUBLE( 1 ) ) / DOUBLE( m_timerFrequency.QuadPart );
			
			UpdateAllViews( NULL );//nothing has been done?
			if ( doneTime.QuadPart != NULL ) {
				m_searchTime = ( doneTime.QuadPart - m_searchStartTime.QuadPart ) * AdjustedTimerFrequency;
				}
			else {
				//m_searchTime = -FLT_MAX;
				m_searchTime = -2;//Negative (that's not -1) informs WriteTimeToStatusBar that there was a problem.
				}
			GetMainFrame( )->RestoreGraphView( );
			m_rootItem->SortChildren( );
			//Complete?
			auto DirStatView = ( GetMainFrame( )->GetDirstatView( ) );
			if ( DirStatView != NULL ) {
				DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
				}

			m_timeTextWritten = true;
			#ifdef DUMP_MEMUSAGE
			_CrtMemDumpAllObjectsSince( NULL );
			#endif

			}
		else {
			ASSERT( m_workingItem != NULL );
			if ( m_workingItem != NULL ) { // to be honest, "defensive programming" is stupid, but c'est la vie: it's safer. //<== Whoever wrote this is wrong about the stupidity of defensive programming
				GetMainFrame( )->SetProgressPos( m_workingItem->GetProgressPos( ) );
				}
			m_rootItem->SortChildren( );
			UpdateAllViews(NULL, HINT_SOMEWORKDONE);
			}

		}
	if ( m_rootItem->IsDone( ) && m_timeTextWritten ) {
		SetWorkingItem( NULL, true );
		m_rootItem->SortChildren( );
		return true;
		}
	else {
		return false;
		}
	}

bool CDirstatDoc::IsDrive(_In_ const CString spec) const {
	return ( spec.GetLength( ) == 3 && spec[ 1 ] == _T( ':' ) && spec[ 2 ] == _T( '\\' ) );
	}

void CDirstatDoc::RefreshMountPointItems() {
	/*
	  Starts a refresh of all mount points in our tree.
	  Called when the user changes the follow mount points option.
	*/
	CWaitCursor wc;
	CItem *root = GetRootItem( );
	if ( root == NULL ) {
		return;
		}
	RecurseRefreshMountPointItems( root );
	}

void CDirstatDoc::RefreshJunctionItems() {
	/*
	  Starts a refresh of all junction points in our tree.
	  Called when the user changes the ignore junction points option.
	*/
	CWaitCursor wc;//?
	CItem *root =  GetRootItem();
	if ( root == NULL ) {
		return;
		}
	RecurseRefreshJunctionItems(root);
	}

bool CDirstatDoc::IsRootDone()    const {
	return ( ( m_rootItem != NULL ) && m_rootItem->IsDone( ) );
	}

_Must_inspect_result_ CItem *CDirstatDoc::GetRootItem() const {
	return m_rootItem;
	}

_Must_inspect_result_ CItem *CDirstatDoc::GetZoomItem() const {
	return m_zoomItem;
	}

bool CDirstatDoc::IsZoomed() const {
	return GetZoomItem() != GetRootItem();
	}

void CDirstatDoc::SetSelection(_In_ const CItem *item, _In_ const bool keepReselectChildStack) {
	if ( item == NULL ) {
		return;
		}
	if ( m_zoomItem == NULL ) {
		return;
		}
	CItem* newzoom = CItem::FindCommonAncestor( m_zoomItem, item );
	if ( newzoom != NULL ) {
		TRACE( _T( "Setting new selection\r\n" ) );
		if ( newzoom != m_zoomItem ) {
			SetZoomItem( newzoom );
			}
		}
	else {
		TRACE( _T( "Can't zoom to NULL!\r\n" ) );
		AfxCheckMemory( );
		ASSERT( false );
		}
	bool keep = ( keepReselectChildStack || ( m_selectedItem == item ) );

	m_selectedItem = const_cast< CItem * >( item );
	GetMainFrame( )->SetSelectionMessageText( );

	if ( !keep ) {
		ClearReselectChildStack( );
		}
	}

_Must_inspect_result_ CItem *CDirstatDoc::GetSelection() const {
	return m_selectedItem;
	}

void CDirstatDoc::SetHighlightExtension(_In_ const LPCTSTR ext) {
	m_highlightExtension = ext;
	TRACE( _T( "Highlighting extension %s\r\n" ), m_highlightExtension );
	GetMainFrame( )->SetSelectionMessageText( );
	}

CString CDirstatDoc::GetHighlightExtension() const {
	return m_highlightExtension;
	}

void CDirstatDoc::UnlinkRoot() {
	/*
	  The very root has been deleted.
	*/
	TRACE( _T( "The very root has been deleted!\r\n" ) );
	DeleteContents( );
	UpdateAllViews( NULL, HINT_NEWROOT );
	}


LONGLONG CDirstatDoc::GetWorkingItemReadJobs() const {
	if ( m_workingItem != NULL ) {
		return m_workingItem->GetReadJobs( );
		}
	else {
		return 0;//hmmm
		}
	}

void CDirstatDoc::OpenItem(_In_ const CItem *item) {
	ASSERT( item != NULL );
	CWaitCursor wc;
	try
	{
		CString path;
		switch ( item->GetType( ) )
		{
		case IT_MYCOMPUTER:
			{
				auto sei = zeroInitSEI( );
				CCoTaskMem<LPITEMIDLIST> pidl;
			
				GetPidlOfMyComputer( &pidl );
				sei.lpIDList = pidl;
				sei.fMask   |= SEE_MASK_IDLIST;
				ShellExecuteEx( &sei );
				// ShellExecuteEx seems to display its own Messagebox, if failed.
				return;
			}
			break;
		case IT_DRIVE:
		case IT_DIRECTORY:
			path = item->GetFolderPath( );
			break;
		case IT_FILE:
			path = item->GetPath( );
			break;
		default:
			AfxCheckMemory( );
			ASSERT( false );
		}
		ShellExecuteWithAssocDialog( *AfxGetMainWnd( ), path );
	}
	catch (CException *pe)
	{
		pe->ReportError( );
		pe->Delete( );
	}
	AfxCheckMemory( );
	}

void CDirstatDoc::RecurseRefreshMountPointItems(_In_ CItem* item) {
	if ( ( item->GetType( ) == IT_DIRECTORY ) && ( item != GetRootItem( ) ) && GetApp( )->IsMountPoint( item->GetPath( ) ) ) {
		RefreshItem( item );
		}
	for ( auto i = 0; i < item->GetChildrenCount( ); i++ ) {
		RecurseRefreshMountPointItems( item->GetChild( i ) );//ranged for?
		}
	}

void CDirstatDoc::RecurseRefreshJunctionItems(_In_ CItem* item) {
	if ( ( item->GetType( ) == IT_DIRECTORY ) && ( item != GetRootItem( ) ) && GetApp( )->IsJunctionPoint( item->GetPath( ) ) ) {
		RefreshItem( item );
		}
	for ( auto i = 0; i < item->GetChildrenCount( ); i++ ) {
		RecurseRefreshJunctionItems( item->GetChild( i ) );
		}
	}

void CDirstatDoc::GetDriveItems(_Inout_ CArray<CItem *, CItem *>& drives) {
	/*
	  Gets all items of type IT_DRIVE. Adds them to the CArray that is passed in.
	*/
	drives.RemoveAll( );

	auto root = GetRootItem( );
	
	if ( root == NULL ) {
		AfxCheckMemory( );
		ASSERT( false );//sensible?
		return;
		}
	else if ( root->GetType( ) == IT_MYCOMPUTER ) {
		for ( auto i = 0; i < root->GetChildrenCount( ); i++ ) {
			auto drive = root->GetChild( i );
			if ( drive != NULL ) {
				if ( drive->GetType( ) == IT_DRIVE ) {
					drives.Add( drive );
					}
				else {
					AfxCheckMemory( );
					ASSERT( false );
					}
				}
			else {
				AfxCheckMemory( );
				ASSERT( false );
				}
			}
		}
	else if ( root->GetType( ) == IT_DRIVE ) {
		drives.Add( root );
		}
	AfxCheckMemory( );
	}

std::vector<CItem*> CDirstatDoc::modernGetDriveItems( ) {
	auto root = GetRootItem( );
	if ( root == NULL ) {
		std::vector<CItem*> nullVec;
		return std::move( nullVec );
		}
	std::vector<CItem*> drives;
	auto rootType = root->GetType( );
	if ( rootType == IT_MYCOMPUTER ) {
		for ( auto i = 0; i < root->GetChildrenCount( ); ++i ) {
			auto aChild = root->GetChild( i );
			if ( aChild != NULL ) {
				if ( aChild->GetType( ) == IT_DRIVE ) {
					drives.emplace_back( std::move( aChild ) );
					}
				else {
					AfxCheckMemory( );
					ASSERT( false );
					}
				}
			else {
				AfxCheckMemory( );
				ASSERT( false );
				}
			}
		}
	else if ( rootType == IT_DRIVE ) {
		drives.emplace_back( root );
		}
	
	return std::move( drives );
	}

void CDirstatDoc::RebuildExtensionData() {
	/*
	  Assigns colors to all known file types (i.e. `Extensions`)
	*/
	CWaitCursor wc;
	
	m_extensionRecords.clear( );
	m_extensionRecords.reserve( 100000 );
		
	m_rootItem->stdRecurseCollectExtensionData( m_extensionRecords );

	stdSetExtensionColors( m_extensionRecords );
	std::sort( m_extensionRecords.begin( ), m_extensionRecords.end( ), SExtensionRecord::compareSExtensionRecordByBytes );

	m_extensionRecords.shrink_to_fit( );
	m_extensionDataValid = true;
	}

void CDirstatDoc::stdSetExtensionColors( _Inout_ std::vector<SExtensionRecord>& extensionsToSet ) {
	/*
	  New, much faster, method of assigning colors to extensions. For every element in reverseExtensionMap, assigns a color to the `color` field of an element at key std::pair(LONGLONG, CString). The color assigned is chosen by rotating through a default palette.
	*/
	static const auto colorVector = CTreemap::GetDefaultPaletteAsVector( );
	std::vector<COLORREF>::size_type processed = 0;

	//int worked = 0;
	for ( auto& anExtension : extensionsToSet ) {
		COLORREF test = colorVector.at( processed % ( colorVector.size( ) ) );
		++processed;
		if ( processed < ( colorVector.size( ) ) ) {//TODO colors.GetSize( )-> colorsSize
			test = colorVector.at( processed );
			}
		anExtension.color = test;
		//TRACE( _T( "processed: %i, ( processed (mod) colorVector.size() ): %i, c: %lu, color @ [%s]: %lu\r\n" ), processed, ( processed % colorVector.size()), test, anExtension.ext, anExtension.color );
		}
	for ( const auto& a : extensionsToSet ) {
		static_assert( sizeof( LONGLONG ) == 8, "bad format specifiers!" );
		static_assert( sizeof( DWORD ) == sizeof( unsigned long ), "bad format specifiers!" );
		TRACE( _T( "%s: (Bytes: %I64x), (Color: %lu), (Files: %I32x)\r\n" ), a.ext, a.bytes, a.color, a.files );//TODO: bytes has bad format specifier!
		ASSERT( a.color != 0 );
		}
	}

CExtensionData *CDirstatDoc::_pqsortExtensionData;

INT __cdecl CDirstatDoc::_compareExtensions(_In_ const void *item1, _In_ const void *item2) {
	/*
	  The comparison function that was used for qsort in the old sort method.
	*/
	CString *ext1 = (CString *)item1;
	CString *ext2 = (CString *)item2;
	SExtensionRecord r1;
	SExtensionRecord r2;
	VERIFY( _pqsortExtensionData->Lookup( *ext1, r1 ) );
	VERIFY( _pqsortExtensionData->Lookup( *ext2, r2 ) );
	return signum( r2.bytes - r1.bytes );
	}

void CDirstatDoc::SetWorkingItemAncestor(_In_ CItem *item) {
	if ( m_workingItem != NULL ) {
		SetWorkingItem( CItem::FindCommonAncestor( m_workingItem, item ) );
		}
	else {
		SetWorkingItem( item );
		}
	}

void CDirstatDoc::SetWorkingItem( _In_opt_ CItem *item) {
	if ( GetMainFrame( ) != NULL ) {
		if ( item != NULL ) {
			GetMainFrame( )->ShowProgress( item->GetProgressRange( ) );
			}
		else {
			GetMainFrame( )->HideProgress( );
			//GetMainFrame( )->WriteTimeToStatusBar( );
			}
		}
	m_workingItem = item;
	}

void CDirstatDoc::SetWorkingItem(_In_opt_ CItem *item, _In_ bool hideTiming) {
	if ( GetMainFrame( ) != NULL ) {
		if ( item != NULL ) {
			GetMainFrame( )->ShowProgress( item->GetProgressRange( ) );
			}
		else if ( hideTiming ) {
			GetMainFrame( )->HideProgress( );
			//GetMainFrame( )->WriteTimeToStatusBar( );
			}
		}
	m_workingItem = item;
	}


bool CDirstatDoc::DeletePhysicalItem( _In_ CItem *item, _In_ const bool toTrashBin) {
	/*
	  Deletes a file or directory via SHFileOperation.
	  Return: false, if canceled
	*/
	if ( CPersistence::GetShowDeleteWarning( ) ) {
		CDeleteWarningDlg warning;
		warning.m_fileName = item->GetPath( );
		if ( IDYES != warning.DoModal( ) ) {
			return false;
			}
		CPersistence::SetShowDeleteWarning( !warning.m_dontShowAgain );
		}

	ASSERT( item->GetParent( ) != NULL );

	CModalShellApi msa;
	msa.DeleteFile( item->GetPath( ), toTrashBin );

	RefreshItem( item );
	AfxCheckMemory( );
	return true;
	}

void CDirstatDoc::SetZoomItem(_In_ CItem *item) {
	if ( item == NULL ) {
		return;
		}
	m_zoomItem = item;
	AfxCheckMemory( );
	UpdateAllViews( NULL, HINT_ZOOMCHANGED );
	}

void CDirstatDoc::RefreshItem(_In_ CItem *item) {
	/*
	  Starts a refresh of an item.
	  If the physical item has been deleted,
	  updates selection, zoom and working item accordingly.	
	*/
	ASSERT( item != NULL );
	CWaitCursor wc;
	ClearReselectChildStack( );

	if ( item->IsAncestorOf( GetZoomItem( ) ) ) {
		SetZoomItem( item );
		}
	if ( item->IsAncestorOf( GetSelection( ) ) ) {
		SetSelection( item );
		UpdateAllViews( NULL, HINT_SELECTIONCHANGED );
		}

	SetWorkingItemAncestor( item );
	auto parent = item->GetParent( );
	if ( parent != NULL ) {
		if ( !item->StartRefresh( ) ) {
			if ( GetZoomItem( ) == item ) {
				SetZoomItem( parent );
				}
			if ( GetSelection( ) == item ) {
				SetSelection( parent );
				UpdateAllViews( NULL, HINT_SELECTIONCHANGED );
				}
			if ( m_workingItem == item ) {
				SetWorkingItem( parent );
				}
			}
		}
	else {
		//ASSERT( false );//can happen when item is root of tree
		}
	UpdateAllViews( NULL );
	}

void CDirstatDoc::PushReselectChild( CItem *item ) {
	m_reselectChildStack.AddHead( item );
	}

_Must_inspect_result_ CItem *CDirstatDoc::PopReselectChild( ) {
	return m_reselectChildStack.RemoveHead( );
	}

void CDirstatDoc::ClearReselectChildStack( ) {
	m_reselectChildStack.RemoveAll( );
	}

bool CDirstatDoc::IsReselectChildAvailable( ) const {
	return !m_reselectChildStack.IsEmpty( );
	}

bool CDirstatDoc::DirectoryListHasFocus( ) const {
	return ( GetMainFrame( )->GetLogicalFocus( ) == LF_DIRECTORYLIST );
	}

BEGIN_MESSAGE_MAP(CDirstatDoc, CDocument)
	ON_COMMAND(ID_REFRESHSELECTED, OnRefreshselected)
	ON_UPDATE_COMMAND_UI(ID_REFRESHSELECTED, OnUpdateRefreshselected)
	ON_COMMAND(ID_REFRESHALL, OnRefreshall)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWFREESPACE, OnUpdateViewShowfreespace)
	ON_COMMAND(ID_VIEW_SHOWFREESPACE, OnViewShowfreespace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWUNKNOWN, OnUpdateViewShowunknown)
	ON_COMMAND(ID_VIEW_SHOWUNKNOWN, OnViewShowunknown)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_SELECTPARENT, OnUpdateTreemapSelectparent)
	ON_COMMAND(ID_TREEMAP_SELECTPARENT, OnTreemapSelectparent)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMIN, OnUpdateTreemapZoomin)
	ON_COMMAND(ID_TREEMAP_ZOOMIN, OnTreemapZoomin)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMOUT, OnUpdateTreemapZoomout)
	ON_COMMAND(ID_TREEMAP_ZOOMOUT, OnTreemapZoomout)
	ON_UPDATE_COMMAND_UI(ID_REFRESHALL, OnUpdateRefreshall)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_RESELECTCHILD, OnUpdateTreemapReselectchild)
	ON_COMMAND(ID_TREEMAP_RESELECTCHILD, OnTreemapReselectchild)
END_MESSAGE_MAP()


void CDirstatDoc::OnUpdateRefreshselected( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( DirectoryListHasFocus( ) && GetSelection( ) != NULL && GetSelection( )->GetType( ) != IT_FREESPACE && GetSelection( )->GetType( ) != IT_UNKNOWN );
	}

void CDirstatDoc::OnRefreshselected( ) {
	RefreshItem( GetSelection( ) );
	}

void CDirstatDoc::OnUpdateRefreshall( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( GetRootItem( ) != NULL );
	}

void CDirstatDoc::OnRefreshall( ) {
	RefreshItem( GetRootItem( ) );
	}

void CDirstatDoc::OnUpdateEditCopy( CCmdUI *pCmdUI ) {
	const CItem *item = GetSelection( );
	if ( item == NULL ) {
		TRACE( _T( "Whoops! That's a NULL item!\r\n" ) );
		return;
		}
	auto thisItemType = item->GetType( );
	pCmdUI->Enable( DirectoryListHasFocus( ) && item != NULL && thisItemType != IT_MYCOMPUTER && thisItemType != IT_FILESFOLDER && thisItemType != IT_FREESPACE && thisItemType != IT_UNKNOWN );
	}

void CDirstatDoc::OnEditCopy() {
	TRACE( _T( "User chose 'Edit'->'Copy'!\r\n") );
	const CItem *item = GetSelection( );
	if ( item == NULL ) {
		TRACE( _T( "You tried to copy nothing! What does that even mean?\r\n" ) );
		return;
		}
	ASSERT( item->GetType( ) == IT_DRIVE || item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE );

	GetMainFrame( )->CopyToClipboard( item->GetPath( ) );
	}


void CDirstatDoc::OnUpdateViewShowfreespace( CCmdUI *pCmdUI ) {
	pCmdUI->SetCheck( m_showFreeSpace );
	}

void CDirstatDoc::OnViewShowfreespace( ) {
	auto drives = modernGetDriveItems( );
	if ( m_showFreeSpace ) {
		for ( const auto& aDrive : drives ) {
			auto freeSpaceItem = aDrive->FindFreeSpaceItem( );
			if ( freeSpaceItem == NULL ) { }
			else {
				if ( GetSelection( ) == freeSpaceItem ) {
					SetSelection( aDrive->GetParent( ) );
					}
				if ( GetZoomItem( ) == freeSpaceItem ) {
					auto Parent = freeSpaceItem->GetParent( );
					if ( Parent != NULL ) {
						m_zoomItem = Parent;
						}
					}
				aDrive->RemoveFreeSpaceItem( );
				}
			}
		m_showFreeSpace = false;
		}
	else {
		for ( auto& aDrive : drives ) {
			aDrive->CreateFreeSpaceItem( );
			}
		m_showFreeSpace = true;
		}
	if ( drives.size( ) > 0 ) {
		SetWorkingItem( GetRootItem( ) );
		}
	UpdateAllViews( NULL );
	}

void CDirstatDoc::OnUpdateViewShowunknown(CCmdUI *pCmdUI) {
	pCmdUI->SetCheck( m_showUnknown );
	}

void CDirstatDoc::OnViewShowunknown() {
	auto drives = modernGetDriveItems( );
	if ( m_showUnknown ) {
		for ( auto& aDrive : drives) {
			auto unknownItem = aDrive->FindUnknownItem( );
			if ( unknownItem == NULL ) { }
			else {
				if ( GetSelection( ) == unknownItem ) {
					SetSelection( unknownItem->GetParent( ) );
					}
				if ( GetZoomItem( ) == unknownItem ) {
					auto Parent = unknownItem->GetParent( );
					if ( Parent != NULL ) {
						m_zoomItem = unknownItem->GetParent( );
						}
					}
				aDrive->RemoveUnknownItem( );
				}
			}
		m_showUnknown = false;
		}
	else {
		for ( auto& aDrive : drives ) {
			aDrive->CreateUnknownItem( );
			}
		m_showUnknown = true;
		}
	if ( drives.size( ) > 0 ) {
		SetWorkingItem( GetRootItem( ) );
		}
	UpdateAllViews( NULL );
	}

void CDirstatDoc::OnUpdateTreemapZoomin( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( ( m_rootItem != NULL ) && ( m_rootItem->IsDone( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( ) != GetZoomItem( ) ) );
	}

void CDirstatDoc::OnTreemapZoomin( ) {
	AfxCheckMemory( );
	CItem* p = GetSelection( );
	CItem* z = NULL;
	auto zoomItem = GetZoomItem( );
	while ( p != zoomItem ) {
		z = p;
		p = p->GetParent( );
		}
	if ( z == NULL ) {
		return;
		}
	else {
		ASSERT( z != NULL );
		SetZoomItem( z );
		}
	}

void CDirstatDoc::OnUpdateTreemapZoomout( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( ( m_rootItem != NULL ) && ( m_rootItem->IsDone( ) ) && ( GetZoomItem( ) != m_rootItem ) );
	}

void CDirstatDoc::OnTreemapZoomout( ) {
	AfxCheckMemory( );
	auto ZoomItem = GetZoomItem();
	if ( ZoomItem != NULL ) {
		auto parent = ZoomItem->GetParent( );
		if ( parent != NULL ) {
			SetZoomItem( parent );
			}
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CDirstatDoc::OnUpdateExplorerHere( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( )->GetType( ) != IT_FREESPACE ) && ( GetSelection( )->GetType( ) != IT_UNKNOWN ) );
	}

void CDirstatDoc::OnExplorerHere( ) {
	AfxCheckMemory( );
	try
	{
		
		const CItem *item = GetSelection( );
		if ( item != NULL ) {
			TRACE( _T( "User wants to open Explorer in %s!\r\n" ), item->GetFolderPath( ) );
			if ( item->GetType( ) == IT_MYCOMPUTER ) {
				auto sei = zeroInitSEI( );
				sei.cbSize = sizeof( sei );
				sei.hwnd = *AfxGetMainWnd( );
				sei.lpVerb = _T( "explore" );
				sei.nShow = SW_SHOWNORMAL;

				CCoTaskMem<LPITEMIDLIST> pidl;
				GetPidlOfMyComputer( &pidl );

				sei.lpIDList = pidl;
				sei.fMask |= SEE_MASK_IDLIST;

				ShellExecuteEx( &sei );
				// ShellExecuteEx seems to display its own Messagebox on error.
				}
			else {
				MyShellExecute( *AfxGetMainWnd( ), _T( "explore" ), item->GetFolderPath( ), NULL, NULL, SW_SHOWNORMAL );
				}
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			}
	}
	catch ( CException *pe )
	{
		pe->ReportError( );
		pe->Delete( );
	}
	}

void CDirstatDoc::OnUpdateCommandPromptHere( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( )->GetType( ) != IT_MYCOMPUTER ) && ( GetSelection( )->GetType( ) != IT_FREESPACE ) && ( GetSelection( )->GetType( ) != IT_UNKNOWN ) && ( !( GetSelection( )->HasUncPath( ) ) ) );
	}

void CDirstatDoc::OnCommandPromptHere( ) {
	try
	{
		auto item = GetSelection( );
		if ( item != NULL ) {
			TRACE( _T( "User wants to open a command prompt in %s!\r\n" ), item->GetFolderPath( ) );
			CString cmd = GetCOMSPEC( );

			MyShellExecute( *AfxGetMainWnd( ), _T( "open" ), cmd, NULL, item->GetFolderPath( ), SW_SHOWNORMAL );
			}
	}
	catch ( CException *pe )
	{
		pe->ReportError( );
		pe->Delete( );
	}
	}

void CDirstatDoc::OnUpdateCleanupDeletetotrashbin( CCmdUI *pCmdUI ) {
	auto item = GetSelection( );
	if ( item != NULL ) {
		pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE ) && ( !( item->IsRootItem( ) ) ) );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CDirstatDoc::OnCleanupDeletetotrashbin( ) {
	auto item = GetSelection( );
	
	if ( item == NULL || item->GetType( ) != IT_DIRECTORY && item->GetType( ) != IT_FILE || item->IsRootItem( ) ) {
		return;
		}

	if ( DeletePhysicalItem( item, true ) ) {
		UpdateAllViews( NULL );
		}
	}

void CDirstatDoc::OnUpdateCleanupDelete( CCmdUI *pCmdUI ) {
	auto item = GetSelection( );
	if ( item != NULL ) {
		pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE ) && ( !( item->IsRootItem( ) ) ) );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CDirstatDoc::OnCleanupDelete( ) {
	auto item = GetSelection( );
	if ( item == NULL ) {
		AfxCheckMemory( );
		ASSERT( false );
		return;//MUST check here, not with GetType check - else we cannot count on NOT dereferencing item
		}
	if ( ( item->GetType( ) != IT_DIRECTORY && item->GetType( ) != IT_FILE ) || ( item->IsRootItem( ) ) ) {
		return;
		}

	if ( DeletePhysicalItem( item, false ) ) {
		SetWorkingItem( GetRootItem( ) );
		UpdateAllViews( NULL );
		}
	}
void CDirstatDoc::OnUpdateTreemapSelectparent( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( ( GetSelection( ) != NULL ) && ( GetSelection( )->GetParent( ) != NULL ) );
}

void CDirstatDoc::OnTreemapSelectparent( ) {
	auto theSelection = GetSelection( );
	if ( theSelection != NULL ) {
		PushReselectChild( theSelection );
		auto p = theSelection->GetParent( );
		if ( p != NULL ) {
			SetSelection( p, true );
			UpdateAllViews( NULL, HINT_SHOWNEWSELECTION );
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			}
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CDirstatDoc::OnUpdateTreemapReselectchild( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( IsReselectChildAvailable( ) );
	}

void CDirstatDoc::OnTreemapReselectchild( ) {
	CItem *item = PopReselectChild( );
	SetSelection( item, true );
	UpdateAllViews( NULL, HINT_SHOWNEWSELECTION );
	}


// CDirstatDoc Diagnostics
#ifdef _DEBUG
void CDirstatDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDirstatDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG



// $Log$
// Revision 1.14  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.13  2004/11/07 10:17:37  bseifert
// Bugfix: Recursive UDCs must not follow junction points.
//
// Revision 1.12  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
