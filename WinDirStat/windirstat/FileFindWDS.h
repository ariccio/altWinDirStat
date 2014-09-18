// FileFindWDS.h	- Declaration of CFileFindWDS
//
// WinDirStat - Directory Statistics
// Copyright (C) 2004 Assarbad
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
// Author: assarbad@users.sourceforge.net
//
// Last modified: $Date$

#pragma once
#include "stdafx.h"
//#include <afx.h> // Declaration of prototype for CFileFind

class CFileFindWDS : public CFileFind {
public:
	 CFileFindWDS ( void );
	~CFileFindWDS ( void );
	                           DWORD     GetAttributes       ( ) const;
	_Success_(return != NULL ) ULONGLONG GetCompressedLength ( ) const;
	_Success_(return != NULL ) ULONGLONG GetCompressedLength ( PCWSTR name ) const;
	_Success_(return != NULL ) PWSTR altGetFileName( ) const;
	CString altGetFilePath( ) const;
	
	};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//THIS IS NOT A WORKING IMPLEMENTATION!
//namespace AsyncWalk {
//	std::future<std::unique_ptr<std::vector<FILEINFO>>> descendDirectory( _Inout_ WIN32_FIND_DATA& fData, _In_ const std::wstring& normSzDir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo );
//	std::unique_ptr<std::vector<FILEINFO>> stdRecurseFindFutures( _In_ std::wstring dir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo );
//	}


// $Log$
// Revision 1.3  2004/11/29 09:07:53  assarbad
// - Changed scope for including afx.h
//
// Revision 1.2  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.1  2004/11/25 23:07:24  assarbad
// - Derived CFileFindWDS from CFileFind to correct a problem of the ANSI version
//
