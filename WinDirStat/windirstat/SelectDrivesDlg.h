// SelectDrivesDlg.h	- Declaration of CDriveItem, CDrivesList and CSelectDrivesDlg
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_SELECTDRIVESDLG_H
#define WDS_SELECTDRIVESDLG_H


#include "ownerdrawnlistcontrol.h"
#include "layout.h"

class CDrivesList;
class CDriveItem;

// CDriveItem. An item in the CDrivesList Control. All methods are called by the gui thread.
class CDriveItem final : public COwnerDrawnListItem {
public:
	CDriveItem                ( _In_z_ PCWSTR name, const std::uint16_t length ) : m_path( name ), m_totalBytes( 0 ), m_freeBytes( 0 ), m_used( -1 ), COwnerDrawnListItem( name, length ) { }
	CDriveItem( const CDriveItem& in ) = delete;
	CDriveItem& operator=( const CDriveItem& in ) = delete;

	virtual ~CDriveItem( ) final = default;
	
	virtual COLORREF ItemTextColor( ) const override final {
		return default_item_text_color( );
		}

private:
	//concrete_compare is called as a single line INSIDE a single line function. Let's ask for inlining.
	inline  INT concrete_compare ( _In_ const CDriveItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const;
	virtual INT Compare       ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const override final;
	virtual bool DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ _Deref_out_range_( 0, 100 ) INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const override final {
		UNREFERENCED_PARAMETER( pdc );
		UNREFERENCED_PARAMETER( state );
		UNREFERENCED_PARAMETER( rc );
		UNREFERENCED_PARAMETER( focusLeft );
		UNREFERENCED_PARAMETER( subitem );
		UNREFERENCED_PARAMETER( list );
		if ( width != NULL ) {
			*width = 100;
			}
		return false;

		}

	_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
	virtual HRESULT      Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const override final;

public:
									   const std::wstring      m_path; // e.g. "C:\"

	//18446744073709551615 is the maximum theoretical size of an NTFS file              according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t     m_totalBytes; // Capacity
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t     m_freeBytes;  // Free space
	_Field_range_( 0, 1 )                    DOUBLE            m_used;       // used space / total space
	};


// CDriveInformationThread. Does the GetVolumeInformation() call, which may hang for ca. 30 sec, it a network drive is not accessible.
class CDriveInformationThread final : public CWinThread {

public:
	CDriveInformationThread& operator=( const CDriveInformationThread& in ) = delete;
	CDriveInformationThread( const CDriveInformationThread& in ) = delete;

	CDriveInformationThread( _In_  std::wstring path, LPARAM   driveItem, HWND           dialog, UINT           serial, rsize_t thread_num );
	virtual ~CDriveInformationThread( ) final = default;
	virtual BOOL InitInstance          ( ) override final;
	
	

public:
						const std::wstring              m_path;         // Path like "C:\"
						const LPARAM                    m_driveItem;    // The list item, we belong to
						std::atomic<HWND>               m_dialog;
						//C4820: 'CDriveInformationThread' : '4' bytes padding added after data member 'CDriveInformationThread::m_serial'
						const UINT                      m_serial;       // serial number of m_dialog
						const rsize_t                   m_threadNum;
	// "[out]"-parameters
						std::atomic<PWSTR>              m_name;         // Result: name like "BOOT (C:)", valid if m_success
						std::atomic<std::uint64_t>      m_totalBytes;   // Result: capacity of the drive, valid if m_success
						std::atomic<std::uint64_t>      m_freeBytes;    // Result: free space on the drive, valid if m_success
						//C4820: 'CDriveInformationThread' : '7' bytes padding added after data member 'CDriveInformationThread::m_success'
						std::atomic<bool>               m_success;      // Result: false, iff drive is unaccessible.

	};

class CDrivesList final : public COwnerDrawnListCtrl {
	DECLARE_DYNAMIC(CDrivesList)
public:
	CDrivesList( );

	_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
	const CDriveItem* GetItem( _In_ _In_range_( 0, INT_MAX ) const int i ) const;

	CDrivesList& operator=( const CDrivesList& in ) = delete;
	CDrivesList( const CDrivesList& in ) = delete;

	void SysColorChanged( ) {
		InitializeColors( );
		}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown( const UINT nFlags, const CPoint point );
	afx_msg void OnLvnDeleteitem( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void MeasureItem( PMEASUREITEMSTRUCT pMeasureItemStruct );//const
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	};

// CSelectDrivesDlg. The initial dialog, where the user can select one or more drives or a folder for scanning.
class CSelectDrivesDlg final : public CDialog {
	DECLARE_DYNAMIC( CSelectDrivesDlg )
	enum {
		IDD = IDD_SELECTDRIVES
		};

public:
	CSelectDrivesDlg( CWnd* pParent = NULL );
	virtual ~CSelectDrivesDlg( ) final;

	CSelectDrivesDlg& operator=( const CSelectDrivesDlg& in ) = delete;
	CSelectDrivesDlg( const CSelectDrivesDlg& in ) = delete;

protected:
	_Pre_defensive_ virtual void DoDataExchange ( CDataExchange* pDX ) override final;
	                virtual BOOL OnInitDialog   (                    ) override final;
	_Pre_defensive_ virtual void OnOK           (                    ) override final;


	void buildSelectList( );
	void initWindow( );
	void addControls( );
	void insertColumns( );
	void setListOptions( );

	_Pre_defensive_ void UpdateButtons          (                    );

public:
	       // Dialog Data
	       //C4820: 'CSelectDrivesDlg' : '4' bytes padding added after data member 'CSelectDrivesDlg::m_radio'
	       int                       m_radio;       // out.
		   std::wstring              m_folder_name_heap;// out. Valid if m_radio = RADIO_AFOLDER
	       std::vector<std::wstring> m_drives;	    // out. Valid if m_radio != RADIO_AFOLDER

protected:
	static UINT                      _serial;       // Each Instance of this dialog gets a serial number
	       CDrivesList               m_list;
	       CButton                   m_okButton;
	       std::vector<std::wstring> m_selectedDrives;
	       CLayout                   m_layout;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBrowsefolder();
	afx_msg void OnLbnSelchangeDrives();
	afx_msg void OnMeasureItem( const INT nIDCtl, PMEASUREITEMSTRUCT pMeasureItemStruct );
	afx_msg void OnLvnItemchangedDrives( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnSize( UINT nType, INT cx, INT cy );
	afx_msg void OnGetMinMaxInfo( _Out_ MINMAXINFO* lpMMI );
	afx_msg void OnDestroy( );

	afx_msg LRESULT OnWmuOk( const WPARAM, const LPARAM ) {
		OnOK( );
		return 0;
		}

	afx_msg _Function_class_( "GUI_THREAD" ) LRESULT OnWmuThreadFinished( const WPARAM, const LPARAM lparam );
	
	afx_msg void OnSysColorChange( );

	};


#else


#endif
