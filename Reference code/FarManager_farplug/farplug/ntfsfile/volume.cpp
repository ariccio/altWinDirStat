#define _ERROR_WINDOWS
#include "error.h"

#include "utils.h"
#include "ntfs.h"
#include "options.h"
#include "volume.h"

extern struct FarStandardFunctions g_fsf;

const unsigned c_max_rp = 256;

const wchar_t* c_invalid_symlink_path = L"Invalid symbolic link target";
const wchar_t* c_invalid_unc_path = L"Invalid UNC path";
const wchar_t* c_too_many_rp = L"Path contains too many reparse points";
const wchar_t* c_unknown_rp_tag = L"Unknown reparse tag";

void clean_path( std::string& path ) {
	if ( path.equal( 0, L"\\??\\" ) || path.equal( 0, L"\\\\?\\" ) ) {
		path.remove( 0, 4 );
		}
	unsigned i = 0;
	while ( i < path.size( ) ) {
		if ( path[ i ] == L'/' ) {
			path.item( i ) = L'\\';
			}
		if ( ( i > 1 ) && ( path[ i ] == L'\\' ) && ( path[ i - 1 ] == L'\\' ) ) {
			path.remove( i, 1 );
			}
		else {
			i++;
			}
		}
	path = del_trailing_slash( path );
	}

// expand symbolic links / mount points and find real file path
// possible return values:
// c:\path\...
// \\?\Volume{1edbf4ba-e9fe-11dc-86b3-005056c00008}\path\...
// \\?\UNC\server\share\path\...
std::string get_real_path( const std::string& fp ) {
	std::string real_path;
	const unsigned c_buf_size = 0x10000;
	int size = g_fsf.ConvertPath( CPM_REAL, fp.data( ), real_path.buf( c_buf_size ), c_buf_size );
	if ( size > c_buf_size ) {
		g_fsf.ConvertPath( CPM_REAL, fp.data( ), real_path.buf( size ), size );
		}
	real_path.set_size( );
	DBG_LOG( L"get_real_path() = '" + real_path + L"'" );
	return real_path;
	}

VolumeInfo::VolumeInfo( const std::string& file_name ) {
	name = extract_path_root( get_real_path( file_name ) );

	DWORD SectorsPerCluster;
	DWORD BytesPerSector;
	DWORD NumberOfFreeClusters;
	DWORD TotalNumberOfClusters;
	CHECK_SYS( GetDiskFreeSpaceW( add_trailing_slash( name ).data( ), &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) );
	cluster_size = SectorsPerCluster * BytesPerSector;
	}

std::string get_volume_path( const std::string& volume_name ) {
	if ( volume_name.equal( 0, L"\\\\?\\" ) ) {
		return volume_name;
		}
	else {
		return L"\\\\.\\" + volume_name;
		}
	}

void NtfsVolume::open( const std::string& volume_name ) {
	close( );
	try {
		name = volume_name;
		synced = false;

		CHECK_MSG( !is_unc_path( name ), L"Network shares are not supported" );

		/* get volume information */
		wchar_t vlm_label[ MAX_PATH ];
		DWORD vlm_comp_len;
		DWORD flags;
		wchar_t vlm_fs[ MAX_PATH ];
		CHECK_SYS( GetVolumeInformationW( add_trailing_slash( name ).data( ), vlm_label, ARRAYSIZE( vlm_label ), &serial, &vlm_comp_len, &flags, vlm_fs, ARRAYSIZE( vlm_fs ) ) );

		CHECK_MSG( _wcsicmp( vlm_fs, L"NTFS" ) == 0, L"Only NTFS volumes are supported" );

		/* allocate volume handle */
		handle = CreateFileW( get_volume_path( name ).data( ), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL );
		CHECK_SYS( handle != INVALID_HANDLE_VALUE );

		/* get NTFS specific volume information */
		DWORD bytes_ret;
		NTFS_VOLUME_DATA_BUFFER ntfs_vol_data;
		CHECK_SYS( DeviceIoControl( handle, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &ntfs_vol_data, sizeof( ntfs_vol_data ), &bytes_ret, NULL ) );
		file_rec_size = ntfs_vol_data.BytesPerFileRecordSegment;
		cluster_size = ntfs_vol_data.BytesPerCluster;
		mft_size = ntfs_vol_data.MftValidDataLength.QuadPart;
		}
	catch ( ... ) {
		close( );
		throw;
		}
	}

void NtfsVolume::flush( ) {
	if ( !synced ) {
		HANDLE handle = CreateFileW( get_volume_path( name ).data( ), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
		if ( handle != INVALID_HANDLE_VALUE ) {
			FlushFileBuffers( handle );
			CloseHandle( handle );
			DBG_LOG( std::string( L"volume flushed" ) );
			}
		synced = true;
		}
	}

std::string get_volume_guid( const std::string& volume_name ) {
	std::string volume_id;
	const unsigned c_max_volume_id_size = 50;
	CHECK_SYS( GetVolumeNameForVolumeMountPointW( add_trailing_slash( volume_name ).data( ), volume_id.buf( c_max_volume_id_size ), c_max_volume_id_size ) );
	volume_id.set_size( );
	unsigned b_idx = volume_id.search( L'{' );
	unsigned e_idx = volume_id.search( b_idx, L'}' );
	if ( ( b_idx == -1 ) || ( e_idx == -1 ) ) {
		FAIL( MsgError( L"Unexpected volume name" ) )
		}
	return volume_id.slice( b_idx + 1, e_idx - b_idx - 1 );
	}