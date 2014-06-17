#ifndef _UI_H
#define _UI_H

std::string far_get_msg(int id);
const wchar_t* far_msg_ptr(int id);
intptr_t far_message(const GUID& guid, const std::string& msg, intptr_t button_cnt = 0, FARMESSAGEFLAGS flags = 0);
intptr_t far_message(const GUID& guid, const wchar_t* const* msg, size_t msg_cnt, intptr_t button_cnt = 0, FARMESSAGEFLAGS flags = 0);
void far_load_colors();
intptr_t far_menu(const GUID& guid, const std::string& title, const std::vector<std::string>& items);
intptr_t far_viewer(const std::string& file_name, const std::string& title);
void draw_text_box(const std::string& title, const std::vector<std::string>& lines, unsigned client_xs);

class UiLink {
private:
  HANDLE h_scr;
  bool lazy;
  unsigned __int64 t_next;
  bool clear_scr;
public:
  UiLink(bool lazy);
  ~UiLink();
  bool update_needed();
  void force_update();
};

std::vector<std::string> get_size_suffixes();
std::vector<std::string> get_speed_suffixes();

enum LogOpType {
  lotScanDir,
  lotCreateDir,
  lotCopyFile,
  lotDeleteFile,
  lotDeleteDir,
};

struct LogRecord {
  LogOpType type;
  std::string object;
  std::string message;
};

struct Log: public std::vector<LogRecord> {
  void add(LogOpType type, const std::string& object, const std::string& message) {
    LogRecord log_rec;
    log_rec.type = type;
    log_rec.object = object;
    log_rec.message = message;
    std::vector<LogRecord>::add(log_rec);
  }
};

unsigned get_msg_width();

void draw_progress_msg(const std::string& message);

struct FileListOptions {
  bool hide_rom_files;
};

struct CreateListStats {
  unsigned __int64 size;
  unsigned files;
  unsigned dirs;
  unsigned errors;
  CreateListStats(): size(0), files(0), dirs(0), errors(0) {
  }
};

struct CreateListOptions {
  bool ignore_errors;
  bool show_error;
};

void draw_create_list_progress(const CreateListStats& stats);

struct CopyFilesOptions {
  std::string dst_dir;
  bool ignore_errors;
  OverwriteOption overwrite;
  ShowStatsOption show_stats;
  bool move_files;
  bool show_error;
  bool show_dialog;
  bool copy_shared;
  bool use_file_filters;
  bool use_tmp_files;
};

bool show_copy_files_dlg(CopyFilesOptions& options, bool f_put);

struct CopyFilesStats {
  unsigned files;
  unsigned dirs;
  unsigned overwritten;
  unsigned skipped;
  unsigned errors;
  CopyFilesStats(): files(0), dirs(0), overwritten(0), skipped(0), errors(0) {
  }
};

enum OverwriteAction {
  oaYes,
  oaYesAll,
  oaNo,
  oaNoAll,
  oaCancel,
};

OverwriteAction show_overwrite_dlg(const std::string& file_path, const FileInfo& src_file_info, const FileInfo& dst_file_info);

void show_copy_files_results_dlg(const CopyFilesStats& stats, const Log& log);

struct CopyFilesProgress {
  std::string src_path;
  std::string dst_path;
  unsigned __int64 file_size;
  unsigned __int64 copied_file_size;
  unsigned __int64 total_size;
  unsigned __int64 processed_total_size;
  unsigned __int64 copied_total_size;
  unsigned __int64 start_time;
  unsigned __int64 file_start_time;
};

void draw_copy_files_progress(const CopyFilesProgress& progress, const CopyFilesStats& stats, bool move);
void draw_move_remote_files_progress(const CopyFilesProgress& progress, const CopyFilesStats& stats);

struct DeleteFilesOptions {
  bool ignore_errors;
  ShowStatsOption show_stats;
  bool show_error;
  bool show_dialog;
};

bool show_delete_files_dlg(DeleteFilesOptions& options);

struct DeleteFilesProgress {
  std::string curr_path;
  unsigned objects;
  unsigned total_objects;
  unsigned __int64 start_time;
};

struct DeleteFilesStats {
  unsigned files;
  unsigned dirs;
  unsigned errors;
  DeleteFilesStats(): files(0), dirs(0), errors(0) {
  }
};

void draw_delete_files_progress(const DeleteFilesProgress& progress, const DeleteFilesStats& stats);
void show_delete_files_results_dlg(const DeleteFilesStats& stats, const Log& log);

struct CreateDirOptions {
  std::string file_name;
};

bool show_create_dir_dlg(CreateDirOptions& options);

bool show_plugin_options_dlg(PluginOptions& options);

struct FileAttrOptions {
  bool single_file;
  DWORD attr_and;
  DWORD attr_or;
};

bool show_file_attr_dlg(FileAttrOptions& options);

struct RunOptions {
  std::string cmd_line;
};

bool show_run_dlg(RunOptions& options);

struct FilterSelection {
  unsigned src_idx;
  unsigned dst_idx;
};

bool show_filters_dlg(const std::vector<FileFilters>& filter_list, Array<FilterSelection>& filter_selection);

#endif // _UI_H
