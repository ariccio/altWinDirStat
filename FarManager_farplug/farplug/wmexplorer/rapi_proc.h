#ifndef _RAPI_PROC_H
#define _RAPI_PROC_H

#define COMPOSE_PATH(dir, file) ((dir) + ((((dir).size() != 0) && ((dir).last() != L'\\')) ? L"\\" : L"") + (file))
#define COMPOSE_PATH2(target, dir, file) \
  (target) = (dir); \
  if (((target).size() != 0) && ((target).last() != L'\\')) (target) += L'\\'; \
  (target) += (file);
#define IS_DIR(find_data) (((find_data).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)

extern bool g_rapi2;

struct AutoBuffer {
private:
  enum {
    c_def_size = 512 * 1024,
    c_min_size = 128 * 1024,
    c_max_size = 8 * 1024 * 1024,
  };
  bool auto_size;
  unsigned buffer_size;
  Array<unsigned char> buffer;
  unsigned __int64 start_time;
public:
  AutoBuffer(unsigned size): auto_size(size == -1), buffer_size(size == -1 ? c_def_size : size) {
  }
  unsigned char* ptr() {
    return buffer.buf(buffer_size);
  }
  unsigned size() const {
    return buffer_size;
  }
  void start_measurement();
  void end_measurement(unsigned __int64 data_size);
};

typedef std::vector<FileInfo> FileList;

struct PanelFileInfo: public FileInfo {
  std::string file_dir;
};

typedef std::vector<PanelFileInfo> PanelFileList;

void init_if_needed();
void cleanup();

void get_device_list(std::vector<DeviceInfo>& list);

void create_session(RAPIDEVICEID id, PluginInstance* plugin);
void end_session(PluginInstance* plugin);

void gen_file_list(const std::string& directory, IRAPISession* session, PluginItemList& file_list, const FileListOptions& options, UiLink& ui);

FileList create_file_list(const std::string& src_dir, const std::string& src_file_name, CreateListStats& stats, CreateListOptions& options, UiLink& ui, Log& log, PluginInstance* plugin);
FileList create_file_list(const std::string& src_dir, const std::string& src_file_name, CreateListStats& stats, CreateListOptions& options, UiLink& ui, Log& log, IRAPISession* session, PluginInstance* plugin);

void prepare_target_path(const std::string& path, PluginInstance* plugin);
void prepare_target_path(const std::string& path, IRAPISession* session, PluginInstance* plugin);

void copy_files(bool src_remote, const std::string& src_dir, const FileList& src_file_list, bool dst_remote, const std::string& dst_dir, const std::string& dst_new_name, CopyFilesStats& stats, CopyFilesProgress& progress, CopyFilesOptions& options, UiLink& ui, AutoBuffer& buffer, Log& log, std::vector<FilterInterface>& filters, IRAPISession* session, PluginInstance* plugin);
bool move_remote_file(const std::string& src_path, const std::string& dst_path, IRAPISession* session);

void delete_files(bool dst_remote, const std::string& dst_dir, const FileList& dst_file_list, DeleteFilesStats& stats, DeleteFilesProgress& progress, DeleteFilesOptions& options, UiLink& ui, Log& log, IRAPISession* session, PluginInstance* plugin);

void create_dir(const std::string& dst_dir, const std::string& dst_file_name, IRAPISession* session, PluginInstance* plugin);

void set_file_attr(const std::string& file_path, DWORD attr, IRAPISession* session, PluginInstance* plugin);

void find_real_file_path(FilePath& fp, IRAPISession* session);
bool dir_exists(const std::string& path);
bool dir_exists(const std::string& path, IRAPISession* session);

void refresh_system_info(PluginInstance* plugin);

std::string get_open_command(const std::string& file_dir, const std::string& file_name, IRAPISession* session);
void create_process(const std::string& app_name, const std::string& params, IRAPISession* session);

void panel_items_to_file_list(PluginPanelItem *PanelItem, size_t ItemsNumber, FileList& panel_file_list);
void file_panel_items_to_file_list(const std::string& panel_path, PluginPanelItem *PanelItem, size_t ItemsNumber, PanelFileList& panel_file_list, UiLink& ui, PluginInstance* plugin);

#endif // _RAPI_PROC_H
