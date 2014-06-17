#include <vector>
#include <array>
#ifndef _CONTENT_H
#define _CONTENT_H

// content analysis results
struct ContentInfo {
	u64 file_size;
	u64 time; // ms
	u64 comp_size;
	std::array<u8, 4> crc32;
	std::array<u8, 4> md5; // 4 not accurate
	std::array<u8, 4> sha1;// 4 not accurate
	std::array<u8, 4> sha256;// 4 not accurate
	std::array<u8, 4> ed2k;// 4 not accurate
	std::array<u8, 4> crc16;// 4 not accurate
	};

struct CompressionStats {
	u64 data_size; // total file data size
	u64 comp_size; // total compressed data size
	u64 time; // total processing time in ms
	unsigned file_cnt; // number of files processed
	unsigned dir_cnt; // number of dirs processed
	unsigned reparse_cnt; // number of reparse points skipped
	unsigned err_cnt; // number of files/dirs skipped because of errors
	};

bool show_options_dialog( ContentOptions& options, bool single_file );
void process_file_content( const std::string& file_name, const ContentOptions& options, ContentInfo& result );
void show_result_dialog( const std::string& file_name, const ContentOptions& options, const ContentInfo& info );
void compress_files( const std::vector<std::string>& file_list, CompressionStats& result );
void show_result_dialog( const CompressionStats& stats );

#endif /* _CONTENT_H */
