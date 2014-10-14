// modalshellapi.h	- Declaration of CModalShellApi
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

#ifndef MODALSHELLAPI_H
#define MODALSHELLAPI_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"

// CModalShellApi. Modal version of the shell functions
// EmptyRecycleBin and DeleteFile.
// 
// See comment on CModalApiShuttle.

namespace {
	enum {
		EMPTY_RECYCLE_BIN,
		DELETE_FILE
		};
	}


// CModalApiShuttle. (Base class for CModalShellApi and CModalSendMail.)
//
// The SHFileOperation() function shows modeless dialogs, but we want them to be modal.
//
// When the operation window is destroyed, the system brings some other window to the foreground and WinDirStat ends up in the background. That's because it is still disabled at that moment.
//
// So my solution is this:
// First create an invisible (zero size) (but enabled) modal dialog, then do the operation in its OnInitDialog function and end the dialog.
class CModalApiShuttle : public CDialog {
	//DECLARE_DYNAMIC(CModalApiShuttle)

public:
	CModalApiShuttle( CWnd* pParent = NULL ) : CDialog( CModalApiShuttle::IDD, pParent ) { }

protected:
	enum { IDD = IDD_MODALAPISHUTTLE };
	virtual BOOL OnInitDialog( ) override {
		CDialog::OnInitDialog( );

		CRect rc;
		AfxGetMainWnd( )->GetWindowRect( rc );
		rc.right  = rc.left;
		rc.bottom = rc.top;

		MoveWindow( rc, false );

		EnableWindow( true );
		ShowWindow( SW_SHOW );

		DoOperation( );

		EndDialog( IDOK );
		return TRUE;
		}
	
	
	//DECLARE_MESSAGE_MAP()

	virtual void DoOperation( ) = 0;
	};



class CModalShellApi : public CModalApiShuttle {
public:

	void DeleteFileW( _In_z_ PCWSTR fileName, _In_ bool toRecycleBin ) {
		m_operation    = DELETE_FILE;
		m_fileName     = fileName;
		m_toRecycleBin = toRecycleBin;
		DoModal( );
		}

protected:
	virtual void DoOperation( ) {
		if ( m_operation == DELETE_FILE ) {
			return DoDeleteFile( );
			}
		}

	void DoDeleteFile( ) {
		const auto len = m_fileName.GetLength( );
		PTSTR psz = m_fileName.GetBuffer( len + 2 );
		psz[ len + 1 ] = 0;

		auto sfos   = zeroInitSHFILEOPSTRUCT( );
		sfos.wFunc  = FO_DELETE;
		sfos.pFrom  = psz;
		sfos.fFlags = m_toRecycleBin ? FOF_ALLOWUNDO : 0;
		sfos.hwnd   = *AfxGetMainWnd( );

		( void ) SHFileOperation( &sfos );

		m_fileName.ReleaseBuffer( );
		}

	INT     m_operation;		// Enum specifying the desired operation
	CString m_fileName;		// File name to be deleted
	bool    m_toRecycleBin;	// True if file shall only be move to the recycle bin
	};

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
