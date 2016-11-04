// dirstatdoc.cpp: Implementation of CDirstatDoc
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


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

	void failed_to_open_clipboard( ) {
		displayWindowsMsgBoxWithError( );
		displayWindowsMsgBoxWithMessage( L"Cannot open the clipboard." );
		TRACE( _T( "Cannot open the clipboard!\r\n" ) );
		}

	class Clipboard_wrapper final {
		public:
		DISALLOW_COPY_AND_ASSIGN( Clipboard_wrapper );

		Clipboard_wrapper( const HWND hWnd, const bool empty ) : m_open {
																		//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
																		//If the window handle identifies an existing window, the return value is nonzero.
																		//If the window handle does not identify an existing window, the return value is zero.
																		( ASSERT( ::IsWindow( hWnd ) ), 

																		//OpenClipboard function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms649048.aspx
																		//If the function succeeds, the return value is nonzero.
																		//If the function fails, the return value is zero.
																		//To get extended error information, call GetLastError.
																		::OpenClipboard( hWnd ) )
																	 } {
			if ( !m_open ) {
				failed_to_open_clipboard( );
				return;
				}
			if ( empty ) {
				//EmptyClipboard function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms649037.aspx
				//If the function succeeds, the return value is nonzero.
				//If the function fails, the return value is zero.
				//To get extended error information, call GetLastError.
				if ( !::EmptyClipboard( ) ) {
					displayWindowsMsgBoxWithError( );
					displayWindowsMsgBoxWithMessage( L"Cannot empty the clipboard." );
					TRACE( _T( "Cannot empty the clipboard!\r\n" ) );
					}
				}
			}
		~Clipboard_wrapper( ) {
			if ( m_open ) {
				//CloseClipboard function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms649035.aspx
				//If the function succeeds, the return value is nonzero.
				//If the function fails, the return value is zero.
				//To get extended error information, call GetLastError.
				VERIFY( ::CloseClipboard( ) );
				}
			}
		private:
		const BOOL m_open;
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
	void AddFileExtensionData( _Out_ std::vector<SExtensionRecord>* const extensionRecords, _Inout_ std::unordered_map<std::wstring, minimal_SExtensionRecord>* const extensionMap ) {
		extensionRecords->reserve( extensionMap->size( ) + 1 );
		for ( auto& anExt : (*extensionMap) ) {
			extensionRecords->emplace_back( std::move( anExt.second.files ), std::move( anExt.second.bytes ), std::move( anExt.first ) );
			}
		}

	_Success_( return > 32 ) INT_PTR ShellExecuteWithAssocDialog( _In_ const HWND hwnd, _In_ std::wstring filename ) {
		//WTL::CWaitCursor wc;
		auto u = reinterpret_cast< INT_PTR >( ::ShellExecuteW( hwnd, NULL, filename.c_str( ), NULL, NULL, SW_SHOWNORMAL ) );
		if ( u == SE_ERR_NOASSOC ) {
			// Q192352
			const rsize_t dir_buf_size = MAX_PATH;
			_Null_terminated_ wchar_t dir_buf[ MAX_PATH ] = { 0 };
			std::wstring parameters_filename( L"shell32.dll,OpenAs_RunDLL " + std::move( filename ) );

			//-- Get the system directory so that we know where Rundll32.exe resides.
			//GetSystemDirectory function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724373.aspx
			//If the function succeeds, the return value is the length, in TCHARs, of the string copied to the buffer, not including the terminating null character.
			//If the length is greater than the size of the buffer, the return value is the size of the buffer required to hold the path, including the terminating null character.
			const auto sys_dir_res = ::GetSystemDirectoryW( dir_buf, dir_buf_size );
			if ( sys_dir_res == 0 ) {
				displayWindowsMsgBoxWithError( );
				std::terminate( );
				//shut analyze up, thinks we continue execution!
				return -1;
				}
			if ( sys_dir_res < dir_buf_size ) {
				ASSERT( ::wcslen( dir_buf ) == ( sys_dir_res + 1 ) );
				return reinterpret_cast< INT_PTR >( ::ShellExecuteW( hwnd, L"open", L"RUNDLL32.EXE", parameters_filename.c_str( ), dir_buf, SW_SHOWNORMAL ) );
				}
			ASSERT( sys_dir_res >= dir_buf_size );
			if ( sys_dir_res >= dir_buf_size ) {
				const auto str_ptr = std::make_unique<_Null_terminated_ wchar_t[ ]>( sys_dir_res );
				const auto sys_dir_res_2 = ::GetSystemDirectoryW( str_ptr.get( ), sys_dir_res );
				if ( ( sys_dir_res_2 != 0 ) && ( ( sys_dir_res_2 + 1 ) == sys_dir_res ) ) {
					ASSERT( ( sys_dir_res_2 + 1 )  == sys_dir_res );
					ASSERT( ::wcslen( str_ptr.get( ) ) == sys_dir_res );
					ASSERT( ::wcslen( str_ptr.get( ) ) == ( sys_dir_res_2 + 1 ) );
					return reinterpret_cast< INT_PTR >( ::ShellExecuteW( hwnd, L"open", L"RUNDLL32.EXE", parameters_filename.c_str( ), str_ptr.get( ), SW_SHOWNORMAL ) );
					}
				displayWindowsMsgBoxWithMessage( L"Something is extremely wrong (GetSystemDirectoryW)!!" );
				std::terminate( );
				}
			}
		return u;
		}

	void check8Dot3NameCreationAndNotifyUser( ) {
		HKEY keyHandle = { NULL };

		const auto res_1 = ::RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\FileSystem", NULL, KEY_READ, &keyHandle );

		if ( res_1 != ERROR_SUCCESS ) {
			TRACE( _T( "key not found!\r\n" ) );
			return;
			}
		DWORD valueType = 0;
		static_assert( sizeof( BYTE ) == 1, "bad BYTE size!" );
		BYTE data[ 4 ] = { 0 };
		static_assert( sizeof( data ) == sizeof( REG_DWORD ), "bad size!" );
			
		DWORD bufferSize = sizeof( data );
			
		const auto res_2 = ::RegQueryValueExW( keyHandle, L"NtfsDisable8dot3NameCreation", NULL, &valueType, &data[0], &bufferSize );

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
			::MessageBoxW( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
			}

		if ( value == 2 ) {
			std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_per_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
			::MessageBoxW( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
			}

		if ( value == 3 ) {
			std::wstring message = std::wstring( global_strings::eight_dot_three_gen_notif1 ) + std::wstring( global_strings::eight_dot_three_sys_volume ) + std::wstring( global_strings::eight_dot_three_gen_notif2 );
			::MessageBoxW( NULL, message.c_str( ), global_strings::gen_performance_warning, MB_ICONWARNING );
			}
		}

	void SetExtensionColors( _Inout_ std::vector<SExtensionRecord>* const extensionsToSet ) {
		/*
		  New, much faster, method of assigning colors to extensions. For every element in reverseExtensionMap, assigns a color to the `color` field of an element at key (no longer accurate description). The color assigned is chosen by rotating through a default palette.
		*/
		COLORREF colorArray[ 13 ];

		const auto sizeOfArray = GetDefaultPaletteAsArray( colorArray );


		std::vector<COLORREF>::size_type processed = 0;

		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( auto& anExtension : (*extensionsToSet) ) {
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


	void rebuild_extension_data( _Out_ std::vector<SExtensionRecord>* const extension_records, _In_ const CTreeListItem* const root_item ) {
		std::unordered_map<std::wstring, minimal_SExtensionRecord> extensionMap;
		extensionMap.reserve( root_item->files_recurse( ) );

		root_item->stdRecurseCollectExtensionData( &extensionMap );
		AddFileExtensionData( extension_records, &extensionMap );
		SetExtensionColors( extension_records );
		std::sort( extension_records->begin( ), extension_records->end( ), s_compareSExtensionRecordByBytes( ) );

		extension_records->shrink_to_fit( );

		}

	void convert_vector_of_extension_records_to_map( _In_ const std::vector<SExtensionRecord>* const records, _Inout_ std::unordered_map<std::wstring, COLORREF>* const color_map ) {
		if ( records != NULL ) {
			color_map->reserve( records->size( ) );
			for ( const auto& aRecord : ( *records ) ) {
				(*color_map)[ aRecord.ext ] = aRecord.color;
				}
			}
		}

	void CopyToClipboard( _In_ const std::wstring psz, _In_ const HWND hWnd ) {
		Clipboard_wrapper clipboard( hWnd, true );
		const rsize_t strSizeInBytes = ( ( psz.length( ) + 1 ) * sizeof( WCHAR ) );

		//GlobalAlloc function: https://msdn.microsoft.com/en-us/library/windows/desktop/aa366574.aspx
		//If the function succeeds, the return value is a handle to the newly allocated memory object.
		//If the function fails, the return value is NULL.
		//To get extended error information, call GetLastError.

		const HGLOBAL handle_globally_allocated_memory = ::GlobalAlloc( GMEM_MOVEABLE bitand GMEM_ZEROINIT, strSizeInBytes );
		if ( handle_globally_allocated_memory == NULL ) {
			displayWindowsMsgBoxWithMessage( global_strings::global_alloc_failed );
			TRACE( L"%s\r\n", global_strings::global_alloc_failed );
			return;
			}

		const auto lp = ::GlobalLock( handle_globally_allocated_memory );
		if ( lp == NULL ) {
			displayWindowsMsgBoxWithMessage( L"GlobalLock failed!" );
			//GlobalFree function: https://msdn.microsoft.com/en-us/library/windows/desktop/aa366579.aspx
			//If the function succeeds, the return value is NULL.
			//If the function fails, the return value is equal to a handle to the global memory object.
			//To get extended error information, call GetLastError.
			VERIFY( !::GlobalFree( handle_globally_allocated_memory ) );
			return;
			}

		auto strP = static_cast< PWSTR >( lp );

		const HRESULT strCopyRes = StringCchCopyW( strP, ( psz.length( ) + 1 ), psz.c_str( ) );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( strCopyRes, "StringCchCopyW" );
		if ( !SUCCEEDED( strCopyRes ) ) {
			if ( strCopyRes == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				displayWindowsMsgBoxWithMessage( std::move( std::wstring( global_strings::string_cch_copy_failed ) + std::wstring( L"(STRSAFE_E_INSUFFICIENT_BUFFER)" ) ) );
				}
			else {
				displayWindowsMsgBoxWithMessage( global_strings::string_cch_copy_failed );
				}
			//GlobalUnlock function: https://msdn.microsoft.com/en-us/library/windows/desktop/aa366595.aspx
			//If the memory object is still locked after decrementing the lock count, the return value is a nonzero value.
			//If the memory object is unlocked after decrementing the lock count, the function returns zero and GetLastError returns NO_ERROR.
			//If the function fails, the return value is zero and GetLastError returns a value other than NO_ERROR.
			const BOOL unlock_res = ::GlobalUnlock( handle_globally_allocated_memory );
			strP = NULL;
			if ( unlock_res == 0 ) {
	#ifdef DEBUG
				const auto last_err = ::GetLastError( );
				ASSERT( last_err == NO_ERROR );
	#endif
				}
			return;
			}

		//GlobalUnlock function: https://msdn.microsoft.com/en-us/library/windows/desktop/aa366595.aspx
		//If the memory object is still locked after decrementing the lock count, the return value is a nonzero value.
		//If the memory object is unlocked after decrementing the lock count, the function returns zero and GetLastError returns NO_ERROR.
		//If the function fails, the return value is zero and GetLastError returns a value other than NO_ERROR.
		if ( ::GlobalUnlock( handle_globally_allocated_memory ) == 0 ) {
			const auto err = ::GetLastError( );
			if ( err != NO_ERROR ) {
				displayWindowsMsgBoxWithMessage( L"GlobalUnlock failed!" );
				return;
				}
			}
		//wtf is going on here?
		UINT uFormat = CF_TEXT;
		uFormat = CF_UNICODETEXT;

		//SetClipboardData function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms649051.aspx
		//If the function succeeds, the return value is the handle to the data.
		//If the function fails, the return value is NULL.
		//To get extended error information, call GetLastError.
		if ( NULL == ::SetClipboardData( uFormat, handle_globally_allocated_memory ) ) {
			displayWindowsMsgBoxWithMessage( global_strings::cannot_set_clipboard_data );
			TRACE( L"%s\r\n", global_strings::cannot_set_clipboard_data );
			return;
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
CDirstatDoc::CDirstatDoc( ) : m_frameptr( GetMainFrame( ) ), m_appptr( GetApp( ) ) {
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
	FILETIME t = { 0 };
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
	m_rootItem = std::make_unique<CTreeListItem>( UINT64_ERROR, t, 0, false, reinterpret_cast<CTreeListItem*>( NULL ), new_name_ptr, static_cast<std::uint16_t>( new_name_length ) );
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
	CDirstatDoc::buildDriveItems( pszPathName );

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );

	m_searchStartTime = help_QueryPerformanceCounter( );

	m_frameptr->m_wndSplitter.SetSplitterPos( 1.0 );
	m_frameptr->m_wndSubSplitter.SetSplitterPos( 1.0 );
	
	
	CDocument::UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NEWROOT );
	return true;
	}

COLORREF CDirstatDoc::GetCushionColor( _In_z_ PCWSTR const ext ) {
	if ( !m_extensionDataValid ) {
		CDirstatDoc::RebuildExtensionData( );
		}
	if ( m_colorMap.empty( ) ) {
		convert_vector_of_extension_records_to_map( GetExtensionRecords( ), &m_colorMap );
		}
		
	if ( m_colorMap.count( ext ) > 0 ) {
		return m_colorMap.at( ext );
		}
	TRACE( _T( "Extension %s not in colorMap!\r\n" ), ext );
	CDirstatDoc::RebuildExtensionData( );
	convert_vector_of_extension_records_to_map( GetExtensionRecords( ), &m_colorMap );
	if ( m_colorMap.count( ext ) > 0 ) {
		return m_colorMap.at( ext );
		}
	TRACE( _T( "Extension %s not in rebuilt colorMap!\r\n" ), ext );
	ASSERT( false );
	return static_cast<COLORREF>( 0 );
	}

//We need a getter (NOT public data member) because we may need to do work before accessing.
_Ret_notnull_ const std::vector<SExtensionRecord>* CDirstatDoc::GetExtensionRecords( ) {
	if ( !m_extensionDataValid ) {
		CDirstatDoc::RebuildExtensionData( );
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
	CDirstatDoc::SortTreeList();
	m_timeTextWritten = true;
	TRACE( _T( "All work finished!\r\n" ) );
	return true;
	}

bool CDirstatDoc::Work( ) {
	//return: true if done or suspended.

	if ( ( !m_rootItem ) || m_timeTextWritten ) {
		return true;
		}
	if ( !m_rootItem->m_attr.m_done ) {
		//WTL::CWaitCursor wc;

		auto path( m_rootItem->GetPath( ) );
		const auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
		if ( strcmp_path != 0 ) {
			path = L"\\\\?\\" + path;
			TRACE( _T( "path fixed as: %s\r\n" ), path.c_str( ) );
			}
		const auto thisApp = m_appptr;
		
		auto timing_and_elevate = DoSomeWorkShim( m_rootItem.get( ), std::move( path ), thisApp, true );
		m_compressed_file_timing = timing_and_elevate.first;
		//const bool should_we_elevate = timing_and_elevate.second;
		ASSERT( m_rootItem->m_attr.m_done );
		//if ( should_we_elevate ) {
		//	displayWindowsMsgBoxWithMessage( L"Couldn't query MFT file size, as Windows denied access. If you'd like to see the size of the MFT, run as an administrator." );
		//	}
		
		//cache the size of root item
		m_rootItem->refresh_sizeCache( );

		const auto res = CDirstatDoc::OnWorkFinished( );
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
	if ( !m_rootItem ) {
		return false;
		}
	return m_rootItem->m_attr.m_done;
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

_Pre_satisfies_( item.m_child_info.m_child_info_ptr == nullptr )
void CDirstatDoc::OpenItem( _In_ const CTreeListItem& item ) {
	if ( item.m_child_info.m_child_info_ptr != nullptr ) {
		TRACE( _T( "CDirstatDoc::OpenItem called on an item with children! Not opening!\r\n" ) );
		return;
		}
	//WTL::CWaitCursor wc;
	std::wstring path( item.GetPath( ) );
	//TODO: BUGBUG: Won't be able to open an item that's at a path longer than MAX_PATH!
	const auto doesFileExist = ::PathFileExistsW( path.c_str( ) );
	if ( !doesFileExist ) {
		TRACE( _T( "Path (%s) is invalid!\r\n" ), path.c_str( ) );
		const std::wstring pathMsg( L"Path (" + path + L") is invalid!\r\n");
		AfxMessageBox( pathMsg.c_str( ) );
		displayWindowsMsgBoxWithError( );
		return;
		}

	const auto ShellExRes = ShellExecuteWithAssocDialog( AfxGetMainWnd( )->m_hWnd, std::move( path ) );
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

	rebuild_extension_data( &m_extensionRecords, rootTemp );

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
	CopyToClipboard( std::move( itemPath ), m_frameptr->m_hWnd );
	//m_frameptr->CopyToClipboard( std::move( itemPath ) );
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

#endif
