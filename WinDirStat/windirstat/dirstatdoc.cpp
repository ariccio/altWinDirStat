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

CDirstatDoc* _theDocument;

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
	m_freeDiskSpace            = UINT64_MAX;
	m_searchTime               = DBL_MAX;
	m_totalDiskSpace           = UINT64_MAX;
	}

CDirstatDoc::~CDirstatDoc( ) {
	/*
	  Pretty, isn't it?
	  Need to check if m_rootItem, m_zoomItem, m_selectedItem, are equal to any of the others, as they may have been at some point. Else, a memory leak, or (worse) an access violation.
	*/
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
	if ( _theDocument != NULL ) {
		_theDocument   = NULL;
		}
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
						ret += CString( _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.;
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

std::int64_t CDirstatDoc::GetTotlDiskSpace( _In_ CString path ) {
	if ( ( m_freeDiskSpace == UINT64_MAX ) || ( m_totalDiskSpace == UINT64_MAX ) ) {
		MyGetDiskFreeSpace( path, m_totalDiskSpace, m_freeDiskSpace );
		}
	return m_totalDiskSpace;
	}

std::int64_t CDirstatDoc::GetFreeDiskSpace( _In_ CString path ) {
	if ( ( m_freeDiskSpace == UINT64_MAX ) || ( m_totalDiskSpace == UINT64_MAX ) ) {
		MyGetDiskFreeSpace( path, m_totalDiskSpace, m_freeDiskSpace );
		}
	return m_freeDiskSpace;
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

void CDirstatDoc::DecodeSingleSelection( _In_ CString f, _Inout_ CStringArray& drives, _Inout_ CString& folder ) {
	if ( f.GetLength( ) == 2 && f[ 1 ] == _T( ':' ) ) {
		ASSERT( ( f.GetLength( ) == 2 ) && ( f[ 1 ] == _T( ':' ) ) );
		f += _T( "\\" );
		TRACE( _T( "Inserting drive: %s\r\n" ), f );
		drives.Add( f );
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

// The inverse of EncodeSelection
void CDirstatDoc::DecodeSelection(_In_ const CString s, _Inout_ CString& folder, _Inout_ CStringArray& drives) {
	// s is either something like "C:\programme" or something like "C:|D:|E:".

	CStringArray sa;
	INT i = 0;
	addTokens( s, sa, i, _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.

	ASSERT( sa.GetSize( ) > 0 );
	for ( INT j = 0; j < sa.GetSize( ); j++ ) {
		auto f = sa[ j ];
		DecodeSingleSelection( f, drives, folder );
		}
	}

void CDirstatDoc::DeleteContents() {
	if ( m_rootItem != NULL ) {
		delete m_rootItem;
		m_selectedItem = NULL;
		m_zoomItem     = NULL;
		m_rootItem     = NULL;
		}
	SetWorkingItem( NULL );
	GetApp( )->ReReadMountPoints( );
	}

BOOL CDirstatDoc::OnNewDocument( ) {
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	UpdateAllViews( NULL, HINT_NEWROOT );
	return TRUE;
	}


void CDirstatDoc::buildDriveItems( _In_ CStringArray& rootFolders, _Inout_ std::vector<std::shared_ptr<CItem>>& smart_driveItems ) {
	if ( m_showMyComputer ) {
		m_rootItem = new CItem { ITEMTYPE( IT_MYCOMPUTER | ITF_ROOTITEM ), LoadString( IDS_MYCOMPUTER ) };
		for ( INT i = 0; i < rootFolders.GetSize( ); i++ ) {
			auto drive = new CItem{ IT_DRIVE, rootFolders[ i ] };
			auto smart_drive = std::make_shared<CItem>( IT_DRIVE, rootFolders[ i ] );	
			smart_driveItems.emplace_back( std::move( smart_drive ) );
			m_rootItem->AddChild( drive );
			}
		}
	else {
		auto type = IsDrive( rootFolders[ 0 ] ) ? IT_DRIVE : IT_DIRECTORY;
		m_rootItem = new CItem { ITEMTYPE( type | ITF_ROOTITEM ), rootFolders[ 0 ], false };
		if ( m_rootItem->GetType( ) == IT_DRIVE ) {
			smart_driveItems.emplace_back( std::make_shared<CItem>( ITEMTYPE( type | ITF_ROOTITEM ), rootFolders[ 0 ], false ) );
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

BOOL CDirstatDoc::OnOpenDocument(_In_z_ LPCTSTR lpszPathName) {
	CDocument::OnNewDocument(); // --> DeleteContents()
	TRACE( _T( "Opening new document, path: %s\r\n" ), lpszPathName );
	CString spec = lpszPathName;
	CString folder;
	CStringArray drives;
	DecodeSelection( spec, folder, drives );
	check8Dot3NameCreationAndNotifyUser( );
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
		a += (__FUNCTION__, __LINE__ );
		MessageBox( NULL, TEXT( "QueryPerformanceCounter failed!!" ), a.c_str( ), MB_OK );
		}
	behavedWell = QueryPerformanceFrequency( &m_timerFrequency );
	if ( !behavedWell ) {
		std::wstring a;
		a += ( __FUNCTION__, __LINE__ );
		MessageBox( NULL, TEXT( "QueryPerformanceCounter failed!!" ), a.c_str( ), MB_OK );
		}
	
	SetWorkingItem( m_rootItem );
	GetMainFrame( )->FirstUpdateProgress( );
	GetMainFrame( )->MinimizeGraphView( );
	GetMainFrame( )->MinimizeTypeView( );
	UpdateAllViews( NULL, HINT_NEWROOT );
	GetMainFrame( )->FirstUpdateProgress( );
	return true;
	}

void CDirstatDoc::SetPathName( _In_z_ LPCTSTR lpszPathName, BOOL /*bAddToMRU*/) {
	/*
	  We don't want MFCs AfxFullPath()-Logic, because lpszPathName is not a path. So we have overridden this.
	  MRU would be fine but is not implemented yet.
	*/
	m_strPathName = lpszPathName;
	ASSERT( !m_strPathName.IsEmpty( ) );// must be set to something
	m_bEmbedded = FALSE;
	SetTitle( lpszPathName );
	ASSERT_VALID( this );
	}

void CDirstatDoc::Serialize(_In_ const CArchive& /*ar*/) { }

// Prefix the window title (with percentage or "Scanning")
void CDirstatDoc::SetTitlePrefix( const CString prefix ) {
	auto docName = prefix + GetTitle( );
	TRACE( _T( "Setting window title to '%s'\r\n" ), docName );
	GetMainFrame( )->UpdateFrameTitleForDocument( docName );
	}


COLORREF CDirstatDoc::GetCushionColor( _In_ LPCWSTR ext ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}

	if ( m_extensionRecords.size( ) < 10000 ) {
		for ( const auto& aRecord : m_extensionRecords ) {
			if ( aRecord.ext == ext ) {
				return aRecord.color;
				};
			}
		}
	else {
		if ( m_colorMap.empty( ) ) {
			VectorExtensionRecordsToMap( );
			}
		return m_colorMap.at( ext );
		}
	ASSERT( false );
	return COLORREF( 0 );
	}

COLORREF CDirstatDoc::GetZoomColor() const {
	return RGB( 0, 0, 255 );
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

_Success_( return != UINT64_MAX ) std::uint64_t CDirstatDoc::GetRootSize() const {
	ASSERT( m_rootItem != NULL );
	ASSERT( IsRootDone( ) );
	if ( m_rootItem != NULL ) {
		return m_rootItem->GetSize( );
		}
	return UINT64_MAX;
	}

void CDirstatDoc::ForgetItemTree( ) {
	m_zoomItem = NULL;
	m_selectedItem = NULL;
	if ( m_rootItem != NULL ) {
			delete m_rootItem;
			m_rootItem = NULL;
		}
	}

void CDirstatDoc::SortTreeList( ) {
	m_rootItem->SortChildren( );
	auto DirStatView = ( GetMainFrame( )->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
		}
	}

bool CDirstatDoc::WorkFinished( ) {
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
	//Complete?
	SortTreeList();
	m_timeTextWritten = true;
	#ifdef DUMP_MEMUSAGE
	_CrtMemDumpAllObjectsSince( NULL );
	#endif
	return true;
	}

bool CDirstatDoc::Work( _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t ticks ) {
	/*
	  This method does some work for ticks ms. 
	  return: true if done or suspended.
	*/
	if ( m_rootItem == NULL ) { //Bail out!
		TRACE( _T( "There's no work to do! (m_rootItem == NULL) - What the hell? - This can occur if user clicks cancel in drive select box on first opening.\r\n" ) );
		return true;
		}

	if ( GetMainFrame( )->IsProgressSuspended( ) ) {
		return true;
		}

	if ( !m_rootItem->IsDone( ) ) {
		m_rootItem->DoSomeWork( ticks );
		if ( m_rootItem->IsDone( ) ) {
			return WorkFinished( );
			}
		ASSERT( m_workingItem != NULL );
		if ( m_workingItem != NULL ) { // to be honest, "defensive programming" is stupid, but c'est la vie: it's safer. //<== Whoever wrote this is wrong about the stupidity of defensive programming
			GetMainFrame( )->SetProgressPos( m_workingItem->GetProgressPos( ) );
			}
		m_rootItem->SortChildren( );
		UpdateAllViews( NULL, HINT_SOMEWORKDONE );
		}
	if ( m_rootItem->IsDone( ) && m_timeTextWritten ) {
		SetWorkingItem( NULL, true );
		m_rootItem->SortChildren( );
		return true;
		}
	return false;
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
	auto root = GetRootItem( );
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
	auto root =  GetRootItem();
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

void CDirstatDoc::SetSelection( _In_ CItem *item, _In_ const bool keepReselectChildStack ) {
	if ( ( item == NULL ) || ( m_zoomItem == NULL ) ) {
		return;
		}
	auto newzoom = CItem::FindCommonAncestor( m_zoomItem, item );
	if ( newzoom != NULL ) {
		if ( newzoom != m_zoomItem ) {
			TRACE( _T( "Setting new selection\r\n" ) );
			SetZoomItem( newzoom );
			}
		}
	ASSERT( newzoom != NULL );

	m_selectedItem = item;
	GetMainFrame( )->SetSelectionMessageText( );

	if ( !( keepReselectChildStack || ( m_selectedItem == item ) ) ) {
		ClearReselectChildStack( );
		}
	}

_Must_inspect_result_ CItem *CDirstatDoc::GetSelection() const {
	return m_selectedItem;
	}

void CDirstatDoc::SetHighlightExtension(_In_ const LPCTSTR ext) {
#ifdef _DEBUG
	auto oHighlight = m_highlightExtension;
#endif
	if ( m_highlightExtension.CompareNoCase( ext ) != 0 ) {
		m_highlightExtension = ext;
#ifdef _DEBUG
		TRACE( _T( "Highlighting extension %s, old highlight: %s\r\n" ), m_highlightExtension, oHighlight );
#endif
		GetMainFrame( )->SetSelectionMessageText( );
		}
	else {
		TRACE( _T( "NOT highlighting extension: %s\r\n" ), ext );
		}
	}

CString CDirstatDoc::GetHighlightExtension( ) const {
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
	return 0;
	}

void CDirstatDoc::OpenItem(_In_ const CItem* item) {
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
		case IT_DRIVE:
		case IT_DIRECTORY:
			path = item->GetFolderPath( );
			break;
		case IT_FILE:
			path = item->GetPath( );
			break;
		default:
			ASSERT( false );
		}
		ShellExecuteWithAssocDialog( *AfxGetMainWnd( ), path );
	}
	catch ( CException *pe )
	{
		pe->ReportError( );
		pe->Delete( );
	}
	}

void CDirstatDoc::RecurseRefreshMountPointItems(_In_ CItem* item) {
	if ( ( item->GetType( ) == IT_DIRECTORY ) && ( item != GetRootItem( ) ) && GetApp( )->IsMountPoint( item->GetPath( ) ) ) {
		RefreshItem( item );
		}
	for ( auto i = 0; i < item->GetChildrenCount( ); i++ ) {
		RecurseRefreshMountPointItems( item->GetChildGuaranteedValid( i ) );//ranged for?
		}
	}

void CDirstatDoc::RecurseRefreshJunctionItems(_In_ CItem* item) {
	if ( ( item->GetType( ) == IT_DIRECTORY ) && ( item != GetRootItem( ) ) && GetApp( )->IsJunctionPoint( item->GetPath( ) ) ) {
		RefreshItem( item );
		}
	for ( auto i = 0; i < item->GetChildrenCount( ); i++ ) {
		RecurseRefreshJunctionItems( item->GetChildGuaranteedValid( i ) );
		}
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
			auto aChild = root->GetChildGuaranteedValid( i );
			ASSERT( aChild != NULL );
			if ( aChild->GetType( ) == IT_DRIVE ) {
				drives.emplace_back( std::move( aChild ) );
				}
			ASSERT( aChild->GetType( ) == IT_DRIVE );
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
	
	std::map<CString, SExtensionRecord> extensionMap;

	m_rootItem->stdRecurseCollectExtensionData( /*m_extensionRecords,*/ extensionMap );

	AddFileExtensionData( m_extensionRecords, extensionMap );

	stdSetExtensionColors( m_extensionRecords );
	std::sort( m_extensionRecords.begin( ), m_extensionRecords.end( ), s_compareSExtensionRecordByBytes( ) );

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
		auto test = colorVector.at( processed % ( colorVector.size( ) ) );
		++processed;
		if ( processed < ( colorVector.size( ) ) ) {//TODO colors.GetSize( )-> colorsSize
			test = colorVector.at( processed );
			}
		anExtension.color = test;
		//TRACE( _T( "processed: %i, ( processed (mod) colorVector.size() ): %i, c: %lu, color @ [%s]: %lu\r\n" ), processed, ( processed % colorVector.size()), test, anExtension.ext, anExtension.color );
		}
#ifdef _DEBUG
	for ( const auto& a : extensionsToSet ) {
		static_assert( sizeof( LONGLONG ) == 8, "bad format specifiers!" );
		static_assert( sizeof( DWORD ) == sizeof( unsigned long ), "bad format specifiers!" );
		TRACE( _T( "%s: (Bytes: %I64x), (Color: %lu), (Files: %I32x)\r\n" ), a.ext, a.bytes, a.color, a.files );//TODO: bytes has bad format specifier!
		ASSERT( a.color != 0 );
		}
#endif
	}

//CExtensionData *CDirstatDoc::_pqsortExtensionData;

void CDirstatDoc::SetWorkingItemAncestor(_In_ CItem *item ) {
	if ( m_workingItem != NULL ) {
		SetWorkingItem( CItem::FindCommonAncestor( m_workingItem, item ) );
		}
	else {
		SetWorkingItem( item );
		}
	}

void CDirstatDoc::SetWorkingItem( _In_opt_ CItem *item ) {
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

void CDirstatDoc::SetWorkingItem(_In_opt_ CItem *item, _In_ bool hideTiming ) {
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


bool CDirstatDoc::DeletePhysicalItem( _In_ CItem *item, _In_ const bool toTrashBin ) {
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
	return true;
	}

void CDirstatDoc::SetZoomItem(_In_ CItem *item) {
	if ( item == NULL ) {
		return;
		}
	m_zoomItem = item;
	UpdateAllViews( NULL, HINT_ZOOMCHANGED );
	}

void CDirstatDoc::VectorExtensionRecordsToMap( ) {
	auto records = GetExtensionRecords( );
	if ( records != NULL ) {
		for ( const auto& aRecord : ( *records ) ) {
			m_colorMap[ aRecord.ext ] = aRecord.color;
			}
		}
	}

void CDirstatDoc::RefreshItem( _In_ CItem *item ) {
	/*
	  Starts a refresh of an item.
	  If the physical item has been deleted, updates selection, zoom and working item accordingly.
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
	UpdateAllViews( NULL );
	}

void CDirstatDoc::PushReselectChild( CItem* item ) {
	m_reselectChildStack.AddHead( item );
	}

_Must_inspect_result_ CItem* CDirstatDoc::PopReselectChild( ) {
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
	const auto item = GetSelection( );
	if ( item == NULL ) {
		TRACE( _T( "Whoops! That's a NULL item!\r\n" ) );
		return;
		}
	auto thisItemType = item->GetType( );
	pCmdUI->Enable( DirectoryListHasFocus( ) && item != NULL && thisItemType != IT_MYCOMPUTER && thisItemType != IT_FILESFOLDER && thisItemType != IT_FREESPACE && thisItemType != IT_UNKNOWN );
	}

void CDirstatDoc::OnEditCopy() {
	TRACE( _T( "User chose 'Edit'->'Copy'!\r\n") );
	const auto item = GetSelection( );
	if ( item == NULL ) {
		TRACE( _T( "You tried to copy nothing! What does that even mean?\r\n" ) );
		return;
		}
	ASSERT( item->GetType( ) == IT_DRIVE || item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE );

	auto pathToCopy = item->GetPath( );
	GetMainFrame( )->CopyToClipboard( pathToCopy, static_cast<size_t>( pathToCopy.GetLength( ) ) );
	}


void CDirstatDoc::OnUpdateViewShowfreespace( CCmdUI *pCmdUI ) {
	pCmdUI->SetCheck( m_showFreeSpace );
	}

void CDirstatDoc::RemoveFreespaceItem( CItem* drive ) {
	auto freeSpaceItem = drive->FindFreeSpaceItem( );
	ASSERT( freeSpaceItem != NULL );
	if ( freeSpaceItem == NULL ) { 
		return;
		}
	if ( GetSelection( ) == freeSpaceItem ) {
		SetSelection( drive->GetParent( ) );
		}
	if ( GetZoomItem( ) == freeSpaceItem ) {
		auto Parent = freeSpaceItem->GetParent( );
		if ( Parent != NULL ) {
			m_zoomItem = Parent;
			}
		}
	drive->RemoveFreeSpaceItem( );

	}

void CDirstatDoc::OnViewShowfreespace( ) {
	auto drives = modernGetDriveItems( );
	if ( m_showFreeSpace ) {
		for ( const auto& aDrive : drives ) {
			RemoveFreespaceItem( aDrive );
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

void CDirstatDoc::RemoveUnknownItem( CItem* drive ) {
	auto unknownItem = drive->FindUnknownItem( );
	ASSERT( unknownItem != NULL );
	if ( unknownItem == NULL ) {
		return;
		}
	if ( GetSelection( ) == unknownItem ) {
		SetSelection( unknownItem->GetParent( ) );
		}
	if ( GetZoomItem( ) == unknownItem ) {
		auto Parent = unknownItem->GetParent( );
		if ( Parent != NULL ) {
			m_zoomItem = unknownItem->GetParent( );
			}
		}
	drive->RemoveUnknownItem( );
	}


void CDirstatDoc::OnViewShowunknown() {
	auto drives = modernGetDriveItems( );
	if ( m_showUnknown ) {
		for ( auto& drive : drives) {
			RemoveUnknownItem( drive );
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
	auto p = GetSelection( );
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
	auto ZoomItem = GetZoomItem();
	if ( ZoomItem != NULL ) {
		auto parent = ZoomItem->GetParent( );
		if ( parent != NULL ) {
			SetZoomItem( parent );
			}
		}
	ASSERT( ZoomItem != NULL );
	}

void CDirstatDoc::OnUpdateExplorerHere( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( )->GetType( ) != IT_FREESPACE ) && ( GetSelection( )->GetType( ) != IT_UNKNOWN ) );
	}

void CDirstatDoc::OnExplorerHere( ) {
	try
	{
		
		const auto item = GetSelection( );
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
				if ( FAILED( *pidl ) ) {
					displayWindowsMsgBoxWithError( _T( "Failed to get pidl of 'My Computer'!" ) );
					return;
					}
				sei.lpIDList = pidl;
				sei.fMask |= SEE_MASK_IDLIST;

				ShellExecuteEx( &sei );
				// ShellExecuteEx seems to display its own Messagebox on error.
				}
			else {
				MyShellExecute( *AfxGetMainWnd( ), _T( "explore" ), item->GetFolderPath( ), NULL, NULL, SW_SHOWNORMAL );
				}
			}
		ASSERT( item != NULL );
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
			auto cmd = GetCOMSPEC( );

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
	ASSERT( item != NULL );
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
	ASSERT( item != NULL );
	}

void CDirstatDoc::OnCleanupDelete( ) {
	auto item = GetSelection( );
	if ( item == NULL ) {
		ASSERT( item != NULL );
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
		ASSERT( p != NULL );
		}
	ASSERT( theSelection != NULL );
	}

void CDirstatDoc::OnUpdateTreemapReselectchild( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( IsReselectChildAvailable( ) );
	}

void CDirstatDoc::OnTreemapReselectchild( ) {
	auto item = PopReselectChild( );
	SetSelection( item, true );
	UpdateAllViews( NULL, HINT_SHOWNEWSELECTION );
	}


// CDirstatDoc Diagnostics
#ifdef _DEBUG
void CDirstatDoc::AssertValid( ) const {
	CDocument::AssertValid();
	}

void CDirstatDoc::Dump( CDumpContext& dc ) const {
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
