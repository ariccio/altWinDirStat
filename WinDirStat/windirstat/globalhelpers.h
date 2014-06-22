// globalhelpers.h	- Declaration of global helper functions
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


CString GetCOMSPEC                 (                                                    );
CString GetFolderNameFromPath      ( _In_ const LPCTSTR path                            );
CString GetLocaleString            ( _In_ const LCTYPE lctype, _In_ const LANGID langid );
CString GetLocaleLanguage          ( _In_ const LANGID langid                           );
CString GetLocaleThousandSeparator (                                                    );
CString GetLocaleDecimalSeparator  (                                                    );
CString GetSpec_Bytes              (                                                    );
CString GetSpec_KB                 (                                                    );
CString GetSpec_MB                 (                                                    );
CString GetSpec_GB                 (                                                    );
CString GetSpec_TB                 (                                                    );
CString lGetUserName               (                                                    );

CString FormatAttributes           ( _In_ const DWORD              attr                                                                );
CString FormatBytes                ( _In_ const LONGLONG           n                                                                   );
CString FormatCount                ( _In_       LONGLONG           n                                                                   );
CString FormatDouble               ( _In_       DOUBLE             d                                                                   );
CString FormatFileTime             ( _In_ const FILETIME&          t                                                                   );
CString FormatLongLongHuman        ( _In_ const LONGLONG           n                                                                   );
CString FormatMilliseconds         ( _In_ const unsigned long long ms                                                                  );
CString FormatVolumeNameOfRootPath ( _In_ const CString            rootPath                                                            );
CString FormatVolumeName           ( _In_ const CString            rootPath,    _In_ const CString   volumeName                        );

CString MyQueryDosDevice           ( _In_ const LPCTSTR            drive                                                               );
CString PadWidthBlanks             ( _In_       CString            n,           _In_ const INT       width                             );
CString PathFromVolumeName         ( _In_ const CString            name                                                                );

bool DriveExists                   ( _In_ const CString&           path                                                                );
bool FolderExists                  ( _In_ const LPCTSTR            path                                                                );
bool GetVolumeName                 ( _In_ const LPCTSTR            rootPath,    _Inout_    CString&  volumeName                        );
bool IsHexDigit                    ( _In_ const INT                c                                                                   );
bool IsSUBSTedDrive                ( _In_ const LPCTSTR            drive                                                               );
void MyGetDiskFreeSpace            ( _In_ const LPCTSTR            pszRootPath, _Inout_    LONGLONG& total, _Inout_ LONGLONG& unused   );
void WaitForHandleWithRepainting   ( _In_ const HANDLE h                                                                               );

CString GetParseNameOfMyComputer   (                                                             ) throw ( CException * );
void GetPidlOfMyComputer           ( _Inout_    LPITEMIDLIST *ppidl                              ) throw ( CException * );
void ShellExecuteWithAssocDialog   ( _In_ const HWND hwnd,           _In_ const LPCTSTR filename ) throw ( CException * );


void check8Dot3NameCreationAndNotifyUser( );
void displayWindowsMsgBoxWithError( );

LONGLONG GetTotalDiskSpace( _In_ const CString );

STORAGE_DEVICE_NUMBER   zeroInitSTORAGE_DEVICE_NUMBER   ( );
SHELLEXECUTEINFO        zeroInitSEI                     ( );
WINDOWPLACEMENT         zeroInitWINDOWPLACEMENT         ( );
USN_JOURNAL_DATA        zeroInitUSN_JOURNAL_DATA        ( );
LVHITTESTINFO           zeroInitLVHITTESTINFO           ( );
HDITEM                  zeroInitHDITEM                  ( );
LVFINDINFO              zeroInitLVFINDINFO              ( );
LVITEM                  zeroInitLVITEM                  ( );
MFT_ENUM_DATA_V0        zeroInitMFT_ENUM_DATA_V0        ( );
MFT_ENUM_DATA_V1        zeroInitMFT_ENUM_DATA_V1        ( );
PROCESS_MEMORY_COUNTERS zeroInitPROCESS_MEMORY_COUNTERS ( );
STARTUPINFO             zeroInitSTARTUPINFO             ( );
PROCESS_INFORMATION     zeroInitPROCESS_INFORMATION     ( );
NMLISTVIEW              zeroInitNMLISTVIEW              ( );
BROWSEINFO              zeroInitBROWSEINFO              ( );

// $Log$
// Revision 1.15  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.14  2004/11/25 21:13:38  assarbad
// - Implemented "attributes" column in the treelist
// - Adopted width in German dialog
// - Provided German, Russian and English version of IDS_TREECOL_ATTRIBUTES
//
// Revision 1.13  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.12  2004/11/12 13:19:44  assarbad
// - Minor changes and additions (in preparation for the solution of the "Browse for Folder" problem)
//
// Revision 1.11  2004/11/07 20:14:30  assarbad
// - Added wrapper for GetCompressedFileSize() so that by default the compressed file size will be shown.
//
// Revision 1.10  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
