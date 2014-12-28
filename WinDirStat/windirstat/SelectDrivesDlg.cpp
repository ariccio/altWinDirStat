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
//#include "ownerdrawnlistcontrol.h"
#include "options.h"
#include "globalhelpers.h"


namespace {

	

	UINT WMU_THREADFINISHED = RegisterWindowMessageW( _T( "{F03D3293-86E0-4c87-B559-5FD103F5AF58}" ) );
	static CRITICAL_SECTION _csRunningThreads;
	_Guarded_by_( _csRunningThreads ) static std::map<CDriveInformationThread*, CDriveInformationThread*> map_runningThreads;

	// Return: false, if drive not accessible
	_Success_( return != false )
	bool RetrieveDriveInformation( _In_ const std::wstring& path, _Out_ std::wstring& name, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& total, _Out_ _Out_range_( 0, 18446744073709551615 ) std::uint64_t& free ) {
		CString volumeName;

		if ( !GetVolumeName( path.c_str( ), volumeName ) ) {
			name = L"GetVolumeName failed!";
			return false;
			}
		name = FormatVolumeName( path, std::wstring( volumeName.GetString( ) ) );
		MyGetDiskFreeSpace( path.c_str( ), total, free );
		ASSERT( free <= total );
		return true;
		}

	INT CALLBACK _BrowseCallbackProc_( _In_ HWND hWnd, _In_ UINT uMsg, LPARAM lParam, _In_ LPARAM pData ) {
		/*
		  Callback function for the dialog shown by SHBrowseForFolder()
		*/
		UNREFERENCED_PARAMETER( lParam );

		if ( uMsg == BFFM_INITIALIZED ) {
			::SendMessageW( hWnd, BFFM_SETSELECTION, TRUE, pData );
			}
		return 0;
		}

	void SetDriveInformation( _In_ CDriveItem* thisDriveItem, _In_ const bool success, _In_ std::wstring name, _In_ const std::uint64_t total, _In_ const std::uint64_t free ) {
		//m_success  = success;

		if ( success ) {
			thisDriveItem->m_name       = std::move( name );
			thisDriveItem->m_totalBytes = total;
			thisDriveItem->m_freeBytes  = free;
			thisDriveItem->m_used       = 0;

			if ( thisDriveItem->m_totalBytes > 0 ) {
				ASSERT( thisDriveItem->m_totalBytes >= thisDriveItem->m_freeBytes );
				thisDriveItem->m_used = static_cast<DOUBLE>( thisDriveItem->m_totalBytes - thisDriveItem->m_freeBytes ) / static_cast<DOUBLE>( thisDriveItem->m_totalBytes );
				}
			}
		else {
			thisDriveItem->m_totalBytes = UINT64_MAX;
			thisDriveItem->m_freeBytes  = UINT64_MAX;
			thisDriveItem->m_used       = -1;
			thisDriveItem->m_name       = std::move( name );
			}
		}

	
	}

/////////////////////////////////////////////////////////////////////////////
CDriveItem::CDriveItem( CDrivesList* const list, _In_ std::wstring pszPath ) : m_list( list ), m_path( pszPath ), /*m_success( false ),*/ m_totalBytes( 0 ), m_freeBytes( 0 ), m_used( -1 ), m_name( pszPath ) { }

INT CDriveItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const {
	const auto other = static_cast<const CDriveItem*>( baseOther );
	switch ( subitem )
	{
		case column::COL_NAME:
			return signum( m_path.compare( other->m_path ) );

		case column::COL_TOTAL:
			return signum( m_totalBytes - other->m_totalBytes );

		case column::COL_FREE:
			return signum( m_freeBytes - other->m_freeBytes );

		case column::COL_ATTRIBUTES:
		case column::COL_BYTES:
		case column::COL_BYTESPERCENT:
		case column::COL_FILES_TYPEVIEW:
	  //case column::COL_LASTCHANGE:
		default:
			ASSERT( false );
			return 0;
	}
	}

//TODO: check if ` _When_( ( subitem ==COL_NAME ) || (subitem == COL_GRAPH), _Out_opt_ ) ` is a valid/descriptive annotation for width
bool CDriveItem::DrawSubitem( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ _Deref_out_range_( 0, 100 ) INT* const width, _Inout_ INT* const focusLeft ) const {
	//ASSERT_VALID( pdc );
	if ( subitem == column::COL_NAME ) {
		DrawLabel( m_list, pdc, rc, state, width, focusLeft );
		return true;
		}
	if ( width != NULL ) {
		*width = 100;
		}
	return false;
	}

_Must_inspect_result_
HRESULT CDriveItem::Text_WriteToStackBuffer_COL_NAME( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchCopyW( psz_text, strSize, m_name.c_str( ) );
	size_t chars_remaining = 0;
	const auto res = StringCchCopyExW( psz_text, strSize, m_name.c_str( ), NULL, &chars_remaining, 0 );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = ( m_name.length( ) + 2 );
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			ASSERT( m_name.length( ) == wcslen( psz_text ) );
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
				
	return res;
	}

_Must_inspect_result_
HRESULT CDriveItem::Text_WriteToStackBuffer_COL_TOTAL( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	const auto res = FormatBytes( m_totalBytes, psz_text, strSize, chars_written );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	return res;
	}

_Must_inspect_result_
HRESULT CDriveItem::Text_WriteToStackBuffer_COL_FREE( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	const auto res = FormatBytes( m_freeBytes, psz_text, strSize, chars_written );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	return res;
	}

_Must_inspect_result_
HRESULT CDriveItem::Text_WriteToStackBuffer_default( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	ASSERT( false );
	ASSERT( strSize > 41 );
	if ( strSize > 41 ) {
		const auto res = StringCchPrintfW( psz_text, strSize, L"BAD GetText_WriteToStackBuffer - subitem" );
		chars_written = strSize;
		sizeBuffNeed = SIZE_T_MAX;
		chars_written = ( SUCCEEDED( res ) ? 41 : strSize );
		return res;
		}
	return STRSAFE_E_INVALID_PARAMETER;
	}

_Must_inspect_result_
HRESULT CDriveItem::Text_WriteToStackBuffer( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	switch ( subitem )
	{
			case column::COL_NAME:
				return Text_WriteToStackBuffer_COL_NAME( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_TOTAL:
				return Text_WriteToStackBuffer_COL_TOTAL( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_FREE:
				return Text_WriteToStackBuffer_COL_FREE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			
			case column::COL_ATTRIBUTES:
			case column::COL_ITEMS:
			default:
				return Text_WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	}
	}

std::wstring CDriveItem::Text( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const {
	switch ( subitem )
	{
		case column::COL_NAME:
			return m_name;

		case column::COL_FREE:
		case column::COL_TOTAL:
			//m_used != -1 -> success!
			ASSERT( m_used != -1 );
			if ( m_used != -1 ) {
				return FormatBytes( ( ( subitem == column::COL_TOTAL ) ? m_totalBytes : m_freeBytes ), GetOptions( )->m_humanFormat );
				}
			return _T( "" );

		default:
			ASSERT( false );
			return _T( "" );
	}
	}

void CDriveInformationThread::AddRunningThread( ) {
	//CSingleLock lock( &_csRunningThreads, true );
	EnterCriticalSection( &_csRunningThreads );
	map_runningThreads[ this ] = 0;
	LeaveCriticalSection( &_csRunningThreads );
	}

void CDriveInformationThread::RemoveRunningThread( ) {
	EnterCriticalSection( &_csRunningThreads );
	map_runningThreads.erase( this );
	LeaveCriticalSection( &_csRunningThreads );
	}

void CDriveInformationThread::InvalidateDialogHandle( ) {
	/*
	  This static method is called by the dialog when the dialog gets closed.
	  We set the m_dialog members of all running threads to null, so that they don't send messages around to a no-more-existing window.
	*/
	EnterCriticalSection( &_csRunningThreads );
	for ( auto& aThread : map_runningThreads ) {
		EnterCriticalSection( &aThread.first->m_cs );
		aThread.first->m_dialog = { NULL };
		LeaveCriticalSection( &aThread.first->m_cs );
		}
	LeaveCriticalSection( &_csRunningThreads );
	}

//void CDriveInformationThread::OnAppExit( ) {/*We need not do anything here.*/}


CDriveInformationThread::CDriveInformationThread( _In_ std::wstring path, LPARAM driveItem, HWND dialog, UINT serial ) : m_path( std::move( path ) ), m_driveItem( driveItem ), m_serial( serial ) {
	/*
	  The constructor starts the thread.
	*/
	InitializeCriticalSection( &m_cs );
	ASSERT( m_bAutoDelete );

	m_dialog     = dialog;
	m_totalBytes =      0;
	m_freeBytes  =      0;
	m_success    =  false;

	AddRunningThread( );

	VERIFY( CreateThread( ) );
	}

CDriveInformationThread::~CDriveInformationThread( ) {
	DeleteCriticalSection( &m_cs );
	//CWinThread::ExitInstance( );
	}

BOOL CDriveInformationThread::InitInstance( ) {
	EnterCriticalSection( &_csRunningThreads );
	m_success = RetrieveDriveInformation( m_path, m_name, m_totalBytes, m_freeBytes );
	LeaveCriticalSection( &_csRunningThreads );
	HWND dialog = { NULL };

		{
		EnterCriticalSection( &m_cs );
		dialog = m_dialog; // Of course, we must release m_cs here to avoid deadlocks.
		LeaveCriticalSection( &m_cs );
		}

	if ( dialog != NULL ) {
		/*
		  Theoretically the dialog may have been closed at this point. SendMessage() to a non-existing window simply fails immediately.
		  If in the meantime the system recycled the window handle, (it may even belong to another process now?!), we are safe, because WMU_THREADFINISHED is a unique registered message. (Well if the other process crashes because of our message, there is nothing we can do about it.)
		  If the window handle is recycled by a new Select drives dialog, its new serial will prevent it from reacting.
		  */
		TRACE( _T( "Sending WMU_THREADFINISHED! m_serial: %u\r\n" ), m_serial );
		SendMessageW( dialog, WMU_THREADFINISHED, m_serial, reinterpret_cast<LPARAM>( this ) );
		}
	RemoveRunningThread( );
	ASSERT( m_bAutoDelete ); // Object will delete itself.
	return false; // no Run(), please!
	}


LPARAM CDriveInformationThread::GetDriveInformation( _Out_ bool& success, _Out_ std::wstring& name, _Out_ std::uint64_t& total, _Out_ std::uint64_t& free ) const {
	/*
	  This method is only called by the gui thread, while we hang in SendMessage(dialog, WMU_THREADFINISHED, 0, this). So we need no synchronization.
	*/
	name    = m_name;
	total   = m_totalBytes;
	free    = m_freeBytes;
	success = m_success;
	return m_driveItem;
	}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CDrivesList, COwnerDrawnListCtrl )

void CDrivesList::OnLButtonDown( const UINT /*nFlags*/, const CPoint /*point*/ ) {
	if ( GetFocus( ) == this || GetSelectedCount( ) == 0 ) { // We simulate Ctrl-Key-Down here, so that the dialog can be driven with one hand (mouse) only.
		const auto msg = GetCurrentMessage( );
		DefWindowProcW( msg->message, msg->wParam | MK_CONTROL, msg->lParam );
		}
	else {
		SetFocus( );
		// Send a LVN_ITEMCHANGED to the parent, so that it can update the radio button.
		auto lv = zeroInitNMLISTVIEW( );
		lv.hdr.hwndFrom = m_hWnd;
		lv.hdr.idFrom   = static_cast<UINT_PTR>( GetDlgCtrlID( ) );
		lv.hdr.code     = LVN_ITEMCHANGED;
		TRACE( _T( "Sending LVN_ITEMCHANGED ( via WM_NOTIFY ) to parent!\r\n" ) );
		GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), reinterpret_cast<LPARAM>( &lv ) );
		}
	}

void CDrivesList::OnLvnDeleteitem( NMHDR* pNMHDR, LRESULT* pResult ) {
	auto pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	delete GetItem( pNMLV->iItem );
	//DeleteItem( pNMLV->iItem );
	pNMLV->iItem = -1;
	pNMLV->iSubItem = 0;
	*pResult = 0;
	}

void CDrivesList::MeasureItem( PMEASUREITEMSTRUCT pMeasureItemStruct ) {
	pMeasureItemStruct->itemHeight = m_rowHeight;
	}

void CDrivesList::OnNMDblclk( NMHDR* /*pNMHDR*/, LRESULT* pResult ) {
	*pResult = 0;

	auto point = GetCurrentMessage( )->pt;
	ScreenToClient( &point );
	const auto i = HitTest( point );
	if ( i == -1 ) {
		return;
		}
	const auto item_count = GetItemCount( );
	for ( int k = 0; k < item_count; k++ ) {
		VERIFY( SetItemState( k, ( k == i ? LVIS_SELECTED : static_cast<UINT>( 0 ) ), LVIS_SELECTED ) );
		}
	TRACE( _T( "User double-clicked! Sending WMU_OK!\r\n" ) );
	GetParent( )->SendMessageW( WMU_OK );
	}

BEGIN_MESSAGE_MAP(CDrivesList, COwnerDrawnListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteitem)
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSelectDrivesDlg, CDialog)

UINT CSelectDrivesDlg::_serial;

CSelectDrivesDlg::CSelectDrivesDlg( CWnd* pParent /*=NULL*/ ) : CDialog( CSelectDrivesDlg::IDD, pParent ), m_layout( this, global_strings::select_drives_dialog_layout ), m_radio( RADIO_ALLLOCALDRIVES ) {
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
	DDX_Text( pDX, IDC_FOLDERNAME, m_folderName );
	DDX_Control( pDX, IDOK, m_okButton );
	}


BEGIN_MESSAGE_MAP(CSelectDrivesDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSEFOLDER, OnBnClickedBrowsefolder)
	ON_BN_CLICKED(IDC_AFOLDER, UpdateButtons)
	ON_BN_CLICKED(IDC_SOMEDRIVES, UpdateButtons)
	ON_EN_CHANGE(IDC_FOLDERNAME, UpdateButtons)
	ON_WM_MEASUREITEM()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DRIVES, OnLvnItemchangedDrives)
	ON_BN_CLICKED(IDC_ALLLOCALDRIVES, UpdateButtons)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_MESSAGE(WMU_OK, OnWmuOk)
	ON_REGISTERED_MESSAGE(WMU_THREADFINISHED, OnWmuThreadFinished)
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
	UpdateWindow(             );
	BringWindowToTop(         );
	VERIFY( SetForegroundWindow(      ) );
	}

void CSelectDrivesDlg::buildSelectList( ) {
	const auto drives = GetLogicalDrives( );
	INT i = 0;
	DWORD mask = 0x00000001;
	for ( i = 0; i < 32; i++, mask <<= 1 ) {
		if ( ( drives bitand mask ) == 0 ) {
			continue;
			}

		CString s;
		s.Format( _T( "%c:\\" ), i + _T( 'A' ) );

		const auto type = GetDriveTypeW( s );
		if ( ( type == DRIVE_UNKNOWN ) || ( type == DRIVE_NO_ROOT_DIR ) ) {
			continue;
			}

		// The check of remote drives will be done in the background by the CDriveInformationThread.
		EnterCriticalSection( &_csRunningThreads );
		if ( ( type != DRIVE_REMOTE ) && ( !DriveExists( s ) ) ) {
			LeaveCriticalSection( &_csRunningThreads );
			continue;
			}
		LeaveCriticalSection( &_csRunningThreads );
		const auto item = new CDriveItem { &m_list, std::wstring( s.GetString( ) ) };
		m_list.InsertListItem( m_list.GetItemCount( ), item );
		
		new CDriveInformationThread { item->m_path, reinterpret_cast< LPARAM >( item ), m_hWnd, _serial };// (will delete itself when finished.)
		//item->StartQuery( m_hWnd, _serial );


		for ( size_t k = 0; k < m_selectedDrives.size( ); k++ ) {
			ASSERT( item->m_path.length( ) > 1 );
			if ( item->m_path.substr( 0, 2 ) == m_selectedDrives.at( k ) ) {
				m_list.SelectItem( item );
				break;
				}
			}
		}
	}

BOOL CSelectDrivesDlg::OnInitDialog( ) {
	CWaitCursor wc;
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

	m_folderName = CPersistence::GetSelectDrivesFolder( );
	
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
	const wchar_t bobtitle[ ] = L"WinDirStat - Select Folder";
	const UINT flags = BIF_RETURNONLYFSDIRS bitor BIF_USENEWUI bitor BIF_NONEWFOLDERBUTTON;
	WTL::CFolderDialog bob { NULL, bobtitle, flags};
	bob.SetInitialFolder( m_folderName );
	auto resDoModal = bob.DoModal( );
	if ( resDoModal == IDOK ) {
		m_folderName = bob.GetFolderPath( );
		TRACE( _T( "User chose: %s\r\n" ), m_folderName );
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
		m_folderName = MyGetFullPathName( m_folderName );
		TRACE( _T( "MyGetFullPathName( m_folderName ): %s\r\n" ), m_folderName );
		VERIFY( UpdateData( false ) );
		}
	else {
		for ( INT i = 0; i < m_list.GetItemCount( ); i++ ) {
			const auto item = m_list.GetItem( i );
			if (    ( m_radio == RADIO_ALLLOCALDRIVES          ) && 
				  //( !item->m_isRemote                        ) && 
					( !IsSUBSTedDrive( item->m_path.c_str( ) ) ) ||
					( m_radio == RADIO_SOMEDRIVES              ) && 
					( m_list.IsItemSelected( i )               )
					                                           ) {
				ASSERT( item->m_path.length( ) > 1 );
				m_drives.emplace_back( item->m_path.substr( 0, 2 ) );
				}
			if ( m_list.IsItemSelected( i ) ) {
				ASSERT( item->m_path.length( ) > 1 );
				m_selectedDrives.emplace_back( item->m_path.substr( 0, 2 ) );
				}
			}
		}
	CPersistence::SetSelectDrivesRadio ( m_radio          );
	CPersistence::SetSelectDrivesFolder( m_folderName     );
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
				if ( !m_folderName.IsEmpty( ) ) {
					if ( m_folderName.GetLength( ) >= 2 && m_folderName.Left( 2 ) == L"\\\\" ) {
						enableOk = TRUE;
						}
					else {
						CString pattern = m_folderName;
						if ( pattern.Right( 1 ) != _T( "\\" ) ) {
							pattern += _T( "\\" );
							}
						pattern += _T( "*.*" );
						CFileFind finder;
						const BOOL b = finder.FindFile( pattern );
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
	bool success = false;
	std::wstring name;
	std::uint64_t total = 0;
	std::uint64_t free = 0;
	EnterCriticalSection( &_csRunningThreads );
	auto driveItem = thread->GetDriveInformation( success, name, total, free );
	LeaveCriticalSection( &_csRunningThreads );
	//underscore after `inf` so that one of my VS extensions doesn't color the output text
	if ( success ) {
		TRACE( _T( "thread (%p)->GetDriveInformation succeeded!, name: %s, total: %I64u, free: %I64u\r\n" ), thread, name.c_str( ), total, free );
		}
	else {
		TRACE( _T( "thread (%p)->GetDriveInformation failed!, name: %s, total: %I64u, free: %I64u\r\n" ), thread, name.c_str( ), total, free );
		}
	
	

	// For paranoia's sake we check, whether driveItem is in our list. (and we so find its index.)
	auto fi = zeroInitLVFINDINFO( );
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

CDrivesList::CDrivesList( ) : COwnerDrawnListCtrl( global_strings::drives_str, 20 ) { }

const CDriveItem* CDrivesList::GetItem( const INT i ) const {
	return reinterpret_cast< CDriveItem * > ( GetItemData( i ) );
	}


const bool CDrivesList::IsItemSelected( const INT i ) const {
	return ( LVIS_SELECTED == GetItemState( i, LVIS_SELECTED ) );
	}


void CDrivesList::SelectItem( _In_ const CDriveItem* const item ) {
	auto i = FindListItem( item );
	VERIFY( SetItemState( i, LVIS_SELECTED, LVIS_SELECTED ) );
	}

void CSelectDrivesDlg::OnGetMinMaxInfo( _Out_ MINMAXINFO* lpMMI ) {
	m_layout.OnGetMinMaxInfo( lpMMI );
	CDialog::OnGetMinMaxInfo( lpMMI );
	}

void CSelectDrivesDlg::OnDestroy( ) {
	CDriveInformationThread::InvalidateDialogHandle( );
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
// Revision 1.21  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.20  2004/12/31 16:01:42  bseifert
// Bugfixes. See changelog 2004-12-31.
//
// Revision 1.19  2004/12/19 10:52:39  bseifert
// Minor fixes.
//
// Revision 1.18  2004/11/14 21:50:44  assarbad
// - Pre-select the last used folder
//
// Revision 1.17  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.16  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.15  2004/11/12 16:54:43  assarbad
// - Corrected a comment which actually refers to ExitProcess() but to TerminateProcess() as previously assumed.
//
// Revision 1.14  2004/11/12 13:19:44  assarbad
// - Minor changes and additions (in preparation for the solution of the "Browse for Folder" problem)
//
// Revision 1.13  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.12  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.11  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
