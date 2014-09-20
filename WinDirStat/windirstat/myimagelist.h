// myimagelist.h	- Declaration of CMyImageList
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


#ifndef MYIMAGELIST_H
#define MYIMAGELIST_H
#else
#error ass
#endif

#pragma once
#include "stdafx.h"

//
// CMyImageList. Both CDirstatView and CTypeView use this central
// image list. It caches the system image list images as needed,
// and adds 4 special images at initialization.
// This is because I don't want to deal with two images lists.
//
class CMyImageList: public CImageList
{
public:
	CMyImageList          ( );
	virtual ~CMyImageList ( );

	void Initialize                (                                   );
	INT  GetEmptyImage             (                                   );
	INT  GetExtImageAndDescription ( _In_z_ LPCTSTR ext, _Inout_ CString& description );
	INT  GetFileImage              ( _In_z_  LPCTSTR path                      );
	INT  GetFilesFolderImage       (                                   );
	INT  GetFolderImage            (                                   );
	INT  GetFreeSpaceImage         (                                   );
	INT  GetJunctionImage          (                                   );
	INT  GetMountPointImage        (                                   );
	INT  GetMyComputerImage        (                                   );
	INT  GetUnknownImage           (                                   );

protected:
	INT     CacheIcon       ( _In_z_ LPCTSTR path, _In_ UINT flags, _Inout_opt_ CString *psTypeName = NULL );
	CString GetADriveSpec   ( );
	void    AddCustomImages ( );
	
	CMap<INT, INT, INT, INT> m_indexMap;	// system image list index -> our index

	INT m_filesFolderImage;	// <Files>
	INT m_freeSpaceImage;	// <Free Space>
	INT m_unknownImage;		// <Unknown>
	INT m_emptyImage;		// For items whose image cannot be found

	// Junction point
	INT m_junctionImage;
};


// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
