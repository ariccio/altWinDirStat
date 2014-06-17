#include "util.h"
#include "file_info.h"

FileInfo::FileInfo(const WIN32_FIND_DATAW& find_data) {
  file_name = find_data.cFileName;
  attr = find_data.dwFileAttributes;
  creation_time = find_data.ftCreationTime;
  access_time = find_data.ftLastAccessTime;
  write_time = find_data.ftLastWriteTime;
  size = FILE_SIZE(find_data);
  child_cnt = 0;
}

FileInfo::FileInfo(const CE_FIND_DATA& find_data) {
  file_name = find_data.cFileName;
  attr = find_data.dwFileAttributes;
  creation_time = find_data.ftCreationTime;
  access_time = find_data.ftLastAccessTime;
  write_time = find_data.ftLastWriteTime;
  size = FILE_SIZE(find_data);
  child_cnt = 0;
}

FileInfo::FileInfo(const PluginPanelItem& find_data) {
  file_name = find_data.FileName;
  attr = find_data.FileAttributes;
  creation_time = find_data.CreationTime;
  access_time = find_data.LastAccessTime;
  write_time = find_data.LastWriteTime;
  size = find_data.FileSize;
  child_cnt = 0;
}

bool FileInfo::is_dir() const {
  return (attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}
