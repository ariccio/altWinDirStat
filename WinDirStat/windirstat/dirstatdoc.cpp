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
			std::wstring token;
			while ( i < s.length( ) && s.at( i ) != EncodingSeparator ) {
				token += s.at( i++ );
				}
			const std::wstring test( std::move( token ) );
			TRACE( _T( "string to trim: %s\r\n" ), test.c_str( ) );
			const auto idx_past_last_leading_space = test.find_first_not_of( L' ' );

			const std::wstring test_leading_spaces_trimmed( ( idx_past_last_leading_space == std::wstring::npos ) ? ( test.c_str( ) + test.length( ) - 1 ) : ( test.c_str( ) + idx_past_last_leading_space ) );

			TRACE( _T( "string starting at (one past the) last leading space: %s\r\n" ), test_leading_spaces_trimmed.c_str( ) );

			const auto idx_past_last_trailing_space = test_leading_spaces_trimmed.find_last_not_of( L' ' );

			const std::wstring test_leading_and_trailing_spaces_trimmed( ( idx_past_last_trailing_space != std::wstring::npos ) ? test_leading_spaces_trimmed.substr( 0, ( idx_past_last_trailing_space + 1 ) ) : test_leading_spaces_trimmed );

			TRACE( _T( "string with trailing spaces trimmed: %s\r\n" ), test_leading_and_trailing_spaces_trimmed.c_str( ) );

			ASSERT( !test_leading_and_trailing_spaces_trimmed.empty( ) );

			//const std::wstring w_token( test_leading_and_trailing_spaces_trimmed );
			//ASSERT( !w_token.empty( ) );

			sa.emplace_back( std::move( test_leading_and_trailing_spaces_trimmed ) );

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

	_Success_( return > 32 ) INT_PTR ShellExecuteWithAssocDialog( _In_ const HWND hwnd, _In_ std::wstring filename ) {
		WTL::CWaitCursor wc;
		auto u = reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, NULL, filename.c_str( ), NULL, NULL, SW_SHOWNORMAL ) );
		if ( u == SE_ERR_NOASSOC ) {
			// Q192352
			const rsize_t dir_buf_size = MAX_PATH;
			_Null_terminated_ wchar_t dir_buf[ MAX_PATH ] = { 0 };
			std::wstring parameters_filename( L"shell32.dll,OpenAs_RunDLL " + std::move( filename ) );

			//-- Get the system directory so that we know where Rundll32.exe resides.
			const auto sys_dir_res = GetSystemDirectoryW( dir_buf, dir_buf_size );
			if ( sys_dir_res == 0 ) {
				displayWindowsMsgBoxWithError( );
				std::terminate( );
				//shut analyze up, thinks we continue execution!
				return -1;
				}
			if ( sys_dir_res < dir_buf_size ) {
				return reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, _T( "open" ), _T( "RUNDLL32.EXE" ), parameters_filename.c_str( ), dir_buf, SW_SHOWNORMAL ) );
				}
			ASSERT( sys_dir_res > dir_buf_size );
			if ( sys_dir_res > dir_buf_size ) {
				const auto str_ptr = std::make_unique<_Null_terminated_ wchar_t[ ]>( sys_dir_res );
				const auto sys_dir_res_2 = GetSystemDirectoryW( str_ptr.get( ), sys_dir_res );
				if ( ( sys_dir_res_2 != 0 ) && ( sys_dir_res_2 < sys_dir_res ) ) {
					return reinterpret_cast< INT_PTR >( ShellExecuteW( hwnd, _T( "open" ), _T( "RUNDLL32.EXE" ), parameters_filename.c_str( ), str_ptr.get( ), SW_SHOWNORMAL ) );
					}
				displayWindowsMsgBoxWithMessage( L"Something is extremely wrong (GetSystemDirectoryW)!!" );
				std::terminate( );
				}
			}
		return u;
		}

	void check8Dot3NameCreationAndNotifyUser( ) {
		HKEY keyHandle = { NULL };

		const auto res_1 = RegOpenKeyExW( HKEY_LOCAL_MACHINE, _T( "SYSTEM\\CurrentControlSet\\Control\\FileSystem" ), NULL, KEY_READ, &keyHandle );

		if ( res_1 != ERROR_SUCCESS ) {
			TRACE( _T( "key not found!\r\n" ) );
			return;
			}
		DWORD valueType = 0;
		static_assert( sizeof( BYTE ) == 1, "bad BYTE size!" );
		BYTE data[ 4 ];
		static_assert( sizeof( data ) == sizeof( REG_DWORD ), "bad size!" );
			
		DWORD bufferSize = sizeof( data );
			
		const auto res_2 = RegQueryValueExW( keyHandle, _T( "NtfsDisable8dot3NameCreation" ), NULL, &valueType, &data[0], &bufferSize );

		if ( res_2 != ERROR_SUCCESS ) {
			if ( res_2 == ERROR_MORE_DATA ) {
				return;
				}
			else if ( res_2 == ERROR_FILE_NOT_FOUND) {
				return;
				}
			return;
			}
		const DWORD value = data[ 0 ];
		/*
			0 = NTFS creates short file names. This setting enables applications that cannot process long file names and computers that use differentcode pages to find the files.
			1 = NTFS does not create short file names. Although this setting increases file performance, applications that cannot process long file names, and computers that use different code pages, might not be able to find the files.
			2 = NTFS sets the 8.3 naming convention creation on a per volume basis.
			3 = NTFS disables 8dot3 name creation on all volumes except the system volume.
		*/
		if ( value == 0 ) {
			std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_all_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
			WTL::AtlMessageBox( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
			}

		if ( value == 2 ) {
			std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_per_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
			WTL::AtlMessageBox( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
			}

		if ( value == 3 ) {
			std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_sys_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
			WTL::AtlMessageBox( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
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

	_Null_terminated_ wchar_t iter_char[ iter_char_count ];
	const auto res = _snwprintf_s( iter_char, iter_char_count, _TRUNCATE, L"WDS: ~CDirstatDoc %u iterations\r\n", unsigned( m_iterations ) );
	ASSERT( res >= 0 );
	ASSERT( res < iter_char_count );
	if ( res >= 0 ) {
		if ( res < iter_char_count ) {
			OutputDebugStringW( iter_char );
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
	if ( m_frameptr != NULL ) {
		const auto DirstatView = m_frameptr->GetDirstatView( );
		if ( DirstatView != NULL ) {
			if ( DirstatView->m_treeListControl.m_pDocument != this ) {
				DirstatView->m_treeListControl.m_pDocument = this;
				}
			}
		}
	TRACE( _T( "New document...\r\n" ) );
	UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return TRUE;
	}

void CDirstatDoc::buildDriveItems( _In_ const std::vector<std::wstring>& rootFolders ) {
	FILETIME t;
	//zeroDate( t );
	memset_zero_struct( t );
	const auto new_name_length = rootFolders.at( 0 ).length( );
	ASSERT( new_name_length < UINT16_MAX );

	PWSTR new_name_ptr = nullptr;
	const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, rootFolders.at( 0 ) );
	if ( !SUCCEEDED( copy_res ) ) {
		displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (CDirstatDoc::buildDriveItems)(aborting!)" );
		displayWindowsMsgBoxWithMessage( rootFolders.at( 0 ) );
		}

	//                                          IT_DIRECTORY
	m_rootItem = std::make_unique<CItemBranch>( 0, t, 0, false, reinterpret_cast<CItemBranch*>( NULL ), new_name_ptr, static_cast<std::uint16_t>( new_name_length ) );
	//m_rootItem->m_parent = { NULL };

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
	if ( m_frameptr != NULL ) {
		const auto DirstatView = m_frameptr->GetDirstatView( );
		if ( DirstatView != NULL ) {
			if ( DirstatView->m_treeListControl.m_pDocument != this ) {
				DirstatView->m_treeListControl.m_pDocument = this;
				}
			}
		}

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
	
	ASSERT( m_frameptr == GetMainFrame( ) );
	const auto DirStatView = ( m_frameptr->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		m_rootItem->SortChildren( &( DirStatView->m_treeListControl ) );
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
	if ( doneTime.QuadPart != 0 ) {
		m_searchTime = ( doneTime.QuadPart - m_searchStartTime.QuadPart ) * AdjustedTimerFrequency;
		}
	else {
		ASSERT( doneTime.QuadPart != 0 );
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
		const auto DirStatView = ( m_frameptr->GetDirstatView( ) );
		ASSERT( DirStatView != NULL );
		if ( DirStatView == NULL ) {
			displayWindowsMsgBoxWithMessage( L"DirStatView is NULL!!! this should never happen!!" );
			std::terminate( );
			}
		m_rootItem->AddChildren( &( DirStatView->m_treeListControl ) );
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
		if ( processed < ( sizeOfArray ) ) {
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
		TRACE( _T( "%s: (Bytes: %I64u), (Color: %lu), (Files: %I32x)\r\n" ), a.ext.c_str( ), std::uint64_t( a.bytes ), a.color, a.files );
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
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &( CDirstatDoc::OnUpdateEditCopy ) )
	ON_COMMAND(ID_EDIT_COPY, &( CDirstatDoc::OnEditCopy ) )
END_MESSAGE_MAP()

void CDirstatDoc::OnUpdateEditCopy( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( m_selectedItem != NULL );
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