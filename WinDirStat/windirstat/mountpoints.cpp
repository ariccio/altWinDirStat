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
//#include "osspecific.h"

//#include "mountpoints.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//This code is REALLY scary. TODO: cleanup.

//CMountPoints::~CMountPoints( ) {
//	Clear();
//	}

void CMountPoints::Clear( ) {
	m_drive.RemoveAll();
	auto pos = m_volume.GetStartPosition( );
	while ( pos != NULL ) {
		CString volume;
		CArray<SPointVolume, SPointVolume&>* pva = NULL;
		m_volume.GetNextAssoc( pos, volume, pva );
		ASSERT_VALID( pva );
		delete pva;
		pva = NULL;
		}
	m_volume.RemoveAll( );
	}

//void CMountPoints::Initialize( ) {
//	Clear( );
//	GetDriveVolumes( );
//	GetAllMountPoints( );
//	}

void CMountPoints::GetDriveVolumes( ) {
	m_drive.SetSize( 32 );

	auto drives = GetLogicalDrives( );
	//INT i = 0;
	DWORD mask = 0x00000001;
	for ( INT i = 0; i < 32; i++, mask <<= 1 ) {
		CString volume;
		if ( ( drives & mask ) != 0 ) {
			CString s;
			s.Format( _T( "%c:\\" ), i + _T( 'A' ) );

			BOOL b = GetVolumeNameForVolumeMountPoint( s, volume.GetBuffer( _MAX_PATH ), _MAX_PATH );
			volume.ReleaseBuffer( );

			if ( !b ) {
				TRACE( _T( "GetVolumeNameForVolumeMountPoint(%s) failed.\r\n" ), s );
				volume.Empty( );
				}
			}
		m_drive[ i ] = volume;
		}
	}

void CMountPoints::GetAllMountPoints( ) {
	TCHAR volume[ _MAX_PATH ] = { 0 };
	HANDLE hvol = FindFirstVolume( volume, countof( volume ) );
	if ( hvol == INVALID_HANDLE_VALUE ) {
		TRACE( _T( "No volumes found.\r\n" ) );
		return;
		}

	for ( BOOL bContinue = true; bContinue; bContinue = FindNextVolume( hvol, volume, countof( volume ) ) ) {
		auto pva = new CArray<SPointVolume, SPointVolume&>;
		ASSERT_VALID( pva );

		DWORD sysflags;
		CString fsname;
		BOOL b = GetVolumeInformation( volume, NULL, 0, NULL, NULL, &sysflags, fsname.GetBuffer( _MAX_PATH ), _MAX_PATH );
		fsname.ReleaseBuffer( );

		if ( !b ) {
			TRACE( _T( "This file system (%s) is not ready.\r\n" ), volume );
			m_volume.SetAt( volume, pva );
			continue;
			}

		if ( ( sysflags & FILE_SUPPORTS_REPARSE_POINTS ) == 0 ) {
			TRACE( _T( "This file system (%s) does not support reparse points, and therefore does not support volume mount points.\r\n" ), volume );
			m_volume.SetAt( volume, pva );
			continue;
			}

		TCHAR point[ _MAX_PATH ] = { 0 };
		HANDLE h = FindFirstVolumeMountPoint( volume, point, countof( point ) );
		if ( h == INVALID_HANDLE_VALUE ) {
			TRACE( _T( "No volume mount points found on (%s).\r\n" ), volume );
			m_volume.SetAt( volume, pva );
			continue;
			}
		TRACE( _T( "Found a mount point!\r\n" ) );

		for ( BOOL bCont = true; bCont; bCont = FindNextVolumeMountPoint( h, point, countof( point ) ) ) {
			CString uniquePath = volume;
			uniquePath += point;
			CString mountedVolume;

			BOOL b2 = GetVolumeNameForVolumeMountPoint( uniquePath, mountedVolume.GetBuffer( _MAX_PATH ), _MAX_PATH );
			mountedVolume.ReleaseBuffer( );

			if ( !b2 ) {
				TRACE( _T( "GetVolumeNameForVolumeMountPoint(%s) failed.\r\n" ), uniquePath );
				continue;
				}
			TRACE( _T( "Found a mount point, path: %s, mountedVolume: %s \r\n" ), uniquePath, mountedVolume );

			SPointVolume pv;
			pv.point = point;
			pv.volume = mountedVolume;

			pv.point.MakeLower( );

			pva->Add( pv );
			}
		FindVolumeMountPointClose( h );
		m_volume.SetAt( volume, pva );
		}
	auto FindVolumeCloseRes = FindVolumeClose( hvol );

	ASSERT( FindVolumeCloseRes );

#ifdef _DEBUG
	auto pos = m_volume.GetStartPosition( );
	while ( pos != NULL ) {
		CString volume_str;
		CArray<SPointVolume, SPointVolume&> *pva = NULL;
		m_volume.GetNextAssoc( pos, volume_str, pva );
		pva->AssertValid( );
		}
#endif
	}


bool CMountPoints::IsMountPoint( _In_ CString path ) const {
	if ( path.GetLength( ) < 3 || path[ 1 ] != _T( ':' ) || path[ 2 ] != _T( '\\' ) ) {
		// Don't know how to make out mount points on UNC paths ###
		return false;
		}

	ASSERT( ( path.GetLength( ) >= 3 ) && ( path[ 1 ] == _T( ':' ) ) && ( path[ 2 ] == _T( '\\' ) ) );

	if ( path.Right( 1 ) != _T( '\\' ) ) {
		path += _T( "\\" );
		}

	path.MakeLower( );

	CString volume = m_drive[ path[ 0 ] - _T( 'a' ) ];
	path = path.Mid( 3 );

	return IsVolumeMountPoint( volume, path );
	}


//bool CMountPoints::IsJunctionPoint( _In_ CString path ) const {
//	/*
//	  Check whether the current item is a junction point but no volume mount point as the latter ones are treated differently (see above).
//	  CAN ALSO BE A REPARSE POINT!
//	*/
//	if ( IsMountPoint( path ) ) {
//		return false;
//		}
//
//	auto attr = GetFileAttributes( path );
//	if ( attr == INVALID_FILE_ATTRIBUTES ) {
//		return false;
//		}
//
//	return ( ( attr & FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
//	}

bool CMountPoints::IsJunctionPoint( _In_ CString path, _In_ DWORD fAttributes) const {
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

	return ( ( fAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
	}


bool CMountPoints::IsVolumeMountPoint( _In_ CString volume, _In_ CString path ) const {
	for (;;) {//ENDLESS loop
		CArray<SPointVolume, SPointVolume&>* pva = NULL;
		if ( !m_volume.Lookup( volume, pva ) ) {
			TRACE( _T( "CMountPoints: Volume(%s) unknown!\r\n" ), volume );
			return false;
			}
		ASSERT( pva != NULL );
		TRACE( _T( "CMountPoints: Volume(%s) known!\r\n" ), volume );
		CString point;
		for ( INT i = 0; i < pva->GetSize( ); i++ ) {
			point = ( *pva )[ i ].point;
			if ( path.Left( point.GetLength( ) ) == point ) {
				break;
				}
			if ( i >= pva->GetSize( ) ) {
				return false;
				}
			if ( path.GetLength( ) == point.GetLength( ) ) {
				return true;
				}
			volume = ( *pva )[ i ].volume;
			path = path.Mid( point.GetLength( ) );
			}
		return false;
		}
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
