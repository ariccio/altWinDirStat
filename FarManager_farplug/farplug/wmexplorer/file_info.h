#pragma once

#define FILE_SIZE(find_data) ((((unsigned __int64) (find_data).nFileSizeHigh) << 32) + (find_data).nFileSizeLow)

struct FileInfo {
  std::string file_name;
  uintptr_t attr;
  FILETIME creation_time;
  FILETIME access_time;
  FILETIME write_time;
  unsigned __int64 size;
  unsigned child_cnt;
  FileInfo() {
  }
  FileInfo(const WIN32_FIND_DATAW& find_data);
  FileInfo(const CE_FIND_DATA& find_data);
  FileInfo(const PluginPanelItem& find_data);
  bool is_dir() const;
};
