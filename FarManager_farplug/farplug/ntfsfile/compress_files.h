#pragma once
#include <vector>
#include <string>

void plugin_compress_files( const std::vector<std::string>& file_list, const CompressFilesParams& params, Log& log );
bool show_compress_files_dialog( CompressFilesParams& params );
