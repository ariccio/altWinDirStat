// SelectDrivesDlg.cpp	- Implementation of CDriveItem, CDrivesList and CSelectDrivesDlg
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
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

#include "selectdrivesdlg.h"
#include "options.h"
#include "globalhelpers.h"


namespace {
	UINT WMU_THREADFINISHED = RegisterWindowMessageW( _T( "{F03D3293-86E0-4c87-B559-5FD103F5AF58}" ) );
	static CRITICAL_SECTION _csRunningThreads;
	_Guarded_by_( _csRunningThreads ) static std::vector<CDriveInformationThread*> vec_runningThreads;

	std::tuple<bool, PWSTR, std::uint64_t, std::uint64_t> RetrieveDriveInformation( _In_ const std::wstring path ) {

		const rsize_t volume_name_size = ( MAX_PATH + 1u );
		wchar_t volume_name[ volume_name_size ] = { 0 };

		std::uint64_t total = 0;
		std::uint64_t free = 0;
		//http://stackoverflow.com/a/3761217/625687
		PWSTR formatted_volume_name = new wchar_t[ volume_name_size ]( );

		if ( !GetVolumeName( path.c_str( ), volume_name ) ) {
			//name = L"GetVolumeName failed!";

			return std::make_tuple( false, formatted_volume_name, total, free );
			}
		//wchar_t formatted_volume_name[ volume_name_size ] = { 0 };
		FormatVolumeName( path, volume_name, formatted_volume_name );
		MyGetDiskFreeSpace( path.c_str( ), total, free );
		ASSERT( free <= total );
		return std::make_tuple( true, formatted_volume_name, total, free );
		}

	void SetDriveInformation( _In_ CDriveItem* thisDriveItem, _In_ const bool success, _In_ std::wstring name, _In_ const std::uint64_t total, _In_ const std::uint64_t free ) {
		if ( success ) {
			if ( thisDriveItem->m_name.get( ) != nullptr ) {
				thisDriveItem->m_name.reset( );
				//thisDriveItem->m_name = NULL;
				}
			ASSERT( name.length( ) < UINT16_MAX );
			const auto new_name_length = static_cast<std::uint16_t>( name.length( ) );
			ASSERT( new_name_length < UINT16_MAX );

			PWSTR new_name_ptr_temp = nullptr;
			const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr_temp, new_name_length, name );
			ASSERT( SUCCEEDED( copy_res ) );
			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (SetDriveInformation, success)(aborting!)" );
				displayWindowsMsgBoxWithMessage( name.c_str( ) );
				std::terminate( );
				}
			else {
				PCWSTR new_name_ptr = new_name_ptr_temp;
				thisDriveItem->m_name.reset( new_name_ptr );
				thisDriveItem->m_name_length = new_name_length;
				}

			thisDriveItem->m_totalBytes  = total;
			thisDriveItem->m_freeBytes   = free;
			thisDriveItem->m_used        = 0;

			if ( thisDriveItem->m_totalBytes > 0 ) {
				ASSERT( thisDriveItem->m_totalBytes >= thisDriveItem->m_freeBytes );
				thisDriveItem->m_used = static_cast<DOUBLE>( thisDriveItem->m_totalBytes - thisDriveItem->m_freeBytes ) / static_cast<DOUBLE>( thisDriveItem->m_totalBytes );
				}
			}
		else {
			thisDriveItem->m_totalBytes = UINT64_MAX;
			thisDriveItem->m_freeBytes  = UINT64_MAX;
			thisDriveItem->m_used       = -1;

			if ( thisDriveItem->m_name != nullptr ) {
				thisDriveItem->m_name.reset( );
				//thisDriveItem->m_name = NULL;
				}
			const auto new_name_length = static_cast<std::uint16_t>( name.length( ) );
			ASSERT( new_name_length < UINT16_MAX );
			
			PWSTR new_name_ptr_temp = nullptr;
			const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr_temp, new_name_length, name );
			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (SetDriveInformation)(aborting!)" );
				displayWindowsMsgBoxWithMessage( name.c_str( ) );
				std::terminate( );
				}
			else {
				PCWSTR new_name_ptr = new_name_ptr_temp;

				thisDriveItem->m_name.reset( new_name_ptr );
				thisDriveItem->m_name_length = new_name_length;
				}
			}
		}

	void log_GetDriveInformation_result( _In_ const CDriveInformationThread* const thread, _In_z_ PCWSTR name, _In_ const std::uint64_t total, _In_ const std::uint64_t free, _In_ const bool success ) {
		const rsize_t buffer_size = 256;
		wchar_t buffer_debug_out[ buffer_size ] = { 0 };
		if ( success ) {
			TRACE( _T( "thread (%p)->GetDriveInformation succeeded!, name: %s, total: %I64u, free: %I64u\r\n" ), thread, name, total, free );

			const HRESULT pf_res_1 = StringCchPrintfW( buffer_debug_out, buffer_size, L"WDS: thread (%p)->GetDriveInformation succeeded!, name: %s, total: %I64u, free: %I64u\r\n", thread, name, total, free );
			ASSERT( SUCCEEDED( pf_res_1 ) );
			if ( SUCCEEDED( pf_res_1 ) ) {
				OutputDebugStringW( buffer_debug_out );
				}
			}
		else {
			TRACE( _T( "thread (%p)->GetDriveInformation failed!, name: %s, total: %I64u, free: %I64u\r\n" ), thread, name, total, free );
			const HRESULT pf_res_1 = StringCchPrintfW( buffer_debug_out, buffer_size, L"WDS: thread (%p)->GetDriveInformation failed!, name: %s, total: %I64u, free: %I64u\r\n", thread, name, total, free );
			ASSERT( SUCCEEDED( pf_res_1 ) );
			if ( SUCCEEDED( pf_res_1 ) ) {
				OutputDebugStringW( buffer_debug_out );
				}
			}
		}
	
	}

INT CDriveItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	const auto other = static_cast<const CDriveItem*>( baseOther );
	switch ( subitem )
	{
		case column::COL_NAME:
			return signum( m_path.compare( other->m_path ) );

		case column::COL_TOTAL:
			return signum( static_cast<std::int64_t>( m_totalBytes ) - static_cast<std::int64_t>( other->m_totalBytes ) );

		case column::COL_FREE:
			return signum( static_cast<std::int64_t>( m_freeBytes ) - static_cast<std::int64_t>( other->m_freeBytes ) );

		case column::COL_ATTRIBUTES:
		case column::COL_BYTES:
		case column::COL_BYTESPERCENT:
		case column::COL_FILES_TYPEVIEW:
		default:
			ASSERT( false );
			return 0;
	}
	}

_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CDriveItem::Text_WriteToStackBuffer_COL_TOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	const auto res = FormatBytes( ( ( subitem == column::COL_TOTAL ) ? m_totalBytes : m_freeBytes ), psz_text, strSize, chars_written );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	return res;
	}

_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CDriveItem::WriteToStackBuffer_default( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	ASSERT( false );
	if ( strSize > 41 ) {
		write_bad_fmt_msg( psz_text, chars_written );
		return S_OK;
		}
	sizeBuffNeed = 64;
	return STRSAFE_E_INSUFFICIENT_BUFFER;
	}

_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CDriveItem::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	switch ( subitem )
	{
			case column::COL_TOTAL:
			case column::COL_FREE:
				return Text_WriteToStackBuffer_COL_TOTAL( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_NAME:
			case column::COL_ITEMS:
			case column::COL_BYTESPERCENT:
			case column::COL_FILES_TYPEVIEW:
			case column::COL_ATTRIBUTES:
			default:
				return WriteToStackBuffer_default( psz_text, strSize, sizeBuffNeed, chars_written );
	}
	}
CDriveInformationThread::CDriveInformationThread( _In_ std::wstring path, LPARAM driveItem, HWND dialog, UINT serial, rsize_t thread_num ) : m_path( std::move( path ) ), m_driveItem( driveItem ), m_serial( serial ), m_threadNum( thread_num ), m_dialog( dialog ), m_totalBytes( 0 ), m_freeBytes( 0 ), m_success( false ) {
	ASSERT( m_bAutoDelete );

	EnterCriticalSection( &_csRunningThreads );
	if ( m_threadNum > vec_runningThreads.size( ) ) {
		vec_runningThreads.resize( m_threadNum + 1 );
		}
	vec_runningThreads.at( m_threadNum ) = this;
	LeaveCriticalSection( &_csRunningThreads );


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

	EnterCriticalSection( &_csRunningThreads );
	vec_runningThreads.at( m_threadNum ) = nullptr;
	LeaveCriticalSection( &_csRunningThreads );


	ASSERT( m_bAutoDelete ); // Object will delete itself.
	return false; // no Run(), please!
	}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CDrivesList, COwnerDrawnListCtrl )

void CDrivesList::OnLButtonDown( const UINT /*nFlags*/, const CPoint /*point*/ ) {
	if ( ( GetFocus( ) == this ) || ( GetSelectedCount( ) == 0 ) ) { // We simulate Ctrl-Key-Down here, so that the dialog can be driven with one hand (mouse) only.
		const auto msg = GetCurrentMessage( );
		DefWindowProcW( msg->message, msg->wParam | MK_CONTROL, msg->lParam );
		}
	else {
		SetFocus( );
		// Send a LVN_ITEMCHANGED to the parent, so that it can update the radio button.
		//auto lv = zeroInitNMLISTVIEW( );
		auto lv = zero_init_struct<NMLISTVIEW>( );
		lv.hdr.hwndFrom = m_hWnd;
		lv.hdr.idFrom   = static_cast<UINT_PTR>( GetDlgCtrlID( ) );
		lv.hdr.code     = LVN_ITEMCHANGED;
		TRACE( _T( "Sending LVN_ITEMCHANGED ( via WM_NOTIFY ) to parent!\r\n" ) );
		GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), reinterpret_cast<LPARAM>( &lv ) );
		}
	}

void CDrivesList::OnLvnDeleteitem( NMHDR* pNMHDR, LRESULT* pResult ) {
	auto pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	const auto drive_list_item = GetItem( pNMLV->iItem );
	TRACE( _T( "Deleting CDriveItem: %p\r\n" ), drive_list_item );
	pNMLV->iItem = -1;
	pNMLV->iSubItem = 0;
	delete drive_list_item;
	*pResult = 0;
	}

void CDrivesList::MeasureItem( PMEASUREITEMSTRUCT pMeasureItemStruct ) {
	pMeasureItemStruct->itemHeight = m_rowHeight;
	}

void CDrivesList::OnNMDblclk( NMHDR* /*pNMHDR*/, LRESULT* pResult ) {
	*pResult = 0;

	auto point = GetCurrentMessage( )->pt;
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY( ::ScreenToClient( m_hWnd, &point ) );
	//ScreenToClient( &point );

	const auto i = HitTest( point );
	if ( i == -1 ) {
		return;
		}
	const auto item_count = GetItemCount( );
	//Not vectorized: 1200, loop contains data dependencies
	for ( int k = 0; k < item_count; k++ ) {
		VERIFY( SetItemState( k, ( k == i ? LVIS_SELECTED : static_cast<UINT>( 0u ) ), LVIS_SELECTED ) );
		}
	TRACE( _T( "User double-clicked! Sending WMU_OK!\r\n" ) );
	GetParent( )->SendMessageW( WMU_OK );
	}

CDrivesList::CDrivesList( ) : COwnerDrawnListCtrl( global_strings::drives_str, 20 ) { }

_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
const CDriveItem* CDrivesList::GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const {
	ASSERT( i < GetItemCount( ) );
	const auto itemCount = GetItemCount( );
	if ( i < itemCount ) {
		return reinterpret_cast< CDriveItem* >( GetItemData( static_cast<int>( i ) ) );
		}
	return NULL;
	}


BEGIN_MESSAGE_MAP(CDrivesList, COwnerDrawnListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &( CDrivesList::OnLvnDeleteitem ) )
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(NM_DBLCLK, &( CDrivesList::OnNMDblclk ) )
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSelectDrivesDlg, CDialog)

UINT CSelectDrivesDlg::_serial;

#pragma warning(suppress:4355)
CSelectDrivesDlg::CSelectDrivesDlg( CWnd* pParent /*=NULL*/ ) : CDialog( CSelectDrivesDlg::IDD, pParent ), m_layout( static_cast<CWnd*>( this ), global_strings::select_drives_dialog_layout ), m_radio( RADIO_ALLLOCALDRIVES ) {
	_serial++;
	InitializeCriticalSection( &_csRunningThreads );
	}

CSelectDrivesDlg::~CSelectDrivesDlg( ) {
	DeleteCriticalSection( &_csRunningThreads );
	}

_Pre_defensive_ void CSelectDrivesDlg::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_DRIVES, m_list );
	DDX_Radio( pDX, IDC_ALLDRIVES, static_cast<int>( m_radio ) );
	CString local_folder_name = m_folder_name_heap.c_str( );
	DDX_Text( pDX, IDC_FOLDERNAME, local_folder_name );
	DDX_Control( pDX, IDOK, m_okButton );
	m_folder_name_heap = local_folder_name.GetString( );
	}


BEGIN_MESSAGE_MAP(CSelectDrivesDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSEFOLDER, &( CSelectDrivesDlg::OnBnClickedBrowsefolder ) )
	ON_BN_CLICKED(IDC_AFOLDER, &( CSelectDrivesDlg::UpdateButtons ) )
	ON_BN_CLICKED(IDC_SOMEDRIVES, &( CSelectDrivesDlg::UpdateButtons ) )
	ON_EN_CHANGE(IDC_FOLDERNAME, &( CSelectDrivesDlg::UpdateButtons ) )
	ON_WM_MEASUREITEM()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DRIVES, &( CSelectDrivesDlg::OnLvnItemchangedDrives ) )
	ON_BN_CLICKED(IDC_ALLLOCALDRIVES, &( CSelectDrivesDlg::UpdateButtons ) )
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_MESSAGE(WMU_OK, &( CSelectDrivesDlg::OnWmuOk ) )
	ON_REGISTERED_MESSAGE(WMU_THREADFINISHED, &( CSelectDrivesDlg::OnWmuThreadFinished ) )
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

void CSelectDrivesDlg::addControls( ) {
	m_layout.AddControl( IDOK,                  1, 0, 0, 0 );
	m_layout.AddControl( IDCANCEL,              1, 0, 0, 0 );
	m_layout.AddControl( IDC_DRIVES,            0, 0, 1, 1 );
	m_layout.AddControl( IDC_AFOLDER,           0, 1, 0, 0 );
	m_layout.AddControl( IDC_FOLDERNAME,        0, 1, 1, 0 );
	m_layout.AddControl( IDC_BROWSEFOLDER,      1, 1, 0, 0 );

	}

void CSelectDrivesDlg::insertColumns( ) {
	m_list.InsertColumn( column::COL_NAME,        global_strings::name,        LVCFMT_LEFT , 120, column::COL_NAME        );
	m_list.InsertColumn( column::COL_TOTAL,       global_strings::total,       LVCFMT_RIGHT,  55, column::COL_TOTAL       );
	m_list.InsertColumn( column::COL_FREE,        global_strings::free,        LVCFMT_RIGHT,  55, column::COL_FREE        );
	}

void CSelectDrivesDlg::setListOptions( ) {
	auto Options = GetOptions( );
	m_list.ShowGrid(             Options->m_listGrid );
	m_list.ShowStripes(          Options->m_listStripes );
	m_list.ShowFullRowSelection( Options->m_listFullRowSelection );
	}

void CSelectDrivesDlg::initWindow( ) {
	ShowWindow( SW_SHOWNORMAL );
	ASSERT(::IsWindow(m_hWnd));
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
	for ( i = 0; i < 32; i++, mask <<= 1 ) {
		if ( ( drives bitand mask ) == 0 ) {
			continue;
			}



		const rsize_t drive_name_buffer_size = ( MAX_PATH * 2 );
		wchar_t drive_name_buffer[ drive_name_buffer_size ] = { 0 };
		rsize_t chars_remaining = 0;
		const HRESULT fmt_res = StringCchPrintfExW( drive_name_buffer, drive_name_buffer_size, NULL, &chars_remaining, 0, L"%c:\\", ( i + _T( 'A' ) ) );
		//const HRESULT fmt_res = StringCchPrintfW( drive_name_buffer, drive_name_buffer_size, L"%c:\\", ( i + _T( 'A' ) ) );
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( !SUCCEEDED( fmt_res ) ) {
			if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! STRSAFE_E_INSUFFICIENT_BUFFER!!" );
				std::terminate( );
				}
			if ( fmt_res == STRSAFE_E_INVALID_PARAMETER ) {
				displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! STRSAFE_E_INVALID_PARAMETER!!" );
				std::terminate( );
				}
			if ( fmt_res == STRSAFE_E_END_OF_FILE ) {
				displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! STRSAFE_E_END_OF_FILE!!" );
				std::terminate( );
				}
			else {
				displayWindowsMsgBoxWithMessage( L"CSelectDrivesDlg::buildSelectList failed!!! (unknown error)" );
				std::terminate( );
				}
			displayWindowsMsgBoxWithMessage( L"Unintended execution in CSelectDrivesDlg::buildSelectList!!! (anyways, there's an unknown error)" );
			std::terminate( );
			
			//shut `/analyze` up.
			return;
			}

		const rsize_t drive_name_length = ( drive_name_buffer_size - chars_remaining );

		//const PCWSTR drive_name = drive_name_buffer;

		
		//s.Format( _T( "%c:\\" ), i + _T( 'A' ) );
		//ASSERT( wcscmp( s.GetString( ), drive_name_buffer ) == 0 );
		//ASSERT( static_cast<int>( wcslen( drive_name_buffer ) ) == s.GetLength( ) );
		//ASSERT( static_cast<int>( drive_name_length ) == s.GetLength( ) );

		const auto type = GetDriveTypeW( drive_name_buffer );
		if ( ( type == DRIVE_UNKNOWN ) || ( type == DRIVE_NO_ROOT_DIR ) ) {
			continue;
			}

		// The check of remote drives will be done in the background by the CDriveInformationThread.
		EnterCriticalSection( &_csRunningThreads );
		if ( ( type != DRIVE_REMOTE ) && ( !DriveExists( drive_name_buffer, drive_name_length ) ) ) {
			LeaveCriticalSection( &_csRunningThreads );
			continue;
			}
		LeaveCriticalSection( &_csRunningThreads );
		//ASSERT( s.GetLength( ) < UINT16_MAX );
		ASSERT( drive_name_length < UINT16_MAX );
		//const auto new_name_length = static_cast<rsize_t>( s.GetLength( ) );
		//ASSERT( new_name_length < UINT16_MAX );
		//ASSERT( drive_name_length == new_name_length );

		PWSTR new_name_ptr = nullptr;
		//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, s.GetString( ) );
		const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, drive_name_length, drive_name_buffer );
		ASSERT( SUCCEEDED( copy_res ) );
		if ( !SUCCEEDED( copy_res ) ) {
			displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (buildSelectList)(aborting!)" );
			displayWindowsMsgBoxWithMessage( drive_name_buffer );
			std::terminate( );
			}
		else {
			ASSERT( drive_name_length < UINT16_MAX );
			ASSERT( wcscmp( new_name_ptr, drive_name_buffer ) == 0 );
			const auto item = new CDriveItem { new_name_ptr, static_cast< std::uint16_t >( drive_name_length ) };
			m_list.InsertListItem( m_list.GetItemCount( ), item );

			new CDriveInformationThread { item->m_path, reinterpret_cast< LPARAM >( item ), m_hWnd, _serial, static_cast< rsize_t >( i ) };// (will delete itself when finished.)
			//item->StartQuery( m_hWnd, _serial );


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
	
	const UINT flags = BIF_RETURNONLYFSDIRS bitor BIF_USENEWUI bitor BIF_NONEWFOLDERBUTTON;
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

_Pre_defensive_ void CSelectDrivesDlg::OnOK( ) {
	TRACE( _T( "User hit ok...\r\n" ) );
	VERIFY( UpdateData( ) );

	m_drives.        clear( );
	m_selectedDrives.clear( );

	if ( m_radio == RADIO_AFOLDER ) {

		const rsize_t full_path_buffer_size = 128;
		wchar_t full_path_buffer[ full_path_buffer_size ] = { 0 };
		rsize_t chars_written = 0;
	
		const HRESULT path_res = GetFullPathName_WriteToStackBuffer( m_folder_name_heap.c_str( ), full_path_buffer, full_path_buffer_size, chars_written );

		if ( SUCCEEDED( path_res ) ) {
			m_folder_name_heap = full_path_buffer;
			}
		else {
			const auto folder_path = dynamic_GetFullPathName( m_folder_name_heap.c_str( ) );
			m_folder_name_heap = folder_path.c_str( );
			}

		//m_folderName = m_folder_name_heap.c_str( );

		//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
		TRACE( _T( "MyGetFullPathName( m_folder_name_heap ): %s\r\n" ), m_folder_name_heap.c_str( ) );
		VERIFY( UpdateData( false ) );
		}
	else {
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
	//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
	CPersistence::SetSelectDrivesRadio ( m_radio          );
	CPersistence::SetSelectDrivesFolder( m_folder_name_heap.c_str( ) );
	CPersistence::SetSelectDrivesDrives( m_selectedDrives );

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
	EnterCriticalSection( &_csRunningThreads );
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
	LeaveCriticalSection( &_csRunningThreads );
	
	log_GetDriveInformation_result( thread, name.c_str( ), total, free, success );

	//void log_GetDriveInformation_result( _In_ const CDriveInformationThread* const thread, _In_z_ PCWSTR name, _In_ const std::uint64_t total, _In_ const std::uint64_t )
	//const rsize_t buffer_size = 256;
	//wchar_t buffer_debug_out[ buffer_size ] = { 0 };
	//if ( success ) {
	//	TRACE( _T( "thread (%p)->GetDriveInformation succeeded!, name: %s, total: %I64u, free: %I64u\r\n" ), thread, name.c_str( ), total, free );
	//	const HRESULT pf_res_1 = StringCchPrintfW( buffer_debug_out, buffer_size, L"WDS: thread (%p)->GetDriveInformation succeeded!, name: %s, total: %I64u, free: %I64u\r\n", thread, name.c_str( ), total, free );
	//	ASSERT( SUCCEEDED( pf_res_1 ) );
	//	if ( SUCCEEDED( pf_res_1 ) ) {
	//		OutputDebugStringW( buffer_debug_out );
	//		}
	//	}
	//else {
	//	TRACE( _T( "thread (%p)->GetDriveInformation failed!, name: %s, total: %I64u, free: %I64u\r\n" ), thread, name.c_str( ), total, free );
	//	const HRESULT pf_res_1 = StringCchPrintfW( buffer_debug_out, buffer_size, L"WDS: thread (%p)->GetDriveInformation failed!, name: %s, total: %I64u, free: %I64u\r\n", thread, name.c_str( ), total, free );
	//	ASSERT( SUCCEEDED( pf_res_1 ) );
	//	if ( SUCCEEDED( pf_res_1 ) ) {
	//		OutputDebugStringW( buffer_debug_out );
	//		}
	//	}
	
	

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
	EnterCriticalSection( &_csRunningThreads );
	SetDriveInformation( item, success, std::move( name ), total, free );
	LeaveCriticalSection( &_csRunningThreads );
	VERIFY( m_list.RedrawItems( i, i ) );
	m_list.SortItems  (      );
	//TRACE( _T( "CSelectDrivesDlg::OnWmuThreadFinished\r\n") );
	//delete thread;
	//thread = NULL;
	return 0;//NULL??
	}


void CSelectDrivesDlg::OnGetMinMaxInfo( _Out_ MINMAXINFO* lpMMI ) {
	m_layout.OnGetMinMaxInfo( lpMMI );
	CDialog::OnGetMinMaxInfo( lpMMI );
	}

void CSelectDrivesDlg::OnDestroy( ) {
	EnterCriticalSection( &_csRunningThreads );
	for ( auto& aThread : vec_runningThreads ) {
		
		if ( aThread != nullptr ) {
			//EnterCriticalSection( &aThread->m_cs );
			aThread->m_dialog.store( nullptr );
			//LeaveCriticalSection( &aThread->m_cs );
			}
		
		}
	LeaveCriticalSection( &_csRunningThreads );
	//CDriveInformationThread::InvalidateDialogHandle( );
	m_layout.OnDestroy( );
	CDialog::OnDestroy( );
	}

void CSelectDrivesDlg::OnMeasureItem( const INT nIDCtl, PMEASUREITEMSTRUCT pMeasureItemStruct ) {
	if ( nIDCtl == IDC_DRIVES ) {
		pMeasureItemStruct->itemHeight = 20;
		}
	else {
		CDialog::OnMeasureItem( nIDCtl, pMeasureItemStruct );
		}
	}


void CSelectDrivesDlg::OnLvnItemchangedDrives( NMHDR* pNMHDR, LRESULT* pResult ) {
	UNREFERENCED_PARAMETER( pNMHDR );
	m_radio = RADIO_SOMEDRIVES;
	VERIFY( UpdateData( false ) );
	UpdateButtons( );
	*pResult = 0;
	}


void CSelectDrivesDlg::OnSysColorChange( ) {
	CDialog::OnSysColorChange();
	m_list.SysColorChanged();
	}

void CSelectDrivesDlg::OnSize( UINT nType, INT cx, INT cy ) {
	CDialog::OnSize( nType, cx, cy );
	m_layout.OnSize( );
	}



// $Log$
// Revision 1.18  2004/11/14 21:50:44  assarbad
// - Pre-select the last used folder
//
// Revision 1.13  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.12  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
