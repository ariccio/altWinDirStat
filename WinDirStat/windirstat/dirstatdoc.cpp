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
#include "dirstatdoc.h"
#include "item.h"
#include "dirstatview.h"
#include "globalhelpers.h"
#include "windirstat.h"
#include "mainframe.h"

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

	std::vector<std::wstring> addTokens( _In_ const std::wstring& s, _Inout_ rsize_t& i, _In_ TCHAR EncodingSeparator ) {
		std::vector<std::wstring> sa;
		while ( i < s.length( ) ) {
			CString token;
			while ( i < s.length( ) && s.at( i ) != EncodingSeparator ) {
				token += s.at( i++ );
				}
		
			token.TrimLeft( );
			token.TrimRight( );
			ASSERT( !token.IsEmpty( ) );
			sa.emplace_back( std::wstring( token.GetString( ) ) );

			if ( i < s.length( ) ) {
				i++;
				}
			}
		return sa;
		}


	void DecodeSingleSelection( _In_ std::wstring f, _Inout_ std::vector<std::wstring>& drives, _Inout_ std::wstring& folder ) {
		if ( f.length( ) == 2 && f[ 1 ] == _T( ':' ) ) {
			ASSERT( ( f.length( ) == 2 ) && ( f[ 1 ] == _T( ':' ) ) );
			f += _T( "\\" );
			
			auto strcmp_path = f.compare( 0, 4, L"\\\\?\\", 0, 4 );
			if ( strcmp_path != 0 ) {
				auto fixedPath = L"\\\\?\\" + f;
				TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
				f = fixedPath;
				}

			TRACE( _T( "Inserting drive: %s\r\n" ), f.c_str( ) );
			drives.emplace_back( f );
			}
		else {
			// Remove trailing backslash, if any and not drive-root.
			if ( f.length( ) > 0 && f.back( ) == _T( '\\' ) && ( f.length( ) != 3 || f[ 1 ] != _T( ':' ) ) ) {
				f = f.substr( 0, f.length( ) - 1 );
				}
			TRACE( _T( "Whoops! %s is not a drive, it's a folder!\r\n" ), f );
			folder = f;
			}
		}

	std::vector<std::wstring> DecodeSelection( _In_ const std::wstring& s, _Inout_ std::wstring& folder ) {
		
		std::vector<std::wstring> drives;
		// s is either something like "C:\programme" or something like "C:|D:|E:".
		rsize_t i = 0;
		auto sa = addTokens( s, i, _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.

		ASSERT( sa.size( ) > 0 );
		for ( size_t j = 0; j < sa.size( ); j++ ) {
			DecodeSingleSelection( sa.at( j ), drives, folder );
			}
		return drives;
		}
	std::vector<COLORREF> GetDefaultPaletteAsVector( ) {
		std::vector<COLORREF> colorVector;
		std::vector<COLORREF> defaultColorVec = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 150, 150, 255 ), RGB( 255, 150, 150 ), RGB( 150, 255, 150 ), RGB( 150, 255, 255 ), RGB( 255, 150, 255 ), RGB( 255, 255, 150 ), RGB( 255, 255, 255 ) };
		colorVector.reserve( defaultColorVec.size( ) + 1 );
		for ( auto& aColor : defaultColorVec ) {
			colorVector.emplace_back( CColorSpace::MakeBrightColor( aColor, PALETTE_BRIGHTNESS ) );
			}
		return colorVector;
		}

	}

CDirstatDoc* _theDocument;

CDirstatDoc* GetDocument() {
	ASSERT( _theDocument != NULL );
	return _theDocument;
	}

IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)

_Pre_satisfies_( _theDocument == NULL ) _Post_satisfies_( _theDocument == this )
CDirstatDoc::CDirstatDoc( ) : m_workingItem( NULL ), m_zoomItem( NULL ), m_selectedItem( NULL ), m_extensionDataValid( false ), m_timeTextWritten( false ), m_showMyComputer( true ), m_freeDiskSpace( UINT64_MAX ), m_totalDiskSpace( UINT64_MAX ), m_searchTime( DBL_MAX ) {
	ASSERT( _theDocument == NULL );
	_theDocument               = this;
	m_searchStartTime.QuadPart = 0;
	m_timerFrequency.QuadPart  = 0;
	}

CDirstatDoc::~CDirstatDoc( ) {
	_theDocument = { NULL };
	m_rootItem.reset( );
	}

void CDirstatDoc::DeleteContents( ) {
	if ( m_rootItem ) {
		m_rootItem.reset( );
		}
	m_selectedItem = { NULL };
	m_zoomItem     = { NULL };
	m_workingItem  = { NULL };
	m_timeTextWritten = false;
	//GetApp( )->m_mountPoints.Initialize( );
	}

BOOL CDirstatDoc::OnNewDocument( ) {
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return TRUE;
	}

void CDirstatDoc::buildDriveItems( _In_ const std::vector<std::wstring>& rootFolders ) {
	FILETIME t;
	zeroDate( t );
	if ( m_showMyComputer ) {
		for ( size_t i = 0; i < rootFolders.size( ); i++ ) {
			auto smart_drive = new CItemBranch { IT_DIRECTORY, std::move( rootFolders.at( i ) ), static_cast<std::uint64_t>( 0 ), t, 0, false };
			smart_drive->m_parent = m_rootItem.get( );
			m_rootItem->AddChild( smart_drive );
			}
		}
	else {
		m_rootItem = std::make_unique<CItemBranch>( IT_DIRECTORY, std::move( rootFolders.at( 0 ) ), 0, t, 0, false );
		m_rootItem->m_parent = { NULL };
		}
	}

std::vector<std::wstring> CDirstatDoc::buildRootFolders( _In_ std::vector<std::wstring>& drives, _In_ std::wstring& folder ) {
	std::vector<std::wstring> rootFolders;
	if ( drives.size( ) > 0 ) {
		m_showMyComputer = ( drives.size( ) > 1 );
		for ( size_t i = 0; i < drives.size( ); i++ ) {
			rootFolders.emplace_back( drives[ i ] );
			}
		}
	else {
		ASSERT( !folder.empty( ) );
		m_showMyComputer = false;
		rootFolders.emplace_back( folder );
		}
	return rootFolders;
	}


BOOL CDirstatDoc::OnOpenDocument( _In_z_ PCWSTR pszPathName ) {
	GetApp( )->m_mountPoints.Initialize( );
	CDocument::OnNewDocument(); // --> DeleteContents()
	TRACE( _T( "Opening new document, path: %s\r\n" ), pszPathName );
	std::wstring spec = pszPathName;
	std::wstring folder;
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
	
	//GetMainFrame( )->MinimizeGraphView( );
	GetMainFrame( )->m_wndSplitter.SetSplitterPos( 1.0 );
	
	
	//GetMainFrame( )->MinimizeTypeView( );
	GetMainFrame( )->m_wndSubSplitter.SetSplitterPos( 1.0 );
	
	
	UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	//GetMainFrame( )->FirstUpdateProgress( );
	return true;
	}

// Prefix the window title (with percentage or "Scanning")
void CDirstatDoc::SetTitlePrefix( _In_ std::wstring prefix ) const {
	auto docName = std::wstring( prefix + GetTitle( ).GetString( ) );
	TRACE( _T( "Setting window title to '%s'\r\n" ), docName.c_str( ) );
	GetMainFrame( )->UpdateFrameTitleForDocument( docName.c_str( ) );
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
	return static_cast<COLORREF>( 0 );
	}

_Ret_notnull_ const std::vector<SExtensionRecord>* CDirstatDoc::GetExtensionRecords( ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
 	return &m_extensionRecords;
	}

_Success_( return < UINT64_MAX )
std::uint64_t CDirstatDoc::GetRootSize( ) const {
	ASSERT( IsRootDone( ) );
	if ( m_rootItem ) {
		auto retVal = m_rootItem->size_recurse( );
		return retVal;
		}
	return UINT64_MAX;
	}

void CDirstatDoc::ForgetItemTree( ) {
	TRACE( _T( "forgetting tree...\r\n" ) );
	const auto qpc_1 = help_QueryPerformanceCounter( );
	m_zoomItem     = { NULL };
	m_selectedItem = { NULL };
	m_workingItem  = { NULL };
	m_rootItem.reset( );
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );
	const auto timing = ( static_cast<double>( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast<double>( 1.0 ) / static_cast<double>( qpf.QuadPart ) ) );
	TRACE( _T( "ForgetItemTree timing: %f\r\n" ), timing );
	}

void CDirstatDoc::SortTreeList( ) {
	ASSERT( m_rootItem != NULL );
	m_rootItem->SortChildren( );
	auto DirStatView = ( GetMainFrame( )->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
		}
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
	const DOUBLE AdjustedTimerFrequency = ( static_cast<DOUBLE>( 1 ) ) / static_cast<DOUBLE>( m_timerFrequency.QuadPart );
			
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
	auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp_path != 0 ) {
		path = L"\\\\?\\" + path;
		TRACE( _T( "path fixed as: %s\r\n" ), path.c_str( ) );
		}
		DoSomeWorkShim( m_rootItem.get( ), std::move( path ), true );
		ASSERT( m_rootItem->IsTreeDone( ) );
		
		//cache the size of root item
		m_rootItem->refresh_sizeCache( );

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

_Must_inspect_result_ _Ret_maybenull_ 
const CItemBranch* CDirstatDoc::GetRootItem( ) const {
	return m_rootItem.get( );
	}

_Must_inspect_result_ _Ret_maybenull_
const CItemBranch* CDirstatDoc::GetZoomItem( ) const {
	return m_zoomItem;
	}

_Ret_range_( 0, 33000 ) 
DOUBLE CDirstatDoc::GetNameLength( ) const {
 	return m_rootItem->averageNameLength( );
	}

bool CDirstatDoc::IsZoomed( ) const {
	return m_zoomItem != m_rootItem.get( );
	}

_Pre_satisfies_( this->m_zoomItem != NULL ) _When_( ( this->m_zoomItem != NULL ), _Post_satisfies_( m_selectedItem == ( &item ) ) ) 
void CDirstatDoc::SetSelection( _In_ const CItemBranch& item ) {
	if ( m_zoomItem == NULL ) {
		return;
		}
	auto newzoom = FindCommonAncestor( m_zoomItem, item );
	if ( newzoom != NULL ) {
		if ( newzoom != m_zoomItem ) {
			TRACE( _T( "Setting new selection\r\n" ) );
			SetZoomItem( *newzoom );
			}
		}
	ASSERT( newzoom != NULL );

	m_selectedItem = const_cast< CItemBranch* >( &item );
	GetMainFrame( )->SetSelectionMessageText( );

	}

_Must_inspect_result_ _Ret_maybenull_
const CItemBranch* CDirstatDoc::GetSelection( ) const {
	return m_selectedItem;
	}

void CDirstatDoc::SetHighlightExtension( _In_ const std::wstring ext ) {
	if ( m_highlightExtension.compare( ext ) != 0 ) {
		m_highlightExtension = ext;
		TRACE( _T( "Highlighting extension %s\r\n" ), m_highlightExtension.c_str( ) );
		GetMainFrame( )->SetSelectionMessageText( );
		}
	else {
		TRACE( _T( "NOT highlighting extension: %s (already selected)\r\n" ), ext.c_str( ) );
		}
	}

const std::wstring& CDirstatDoc::GetHighlightExtension( ) const {
	return m_highlightExtension;
	}

_Pre_satisfies_( item.m_type == IT_FILE )
void CDirstatDoc::OpenItem( _In_ const CItemBranch& item ) {
	CWaitCursor wc;
	std::wstring path;
	if ( item.m_type == IT_FILE ) {
		path = item.GetPath( ).c_str( );
		}
	auto doesFileExist = PathFileExistsW( path.c_str( ) );
	if ( !doesFileExist ) {
		TRACE( _T( "Path (%s) is invalid!\r\n" ), path.c_str( ) );
		std::wstring pathMsg( L"Path (" + path + L") is invalid!\r\n");
		AfxMessageBox( pathMsg.c_str( ) );
		displayWindowsMsgBoxWithError( );
		return;
		}

	auto ShellExRes = ShellExecuteWithAssocDialog( *AfxGetMainWnd( ), path.c_str( ) );
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
	const auto colorVector = GetDefaultPaletteAsVector( );
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
		TRACE( _T( "processed: %i, ( processed (mod) colorVector.size() ): %i, c: %lu, color @ [%s]: %lu\r\n" ), processed, ( processed % colorVector.size()), test, anExtension.ext.c_str( ), anExtension.color );
#endif
#endif

		}
#ifdef _DEBUG
#ifdef EXTENSION_LIST_DEBUG
	for ( const auto& a : extensionsToSet ) {
		static_assert( sizeof( LONGLONG ) == 8, "bad format specifiers!" );
		static_assert( sizeof( DWORD ) == sizeof( unsigned long ), "bad format specifiers!" );
		TRACE( _T( "%s: (Bytes: %I64x), (Color: %lu), (Files: %I32x)\r\n" ), a.ext.c_str( ), a.bytes, a.color, a.files );//TODO: bytes has bad format specifier!
		ASSERT( a.color != 0 );
		}
#endif
#endif
	}

void CDirstatDoc::SetWorkingItem( _In_opt_ const CItemBranch* const item ) {
	m_workingItem = item;
	}

void CDirstatDoc::SetZoomItem( _In_ const CItemBranch& item ) {
	m_zoomItem = ( &item );
	UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_ZOOMCHANGED );
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
	return ( GetMainFrame( )->m_logicalFocus == focus::LF_DIRECTORYLIST );
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
	CItemBranch const* z = { NULL };
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
	SetZoomItem( *z );
	}

void CDirstatDoc::OnUpdateTreemapZoomout( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( m_rootItem && ( m_rootItem->IsTreeDone( ) ) && ( m_zoomItem != m_rootItem.get( ) ) );
	}

_Pre_satisfies_( this->m_zoomItem != NULL )
void CDirstatDoc::OnTreemapZoomout( ) {
	if ( m_zoomItem != NULL ) {
		auto parent = m_zoomItem->GetParent( );
		if ( parent != NULL ) {
			SetZoomItem( *parent );
			}
		}
	}

void CDirstatDoc::OnUpdateTreemapSelectparent( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( ( m_selectedItem != NULL ) && ( m_selectedItem->GetParent( ) != NULL ) );
	}

_Pre_satisfies_( this->m_selectedItem != NULL )
void CDirstatDoc::OnTreemapSelectparent( ) {
	if ( m_selectedItem != NULL ) {
		auto p = m_selectedItem->GetParent( );
		ASSERT( p != NULL );
		if ( p != NULL ) {
			SetSelection( *p );
			UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION );
			}
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
