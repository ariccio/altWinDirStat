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


//encourage inter-procedural optimization (and class-heirarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"


#include "dirstatview.h"
#include "globalhelpers.h"
#include "windirstat.h"
#include "mainframe.h"

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

	_At_( i, _Pre_satisfies_( i == 0 ) )
	std::vector<std::wstring> addTokens( _In_ const std::wstring s, _Inout_ rsize_t& i, _In_ const wchar_t EncodingSeparator ) {
		std::vector<std::wstring> sa;
		while ( i < s.length( ) ) {
			CString token;
			//std::wstring token_;
			while ( i < s.length( ) && s.at( i ) != EncodingSeparator ) {
				token += s.at( i++ );
				//token_ += s.at( i++ );
				}
		
			TRACE( _T( "Token before trimming: %s\r\n" ), token );
			token.TrimLeft( );
			token.TrimRight( );
			TRACE( _T( "Token after trimming : %s\r\n" ), token );
			ASSERT( !token.IsEmpty( ) );
			sa.emplace_back( std::wstring( token.GetString( ) ) );

			if ( i < s.length( ) ) {
				i++;
				}
			}
		return sa;
		}


	void DecodeSingleSelection( _In_ std::wstring f, _Inout_ std::vector<std::wstring>& drives, _Out_ std::wstring& folder ) {
		if ( f.length( ) == 2 && f[ 1 ] == _T( ':' ) ) {
			ASSERT( ( f.length( ) == 2 ) && ( f[ 1 ] == _T( ':' ) ) );
			f += _T( "\\" );
			
			const auto strcmp_path = f.compare( 0, 4, L"\\\\?\\", 0, 4 );
			if ( strcmp_path != 0 ) {
				auto fixedPath = L"\\\\?\\" + f;
				TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
				f = fixedPath;
				}

			TRACE( _T( "Inserting drive: %s\r\n" ), f.c_str( ) );
			drives.emplace_back( f );
			folder = L"";//there is no folder.
			}
		else {
			// Remove trailing backslash, if any and not drive-root.
			if ( f.length( ) > 0 && f.back( ) == _T( '\\' ) && ( f.length( ) != 3 || f[ 1 ] != _T( ':' ) ) ) {
				f = f.substr( 0, f.length( ) - 1 );
				}
			TRACE( _T( "Whoops! %s is not a drive, it's a folder!\r\n" ), f.c_str( ) );
			folder = f;
			}
		}

	std::vector<std::wstring> DecodeSelection( _In_ PCWSTR s, _Inout_ std::wstring& folder ) {
		
		TRACE( _T( "decoding selection: %s\r\n" ), s );
		std::vector<std::wstring> drives;
		// s is either something like "C:\programme" or something like "C:|D:|E:".
		rsize_t i = 0;
		const auto sa = addTokens( std::wstring( s ), i, _T( '|' ) );// `|` is the encoding separator, which is not allowed in file names.

		ASSERT( sa.size( ) > 0 );
		for ( size_t j = 0; j < sa.size( ); j++ ) {
			DecodeSingleSelection( sa.at( j ), drives, folder );
			}
		return drives;
		}

	rsize_t GetDefaultPaletteAsArray( _Out_ _Pre_writable_size_( 13 ) _Post_readable_size_( return ) COLORREF( &colorArray )[ 13 ] ) {
		rsize_t i = 0;
		const COLORREF defaultColors[ ] = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 150, 150, 255 ), RGB( 255, 150, 150 ), RGB( 150, 255, 150 ), RGB( 150, 255, 255 ), RGB( 255, 150, 255 ), RGB( 255, 255, 150 ), RGB( 255, 255, 255 ) };
		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( i = 0; i < 13; ++i ) {
			colorArray[ i ] = CColorSpace::MakeBrightColor( defaultColors[ i ], PALETTE_BRIGHTNESS );
			}
		return i;
		}
	void AddFileExtensionData( _Out_ _Pre_satisfies_( ( extensionRecords._Mylast - extensionRecords._Myfirst ) == 0 ) std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) {
		extensionRecords.reserve( extensionMap.size( ) + 1 );
		for ( auto& anExt : extensionMap ) {
			extensionRecords.emplace_back( std::move( anExt.second ) );
			}
		}

	}

CDirstatDoc* _theDocument;

CDirstatDoc* GetDocument() {
	ASSERT( _theDocument != NULL );
	return _theDocument;
	}

/*


	IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)

	--becomes--


	CObject* PASCAL CDirstatDoc::CreateObject() \
		{ return new CDirstatDoc; } \

	AFX_COMDAT const CRuntimeClass CDirstatDoc::classCDirstatDoc = { \
																	"CDirstatDoc", \
																	sizeof(class CDirstatDoc), \
																	0xFFFF, \
																	CDirstatDoc::CreateObject, \
																	((CRuntimeClass*)(&CDocument::classCDocument)), \
																	NULL, \
																	NULL \
																	}; \
			
	CRuntimeClass* CDirstatDoc::GetRuntimeClass() const \
		{ return ((CRuntimeClass*)(&CDirstatDoc::classCDirstatDoc)); }

*/

IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)

_Pre_satisfies_( _theDocument == NULL ) _Post_satisfies_( _theDocument == this )
CDirstatDoc::CDirstatDoc( ) : m_workingItem( NULL ), m_selectedItem( NULL ), m_extensionDataValid( false ), m_timeTextWritten( false ), m_showMyComputer( true ), m_searchTime( DBL_MAX ), m_iterations( 0 ), m_compressed_file_timing( -1 ), m_frameptr( GetMainFrame( ) ) {
	ASSERT( _theDocument == NULL );
	_theDocument               = this;
	TRACE( _T( "_theDocument has been set to %p\r\n" ), _theDocument );
	m_searchStartTime.QuadPart = 0;
	//m_timerFrequency.QuadPart  = 0;
	}

CDirstatDoc::~CDirstatDoc( ) {
	const rsize_t iter_char_count = 128;

	wchar_t iter_char[ iter_char_count ];
	const auto res = _snwprintf_s( iter_char, iter_char_count, _TRUNCATE, L"WDS: ~CDirstatDoc %u iterations\r\n", unsigned( m_iterations ) );
	ASSERT( res >= 0 );
	ASSERT( res < iter_char_count );
	if ( res >= 0 ) {
		if ( res < iter_char_count ) {
			OutputDebugString( iter_char );
			}
		}
	_theDocument = { NULL };
	m_rootItem.reset( );
	}

void CDirstatDoc::DeleteContents( ) {
	m_rootItem.reset( );
	m_selectedItem = { NULL };
	m_workingItem  = { NULL };
	m_timeTextWritten = false;
	}

BOOL CDirstatDoc::OnNewDocument( ) {
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	TRACE( _T( "New document...\r\n" ) );
	UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return TRUE;
	}

void CDirstatDoc::buildDriveItems( _In_ const std::vector<std::wstring>& rootFolders ) {
	FILETIME t;
	zeroDate( t );
	if ( m_showMyComputer ) {
		//BUGBUG: TODO:
		for ( size_t i = 0; i < rootFolders.size( ); i++ ) {
			}
		}
	else {
		const auto new_name_length = rootFolders.at( 0 ).length( );
		ASSERT( new_name_length < UINT16_MAX );
		//_Null_terminated_ _Field_size_( new_name_length + 1u ) PWSTR new_name_ptr = new wchar_t[ new_name_length + 1u ];
		//const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1u ), rootFolders.at( 0 ).c_str( ) );
		//if ( cpy_res != 0 ) {
		//	std::terminate( );
		//	}
		//ASSERT( wcslen( new_name_ptr ) == new_name_length );
		//ASSERT( wcscmp( new_name_ptr, rootFolders.at( 0 ).c_str( ) ) == 0 );

		PWSTR new_name_ptr = nullptr;
		const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, rootFolders.at( 0 ) );
		if ( !SUCCEEDED( copy_res ) ) {
			_CrtDbgBreak( );
			}

		//                                          IT_DIRECTORY
		m_rootItem = std::make_unique<CItemBranch>( 0, t, 0, false, reinterpret_cast<CItemBranch*>( NULL ), new_name_ptr, static_cast<std::uint16_t>( new_name_length ) );
		//m_rootItem->m_parent = { NULL };
		}
	}

std::vector<std::wstring> CDirstatDoc::buildRootFolders( _In_ const std::vector<std::wstring>& drives, _In_ std::wstring& folder ) {
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
	m_frameptr = GetMainFrame( );
	++m_iterations;
	GetApp( )->m_mountPoints.Initialize( );
	TRACE( _T( "Opening new document, path: %s\r\n" ), pszPathName );
	VERIFY( CDocument::OnNewDocument( ) ); // --> DeleteContents()
	const std::wstring spec( pszPathName );
	std::wstring folder;
	const auto drives( DecodeSelection( pszPathName, folder ) );
	check8Dot3NameCreationAndNotifyUser( );

#ifdef PERF_DEBUG_SLEEP
	displayWindowsMsgBoxWithMessage( _T( "PERF_DEBUG_SLEEP ENABLED! this is meant for debugging!" ) );
#endif

	const auto rootFolders_( buildRootFolders( drives, folder ) );
	buildDriveItems( rootFolders_ );

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );

	m_searchStartTime = help_QueryPerformanceCounter( );	
	m_workingItem = m_rootItem.get( );
	ASSERT( GetMainFrame( ) == m_frameptr );

	m_frameptr->m_wndSplitter.SetSplitterPos( 1.0 );
	m_frameptr->m_wndSubSplitter.SetSplitterPos( 1.0 );
	
	
	UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return true;
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

void CDirstatDoc::ForgetItemTree( ) {
	m_selectedItem = { NULL };
	m_workingItem  = { NULL };
	m_rootItem.reset( );
	}

void CDirstatDoc::SortTreeList( ) {
	ASSERT( m_rootItem != NULL );
	m_rootItem->SortChildren( );
	ASSERT( m_frameptr == GetMainFrame( ) );
	const auto DirStatView = ( m_frameptr->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
		}
	}

bool CDirstatDoc::OnWorkFinished( ) {
	TRACE( _T( "Finished walking tree...\r\n" ) );
#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif
	m_extensionDataValid = false;
	ASSERT( m_frameptr == GetMainFrame( ) );
	m_frameptr->RestoreTypeView( );

	const auto doneTime = help_QueryPerformanceCounter( );
	const DOUBLE AdjustedTimerFrequency = ( static_cast<DOUBLE>( 1 ) ) / static_cast<DOUBLE>( help_QueryPerformanceFrequency( ).QuadPart );
			
	UpdateAllViews( NULL );
	if ( doneTime.QuadPart != NULL ) {
		m_searchTime = ( doneTime.QuadPart - m_searchStartTime.QuadPart ) * AdjustedTimerFrequency;
		}
	else {
		m_searchTime = -2;//Negative (that's not -1) informs WriteTimeToStatusBar that there was a problem.
		}
	ASSERT( m_frameptr == GetMainFrame( ) );
	m_frameptr->RestoreGraphView( );
	//Complete?
	SortTreeList();
	m_timeTextWritten = true;
	TRACE( _T( "All work finished!\r\n" ) );
#ifdef DUMP_MEMUSAGE
	//_CrtMemDumpAllObjectsSince( NULL );
#endif
	return true;
	}

bool CDirstatDoc::Work( ) {
	//return: true if done or suspended.

	if ( ( !m_rootItem ) || m_timeTextWritten ) {
		ASSERT( m_workingItem == NULL );
		return true;
		}
	if ( !m_rootItem->m_attr.m_done ) {
		WTL::CWaitCursor wc;
		auto path( m_rootItem->GetPath( ) );
		const auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
		if ( strcmp_path != 0 ) {
			path = L"\\\\?\\" + path;
			TRACE( _T( "path fixed as: %s\r\n" ), path.c_str( ) );
			}
		const auto thisApp = GetApp( );
		m_compressed_file_timing = DoSomeWorkShim( m_rootItem.get( ), std::move( path ), thisApp, true );
		ASSERT( m_rootItem->m_attr.m_done );
		
		//cache the size of root item
		m_rootItem->refresh_sizeCache( );

		//SetWorkingItem( NULL );
		m_workingItem = NULL;
		const auto res = OnWorkFinished( );
		m_rootItem->AddChildren( );
		return res;
		}
	ASSERT( m_workingItem != NULL );
	return false;
	}

bool CDirstatDoc::IsRootDone( ) const {
	const auto retVal = ( m_rootItem && m_rootItem->m_attr.m_done );
	return retVal;
	}

void CDirstatDoc::SetSelection( _In_ const CItemBranch& item ) {
	m_selectedItem = const_cast< CItemBranch* >( &item );
	ASSERT( m_frameptr == GetMainFrame( ) );
	m_frameptr->SetSelectionMessageText( );
	}

void CDirstatDoc::SetHighlightExtension( _In_ const std::wstring ext ) {
	TRACE( _T( "Highlighting extension %s; previously highlighted: %s\r\n" ), ext.c_str( ), m_highlightExtension.c_str( ) );
	m_highlightExtension = std::move( ext );
	ASSERT( m_frameptr == GetMainFrame( ) );
	m_frameptr->SetSelectionMessageText( );
	}

//_Pre_satisfies_( item.m_type == IT_FILE )
_Pre_satisfies_( item.m_children._Myptr == nullptr )
void CDirstatDoc::OpenItem( _In_ const CItemBranch& item ) {
	WTL::CWaitCursor wc;
	std::wstring path;
	//if ( item.m_type == IT_FILE ) {
	if ( item.m_children == nullptr ) {
		path = item.GetPath( ).c_str( );
		}
	const auto doesFileExist = PathFileExistsW( path.c_str( ) );
	if ( !doesFileExist ) {
		TRACE( _T( "Path (%s) is invalid!\r\n" ), path.c_str( ) );
		std::wstring pathMsg( L"Path (" + path + L") is invalid!\r\n");
		AfxMessageBox( pathMsg.c_str( ) );
		displayWindowsMsgBoxWithError( );
		return;
		}

	const auto ShellExRes = ShellExecuteWithAssocDialog( *AfxGetMainWnd( ), std::move( path ) );
	if ( ShellExRes < 33 ) {
		return displayWindowsMsgBoxWithError( );
		}
	}

void CDirstatDoc::RebuildExtensionData() {
	//Assigns colors to all known file types (i.e. `Extensions`)

	WTL::CWaitCursor wc;
	
	m_extensionRecords.clear( );
	
	std::unordered_map<std::wstring, SExtensionRecord> extensionMap;

	auto rootTemp = m_rootItem.get( );
	
	extensionMap.reserve( rootTemp->files_recurse( ) );

	rootTemp->stdRecurseCollectExtensionData( extensionMap );
	AddFileExtensionData( m_extensionRecords, extensionMap );

	stdSetExtensionColors( m_extensionRecords );
	std::sort( m_extensionRecords.begin( ), m_extensionRecords.end( ), s_compareSExtensionRecordByBytes( ) );

	m_extensionRecords.shrink_to_fit( );
	m_extensionDataValid = true;
	}

void CDirstatDoc::stdSetExtensionColors( _Inout_ std::vector<SExtensionRecord>& extensionsToSet ) {
	/*
	  New, much faster, method of assigning colors to extensions. For every element in reverseExtensionMap, assigns a color to the `color` field of an element at key (no longer accurate description). The color assigned is chosen by rotating through a default palette.
	*/
	//const auto colorVector = GetDefaultPaletteAsVector( );

	COLORREF colorArray[ 13 ];

	const auto sizeOfArray = GetDefaultPaletteAsArray( colorArray );


	std::vector<COLORREF>::size_type processed = 0;

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto& anExtension : extensionsToSet ) {
		//auto test = colorVector.at( processed % ( colorVector.size( ) ) );
		auto test = colorArray[ processed % ( sizeOfArray ) ];
		++processed;
		if ( processed < ( sizeOfArray ) ) {//TODO colors.GetSize( )-> colorsSize
			test = colorArray[ processed ];
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

void CDirstatDoc::VectorExtensionRecordsToMap( ) {
	auto records = GetExtensionRecords( );
	if ( records != NULL ) {
		m_colorMap.reserve( records->size( ) );
		for ( const auto& aRecord : ( *records ) ) {
			m_colorMap[ aRecord.ext ] = aRecord.color;
			}
		}
	}





BEGIN_MESSAGE_MAP(CDirstatDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
END_MESSAGE_MAP()

void CDirstatDoc::OnUpdateEditCopy( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( /*( GetMainFrame( )->m_logicalFocus == focus::LF_DIRECTORYLIST ) &&*/ m_selectedItem != NULL );
	}

void CDirstatDoc::OnEditCopy( ) {
	TRACE( _T( "User chose 'Edit'->'Copy'!\r\n") );
	if ( m_selectedItem == NULL ) {
		TRACE( _T( "You tried to copy nothing! What does that even mean?\r\n" ) );
		return;
		}

	auto itemPath = m_selectedItem->GetPath( );
	
	if ( itemPath.substr( 0, 4 ).compare( L"\\\\?\\" ) == 0 ) {
		itemPath = itemPath.substr( 4, itemPath.length( ) - 4 );
		}
	
	itemPath.resize( itemPath.length( ) + MAX_PATH );
	ASSERT( m_frameptr == GetMainFrame( ) );
	m_frameptr->CopyToClipboard( std::move( itemPath ) );
	//itemPath.ReleaseBuffer( );
	}


// CDirstatDoc Diagnostics
#ifdef _DEBUG
void CDirstatDoc::AssertValid( ) const {
	CDocument::AssertValid();
	}

void CDirstatDoc::Dump( CDumpContext& dc ) const {
	TRACE( _T( "CDirstatDoc::Dump\r\n" ) );
	CDocument::Dump(dc);
	}
#endif //_DEBUG

// $Log$
// Revision 1.14  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.13  2004/11/07 10:17:37  bseifert
// Bugfix: Recursive UDCs must not follow junction points.