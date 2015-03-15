// mountpoints.h	- Declaration of CMountPoins
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_MOUNTPOINTS_H
#define WDS_MOUNTPOINTS_H



WDS_FILE_INCLUDE_MESSAGE

class CMountPoints final {

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

	const bool IsJunctionPoint( _In_ const std::wstring& path, _In_ const DWORD fAttributes ) const {
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

	const bool IsMountPoint( _In_ const std::wstring& path ) const;

	
private:
	void Clear( );

	void GetDriveVolumes( );

	void GetAllMountPoints( );

	const bool IsVolumeMountPoint( _In_ _In_range_( 0, ( M_DRIVE_ARRAY_SIZE - 1 ) ) const int index_in_m_drive, _In_ const std::wstring& path ) const;


	// m_drive contains the volume identifiers of the Drives A:, B: etc.
	// mdrive[0] = Volume identifier of A:\.
	_Field_size_( M_DRIVE_ARRAY_SIZE ) std::wstring m_drive[ M_DRIVE_ARRAY_SIZE ];

	std::unordered_map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> m_volume;
	};
#else

#endif
