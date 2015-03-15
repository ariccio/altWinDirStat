// pagegeneral.h	- Declaration of CPageGeneral
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"
//#include "windirstat.h"

#ifndef WDS_PAGEGENERAL_H
#define WDS_PAGEGENERAL_H

WDS_FILE_INCLUDE_MESSAGE


class COptionsPropertySheet;
class CPageGeneral;
class CDirstatApp;

// CPageGeneral. "Settings" property page "General".
class CPageGeneral final : public CPropertyPage {
	DECLARE_DYNAMIC(CPageGeneral)
	enum {
		IDD = IDD_PAGE_GENERAL
		};

public:

	CPageGeneral& operator=( const CPageGeneral& in ) = delete;
	CPageGeneral( const CPageGeneral& in ) = delete;

	CPageGeneral( CDirstatApp* app ) : CPropertyPage( CPageGeneral::IDD ), m_followMountPoints( FALSE ), m_followJunctionPoints( FALSE ), m_humanFormat( FALSE ), m_listGrid( FALSE ), m_listStripes( FALSE ), m_listFullRowSelection( FALSE ),  m_appptr( app ) { }

protected:

	virtual void DoDataExchange( CDataExchange* pDX ) override final {
		CWnd::DoDataExchange( pDX );
		DDX_Check   ( pDX, IDC_HUMANFORMAT,       m_humanFormat             );
		DDX_Check   ( pDX, IDC_FOLLOWMOUNTPOINTS, m_followMountPoints       );
		DDX_Check   ( pDX, IDC_FOLLOWJUNCTIONS,   m_followJunctionPoints    );
		DDX_Control ( pDX, IDC_FOLLOWMOUNTPOINTS, m_ctlFollowMountPoints    );
		DDX_Control ( pDX, IDC_FOLLOWJUNCTIONS,   m_ctlFollowJunctionPoints );
		DDX_Check   ( pDX, IDC_SHOWGRID,          m_listGrid                );
		DDX_Check   ( pDX, IDC_SHOWSTRIPES,       m_listStripes             );
		DDX_Check   ( pDX, IDC_FULLROWSELECTION,  m_listFullRowSelection    );
		}

	virtual BOOL OnInitDialog   (                    ) override final;
	virtual void OnOK           (                    ) override final;

	BOOL      m_followMountPoints;
	BOOL      m_followJunctionPoints;
	BOOL      m_humanFormat;
	BOOL      m_listGrid;
	BOOL      m_listStripes;
	BOOL      m_listFullRowSelection;

	CButton   m_ctlFollowMountPoints;
	CButton   m_ctlFollowJunctionPoints;
	CDirstatApp* m_appptr;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAnyOption( ) {
		SetModified( );
		}

	_Must_inspect_result_ COptionsPropertySheet* GetSheet( );

	};


#else

#endif
