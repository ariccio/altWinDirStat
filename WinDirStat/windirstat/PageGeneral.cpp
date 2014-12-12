// PageGeneral.cpp	- Implementation of CPageGeneral
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

#include "stdafx.h"
#include "pagegeneral.h"
#include "options.h"

//#include "windirstat.h"
#include "mainframe.h"		// COptionsPropertySheet

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC( CPageGeneral, CPropertyPage )

_Must_inspect_result_ COptionsPropertySheet* CPageGeneral::GetSheet( ) {
	COptionsPropertySheet* sheet = DYNAMIC_DOWNCAST( COptionsPropertySheet, GetParent( ) );
	ASSERT( sheet != NULL );
	return sheet;
	}

BEGIN_MESSAGE_MAP(CPageGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_HUMANFORMAT, OnBnClickedAnyOption)
	ON_BN_CLICKED(IDC_FOLLOWMOUNTPOINTS, OnBnClickedAnyOption)
	ON_BN_CLICKED(IDC_FOLLOWJUNCTIONS, OnBnClickedAnyOption)
	ON_BN_CLICKED(IDC_SHOWGRID, OnBnClickedAnyOption)
	ON_BN_CLICKED(IDC_SHOWSTRIPES, OnBnClickedAnyOption)
	ON_BN_CLICKED(IDC_FULLROWSELECTION, OnBnClickedAnyOption)
	ON_BN_CLICKED(IDC_SHOWTIMESPENT, OnBnClickedAnyOption)
END_MESSAGE_MAP()


BOOL CPageGeneral::OnInitDialog( ) {
	CPropertyPage::OnInitDialog( );
	auto Options = GetOptions( );
	m_humanFormat          = Options->m_humanFormat;
	m_listGrid             = Options->m_listGrid;
	m_listStripes          = Options->m_listGrid;
	m_listFullRowSelection = Options->m_listFullRowSelection;
	m_followMountPoints    = Options->m_followMountPoints;
	m_followJunctionPoints = Options->m_followJunctionPoints;
	m_showTimeSpent        = Options->m_showTimeSpent;
	
	m_followMountPoints = false;	                 // Otherwise we would see pacman only.
	m_ctlFollowMountPoints.ShowWindow( SW_HIDE );    // Ignorance is bliss.
	                                                 // The same for junction points
	m_followJunctionPoints = false;	                 // Otherwise we would see pacman only.
	m_ctlFollowJunctionPoints.ShowWindow( SW_HIDE ); // Ignorance is bliss.


	UpdateData( false );
	return TRUE;
	}

void CPageGeneral::OnOK( ) {
	UpdateData( );
	auto Options = GetOptions( );
	//Compare with TRUE to prevent int->bool coercion
	Options->SetHumanFormat          ( ( ( m_humanFormat          == TRUE ) ? true : false ) );
	Options->m_followMountPoints     = ( ( m_followMountPoints    == TRUE ) ? true : false );
	Options->m_followJunctionPoints  = ( ( m_followJunctionPoints == TRUE ) ? true : false );
	Options->SetListGrid             ( ( ( m_listGrid             == TRUE ) ? true : false ) );
	Options->SetListStripes          ( ( ( m_listStripes          == TRUE ) ? true : false ) );
	Options->SetListFullRowSelection ( ( ( m_listFullRowSelection == TRUE ) ? true : false ) );
	Options->m_showTimeSpent         = ( ( m_showTimeSpent        == TRUE ) ? true : false );

	CPropertyPage::OnOK( );
	}

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
