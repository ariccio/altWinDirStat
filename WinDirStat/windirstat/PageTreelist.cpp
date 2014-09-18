// PageTreelist.cpp		- Implementation of CPageTreelist
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
//#include ".\pagetreelist.h"

//#include "windirstat.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC( CPageTreelist, CPropertyPage )

CPageTreelist::CPageTreelist( ) : CPropertyPage( IDD_PAGE_TREELIST ), m_showTimeSpent( FALSE )/*, m_treelistColorCount( NULL )*/ { }

void CPageTreelist::DoDataExchange( CDataExchange* pDX ) {
	CPropertyPage::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_SHOWTIMESPENT, m_showTimeSpent );
	}


BEGIN_MESSAGE_MAP(CPageTreelist, CPropertyPage)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_SHOWTIMESPENT, OnBnClickedShowTimeSpent)
END_MESSAGE_MAP()


BOOL CPageTreelist::OnInitDialog( ) {
	CPropertyPage::OnInitDialog( );
	auto Options = GetOptions( );
	if ( Options != NULL ) {

		m_showTimeSpent = Options->IsShowTimeSpent( );
		}
	ASSERT( Options != NULL );
	UpdateData( false );
	return TRUE;
	}

void CPageTreelist::OnOK( ) {
	UpdateData( );
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		//m_showTimeSpent is BOOL here because of DDX_CHECK in DoDataExchange
		Options->SetShowTimeSpent( ( ( m_showTimeSpent == TRUE ) ? true : false ) );
		}
	ASSERT( Options != NULL );
	CPropertyPage::OnOK( );
	}

void CPageTreelist::OnBnClickedShowTimeSpent( ) {
	SetModified( );
	}

void CPageTreelist::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
	SetModified();
	CPropertyPage::OnVScroll(nSBCode, nPos, pScrollBar);
	}

// $Log$
// Revision 1.5  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.4  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.3  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
