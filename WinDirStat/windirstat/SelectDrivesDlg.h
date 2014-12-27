// SelectDrivesDlg.h	- Declaration of CDriveItem, CDrivesList and CSelectDrivesDlg
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


#ifndef SELECTDRIVESDLG_H
#define SELECTDRIVESDLG_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"
#include "ownerdrawnlistcontrol.h"
#include "layout.h"

class CDrivesList;
class CDriveItem;

// CDriveItem. An item in the CDrivesList Control. All methods are called by the gui thread.
class CDriveItem : public COwnerDrawnListItem {
public:
	CDriveItem                ( CDrivesList* const list,             _In_ std::wstring pszPath );
	virtual ~CDriveItem( ) { }

private:
	virtual INT Compare       ( _In_ const COwnerDrawnListItem* const other, _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const override final;
	virtual bool DrawSubitem  ( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ _Deref_out_range_( 0, 100 ) INT* const width, _Inout_ INT* const focusLeft ) const override final;

	virtual std::wstring Text( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const override final;

	_Must_inspect_result_
	virtual HRESULT      Text_WriteToStackBuffer( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const override final;

	_Must_inspect_result_
	        HRESULT      Text_WriteToStackBuffer_COL_NAME( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

	_Must_inspect_result_
	        HRESULT      Text_WriteToStackBuffer_COL_TOTAL( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

	_Must_inspect_result_
	        HRESULT      Text_WriteToStackBuffer_COL_FREE( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

	_Must_inspect_result_
	        HRESULT      Text_WriteToStackBuffer_default( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;


public:
											 CDrivesList*      m_list; // Backpointer
									   const std::wstring      m_path; // e.g. "C:\"
										     std::wstring      m_name; // e.g. "BOOT (C:)"

	//18446744073709551615 is the maximum theoretical size of an NTFS file              according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t     m_totalBytes; // Capacity
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t     m_freeBytes;  // Free space
	_Field_range_( 0, 1 )                    DOUBLE            m_used;       // used space / total space
	};


// CDriveInformationThread. Does the GetVolumeInformation() call, which may hang for ca. 30 sec, it a network drive is not accessible.
class CDriveInformationThread : public CWinThread {
	// Set of all running CDriveInformationThreads.
	// Used by InvalidateDialogHandle().

	// The objects register and deregister themselves in _runningThreads
	void AddRunningThread              ( );
	void RemoveRunningThread           ( );

public:
	static void InvalidateDialogHandle ( );
	//static void OnAppExit( ) { }

	CDriveInformationThread            ( _In_  std::wstring path,            LPARAM   driveItem,       HWND           dialog,        UINT           serial    );
	LPARAM GetDriveInformation         ( _Out_ bool&  success, _Out_ std::wstring& name,    _Out_ std::uint64_t& total, _Out_ std::uint64_t& free ) const;

	virtual ~CDriveInformationThread( );

	virtual BOOL InitInstance          ( ) override final;
	
	

private:
	                                   const std::wstring       m_path;         // Path like "C:\"
	                                   const LPARAM             m_driveItem;    // The list item, we belong to
	                                         CRITICAL_SECTION   m_cs;           // for m_dialog
	_Guarded_by_( m_cs )                     HWND               m_dialog;
	                                   const UINT               m_serial;       // serial number of m_dialog
	// "[out]"-parameters
	                                         std::wstring       m_name;         // Result: name like "BOOT (C:)", valid if m_success
	
	//18446744073709551615 is the maximum theoretical size of an NTFS file              according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t      m_totalBytes;   // Result: capacity of the drive, valid if m_success
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t      m_freeBytes;    // Result: free space on the drive, valid if m_success
	                                         bool               m_success;      // Result: false, iff drive is unaccessible.
	};

class CDrivesList : public COwnerDrawnListCtrl {
	DECLARE_DYNAMIC(CDrivesList)
public:
	CDrivesList( );
	const CDriveItem* GetItem( const INT i ) const;

	void SelectItem( _In_ const CDriveItem* const item );
	const bool IsItemSelected( const INT i ) const;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown( const UINT nFlags, const CPoint point );
	afx_msg void OnLvnDeleteitem( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void MeasureItem( PMEASUREITEMSTRUCT pMeasureItemStruct );//const
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	};

//
// CSelectDrivesDlg. The initial dialog, where the user can select 
// one or more drives or a folder for scanning.
class CSelectDrivesDlg : public CDialog {
	DECLARE_DYNAMIC( CSelectDrivesDlg )
	enum {
		IDD = IDD_SELECTDRIVES
		};

public:
	CSelectDrivesDlg( CWnd* pParent = NULL );
	virtual ~CSelectDrivesDlg();

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
	       int                       m_radio;       // out.
	       CString                   m_folderName;  // out. Valid if m_radio = RADIO_AFOLDER
	       std::vector<std::wstring> m_drives;	    // out. Valid if m_radio != RADIO_AFOLDER

protected:
	static UINT                      _serial;       // Each Instance of this dialog gets a serial number
	       CDrivesList               m_list;
	       CButton                   m_okButton;
	       std::vector<std::wstring> m_selectedDrives;
	       CLayout                   m_layout;

	// Callback function for the dialog shown by SHBrowseForFolder()
	// MUST be static!
	//static INT CALLBACK BrowseCallbackProc( _In_ HWND hWnd, _In_ UINT uMsg, LPARAM lParam, _In_ LPARAM pData);

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

// $Log$
// Revision 1.11  2004/11/14 21:50:44  assarbad
// - Pre-select the last used folder
//
// Revision 1.10  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.9  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.8  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.7  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.6  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
