// SelectDrivesDlg.cpp	- Implementation of CDriveItem, CDrivesList and CSelectDrivesDlg
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_SELECTDRIVESDLG_CPP
#define WDS_SELECTDRIVESDLG_CPP

#include "selectdrivesdlg.h"
#include "options.h"
#include "globalhelpers.h"



namespace {
	UINT WMU_THREADFINISHED = RegisterWindowMessageW( _T( "{F03D3293-86E0-4c87-B559-5FD103F5AF58}" ) );

	const rsize_t volume_name_size = ( MAX_PATH + 1u );
	const rsize_t max_number_of_named_drives = 32u;
	const rsize_t volume_name_pool_size = ( volume_name_size * max_number_of_named_drives );


	std::tuple<bool, PWSTR, std::uint64_t, std::uint64_t> RetrieveDriveInformation_GetVolumeName_succeeded( _In_ const std::wstring path, _In_ _Null_terminated_ wchar_t( &volume_name )[ volume_name_size ], _Inout_ _Pre_writable_size_( volume_name_size ) PWSTR formatted_volume_name ) {
		std::uint64_t total = 0;
		std::uint64_t free = 0;
		wds_fmt::FormatVolumeName( path, volume_name, formatted_volume_name );
		MyGetDiskFreeSpace( path.c_str( ), total, free );
		ASSERT( free <= total );
		return std::make_tuple( true, formatted_volume_name, total, free );
		}

	std::tuple<bool, PWSTR, std::uint64_t, std::uint64_t> RetrieveDriveInformation( _In_ const std::wstring path ) {
		_Null_terminated_ wchar_t volume_name[ volume_name_size ] = { 0 };

		//http://stackoverflow.com/a/3761217/625687
		PWSTR formatted_volume_name = new wchar_t[ volume_name_size ]( );
		memset( formatted_volume_name, 0, ( sizeof( decltype( formatted_volume_name[ 0 ] ) ) * volume_name_size ) );

		if ( !GetVolumeName( path.c_str( ), volume_name ) ) {
			//name = L"GetVolumeName failed!";
			std::uint64_t total = 0;
			std::uint64_t free = 0;
			return std::make_tuple( false, formatted_volume_name, total, free );
			}
		return RetrieveDriveInformation_GetVolumeName_succeeded( path, volume_name, formatted_volume_name );
		}

	void SetDriveInformation_set_valid_info( _Inout_ CDriveItem* const thisDriveItem, _In_ const std::wstring name, _In_ const std::uint64_t total, _In_ const std::uint64_t free ) {
		thisDriveItem->m_totalBytes  = total;
		thisDriveItem->m_freeBytes   = free;
		thisDriveItem->m_used        = 0;
		}

	void SetDriveInformation_failure( _Inout_ CDriveItem* const thisDriveItem ) {
		thisDriveItem->m_totalBytes = UINT64_MAX;
		thisDriveItem->m_freeBytes  = UINT64_MAX;
		thisDriveItem->m_used       = -1;
		}

	void SetDriveInformation_copy_name_and_set_m_used( _Inout_ CDriveItem* const thisDriveItem, _In_ const std::wstring name, _In_ Children_String_Heap_Manager* name_pool ) {

		ASSERT( name.length( ) < UINT16_MAX );
		const auto new_name_length = static_cast<std::uint16_t>( name.length( ) );

		PWSTR new_name_ptr_temp = nullptr;
		//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr_temp, new_name_length, name );
		const HRESULT copy_res = name_pool->copy_name_str_into_buffer( new_name_ptr_temp, ( new_name_length + 1u ), name );
		
		
		ASSERT( SUCCEEDED( copy_res ) );
		if ( !SUCCEEDED( copy_res ) ) {
			displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (SetDriveInformation, success)(aborting!)" );
			displayWindowsMsgBoxWithMessage( name.c_str( ) );
			std::terminate( );
			}

		//PCWSTR const new_name_ptr = new_name_ptr_temp;
		thisDriveItem->m_name = new_name_ptr_temp;
		thisDriveItem->m_name_length = new_name_length;
		if ( thisDriveItem->m_totalBytes == 0 ) {
			return;
			}

		ASSERT( thisDriveItem->m_totalBytes >= thisDriveItem->m_freeBytes );
		thisDriveItem->m_used = static_cast<DOUBLE>( thisDriveItem->m_totalBytes - thisDriveItem->m_freeBytes ) / static_cast<DOUBLE>( thisDriveItem->m_totalBytes );
		}


	void SetDriveInformation( _In_ CDriveItem* const thisDriveItem, _In_ const bool success, _In_ const std::wstring name, _In_ const std::uint64_t total, _In_ const std::uint64_t free, _In_ Children_String_Heap_Manager* const name_pool ) {
		if ( success ) {
			SetDriveInformation_set_valid_info( thisDriveItem, name, total, free );
			return SetDriveInformation_copy_name_and_set_m_used( thisDriveItem, name, name_pool );
			}
		SetDriveInformation_failure( thisDriveItem );
		SetDriveInformation_copy_name_and_set_m_used( thisDriveItem, name, name_pool );
		return;
		}

	void log_GetDriveInformation_failed_or_succeeded( _In_ const CDriveInformationThread* const thread, _In_z_ PCWSTR const name, _In_ const std::uint64_t total, _In_ const std::uint64_t free, _In_z_ PCWSTR const format_str ) {
		const rsize_t buffer_size = 256;
		_Null_terminated_ wchar_t buffer_debug_out_1[ buffer_size ] = { 0 };
		TRACE( format_str, thread, name, total, free );
		const HRESULT pf_res_1 = StringCchPrintfW( buffer_debug_out_1, buffer_size, format_str, thread, name, total, free );
		ASSERT( SUCCEEDED( pf_res_1 ) );
		if ( SUCCEEDED( pf_res_1 ) ) {
			OutputDebugStringW( buffer_debug_out_1 );
			return;
			}
		return;
		}

	void log_GetDriveInformation_failed( _In_ const CDriveInformationThread* const thread, _In_z_ PCWSTR const name, _In_ const std::uint64_t total, _In_ const std::uint64_t free ) {
		return log_GetDriveInformation_failed_or_succeeded( thread, name, total, free, global_strings::GetDriveInformation_failed_fmt_str );
		}

	void log_GetDriveInformation_succeeded( _In_ const CDriveInformationThread* const thread, _In_z_ PCWSTR const name, _In_ const std::uint64_t total, _In_ const std::uint64_t free ) {
		return log_GetDriveInformation_failed_or_succeeded( thread, name, total, free, global_strings::GetDriveInformation_succeed_fmt_str );
		}

	void log_GetDriveInformation_result( _In_ const CDriveInformationThread* const thread, _In_z_ PCWSTR const name, _In_ const std::uint64_t total, _In_ const std::uint64_t free, _In_ const bool success ) {
		if ( success ) {
			return log_GetDriveInformation_succeeded( thread, name, total, free );
			}
		return log_GetDriveInformation_failed( thread, name, total, free );
		}

	void build_select_list_failed( const HRESULT fmt_res ) {
		ASSERT( !SUCCEEDED( fmt_res ) );
		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! STRSAFE_E_INSUFFICIENT_BUFFER!!" );
			std::terminate( );
			return;
			}
		if ( fmt_res == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! STRSAFE_E_INVALID_PARAMETER!!" );
			std::terminate( );
			return;
			}
		if ( fmt_res == STRSAFE_E_END_OF_FILE ) {
			displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! STRSAFE_E_END_OF_FILE!!" );
			std::terminate( );
			return;
			}
		displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! (unknown error)" );
		std::terminate( );
		return;
		}
	
	void build_4_char_drive_info_str( _Out_ _Post_z_ wchar_t ( &drive_info_left_4_temp )[ 5u ], _In_z_ const wchar_t ( &drive_info )[ 512u ] ) {
		drive_info_left_4_temp[ 0 ] = drive_info[ 0 ];
		drive_info_left_4_temp[ 1 ] = drive_info[ 1 ];
		drive_info_left_4_temp[ 2 ] = drive_info[ 2 ];
		drive_info_left_4_temp[ 3 ] = drive_info[ 3 ];
		}

	bool IsSUBSTedDrive( _In_z_ PCWSTR const drive ) {
		/*
		  drive is a drive spec like C: or C:\ or C:\path (path is ignored).
		  This function returns true, if QueryDosDevice() is supported and drive is a SUBSTed drive.
		*/
		const rsize_t info_buffer_size = 512u;
		_Null_terminated_ wchar_t drive_info[ info_buffer_size ] = { 0 };

		const bool query_succeeded = MyQueryDosDevice( drive, drive_info );
		//ASSERT( info.Compare( drive_info ) == 0 );
		if ( query_succeeded ) {
			_Null_terminated_ wchar_t drive_info_left_4_temp[ 5 ] = { 0 };
			build_4_char_drive_info_str( drive_info_left_4_temp, drive_info );
			_Null_terminated_ const wchar_t( &drive_info_left_4 )[ 5 ] = drive_info_left_4_temp;

			return ( ( wcslen( drive_info ) >= 4 ) && ( wcscmp( drive_info_left_4, L"\\??\\" ) == 0 ) );
			}

		return false;
		}

	const bool check_result_allocate_and_copy_name_str( _In_ const HRESULT copy_res, _In_z_ PCWSTR const drive_name_buffer ) {
		ASSERT( SUCCEEDED( copy_res ) );
		if ( !SUCCEEDED( copy_res ) ) {
			displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (buildSelectList)(aborting!)" );
			displayWindowsMsgBoxWithMessage( drive_name_buffer );
			std::terminate( );
			return false;
			}
		return true;
		}

#ifdef new
#pragma push_macro("new")
#define WDS_SELECTDRIVESDLG_PUSHED_MACRO_NEW
#undef new
#endif


	const CDriveItem* placement_new_construct( _Inout_ CDrivesList& m_list, _In_opt_z_ PCWSTR const new_name_ptr, _In_ const rsize_t drive_name_length ) {
		if ( new_name_ptr == nullptr ) {
			//construct a null object
			new ( m_list.m_drives.get( ) + m_list.m_drives_count ) CDriveItem { L"", static_cast< std::uint16_t >( 0u ) };
			const auto item = ( m_list.m_drives.get( ) + m_list.m_drives_count );
			++( m_list.m_drives_count );
			return item;
			}
		new ( m_list.m_drives.get( ) + m_list.m_drives_count ) CDriveItem { new_name_ptr, static_cast< std::uint16_t >( drive_name_length ) };
		m_list.InsertListItem( m_list.GetItemCount( ), ( m_list.m_drives.get( ) + m_list.m_drives_count ) );
		const auto item = ( m_list.m_drives.get( ) + m_list.m_drives_count );
		++( m_list.m_drives_count );
		return item;
		}
#ifdef WDS_SELECTDRIVESDLG_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_SELECTDRIVESDLG_PUSHED_MACRO_NEW
#endif

	}

CDriveInformationThread::CDriveInformationThread( _In_ std::wstring path, LPARAM driveItem, HWND dialog, UINT serial, rsize_t thread_num, _In_ CRITICAL_SECTION* const cs_in, _In_ std::vector<CDriveInformationThread*>* const dlg_in ) : m_path( std::move( path ) ), m_driveItem( driveItem ), m_serial( serial ), m_threadNum( thread_num ), m_dialog( dialog ), m_totalBytes( 0 ), m_freeBytes( 0 ), m_success( false ), dialog_CRITICAL_SECTION_running_threads( cs_in ), dialog_running_threads( dlg_in ) {
	ASSERT( m_bAutoDelete );

	EnterCriticalSection( dialog_CRITICAL_SECTION_running_threads );

	if ( m_threadNum > dialog_running_threads->size( ) ) {
		dialog_running_threads->resize( m_threadNum + 1 );
		}
	dialog_running_threads->at( m_threadNum ) = this;

	LeaveCriticalSection( dialog_CRITICAL_SECTION_running_threads );

	VERIFY( CreateThread( ) );
	}

BOOL CDriveInformationThread::InitInstance( ) {
	const auto drive_tuple = RetrieveDriveInformation( m_path );
	auto m_name_previous = m_name.load( );
	if ( m_name_previous != std::get<1>( drive_tuple ) ) {
		delete[ ] m_name_previous;
		m_name.store( nullptr );
		m_name_previous = nullptr;
		}

	m_success   .store( std::get<0>( drive_tuple ) );
	m_name      .store( std::get<1>( drive_tuple ) );
	m_totalBytes.store( std::get<2>( drive_tuple ) );
	m_freeBytes .store( std::get<3>( drive_tuple ) );
	const HWND dialog = m_dialog.load( );

	if ( dialog != NULL ) {
		//Theoretically the dialog may have been closed at this point. If in the meantime the system recycled the window handle, (it may even belong to another process now?!), we are safe, because WMU_THREADFINISHED is a unique registered message.
		TRACE( _T( "Sending WMU_THREADFINISHED! m_serial: %u\r\n" ), m_serial );
		SendMessageW( dialog, WMU_THREADFINISHED, m_serial, reinterpret_cast<LPARAM>( this ) );
		}

	EnterCriticalSection( dialog_CRITICAL_SECTION_running_threads );
	dialog_running_threads->at( m_threadNum ) = nullptr;
	LeaveCriticalSection( dialog_CRITICAL_SECTION_running_threads );

	ASSERT( m_bAutoDelete ); // Object will delete itself.
	return false; // no Run(), please!
	}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CDrivesList, COwnerDrawnListCtrl )


BEGIN_MESSAGE_MAP(CDrivesList, COwnerDrawnListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &( CDrivesList::OnLvnDeleteitem ) )
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(NM_DBLCLK,      &( CDrivesList::OnNMDblclk ) )
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSelectDrivesDlg, CDialog)

UINT CSelectDrivesDlg::_serial;


#pragma warning(suppress:4355)
CSelectDrivesDlg::CSelectDrivesDlg( CWnd* pParent /*=NULL*/ ) : CDialog( CSelectDrivesDlg::IDD, pParent ), m_radio( RADIO_ALLLOCALDRIVES ), m_layout( static_cast<CWnd*>( this ), global_strings::select_drives_dialog_layout ), m_name_pool( volume_name_pool_size ) {
	_serial++;
	//InitializeCriticalSection_wrapper( _csRunningThreads );
	InitializeCriticalSection_wrapper( m_running_threads_CRITICAL_SECTION );
	}

_Pre_defensive_ void CSelectDrivesDlg::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_DRIVES, m_list );
	DDX_Radio( pDX, IDC_ALLDRIVES, static_cast<int>( m_radio ) );
	
	//I hate CString.
	//CString local_folder_name = m_folder_name_heap.c_str( );

	const rsize_t largest_possible_filepath = 33000u;

	std::unique_ptr<_Null_terminated_ wchar_t[ ]> buffer = std::make_unique<wchar_t[]>( largest_possible_filepath );
	static_assert( sizeof( decltype( buffer.get( )[ 0 ] ) ) == sizeof( wchar_t ), "bad memset size!" );
	memset( buffer.get( ), 0, ( ( sizeof( decltype( buffer.get( )[ 0 ] ) ) ) * largest_possible_filepath ) );

	PWSTR buffer_ptr = buffer.get( );

	const HRESULT res = StringCchCopyW( buffer_ptr, largest_possible_filepath, m_folder_name_heap.c_str( ) );
	ASSERT( SUCCEEDED( res ) );
	if ( !SUCCEEDED( res ) ) {
		TRACE( _T( "oops!\r\n" ) );
		}

	//DDX_Text( pDX, IDC_FOLDERNAME, local_folder_name );
	DDX_Text( pDX, IDC_FOLDERNAME, buffer_ptr, static_cast<int>( largest_possible_filepath ) );
	
	DDX_Control( pDX, IDOK, m_okButton );
	m_folder_name_heap = buffer_ptr;
	}


BEGIN_MESSAGE_MAP(CSelectDrivesDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSEFOLDER,           &( CSelectDrivesDlg::OnBnClickedBrowsefolder ) )
	ON_BN_CLICKED(IDC_AFOLDER,                &( CSelectDrivesDlg::UpdateButtons ) )
	ON_BN_CLICKED(IDC_SOMEDRIVES,             &( CSelectDrivesDlg::UpdateButtons ) )
	ON_EN_CHANGE(IDC_FOLDERNAME,              &( CSelectDrivesDlg::UpdateButtons ) )
	ON_WM_MEASUREITEM()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DRIVES,    &( CSelectDrivesDlg::OnLvnItemchangedDrives ) )
	ON_BN_CLICKED(IDC_ALLLOCALDRIVES,         &( CSelectDrivesDlg::UpdateButtons ) )
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_MESSAGE(WMU_OK,                        &( CSelectDrivesDlg::OnWmuOk ) )
	ON_REGISTERED_MESSAGE(WMU_THREADFINISHED, &( CSelectDrivesDlg::OnWmuThreadFinished ) )
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

void CSelectDrivesDlg::setListOptions( ) {
	const auto Options = GetOptions( );
	m_list.ShowGrid(             Options->m_listGrid );
	m_list.ShowStripes(          Options->m_listStripes );
	m_list.ShowFullRowSelection( Options->m_listFullRowSelection );
	}

void CSelectDrivesDlg::initWindow( ) {
	ShowWindow( SW_SHOWNORMAL );
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::UpdateWindow( m_hWnd ) );
	UpdateWindow(             );
	BringWindowToTop(         );
	SetForegroundWindow(      );
	}

void CSelectDrivesDlg::buildSelectList( ) {
	const auto drives = GetLogicalDrives( );
	INT i = 0;
	DWORD mask = 0x00000001;
	m_list.m_drives.reset( new CDriveItem[ max_number_of_named_drives ] );
	m_list.m_drives_count = 0;
	for ( i = 0; i < static_cast<int>( max_number_of_named_drives ); i++, mask <<= 1 ) {
		
		
		if ( ( drives bitand mask ) == 0 ) {
			(void)placement_new_construct( m_list, nullptr, 0 );
			continue;
			}

		const rsize_t drive_name_buffer_size = ( MAX_PATH * 2 );
		_Null_terminated_ wchar_t drive_name_buffer[ drive_name_buffer_size ] = { 0 };
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( drive_name_buffer, drive_name_buffer_size, NULL, &chars_remaining, 0, L"%c:\\", ( i + _T( 'A' ) ) );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( !SUCCEEDED( fmt_res ) ) {

			build_select_list_failed( fmt_res );
			//shut `/analyze` up.
			return;
			}

		const rsize_t drive_name_length = ( drive_name_buffer_size - chars_remaining );
		ASSERT( wcslen( drive_name_buffer ) == drive_name_length );

		const auto type = GetDriveTypeW( drive_name_buffer );
		if ( ( type == DRIVE_UNKNOWN ) || ( type == DRIVE_NO_ROOT_DIR ) ) {
			(void)placement_new_construct( m_list, nullptr, 0 );
			continue;
			}

		// The check of remote drives will be done in the background by the CDriveInformationThread.
		EnterCriticalSection( &m_running_threads_CRITICAL_SECTION );

		if ( ( type != DRIVE_REMOTE ) && ( !DriveExists( drive_name_buffer, drive_name_length ) ) ) {
			LeaveCriticalSection( &m_running_threads_CRITICAL_SECTION );
			(void)placement_new_construct( m_list, nullptr, 0 );
			continue;
			}
		LeaveCriticalSection( &m_running_threads_CRITICAL_SECTION );
		ASSERT( drive_name_length < UINT16_MAX );

		PWSTR new_name_ptr = nullptr;
		const HRESULT copy_res = m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( drive_name_length + 1u ), drive_name_buffer );

		//void check_result_allocate_and_copy_name_str( _In_ const HRESULT copy_res, _In_z_ PCWSTR const drive_name_buffer )

		const bool do_we_continue = check_result_allocate_and_copy_name_str( copy_res, drive_name_buffer );
		if ( !do_we_continue ) {
			return;
			}

		ASSERT( drive_name_length < UINT16_MAX );
		ASSERT( wcscmp( new_name_ptr, drive_name_buffer ) == 0 );
		//const auto item = new CDriveItem { std::move( new_name_ptr ), static_cast< std::uint16_t >( drive_name_length ) };

		//const CDriveItem* placement_new_construct( _Inout_ CDrivesList& m_list, _In_z_ PCWSTR const new_name_ptr, _In_ const rsize_t drive_name_length )
		const auto item = placement_new_construct( m_list, new_name_ptr, drive_name_length );

		//new ( m_list.m_drives.get( ) + m_list.m_drives_count ) CDriveItem { new_name_ptr, static_cast< std::uint16_t >( drive_name_length ) };
		//m_list.InsertListItem( m_list.GetItemCount( ), ( m_list.m_drives.get( ) + m_list.m_drives_count ) );
		//const auto item = ( m_list.m_drives.get( ) + m_list.m_drives_count );
		//++( m_list.m_drives_count );

		EnterCriticalSection( &m_running_threads_CRITICAL_SECTION );
		CRITICAL_SECTION* const crit_sec_temp = ( &m_running_threads_CRITICAL_SECTION );
		std::vector<CDriveInformationThread*>* const run_thread_temp = ( &m_running_threads );
		LeaveCriticalSection( &m_running_threads_CRITICAL_SECTION );

		new CDriveInformationThread { item->m_path, reinterpret_cast< LPARAM >( item ), m_hWnd, _serial, static_cast< rsize_t >( i ), crit_sec_temp, run_thread_temp };// (will delete itself when finished.)

		for ( size_t k = 0; k < m_selectedDrives.size( ); k++ ) {
			ASSERT( item->m_path.length( ) > 1 );
			if ( item->m_path.substr( 0, 2 ) == m_selectedDrives.at( k ) ) {
				const auto item_position = m_list.FindListItem( item );
				VERIFY( m_list.SetItemState( item_position, LVIS_SELECTED, LVIS_SELECTED ) );
				break;
				}
			}
		}
	}

BOOL CSelectDrivesDlg::OnInitDialog( ) {
	WTL::CWaitCursor wc;
	VERIFY( CDialog::OnInitDialog( ) );
	if ( WMU_THREADFINISHED == 0 ) {
		TRACE( "RegisterMessage() failed. Using WM_USER + 123\r\n" );
		WMU_THREADFINISHED = WM_USER + 123;
		}

	VERIFY( ModifyStyle( 0, WS_CLIPCHILDREN ) );
	addControls( );
	m_layout.OnInitDialog( true );
	setListOptions( );
	m_list.SetExtendedStyle( m_list.GetExtendedStyle( ) | LVS_EX_HEADERDRAGDROP );
	// If we set an ImageList here, OnMeasureItem will have no effect ?!

	insertColumns( );
	TRACE( _T( "Loading persistent attributes....\r\n" ) );
	m_list.OnColumnsInserted( );

	m_folder_name_heap = CPersistence::GetSelectDrivesFolder( );
	//m_folderName = m_folder_name_heap.c_str( );
	//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );

	CPersistence::GetSelectDrivesDrives( m_selectedDrives );
	initWindow( );
	buildSelectList( );
	m_list.SortItems( );
	m_radio = CPersistence::GetSelectDrivesRadio( );
	VERIFY( UpdateData( false ) );

	switch ( m_radio )
	{
		case RADIO_ALLLOCALDRIVES:
		case RADIO_AFOLDER:
			m_okButton.SetFocus( );
			break;
		case RADIO_SOMEDRIVES:
			m_list.SetFocus( );
			break;
		default:
			m_list.SetFocus( );
	}
	UpdateButtons( );
	return false; // we have set the focus.
	}

void CSelectDrivesDlg::OnBnClickedBrowsefolder( ) {
	TRACE( _T( "User wants to select a folder to analyze...\r\n" ) );
	
	const UINT flags = ( BIF_RETURNONLYFSDIRS bitor BIF_USENEWUI bitor BIF_NONEWFOLDERBUTTON );
	WTL::CFolderDialog bob { NULL, global_strings::select_folder_dialog_title_text, flags };
	//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
	bob.SetInitialFolder( m_folder_name_heap.c_str( ) );
	auto resDoModal = bob.DoModal( );
	if ( resDoModal == IDOK ) {
		m_folder_name_heap = bob.GetFolderPath( );
		//m_folderName = m_folder_name_heap.c_str( );
		//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
		TRACE( _T( "User chose: %s\r\n" ), m_folder_name_heap.c_str( ) );
		m_radio = RADIO_AFOLDER;
		VERIFY( UpdateData( false ) );
		UpdateButtons( );
		}
	else {
		TRACE( _T( "user hit cancel - no changes necessary.\r\n" ) );
		}

	}


_Pre_satisfies_( m_radio != RADIO_AFOLDER )
void CSelectDrivesDlg::handle_RADIO_other( ) {
	for ( INT i = 0; i < m_list.GetItemCount( ); i++ ) {
		const auto item = m_list.GetItem( i );
		if ( item == NULL ) {
			displayWindowsMsgBoxWithMessage( L"Error in CSelectDrivesDlg::OnOK: item == NULL (aborting)" );
			std::terminate( );
			//`/analyze` is confused.
			return;
			}
		if (    ( m_radio == RADIO_ALLLOCALDRIVES          ) && 
				( !IsSUBSTedDrive( item->m_path.c_str( ) ) ) ||
				( m_radio == RADIO_SOMEDRIVES              ) && 
				( ( LVIS_SELECTED == m_list.GetItemState( i, LVIS_SELECTED ) ) )
					                                        ) {
			//( LVIS_SELECTED == m_list.GetItemState( i, LVIS_SELECTED ) )
			ASSERT( item->m_path.length( ) > 1 );
			m_drives.emplace_back( item->m_path.substr( 0, 2 ) );
			}

		if ( ( LVIS_SELECTED == m_list.GetItemState( i, LVIS_SELECTED ) ) ) {
			ASSERT( item->m_path.length( ) > 1 );
			m_selectedDrives.emplace_back( item->m_path.substr( 0, 2 ) );
			}
		}
	}

_Pre_defensive_ void CSelectDrivesDlg::OnOK( ) {
	TRACE( _T( "User hit ok...\r\n" ) );
	VERIFY( UpdateData( ) );

	m_drives.clear( );
	m_selectedDrives.clear( );

	if ( m_radio == RADIO_AFOLDER ) {
		handle_RADIO_AFOLDER( );
		}
	else {
		handle_RADIO_other( );
		}

	//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
	CPersistence::SetSelectDrivesRadio ( m_radio          );
	CPersistence::SetSelectDrivesFolder( m_folder_name_heap.c_str( ) );
	CPersistence::SetSelectDrivesDrives( m_selectedDrives );

	if ( m_selectedDrives.size( ) > 1 ) {
		displayWindowsMsgBoxWithMessage( L"Scanning multiple drives at once is NOT currently implemented. Please try one at a time." );
		}

	CDialog::OnOK( );
	}

_Pre_defensive_ void CSelectDrivesDlg::UpdateButtons( ) {
	VERIFY( UpdateData( ) );
	BOOL enableOk = FALSE;
	switch ( m_radio )
		{
			case RADIO_ALLLOCALDRIVES:
				enableOk = TRUE;
				break;
			case RADIO_SOMEDRIVES:
				enableOk = ( ( m_list.GetSelectedCount( ) > 0 ) ? TRUE : FALSE );
				break;
			case RADIO_AFOLDER:
				//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
				if ( !m_folder_name_heap.empty( ) ) {
					//ASSERT( ( m_folderName.Left( 2 ).Compare( L"\\\\" ) ) == m_folder_name_heap.compare( 0, 2, L"\\\\", 2 ) );
					//ASSERT( ( m_folderName.Left( 2 ) == L"\\\\" ) == ( m_folder_name_heap.substr( 0, 2 ) == L"\\\\" ) );
					if ( ( m_folder_name_heap.length( ) >= 2 ) && ( m_folder_name_heap.compare( 0, 2, L"\\\\", 2 ) == 0 ) ) {
						enableOk = TRUE;
						}
					else {
						//CString pattern = m_folder_name_heap.c_str( );
						auto pattern = m_folder_name_heap;
						if ( m_folder_name_heap.back( ) != L'\\' ) {
							pattern += L'\\';
							}
						
						//if ( pattern.Right( 1 ) != _T( "\\" ) ) {
						//	pattern += _T( "\\" );
						//	}
						pattern += L"*.*";
						CFileFind finder;
						const BOOL b = finder.FindFile( pattern.c_str( ) );
						enableOk = b;
						}
					}
				break;
			default:
				ASSERT( false );
		}
	m_okButton.EnableWindow( enableOk );
	}

LRESULT _Function_class_( "GUI_THREAD" ) CSelectDrivesDlg::OnWmuThreadFinished( const WPARAM serial, const LPARAM lparam ) {
	/*
	  This message is _sent_ by a CDriveInformationThread.
	  */
	TRACE( _T( "Entering OnWmuThreadFinished...\r\n" ) );
	if ( serial != _serial ) {
		TRACE( _T( "Leaving OnWmuThreadFinished: invalid serial (window handle recycled?)\r\n" ) );
		return 0;
		}
	const auto thread = reinterpret_cast< CDriveInformationThread * > ( lparam );
	//EnterCriticalSection( &_csRunningThreads );
	EnterCriticalSection( &m_running_threads_CRITICAL_SECTION );

	//auto driveItem = thread->GetDriveInformation( success, name, total, free );
	auto driveItem = thread->m_driveItem;
	const std::uint64_t total = thread->m_totalBytes.load( );
	const std::uint64_t free = thread->m_freeBytes.load( );
	const bool success = thread->m_success.load( );
	const std::wstring name = thread->m_name.load( );
	const auto old_name_value = thread->m_name.load( );
	thread->m_name.store( nullptr );
	delete[ ] old_name_value;
	//delete[ ] thread->m_name.load( );
	
	LeaveCriticalSection( &m_running_threads_CRITICAL_SECTION );
	//LeaveCriticalSection( &_csRunningThreads );
	
	log_GetDriveInformation_result( thread, name.c_str( ), total, free, success );

	// For paranoia's sake we check, whether driveItem is in our list. (and we so find its index.)
	//auto fi = zeroInitLVFINDINFO( );
	auto fi = zero_init_struct<LVFINDINFO>( );
	fi.flags  = LVFI_PARAM;
	fi.lParam = driveItem;

	auto i = m_list.FindItem( &fi );
	if ( i == -1 ) {
		TRACE( _T( "Leaving OnWmuThreadFinished: item not found!\r\n" ) );
		return 0;
		}

	auto item = reinterpret_cast<CDriveItem *>( driveItem );
	//EnterCriticalSection( &_csRunningThreads );
	EnterCriticalSection( &m_running_threads_CRITICAL_SECTION );
	
	SetDriveInformation( item, success, std::move( name ), total, free, &m_name_pool );
	
	LeaveCriticalSection( &m_running_threads_CRITICAL_SECTION );
	//LeaveCriticalSection( &_csRunningThreads );
	
	VERIFY( m_list.RedrawItems( i, i ) );
	m_list.SortItems  (      );
	//TRACE( _T( "CSelectDrivesDlg::OnWmuThreadFinished\r\n") );
	//delete thread;
	//thread = NULL;
	return 0;//NULL??
	}

void CSelectDrivesDlg::OnDestroy( ) {
	EnterCriticalSection( &m_running_threads_CRITICAL_SECTION );

	for ( auto& aThread : m_running_threads ) {
		if ( aThread != nullptr ) {
			aThread->m_dialog.store( nullptr );
			}
		}

	LeaveCriticalSection( &m_running_threads_CRITICAL_SECTION );
	//CDriveInformationThread::InvalidateDialogHandle( );
	m_layout.OnDestroy( );
	CDialog::OnDestroy( );
	}



#else

#endif