// mountpoints.h	- Declaration of CMountPoins
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once

#ifndef WDS_MOUNTPOINTS_CPP_INCLUDED
#define WDS_MOUNTPOINTS_CPP_INCLUDED

#include "mountpoints.h"
#include "globalhelpers.h"

WDS_FILE_INCLUDE_MESSAGE


namespace {
	void FindVolumeMountPointCloseHandle( _In_ _Post_invalid_ HANDLE hFindVolumeMountPoint ) noexcept {
		VERIFY( ::FindVolumeMountPointClose( hFindVolumeMountPoint ) );
		}
	BOOL FindVolumeCloseHandle( _In_ _Post_invalid_ HANDLE hFindVolume ) noexcept {
		return ::FindVolumeClose( hFindVolume );
		}
	}

const bool CMountPoints::IsVolume( _In_ const std::wstring& path ) const {
	if ( ( path.length( ) < 2 ) || ( path[ 1 ] != L':' ) || ( path.length( ) > 3 ) ) {
		// Don't know how to make out mount points on UNC paths ###
		return false;
		}
	ASSERT( ( path.at( 1 )  == L':' ) );

	const auto pathAtZero = ::towlower( path[ 0 ] );
	const auto weirdAss_a = _T( 'a' );
	const auto indexItem  = pathAtZero - weirdAss_a;
	//return IsVolumeMountPoint( indexItem, path );
	ASSERT( indexItem < M_DRIVE_ARRAY_SIZE );
	if ( indexItem >= M_DRIVE_ARRAY_SIZE ) {
		return false;
		}

	return true;
	}



const bool CMountPoints::IsMountPoint( _In_ const std::wstring& path ) const {
	if ( ( path.length( ) < 3 ) || ( path[ 1 ] != L':' ) || ( path[ 2 ] != L'\\' ) ) {
		// Don't know how to make out mount points on UNC paths ###
		return false;
		}
	if ( m_volume.empty( ) ) {
		return false;
		}
	ASSERT( ( path.length( ) >= 3 ) && ( path.at( 1 )  == L':' ) && ( path.at( 2 )  == L'\\' ) );

	const std::wint_t pathAtZero = ::towlower( path[ 0 ] );
	const wchar_t weirdAss_a = L'a';
	if( weirdAss_a > pathAtZero ){
		std::terminate();
		}
	const auto indexItem  = pathAtZero - weirdAss_a;
	if ( indexItem < 0 ){
		std::terminate();
		}
	return IsVolumeMountPoint( static_cast<rsize_t>( indexItem ), path );
	}

void CMountPoints::Clear( ) noexcept {
	for ( size_t i = 0; i < M_DRIVE_ARRAY_SIZE; ++i ) {
		m_drive[ i ].clear( );
		}
	m_volume.clear( );
	}

const bool CMountPoints::IsVolumeMountPoint( _In_ _In_range_( 0, ( M_DRIVE_ARRAY_SIZE - 1 ) ) const rsize_t index_in_m_drive, _In_ const std::wstring& path ) const {
	if ( m_volume.empty( ) ) {
		return false;
		}
	ASSERT( index_in_m_drive < M_DRIVE_ARRAY_SIZE );
	if ( index_in_m_drive >= M_DRIVE_ARRAY_SIZE ) {
		return false;
		}

	if ( m_volume.count( m_drive[ index_in_m_drive ] ) == 0 ) {
		TRACE( _T( "CMountPoints: Volume(%s) unknown!\r\n" ), m_drive[ index_in_m_drive ].c_str( ) );
		return false;
		}
	const std::vector<std::pair<std::wstring, std::wstring>>& pointer_volume_array = m_volume.at( m_drive[ index_in_m_drive ] );
	std::wstring fixedPath( path );
	ASSERT( fixedPath.length( ) > 0 );
	if ( fixedPath.back( ) != _T( '\\' ) ) {
		fixedPath += _T( "\\" );
		}

	for ( const auto& aPoint : pointer_volume_array ) {
		const auto len = aPoint.first.length( );
		//Ok, this is terribly wasteful.
		if ( fixedPath.substr( 3 ).substr( 0, len ).compare( aPoint.first ) ==  0 ) {
			break;
			}
		if ( fixedPath.substr( 3 ).length( ) == len ) {
			return true;
			}
		}
	return false;
	}

void CMountPoints::GetDriveVolumes( ) noexcept {
	const rsize_t s_char_buffer_size = 5u;
	_Null_terminated_ wchar_t small_buffer_volume_name[ s_char_buffer_size ] = { 0 };
	const auto drives = ::GetLogicalDrives( );
	DWORD mask = 0x00000001;

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( INT i = 0; i < static_cast<INT>( M_DRIVE_ARRAY_SIZE ); i++, mask <<= 1 ) {
		const rsize_t larger_buffer_size = MAX_PATH;
		_Null_terminated_ wchar_t volume_[ larger_buffer_size ] = { 0 };
		if ( ( drives bitand mask ) != 0 ) {
			const auto swps = swprintf_s( small_buffer_volume_name, L"%c:\\", ( i + _T( 'A' ) ) );
			if ( swps == -1 ) {
				::MessageBoxW( NULL, L"unexpected error in CMountPoints::GetDriveVolumes!!(aborting)", L"Error", MB_OK );
				std::terminate( );
				}
			ASSERT( wcslen( small_buffer_volume_name ) < 5 );

			const BOOL b = ::GetVolumeNameForVolumeMountPointW( small_buffer_volume_name, volume_, larger_buffer_size );
			if ( !b ) {
#ifdef DEBUG
				TRACE( _T( "GetVolumeNameForVolumeMountPoint(%s) failed.\r\n" ), small_buffer_volume_name );
#endif
				}
			}
		m_drive[ static_cast<size_t>( i ) ] = volume_;
		}
	}


void CMountPoints::GetAllMountPoints( ) {
	const rsize_t volumeTCHARsize = MAX_PATH;
	_Null_terminated_ wchar_t volume[ volumeTCHARsize ] = { 0 };
	const HANDLE hvol = ::FindFirstVolumeW( volume, volumeTCHARsize );
	if ( hvol == INVALID_HANDLE_VALUE ) {
		TRACE( _T( "No volumes found.\r\n" ) );
#ifdef DEBUG
		const DWORD lastErr = ::GetLastError( );
		TRACE( _T( "FindFirstVolumeW failed, error: `%lu`.\r\n" ), lastErr, volume );

		const rsize_t err_buf_size = 1024u;
		wchar_t err_buff[ err_buf_size ] = { 0 };
		rsize_t chars_written_unused = 0u;
		const HRESULT err_fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buf_size, chars_written_unused, lastErr );
		if ( SUCCEEDED( err_fmt_res ) ) {
			TRACE( L"FindFirstVolumeW error message: %s\r\n", err_buff );
			}
		else {
			TRACE( L"Failed to format the FindFirstVolumeW error message!\r\n" );
			}
#endif
		return;
		}

	for ( BOOL bContinue = true; bContinue; bContinue = ::FindNextVolumeW( hvol, volume, volumeTCHARsize ) ) {

		DWORD sysflags;
		_Null_terminated_ wchar_t fsname_[ volumeTCHARsize ] = { 0 };
		const BOOL b = ::GetVolumeInformationW( volume, NULL, 0, NULL, NULL, &sysflags, fsname_, volumeTCHARsize );
		if ( !b ) {
#ifdef DEBUG
			const DWORD lastErr = ::GetLastError( );
			TRACE( _T( "GetVolumeInformationW failed, error: `%lu`. File system (%s) is not ready?\r\n" ), lastErr, volume );

			const rsize_t err_buf_size = 1024u;
			wchar_t err_buff[ err_buf_size ] = { 0 };
			rsize_t chars_written_unused = 0u;
			const HRESULT err_fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buf_size, chars_written_unused, lastErr );
			if ( SUCCEEDED( err_fmt_res ) ) {
				TRACE( L"GetVolumeInformationW error message: %s\r\n", err_buff );
				}
			else {
				TRACE( L"Failed to format the GetVolumeInformationW error message!\r\n" );
				}
#endif
			continue;
			}

		if ( ( sysflags bitand FILE_SUPPORTS_REPARSE_POINTS ) == 0 ) {
#ifdef DEBUG
			TRACE( _T( "This file system (%s) does not support reparse points, and therefore does not support volume mount points.\r\n" ), volume );
#endif
			continue;
			}

		_Null_terminated_ wchar_t point[ volumeTCHARsize ] = { 0 };
		const HANDLE h = ::FindFirstVolumeMountPointW( volume, point, volumeTCHARsize );
		if ( h == INVALID_HANDLE_VALUE ) {
#ifdef DEBUG
			const DWORD lastErr = ::GetLastError( );
			TRACE( _T( "FindFirstVolumeMountPointW failed, error: `%lu`. No volume mnt pts on (%s)?\r\n" ), lastErr, volume );

			const rsize_t err_buf_size = 1024u;
			wchar_t err_buff[ err_buf_size ] = { 0 };
			rsize_t chars_written_unused = 0u;
			const HRESULT err_fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buf_size, chars_written_unused, lastErr );
			if ( SUCCEEDED( err_fmt_res ) ) {
				TRACE( L"FindFirstVolumeMountPointW error message: %s\r\n", err_buff );
				}
			else {
				TRACE( L"Failed to format the FindFirstVolumeMountPointW error message!\r\n" );
				}

#endif
			continue;
			}

		std::vector<std::pair<std::wstring, std::wstring>> pointer_volume_array;
		for ( BOOL bCont = true; bCont; bCont = ::FindNextVolumeMountPointW( h, point, volumeTCHARsize ) ) {
			std::wstring uniquePath_temp( volume );
			uniquePath_temp += point;
			const std::wstring uniquePath( std::move( uniquePath_temp ) );
			_Null_terminated_ wchar_t mountedVolume_[ volumeTCHARsize ] = { 0 };
			const BOOL b2 = ::GetVolumeNameForVolumeMountPointW( uniquePath.c_str( ), mountedVolume_, volumeTCHARsize );
			if ( !b2 ) {
#ifdef DEBUG
				const DWORD lastErr = ::GetLastError( );
				TRACE( _T( "GetVolumeNameForVolumeMountPoint(%s) failed, error: `%lu`.\r\n" ), uniquePath.c_str( ), lastErr );

				const rsize_t err_buf_size = 1024u;
				wchar_t err_buff[ err_buf_size ] = { 0 };
				rsize_t chars_written_unused = 0u;
				const HRESULT err_fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buf_size, chars_written_unused, lastErr );
				if ( SUCCEEDED( err_fmt_res ) ) {
					TRACE( L"GetVolumeNameForVolumeMountPoint error message: %s\r\n", err_buff );
					}
				else {
					TRACE( L"Failed to format the GetVolumeNameForVolumeMountPoint error message!\r\n" );
					}

#endif
				continue;
				}
#ifdef DEBUG
			TRACE( _T( "Found a mount point, path: %s, mountedVolume: %s \r\n" ), uniquePath.c_str( ), mountedVolume_ );
#endif
			pointer_volume_array.emplace_back( std::make_pair( point, mountedVolume_ ) );
			}
		FindVolumeMountPointCloseHandle( h );
		
		
		//SAL catches this :)
		//FindVolumeMountPointCloseHandle( h );
		
		m_volume[ volume ] = std::move( pointer_volume_array );
		}
	const DWORD lastErr = ::GetLastError( );
	ASSERT( lastErr == ERROR_NO_MORE_FILES );

	if ( lastErr != ERROR_NO_MORE_FILES ) {
		//TODO: WTF?

		}


	const auto FindVolumeCloseRes = ::FindVolumeCloseHandle( hvol );
	if ( !( FindVolumeCloseRes ) ) {
		::MessageBoxW( NULL, L"Failed to close a handle in CMountPoints::GetAllMountPoints. Something is wrong!", L"Error", MB_OK );
		std::terminate( );
		}
	}


#endif