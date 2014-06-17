#pragma once
#include <Windows.h>

struct AttrInfo {
	u32 type;
	bool mft_ext_rec;
	bool resident;
	bool compressed;
	bool encrypted;
	bool sparse;
	u64 data_size;
	u64 disk_size;
	u64 valid_size;
	std::string name;
	u64 fragments;
	std::string type_name( ) const;
	};

struct FileNameAttr {
	u64 parent_directory;
	u64 creation_time;
	u64 last_data_change_time;
	u64 last_mft_change_time;
	u64 last_access_time;
	u32 file_attributes;
	std::string name;
	u8 file_name_type;
	};

struct StdInfo {
	u64 creation_time;
	u64 last_data_change_time;
	u64 last_mft_change_time;
	u64 last_access_time;
	u32 file_attributes;
	};

class FileInfo {
	private:
	u64 base_file_rec_num;
	struct DataRun {
		u64 lcn;
		u64 len;
		DataRun( u64 lcn, u64 len ) : lcn( lcn ), len( len ) { }
		};
	u64 prev_lcn;
	u64 prev_len;
	Array<u8> base_file_rec_buf;
	Array<u8> ext_file_rec_buf;
	u64 load_mft_record( u64 mft_rec_num, Array<u8>& ntfs_file_rec_buf );
	unsigned find_attribute( const Array<u8>& ntfs_file_rec_buf, u32 type, u16 instance = 0 );
	Array<DataRun> decode_data_runs( const Array<u8>& ntfs_file_rec_buf, unsigned attr_off );
	void process_attribute( const Array<u8>& ntfs_file_rec_buf, unsigned attr_off );
	void process_attr_list_entry( const ATTR_LIST_ENTRY* attr_list_entry, Array<u64>& ext_rec_list );
	public:
	// filled by external code
	NtfsVolume* volume;
	std::string file_name;
	unsigned hard_link_cnt;
	bool directory;
	bool reparse;
	// filled by process_file_record()
	unsigned mft_rec_cnt;
	StdInfo std_info;
	std::vector<AttrInfo> attr_list;
	std::vector<FileNameAttr> file_name_list;
	public:
	bool operator==( const FileInfo& file_info ) const {
		return base_file_rec_num == file_info.base_file_rec_num;
		}
	bool operator>( const FileInfo& file_info ) const {
		return base_file_rec_num > file_info.base_file_rec_num;
		}
	public:
	u64 load_base_file_rec( u64 file_ref_num ) {
		return base_file_rec_num = load_mft_record( file_ref_num, base_file_rec_buf );
		}
	u64 file_ref_num( ) const {
		return base_file_rec_num;
		}
	const MFT_RECORD* base_mft_rec( ) const {
		return reinterpret_cast< const MFT_RECORD* >( base_file_rec_buf.data( ) );
		}
	void process_base_file_rec( );
	void process_file( u64 file_ref_num ) {
		load_base_file_rec( file_ref_num );
		process_base_file_rec( );
		}
	void find_full_paths( );
	};
