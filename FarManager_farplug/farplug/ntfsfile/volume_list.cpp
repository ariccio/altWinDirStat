#define _ERROR_WINDOWS
#include "error.h"

#include "options.h"
#include "utils.h"
#include "ntfs.h"
#include "volume.h"
#include "ntfs_file.h"
#include "file_panel.h"

class VolumeEnum {
	private:
	HANDLE h_enum;
	static const unsigned c_buf_size = MAX_PATH;
	unique_ptr<wchar_t[ ]> volume_path;
	public:
	VolumeEnum( ) : h_enum( INVALID_HANDLE_VALUE ), volume_path( new wchar_t[ c_buf_size ] ) { }
	~VolumeEnum( ) {
		if ( h_enum != INVALID_HANDLE_VALUE ) {
			FindVolumeClose( h_enum );
			}
		}
	bool next( ) {
		if ( h_enum == INVALID_HANDLE_VALUE ) {
			h_enum = FindFirstVolumeW( volume_path.get( ), c_buf_size );
			if ( h_enum == INVALID_HANDLE_VALUE ) {
				if ( GetLastError( ) == ERROR_NO_MORE_FILES ) {
					return false;
					}
				CHECK_SYS( false );
				}
			}
		else {
			BOOL res = FindNextVolumeW( h_enum, volume_path.get( ), c_buf_size );
			if ( !res ) {
				if ( GetLastError( ) == ERROR_NO_MORE_FILES ) {
					return false;
					}
				CHECK_SYS( false );
				}
			}
		return true;
		}
	wstring get_volume_path( ) const {
		return volume_path.get( );
		}
	};

class LogicalDrivesEnum {
	private:
	const wchar_t* ptr;
	unique_ptr<wchar_t[ ]> drives;
	public:
	LogicalDrivesEnum( ) : ptr( nullptr ) { }
	bool next( ) {
		if ( ptr == nullptr ) {
			unsigned buf_size = MAX_PATH;
			drives.reset( new wchar_t[ buf_size ] );
			DWORD len = GetLogicalDriveStringsW( buf_size, drives.get( ) );
			if ( len > buf_size ) {
				buf_size = len;
				drives.reset( new wchar_t[ buf_size ] );
				len = GetLogicalDriveStringsW( buf_size, drives.get( ) );
				}
			CHECK_SYS( len );
			ptr = drives.get( );
			}
		else {
			ptr = ptr + wcslen( ptr ) + 1;
			}
		return *ptr != 0;
		}
	wstring get_drive( ) const {
		return ptr;
		}
	};

list<wstring> get_volume_mount_points( const wstring& volume_guid_path ) {
	unsigned buf_size = MAX_PATH;
	unique_ptr<wchar_t[ ]> buffer( new wchar_t[ buf_size ] );
	DWORD len;
	BOOL res = GetVolumePathNamesForVolumeNameW( add_trailing_slash( volume_guid_path ).c_str( ), buffer.get( ), buf_size, &len );
	if ( !res ) {
		if ( GetLastError( ) == ERROR_MORE_DATA ) {
			buf_size = len;
			buffer.reset( new wchar_t[ buf_size ] );
			res = GetVolumePathNamesForVolumeNameW( add_trailing_slash( volume_guid_path ).c_str( ), buffer.get( ), buf_size, &len );
			}
		CHECK_SYS( res );
		}
	list<wstring> result;
	const wchar_t* path = buffer.get( );
	while ( *path ) {
		unsigned len = wcslen( path );
		result.push_back( wstring( path, len ) );
		path += len + 1;
		}
	return result;
	}

bool get_device_path( const wstring& volume_guid_path, wstring& volume_dev_path ) {
	const wstring c_prefix( L"\\\\?\\" );
	if ( volume_guid_path.size( ) < c_prefix.size( ) || volume_guid_path.substr( 0, c_prefix.size( ) ) != c_prefix ) {
		return false;
		}
	unsigned buf_size = MAX_PATH;
	unique_ptr<wchar_t[ ]> buffer( new wchar_t[ buf_size ] );
	SetLastError( NO_ERROR );
	DWORD len = QueryDosDeviceW( del_trailing_slash( volume_guid_path ).substr( 4 ).c_str( ), buffer.get( ), buf_size );
	if ( len == 0 || GetLastError( ) != NO_ERROR ) {
		return false;
		}
	volume_dev_path.assign( buffer.get( ) );
	return true;
	}

bool get_volume_guid_path( const wstring& volume_mount_point, wstring& volume_guid_path ) {
	unsigned buf_size = MAX_PATH;
	unique_ptr<wchar_t[ ]> buffer( new wchar_t[ buf_size ] );
	if ( !::GetVolumeNameForVolumeMountPointW( add_trailing_slash( volume_mount_point ).c_str( ), buffer.get( ), buf_size ) ) {
		return false;
		}
	volume_guid_path.assign( buffer.get( ) );
	return true;
	}

struct VolumeItem {
	wstring guid_path;
	wstring drive_path;
	wstring dev_path;
	list<wstring> mount_points;
	};

list<VolumeItem> enum_volumes( ) {
	list<VolumeItem> vol_list;
	LogicalDrivesEnum drive_enum;
	map<wstring, wstring> vol_guid_to_drive_map;
	while ( drive_enum.next( ) ) {
		wstring drive = drive_enum.get_drive( );
		wstring vol_guid_path;
		if ( get_volume_guid_path( drive, vol_guid_path ) ) {
			vol_guid_to_drive_map[ vol_guid_path ] = drive;
			}
		}

	VolumeEnum vol_enum;
	while ( vol_enum.next( ) ) {
		VolumeItem vol_item;
		vol_item.guid_path = vol_enum.get_volume_path( );
		auto vol_iter = vol_guid_to_drive_map.find( vol_item.guid_path );
		if ( vol_iter != vol_guid_to_drive_map.end( ) ) {
			vol_item.drive_path = vol_iter->second;
			}

		get_device_path( vol_item.guid_path, vol_item.dev_path );

		vol_item.mount_points = get_volume_mount_points( vol_item.guid_path );
		auto mp_iter = vol_item.mount_points.begin( );
		while ( mp_iter != vol_item.mount_points.end( ) ) {
			if ( is_root_path( std::string( mp_iter->data( ), mp_iter->size( ) ) ) ) {
				mp_iter = vol_item.mount_points.erase( mp_iter );
				}
			else {
				++mp_iter;
				}
			}

		vol_list.push_back( vol_item );
		}

	return vol_list;
	}

PluginItemList FilePanel::create_volume_items( ) {
	auto volume_list = enum_volumes( );
	PluginItemList pi_list;
	pi_list.extend( volume_list.size( ) );
	pi_list.names.extend( volume_list.size( ) );
	pi_list.col_str.extend( volume_list.size( ) * 2 );
	pi_list.col_data.extend( volume_list.size( ) );
	unsigned col_N_width = 0;
	unsigned col_C0_width = 0;
	unsigned col_C1_width = 0;
	Array<const wchar_t*> col_data;
	for ( auto vol_iter = volume_list.cbegin( ); vol_iter != volume_list.cend( ); ++vol_iter ) {
		wstring name = vol_iter->drive_path.empty( ) ? vol_iter->guid_path : vol_iter->drive_path;
		if ( name.size( ) > col_N_width ) {
			col_N_width = name.size( );
			}
		pi_list.names += std::string( name.data( ), name.size( ) );
		PluginPanelItem pi;
		memzero( pi );
		pi.FileName = pi_list.names.last( ).data( );
		pi.FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		// custom columns
		col_data.clear( );
		pi_list.col_str += std::string( vol_iter->dev_path.data( ), vol_iter->dev_path.size( ) );
		col_data += pi_list.col_str.last( ).data( );
		if ( vol_iter->dev_path.size( ) > col_C0_width ) {
			col_C0_width = vol_iter->dev_path.size( );
			}
		std::string mount_points;
		for ( auto mp_iter = vol_iter->mount_points.cbegin( ); mp_iter != vol_iter->mount_points.cend( ); ++mp_iter ) {
			if ( mount_points.size( ) ) {
				mount_points += L';';
				}
			mount_points.add( mp_iter->data( ), mp_iter->size( ) );
			}
		pi_list.col_str += mount_points;
		col_data += pi_list.col_str.last( ).data( );
		if ( mount_points.size( ) > col_C1_width ) {
			col_C1_width = mount_points.size( );
			}
		pi_list.col_data += col_data;
		pi.CustomColumnData = const_cast< wchar_t** >( pi_list.col_data.last( ).data( ) );
		pi.CustomColumnNumber = pi_list.col_data.last( ).size( );
		pi_list += pi;
		}
	if ( col_N_width < 7 ) {
		col_N_width = 7;
		}
	col_widths.copy_fmt( L"%u,%u,%u", col_N_width, col_C0_width, col_C1_width );
	return pi_list;
	}