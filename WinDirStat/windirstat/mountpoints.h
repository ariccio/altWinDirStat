// mountpoints.h	- Declaratio of CMountPoins
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


#ifndef MOUNTPOINTS_H
#define MOUNTPOINTS_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"

class CMountPoints {
	struct SPointVolume {
		CString point;	// Path like "mount\backup\"
		CString volume;	// Volume identifier
		};

public:
	~CMountPoints( ) {
		Clear( );
		}
	void Initialize( ) {
		Clear( );
		GetDriveVolumes( );
		GetAllMountPoints( );
		}

	bool IsMountPoint       ( _In_ CString path                          ) const;
	bool IsJunctionPoint    ( _In_ CString path,  _In_ DWORD fAttributes ) const;
	bool IsJunctionPoint    ( _In_ CString path,  _In_ attribs& attr ) const;
private:
	void Clear              ( );
	void GetDriveVolumes    ( );
	void GetAllMountPoints  ( );

	bool IsVolumeMountPoint (_In_ const CString& volume, _In_ const CString& path      ) const;

	// m_drive contains the volume identifiers of the Drives A:, B: etc.
	// mdrive[0] = Volume identifier of A:\.
	CArray<CString, PCWSTR> m_drive;
	std::map<CString, std::unique_ptr<std::vector<SPointVolume>>> m_volume;
	};

// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
