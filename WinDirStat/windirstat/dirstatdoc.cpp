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

	UINT workerThreadFunc( LPVOID pParam ) {
		return 0;
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


	void DecodeSingleSelection( _In_ CString f, _Inout_ CStringArray& drives, _Inout_ CString& folder ) {
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

	void DecodeSelection( _In_ const CString s, _Inout_ CString& folder, _Inout_ CStringArray& drives ) {
		// s is either something like "C:\programme" or something like "C:|D:|E:".
		CStringArray sa;
		INT i = 0;
		addTokens( s, sa, i, _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.

		ASSERT( sa.GetSize( ) > 0 );
		for ( INT j = 0; j < sa.GetSize( ); j++ ) {
			DecodeSingleSelection( sa[ j ], drives, folder );
			}
		}

	}

CDirstatDoc* _theDocument;

CDirstatDoc* GetDocument() {
	ASSERT( _theDocument != NULL );
	return _theDocument;
	}

IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)

CDirstatDoc::CDirstatDoc( ) : m_workingItem( NULL ), m_zoomItem( NULL ), m_selectedItem( NULL ), m_extensionDataValid( false ), m_timeTextWritten( false ), m_showMyComputer( true ) {
	InitializeCriticalSection( &m_rootItemCriticalSection );
	ASSERT( _theDocument == NULL );
	_theDocument               = this;
	m_searchStartTime.QuadPart = NULL;
	m_timerFrequency.QuadPart  = NULL;
	m_freeDiskSpace            = UINT64_MAX;
	m_searchTime               = DBL_MAX;
	m_totalDiskSpace           = UINT64_MAX;
	}

CDirstatDoc::~CDirstatDoc( ) {
	_theDocument = NULL;
	DeleteCriticalSection( &m_rootItemCriticalSection );
	}

void CDirstatDoc::DeleteContents() {
	EnterCriticalSection( &m_rootItemCriticalSection );
	if ( m_rootItem ) {
		m_rootItem.reset( );
		m_selectedItem = NULL;
		m_zoomItem     = NULL;
		}
	SetWorkingItem( NULL );
	LeaveCriticalSection( &m_rootItemCriticalSection );
	GetApp( )->ReReadMountPoints( );
	}

BOOL CDirstatDoc::OnNewDocument( ) {
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	UpdateAllViews( NULL, HINT_NEWROOT );
	return TRUE;
	}


void CDirstatDoc::buildDriveItems( _In_ CStringArray& rootFolders ) {
	FILETIME t;
	zeroDate( t );
	if ( m_showMyComputer ) {
		EnterCriticalSection( &m_rootItemCriticalSection );
		m_rootItem = std::make_unique<CItemBranch>( IT_MYCOMPUTER, L"My Computer", 0, t, 0, false, true, false );//L"My Computer"
		LeaveCriticalSection( &m_rootItemCriticalSection );

		for ( INT i = 0; i < rootFolders.GetSize( ); i++ ) {
			auto smart_drive = std::make_unique<CItemBranch>( IT_DRIVE, rootFolders[ i ], 0, t, 0, false, true );	
			EnterCriticalSection( &m_rootItemCriticalSection );
			m_rootItem->AddChild( smart_drive.get( ) );
			LeaveCriticalSection( &m_rootItemCriticalSection );
			}
		}
	else {
		auto type = IsDrive( rootFolders[ 0 ] ) ? IT_DRIVE : IT_DIRECTORY;
		EnterCriticalSection( &m_rootItemCriticalSection );
		m_rootItem = std::make_unique<CItemBranch>( type, rootFolders[ 0 ], 0, t, 0, false, true );
		//m_rootItem->UpdateLastChange( );
		LeaveCriticalSection( &m_rootItemCriticalSection );
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

BOOL CDirstatDoc::OnOpenDocument(_In_z_ LPCTSTR lpszPathName) {
	CDocument::OnNewDocument(); // --> DeleteContents()
	TRACE( _T( "Opening new document, path: %s\r\n" ), lpszPathName );
	CString spec = lpszPathName;
	CString folder;
	CStringArray drives;
	DecodeSelection(spec, folder, drives);
	check8Dot3NameCreationAndNotifyUser( );
	CStringArray rootFolders;
	buildRootFolders( drives, folder, rootFolders );

	buildDriveItems( rootFolders );

	EnterCriticalSection( &m_rootItemCriticalSection );
	m_zoomItem = m_rootItem.get( );
	LeaveCriticalSection( &m_rootItemCriticalSection );

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );

	m_searchStartTime = help_QueryPerformanceCounter( );

	BOOL behavedWell = QueryPerformanceFrequency( &m_timerFrequency );
	if ( !behavedWell ) {
		std::wstring a;
		a += ( __FUNCTION__, __LINE__ );
		MessageBox( NULL, TEXT( "QueryPerformanceFrequency failed!!" ), a.c_str( ), MB_OK );
		}
	
	EnterCriticalSection( &m_rootItemCriticalSection );
	SetWorkingItem( m_rootItem.get( ) );
	LeaveCriticalSection( &m_rootItemCriticalSection );
	GetMainFrame( )->FirstUpdateProgress( );
	GetMainFrame( )->MinimizeGraphView( );
	GetMainFrame( )->MinimizeTypeView( );
	UpdateAllViews( NULL, HINT_NEWROOT );
	GetMainFrame( )->FirstUpdateProgress( );
	return true;
	}

void CDirstatDoc::Serialize(_In_ const CArchive& /*ar*/) { }

// Prefix the window title (with percentage or "Scanning")
void CDirstatDoc::SetTitlePrefix( _In_ const CString prefix ) {
	auto docName = prefix + GetTitle( );
	TRACE( _T( "Setting window title to '%s'\r\n" ), docName );
	GetMainFrame( )->UpdateFrameTitleForDocument( docName );
	}


COLORREF CDirstatDoc::GetCushionColor( _In_ PCWSTR ext ) {
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

COLORREF CDirstatDoc::GetZoomColor() const {
	return RGB( 0, 0, 255 );
	}

_Must_inspect_result_ std::vector<SExtensionRecord>* CDirstatDoc::GetExtensionRecords( ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
 	return &m_extensionRecords;
	}

_Success_( return != UINT64_MAX ) _Requires_lock_held_( m_rootItemCriticalSection ) std::uint64_t CDirstatDoc::GetRootSize( ) const {
	ASSERT( m_rootItem != NULL );
	ASSERT( IsRootDone( ) );
	if ( m_rootItem ) {
		return m_rootItem->m_size;
		}
	return UINT64_MAX;
	}

void CDirstatDoc::ForgetItemTree( ) {
	m_zoomItem = NULL;
	m_selectedItem = NULL;
	EnterCriticalSection( &m_rootItemCriticalSection );
	m_rootItem.reset( );
	LeaveCriticalSection( &m_rootItemCriticalSection );
	}

void CDirstatDoc::SortTreeList( ) {
	EnterCriticalSection( &m_rootItemCriticalSection );
	ASSERT( m_rootItem != NULL );
	m_rootItem->SortChildren( );
	LeaveCriticalSection( &m_rootItemCriticalSection );
	auto DirStatView = ( GetMainFrame( )->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
		}
	}

_Requires_lock_held_( m_rootItemCriticalSection ) DOUBLE CDirstatDoc::GetNameLength( ) const {
	return m_rootItem->averageNameLength( );
	}

bool CDirstatDoc::OnWorkFinished( ) {
	TRACE( _T( "Finished walking tree...\r\n" ) );
	m_extensionDataValid = false;
	//TRACE( _T( "Average name length: %f\r\n" ), m_rootItem->averageNameLength( ) );
	GetMainFrame( )->SetProgressPos100( );
	GetMainFrame( )->RestoreTypeView( );

	auto doneTime = help_QueryPerformanceCounter( );
	const DOUBLE AdjustedTimerFrequency = ( DOUBLE( 1 ) ) / DOUBLE( m_timerFrequency.QuadPart );
			
	UpdateAllViews( NULL );//nothing has been done?
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
#ifdef DUMP_MEMUSAGE
	_CrtMemDumpAllObjectsSince( NULL );
#endif
	return true;
	}

bool CDirstatDoc::Work( _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t ticks ) {
	/*
	  This method does some work (walking tree) for ticks ms. 
	  return: true if done or suspended.
	*/
	EnterCriticalSection( &m_rootItemCriticalSection );
	if ( !m_rootItem ) { //Bail out!
		TRACE( _T( "There's no work to do! (m_rootItem == NULL) - What the hell? - This can occur if user clicks cancel in drive select box on first opening.\r\n" ) );
		LeaveCriticalSection( &m_rootItemCriticalSection );
		return true;
		}
	if ( !m_rootItem->IsDone( ) ) {
		//m_rootItem->DoSomeWork( ticks );
		DoSomeWork( m_rootItem.get( ), ticks );
		if ( m_rootItem->IsDone( ) ) {
			LeaveCriticalSection( &m_rootItemCriticalSection );
			return OnWorkFinished( );
			}
		ASSERT( m_workingItem != NULL );
		if ( m_workingItem != NULL ) {
			GetMainFrame( )->SetProgressPos( m_workingItem->GetProgressPos( ) );
			}
		m_rootItem->SortChildren( );//TODO: necessary?
		UpdateAllViews( NULL, HINT_SOMEWORKDONE );
		}
	if ( m_rootItem->IsDone( ) && m_timeTextWritten ) {
		SetWorkingItem( NULL, true );
		//m_rootItem->SortChildren( );
		LeaveCriticalSection( &m_rootItemCriticalSection );
		return true;
		}
	LeaveCriticalSection( &m_rootItemCriticalSection );
	return false;
	}

bool CDirstatDoc::IsDrive( _In_ const CString spec ) const {
	return ( spec.GetLength( ) == 3 && spec[ 1 ] == _T( ':' ) && spec[ 2 ] == _T( '\\' ) );
	}

_Requires_lock_held_( m_rootItemCriticalSection ) bool CDirstatDoc::IsRootDone( ) const {
	return ( ( m_rootItem != NULL ) && m_rootItem->IsDone( ) );
	}

_Requires_lock_held_( m_rootItemCriticalSection ) _Must_inspect_result_ CItemBranch* CDirstatDoc::GetRootItem( ) const {
	return m_rootItem.get( );
	}

_Must_inspect_result_ CItemBranch* CDirstatDoc::GetZoomItem( ) const {
	return m_zoomItem;
	}

_Requires_lock_held_( m_rootItemCriticalSection ) bool CDirstatDoc::IsZoomed( ) const {
	return GetZoomItem( ) != GetRootItem( );
	}

void CDirstatDoc::SetSelection( _In_ const CItemBranch* item, _In_ const bool keepReselectChildStack ) {
	ASSERT( item != NULL );
	if ( ( item == NULL ) || ( m_zoomItem == NULL ) ) {
		return;
		}
	auto newzoom = CItemBranch::FindCommonAncestor( m_zoomItem, item );
	if ( newzoom != NULL ) {
		if ( newzoom != m_zoomItem ) {
			TRACE( _T( "Setting new selection\r\n" ) );
			SetZoomItem( newzoom );
			}
		}
	ASSERT( newzoom != NULL );

	m_selectedItem = const_cast< CItemBranch* >( item );
	GetMainFrame( )->SetSelectionMessageText( );

	if ( !( keepReselectChildStack || ( m_selectedItem == item ) ) ) {
		ClearReselectChildStack( );
		}
	}

_Must_inspect_result_ CItemBranch *CDirstatDoc::GetSelection() const {
	return m_selectedItem;
	}

void CDirstatDoc::SetHighlightExtension(_In_z_ const LPCTSTR ext) {
	if ( m_highlightExtension.CompareNoCase( ext ) != 0 ) {
		m_highlightExtension = ext;
		TRACE( _T( "Highlighting extension %s\r\n" ), m_highlightExtension );
		GetMainFrame( )->SetSelectionMessageText( );
		}
	else {
		TRACE( _T( "NOT highlighting extension: %s (already selected)\r\n" ), ext );
		}
	}

CString CDirstatDoc::GetHighlightExtension( ) const {
	return m_highlightExtension;
	}

void CDirstatDoc::OpenItem(_In_ const CItemBranch* item) {
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
				auto res = ShellExecuteEx( &sei );
				// ShellExecuteEx seems to display its own Messagebox, if failed.
				if ( res != TRUE ) {
					TRACE( _T( "ShellExecuteEx failed! Error: %s\r\n" ), GetShellExecuteError( GetLastError( ) ) );
					}
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
		auto ShellExRes = ShellExecuteWithAssocDialog( *AfxGetMainWnd( ), path );
		if ( ShellExRes < 33 ) {
			 return displayWindowsMsgBoxWithMessage( GetShellExecuteError( ShellExRes ) );
			}
	}
	catch ( CException *pe )
	{
		pe->ReportError( );
		pe->Delete( );
	}
	}

void CDirstatDoc::RebuildExtensionData() {
	/*
	  Assigns colors to all known file types (i.e. `Extensions`)
	*/
	CWaitCursor wc;
	
	m_extensionRecords.clear( );
	m_extensionRecords.reserve( 100000 );
	
	std::map<CString, SExtensionRecord> extensionMap;
	EnterCriticalSection( &m_rootItemCriticalSection );
	m_rootItem->stdRecurseCollectExtensionData( /*m_extensionRecords,*/ extensionMap );
	LeaveCriticalSection( &m_rootItemCriticalSection );

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

void CDirstatDoc::SetWorkingItem( _In_opt_ CItemBranch* item ) {
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

void CDirstatDoc::SetWorkingItem(_In_opt_ CItemBranch* item, _In_ bool hideTiming ) {
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


bool CDirstatDoc::DeletePhysicalItem( _In_ CItemBranch* item, _In_ const bool toTrashBin ) {
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
	ClearReselectChildStack( );
	SetZoomItem( item->GetParent( ) );
	SetSelection( item->GetParent( ) );
	UpdateAllViews( NULL, HINT_SELECTIONCHANGED );
	return true;
	}

void CDirstatDoc::SetZoomItem(_In_ CItemBranch* item) {
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

void CDirstatDoc::PushReselectChild( _In_ CItemBranch* item ) {
	m_reselectChildStack.AddHead( item );
	}

_Must_inspect_result_ CItemBranch* CDirstatDoc::PopReselectChild( ) {
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
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_SELECTPARENT, OnUpdateTreemapSelectparent)
	ON_COMMAND(ID_TREEMAP_SELECTPARENT, OnTreemapSelectparent)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMIN, OnUpdateTreemapZoomin)
	ON_COMMAND(ID_TREEMAP_ZOOMIN, OnTreemapZoomin)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMOUT, OnUpdateTreemapZoomout)
	ON_COMMAND(ID_TREEMAP_ZOOMOUT, OnTreemapZoomout)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_RESELECTCHILD, OnUpdateTreemapReselectchild)
	ON_COMMAND(ID_TREEMAP_RESELECTCHILD, OnTreemapReselectchild)
END_MESSAGE_MAP()

void CDirstatDoc::OnUpdateEditCopy( CCmdUI *pCmdUI ) {
	const auto item = GetSelection( );
	if ( item == NULL ) {
		TRACE( _T( "Whoops! That's a NULL item!\r\n" ) );
		return;
		}
	auto thisItemType = item->GetType( );
	pCmdUI->Enable( DirectoryListHasFocus( ) && item != NULL && thisItemType != IT_MYCOMPUTER && thisItemType != IT_FILESFOLDER /*&& thisItemType != IT_FREESPACE*/ /*&& thisItemType != IT_UNKNOWN*/ );
	}

void CDirstatDoc::OnEditCopy() {
	TRACE( _T( "User chose 'Edit'->'Copy'!\r\n") );
	const auto item = GetSelection( );
	if ( item == NULL ) {
		TRACE( _T( "You tried to copy nothing! What does that even mean?\r\n" ) );
		return;
		}
	ASSERT( item->GetType( ) == IT_DRIVE || item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE );

	GetMainFrame( )->CopyToClipboard( item->GetPath( ) );
	}

void CDirstatDoc::OnUpdateTreemapZoomin( CCmdUI *pCmdUI ) {
	EnterCriticalSection( &m_rootItemCriticalSection );
	pCmdUI->Enable( ( m_rootItem != NULL ) && ( m_rootItem->IsDone( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( ) != GetZoomItem( ) ) );
	LeaveCriticalSection( &m_rootItemCriticalSection );
	}

void CDirstatDoc::OnTreemapZoomin( ) {
	auto p = GetSelection( );
	CItemBranch* z = NULL;
	auto zoomItem = GetZoomItem( );
	while ( p != zoomItem ) {
		z = p;
		p = p->GetParent( );
		}
	if ( z == NULL ) {
		return;
		}
	ASSERT( z != NULL );
	SetZoomItem( z );
	}

void CDirstatDoc::OnUpdateTreemapZoomout( CCmdUI *pCmdUI ) {
	EnterCriticalSection( &m_rootItemCriticalSection );
	pCmdUI->Enable( ( m_rootItem != NULL ) && ( m_rootItem->IsDone( ) ) && ( GetZoomItem( ) != m_rootItem.get( ) ) );
	LeaveCriticalSection( &m_rootItemCriticalSection );
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
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) /*&& ( GetSelection( )->GetType( ) != IT_FREESPACE )*/ /*&& ( GetSelection( )->GetType( ) != IT_UNKNOWN )*/ );
	}

void CDirstatDoc::OnExplorerHere( ) {
	try
	{
		const auto item = GetSelection( );
		if ( item != NULL ) {
			TRACE( _T( "User wants to open Explorer in %s!\r\n" ), item->GetFolderPath( ) );
			if ( item->GetType( ) == IT_MYCOMPUTER ) {
				auto sei = partInitSEI( );
				sei.hwnd = *AfxGetMainWnd( );
				sei.lpVerb = _T( "explore" );
				sei.nShow = SW_SHOWNORMAL;

				CCoTaskMem<LPITEMIDLIST> pidl;
				auto pidlRes = GetPidlOfMyComputer( &pidl );
				if ( FAILED( pidlRes ) ) {
					return displayWindowsMsgBoxWithError( );
					}

				sei.lpIDList = pidl;
				sei.fMask |= SEE_MASK_IDLIST;

				ShellExecuteEx( &sei ); // ShellExecuteEx seems to display its own Messagebox on error.
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
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( )->GetType( ) != IT_MYCOMPUTER ) );
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
