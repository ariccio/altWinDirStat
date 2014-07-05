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
#include "windirstat.h"
#include "mainframe.h"		// COptionsPropertySheet
#include ".\pagegeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC( CPageGeneral, CPropertyPage )

CPageGeneral::CPageGeneral( ) : CPropertyPage( CPageGeneral::IDD ), m_followMountPoints( FALSE ), m_followJunctionPoints( FALSE ), m_humanFormat( FALSE ), m_listGrid( FALSE ), m_listStripes( FALSE ), m_listFullRowSelection( FALSE )
#ifdef PAGEREPORT
 ,m_useWdsLocale( FALSE )
#endif
	{//TODO: check defaults!
	AfxCheckMemory( );
	}

CPageGeneral::~CPageGeneral( ) {
	AfxCheckMemory( );
	}

_Must_inspect_result_ COptionsPropertySheet *CPageGeneral::GetSheet( ) {
	AfxCheckMemory( );
	COptionsPropertySheet *sheet = DYNAMIC_DOWNCAST(COptionsPropertySheet, GetParent());
	ASSERT(sheet != NULL);
	return sheet;
	}

void CPageGeneral::DoDataExchange( CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_HUMANFORMAT, m_humanFormat);
	DDX_Check(pDX, IDC_FOLLOWMOUNTPOINTS, m_followMountPoints);
	DDX_Check(pDX, IDC_FOLLOWJUNCTIONS, m_followJunctionPoints);
	//DDX_Check(pDX, IDC_USEWDSLOCALE, m_useWdsLocale);
	//DDX_Control(pDX, IDC_COMBO, m_combo);
	DDX_Control(pDX, IDC_FOLLOWMOUNTPOINTS, m_ctlFollowMountPoints);
	DDX_Control(pDX, IDC_FOLLOWJUNCTIONS, m_ctlFollowJunctionPoints);
	DDX_Check(pDX, IDC_SHOWGRID, m_listGrid);
	DDX_Check(pDX, IDC_SHOWSTRIPES, m_listStripes);
	DDX_Check(pDX, IDC_FULLROWSELECTION, m_listFullRowSelection);
}


BEGIN_MESSAGE_MAP(CPageGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_HUMANFORMAT, OnBnClickedHumanformat)
	ON_BN_CLICKED(IDC_FOLLOWMOUNTPOINTS, OnBnClickedFollowmountpoints)
	ON_BN_CLICKED(IDC_FOLLOWJUNCTIONS, OnBnClickedFollowjunctionpoints)
	//ON_BN_CLICKED(IDC_USEWDSLOCALE, OnBnClickedUseWdsLocale)
	//ON_CBN_SELENDOK(IDC_COMBO, OnCbnSelendokCombo)
	ON_BN_CLICKED(IDC_SHOWGRID, OnBnClickedListGrid)
	ON_BN_CLICKED(IDC_SHOWSTRIPES, OnBnClickedListStripes)
	ON_BN_CLICKED(IDC_FULLROWSELECTION, OnBnClickedListFullRowSelection)
END_MESSAGE_MAP()


BOOL CPageGeneral::OnInitDialog( ) {
	CPropertyPage::OnInitDialog( );
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		m_humanFormat = Options->IsHumanFormat( );
		m_listGrid = Options->IsListGrid( );
		m_listStripes = Options->IsListStripes( );
		m_listFullRowSelection = Options->IsListFullRowSelection( );

		m_followMountPoints = Options->IsFollowMountPoints( );
		m_followJunctionPoints = Options->IsFollowJunctionPoints( );

#ifdef USELOCALE
		m_useWdsLocale = Options->IsUseWdsLocale( );
#endif

		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	CVolumeApi va;
	if ( !va.IsSupported( ) ) {
		m_followMountPoints = false;	// Otherwise we would see pacman only.
		m_ctlFollowMountPoints.ShowWindow( SW_HIDE ); // Ignorance is bliss.
		// The same for junction points
		m_followJunctionPoints = false;	// Otherwise we would see pacman only.
		m_ctlFollowJunctionPoints.ShowWindow( SW_HIDE ); // Ignorance is bliss.
		}
	else {
		//Valid condition
		}

	UpdateData( false );
	AfxCheckMemory( );
	return TRUE;
	}

void CPageGeneral::OnOK( ) {
	UpdateData();
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		
		Options->SetHumanFormat( ( ( m_humanFormat == TRUE ) ? true : false ) );
				
		Options->SetFollowMountPoints( ( ( m_followMountPoints == TRUE ) ? true : false ) );

		Options->SetFollowJunctionPoints( ( ( m_followJunctionPoints == TRUE ) ? true : false ) );

#ifdef USELOCALE
		Options->SetUseWdsLocale( ( ( m_useWdsLocale == TRUE ) ? true : false ) );
#endif

		Options->SetListGrid( ( ( m_listGrid == TRUE ) ? true : false ) );

		Options->SetListStripes( ( ( m_listStripes == TRUE ) ? true : false ) );

		Options->SetListFullRowSelection( ( ( m_listFullRowSelection == TRUE ) ? true : false ) );

		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	//LANGID id = ( LANGID ) m_combo.GetItemData( m_combo.GetCurSel( ) );

	CPropertyPage::OnOK();
	}

void CPageGeneral::OnBnClickedHumanformat()
{
	SetModified();
}

void CPageGeneral::OnBnClickedFollowmountpoints()
{
	SetModified();
}

void CPageGeneral::OnBnClickedFollowjunctionpoints()
{
	SetModified();
}

void CPageGeneral::OnBnClickedUseWdsLocale()
{
	SetModified();
}

void CPageGeneral::OnBnClickedListGrid()
{
	SetModified();
}

void CPageGeneral::OnBnClickedListStripes()
{
	SetModified();
}

void CPageGeneral::OnBnClickedListFullRowSelection()
{
	SetModified();
}

void CPageGeneral::OnCbnSelendokCombo()
{
	INT i = m_combo.GetCurSel( );
	auto Sheet = GetSheet( );
	if ( Sheet != NULL ) {
		Sheet->SetLanguageChanged( i != m_originalLanguage );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	SetModified( );
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
