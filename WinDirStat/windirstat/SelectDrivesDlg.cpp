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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{

	
	UINT WMU_THREADFINISHED = RegisterWindowMessage(_T("{F03D3293-86E0-4c87-B559-5FD103F5AF58}"));
	static CRITICAL_SECTION _csRunningThreads;
	_Guarded_by_( _csRunningThreads ) static std::map<CDriveInformationThread*, CDriveInformationThread*> map_runningThreads;

	// Return: false, if drive not accessible
	bool RetrieveDriveInformation( _In_z_ const PCWSTR path, _Inout_ CString& name, _Inout_ std::uint64_t& total, _Inout_ std::uint64_t& free ) {
		CString volumeName;

		if ( !GetVolumeName( path, volumeName ) ) {
			return false;
			}
		name = FormatVolumeName( path, volumeName );
		MyGetDiskFreeSpace( path, total, free );
		ASSERT( free <= total );
		return true;
		}

}

/////////////////////////////////////////////////////////////////////////////
CDriveItem::CDriveItem( CDrivesList* const list, _In_z_ PCWSTR pszPath ) : m_list( list ), m_path( pszPath ), m_success( false ), m_totalBytes( 0 ), m_freeBytes( 0 ), m_used( 0 ), m_name( std::move( pszPath ) ), m_querying( true ) {
	m_isRemote   = ( DRIVE_REMOTE == GetDriveTypeW( m_path.c_str( ) ) );
	}

_Pre_satisfies_( this->m_querying ) void CDriveItem::StartQuery( _In_ const HWND dialog, _In_ const UINT serial ) {
	if ( m_querying ) {
		new CDriveInformationThread { m_path.c_str( ), reinterpret_cast< LPARAM >( this ), dialog, serial };// (will delete itself when finished.)
		}
	}

void CDriveItem::SetDriveInformation( _In_ const bool success, _In_z_ const PCWSTR name, _In_ const std::uint64_t total, _In_ const std::uint64_t free ) {
	m_querying = false;
	m_success  = success;

	if ( m_success ) {
		m_name       = name;
		m_totalBytes = total;
		m_freeBytes  = free;
		m_used       = 0;

		if ( m_totalBytes > 0 ) {
			m_used = DOUBLE( m_totalBytes - m_freeBytes ) / DOUBLE( m_totalBytes );
			}
		}
	}

INT CDriveItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, _In_ _In_range_( 0, 7 ) const INT subitem ) const {
	const auto other = static_cast<const CDriveItem*>( baseOther );
	switch ( subitem )
	{
		case COL_NAME:
			return signum( m_path.compare( other->m_path ) );

		case COL_TOTAL:
			return signum( m_totalBytes - other->m_totalBytes );

		case COL_FREE:
			return signum( m_freeBytes - other->m_freeBytes );

		case COL_GRAPH:
		case COL_PERCENTUSED:
			return signum( m_used - other->m_used );

		default:
			ASSERT( false );
			return 0;
	}
	}

//TODO: check if ` _When_( ( subitem ==COL_NAME ) || (subitem == COL_GRAPH), _Out_opt_ ) ` is a valid/descriptive annotation for width
bool CDriveItem::DrawSubitem( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ _Deref_out_range_( 0, 100 ) INT* const width, _Inout_ INT* const focusLeft ) const {
	//ASSERT_VALID( pdc );
	if ( subitem == COL_NAME ) {
		DrawLabel( m_list, nullptr, pdc, rc, state, width, focusLeft );
		return true;
		}
	else if ( subitem == COL_GRAPH ) {
		if ( !m_success ) {
			
			if ( width != NULL ) {
				//Does this make sense?
				*width = 0;
				}
			return false;
			}
		if ( width != NULL ) {
			*width = 100;
			return true;
			}

		DrawSelection( m_list, pdc, rc, state );
		rc.DeflateRect( 3, 5 );
		DrawPercentage( pdc, rc, m_used, RGB( 0, 0, 170 ) );
		return true;
		}
	else {//COL_TOTAL, COL_FREE, COL_PERCENTUSED, COLUMN_COUNT
		if ( width != NULL ) {
			*width = 100;
			}
		}
	return false;
	}

HRESULT CDriveItem::Text_WriteToStackBuffer( _In_range_( 0, 7 ) const INT subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_text, rsize_t strSize, rsize_t& sizeBuffNeed ) const {
	switch ( subitem )
	{
			case COL_NAME:
				{
				auto res = StringCchCopyW( psz_text, strSize, m_name.c_str( ) );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = ( m_name.length( ) + 2 );
					}
				return res;
				}
			case COL_TOTAL:
				{
				auto res = FormatBytes( m_totalBytes, psz_text, strSize );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = 64;//Generic size needed.
					}
				return res;
				}
			case COL_FREE:
				{
				auto res = FormatBytes( m_freeBytes, psz_text, strSize );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = 64;//Generic size needed.
					}
				return res;
				}
			case COL_GRAPH:
				{
				ASSERT( strSize > 13 );
				auto res = StringCchPrintfW( psz_text, strSize, L"%s", ( ( m_querying ) ? ( L"(querying...)" ) : ( L"(unavailable)" ) ) );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = 15;//Generic size needed.
					}
				return res;
				}
			case COL_PERCENTUSED:
				{
				auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( m_used * 100 ) );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = 8;//Generic size needed, overkill;
					}
				return res;
				}
			default:
				{
				ASSERT( strSize > 8 );
				auto res = StringCchPrintfW( psz_text, strSize, L"BAD GetText_WriteToStackBuffer - subitem" );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					if ( strSize > 8 ) {
						write_BAD_FMT( psz_text );
						}
					else {
						displayWindowsMsgBoxWithMessage( std::wstring( L"CDriveItem::GetText_WriteToStackBuffer - SERIOUS ERROR!" ) );
						}
					}
				return res;
				}
	}
	}

std::wstring CDriveItem::Text( _In_ _In_range_( 0, 7 ) const INT subitem ) const {
	switch ( subitem )
	{
		case COL_NAME:
			return m_name;

		case COL_TOTAL:
			ASSERT( m_success );
			if ( m_success ) {
				return FormatBytes( m_totalBytes );
				}
			return _T( "" );

		case COL_FREE:
			ASSERT( m_success );
			if ( m_success ) {
				return FormatBytes( m_freeBytes );
				}
			return _T( "" );

		case COL_GRAPH:
			ASSERT( m_querying );
			if ( m_querying ) {
				return _T( "(querying...)" );
				}
			return _T( "(unavailable)" );

		case COL_PERCENTUSED:
			ASSERT( m_success );
			if ( m_success ) {
				const rsize_t strSize = 64;
				wchar_t percentUsed[ strSize ] = { 0 };

				auto fmt_res = CStyle_FormatDouble( m_used * 100, percentUsed, strSize );

				if ( fmt_res == S_OK ) {
					wchar_t percentage[ 2 ] = { '%', 0 };
					auto res = wcscat_s( percentUsed, strSize, percentage );
					if ( res == 0 ) {
						return percentUsed;
						}
					return _T( "BAD wcscat_s!!!!" );
					}

				//return FormatDouble( m_used * 100 ) + _T( "%" );
				return FormatDouble_w( m_used * 100 ) + _T( "%" );
				}
			return _T( "" );

		default:
			ASSERT( false );
			return _T( "" );
	}
	}



std::wstring CDriveItem::GetDrive( ) const {
	ASSERT( m_path.length( ) > 1 );
	return m_path.substr( 0, 2 );
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
		//CDriveInformationThread* thread;
		//CSingleLock lockObj( &aThread.first->m_cs, true );
		EnterCriticalSection( &aThread.first->m_cs );
		aThread.first->m_dialog = { NULL };
		LeaveCriticalSection( &aThread.first->m_cs );
		}
	LeaveCriticalSection( &_csRunningThreads );
	}

//void CDriveInformationThread::OnAppExit( ) {/*We need not do anything here.*/}


CDriveInformationThread::CDriveInformationThread( _In_z_ PCWSTR path, LPARAM driveItem, HWND dialog, UINT serial ) : m_path( path ), m_driveItem( driveItem ), m_serial( serial ) {
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

BOOL CDriveInformationThread::InitInstance( ) {
	EnterCriticalSection( &_csRunningThreads );
	m_success = RetrieveDriveInformation( m_path, m_name, m_totalBytes, m_freeBytes );
	LeaveCriticalSection( &_csRunningThreads );
	HWND dialog = { NULL };

		{
		//_Requires_lock_held_( m_cs );
		//CSingleLock lock( &m_cs, true );
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
		TRACE( _T( "Sending WMU_THREADFINISHED!\r\n" ) );
		SendMessage( dialog, WMU_THREADFINISHED, m_serial, ( LPARAM )this );
		}
	RemoveRunningThread( );
	ASSERT( m_bAutoDelete ); // Object will delete itself.
	return false; // no Run(), please!
	}


LPARAM CDriveInformationThread::GetDriveInformation( _Inout_ bool& success, _Inout_ CString& name, _Inout_ std::uint64_t& total, _Inout_ std::uint64_t& free ) {
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

IMPLEMENT_DYNAMIC( CDrivesList, COwnerDrawnListControl )

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
		lv.hdr.idFrom   = UINT_PTR( GetDlgCtrlID( ) );
		lv.hdr.code     = LVN_ITEMCHANGED;
		TRACE( _T( "Sending LVN_ITEMCHANGED ( via WM_NOTIFY ) to parent!\r\n" ) );
		GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), ( LPARAM ) &lv );
		}
	}

void CDrivesList::OnNMDblclk( NMHDR* /*pNMHDR*/, LRESULT* pResult ) {
	*pResult = 0;

	auto point = GetCurrentMessage( )->pt;
	ScreenToClient( &point );
	auto i = HitTest( point );
	if ( i == -1 ) {
		return;
		}
	for ( INT k = 0; k < GetItemCount( ); k++ ) {
		SetItemState( k, k == i ? LVIS_SELECTED : UINT( 0 ), LVIS_SELECTED );
		}
	TRACE( _T( "User double-clicked! Sending WMU_OK!\r\n" ) );
	GetParent( )->SendMessage( WMU_OK );
	}

BEGIN_MESSAGE_MAP(CDrivesList, COwnerDrawnListControl)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteitem)
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSelectDrivesDlg, CDialog)

UINT CSelectDrivesDlg::_serial;

CSelectDrivesDlg::CSelectDrivesDlg( CWnd* pParent /*=NULL*/ ) : CDialog( CSelectDrivesDlg::IDD, pParent ), m_layout( this, L"sddlg" ), m_radio( RADIO_ALLLOCALDRIVES ) {
	_serial++;
	InitializeCriticalSection( &_csRunningThreads );
	}

CSelectDrivesDlg::~CSelectDrivesDlg( ) {
	DeleteCriticalSection( &_csRunningThreads );
	}

_Pre_defensive_ void CSelectDrivesDlg::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_DRIVES, m_list );
	DDX_Radio( pDX, IDC_ALLDRIVES, m_radio );
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
	m_list.InsertColumn( COL_NAME,        _T( "Name" ),  LVCFMT_LEFT , 120, COL_NAME        );
	m_list.InsertColumn( COL_TOTAL,       _T( "Total" ),  LVCFMT_RIGHT,  55, COL_TOTAL       );
	m_list.InsertColumn( COL_FREE,        _T( "Free" ),  LVCFMT_RIGHT,  55, COL_FREE        );
	m_list.InsertColumn( COL_GRAPH,       _T( "Used/Total" ),  LVCFMT_LEFT , 100, COL_GRAPH       );
	m_list.InsertColumn( COL_PERCENTUSED, _T( "Used/Total" ),  LVCFMT_RIGHT,  55, COL_PERCENTUSED );
	}

void CSelectDrivesDlg::setListOptions( ) {
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		m_list.ShowGrid(             Options->m_listGrid );
		m_list.ShowStripes(          Options->m_listStripes );
		m_list.ShowFullRowSelection( Options->m_listFullRowSelection );
		}
	ASSERT( Options != NULL );
	}

void CSelectDrivesDlg::initWindow( ) {
	ShowWindow( SW_SHOWNORMAL );
	UpdateWindow(             );
	BringWindowToTop(         );
	SetForegroundWindow(      );
	}

void CSelectDrivesDlg::buildSelectList( ) {
	auto drives = GetLogicalDrives( );
	INT i = 0;
	DWORD mask = 0x00000001;
	for ( i = 0; i < 32; i++, mask <<= 1 ) {
		if ( ( drives & mask ) == 0 ) {
			continue;
			}

		CString s;
		s.Format( _T( "%c:\\" ), i + _T( 'A' ) );

		auto type = GetDriveTypeW( s );
		if ( type == DRIVE_UNKNOWN || type == DRIVE_NO_ROOT_DIR ) {
			continue;
			}

		// The check of remote drives will be done in the background by the CDriveInformationThread.
		EnterCriticalSection( &_csRunningThreads );
		if ( type != DRIVE_REMOTE && !DriveExists( s ) ) {
			LeaveCriticalSection( &_csRunningThreads );
			continue;
			}
		LeaveCriticalSection( &_csRunningThreads );
		auto item = new CDriveItem { &m_list, s };
		m_list.InsertListItem( m_list.GetItemCount( ), item );
		item->StartQuery( m_hWnd, _serial );
		for ( size_t k = 0; k < m_selectedDrives.size( ); k++ ) {
			if ( item->GetDrive( ) == m_selectedDrives.at( k ) ) {
				m_list.SelectItem( item );
				break;
				}
			}
		}
	}

BOOL CSelectDrivesDlg::OnInitDialog( ) {
	CWaitCursor wc;
	CDialog::OnInitDialog( );
	if ( WMU_THREADFINISHED == 0 ) {
		TRACE( "RegisterMessage() failed. Using WM_USER + 123\r\n" );
		WMU_THREADFINISHED = WM_USER + 123;
		}

	ModifyStyle( 0, WS_CLIPCHILDREN );
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
	UpdateData( false );

	switch ( m_radio )
	{
		case RADIO_ALLLOCALDRIVES:
		case RADIO_AFOLDER:
			m_okButton.SetFocus( );
			break;
		case RADIO_SOMEDRIVES:
			m_list.SetFocus( );
			break;
	}
	UpdateButtons( );
	return false; // we have set the focus.
	}

void CSelectDrivesDlg::OnBnClickedBrowsefolder( ) {
	
	CString sDisplayName, sSelectedFolder = m_folderName;
	auto bi = zeroInitBROWSEINFO( );
	bi.hwndOwner  = m_hWnd;
	
	bi.pszDisplayName = sDisplayName.GetBuffer( MAX_PATH );
	bi.lpszTitle      = L"WinDirStat - Select Folder";
	// Set a callback function to pre-select a folder
	bi.lpfn   = BFFCALLBACK( BrowseCallbackProc );
	bi.lParam = LPARAM( sSelectedFolder.GetBuffer( MAX_PATH ) );
	// Set the required flags
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
	
	LPITEMIDLIST pidl = SHBrowseForFolderW( &bi );
	sDisplayName.   ReleaseBuffer( );
	sSelectedFolder.ReleaseBuffer( );

	if ( pidl != NULL ) {
		

		LPSHELLFOLDER pshf = { NULL };
		HRESULT hr = SHGetDesktopFolder( &pshf );
		if ( !( SUCCEEDED( hr ) ) ) {
			CoTaskMemFree( pidl );
			displayWindowsMsgBoxWithError( );
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"SHGetDesktopFolder Failed!" ) ) );
			TRACE( _T( "SHGetDesktopFolder Failed!\r\n" ) );
			return;
			}

		ASSERT( SUCCEEDED( hr ) );
		
		STRRET strret;
		strret.uType = STRRET_CSTR;
		hr = pshf->GetDisplayNameOf( pidl, SHGDN_FORPARSING, &strret );
		ASSERT( SUCCEEDED( hr ) );
		if ( !( SUCCEEDED( hr ) ) ) {
			CoTaskMemFree( pidl );
			pshf->Release( );
			displayWindowsMsgBoxWithError( );
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"GetDisplayNameOf Failed!" ) ) );
			TRACE( _T( "GetDisplayNameOf Failed!\r\n" ) );
			return;
			}

		//CString sDir = MyStrRetToString( pidl, &strret );
		PTSTR strretStrPtr = { NULL };
		if ( StrRetToStr( &strret, NULL, &strretStrPtr ) != S_OK ) {
			CoTaskMemFree( pidl );
			pshf->Release( );
			displayWindowsMsgBoxWithError( );
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"StrRetToStr Failed!" ) ) );
			TRACE( _T( "StrRetToStr Failed!\r\n" ) );
			return;
			}
		
		//CString sDir( strret.cStr );

		CString sDir( strretStrPtr );
		CoTaskMemFree( strretStrPtr );
		CoTaskMemFree( pidl );
		pshf->Release( );
		m_folderName = sDir;
		m_radio = RADIO_AFOLDER;
		UpdateData( false );
		UpdateButtons( );
		}
	}

_Pre_defensive_ void CSelectDrivesDlg::OnOK( ) {
	UpdateData( );

	m_drives.        clear( );
	m_selectedDrives.clear( );

	if ( m_radio == RADIO_AFOLDER ) {
		m_folderName = MyGetFullPathName( m_folderName );
		TRACE( _T( "test: %i\r\n" ), int( m_drives.size( ) ) );
		UpdateData( false );
		}
	else {
		for ( INT i = 0; i < m_list.GetItemCount( ); i++ ) {
			auto item = m_list.GetItem( i );
			if (    ( m_radio == RADIO_ALLLOCALDRIVES ) && 
					( !item->m_isRemote )               && 
					( !IsSUBSTedDrive( item->m_path.c_str( ) ) ) ||
					( m_radio == RADIO_SOMEDRIVES )     && 
					( m_list.IsItemSelected( i ) ) ) {

				m_drives.emplace_back( item->GetDrive( ) );
				}
			if ( m_list.IsItemSelected( i ) ) {
				m_selectedDrives.emplace_back( item->GetDrive( ) );
				}
			}
		}
	CPersistence::SetSelectDrivesRadio ( m_radio          );
	CPersistence::SetSelectDrivesFolder( m_folderName     );
	CPersistence::SetSelectDrivesDrives( m_selectedDrives );

	CDialog::OnOK( );
	}

_Pre_defensive_ void CSelectDrivesDlg::UpdateButtons( ) {
	UpdateData( );
	BOOL enableOk = false;
	switch ( m_radio )
		{
			case RADIO_ALLLOCALDRIVES:
				enableOk = true;
				break;
			case RADIO_SOMEDRIVES:
				enableOk = ( m_list.GetSelectedCount( ) > 0 );
				break;
			case RADIO_AFOLDER:
				if ( !m_folderName.IsEmpty( ) ) {
					if ( m_folderName.GetLength( ) >= 2 && m_folderName.Left( 2 ) == L"\\\\" ) {
						enableOk = true;
						}
					else {
						CString pattern = m_folderName;
						if ( pattern.Right( 1 ) != _T( "\\" ) ) {
							pattern += _T( "\\" );
							}
						pattern += _T( "*.*" );
						CFileFind finder;
						BOOL b = finder.FindFile( pattern );
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
	if (serial != _serial) {
		TRACE(_T("Leaving OnWmuThreadFinished: invalid serial (window handle recycled?)\r\n"));
		return 0;
		}
	auto thread = reinterpret_cast<CDriveInformationThread *> ( lparam );
	bool success = false;
	CString name;
	std::uint64_t total = 0;
	std::uint64_t free  = 0;
	EnterCriticalSection( &_csRunningThreads );
	auto driveItem = thread->GetDriveInformation( success, name, total, free );
	LeaveCriticalSection( &_csRunningThreads );
	//underscore after `inf` so that one of my VS extensions doesn't color the output text
	TRACE( _T( "Got drive inf_ormation: success: %s, name: %s, total: %I64u, free: %I64u\r\n" ), ( success ? _T( "true" ) : _T( "false" ) ), name, total, free );
	

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
	item->SetDriveInformation( success, name, total, free );
	LeaveCriticalSection( &_csRunningThreads );
	m_list.RedrawItems( i, i );
	m_list.SortItems  (      );
	//TRACE( _T( "CSelectDrivesDlg::OnWmuThreadFinished\r\n") );
	return 0;//NULL??
	}

CDrivesList::CDrivesList( ) : COwnerDrawnListControl( _T( "drives" ), 20 ) { }

CDriveItem* CDrivesList::GetItem( const INT i ) const {
	return reinterpret_cast< CDriveItem * > ( GetItemData( i ) );
	}


bool CDrivesList::IsItemSelected( const INT i ) const {
	return ( LVIS_SELECTED == GetItemState( i, LVIS_SELECTED ) );
	}


void CDrivesList::SelectItem( _In_ CDriveItem* const item ) {
	auto i = FindListItem( item );
	SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
	}


INT CALLBACK CSelectDrivesDlg::BrowseCallbackProc( _In_ HWND hWnd, _In_ UINT uMsg, LPARAM lParam, _In_ LPARAM pData ) {
	/*
	  Callback function for the dialog shown by SHBrowseForFolder()
	*/
	UNREFERENCED_PARAMETER(lParam);
	
	switch( uMsg )
	{
		case BFFM_INITIALIZED:
			::SendMessageW( hWnd, BFFM_SETSELECTION, TRUE, pData );
			break;
	}
	return 0;
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
	UpdateData( false );
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


void CDrivesList::MeasureItem( PMEASUREITEMSTRUCT pMeasureItemStruct ) {
	pMeasureItemStruct->itemHeight = m_rowHeight;
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
