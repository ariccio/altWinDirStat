// pagegeneral.h	- Declaration of CPageGeneral
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

#pragma once

#include "stdafx.h"

#ifndef WDS_PAGEGENERAL_H
#define WDS_PAGEGENERAL_H





class COptionsPropertySheet;
class CPageGeneral;

// CPageGeneral. "Settings" property page "General".
class CPageGeneral final : public CPropertyPage {
	DECLARE_DYNAMIC(CPageGeneral)
	enum {
		IDD = IDD_PAGE_GENERAL
		};

public:

	CPageGeneral& operator=( const CPageGeneral& in ) = delete;
	CPageGeneral( const CPageGeneral& in ) = delete;

	CPageGeneral( ) : CPropertyPage( CPageGeneral::IDD ), m_followMountPoints( FALSE ), m_followJunctionPoints( FALSE ), m_humanFormat( FALSE ), m_listGrid( FALSE ), m_listStripes( FALSE ), m_listFullRowSelection( FALSE ), m_showTimeSpent( FALSE ) { }

protected:

	virtual void DoDataExchange( CDataExchange* pDX ) override final {
		CPropertyPage::DoDataExchange( pDX );
		DDX_Check   ( pDX, IDC_HUMANFORMAT,       m_humanFormat             );
		DDX_Check   ( pDX, IDC_FOLLOWMOUNTPOINTS, m_followMountPoints       );
		DDX_Check   ( pDX, IDC_FOLLOWJUNCTIONS,   m_followJunctionPoints    );
		DDX_Control ( pDX, IDC_FOLLOWMOUNTPOINTS, m_ctlFollowMountPoints    );
		DDX_Control ( pDX, IDC_FOLLOWJUNCTIONS,   m_ctlFollowJunctionPoints );
		DDX_Check   ( pDX, IDC_SHOWGRID,          m_listGrid                );
		DDX_Check   ( pDX, IDC_SHOWSTRIPES,       m_listStripes             );
		DDX_Check   ( pDX, IDC_FULLROWSELECTION,  m_listFullRowSelection    );
		DDX_Check   ( pDX, IDC_SHOWTIMESPENT,     m_showTimeSpent           );
		}

	virtual BOOL OnInitDialog   (                    ) override final;
	virtual void OnOK           (                    ) override final;

	BOOL      m_followMountPoints;
	BOOL      m_followJunctionPoints;
	BOOL      m_humanFormat;
	BOOL      m_listGrid;
	BOOL      m_listStripes;
	BOOL      m_listFullRowSelection;
	//C4820: 'CPageGeneral' : '4' bytes padding added after data member 'CPageGeneral::m_showTimeSpent'
	BOOL      m_showTimeSpent;

	CButton   m_ctlFollowMountPoints;
	CButton   m_ctlFollowJunctionPoints;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAnyOption( ) {
		SetModified( );
		}

	_Must_inspect_result_ COptionsPropertySheet* GetSheet( );

	};

// $Log$
// Revision 1.9  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.8  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.7  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.6  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
#else

#endif
