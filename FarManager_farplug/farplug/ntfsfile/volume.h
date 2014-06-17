#pragma once

struct VolumeInfo {
	std::string name;
	unsigned cluster_size;
	VolumeInfo( const std::string& file_name );
	};

struct NtfsVolume {
	std::string name;
	DWORD serial;
	unsigned file_rec_size;
	unsigned cluster_size;
	unsigned __int64 mft_size;
	HANDLE handle;
	bool synced;
	NtfsVolume( ) : handle( INVALID_HANDLE_VALUE ), serial( 0 ) { }
	~NtfsVolume( ) {
		close( );
		}
	void close( ) {
		if ( handle != INVALID_HANDLE_VALUE ) {
			CHECK_SYS( CloseHandle( handle ) );
			handle = INVALID_HANDLE_VALUE;
			}
		name.clear( );
		serial = 0;
		}
	void open( const std::string& volume_name );
	void flush( );
	};

std::string get_real_path( const std::string& fp );
std::string get_volume_guid( const std::string& volume_name );
