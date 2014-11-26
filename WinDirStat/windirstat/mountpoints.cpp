// mountpoints.cpp	- Implementation of CMountPoints
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
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

#include "stdafx.h"

#include "mountpoints.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//This code is REALLY scary. TODO: cleanup.

void CMountPoints::Clear( ) {
	//m_drive.RemoveAll();
	m_drive.clear();
	m_volume.clear( );
	}

void CMountPoints::GetDriveVolumes( ) {
	m_drive.resize( 32 );

	const rsize_t volumeTCHARsize = MAX_PATH;

	wchar_t s_[ volumeTCHARsize ] = { 0 };
	auto drives = GetLogicalDrives( );
	DWORD mask = 0x00000001;
	for ( INT i = 0; i < 32; i++, mask <<= 1 ) {
		wchar_t volume_[ volumeTCHARsize ] = { 0 };
		if ( ( drives bitand mask ) != 0 ) {
			const auto swps = swprintf_s( s_, L"%c:\\", ( i + _T( 'A' ) ) );

			ENSURE( swps != -1 );

			BOOL b = GetVolumeNameForVolumeMountPointW( s_, volume_, volumeTCHARsize );
			if ( !b ) {
				TRACE( _T( "GetVolumeNameForVolumeMountPoint(%s) failed.\r\n" ), s_ );
				}
			}
		m_drive.at( i ) = volume_;
		}
	}

void CMountPoints::GetAllMountPoints( ) {
	const rsize_t volumeTCHARsize = MAX_PATH;
	wchar_t volume[ volumeTCHARsize ] = { 0 };
	HANDLE hvol = FindFirstVolumeW( volume, volumeTCHARsize );
	if ( hvol == INVALID_HANDLE_VALUE ) {
		TRACE( _T( "No volumes found.\r\n" ) );
		return;
		}

	for ( BOOL bContinue = true; bContinue; bContinue = FindNextVolumeW( hvol, volume, volumeTCHARsize ) ) {

		DWORD sysflags;
		wchar_t fsname_[ volumeTCHARsize ] = { 0 };
		BOOL b = GetVolumeInformationW( volume, NULL, 0, NULL, NULL, &sysflags, fsname_, volumeTCHARsize );
		if ( !b ) {
			TRACE( _T( "File system (%s) is not ready.\r\n" ), volume );
			m_volume[ volume ] = std::make_unique<std::vector<SPointVolume>>( );
			continue;
			}

		if ( ( sysflags bitand FILE_SUPPORTS_REPARSE_POINTS ) == 0 ) {
			TRACE( _T( "This file system (%s) does not support reparse points, and therefore does not support volume mount points.\r\n" ), volume );
			m_volume[ volume ] = std::make_unique<std::vector<SPointVolume>>( );
			continue;
			}

		wchar_t point[ volumeTCHARsize ] = { 0 };
		HANDLE h = FindFirstVolumeMountPointW( volume, point, volumeTCHARsize );
		if ( h == INVALID_HANDLE_VALUE ) {
			TRACE( _T( "No volume mnt pts on (%s).\r\n" ), volume );
			m_volume[ volume ] = std::make_unique<std::vector<SPointVolume>>( );
			continue;
			}

		auto pva = std::make_unique<std::vector<SPointVolume>>( );
		for ( BOOL bCont = true; bCont; bCont = FindNextVolumeMountPointW( h, point, volumeTCHARsize ) ) {
			std::wstring uniquePath( volume );
			uniquePath += point;
			wchar_t mountedVolume_[ volumeTCHARsize ] = { 0 };
			BOOL b2 = GetVolumeNameForVolumeMountPointW( uniquePath.c_str( ), mountedVolume_, volumeTCHARsize );
			if ( !b2 ) {
				TRACE( _T( "GetVolumeNameForVolumeMountPoint(%s) failed.\r\n" ), uniquePath.c_str( ) );
				continue;
				}

			TRACE( _T( "Found a mount point, path: %s, mountedVolume: %s \r\n" ), uniquePath.c_str( ), mountedVolume_ );
			SPointVolume pv;
			pv.point = point;
			pv.volume = mountedVolume_;
			pv.point.MakeLower( );

			pva->emplace_back( pv );
			}
		FindVolumeMountPointClose( h );
		m_volume[ volume ] = std::move( pva );
		}
	auto FindVolumeCloseRes = FindVolumeClose( hvol );
	ENSURE( FindVolumeCloseRes );
	}


bool CMountPoints::IsMountPoint( _In_ const std::wstring& path ) const {
	if ( path.length( ) < 3 || path.at( 1 ) != L':' || path.at( 2 ) != L'\\' ) {
		// Don't know how to make out mount points on UNC paths ###
		return false;
		}
	if ( m_volume.empty( ) ) {
		//ASSERT( !IsVolumeMountPoint( volume, path ) );
		return false;
		}
	ASSERT( ( path.length( ) >= 3 ) && ( path.at( 1 )  == L':' ) && ( path.at( 2 )  == L'\\' ) );

	const auto pathAtZero = towlower( path.at( 0 ) );
	const auto weirdAss_a = _T( 'a' );
	const auto indexItem  = pathAtZero - weirdAss_a;
	return IsVolumeMountPoint( indexItem, path.c_str( ) );
	}

bool CMountPoints::IsJunctionPoint( _In_ const std::wstring& path, _In_ const DWORD fAttributes) const {
	/*
	  Check whether the current item is a junction point but no volume mount point as the latter ones are treated differently (see above).
	  CAN ALSO BE A REPARSE POINT!
	*/
	if ( fAttributes == INVALID_FILE_ATTRIBUTES ) {
		return false;
		}
	if ( IsMountPoint( path ) ) {
		return false;
		}
	return ( ( fAttributes bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
	}


bool CMountPoints::IsJunctionPoint( _In_ const std::wstring& path, _In_ const attribs& attr ) const {
	/*
	  Check whether the current item is a junction point but no volume mount point as the latter ones are treated differently (see above).
	  CAN ALSO BE A REPARSE POINT!
	*/
	if ( attr.invalid ) {
		return false;
		}
	if ( IsMountPoint( path ) ) {
		return false;
		}
	return ( attr.reparse );
	}


bool CMountPoints::IsVolumeMountPoint( _In_ const int index_in_m_drive, _In_ const CString& path ) const {
	if ( m_volume.empty( ) ) {
		return false;
		}
	if ( m_volume.count( m_drive.at( index_in_m_drive ) ) == 0 ) {
		TRACE( _T( "CMountPoints: Volume(%s) unknown!\r\n" ), m_drive.at( index_in_m_drive ).c_str( ) );
		return false;
		}
	auto pva = m_volume.at( m_drive.at( index_in_m_drive ) ).get( );
	auto fixedPath = path;
	if ( fixedPath.Right( 1 ) != _T( '\\' ) ) {
		fixedPath += _T( "\\" );
		}

	for ( const auto& aPoint : *pva ) {
		const auto len = aPoint.point.GetLength( );
		if ( fixedPath.Mid( 3 ).Left( len ).CompareNoCase( aPoint.point ) ==  0 ) {
			break;
			}
		if ( fixedPath.Mid( 3 ).GetLength( ) == len ) {
			return true;
			}
		}
	return false;
	}

// $Log$
// Revision 1.6  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.5  2004/12/31 16:01:42  bseifert
// Bugfixes. See changelog 2004-12-31.
//
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
