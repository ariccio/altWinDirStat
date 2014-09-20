// DeleteWarningDlg.cpp - Skeletal implementation of CDeleteWarningDlg

#include "stdafx.h"

IMPLEMENT_DYNAMIC( CDeleteWarningDlg, CDialog )

BEGIN_MESSAGE_MAP(CDeleteWarningDlg, CDialog)
	ON_BN_CLICKED(IDNO, OnBnClickedNo)
	ON_BN_CLICKED(IDYES, OnBnClickedYes)
END_MESSAGE_MAP()

// $Log$
// Revision 1.5  2004/11/13 08:17:06  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.4  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.3  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
