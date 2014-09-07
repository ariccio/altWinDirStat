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

#pragma once

#include "stdafx.h"


//
// CModalShellApi. Modal version of the shell functions
// EmptyRecycleBin and DeleteFile.
// 
// See comment on CModalApiShuttle.
//

namespace {
	enum {
		EMPTY_RECYCLE_BIN,
		DELETE_FILE
		};
	}


class CModalShellApi : public CModalApiShuttle {
public:
	CModalShellApi( ) { }

	//bool IsRecycleBinApiSupported();
	//void EmptyRecycleBin();
	void DeleteFile( _In_z_ LPCTSTR fileName, _In_ bool toRecycleBin ) {
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

	//void DoEmptyRecycleBin();
	void DoDeleteFile( ) {
		auto len = m_fileName.GetLength( );
		LPTSTR psz = m_fileName.GetBuffer( len + 2 );
		psz[ len + 1 ] = 0;

		auto sfos   = zeroInitSHFILEOPSTRUCT( );
		sfos.wFunc  = FO_DELETE;
		sfos.pFrom  = psz;
		sfos.fFlags = m_toRecycleBin ? FOF_ALLOWUNDO : 0;
		sfos.hwnd   = *AfxGetMainWnd( );

		( void ) SHFileOperation( &sfos );

		m_fileName.ReleaseBuffer( );
		}

	//CRecycleBinApi m_rbapi;	// Dynamically linked shell32.dll functions
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
