// osspecific.h		- Declaration of CVolumeApi, CRecycleBinApi, CPsapi, CMapi32Api
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

// I try not to blow up such classes; they shall _only_ hide away the dynamic linking.

#pragma once



//
// CRecycleBinApi. Not always supported on NT and W95/98.
//
//class CRecycleBinApi
//{
//public:
//	CRecycleBinApi();
//	~CRecycleBinApi();
//
//	bool IsSupported();
//
//	HRESULT SHEmptyRecycleBin ( HWND hwnd,           LPCTSTR pszRootPath,           DWORD dwFlags );
//	HRESULT SHQueryRecycleBin ( LPCTSTR pszRootPath, LPSHQUERYRBINFO pSHQueryRBInfo               );
//
//private:
//	typedef HRESULT ( STDAPICALLTYPE *TypeSHEmptyRecycleBin ) ( HWND hwnd,           LPCTSTR pszRootPath,           DWORD dwFlags );
//	typedef HRESULT ( STDAPICALLTYPE *TypeSHQueryRecycleBin ) ( LPCTSTR pszRootPath, LPSHQUERYRBINFO pSHQueryRBInfo               );
//
//	HMODULE               m_dll;
//	bool                  m_UnloadDll;
//	TypeSHEmptyRecycleBin m_SHEmptyRecycleBin;
//	TypeSHQueryRecycleBin m_SHQueryRecycleBin;
//};

//
// GetCompressedFileSize. Only supported on the NT platform
//
//class CGetCompressedFileSizeApi
//{
//public:
//	CGetCompressedFileSizeApi();
//	~CGetCompressedFileSizeApi();
//
//	bool IsSupported() const;
//
//	DWORD     GetCompressedFileSize( _In_ LPCTSTR lpFileName, _Inout_ LPDWORD lpFileSizeHigh );
//	ULONGLONG GetCompressedFileSize( _In_ LPCTSTR lpFileName                         );
//
//private:
//	typedef DWORD( WINAPI *TypeGetCompressedFileSize )( LPCTSTR lpFileName, LPDWORD lpFileSizeHigh );
//
//	HMODULE                   m_dll;
//	bool                      m_UnloadDll;
//	TypeGetCompressedFileSize m_GetCompressedFileSize;
//};

// $Log$
// Revision 1.7  2005/04/17 12:27:21  assarbad
// - For details see changelog of 2005-04-17
//
// Revision 1.6  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.5  2004/11/07 20:14:31  assarbad
// - Added wrapper for GetCompressedFileSize() so that by default the compressed file size will be shown.
//
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
