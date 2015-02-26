// mountpoints.h	- Declaration of CMountPoins
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_MOUNTPOINTS_H
#define WDS_MOUNTPOINTS_H

#include "globalhelpers.h"


namespace {
	void FindVolumeMountPointCloseHandle( _In_ _Post_invalid_ HANDLE hFindVolumeMountPoint ) {
		VERIFY( FindVolumeMountPointClose( hFindVolumeMountPoint ) );
		}
	BOOL FindVolumeCloseHandle( _In_ _Post_invalid_ HANDLE hFindVolume ) {
		return FindVolumeClose( hFindVolume );
		}
	}


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



	//marking `path` as const, OR a reference trashes codegen!
	inline bool IsMountPoint( _In_ const std::wstring& path ) const {
		if ( ( path.length( ) < 3 ) || ( path[ 1 ] != L':' ) || ( path[ 2 ] != L'\\' ) ) {
			// Don't know how to make out mount points on UNC paths ###
			return false;
			}
		if ( m_volume.empty( ) ) {
			return false;
			}
		ASSERT( ( path.length( ) >= 3 ) && ( path.at( 1 )  == L':' ) && ( path.at( 2 )  == L'\\' ) );

		const auto pathAtZero = towlower( path[ 0 ] );
		const auto weirdAss_a = _T( 'a' );
		const auto indexItem  = pathAtZero - weirdAss_a;
		return IsVolumeMountPoint( indexItem, path );
		}

	inline bool IsJunctionPoint( _In_ const std::wstring& path, _In_ const DWORD fAttributes ) const {
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




	void GetDriveVolumes( ) {
		//m_drive.resize( 32 );

		const rsize_t s_char_buffer_size = 5u;
		_Null_terminated_ wchar_t small_buffer_volume_name[ s_char_buffer_size ] = { 0 };
		const auto drives = GetLogicalDrives( );
		DWORD mask = 0x00000001;

		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( INT i = 0; i < static_cast<INT>( M_DRIVE_ARRAY_SIZE ); i++, mask <<= 1 ) {
			const rsize_t larger_buffer_size = MAX_PATH;
			_Null_terminated_ wchar_t volume_[ larger_buffer_size ] = { 0 };
			if ( ( drives bitand mask ) != 0 ) {
				const auto swps = swprintf_s( small_buffer_volume_name, L"%c:\\", ( i + _T( 'A' ) ) );
				if ( swps == -1 ) {
					displayWindowsMsgBoxWithMessage( L"unexpected error in CMountPoints::GetDriveVolumes!!(aborting)" );
					std::terminate( );
					}
				ASSERT( wcslen( small_buffer_volume_name ) < 5 );

				const BOOL b = GetVolumeNameForVolumeMountPointW( small_buffer_volume_name, volume_, larger_buffer_size );
				if ( !b ) {
#ifdef DEBUG
					trace_GetVolumeNameForVolumeMountPoint_failed( small_buffer_volume_name );
#endif
					}
				}
			m_drive[ static_cast<size_t>( i ) ] = volume_;
			}
		}
	void GetAllMountPoints( ) {
		const rsize_t volumeTCHARsize = MAX_PATH;
		_Null_terminated_ wchar_t volume[ volumeTCHARsize ] = { 0 };
		const HANDLE hvol = FindFirstVolumeW( volume, volumeTCHARsize );
		if ( hvol == INVALID_HANDLE_VALUE ) {
			TRACE( _T( "No volumes found.\r\n" ) );
			return;
			}

		for ( BOOL bContinue = true; bContinue; bContinue = FindNextVolumeW( hvol, volume, volumeTCHARsize ) ) {

			DWORD sysflags;
			_Null_terminated_ wchar_t fsname_[ volumeTCHARsize ] = { 0 };
			const BOOL b = GetVolumeInformationW( volume, NULL, 0, NULL, NULL, &sysflags, fsname_, volumeTCHARsize );
			if ( !b ) {
#ifdef DEBUG
				trace_fs_not_rea( volume );
#endif
				//m_volume[ volume ] = std::vector<SPointVolume>( );
				continue;
				}

			if ( ( sysflags bitand FILE_SUPPORTS_REPARSE_POINTS ) == 0 ) {
#ifdef DEBUG
				trace_no_reparse( volume );
#endif
				//m_volume[ volume ] = std::vector<SPointVolume>( );
				continue;
				}

			_Null_terminated_ wchar_t point[ volumeTCHARsize ] = { 0 };
			const HANDLE h = FindFirstVolumeMountPointW( volume, point, volumeTCHARsize );
			if ( h == INVALID_HANDLE_VALUE ) {
#ifdef DEBUG
				trace_no_vol_mnt( volume );
#endif
				//m_volume[ volume ] = std::vector<SPointVolume>( );
				continue;
				}

			//auto pointer_volume_array = std::vector<SPointVolume>( );
			std::vector<std::pair<std::wstring, std::wstring>> pointer_volume_array;
			for ( BOOL bCont = true; bCont; bCont = FindNextVolumeMountPointW( h, point, volumeTCHARsize ) ) {
				std::wstring uniquePath_temp( volume );
				uniquePath_temp += point;
				const std::wstring uniquePath( std::move( uniquePath_temp ) );
				_Null_terminated_ wchar_t mountedVolume_[ volumeTCHARsize ] = { 0 };
				BOOL b2 = GetVolumeNameForVolumeMountPointW( uniquePath.c_str( ), mountedVolume_, volumeTCHARsize );
				if ( !b2 ) {
#ifdef DEBUG
					trace_GetVolumeNameForVolumeMountPoint_failed( uniquePath.c_str( ) );
#endif
					continue;
					}

				//TRACE( _T( "Found a mount point, path: %s, mountedVolume: %s \r\n" ), uniquePath.c_str( ), mountedVolume_ );
				
#ifdef DEBUG
				trace_mntpt_found( uniquePath.c_str( ), mountedVolume_ );
#endif
				//SPointVolume pv;
				//pv.point = point;
				//pv.volume = mountedVolume_;
				//pv.point.MakeLower( );

				//pointer_volume_array.emplace_back( pv );
				pointer_volume_array.emplace_back( std::make_pair( point, mountedVolume_ ) );
				}
			//VERIFY( FindVolumeMountPointClose( h ) );
			FindVolumeMountPointCloseHandle( h );
		
		
			//SAL catches this :)
			//FindVolumeMountPointCloseHandle( h );
		
			m_volume[ volume ] = std::move( pointer_volume_array );
			}
		const auto FindVolumeCloseRes = FindVolumeCloseHandle( hvol );
		if ( !( FindVolumeCloseRes ) ) {
			displayWindowsMsgBoxWithMessage( L"Failed to close a handle in CMountPoints::GetAllMountPoints. Something is wrong!" );
			std::terminate( );
			}
		}
	
	inline bool IsVolumeMountPoint( _In_ _In_range_( 0, ( M_DRIVE_ARRAY_SIZE - 1 ) ) const int index_in_m_drive, _In_ const std::wstring& path ) const {
		if ( m_volume.empty( ) ) {
			return false;
			}
		ASSERT( index_in_m_drive < M_DRIVE_ARRAY_SIZE );
		if ( index_in_m_drive >= M_DRIVE_ARRAY_SIZE ) {
			return false;
			}

		if ( m_volume.count( m_drive[ static_cast<size_t>( index_in_m_drive ) ] ) == 0 ) {
			TRACE( _T( "CMountPoints: Volume(%s) unknown!\r\n" ), m_drive[ static_cast<size_t>( index_in_m_drive ) ].c_str( ) );
			return false;
			}
		const std::vector<std::pair<std::wstring, std::wstring>>& pointer_volume_array = m_volume.at( m_drive[ static_cast<size_t>( index_in_m_drive ) ] );
		auto fixedPath( path );
		ASSERT( fixedPath.length( ) > 0 );
		if ( fixedPath.back( ) != _T( '\\' ) ) {
			fixedPath += _T( "\\" );
			}

		for ( const auto& aPoint : pointer_volume_array ) {
			//const auto len = aPoint.point.length( );
			const auto len = aPoint.first.length( );
			//if ( fixedPath.substr( 3 ).substr( 0, len ).compare( aPoint.point ) ==  0 ) {
			//	break;
			//	}
			if ( fixedPath.substr( 3 ).substr( 0, len ).compare( aPoint.first ) ==  0 ) {
				break;
				}
			if ( fixedPath.substr( 3 ).length( ) == len ) {
				return true;
				}
			}
		return false;
		}

	// m_drive contains the volume identifiers of the Drives A:, B: etc.
	// mdrive[0] = Volume identifier of A:\.
	_Field_size_( M_DRIVE_ARRAY_SIZE ) std::wstring m_drive[ M_DRIVE_ARRAY_SIZE ];

	std::unordered_map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> m_volume;
	};
#else

#endif
