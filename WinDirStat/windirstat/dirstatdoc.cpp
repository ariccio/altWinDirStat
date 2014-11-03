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

//#include "item.h"
//#include ".\dirstatdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
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

	std::vector<CString> addTokens( _In_ const CString& s, _Inout_ INT& i, _In_ TCHAR EncodingSeparator ) {
		std::vector<CString> sa;
		while ( i < s.GetLength( ) ) {
			CString token;
			while ( i < s.GetLength( ) && s[ i ] != EncodingSeparator ) {
				token += s[ i++ ];
				}
		
			token.TrimLeft( );
			token.TrimRight( );
			ASSERT( !token.IsEmpty( ) );
			sa.emplace_back( token );

			if ( i < s.GetLength( ) ) {
				i++;
				}
			}
		return sa;
		}


	void DecodeSingleSelection( _In_ CString f, _Inout_ std::vector<CString>& drives, _Inout_ CString& folder ) {
		if ( f.GetLength( ) == 2 && f[ 1 ] == _T( ':' ) ) {
			ASSERT( ( f.GetLength( ) == 2 ) && ( f[ 1 ] == _T( ':' ) ) );
			f += _T( "\\" );
			TRACE( _T( "Inserting drive: %s\r\n" ), f );
			drives.emplace_back( f );
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

	std::vector<CString> DecodeSelection( _In_ const CString& s, _Inout_ CString& folder ) {
		
		std::vector<CString> drives;
		// s is either something like "C:\programme" or something like "C:|D:|E:".
		INT i = 0;
		auto sa = addTokens( s, i, _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.

		ASSERT( sa.size( ) > 0 );
		for ( size_t j = 0; j < sa.size( ); j++ ) {
			DecodeSingleSelection( sa.at( j ), drives, folder );
			}
		return drives;
		}
	}

CDirstatDoc* _theDocument;

CDirstatDoc* GetDocument() {
	ASSERT( _theDocument != NULL );
	return _theDocument;
	}

IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)

_Pre_satisfies_( _theDocument == NULL ) _Post_satisfies_( _theDocument == this ) CDirstatDoc::CDirstatDoc( ) : m_workingItem( NULL ), m_zoomItem( NULL ), m_selectedItem( NULL ), m_extensionDataValid( false ), m_timeTextWritten( false ), m_showMyComputer( true ), m_freeDiskSpace( UINT64_MAX ), m_totalDiskSpace( UINT64_MAX ), m_searchTime( DBL_MAX ) {
	ASSERT( _theDocument == NULL );
	_theDocument               = this;
	m_searchStartTime.QuadPart = 0;
	m_timerFrequency.QuadPart  = 0;
	}

CDirstatDoc::~CDirstatDoc( ) {
	_theDocument = NULL;
	m_rootItem.reset( );
	}

void CDirstatDoc::DeleteContents( ) {
	if ( m_rootItem ) {
		m_rootItem.reset( );
		}
	m_selectedItem = NULL;
	m_zoomItem     = NULL;
	m_workingItem  = NULL;
	//GetApp( )->m_mountPoints.Initialize( );
	}

BOOL CDirstatDoc::OnNewDocument( ) {
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	UpdateAllViews( NULL, HINT_NEWROOT );
	return TRUE;
	}

void CDirstatDoc::buildDriveItems( _In_ const std::vector<CString>& rootFolders ) {
	FILETIME t;
	zeroDate( t );
	if ( m_showMyComputer ) {
		for ( size_t i = 0; i < rootFolders.size( ); i++ ) {
			auto smart_drive = new CItemBranch { IT_DIRECTORY, rootFolders.at( i ).GetString( ), static_cast<std::uint64_t>( 0 ), t, 0, false };
			smart_drive->m_parent = m_rootItem.get( );
			m_rootItem->AddChild( smart_drive );
			}
		}
	else {
		m_rootItem = std::make_unique<CItemBranch>( IT_DIRECTORY, rootFolders.at( 0 ).GetString( ), 0, t, 0, false );
		m_rootItem->m_parent = NULL;
		}
	}

std::vector<CString> CDirstatDoc::buildRootFolders( _In_ std::vector<CString>& drives, _In_ CString& folder ) {
	std::vector<CString> rootFolders;
	if ( drives.size( ) > 0 ) {
		m_showMyComputer = ( drives.size( ) > 1 );
		for ( size_t i = 0; i < drives.size( ); i++ ) {
			rootFolders.emplace_back( drives[ i ] );
			}
		}
	else {
		ASSERT( !folder.IsEmpty( ) );
		m_showMyComputer = false;
		folder.FreeExtra( );
		rootFolders.emplace_back( folder );
		}
	return rootFolders;
	}


BOOL CDirstatDoc::OnOpenDocument( _In_z_ PCWSTR pszPathName ) {
	GetApp( )->m_mountPoints.Initialize( );
	CDocument::OnNewDocument(); // --> DeleteContents()
	TRACE( _T( "Opening new document, path: %s\r\n" ), pszPathName );
	CString spec = pszPathName;
	CString folder;
	auto drives = DecodeSelection( spec, folder );
	check8Dot3NameCreationAndNotifyUser( );

#ifdef PERF_DEBUG_SLEEP
	displayWindowsMsgBoxWithMessage( _T( "PERF_DEBUG_SLEEP ENABLED! this is meant for debugging!" ) );
#endif

	auto rootFolders_ = buildRootFolders( drives, folder );
	buildDriveItems( rootFolders_ );

	m_zoomItem = m_rootItem.get( );

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );

	m_searchStartTime = help_QueryPerformanceCounter( );	
	m_timerFrequency = help_QueryPerformanceFrequency( );

	m_workingItem = m_rootItem.get( );

	//GetMainFrame( )->FirstUpdateProgress( );
	GetMainFrame( )->MinimizeGraphView( );
	GetMainFrame( )->MinimizeTypeView( );
	UpdateAllViews( NULL, HINT_NEWROOT );
	//GetMainFrame( )->FirstUpdateProgress( );
	return true;
	}

// Prefix the window title (with percentage or "Scanning")
void CDirstatDoc::SetTitlePrefix( _In_ const CString& prefix ) const {
	auto docName = prefix + GetTitle( );
	TRACE( _T( "Setting window title to '%s'\r\n" ), docName );
	GetMainFrame( )->UpdateFrameTitleForDocument( docName );
	}

COLORREF CDirstatDoc::GetCushionColor( _In_z_ PCWSTR ext ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
	if ( m_colorMap.empty( ) ) {
		VectorExtensionRecordsToMap( );
		}
		
	if ( m_colorMap.count( ext ) > 0 ) {
		return m_colorMap.at( ext );
		}
	TRACE( _T( "Extension %s not in colorMap!\r\n" ), ext );
	RebuildExtensionData( );
	VectorExtensionRecordsToMap( );
	if ( m_colorMap.count( ext ) > 0 ) {
		return m_colorMap.at( ext );
		}
	TRACE( _T( "Extension %s not in rebuilt colorMap!\r\n" ), ext );
	ASSERT( false );
	return COLORREF( 0 );
	}

std::vector<SExtensionRecord>* CDirstatDoc::GetExtensionRecords( ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
 	return &m_extensionRecords;
	}

_Success_( return != UINT64_MAX ) std::uint64_t CDirstatDoc::GetRootSize( ) const {
	ASSERT( IsRootDone( ) );
	if ( m_rootItem ) {
		auto retVal = m_rootItem->size_recurse( );
		return retVal;
		}
	return UINT64_MAX;
	}

void CDirstatDoc::ForgetItemTree( ) {
	m_zoomItem = NULL;
	m_selectedItem = NULL;
	m_rootItem.reset( );
	}

void CDirstatDoc::SortTreeList( ) {
	ASSERT( m_rootItem != NULL );
	m_rootItem->SortChildren( );
	auto DirStatView = ( GetMainFrame( )->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
		}
	}

DOUBLE CDirstatDoc::GetNameLength( ) const {
	return m_rootItem->averageNameLength( );
	}

bool CDirstatDoc::OnWorkFinished( ) {
	TRACE( _T( "Finished walking tree...\r\n" ) );
	//m_rootItem->SortChildren( );
	//m_rootItem->SortAndSetDone( );
#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif
	m_extensionDataValid = false;
	GetMainFrame( )->RestoreTypeView( );

	auto doneTime = help_QueryPerformanceCounter( );
	const DOUBLE AdjustedTimerFrequency = ( DOUBLE( 1 ) ) / DOUBLE( m_timerFrequency.QuadPart );
			
	UpdateAllViews( NULL );
	if ( doneTime.QuadPart != NULL ) {
		m_searchTime = ( doneTime.QuadPart - m_searchStartTime.QuadPart ) * AdjustedTimerFrequency;
		}
	else {
		m_searchTime = -2;//Negative (that's not -1) informs WriteTimeToStatusBar that there was a problem.
		}
	GetMainFrame( )->RestoreGraphView( );
	//Complete?
	SortTreeList();
	m_timeTextWritten = true;
	TRACE( _T( "All work finished!\r\n" ) );
#ifdef DUMP_MEMUSAGE
	_CrtMemDumpAllObjectsSince( NULL );
#endif
	return true;
	}

bool CDirstatDoc::Work( ) {
	//return: true if done or suspended.

	if ( ( !m_rootItem ) || m_timeTextWritten ) {
		ASSERT( m_workingItem == NULL );
		//TRACE( _T( "There's no work to do! This can occur if user clicks cancel in drive select box on first opening.\r\n" ) );
		return true;
		}
	if ( !m_rootItem->IsTreeDone( ) ) {
		auto path = ( m_rootItem->GetPath( ) );
		//ASSERT( path.Right( 1 ) != _T( '\\' ) );
		//if ( path.back( ) != _T( '\\' ) ) {
		//	ASSERT( path.back( ) != _T( '*' ) );
		//	//path += _T( "\\*.*" );
		//	}
		//else {
		//	ASSERT( path.back( ) != _T( '*' ) );
		//	//path += _T( "*.*" );
		//	}
		//path += _T( "\\*.*" );
		//DoSomeWork( m_rootItem.get( ), path.GetString( ) );
	auto strcmp = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp != 0 ) {
		//auto fixedPath = L"\\\\?\\" + path;
		//TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		//path = fixedPath;
		path = L"\\\\?\\" + path;
		TRACE( _T( "path fixed as: %s\r\n" ), path.c_str( ) );
		}


		DoSomeWork( m_rootItem.get( ), std::move( path ), true );
		ASSERT( m_rootItem->IsTreeDone( ) );
		SetWorkingItem( NULL );
		auto res = OnWorkFinished( );
		m_rootItem->AddChildren( );
		return res;
		}
	ASSERT( m_workingItem != NULL );
	return false;
	}

bool CDirstatDoc::IsRootDone( ) const {
	auto retVal = ( m_rootItem && m_rootItem->IsTreeDone( ) );
	return retVal;
	}

_Must_inspect_result_ CItemBranch* CDirstatDoc::GetRootItem( ) const {
	return m_rootItem.get( );
	}

_Must_inspect_result_ _Ret_maybenull_ CItemBranch* CDirstatDoc::GetZoomItem( ) const {
	return m_zoomItem;
	}

bool CDirstatDoc::IsZoomed( ) const {
	return m_zoomItem != m_rootItem.get( );
	}

_Pre_satisfies_( this->m_zoomItem != NULL ) _When_( ( item != NULL ) && ( this->m_zoomItem != NULL ), _Post_satisfies_( m_selectedItem == item ) ) void CDirstatDoc::SetSelection( _In_ const CItemBranch* const item ) {
	if ( ( item == NULL ) || ( m_zoomItem == NULL ) ) {
		return;
		}
	auto newzoom = FindCommonAncestor( m_zoomItem, item );
	if ( newzoom != NULL ) {
		if ( newzoom != m_zoomItem ) {
			TRACE( _T( "Setting new selection\r\n" ) );
			SetZoomItem( newzoom );
			}
		}
	ASSERT( newzoom != NULL );

	m_selectedItem = const_cast< CItemBranch* >( item );
	GetMainFrame( )->SetSelectionMessageText( );

	}

_Must_inspect_result_ _Ret_maybenull_ CItemBranch* CDirstatDoc::GetSelection( ) const {
	return m_selectedItem;
	}

void CDirstatDoc::SetHighlightExtension( _In_z_ const PCWSTR ext ) {
	if ( m_highlightExtension.compare( ext ) != 0 ) {
		m_highlightExtension = ext;
		TRACE( _T( "Highlighting extension %s\r\n" ), m_highlightExtension.c_str( ) );
		GetMainFrame( )->SetSelectionMessageText( );
		}
	else {
		TRACE( _T( "NOT highlighting extension: %s (already selected)\r\n" ), ext );
		}
	}

const std::wstring& CDirstatDoc::GetHighlightExtension( ) const {
	return m_highlightExtension;
	}

_Pre_satisfies_( item.m_type == IT_FILE ) void CDirstatDoc::OpenItem( _In_ const CItemBranch& item ) {
	CWaitCursor wc;
	CString path;
	if ( item.m_type == IT_FILE ) {
		path = item.GetPath( ).c_str( );
		}
	auto doesFileExist = PathFileExistsW( path );
	if ( !doesFileExist ) {
		TRACE( _T( "Path (%s) is invalid!\r\n" ), path );
		CString pathMsg( L"Path (" );
		pathMsg += path;
		pathMsg += _T( ") is invalid!\r\n" );
		AfxMessageBox( pathMsg );
		displayWindowsMsgBoxWithError( );
		return;
		}

	auto ShellExRes = ShellExecuteWithAssocDialog( *AfxGetMainWnd( ), path );
	if ( ShellExRes < 33 ) {
		return displayWindowsMsgBoxWithMessage( GetLastErrorAsFormattedMessage( ) );
		}
	}

void CDirstatDoc::RebuildExtensionData() {
	//Assigns colors to all known file types (i.e. `Extensions`)

	CWaitCursor wc;
	
	m_extensionRecords.clear( );
	
	std::map<std::wstring, SExtensionRecord> extensionMap;

	auto rootTemp = m_rootItem.get( );

	rootTemp->stdRecurseCollectExtensionData( extensionMap );
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
	static const auto colorVector = GetDefaultPaletteAsVector( );
	std::vector<COLORREF>::size_type processed = 0;

	for ( auto& anExtension : extensionsToSet ) {
		auto test = colorVector.at( processed % ( colorVector.size( ) ) );
		++processed;
		if ( processed < ( colorVector.size( ) ) ) {//TODO colors.GetSize( )-> colorsSize
			test = colorVector.at( processed );
			}
		anExtension.color = test;
#ifdef _DEBUG
#ifdef EXTENSION_LIST_DEBUG
		TRACE( _T( "processed: %i, ( processed (mod) colorVector.size() ): %i, c: %lu, color @ [%s]: %lu\r\n" ), processed, ( processed % colorVector.size()), test, anExtension.ext, anExtension.color );
#endif
#endif

		}
#ifdef _DEBUG
#ifdef EXTENSION_LIST_DEBUG
	for ( const auto& a : extensionsToSet ) {
		static_assert( sizeof( LONGLONG ) == 8, "bad format specifiers!" );
		static_assert( sizeof( DWORD ) == sizeof( unsigned long ), "bad format specifiers!" );
		TRACE( _T( "%s: (Bytes: %I64x), (Color: %lu), (Files: %I32x)\r\n" ), a.ext, a.bytes, a.color, a.files );//TODO: bytes has bad format specifier!
		ASSERT( a.color != 0 );
		}
#endif
#endif
	}

void CDirstatDoc::SetWorkingItem( _In_opt_ CItemBranch* const item ) {
	m_workingItem = item;
	}

void CDirstatDoc::SetZoomItem( _In_ _Const_ CItemBranch* const item ) {
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

bool CDirstatDoc::DirectoryListHasFocus( ) const {
	return ( GetMainFrame( )->GetLogicalFocus( ) == LF_DIRECTORYLIST );
	}

BEGIN_MESSAGE_MAP(CDirstatDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_SELECTPARENT, OnUpdateTreemapSelectparent)
	ON_COMMAND(ID_TREEMAP_SELECTPARENT, OnTreemapSelectparent)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMIN, OnUpdateTreemapZoomin)
	ON_COMMAND(ID_TREEMAP_ZOOMIN, OnTreemapZoomin)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMOUT, OnUpdateTreemapZoomout)
	ON_COMMAND(ID_TREEMAP_ZOOMOUT, OnTreemapZoomout)
END_MESSAGE_MAP()

void CDirstatDoc::OnUpdateEditCopy( _In_ CCmdUI* pCmdUI ) {
	if ( m_selectedItem == NULL ) {
		TRACE( _T( "Whoops! That's a NULL item!\r\n" ) );
		return;
		}
	pCmdUI->Enable( DirectoryListHasFocus( ) && m_selectedItem != NULL );
	}

void CDirstatDoc::OnEditCopy( ) {
	TRACE( _T( "User chose 'Edit'->'Copy'!\r\n") );
	if ( m_selectedItem == NULL ) {
		TRACE( _T( "You tried to copy nothing! What does that even mean?\r\n" ) );
		return;
		}

	auto itemPath = m_selectedItem->GetPath( );
	itemPath.resize( itemPath.length( ) + MAX_PATH );
	GetMainFrame( )->CopyToClipboard( itemPath.c_str( ), static_cast<rsize_t>( itemPath.length( ) + 1 ) );
	//itemPath.ReleaseBuffer( );
	}

void CDirstatDoc::OnUpdateTreemapZoomin( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( m_rootItem && ( m_rootItem->IsTreeDone( ) ) && ( m_selectedItem != NULL ) && ( m_selectedItem != m_zoomItem ) );
	}

void CDirstatDoc::OnTreemapZoomin( ) {
	auto p = m_selectedItem;
	CItemBranch* z = NULL;
	auto zoomItem = m_zoomItem;
	while ( p != zoomItem ) {
		z = p;
		if ( p != NULL ) {
			p = p->GetParent( );
			}
		else {
			break;
			}
		}
	if ( z == NULL ) {
		return;
		}
	ASSERT( z != NULL );
	SetZoomItem( z );
	}

void CDirstatDoc::OnUpdateTreemapZoomout( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( m_rootItem && ( m_rootItem->IsTreeDone( ) ) && ( m_zoomItem != m_rootItem.get( ) ) );
	}

_Pre_satisfies_( this->m_zoomItem != NULL ) void CDirstatDoc::OnTreemapZoomout( ) {
	if ( m_zoomItem != NULL ) {
		auto parent = m_zoomItem->GetParent( );
		if ( parent != NULL ) {
			SetZoomItem( parent );
			}
		}
	}

void CDirstatDoc::OnUpdateTreemapSelectparent( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( ( m_selectedItem != NULL ) && ( m_selectedItem->GetParent( ) != NULL ) );
	}

_Pre_satisfies_( this->m_selectedItem != NULL ) void CDirstatDoc::OnTreemapSelectparent( ) {
	if ( m_selectedItem != NULL ) {
		auto p = m_selectedItem->GetParent( );
		if ( p != NULL ) {
			SetSelection( p );
			UpdateAllViews( NULL, HINT_SHOWNEWSELECTION );
			}
		ASSERT( p != NULL );
		}
	ASSERT( m_selectedItem != NULL );
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
