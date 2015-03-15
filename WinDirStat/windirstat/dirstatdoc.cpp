// dirstatdoc.cpp: Implementation of CDirstatDoc
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_DIRSTAT_DOC_CPP
#define WDS_DIRSTAT_DOC_CPP

WDS_FILE_INCLUDE_MESSAGE

#include "dirstatdoc.h"
#include "datastructures.h"

//encourage inter-procedural optimization (and class-hierarchy analysis!)
//#include "ownerdrawnlistcontrol.h"
//#include "TreeListControl.h"
#include "typeview.h"

#include "dirstatview.h"

#include "globalhelpers.h"

#include "windirstat.h" //for m_appptr; this header ALSO `#include "mountpoints.h"`s, to prevent the need for PIMPL in CDirstatApp.

#include "mainframe.h"

#include "directory_enumeration.h"

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

	rsize_t GetDefaultPaletteAsArray( _Out_ _Pre_writable_size_( 13 ) _Post_readable_size_( return ) COLORREF( &colorArray )[ 13 ] ) {
		rsize_t i = 0;
		const COLORREF defaultColors[ ] = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 150, 150, 255 ), RGB( 255, 150, 150 ), RGB( 150, 255, 150 ), RGB( 150, 255, 255 ), RGB( 255, 150, 255 ), RGB( 255, 255, 150 ), RGB( 255, 255, 255 ) };
		TRACE( _T( "\r\n\r\nGenerating brizght colors from default colors......\r\n" ) );
		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( i = 0; i < 13; ++i ) {
#ifdef COLOR_DEBUGGING
			TRACE( _T( "default color, R: %u, G: %u, B: %u\r\n" ), static_cast<unsigned>( GetRValue( defaultColors[ i ] ) ), static_cast<unsigned>( GetGValue( defaultColors[ i ] ) ), static_cast<unsigned>( GetBValue( defaultColors[ i ] ) ) );
#endif
			colorArray[ i ] = CColorSpace::MakeBrightColor( defaultColors[ i ], PALETTE_BRIGHTNESS );
#ifdef COLOR_DEBUGGING

			TRACE( _T( "Bright  color, R: %u, G: %u, B: %u\r\n\r\n" ), static_cast<unsigned>( GetRValue( colorArray[ i ] ) ), static_cast<unsigned>( GetGValue( colorArray[ i ] ) ), static_cast<unsigned>( GetBValue( colorArray[ i ] ) ) );
#endif
			}
		TRACE( _T( ".....done!\r\n\r\n" ) );
		return i;
		}
	void AddFileExtensionData( _Out_ _Pre_satisfies_( ( extensionRecords._Mylast - extensionRecords._Myfirst ) == 0 ) std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::unordered_map<std::wstring, minimal_SExtensionRecord>& extensionMap ) {
		extensionRecords.reserve( extensionMap.size( ) + 1 );
		for ( auto& anExt : extensionMap ) {
			extensionRecords.emplace_back( std::move( anExt.second.files ), std::move( anExt.second.bytes ), std::move( anExt.first ) );
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

	void SetExtensionColors( _Inout_ std::vector<SExtensionRecord>& extensionsToSet ) {
		/*
		  New, much faster, method of assigning colors to extensions. For every element in reverseExtensionMap, assigns a color to the `color` field of an element at key (no longer accurate description). The color assigned is chosen by rotating through a default palette.
		*/
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

	//Yes, used
	struct s_compareSExtensionRecordByBytes final {
		bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const { return ( lhs.bytes < rhs.bytes ); }
		};


	void rebuild_extension_data( _Out_ std::vector<SExtensionRecord>& extension_records, _In_ const CTreeListItem* const root_item ) {
		std::unordered_map<std::wstring, minimal_SExtensionRecord> extensionMap;
		extensionMap.reserve( root_item->files_recurse( ) );

		root_item->stdRecurseCollectExtensionData( extensionMap );
		AddFileExtensionData( extension_records, extensionMap );
		SetExtensionColors( extension_records );
		std::sort( extension_records.begin( ), extension_records.end( ), s_compareSExtensionRecordByBytes( ) );

		extension_records.shrink_to_fit( );

		}

	void convert_vector_of_extension_records_to_map( _In_ const std::vector<SExtensionRecord>* const records, _Inout_ std::unordered_map<std::wstring, COLORREF>& color_map ) {
		if ( records != NULL ) {
			color_map.reserve( records->size( ) );
			for ( const auto& aRecord : ( *records ) ) {
				color_map[ aRecord.ext ] = aRecord.color;
				}
			}
		}

	}

CDirstatDoc* _theDocument;

//evil global function!
CDirstatDoc* GetDocument( ) {
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
CDirstatDoc::CDirstatDoc( ) : m_selectedItem( NULL ), m_extensionDataValid( false ), m_timeTextWritten( false ), m_searchTime( DBL_MAX ), m_compressed_file_timing( -1 ), m_frameptr( GetMainFrame( ) ), m_appptr( GetApp( ) ) {
	ASSERT( _theDocument == NULL );
	_theDocument               = this;
	TRACE( _T( "_theDocument has been set to %p\r\n" ), _theDocument );
	m_searchStartTime.QuadPart = 0;
	//m_timerFrequency.QuadPart  = 0;
	}

CDirstatDoc::~CDirstatDoc( ) {
	_theDocument = { NULL };
	}

void CDirstatDoc::DeleteContents( ) {
	m_selectedItem = { NULL };
	m_timeTextWritten = false;
	m_rootItem.reset( );
	m_name_pool.reset( nullptr );
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
	CDocument::UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return TRUE;
	}

void CDirstatDoc::buildDriveItems( _In_z_ PCWSTR const pszPathName ) {
	const std::wstring root_folder( pszPathName );
	FILETIME t;
	//zeroDate( t );
	memset_zero_struct( t );
	const auto new_name_length = root_folder.length( );
	ASSERT( new_name_length < UINT16_MAX );

	m_rootItem.reset( );

	//TODO: BUGBUG: for some reason, we need +2u here! ASSERT fails if we don't!
	m_name_pool.reset( new Children_String_Heap_Manager( new_name_length + 2u ) );

	PWSTR new_name_ptr = nullptr;

	const HRESULT copy_res = m_name_pool->m_buffer_impl->copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), root_folder );

	if ( !SUCCEEDED( copy_res ) ) {
		displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (CDirstatDoc::buildDriveItems)(aborting!)" );
		displayWindowsMsgBoxWithMessage( root_folder );
		}

	//                                          IT_DIRECTORY
	m_rootItem = std::make_unique<CTreeListItem>( UINT64_ERROR, t, 0, false, reinterpret_cast<CTreeListItem*>( NULL ), new_name_ptr, static_cast<std::uint16_t>( new_name_length ) );
	//m_rootItem->m_parent = { NULL };

	}

BOOL CDirstatDoc::OnOpenDocument( _In_z_ PCWSTR const pszPathName ) {
	m_frameptr = GetMainFrame( );
	//++m_iterations;
	m_appptr->m_mountPoints.Initialize( );
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
	
	//TODO: should NO LONGER be relevant.
	//check8Dot3NameCreationAndNotifyUser( );

	//const auto rootFolders_( folder );
	buildDriveItems( pszPathName );

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );

	m_searchStartTime = help_QueryPerformanceCounter( );	
	//m_workingItem = m_rootItem.get( );

	m_frameptr->m_wndSplitter.SetSplitterPos( 1.0 );
	m_frameptr->m_wndSubSplitter.SetSplitterPos( 1.0 );
	
	
	CDocument::UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return true;
	}

COLORREF CDirstatDoc::GetCushionColor( _In_z_ PCWSTR const ext ) {
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
	if ( m_colorMap.empty( ) ) {
		//VectorExtensionRecordsToMap( );
		convert_vector_of_extension_records_to_map( GetExtensionRecords( ), m_colorMap );
		}
		
	if ( m_colorMap.count( ext ) > 0 ) {
		return m_colorMap.at( ext );
		}
	TRACE( _T( "Extension %s not in colorMap!\r\n" ), ext );
	RebuildExtensionData( );
	//VectorExtensionRecordsToMap( );
	convert_vector_of_extension_records_to_map( GetExtensionRecords( ), m_colorMap );
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

void CDirstatDoc::SortTreeList( ) {
	ASSERT( m_rootItem != NULL );
	
	const auto DirStatView = ( m_frameptr->GetDirstatView( ) );
	if ( DirStatView != NULL ) {
		m_rootItem->SortChildren( &( DirStatView->m_treeListControl ) );
		DirStatView->m_treeListControl.Sort( );//awkward, roundabout way of sorting. TOTALLY breaks encapsulation. Deal with it.
		}
	}

bool CDirstatDoc::OnWorkFinished( ) {
	TRACE( _T( "Finished walking tree...\r\n" ) );
	m_extensionDataValid = false;

	m_frameptr->RestoreTypeView( );

	const auto doneTime = help_QueryPerformanceCounter( );
	const DOUBLE AdjustedTimerFrequency = ( static_cast<DOUBLE>( 1 ) ) / static_cast<DOUBLE>( help_QueryPerformanceFrequency( ).QuadPart );
			
	CDocument::UpdateAllViews( NULL );
	if ( doneTime.QuadPart != 0 ) {
		m_searchTime = ( doneTime.QuadPart - m_searchStartTime.QuadPart ) * AdjustedTimerFrequency;
		}
	else {
		ASSERT( doneTime.QuadPart != 0 );
		m_searchTime = -2;//Negative (that's not -1) informs WriteTimeToStatusBar that there was a problem.
		}

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
		const auto thisApp = m_appptr;
		m_compressed_file_timing = DoSomeWorkShim( m_rootItem.get( ), std::move( path ), thisApp, true );
		ASSERT( m_rootItem->m_attr.m_done );
		
		//cache the size of root item
		m_rootItem->refresh_sizeCache( );

		const auto res = OnWorkFinished( );
		const auto DirStatView = ( m_frameptr->GetDirstatView( ) );
		ASSERT( DirStatView != NULL );
		if ( DirStatView == NULL ) {
			displayWindowsMsgBoxWithMessage( L"DirStatView is NULL!!! this should never happen!!" );
			std::terminate( );
			return false;//so analyze understands.
			}
		
		m_rootItem->AddChildren( &( DirStatView->m_treeListControl ) );
		return res;
		}
	return false;
	}

bool CDirstatDoc::IsRootDone( ) const {
	const auto retVal = ( m_rootItem && m_rootItem->m_attr.m_done );
	return retVal;
	}

void CDirstatDoc::SetSelection( _In_ const CTreeListItem& item ) {
	m_selectedItem = const_cast< CTreeListItem* >( &item );
	m_frameptr->SetSelectionMessageText( );
	}

void CDirstatDoc::SetHighlightExtension( _In_ const std::wstring ext ) {
	TRACE( _T( "Highlighting extension %s; previously highlighted: %s\r\n" ), ext.c_str( ), m_highlightExtension.c_str( ) );
	m_highlightExtension = std::move( ext );
	m_frameptr->SetSelectionMessageText( );
	}

//_Pre_satisfies_( item.m_type == IT_FILE )
_Pre_satisfies_( item.m_child_info.m_child_info_ptr == nullptr )
void CDirstatDoc::OpenItem( _In_ const CTreeListItem& item ) {
	WTL::CWaitCursor wc;
	std::wstring path;
	//if ( item.m_type == IT_FILE ) {
	if ( item.m_child_info.m_child_info_ptr == nullptr ) {
		path = item.GetPath( ).c_str( );
		}
	const auto doesFileExist = PathFileExistsW( path.c_str( ) );
	if ( !doesFileExist ) {
		TRACE( _T( "Path (%s) is invalid!\r\n" ), path.c_str( ) );
		const std::wstring pathMsg( L"Path (" + path + L") is invalid!\r\n");
		AfxMessageBox( pathMsg.c_str( ) );
		displayWindowsMsgBoxWithError( );
		return;
		}

	const auto ShellExRes = ShellExecuteWithAssocDialog( ( AfxGetMainWnd( )->m_hWnd ), std::move( path ) );
	if ( ShellExRes < 33 ) {
		return displayWindowsMsgBoxWithError( );
		}
	}

void CDirstatDoc::RebuildExtensionData() {
	//Assigns colors to all known file types (i.e. `Extensions`)
	
	m_extensionRecords.clear( );

	const auto rootTemp = m_rootItem.get( );
	if ( rootTemp == nullptr ) {
		return;
		}

	rebuild_extension_data( m_extensionRecords, rootTemp );

	m_extensionDataValid = true;
	}


BEGIN_MESSAGE_MAP(CDirstatDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &( CDirstatDoc::OnUpdateEditCopy ) )
	ON_COMMAND(ID_EDIT_COPY, &( CDirstatDoc::OnEditCopy ) )
	ON_UPDATE_COMMAND_UI( ID_FILE_OPEN, &CDirstatDoc::OnUpdateFileOpen )
	ON_UPDATE_COMMAND_UI( ID_FILE_NEW, &CDirstatDoc::OnUpdateFileOpenLight )
END_MESSAGE_MAP( )

void CDirstatDoc::OnUpdateFileOpen( CCmdUI *pCmdUI ) {
	if ( m_rootItem == nullptr ) {
		return pCmdUI->Enable( TRUE );
		}
	return pCmdUI->Enable( FALSE );
	}

void CDirstatDoc::OnUpdateFileOpenLight( CCmdUI *pCmdUI ) {
	if ( m_rootItem == nullptr ) {
		return pCmdUI->Enable( TRUE );
		}
	return pCmdUI->Enable( FALSE );
	}


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


#else
#endif

