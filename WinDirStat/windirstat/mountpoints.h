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
	//struct SPointVolume {
	//	std::wstring point;	// Path like "mount\backup\"
	//	std::wstring volume;	// Volume identifier
	//	};

	enum : rsize_t {
		M_DRIVE_ARRAY_SIZE = 32u
		};

public:
	~CMountPoints( ) = default;

	void Initialize( ) {
		Clear( );
		GetDriveVolumes( );
		GetAllMountPoints( );
		}

	bool IsMountPoint       ( _In_ const std::wstring& path                          ) const;
	bool IsJunctionPoint( _In_ const std::wstring& path, _In_ const DWORD fAttributes ) const {
		/*
		  Check whether the current item is a junction point but no volume mount point as the latter ones are treated differently (see above). CAN ALSO BE A REPARSE POINT!
		*/
		if ( fAttributes == INVALID_FILE_ATTRIBUTES ) {
			return false;
			}
		if ( IsMountPoint( path ) ) {
			return false;
			}
		return ( ( fAttributes bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
		}
	
	//bool IsJunctionPoint    ( _In_ const std::wstring& path,  _In_ const attribs& attr     ) const;
private:
	void Clear( ) {
		for ( size_t i = 0; i < M_DRIVE_ARRAY_SIZE; ++i ) {
			m_drive[ i ].clear( );
			}
		m_volume.clear( );
		}




	void GetDriveVolumes    ( );
	void GetAllMountPoints  ( );
	bool IsVolumeMountPoint ( _In_ _In_range_( 0, ( M_DRIVE_ARRAY_SIZE - 1 ) ) const int index_in_m_drive, _In_ const std::wstring& path      ) const;

	// m_drive contains the volume identifiers of the Drives A:, B: etc.
	// mdrive[0] = Volume identifier of A:\.
	_Field_size_( M_DRIVE_ARRAY_SIZE ) std::wstring m_drive[ M_DRIVE_ARRAY_SIZE ];

	std::unordered_map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> m_volume;
	};
#else
#error ass
#endif
