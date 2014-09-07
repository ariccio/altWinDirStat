// commonhelpers.h	- Functions used by windirstat.exe and setup.exe
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

#include "../windirstat/stdafx.h"

CString GetShellExecuteError( _In_       const UINT u                                           );

CString MyStrRetToString    ( _In_       const LPITEMIDLIST pidl, _In_     const STRRET* strret );
CString GetBaseNameFromPath ( _In_z_     const LPCTSTR path                                     );
bool FileExists             ( _In_z_     const LPCTSTR path                                     );
CString LoadString          ( _In_       const UINT resId                                       );
CString MyGetFullPathName   ( _In_z_     const LPCTSTR relativePath                             );
CString GetAppFileName      ( );
CString GetAppFolder        ( );


void MyShellExecute         ( _In_opt_       HWND hwnd,         _In_opt_z_       LPCTSTR lpOperation, _In_z_ LPCTSTR lpFile, _In_opt_z_ LPCTSTR lpParameters, _In_opt_z_ LPCTSTR lpDirectory, _In_ const INT nShowCmd ) /*throw ( CException * )*/;

// $Log$
// Revision 1.4  2004/11/05 16:53:05  assarbad
// Added Date and History tag where appropriate.
//
