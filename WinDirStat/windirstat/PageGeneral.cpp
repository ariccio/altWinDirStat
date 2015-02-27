// PageGeneral.cpp	- Implementation of CPageGeneral
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_PAGEGENERAL_CPP
#define WDS_PAGEGENERAL_CPP

#include "pagegeneral.h"
#include "options.h"

//#include "windirstat.h"
#include "mainframe.h"		// COptionsPropertySheet



IMPLEMENT_DYNAMIC( CPageGeneral, CPropertyPage )

_Must_inspect_result_ COptionsPropertySheet* CPageGeneral::GetSheet( ) {
	COptionsPropertySheet* sheet = STATIC_DOWNCAST( COptionsPropertySheet, GetParent( ) );
	ASSERT( sheet != NULL );
	return sheet;
	}

BEGIN_MESSAGE_MAP(CPageGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_HUMANFORMAT,       &( CPageGeneral::OnBnClickedAnyOption ) )
	ON_BN_CLICKED(IDC_FOLLOWMOUNTPOINTS, &( CPageGeneral::OnBnClickedAnyOption ) )
	ON_BN_CLICKED(IDC_FOLLOWJUNCTIONS,   &( CPageGeneral::OnBnClickedAnyOption ) )
	ON_BN_CLICKED(IDC_SHOWGRID,          &( CPageGeneral::OnBnClickedAnyOption ) )
	ON_BN_CLICKED(IDC_SHOWSTRIPES,       &( CPageGeneral::OnBnClickedAnyOption ) )
	ON_BN_CLICKED(IDC_FULLROWSELECTION,  &( CPageGeneral::OnBnClickedAnyOption ) )
	ON_BN_CLICKED(IDC_SHOWTIMESPENT,     &( CPageGeneral::OnBnClickedAnyOption ) )
END_MESSAGE_MAP()


BOOL CPageGeneral::OnInitDialog( ) {
	CPropertyPage::OnInitDialog( );
	const auto Options = GetOptions( );
	m_humanFormat          = Options->m_humanFormat;
	m_listGrid             = Options->m_listGrid;
	m_listStripes          = Options->m_listStripes;
	m_listFullRowSelection = Options->m_listFullRowSelection;
	m_followMountPoints    = Options->m_followMountPoints;
	m_followJunctionPoints = Options->m_followJunctionPoints;
	//m_showTimeSpent        = Options->m_showTimeSpent;
	
	m_followMountPoints = false;	                 // Otherwise we would see pacman only.
	m_ctlFollowMountPoints.ShowWindow( SW_HIDE );    // Ignorance is bliss.
	                                                 // The same for junction points
	m_followJunctionPoints = false;	                 // Otherwise we would see pacman only.
	m_ctlFollowJunctionPoints.ShowWindow( SW_HIDE ); // Ignorance is bliss.


	VERIFY( UpdateData( false ) );
	return TRUE;
	}

void CPageGeneral::OnOK( ) {
	VERIFY( UpdateData( ) );
	ASSERT( m_appptr != NULL );
	const auto Options = GetOptions( );
	//Compare with TRUE to prevent int->bool coercion
	Options->m_followMountPoints    = ( ( m_followMountPoints    == TRUE ) ? true : false );
	Options->m_followJunctionPoints = ( ( m_followJunctionPoints == TRUE ) ? true : false );
	//Options->m_showTimeSpent        = ( ( m_showTimeSpent        == TRUE ) ? true : false );
	Options->SetHumanFormat         ( ( ( m_humanFormat          == TRUE ) ? true : false ), m_appptr );
	Options->SetListGrid            ( ( ( m_listGrid             == TRUE ) ? true : false ) );
	Options->SetListStripes         ( ( ( m_listStripes          == TRUE ) ? true : false ) );
	Options->SetListFullRowSelection( ( ( m_listFullRowSelection == TRUE ) ? true : false ) );
	CPropertyPage::OnOK( );
	}


#else

#endif