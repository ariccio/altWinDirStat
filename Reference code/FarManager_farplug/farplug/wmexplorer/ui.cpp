#include "msg.h"
#include "guids.h"
#define _ERROR_WINDOWS
#include "error.h"
#include "util.h"
#include "dlgapi.h"
#include "options.h"
#include "file_filters.h"
#include "file_info.h"
#include "ui.h"

extern struct PluginStartupInfo g_far;
extern struct FarStandardFunctions g_fsf;
extern Array<FarColor> g_colors;
extern unsigned __int64 g_time_freq;

std::string far_get_msg(int id) {
  return g_far.GetMsg(&c_plugin_guid, id);
}

const wchar_t* far_msg_ptr(int id) {
  return g_far.GetMsg(&c_plugin_guid, id);
}

intptr_t far_message(const GUID& guid, const std::string& msg, intptr_t button_cnt, FARMESSAGEFLAGS flags) {
  return g_far.Message(&c_plugin_guid, &guid, flags | FMSG_ALLINONE, nullptr, (const wchar_t* const*) msg.data(), 0, button_cnt);
}

intptr_t far_message(const GUID& guid, const wchar_t* const* msg, size_t msg_cnt, intptr_t button_cnt, FARMESSAGEFLAGS flags) {
  return g_far.Message(&c_plugin_guid, &guid, flags, nullptr, msg, msg_cnt, button_cnt);
}

void far_load_colors() {
  unsigned colors_size = (unsigned) g_far.AdvControl(&c_plugin_guid, ACTL_GETARRAYCOLOR, 0, nullptr);
  g_far.AdvControl(&c_plugin_guid, ACTL_GETARRAYCOLOR, colors_size, g_colors.buf(colors_size));
  g_colors.set_size(colors_size);
}

intptr_t far_menu(const GUID& guid, const std::string& title, const std::vector<std::string>& items) {
  Array<FarMenuItem> menu_items;
  FarMenuItem mi;
  for (unsigned i = 0; i < items.size(); i++) {
    memset(&mi, 0, sizeof(mi));
    mi.Text = items[i].data();
    menu_items += mi;
  }
  return g_far.Menu(&c_plugin_guid, &guid, -1, -1, 0, FMENU_WRAPMODE, title.data(), NULL, NULL, NULL, NULL, menu_items.data(), menu_items.size());
}

intptr_t far_viewer(const std::string& file_name, const std::string& title) {
  return g_far.Viewer(file_name.data(), title.data(), 0, 0, -1, -1, VF_DISABLEHISTORY | VF_ENABLE_F6 | VF_NONMODAL, CP_UNICODE);
}

std::string far_get_text(HANDLE h_dlg, unsigned ctrl_id) {
  FarDialogItemData item = { sizeof(FarDialogItemData) };
  item.PtrLength = g_far.SendDlgMessage(h_dlg, DM_GETTEXT, ctrl_id, nullptr);
  std::string text;
  item.PtrData = text.buf(item.PtrLength + 1);
  g_far.SendDlgMessage(h_dlg, DM_GETTEXT, ctrl_id, &item);
  text.set_size(item.PtrLength);
  return text;
}

void draw_text_box(const std::string& title, const std::vector<std::string>& lines, unsigned client_xs) {
  unsigned size = 0;
  for (unsigned i = 0; i < lines.size(); i++) {
    size += min(lines[i].size(), client_xs) + 1;
  }
  std::string text;
  text.extend(title.size() + 1 + size);
  text += title;
  text += L'\n';
  for (unsigned i = 0; i < lines.size(); i++) {
    if (lines[i].size() <= client_xs)
      text += lines[i];
    else
      text += lines[i].left(client_xs);
    text += L'\n';
  }
  far_message(c_progress_dialog_guid, text, 0, FMSG_LEFTALIGN);
}

UiLink::UiLink(bool lazy): lazy(lazy), h_scr(NULL), clear_scr(false) {
  unsigned __int64 t_curr;
  QueryPerformanceCounter((PLARGE_INTEGER) &t_curr);
  if (lazy) t_next = t_curr + g_time_freq;
  else t_next = t_curr;
  far_load_colors();
}

UiLink::~UiLink() {
  if (h_scr != NULL) {
    g_far.RestoreScreen(h_scr);
  }
}

bool UiLink::update_needed() {
  unsigned __int64 t_curr;
  QueryPerformanceCounter((PLARGE_INTEGER) &t_curr);
  if (t_curr >= t_next) {
    if (clear_scr) {
      clear_scr = false;
      if (h_scr != NULL) {
        g_far.RestoreScreen(h_scr);
        h_scr = g_far.SaveScreen(0, 0, -1, -1);
      }
    }
    if (h_scr == NULL) h_scr = g_far.SaveScreen(0, 0, -1, -1);
    HANDLE h_con = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD rec;
    DWORD read_cnt;
    bool esc_key = false;
    while (true) {
      PeekConsoleInput(h_con, &rec, 1, &read_cnt);
      if (read_cnt == 0) break;
      ReadConsoleInput(h_con, &rec, 1, &read_cnt);
      if ((rec.EventType == KEY_EVENT) && (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) && rec.Event.KeyEvent.bKeyDown && ((rec.Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED | RIGHT_CTRL_PRESSED | SHIFT_PRESSED)) == 0)) esc_key = true;
    }
    if (esc_key) {
      
      if (get_app_option(FSSF_CONFIRMATIONS, c_esc_confirmation_option, true)) {
        if (far_message(c_interrupt_dialog_guid, far_get_msg(MSG_PLUGIN_NAME).add('\n').add(far_get_msg(MSG_PROGRESS_INTERRUPT)), 0, FMSG_MB_YESNO) == 0) BREAK;
      }
      else BREAK;
    }
    t_next = t_curr + g_time_freq / 2;
    return true;
  }
  return false;
}

void UiLink::force_update() {
  if (!lazy) QueryPerformanceCounter((PLARGE_INTEGER) &t_next);
  clear_scr = true;
}

std::vector<std::string> get_size_suffixes() {
  std::vector<std::string> suffixes;
  suffixes += far_get_msg(MSG_SUFFIX_SIZE_B);
  suffixes += far_get_msg(MSG_SUFFIX_SIZE_KB);
  suffixes += far_get_msg(MSG_SUFFIX_SIZE_MB);
  suffixes += far_get_msg(MSG_SUFFIX_SIZE_GB);
  suffixes += far_get_msg(MSG_SUFFIX_SIZE_TB);
  return suffixes;
}

std::vector<std::string> get_speed_suffixes() {
  std::vector<std::string> suffixes;
  suffixes += far_get_msg(MSG_SUFFIX_SPEED_B);
  suffixes += far_get_msg(MSG_SUFFIX_SPEED_KB);
  suffixes += far_get_msg(MSG_SUFFIX_SPEED_MB);
  suffixes += far_get_msg(MSG_SUFFIX_SPEED_GB);
  suffixes += far_get_msg(MSG_SUFFIX_SPEED_TB);
  return suffixes;
}

void show_log(const Log& log) {
  try {
    // get system directory for temporary files
    std::string temp_path;
    DWORD temp_path_size = MAX_PATH;
    DWORD len = GetTempPathW(temp_path_size, temp_path.buf(temp_path_size));
    if (len > temp_path_size) {
      temp_path_size = len;
      len = GetTempPathW(temp_path_size, temp_path.buf(temp_path_size));
    }
    CHECK_API(len != 0);
    temp_path.set_size(len);
    // create file name for temporary file
    std::string temp_file_path;
    CHECK_API(GetTempFileNameW(temp_path.data(), L"log", 0, temp_file_path.buf(MAX_PATH)) != 0);
    temp_file_path.set_size();
    try {
      // open file for writing
      HANDLE h_file = CreateFileW(temp_file_path.data(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      CHECK_API(h_file != INVALID_HANDLE_VALUE);
      try {
        // write unicode signature
        const wchar_t sig = 0xFEFF;
        DWORD size_written;
        CHECK_API(WriteFile(h_file, &sig, sizeof(sig), &size_written, NULL) != 0);
        // write log data
        std::string line;
        for (unsigned i = 0; i < log.size(); i++) {
          std::string op;
          if (log[i].type == lotScanDir) op = far_get_msg(MSG_LOG_SCAN_DIR);
          else if (log[i].type == lotCreateDir) op = far_get_msg(MSG_LOG_CREATE_DIR);
          else if (log[i].type == lotCopyFile) op = far_get_msg(MSG_LOG_COPY_FILE);
          else if (log[i].type == lotDeleteFile) op = far_get_msg(MSG_LOG_DELETE_FILE);
          else if (log[i].type == lotDeleteDir) op = far_get_msg(MSG_LOG_DELETE_DIR);
          line.copy_fmt(L"%S: '%S' (%S)\n", &op, &log[i].object, &log[i].message);
          CHECK_API(WriteFile(h_file, line.data(), line.size() * sizeof(wchar_t), &size_written, NULL) != 0);
        }
      }
      finally (VERIFY(CloseHandle(h_file) != 0));
      // open temporary file in Far viewer
      far_viewer(temp_file_path, far_get_msg(MSG_LOG_TITLE));
    }
    finally (VERIFY(DeleteFileW(temp_file_path.data()) != 0));
  }
  catch (Error& e) {
    LOG_ERR(e);
  }
}

// optimum client area width of message dialogs
unsigned get_msg_width() {
  HANDLE con = GetStdHandle(STD_OUTPUT_HANDLE);
  if (con != INVALID_HANDLE_VALUE) {
    CONSOLE_SCREEN_BUFFER_INFO con_info;
    if (GetConsoleScreenBufferInfo(con, &con_info) != 0) {
      unsigned con_width = con_info.srWindow.Right - con_info.srWindow.Left + 1;
      if (con_width >= 80) return con_width - 20;
    }
  }
  return 60;
}

void draw_progress_msg(const std::string& message) {
  const unsigned c_client_xs = message.size();
  std::vector<std::string> lines;
  lines += center(message, c_client_xs);
  draw_text_box(far_get_msg(MSG_PLUGIN_NAME), lines, c_client_xs);
  SetConsoleTitleW(message.data());
  far_set_progress_state(TBPF_INDETERMINATE);
}

void draw_create_list_progress(const CreateListStats& stats) {
  const unsigned c_client_xs = 50;
  std::vector<std::string> lines;
  lines += center(std::string::format(far_get_msg(MSG_CREATE_LIST_PROGRESS_OBJECTS).data(), stats.files, stats.dirs, stats.errors), c_client_xs);
  lines += center(std::string::format(far_get_msg(MSG_CREATE_LIST_PROGRESS_SIZE).data(), &format_data_size(stats.size, get_size_suffixes())), c_client_xs);
  draw_text_box(far_get_msg(MSG_CREATE_LIST_PROGRESS_TITLE), lines, c_client_xs);
  SetConsoleTitleW(far_get_msg(MSG_CREATE_LIST_PROGRESS_TITLE).data());
  far_set_progress_state(TBPF_INDETERMINATE);
}

struct CopyFilesDlgData {
  int dst_dir_ctrl_id;
  int ignore_errors_ctrl_id;
  int oo_skip_ctrl_id;
  int oo_overwrite_ctrl_id;
  int oo_ask_ctrl_id;
  int move_files_ctrl_id;
  int show_stats_always_ctrl_id;
  int show_stats_never_ctrl_id;
  int show_stats_if_error_ctrl_id;
  int copy_shared_ctrl_id;
  int use_file_filters_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  CopyFilesOptions* options;
  bool f_put;
  CopyFilesDlgData(CopyFilesOptions* options, bool f_put): options(options), f_put(f_put) {
  }
};

intptr_t WINAPI copy_files_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  CopyFilesDlgData* dlg_data = (CopyFilesDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    CopyFilesOptions* options = dlg_data->options;

    options->dst_dir = far_get_text(hDlg, dlg_data->dst_dir_ctrl_id);
    options->dst_dir.strip();
    unquote(options->dst_dir);

    options->ignore_errors = DlgItem_GetCheck(g_far, hDlg, dlg_data->ignore_errors_ctrl_id) == BSTATE_CHECKED;

    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->oo_skip_ctrl_id) == BSTATE_CHECKED) options->overwrite = ooSkip;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->oo_overwrite_ctrl_id) == BSTATE_CHECKED) options->overwrite = ooOverwrite;
    else options->overwrite = ooAsk;

    options->move_files = DlgItem_GetCheck(g_far, hDlg, dlg_data->move_files_ctrl_id) == BSTATE_CHECKED;

    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->show_stats_always_ctrl_id) == BSTATE_CHECKED) options->show_stats = ssoAlways;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->show_stats_never_ctrl_id) == BSTATE_CHECKED) options->show_stats = ssoNever;
    else options->show_stats = ssoIfError;

    options->copy_shared = DlgItem_GetCheck(g_far, hDlg, dlg_data->copy_shared_ctrl_id) == BSTATE_CHECKED;
    if (dlg_data->f_put) options->use_file_filters = DlgItem_GetCheck(g_far, hDlg, dlg_data->use_file_filters_ctrl_id) == BSTATE_CHECKED;
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_copy_files_dlg(CopyFilesOptions& options, bool f_put) {
  CopyFilesDlgData dlg_data(&options, f_put);
  const unsigned c_client_xs = 60;
  FarDialog dlg(c_copy_files_dialog_guid, far_get_msg(MSG_COPY_FILES_DLG_TITLE), c_client_xs);
  // source file list
  dlg.label(far_get_msg(MSG_COPY_FILES_DLG_PATH));
  dlg.new_line();
  // destination directory edit box
  dlg_data.dst_dir_ctrl_id = dlg.var_edit_box(options.dst_dir, c_client_xs);
  dlg.new_line();
  // separator
  dlg.separator();
  dlg.new_line();

  // ignore errors option
  dlg_data.ignore_errors_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_IGNORE_ERRORS), options.ignore_errors);
  dlg.new_line();

  // overwrite files option
  dlg.label(far_get_msg(MSG_COPY_FILES_DLG_OO));
  dlg.new_line();
  dlg.spacer(2);
  dlg_data.oo_skip_ctrl_id = dlg.radio_button(far_get_msg(MSG_COPY_FILES_DLG_OO_SKIP), options.overwrite == ooSkip);
  dlg.spacer(2);
  dlg_data.oo_overwrite_ctrl_id = dlg.radio_button(far_get_msg(MSG_COPY_FILES_DLG_OO_OVERWRITE), options.overwrite == ooOverwrite);
  dlg.spacer(2);
  dlg_data.oo_ask_ctrl_id = dlg.radio_button(far_get_msg(MSG_COPY_FILES_DLG_OO_ASK), options.overwrite == ooAsk);
  dlg.new_line();

  // move files option
  dlg_data.move_files_ctrl_id = dlg.check_box(far_get_msg(MSG_COPY_FILES_DLG_MOVE_FILES), options.move_files);
  dlg.new_line();

  // show stats option
  dlg.label(far_get_msg(MSG_OPTIONS_SHOW_STATS));
  dlg.new_line();
  dlg.spacer(2);
  dlg_data.show_stats_always_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_ALWAYS), options.show_stats == ssoAlways, DIF_GROUP);
  dlg.spacer(2);
  dlg_data.show_stats_never_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_NEVER), options.show_stats == ssoNever);
  dlg.spacer(2);
  dlg_data.show_stats_if_error_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_IF_ERROR), options.show_stats == ssoIfError);
  dlg.new_line();

  // copy files open for writing option
  dlg_data.copy_shared_ctrl_id = dlg.check_box(far_get_msg(MSG_COPY_FILES_DLG_COPY_SHARED), options.copy_shared);
  dlg.new_line();
  if (f_put) {
    // use ActiveSync file filters when sending files to device
    dlg_data.use_file_filters_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_USE_FILE_FILTERS), options.use_file_filters);
    dlg.new_line();
  }

  // separator
  dlg.separator();
  dlg.new_line();
  // OK & Cancel buttons
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(copy_files_dlg_proc, &dlg_data, L"copy.files.dlg");

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct OverwriteDlgData {
  int yes_ctrl_id;
  int yes_all_ctrl_id;
  int no_ctrl_id;
  int no_all_ctrl_id;
  int cancel_ctrl_id;
};

OverwriteAction show_overwrite_dlg(const std::string& file_path, const FileInfo& src_file_info, const FileInfo& dst_file_info) {
  OverwriteDlgData dlg_data;
  const unsigned c_client_xs = 60;
  FarDialog dlg(c_overwrite_dialog_guid, far_get_msg(src_file_info.is_dir() ? MSG_OVERWRITE_DLG_DIRECTORY_TITLE : MSG_OVERWRITE_DLG_TITLE), c_client_xs);
  // file info
  dlg.label(fit_str(file_path, c_client_xs));
  dlg.new_line();
  // question
  dlg.label(far_get_msg(src_file_info.is_dir() ? MSG_OVERWRITE_DLG_DIRECTORY_QUESTION : MSG_OVERWRITE_DLG_QUESTION));
  dlg.new_line();
  // separator
  dlg.separator();
  dlg.new_line();
  // source file info
  dlg.label(far_get_msg(MSG_OVERWRITE_DLG_SOURCE));
  dlg.pad(15);
  if (!src_file_info.is_dir()) {
    dlg.label(format_data_size(src_file_info.size, get_size_suffixes()));
    dlg.pad(25);
  }
  dlg.label(format_file_time(src_file_info.write_time));
  if (CompareFileTime(&src_file_info.write_time, &dst_file_info.write_time) > 0) {
    dlg.spacer(1);
    dlg.label(far_get_msg(MSG_OVERWRITE_DLG_NEWER));
  }
  dlg.new_line();
  // destination file info
  dlg.label(far_get_msg(MSG_OVERWRITE_DLG_DESTINATION));
  dlg.pad(15);
  if (!dst_file_info.is_dir()) {
    dlg.label(format_data_size(dst_file_info.size, get_size_suffixes()));
    dlg.pad(25);
  }
  dlg.label(format_file_time(dst_file_info.write_time));
  if (CompareFileTime(&src_file_info.write_time, &dst_file_info.write_time) < 0) {
    dlg.spacer(1);
    dlg.label(far_get_msg(MSG_OVERWRITE_DLG_NEWER));
  }
  dlg.new_line();
  // separator
  dlg.separator();
  dlg.new_line();
  // buttons
  dlg_data.yes_ctrl_id = dlg.def_button(far_get_msg(MSG_OVERWRITE_DLG_YES), DIF_CENTERGROUP);
  dlg_data.yes_all_ctrl_id = dlg.button(far_get_msg(MSG_OVERWRITE_DLG_YES_ALL), DIF_CENTERGROUP);
  dlg_data.no_ctrl_id = dlg.button(far_get_msg(MSG_OVERWRITE_DLG_NO), DIF_CENTERGROUP);
  dlg_data.no_all_ctrl_id = dlg.button(far_get_msg(MSG_OVERWRITE_DLG_NO_ALL), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show();

  if (item == dlg_data.yes_ctrl_id) return oaYes;
  else if (item == dlg_data.yes_all_ctrl_id) return oaYesAll;
  else if (item == dlg_data.no_ctrl_id) return oaNo;
  else if (item == dlg_data.no_all_ctrl_id) return oaNoAll;
  else return oaCancel;
}

void show_copy_files_results_dlg(const CopyFilesStats& stats, const Log& log) {
  std::string msg;
  msg.add(far_get_msg(MSG_COPY_FILES_STATS_TITLE)).add('\n');
  if (stats.errors != 0) msg.add_fmt(far_get_msg(MSG_COPY_FILES_STATS_ERRORS).data(), stats.errors).add('\n');
  msg.add_fmt(far_get_msg(MSG_COPY_FILES_STATS_FILES).data(), stats.files, stats.overwritten, stats.skipped).add('\n');
  msg.add_fmt(far_get_msg(MSG_COPY_FILES_STATS_DIRS).data(), stats.dirs).add('\n');
  msg.add(far_get_msg(MSG_BUTTON_OK)).add('\n');
  if (log.size() != 0) msg.add(far_get_msg(MSG_LOG_SHOW)).add('\n');
  if (far_message(c_copy_files_stats_dialog_guid, msg, log.size() != 0 ? 2 : 1, FMSG_LEFTALIGN) == 1) {
    show_log(log);
  }
}

void draw_copy_files_progress(const CopyFilesProgress& progress, const CopyFilesStats& stats, bool move) {
  const unsigned c_client_xs = 60;
  std::vector<std::string> lines;
  unsigned __int64 progress_total_size = progress.total_size;
  if (progress_total_size < progress.processed_total_size) progress_total_size = progress.processed_total_size;
  if (progress_total_size < progress.copied_total_size) progress_total_size = progress.copied_total_size;
  unsigned __int64 progress_file_size = progress.file_size;
  if (progress_file_size < progress.copied_file_size) progress_file_size = progress.copied_file_size;
  unsigned __int64 curr_time;
  QueryPerformanceCounter((PLARGE_INTEGER) &curr_time);

  // paths
  std::string src_label(far_get_msg(MSG_COPY_FILES_PROGRESS_SRC));
  lines += src_label + ' ' + fit_str(progress.src_path, c_client_xs - src_label.size() - 1);
  std::string dst_label(far_get_msg(MSG_COPY_FILES_PROGRESS_DST));
  lines += dst_label + ' ' + fit_str(progress.dst_path, c_client_xs - dst_label.size() - 1);
  // file data size
  unsigned __int64 file_time = (curr_time - progress.file_start_time) * 1000 / g_time_freq; // ms
  unsigned __int64 file_speed;
  if (file_time == 0) file_speed = 0;
  else file_speed = progress.copied_file_size * 1000 / file_time;
  unsigned file_percent;
  if (progress_file_size == 0) file_percent = 0;
  else file_percent = (unsigned) (progress.copied_file_size * 100 / progress_file_size);
  lines += std::string::format(far_get_msg(MSG_COPY_FILES_PROGRESS_FILE_SIZE).data(), &format_data_size(progress.copied_file_size, get_size_suffixes()), &format_data_size(progress_file_size, get_size_suffixes()), file_percent, &format_data_size(file_speed, get_speed_suffixes()));
  // file progress bar
  unsigned len1;
  if (progress_file_size == 0) len1 = c_client_xs;
  else len1 = (unsigned) (progress.copied_file_size * c_client_xs / progress_file_size);
  unsigned len2 = c_client_xs - len1;
  lines += std::string::format(L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white);
  // separator
  lines += L"\x1";

  // total data size
  unsigned __int64 time = (curr_time - progress.start_time) * 1000 / g_time_freq; // ms
  unsigned __int64 speed;
  if (time == 0) speed = 0;
  else speed = progress.copied_total_size * 1000 / time;
  unsigned percent;
  if (progress_total_size == 0) percent = 0;
  else percent = (unsigned) (progress.processed_total_size * 100 / progress_total_size);
  lines += std::string::format(far_get_msg(MSG_COPY_FILES_PROGRESS_SIZE).data(), &format_data_size(progress.processed_total_size, get_size_suffixes()), &format_data_size(progress_total_size, get_size_suffixes()), percent, &format_data_size(speed, get_speed_suffixes()));
  SetConsoleTitleW(std::string::format(L"{%u%%} %S", percent, &far_get_msg(move ? MSG_MOVE_FILES_PROGRESS_TITLE : MSG_COPY_FILES_PROGRESS_TITLE)).data());
  far_set_progress_state(TBPF_NORMAL);
  far_set_progress_value(percent, 100);

  // times
  unsigned __int64 total_time;
  if (progress.processed_total_size == 0) total_time = time;
  else total_time = time * progress_total_size / progress.processed_total_size;
  lines += std::string::format(far_get_msg(MSG_COPY_FILES_PROGRESS_TIME).data(), &format_time(time), &format_time(total_time - time), &format_time(total_time));

  // total progress bar
  if (progress_total_size == 0) len1 = c_client_xs;
  else len1 = (unsigned) (progress.processed_total_size * c_client_xs / progress_total_size);
  len2 = c_client_xs - len1;
  lines += std::string::format(L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white);
  // separator
  lines += L"\x1";

  // stats
  lines += std::string::format(far_get_msg(MSG_COPY_FILES_PROGRESS_STATS).data(), stats.files, stats.overwritten, stats.skipped, stats.dirs, stats.errors);

  draw_text_box(far_get_msg(move ? MSG_MOVE_FILES_PROGRESS_TITLE : MSG_COPY_FILES_PROGRESS_TITLE), lines, c_client_xs);
}

void draw_move_remote_files_progress(const CopyFilesProgress& progress, const CopyFilesStats& stats) {
  const unsigned c_client_xs = 60;
  std::vector<std::string> lines;
  unsigned __int64 curr_time;
  QueryPerformanceCounter((PLARGE_INTEGER) &curr_time);

  // paths
  std::string src_label(far_get_msg(MSG_COPY_FILES_PROGRESS_SRC));
  lines += src_label + ' ' + fit_str(progress.src_path, c_client_xs - src_label.size() - 1);
  std::string dst_label(far_get_msg(MSG_COPY_FILES_PROGRESS_DST));
  lines += dst_label + ' ' + fit_str(progress.dst_path, c_client_xs - dst_label.size() - 1);
  // separator
  lines += L"\x1";

  // times
  unsigned __int64 time = (curr_time - progress.start_time) * 1000 / g_time_freq; // ms
  lines += std::string::format(far_get_msg(MSG_MOVE_REMOTE_FILES_PROGRESS_TIME).data(), &format_time(time));
  // separator
  lines += L"\x1";

  // stats
  lines += std::string::format(far_get_msg(MSG_MOVE_REMOTE_FILES_PROGRESS_STATS).data(), stats.files, stats.dirs);

  draw_text_box(far_get_msg(MSG_MOVE_REMOTE_FILES_PROGRESS_TITLE), lines, c_client_xs);
  SetConsoleTitleW(far_get_msg(MSG_MOVE_REMOTE_FILES_PROGRESS_TITLE).data());
  far_set_progress_state(TBPF_INDETERMINATE);
}

struct DeleteFilesDlgData {
  int ignore_errors_ctrl_id;
  int show_stats_always_ctrl_id;
  int show_stats_never_ctrl_id;
  int show_stats_if_error_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  DeleteFilesOptions* options;
  DeleteFilesDlgData(DeleteFilesOptions* options): options(options) {
  }
};

intptr_t WINAPI delete_files_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  DeleteFilesDlgData* dlg_data = (DeleteFilesDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    DeleteFilesOptions* options = dlg_data->options;

    options->ignore_errors = DlgItem_GetCheck(g_far, hDlg, dlg_data->ignore_errors_ctrl_id) == BSTATE_CHECKED;

    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->show_stats_always_ctrl_id) == BSTATE_CHECKED) options->show_stats = ssoAlways;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->show_stats_never_ctrl_id) == BSTATE_CHECKED) options->show_stats = ssoNever;
    else options->show_stats = ssoIfError;
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_delete_files_dlg(DeleteFilesOptions& options) {
  DeleteFilesDlgData dlg_data(&options);
  FarDialog dlg(c_delete_files_dialog_guid, far_get_msg(MSG_DELETE_FILES_DLG_TITLE), 60);
  dlg.label(far_get_msg(MSG_DELETE_FILES_DLG_LABEL));
  dlg.new_line();

  // ignore errors option
  dlg_data.ignore_errors_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_IGNORE_ERRORS), options.ignore_errors);
  dlg.new_line();

  // show stats option
  dlg.label(far_get_msg(MSG_OPTIONS_SHOW_STATS));
  dlg.new_line();
  dlg.spacer(2);
  dlg_data.show_stats_always_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_ALWAYS), options.show_stats == ssoAlways, DIF_GROUP);
  dlg.spacer(2);
  dlg_data.show_stats_never_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_NEVER), options.show_stats == ssoNever);
  dlg.spacer(2);
  dlg_data.show_stats_if_error_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_IF_ERROR), options.show_stats == ssoIfError);
  dlg.new_line();

  // separator
  dlg.separator();
  dlg.new_line();

  // OK & Cancel buttons
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(delete_files_dlg_proc, &dlg_data, L"delete.files.dlg");

  // if user cancelled dialog
  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

void draw_delete_files_progress(const DeleteFilesProgress& progress, const DeleteFilesStats& stats) {
  assert(progress.objects <= progress.total_objects);
  const unsigned c_client_xs = 60;
  std::vector<std::string> lines;
  unsigned __int64 curr_time;
  QueryPerformanceCounter((PLARGE_INTEGER) &curr_time);

  // current path
  lines += fit_str(progress.curr_path, c_client_xs);
  // separator
  lines += L"\x1";
  // file progress bar
  unsigned len1;
  if (progress.total_objects == 0) len1 = c_client_xs;
  else len1 = (unsigned) (progress.objects * c_client_xs / progress.total_objects);
  unsigned len2 = c_client_xs - len1;
  lines += std::string::format(L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white);
  // percent
  unsigned percent;
  if (progress.total_objects == 0) percent = 0;
  else percent = progress.objects * 100 / progress.total_objects;
  SetConsoleTitleW(std::string::format(L"{%u%%} %S", percent, &far_get_msg(MSG_DELETE_FILES_PROGRESS_TITLE)).data());
  far_set_progress_state(TBPF_NORMAL);
  far_set_progress_value(percent, 100);
  // separator
  lines += L"\x1";
  // times
  unsigned __int64 time = (curr_time - progress.start_time) * 1000 / g_time_freq; // ms
  unsigned __int64 total_time;
  if (progress.objects == 0) total_time = time;
  else total_time = time * progress.total_objects / progress.objects;
  lines += std::string::format(far_get_msg(MSG_DELETE_FILES_PROGRESS_TIME).data(), &format_time(time), &format_time(total_time - time), &format_time(total_time));
  // separator
  lines += L"\x1";
  // stats
  lines += std::string::format(far_get_msg(MSG_DELETE_FILES_PROGRESS_STATS).data(), stats.files, stats.dirs, stats.errors);

  draw_text_box(far_get_msg(MSG_DELETE_FILES_PROGRESS_TITLE), lines, c_client_xs);
}

void show_delete_files_results_dlg(const DeleteFilesStats& stats, const Log& log) {
  std::string msg;
  msg.add(far_get_msg(MSG_DELETE_FILES_STATS_TITLE)).add('\n');
  if (stats.errors != 0) msg.add_fmt(far_get_msg(MSG_DELETE_FILES_STATS_ERRORS).data(), stats.errors).add('\n');
  msg.add_fmt(far_get_msg(MSG_DELETE_FILES_STATS_FILES).data(), stats.files).add('\n');
  msg.add_fmt(far_get_msg(MSG_DELETE_FILES_STATS_DIRS).data(), stats.dirs).add('\n');
  msg.add(far_get_msg(MSG_BUTTON_OK)).add('\n');
  if (log.size() != 0) msg.add(far_get_msg(MSG_LOG_SHOW)).add('\n');
  if (far_message(c_delete_files_stats_dialog_guid, msg, log.size() != 0 ? 2 : 1, FMSG_LEFTALIGN) == 1) {
    show_log(log);
  }
}

struct CreateDirDlgData {
  int file_name_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  CreateDirOptions* options;
  CreateDirDlgData(CreateDirOptions* options): options(options) {
  }
};

intptr_t WINAPI create_dir_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  CreateDirDlgData* dlg_data = (CreateDirDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    CreateDirOptions* options = dlg_data->options;

    options->file_name = far_get_text(hDlg, dlg_data->file_name_ctrl_id);
    options->file_name.strip();
    unquote(options->file_name);
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_create_dir_dlg(CreateDirOptions& options) {
  CreateDirDlgData dlg_data(&options);
  const unsigned c_client_xs = 60;
  FarDialog dlg(c_create_dir_dialog_guid, far_get_msg(MSG_CREATE_DIR_DLG_TITLE), c_client_xs);
  dlg.label(far_get_msg(MSG_CREATE_DIR_DLG_PATH));
  dlg.new_line();
  // destination directory edit box
  dlg_data.file_name_ctrl_id = dlg.var_edit_box(options.file_name, c_client_xs);
  dlg.new_line();
  dlg.separator();
  dlg.new_line();
  // OK & Cancel buttons
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(create_dir_dlg_proc, &dlg_data);

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct DefOptionValuesDlgData {
  int ignore_errors_ctrl_id;
  int oo_skip_ctrl_id;
  int oo_overwrite_ctrl_id;
  int oo_ask_ctrl_id;
  int show_stats_always_ctrl_id;
  int show_stats_never_ctrl_id;
  int show_stats_if_error_ctrl_id;
  int use_file_filters_ctrl_id;
  int save_def_values_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  PluginOptions* options;
  DefOptionValuesDlgData(PluginOptions* options): options(options) {
  }
};

intptr_t WINAPI def_option_values_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  DefOptionValuesDlgData* dlg_data = (DefOptionValuesDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    PluginOptions* options = dlg_data->options;

    options->ignore_errors = DlgItem_GetCheck(g_far, hDlg, dlg_data->ignore_errors_ctrl_id) == BSTATE_CHECKED;

    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->oo_skip_ctrl_id) == BSTATE_CHECKED) options->overwrite = ooSkip;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->oo_overwrite_ctrl_id) == BSTATE_CHECKED) options->overwrite = ooOverwrite;
    else options->overwrite = ooAsk;

    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->show_stats_always_ctrl_id) == BSTATE_CHECKED) options->show_stats = ssoAlways;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->show_stats_never_ctrl_id) == BSTATE_CHECKED) options->show_stats = ssoNever;
    else options->show_stats = ssoIfError;

    options->use_file_filters = DlgItem_GetCheck(g_far, hDlg, dlg_data->use_file_filters_ctrl_id) == BSTATE_CHECKED;
    options->save_def_values = DlgItem_GetCheck(g_far, hDlg, dlg_data->save_def_values_ctrl_id) == BSTATE_CHECKED;
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_def_option_values_dlg(PluginOptions& options) {
  DefOptionValuesDlgData dlg_data(&options);
  FarDialog dlg(c_def_option_values_dialog, far_get_msg(MSG_OPTIONS_DLG_DEF_OPTION_VALUES_TITLE), 40);

  // ignore errors option
  dlg_data.ignore_errors_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_IGNORE_ERRORS), options.ignore_errors);
  dlg.new_line();
  // overwrite files option
  dlg.label(far_get_msg(MSG_COPY_FILES_DLG_OO));
  dlg.new_line();
  dlg.spacer(2);
  dlg_data.oo_skip_ctrl_id = dlg.radio_button(far_get_msg(MSG_COPY_FILES_DLG_OO_SKIP), options.overwrite == ooSkip);
  dlg.spacer(2);
  dlg_data.oo_overwrite_ctrl_id = dlg.radio_button(far_get_msg(MSG_COPY_FILES_DLG_OO_OVERWRITE), options.overwrite == ooOverwrite);
  dlg.spacer(2);
  dlg_data.oo_ask_ctrl_id = dlg.radio_button(far_get_msg(MSG_COPY_FILES_DLG_OO_ASK), options.overwrite == ooAsk);
  dlg.new_line();
  // show stats option
  dlg.label(far_get_msg(MSG_OPTIONS_SHOW_STATS));
  dlg.new_line();
  dlg.spacer(2);
  dlg_data.show_stats_always_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_ALWAYS), options.show_stats == ssoAlways, DIF_GROUP);
  dlg.spacer(2);
  dlg_data.show_stats_never_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_NEVER), options.show_stats == ssoNever);
  dlg.spacer(2);
  dlg_data.show_stats_if_error_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_SHOW_STATS_IF_ERROR), options.show_stats == ssoIfError);
  dlg.new_line();
  // use ActiveSync file filters when sending files to device
  dlg_data.use_file_filters_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_USE_FILE_FILTERS), options.use_file_filters);
  dlg.new_line();
  // save default values
  dlg_data.save_def_values_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_SAVE_DEF_VALUES), options.save_def_values);
  dlg.new_line();

  dlg.separator();
  dlg.new_line();
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(def_option_values_dlg_proc, &dlg_data, L"Config");

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct HotKeyOptionsDlgData {
  int key_execute_ctrl_id;
  int key_attr_ctrl_id;
  int key_hide_rom_files_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  PluginOptions* options;
  HotKeyOptionsDlgData(PluginOptions* options): options(options) {
  }
};

bool verify_hot_key(HANDLE h_dlg, int ctrl_id) {
  std::string hot_key = far_get_text(h_dlg, ctrl_id);
  INPUT_RECORD input_rec;
  if (!g_fsf.FarNameToInputRecord(hot_key.data(), &input_rec) || input_rec.EventType != KEY_EVENT) {
    far_message(c_error_dialog_guid, far_get_msg(MSG_PLUGIN_NAME) + L"\n" + std::string::format(far_get_msg(MSG_ERR_INVALID_HOT_KEY).data(), hot_key.data()), 0, FMSG_WARNING | FMSG_MB_OK);
    return false;
  }
  return true;
}

std::string get_hot_key(HANDLE h_dlg, int ctrl_id) {
  return far_get_text(h_dlg, ctrl_id);
}

intptr_t WINAPI hot_key_options_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  HotKeyOptionsDlgData* dlg_data = (HotKeyOptionsDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    PluginOptions* options = dlg_data->options;
    if (verify_hot_key(hDlg, dlg_data->key_execute_ctrl_id) && verify_hot_key(hDlg, dlg_data->key_attr_ctrl_id) && verify_hot_key(hDlg, dlg_data->key_hide_rom_files_ctrl_id)) {
      options->key_execute = get_hot_key(hDlg, dlg_data->key_execute_ctrl_id);
      options->key_attr = get_hot_key(hDlg, dlg_data->key_attr_ctrl_id);
      options->key_hide_rom_files = get_hot_key(hDlg, dlg_data->key_hide_rom_files_ctrl_id);
    }
    else return FALSE;
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_hot_key_options_dlg(PluginOptions& options) {
  HotKeyOptionsDlgData dlg_data(&options);
  FarDialog dlg(c_hot_keys_dialog_guid, far_get_msg(MSG_OPTIONS_DLG_HOT_KEYS_TITLE), 40);

  unsigned p = max(max(far_get_msg(MSG_OPTIONS_DLG_HOT_KEY_EXECUTE).size(), far_get_msg(MSG_OPTIONS_DLG_HOT_KEY_ATTR).size()), far_get_msg(MSG_OPTIONS_DLG_HOT_KEY_HIDE_ROM_FILES).size()) + 1;
  dlg.label(far_get_msg(MSG_OPTIONS_DLG_HOT_KEY_EXECUTE));
  dlg.pad(p);
  dlg_data.key_execute_ctrl_id = dlg.var_edit_box(options.key_execute, 15);
  dlg.new_line();
  dlg.label(far_get_msg(MSG_OPTIONS_DLG_HOT_KEY_ATTR));
  dlg.pad(p);
  dlg_data.key_attr_ctrl_id = dlg.var_edit_box(options.key_attr, 15);
  dlg.new_line();
  dlg.label(far_get_msg(MSG_OPTIONS_DLG_HOT_KEY_HIDE_ROM_FILES));
  dlg.pad(p);
  dlg_data.key_hide_rom_files_ctrl_id = dlg.var_edit_box(options.key_hide_rom_files, 15);
  dlg.new_line();

  dlg.separator();
  dlg.new_line();
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(hot_key_options_dlg_proc, &dlg_data, L"Config");

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct PluginOptionsDlgData {
  int add_to_plugin_menu_ctrl_id;
  int add_to_disk_menu_ctrl_id;
  int hide_copy_dlg_ctrl_id;
  int save_last_dir_ctrl_id;
  int hide_rom_files_ctrl_id;
  int exit_on_dot_dot_ctrl_id;
  int show_free_space_ctrl_id;
  int auto_buf_size_ctrl_id;
  int copy_buf_size_ctrl_id;
  int prefix_ctrl_id;
  int rapi_ctrl_id;
  int rapi2_ctrl_id;
  int def_options_values_ctrl_id;
  int hot_keys_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  PluginOptions* options;
  PluginOptionsDlgData(PluginOptions* options): options(options) {
  }
};

intptr_t WINAPI plugin_options_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  PluginOptionsDlgData* dlg_data = (PluginOptionsDlgData*) Dlg_GetDlgData(g_far, hDlg);
  PluginOptions* options = dlg_data->options;
  bool auto_buf_size = DlgItem_GetCheck(g_far, hDlg, dlg_data->auto_buf_size_ctrl_id) == BSTATE_CHECKED;
  if (Msg == DN_INITDIALOG) {
    g_far.SendDlgMessage(hDlg, DM_SHOWITEM, dlg_data->copy_buf_size_ctrl_id, reinterpret_cast<void*>(auto_buf_size ? 0 : 1));
    g_far.SendDlgMessage(hDlg, DM_SHOWITEM, dlg_data->copy_buf_size_ctrl_id + 1, reinterpret_cast<void*>(auto_buf_size ? 0 : 1));
  }
  else if ((Msg == DN_BTNCLICK) && (Param1 == dlg_data->auto_buf_size_ctrl_id)) {
    g_far.SendDlgMessage(hDlg, DM_SHOWITEM, dlg_data->copy_buf_size_ctrl_id, reinterpret_cast<void*>(auto_buf_size ? 0 : 1));
    g_far.SendDlgMessage(hDlg, DM_SHOWITEM, dlg_data->copy_buf_size_ctrl_id + 1, reinterpret_cast<void*>(auto_buf_size ? 0 : 1));
  }
  else if ((Msg == DN_BTNCLICK) && (Param1 == dlg_data->def_options_values_ctrl_id)) {
    if (show_def_option_values_dlg(*options)) save_plugin_options(*options);
    g_far.SendDlgMessage(hDlg, DM_SETFOCUS, dlg_data->ok_ctrl_id, 0);
  }
  else if ((Msg == DN_BTNCLICK) && (Param1 == dlg_data->hot_keys_ctrl_id)) {
    if (show_hot_key_options_dlg(*options)) save_plugin_options(*options);
    g_far.SendDlgMessage(hDlg, DM_SETFOCUS, dlg_data->ok_ctrl_id, 0);
  }
  else if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    std::string prefix = far_get_text(hDlg, dlg_data->prefix_ctrl_id);
    if (prefix.size() == 0) {
      far_message(c_error_dialog_guid, far_get_msg(MSG_PLUGIN_NAME) + L"\n" + far_get_msg(MSG_ERR_INVALID_PREFIX), 0, FMSG_WARNING | FMSG_MB_OK);
      return FALSE;
    }
    options->prefix = prefix;

    options->add_to_plugin_menu = DlgItem_GetCheck(g_far, hDlg, dlg_data->add_to_plugin_menu_ctrl_id) == BSTATE_CHECKED;
    options->add_to_disk_menu = DlgItem_GetCheck(g_far, hDlg, dlg_data->add_to_disk_menu_ctrl_id) == BSTATE_CHECKED;

    options->hide_copy_dlg = DlgItem_GetCheck(g_far, hDlg, dlg_data->hide_copy_dlg_ctrl_id) == BSTATE_CHECKED;
    options->save_last_dir = DlgItem_GetCheck(g_far, hDlg, dlg_data->save_last_dir_ctrl_id) == BSTATE_CHECKED;
    options->hide_rom_files = DlgItem_GetCheck(g_far, hDlg, dlg_data->hide_rom_files_ctrl_id) == BSTATE_CHECKED;
    options->exit_on_dot_dot = DlgItem_GetCheck(g_far, hDlg, dlg_data->exit_on_dot_dot_ctrl_id) == BSTATE_CHECKED;
    options->show_free_space = DlgItem_GetCheck(g_far, hDlg, dlg_data->show_free_space_ctrl_id) == BSTATE_CHECKED;

    if (auto_buf_size) options->copy_buf_size = -1;
    else {
      std::string bs = far_get_text(hDlg, dlg_data->copy_buf_size_ctrl_id);
      options->copy_buf_size = _wtoi(bs.data());
      if (options->copy_buf_size == 0) options->copy_buf_size = -1;
    }

    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->rapi_ctrl_id) == BSTATE_CHECKED) options->access_method = amRapi;
    else options->access_method = amRapi2;
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_plugin_options_dlg(PluginOptions& options) {
  PluginOptionsDlgData dlg_data(&options);
  FarDialog dlg(c_plugin_options_dialog_guid, far_get_msg(MSG_PLUGIN_NAME) + ' ' + far_get_msg(MSG_OPTIONS_DLG_TITLE), 60);
  dlg_data.add_to_disk_menu_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_DISK_MENU), options.add_to_disk_menu);
  dlg.spacer(2);
  dlg_data.add_to_plugin_menu_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_PLUGIN_MENU), options.add_to_plugin_menu);
  dlg.new_line();
  dlg_data.hide_copy_dlg_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_HIDE_COPY_DLG), options.hide_copy_dlg);
  dlg.new_line();
  dlg_data.save_last_dir_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_SAVE_LAST_DIR), options.save_last_dir);
  dlg.new_line();
  dlg_data.hide_rom_files_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_HIDE_ROM_FILES), options.hide_rom_files);
  dlg.new_line();
  dlg_data.exit_on_dot_dot_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_EXIT_ON_DOT_DOT), options.exit_on_dot_dot);
  dlg.new_line();
  dlg_data.show_free_space_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_SHOW_FREE_SPACE), options.show_free_space);
  dlg.new_line();
  dlg.label(far_get_msg(MSG_OPTIONS_DLG_BUFFER_SIZE));
  dlg.spacer(1);
  dlg_data.auto_buf_size_ctrl_id = dlg.check_box(far_get_msg(MSG_OPTIONS_DLG_AUTO_BUF_SIZE), options.copy_buf_size == -1);
  dlg.spacer(1);
  std::string bs;
  if (options.copy_buf_size != -1) bs.copy_fmt(L"%u", options.copy_buf_size);
  dlg_data.copy_buf_size_ctrl_id = dlg.mask_edit_box(bs, L"9999", 4);
  dlg.spacer(1);
  dlg.label(far_get_msg(MSG_SUFFIX_SIZE_KB));
  dlg.new_line();
  dlg.label(far_get_msg(MSG_OPTIONS_DLG_PREFIX));
  dlg.spacer(1);
  dlg_data.prefix_ctrl_id = dlg.var_edit_box(options.prefix, 15);
  dlg.new_line();

  dlg.separator();
  dlg.new_line();
  dlg.label(far_get_msg(MSG_OPTIONS_DLG_ACCESS_METHOD));
  dlg.new_line();
  dlg.spacer(2);
  dlg_data.rapi_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_DLG_RAPI), options.access_method == amRapi, DIF_GROUP);
  dlg.spacer(2);
  dlg_data.rapi2_ctrl_id = dlg.radio_button(far_get_msg(MSG_OPTIONS_DLG_RAPI2), options.access_method == amRapi2);
  dlg.new_line();

  dlg.separator();
  dlg.new_line();
  dlg_data.def_options_values_ctrl_id = dlg.button(far_get_msg(MSG_OPTIONS_DLG_DEF_OPTION_VALUES), DIF_BTNNOCLOSE);
  dlg.spacer(1);
  dlg_data.hot_keys_ctrl_id = dlg.button(far_get_msg(MSG_OPTIONS_DLG_HOT_KEYS), DIF_BTNNOCLOSE);
  dlg.new_line();

  dlg.separator();
  dlg.new_line();
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(plugin_options_dlg_proc, &dlg_data, L"Config");

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct FileAttrDlgData {
  int read_only_ctrl_id;
  int archive_ctrl_id;
  int hidden_ctrl_id;
  int system_ctrl_id;
  int compressed_ctrl_id;
  int encrypted_ctrl_id;
  int sparse_ctrl_id;
  int temporary_ctrl_id;
  int reparse_point_ctrl_id;
  int in_rom_ctrl_id;
  int rom_module_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  FileAttrOptions* options;
  FileAttrDlgData(FileAttrOptions* options): options(options) {
  }
};

intptr_t WINAPI file_attr_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  FileAttrDlgData* dlg_data = (FileAttrDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    FileAttrOptions* options = dlg_data->options;
    options->attr_and = 0xFFFFFFFF;
    options->attr_or = 0;
    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->read_only_ctrl_id) == BSTATE_UNCHECKED) options->attr_and &= ~FILE_ATTRIBUTE_READONLY;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->read_only_ctrl_id) == BSTATE_CHECKED) options->attr_or |= FILE_ATTRIBUTE_READONLY;
    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->archive_ctrl_id) == BSTATE_UNCHECKED) options->attr_and &= ~FILE_ATTRIBUTE_ARCHIVE;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->archive_ctrl_id) == BSTATE_CHECKED) options->attr_or |= FILE_ATTRIBUTE_ARCHIVE;
    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->hidden_ctrl_id) == BSTATE_UNCHECKED) options->attr_and &= ~FILE_ATTRIBUTE_HIDDEN;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->hidden_ctrl_id) == BSTATE_CHECKED) options->attr_or |= FILE_ATTRIBUTE_HIDDEN;
    if (DlgItem_GetCheck(g_far, hDlg, dlg_data->system_ctrl_id) == BSTATE_UNCHECKED) options->attr_and &= ~FILE_ATTRIBUTE_SYSTEM;
    else if (DlgItem_GetCheck(g_far, hDlg, dlg_data->system_ctrl_id) == BSTATE_CHECKED) options->attr_or |= FILE_ATTRIBUTE_SYSTEM;
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

#define ATTR_VALUE(attr) (options.attr_and & attr) ? 1 : ((options.attr_or & attr) ? 2 : 0)
bool show_file_attr_dlg(FileAttrOptions& options) {
  FileAttrDlgData dlg_data(&options);
  const unsigned c_client_xs = 45;
  FarDialog dlg(c_attr_dialog_guid, far_get_msg(MSG_ATTR_DLG_TITLE), c_client_xs);
  DWORD flag = options.single_file ? 0 : DIF_3STATE;

  // attributes
  dlg_data.read_only_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_READ_ONLY), ATTR_VALUE(FILE_ATTRIBUTE_READONLY), flag);
  dlg.pad(c_client_xs / 2);
  dlg_data.sparse_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_SPARSE), ATTR_VALUE(FILE_ATTRIBUTE_SPARSE_FILE), flag | DIF_DISABLE);
  dlg.new_line();

  dlg_data.archive_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_ARCHIVE), ATTR_VALUE(FILE_ATTRIBUTE_ARCHIVE), flag);
  dlg.pad(c_client_xs / 2);
  dlg_data.temporary_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_TEMPORARY), ATTR_VALUE(FILE_ATTRIBUTE_TEMPORARY), flag | DIF_DISABLE);
  dlg.new_line();

  dlg_data.hidden_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_HIDDEN), ATTR_VALUE(FILE_ATTRIBUTE_HIDDEN), flag);
  dlg.pad(c_client_xs / 2);
  dlg_data.reparse_point_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_REPARSE_POINT), ATTR_VALUE(FILE_ATTRIBUTE_REPARSE_POINT), flag | DIF_DISABLE);
  dlg.new_line();

  dlg_data.system_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_SYSTEM), ATTR_VALUE(FILE_ATTRIBUTE_SYSTEM), flag);
  dlg.pad(c_client_xs / 2);
  dlg_data.in_rom_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_IN_ROM), ATTR_VALUE(FILE_ATTRIBUTE_INROM), flag | DIF_DISABLE);
  dlg.new_line();

  dlg_data.compressed_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_COMPRESSED), ATTR_VALUE(FILE_ATTRIBUTE_COMPRESSED), flag | DIF_DISABLE);
  dlg.pad(c_client_xs / 2);
  dlg_data.rom_module_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_ROM_MODULE), ATTR_VALUE(FILE_ATTRIBUTE_ROMMODULE), flag | DIF_DISABLE);
  dlg.new_line();

  dlg_data.encrypted_ctrl_id = dlg.check_box(far_get_msg(MSG_ATTR_DLG_ENCRYPTED), ATTR_VALUE(FILE_ATTRIBUTE_ENCRYPTED), flag | DIF_DISABLE);
  dlg.pad(c_client_xs / 2);
  dlg.new_line();

  dlg.separator();
  dlg.new_line();

  // OK & Cancel buttons
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(file_attr_dlg_proc, &dlg_data);

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct RunDlgData {
  int cmd_line_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;
  RunOptions* options;
  RunDlgData(RunOptions* options): options(options) {
  }
};

intptr_t WINAPI run_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  RunDlgData* dlg_data = (RunDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    RunOptions* options = dlg_data->options;
    options->cmd_line = far_get_text(hDlg, dlg_data->cmd_line_ctrl_id);
    options->cmd_line.strip();
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_run_dlg(RunOptions& options) {
  RunDlgData dlg_data(&options);
  const unsigned c_client_xs = 60;
  FarDialog dlg(c_run_dialog_guid, far_get_msg(MSG_RUN_DLG_TITLE), c_client_xs);
  // command line
  dlg.label(far_get_msg(MSG_RUN_DLG_CMD_LINE));
  dlg.new_line();
  dlg_data.cmd_line_ctrl_id = dlg.var_edit_box(options.cmd_line, c_client_xs, DIF_HISTORY);
  dlg.item().History = L"WMExplorerRun";
  dlg.new_line();
  // separator
  dlg.separator();
  dlg.new_line();
  // OK & Cancel buttons
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(run_dlg_proc, &dlg_data);

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}

struct FilterDlgData {
  int ok_ctrl_id;
  int cancel_ctrl_id;
  Array<int> filter_enabled_ctrl_id;
  Array<int> filter_idx_ctrl_id;
  Array<FilterSelection>* selection;
  FilterDlgData(Array<FilterSelection>* selection): selection(selection) {
  }
};

intptr_t WINAPI filter_dlg_proc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
  FilterDlgData* dlg_data = (FilterDlgData*) Dlg_GetDlgData(g_far, hDlg);
  if ((Msg == DN_CLOSE) && (Param1 >= 0) && (Param1 != dlg_data->cancel_ctrl_id)) {
    dlg_data->selection->clear();
    for (unsigned i = 0; i < dlg_data->filter_enabled_ctrl_id.size(); i++) {
      if (DlgItem_GetCheck(g_far, hDlg, dlg_data->filter_enabled_ctrl_id[i]) == BSTATE_CHECKED) {
        FilterSelection fs;
        fs.src_idx = i;
        fs.dst_idx = (unsigned) DlgList_GetCurPos(g_far, hDlg, dlg_data->filter_idx_ctrl_id[i]);
        dlg_data->selection->add(fs);
      }
    }
  }
  return g_far.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool show_filters_dlg(const std::vector<FileFilters>& filter_list, Array<FilterSelection>& filter_selection) {
  FilterDlgData dlg_data(&filter_selection);
  FarDialog dlg(c_filter_dialog_guid, far_get_msg(MSG_FILTER_DLG_TITLE), 60);
  dlg.label(far_get_msg(MSG_FILTER_DLG_SELECT_FILES));
  dlg.new_line();
  Array<wchar_t> hot_keys;
  for (unsigned i = 0; i < filter_list.size(); i++) {
    // find hot key
    std::string ext = filter_list[i].src_ext;
    for (unsigned j = 0; j < ext.size(); j++) {
      if (hot_keys.search(ext[j]) == -1) {
        hot_keys += ext[j];
        ext.insert(j, L"&");
        break;
      }
    }
    dlg_data.filter_enabled_ctrl_id += dlg.check_box(ext, false);
    dlg.spacer(1);
    dlg.label(far_get_msg(MSG_FILTER_DLG_TO));
    dlg.spacer(1);
    // target file type combo
    std::vector<std::string> items;
    for (unsigned j = 0; j < filter_list[i].size(); j++) {
      items += filter_list[i][j].description + L" (" + filter_list[i][j].dst_ext + L")";
    }
    dlg_data.filter_idx_ctrl_id += dlg.combo_box(items, 0, AUTO_SIZE, DIF_DROPDOWNLIST);
    dlg.new_line();
  }
  // separator
  dlg.separator();
  dlg.new_line();
  // OK & Cancel buttons
  dlg_data.ok_ctrl_id = dlg.def_button(far_get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
  dlg_data.cancel_ctrl_id = dlg.button(far_get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
  dlg.new_line();

  intptr_t item = dlg.show(filter_dlg_proc, &dlg_data);

  return (item != -1) && (item != dlg_data.cancel_ctrl_id);
}
