#pragma once

/* content analysis options structure */
struct ContentOptions {
	bool compression;
	bool crc32;
	bool md5;
	bool sha1;
	bool sha256;
	bool ed2k;
	bool crc16;
	ContentOptions( );
	};

struct FilePanelMode {
	std::string col_types;
	std::string col_widths;
	std::string status_col_types;
	std::string status_col_widths;
	bool wide;
	int sort_mode;
	int reverse_sort;
	int numeric_sort;
	int sort_dirs_first;
	int custom_sort_mode;
	bool show_streams;
	bool show_main_stream;
	bool use_highlighting;
	bool use_usn_journal;
	bool use_existing_usn_journal;
	bool delete_usn_journal;
	bool delete_own_usn_journal;
	bool use_cache;
	bool default_mft_mode;
	bool backward_mft_scan;
	bool flat_mode_auto_off;
	std::string cache_dir;
	FilePanelMode( );
	};

struct CompressFilesParams {
	unsigned min_file_size; // MB
	unsigned max_compression_ratio; // 75% = comp_size / file_size
	unsigned min_file_age; // days
	bool defragment_after_compression;
	CompressFilesParams( );
	};

/* plugin options */
extern bool g_use_standard_inf_units;
extern ContentOptions g_content_options;
extern FilePanelMode g_file_panel_mode;
extern CompressFilesParams g_compress_files_params;

void load_plugin_options( );
void store_plugin_options( );
