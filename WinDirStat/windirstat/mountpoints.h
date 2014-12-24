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

#pragma once

#ifndef MOUNTPOINTS_H
#define MOUNTPOINTS_H
#include "stdafx.h"

class CMountPoints {
	struct SPointVolume {
		std::wstring point;	// Path like "mount\backup\"
		std::wstring volume;	// Volume identifier
		};

public:
	~CMountPoints( );

	void Initialize( ) {
		Clear( );
		GetDriveVolumes( );
		GetAllMountPoints( );
		}

	bool IsMountPoint       ( _In_ const std::wstring& path                          ) const;
	bool IsJunctionPoint    ( _In_ const std::wstring& path,  _In_ const DWORD fAttributes ) const;
	bool IsJunctionPoint    ( _In_ const std::wstring& path,  _In_ const attribs& attr     ) const;
private:
	void Clear              ( );
	void GetDriveVolumes    ( );
	void GetAllMountPoints  ( );
	bool IsVolumeMountPoint ( _In_ _In_range_( 0, SIZE_T_MAX ) const int index_in_m_drive, _In_ const std::wstring& path      ) const;

	// m_drive contains the volume identifiers of the Drives A:, B: etc.
	// mdrive[0] = Volume identifier of A:\.
	std::wstring m_drive[ 32 ];
	std::map<std::wstring, std::unique_ptr<std::vector<SPointVolume>>> m_volume;
	};
#else
#error ass
#endif
