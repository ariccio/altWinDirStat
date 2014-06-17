#define _ERROR_WINDOWS
#include "error.h"

#include "msg.h"
#include "guids.h"
#include "util.h"
#include "options.h"
#include "dlgapi.h"
#include "file_filters.h"
#include "file_info.h"
#include "ui.h"
#include "filepath.h"
#include "plugin.h"
#include "as_api.h"
#include "rapi_proc.h"

const GUID CLSID_RAPI = { 0x35440327, 0x1517, 0x4B72, { 0x86, 0x5E, 0x3F, 0xFE, 0x8E, 0x97, 0x00, 0x2F } };
const GUID IID_IRAPIDesktop = { 0xDCBEB807, 0x14D0, 0x4CBD, { 0x92, 0x6C, 0xB9, 0x91, 0xF4, 0xFD, 0x1B, 0x91 } };

bool g_rapi2 = false;
bool com_init = false;
IRAPIDesktop* desktop = NULL;
HMODULE h_rapi_lib = NULL;
RapiLibrary rapi;
HMODULE h_ceutil_lib = NULL;
CeUtilLibrary ceutil;

void AutoBuffer::start_measurement() {
  if (!auto_size) return;
  QueryPerformanceCounter((PLARGE_INTEGER) &start_time);
}

void AutoBuffer::end_measurement(unsigned __int64 data_size) {
  if (!auto_size || (data_size < c_min_size)) return;
  unsigned __int64 end_time;
  QueryPerformanceCounter((PLARGE_INTEGER) &end_time);
  if (end_time > start_time) {
    unsigned __int64 new_size = mul_div(data_size, g_time_freq / 2, end_time - start_time);
    if (new_size < c_min_size) buffer_size = c_min_size;
    else if (new_size > c_max_size) buffer_size = c_max_size;
    else buffer_size = (unsigned) new_size / 4096 * 4096;
  }
  else buffer_size = c_max_size;
  LOG_MSG(AnsiString::format("buffer size = %u", buffer_size));
}

intptr_t error_dlg(Error& e, const std::string& message) {
  std::string msg;
  msg.add(far_get_msg(MSG_PLUGIN_NAME)).add('\n');
  if (message.size() != 0) msg.add(word_wrap(message, get_msg_width())).add('\n');
  std::string err_msg = word_wrap(e.message(), get_msg_width());
  if (err_msg.size() != 0) msg.add(err_msg).add('\n');
  msg.add_fmt(L"%S:%u v.%u.%u.%u.%u"PLUGIN_TYPE, &extract_file_name(oem_to_unicode(e.file)), e.line, g_version.major, g_version.minor, g_version.patch, g_version.revision).add('\n');
  msg.add(far_get_msg(MSG_BUTTON_RETRY)).add('\n');
  msg.add(far_get_msg(MSG_BUTTON_IGNORE)).add('\n');
  msg.add(far_get_msg(MSG_BUTTON_IGNORE_ALL)).add('\n');
  msg.add(far_get_msg(MSG_BUTTON_CANCEL)).add('\n');
  return far_message(c_error_dialog_guid, msg, 4, FMSG_WARNING);
}

std::string long_path(const std::string& path) {
  if (path.equal(0, L"\\\\?\\") || path.equal(0, L"\\\\.\\")) return path;
  if (path.equal(0, L"\\??\\")) return std::string(path).replace(0, 4, L"\\\\?\\");
  if (path.equal(0, L"\\\\")) return std::string(path).replace(0, 1, L"\\\\?\\UNC");
  return L"\\\\?\\" + path;
}

#define BEGIN_RETRY_BLOCK \
{ \
  bool f_repeat = true; \
  do { \
    try {

#define END_RETRY_BLOCK \
      f_repeat = false; \
    } \
    catch (Error& e) { \
      if (options.ignore_errors || !options.show_error) { \
        throw; \
      } \
      else { \
        intptr_t ret = error_dlg(e, fit_str(plugin->last_object, get_msg_width())); \
        if((ret == -1) || (ret == 3)) { \
          BREAK; \
        } \
        else if (ret == 0) { \
        } \
        else if (ret == 1) { \
          ignore_errors = true; \
          throw; \
        } \
        else if (ret == 2) { \
          ignore_errors = true; \
          options.ignore_errors = true; \
          throw; \
        } \
      } \
    } \
  } \
  while (f_repeat); \
}

#define LOAD_PROC(lib, name) CHECK_API((lib##.name = (P##name*) GetProcAddress(h_##lib##_lib, #name)) != NULL)

void init_if_needed() {
  // init ceutil library
  if (h_ceutil_lib == NULL) {
    try {
      h_ceutil_lib = LoadLibraryW(L"ceutil.dll");
      if ((h_ceutil_lib == NULL) && (GetLastError() == ERROR_MOD_NOT_FOUND)) FAIL(MsgError(far_get_msg(MSG_ERR_NO_ACTIVESYNC)));
      CHECK_API(h_ceutil_lib != NULL);
      LOAD_PROC(ceutil, CeSvcOpenW);
      LOAD_PROC(ceutil, CeSvcClose);
      LOAD_PROC(ceutil, CeSvcEnumProfiles);
      LOAD_PROC(ceutil, CeSvcOpenExW);
    }
    catch (...) {
      if (h_ceutil_lib != NULL) {
        FreeLibrary(h_ceutil_lib);
        h_ceutil_lib = NULL;
      }
      throw;
    }
  }
  // verify that ActiveSync is installed for current user
  HCESVC hkey;
  HRESULT hr = ceutil.CeSvcOpenW(CESVC_ROOT_USER, L"", FALSE, &hkey);
  if (SUCCEEDED(hr)) ceutil.CeSvcClose(hkey);
  if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) FAIL(MsgError(far_get_msg(MSG_ERR_NOT_ACTIVESYNC_USER)));
  CHECK_COM(hr);
  // init rapi library
  if (g_rapi2) {
    if (!com_init) {
      OSVERSIONINFO osver;
      osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      CHECK_API(GetVersionEx(&osver) != 0);
      HRESULT hr;
      if ((osver.dwMajorVersion == 6) && (osver.dwMinorVersion == 0)) { // Vista
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (hr == RPC_E_CHANGED_MODE) FAIL(MsgError(far_get_msg(MSG_ERR_COM_MODEL)));
      }
      else {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (hr == RPC_E_CHANGED_MODE) hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
      }
      if ((hr != S_OK) && (hr != S_FALSE)) CHECK_COM(hr);
      com_init = true;
    }
    if (desktop == NULL) {
      HRESULT hr = CoCreateInstance(CLSID_RAPI, NULL, CLSCTX_INPROC_SERVER, IID_IRAPIDesktop, (void**)&desktop);
      if (hr == REGDB_E_CLASSNOTREG) FAIL(MsgError(far_get_msg(MSG_ERR_NO_ACTIVESYNC)));
      CHECK_COM(hr);
    }
  }
  else {
    if (h_rapi_lib == NULL) {
      const wchar_t* c_rapi_lib_name = L"rapi.dll";
      h_rapi_lib = LoadLibraryW(c_rapi_lib_name);
      if ((h_rapi_lib == NULL) && (GetLastError() == ERROR_MOD_NOT_FOUND)) FAIL(MsgError(far_get_msg(MSG_ERR_NO_ACTIVESYNC)));
      CHECK_API(h_rapi_lib != NULL);
      try {
        LOAD_PROC(rapi, CeRapiInitEx);
        LOAD_PROC(rapi, CeRapiUninit);
        LOAD_PROC(rapi, CeFindAllFiles);
        LOAD_PROC(rapi, CeRapiFreeBuffer);
        LOAD_PROC(rapi, CeFindFirstFile);
        LOAD_PROC(rapi, CeFindNextFile);
        LOAD_PROC(rapi, CeFindClose);
        LOAD_PROC(rapi, CeCreateFile);
        LOAD_PROC(rapi, CeCloseHandle);
        LOAD_PROC(rapi, CeReadFile);
        LOAD_PROC(rapi, CeWriteFile);
        LOAD_PROC(rapi, CeDeleteFile);
        LOAD_PROC(rapi, CeGetFileAttributes);
        LOAD_PROC(rapi, CeSetFileAttributes);
        LOAD_PROC(rapi, CeSetFileTime);
        LOAD_PROC(rapi, CeCreateDirectory);
        LOAD_PROC(rapi, CeRemoveDirectory);
        LOAD_PROC(rapi, CeMoveFile);
        LOAD_PROC(rapi, CeCopyFile);
        LOAD_PROC(rapi, CeGetVersionEx);
        LOAD_PROC(rapi, CeGlobalMemoryStatus);
        LOAD_PROC(rapi, CeGetDiskFreeSpaceEx);
        LOAD_PROC(rapi, CeRegOpenKeyEx);
        LOAD_PROC(rapi, CeRegQueryValueEx);
        LOAD_PROC(rapi, CeRegCloseKey);
        LOAD_PROC(rapi, CeCreateProcess);
        LOAD_PROC(rapi, CeGetSystemPowerStatusEx);
        LOAD_PROC(rapi, CeGetLastError);
        LOAD_PROC(rapi, CeRapiGetError);
      }
      catch (...) {
        FreeLibrary(h_rapi_lib);
        h_rapi_lib = NULL;
        throw;
      }
    }
  }
}

void cleanup() {
  if (g_rapi2) {
    if (desktop != NULL) {
      desktop->Release();
      desktop = NULL;
    }
    if (com_init) {
      CoUninitialize();
      com_init = false;
    }
  }
  else {
    if (h_rapi_lib != NULL) VERIFY(FreeLibrary(h_rapi_lib) != 0);
  }
  if (h_ceutil_lib != NULL) VERIFY(FreeLibrary(h_ceutil_lib) != 0);
}

void get_device_list(std::vector<DeviceInfo>& list) {
  list.clear();
  if (g_rapi2) {
    IRAPIEnumDevices* enum_dev;
    CHECK_COM(desktop->EnumDevices(&enum_dev));
    try {
      ULONG cnt;
      CHECK_COM(enum_dev->GetCount(&cnt));
      DeviceInfo di;
      for (unsigned i = 0; i < cnt; i++) {
        IRAPIDevice* device;
        CHECK_COM(enum_dev->Next(&device));
        try {
          RAPI_DEVICESTATUS dev_status;
          CHECK_COM(device->GetConnectStat(&dev_status));
          if (dev_status == RAPI_DEVICE_CONNECTED) {
            RAPI_DEVICEINFO dev_info;
            CHECK_COM(device->GetDeviceInfo(&dev_info));
            try {
              di.id = dev_info.DeviceId;
              di.name = dev_info.bstrName;
              di.platform = dev_info.bstrPlatform;
            }
            finally (FreeDeviceInfoData(&dev_info));
            RAPI_CONNECTIONINFO con_info;
            CHECK_COM(device->GetConnectionInfo(&con_info));
            if (con_info.connectionType == RAPI_CONNECTION_USB) di.con_type = L"USB";
            else if (con_info.connectionType == RAPI_CONNECTION_IR) di.con_type = L"InfraRed";
            else if (con_info.connectionType == RAPI_CONNECTION_SERIAL) di.con_type = L"Serial";
            else if (con_info.connectionType == RAPI_CONNECTION_NETWORK) di.con_type = L"Network";
            else di.con_type = L"Unknown";
            list += di;
          }
        }
        finally (device->Release());
      }
    }
    finally (enum_dev->Release());
  }
  else {
    list.add(DeviceInfo());
  }
}

void create_session(RAPIDEVICEID id, PluginInstance* plugin) {
  if (g_rapi2) {
    assert(desktop != NULL);
    assert(plugin->device == NULL);
    assert(plugin->session == NULL);

    CHECK_COM(desktop->FindDevice(&id, RAPI_GETDEVICE_NONBLOCKING, &plugin->device));
    CHECK_COM(plugin->device->CreateSession(&plugin->session));
    CHECK_COM(plugin->session->CeRapiInit());
  }
  else {
    RAPIINIT rapi_init;
    memset(&rapi_init, 0, sizeof(rapi_init));
    rapi_init.cbSize = sizeof(RAPIINIT);
    CHECK_COM(rapi.CeRapiInitEx(&rapi_init));
    try {
      DWORD w = WaitForSingleObject(rapi_init.heRapiInit, 0);
      CHECK_API(w != WAIT_FAILED);
      if (w == WAIT_TIMEOUT) FAIL(MsgError(far_get_msg(MSG_ERR_NO_DEVICE)));
      assert(w == WAIT_OBJECT_0);
      CHECK_COM(rapi_init.hrRapiInit);
    }
    catch (...) {
      VERIFY(SUCCEEDED(rapi.CeRapiUninit()));
      throw;
    }
  }
}

void end_session(PluginInstance* plugin) {
  if (g_rapi2) {
    assert(plugin->device != NULL);
    assert(plugin->session != NULL);

    plugin->session->CeRapiUninit();
    plugin->session->Release();
    plugin->session = NULL;

    plugin->device->Release();
    plugin->device = NULL;
  }
  else {
    rapi.CeRapiUninit();
  }
}

void gen_file_list(const std::string& directory, IRAPISession* session, PluginItemList& file_list, const FileListOptions& options, UiLink& ui) {
  file_list.clear();
  DWORD file_count;
  CE_FIND_DATA* find_data;
  CHECK_RAPI(RAPI(CeFindAllFiles(COMPOSE_PATH(directory, L"*").data(), FAF_ATTRIBUTES | FAF_CREATION_TIME | FAF_LASTACCESS_TIME | FAF_LASTWRITE_TIME | FAF_SIZE_HIGH | FAF_SIZE_LOW | FAF_NAME, &file_count, &find_data)) != 0);
  try {
    file_list.extend(file_count);
    PluginPanelItem pi;
    for (unsigned i = 0; i < file_count; i++) {
      memset(&pi, 0, sizeof(pi));
      if (!options.hide_rom_files || ((find_data[i].dwFileAttributes & FILE_ATTRIBUTE_INROM) == 0)) {
        pi.FileAttributes = find_data[i].dwFileAttributes;
        pi.CreationTime = find_data[i].ftCreationTime;
        pi.LastAccessTime = find_data[i].ftLastAccessTime;
        pi.LastWriteTime = find_data[i].ftLastWriteTime;
        pi.FileSize = FILE_SIZE(find_data[i]);
        file_list.names += find_data[i].cFileName;
        pi.FileName = (wchar_t*) file_list.names.last().data();
        pi.AlternateFileName = L"";
        file_list += pi;
      }
    }
  }
  finally (VERIFY(SUCCEEDED(RAPI(CeRapiFreeBuffer(find_data)))));
}

#define ATTR_MASK (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)
#define IS_DOT_DIR(find_data) (IS_DIR(find_data) && ((wcscmp(find_data.cFileName, L".") == 0) || (wcscmp(find_data.cFileName, L"..") == 0)))

void overwrite_decision(const std::string& dst_path, const FileInfo& src_file_info, const FileInfo& dst_file_info, bool& overwrite, OverwriteOption& ov_option) {
  if (src_file_info.is_dir() != dst_file_info.is_dir()) FAIL(CustomError(far_get_msg(MSG_ERR_SAME_NAME_EXISTS), dst_path));
  if (ov_option == ooAsk) {
    OverwriteAction ov_action = show_overwrite_dlg(dst_path, src_file_info, dst_file_info);
    if (ov_action == oaCancel) BREAK;
    if (ov_action == oaYes) overwrite = true;
    else if (ov_action == oaYesAll) {
      overwrite = true;
      ov_option = ooOverwrite;
    }
    else if (ov_action == oaNo) overwrite = false;
    else if (ov_action == oaNoAll) {
      overwrite = false;
      ov_option = ooSkip;
    }
  }
  else if (ov_option == ooOverwrite) overwrite = true;
  else overwrite = false;
}

void list_add_file(const WIN32_FIND_DATAW& src_find_data, CreateListStats& stats, UiLink& ui, FileList& file_list) {
  if (ui.update_needed()) {
    draw_create_list_progress(stats);
  }
  file_list.add(FileInfo(src_find_data));
  if (IS_DIR(src_find_data)) {
    stats.dirs++;
  }
  else {
    stats.files++;
    stats.size += file_list.last().size;
  }
}

void list_add_dir(const std::string& src_dir, WIN32_FIND_DATAW& src_find_data, CreateListStats& stats, CreateListOptions& options, UiLink& ui, Log& log, PluginInstance* plugin, FileList& file_list) {
  bool ignore_errors = options.ignore_errors;
  try {
    std::string src_path = COMPOSE_PATH(src_dir, src_find_data.cFileName);
    bool more = true;
    WIN32_FIND_DATAW find_data;
    HANDLE h_find;
    plugin->last_object = src_path;
    BEGIN_RETRY_BLOCK;
    h_find = FindFirstFileW(long_path(COMPOSE_PATH(src_path, L"*")).data(), &find_data);
    if (h_find == INVALID_HANDLE_VALUE) {
      CHECK_API(GetLastError() == ERROR_NO_MORE_FILES);
      more = false;
    }
    END_RETRY_BLOCK;
    try {
      while (more) {
        if (!IS_DOT_DIR(find_data)) {
          if (ui.update_needed()) {
            draw_create_list_progress(stats);
          }
          list_add_file(find_data, stats, ui, file_list);
          if (IS_DIR(find_data)) {
            unsigned idx = file_list.size() - 1;
            list_add_dir(src_path, find_data, stats, options, ui, log, plugin, file_list);
            file_list.item(idx).child_cnt = file_list.size() - 1 - idx;
          }
        }
        plugin->last_object = src_path;
        BEGIN_RETRY_BLOCK;
        if (FindNextFileW(h_find, &find_data) == 0) {
          CHECK_API(GetLastError() == ERROR_NO_MORE_FILES);
          more = false;
        }
        END_RETRY_BLOCK;
      }
    }
    finally (if (h_find != INVALID_HANDLE_VALUE) VERIFY(FindClose(h_find) != 0));
  }
  catch (Error& e) {
    LOG_ERR(e);
    LOG_UMSG(plugin->last_object);
    if (ignore_errors || options.ignore_errors) {
      stats.errors++;
      log.add(lotScanDir, plugin->last_object, e.message());
    }
    else throw;
  }
}

FileList create_file_list(const std::string& src_dir, const std::string& src_file_name, CreateListStats& stats, CreateListOptions& options, UiLink& ui, Log& log, PluginInstance* plugin) {
  FileList file_list;
  bool ignore_errors = options.ignore_errors;
  try {
    std::string src_path = COMPOSE_PATH(src_dir, src_file_name);
    WIN32_FIND_DATAW find_data;
    HANDLE h_find;
    plugin->last_object = src_path;
    BEGIN_RETRY_BLOCK;
    h_find = FindFirstFileW(long_path(src_path).data(), &find_data);
    CHECK_API(h_find != INVALID_HANDLE_VALUE);
    END_RETRY_BLOCK;
    VERIFY(FindClose(h_find) != 0);
    list_add_file(find_data, stats, ui, file_list);
    if (IS_DIR(find_data)) {
      list_add_dir(src_dir, find_data, stats, options, ui, log, plugin, file_list);
      file_list.item(0).child_cnt = file_list.size() - 1;
    }
  }
  catch (Error& e) {
    LOG_ERR(e);
    LOG_UMSG(plugin->last_object);
    if (ignore_errors || options.ignore_errors) {
      stats.errors++;
      log.add(lotScanDir, plugin->last_object, e.message());
    }
    else throw;
  }
  return file_list;
}

void list_add_file(CE_FIND_DATA& src_find_data, CreateListStats& stats, UiLink& ui, FileList& file_list) {
  if (ui.update_needed()) {
    draw_create_list_progress(stats);
  }
  file_list.add(FileInfo(src_find_data));
  if (IS_DIR(src_find_data)) {
    stats.dirs++;
  }
  else {
    stats.files++;
    stats.size += file_list.last().size;
  }
}

void list_add_dir(const std::string& src_dir, CE_FIND_DATA& src_find_data, CreateListStats& stats, CreateListOptions& options, UiLink& ui, Log& log, IRAPISession* session, PluginInstance* plugin, FileList& file_list) {
  bool ignore_errors = options.ignore_errors;
  try {
    std::string src_path = COMPOSE_PATH(src_dir, src_find_data.cFileName);
    DWORD file_count;
    CE_FIND_DATA* find_data;
    plugin->last_object = src_path;
    BEGIN_RETRY_BLOCK;
    CHECK_RAPI(RAPI(CeFindAllFiles(COMPOSE_PATH(src_path, L"*").data(), FAF_ATTRIBUTES | FAF_CREATION_TIME | FAF_LASTACCESS_TIME | FAF_LASTWRITE_TIME | FAF_SIZE_HIGH | FAF_SIZE_LOW | FAF_NAME, &file_count, &find_data)) != 0);
    END_RETRY_BLOCK;
    try {
      file_list.extend(file_list.size() + file_count);
      for (unsigned i = 0; i < file_count; i++) {
        if (ui.update_needed()) {
          draw_create_list_progress(stats);
        }
        list_add_file(find_data[i], stats, ui, file_list);
        if (IS_DIR(find_data[i])) {
          unsigned idx = file_list.size() - 1;
          list_add_dir(src_path, find_data[i], stats, options, ui, log, session, plugin, file_list);
          file_list.item(idx).child_cnt = file_list.size() - 1 - idx;
        }
      }
    }
    finally (VERIFY(SUCCEEDED(RAPI(CeRapiFreeBuffer(find_data)))));
  }
  catch (Error& e) {
    LOG_ERR(e);
    LOG_UMSG(plugin->last_object);
    if (ignore_errors || options.ignore_errors) {
      stats.errors++;
      log.add(lotScanDir, plugin->last_object, e.message());
    }
    else throw;
  }
}

FileList create_file_list(const std::string& src_dir, const std::string& src_file_name, CreateListStats& stats, CreateListOptions& options, UiLink& ui, Log& log, IRAPISession* session, PluginInstance* plugin) {
  FileList file_list;
  bool ignore_errors = options.ignore_errors;
  try {
    std::string src_path = COMPOSE_PATH(src_dir, src_file_name);
    CE_FIND_DATA find_data;
    plugin->last_object = src_path;
    HANDLE h_find;
    BEGIN_RETRY_BLOCK;
    h_find = RAPI(CeFindFirstFile(src_path.data(), &find_data));
    CHECK_RAPI(h_find != INVALID_HANDLE_VALUE);
    END_RETRY_BLOCK;
    VERIFY(RAPI(CeFindClose(h_find)) != 0);
    list_add_file(find_data, stats, ui, file_list);
    if (IS_DIR(find_data)) {
      list_add_dir(src_dir, find_data, stats, options, ui, log, session, plugin, file_list);
      file_list.item(0).child_cnt = file_list.size() - 1;
    }
  }
  catch (Error& e) {
    LOG_ERR(e);
    LOG_UMSG(plugin->last_object);
    if (ignore_errors || options.ignore_errors) {
      stats.errors++;
      log.add(lotScanDir, plugin->last_object, e.message());
    }
    else throw;
  }
  return file_list;
}

void prepare_target_path(const std::string& path, PluginInstance* plugin) {
  FilePath fp_path(path);
  for (unsigned i = 1; i <= fp_path.size(); i++) {
    std::string partial_path = fp_path.get_partial_path(i);
    if (!dir_exists(partial_path)) {
      plugin->last_object = partial_path;
      CHECK_API(CreateDirectoryW(long_path(partial_path).data(), NULL) != 0);
    }
  }
}

void prepare_target_path(const std::string& path, IRAPISession* session, PluginInstance* plugin) {
  FilePath fp_path(path);
  for (unsigned i = 1; i <= fp_path.size(); i++) {
    std::string partial_path = fp_path.get_partial_path(i);
    if (!dir_exists(partial_path, session)) {
      plugin->last_object = partial_path;
      CHECK_RAPI(RAPI(CeCreateDirectory(partial_path.data(), NULL)) != 0);
    }
  }
}

void copy_files(bool src_remote, const std::string& src_dir, const FileList& src_file_list, bool dst_remote, const std::string& dst_dir, const std::string& dst_new_name, CopyFilesStats& stats, CopyFilesProgress& progress, CopyFilesOptions& options, UiLink& ui, AutoBuffer& buffer, Log& log, std::vector<FilterInterface>& filters, IRAPISession* session, PluginInstance* plugin) {
  Array<unsigned> dir_stack; // directory stack (prev. directory indices in src_file_list)
  FilePath src_fp(src_dir); // current source path
  FilePath dst_fp(dst_dir); // current destination path
  std::string src_path, dst_path;
  std::string src_file_name, dst_file_name;
  // start file list iteration
  for (unsigned i = 0; i < src_file_list.size(); i++) {
    // calc. current file paths
    src_file_name = src_file_list[i].file_name;
    if ((i == 0) && (dst_new_name.size() != 0)) dst_file_name = dst_new_name;
    else dst_file_name = src_file_list[i].file_name;
    COMPOSE_PATH2(src_path, src_fp.get_full_path(), src_file_name);
    COMPOSE_PATH2(dst_path, dst_fp.get_full_path(), dst_file_name);
    bool dst_file_exists = false; // true if destination file exists
    bool overwrite; // true if destination will be overwritten
    // update progress bar
    if (ui.update_needed()) {
      progress.src_path = src_path;
      progress.dst_path = dst_path;
      progress.file_size = src_file_list[i].size;
      progress.copied_file_size = 0;
      QueryPerformanceCounter((PLARGE_INTEGER) &progress.file_start_time);
      draw_copy_files_progress(progress, stats, options.move_files);
    }
    // start directory processing
    if (src_file_list[i].is_dir()) {
      bool ignore_errors = options.ignore_errors;
      bool skip_dir = false;
      try {
        plugin->last_object = dst_path;
        if (dst_remote) {
          CE_FIND_DATA dst_find_data;
          HANDLE h_dst_find = RAPI(CeFindFirstFile(dst_path.data(), &dst_find_data));
          if (h_dst_find != INVALID_HANDLE_VALUE) {
            VERIFY(RAPI(CeFindClose(h_dst_find)) != 0);
            dst_file_exists = true;
            overwrite_decision(dst_path, src_file_list[i], FileInfo(dst_find_data), overwrite, options.overwrite);
          }
        }
        else {
          WIN32_FIND_DATAW dst_find_data;
          HANDLE h_dst_find = FindFirstFileW(long_path(dst_path).data(), &dst_find_data);
          if (h_dst_find != INVALID_HANDLE_VALUE) {
            VERIFY(FindClose(h_dst_find) != 0);
            dst_file_exists = true;
            overwrite_decision(dst_path, src_file_list[i], FileInfo(dst_find_data), overwrite, options.overwrite);
          }
        }
        if (!dst_file_exists) {
          BEGIN_RETRY_BLOCK;
          if (dst_remote) {
            CHECK_RAPI(RAPI(CeCreateDirectory(dst_path.data(), NULL)) != 0);
          }
          else {
            CHECK_API(CreateDirectoryW(long_path(dst_path).data(), NULL) != 0);
          }
          END_RETRY_BLOCK;
        }
        skip_dir = dst_file_exists && !overwrite;
        if (!skip_dir) {
          BEGIN_RETRY_BLOCK;
          if (dst_remote) {
            CHECK_API(RAPI(CeSetFileAttributes(dst_path.data(), src_file_list[i].attr & ATTR_MASK)) != 0);
          }
          else {
            CHECK_API(SetFileAttributesW(long_path(dst_path).data(), src_file_list[i].attr & ATTR_MASK) != 0);
          }
          END_RETRY_BLOCK;
          // include directory into current paths
          src_fp += src_file_name;
          dst_fp += dst_file_name;
          // save directory into stack
          dir_stack.insert(0, i);
        }
      }
      catch (Error& e) {
        LOG_ERR(e);
        LOG_UMSG(plugin->last_object);
        if (ignore_errors || options.ignore_errors) {
          stats.errors++;
          skip_dir = true;
          log.add(lotCreateDir, plugin->last_object, e.message());
        }
        else throw;
      }
      // skip directory and its children
      if (skip_dir) {
        for (unsigned j = 0; j < src_file_list[i].child_cnt; j++) progress.processed_total_size += src_file_list[i + j].size;
        i += src_file_list[i].child_cnt;
      }
    }
    // process file
    else {
      bool ignore_errors = options.ignore_errors;
      unsigned __int64 progress_processed_total_size = progress.processed_total_size;
      try {
        // remote file copy
        if (src_remote && dst_remote) {
          CE_FIND_DATA dst_find_data;
          HANDLE h_dst_find = RAPI(CeFindFirstFile(dst_path.data(), &dst_find_data));
          if (h_dst_find != INVALID_HANDLE_VALUE) {
            VERIFY(RAPI(CeFindClose(h_dst_find)) != 0);
            dst_file_exists = true;
            overwrite_decision(dst_path, src_file_list[i], FileInfo(dst_find_data), overwrite, options.overwrite);
            if (overwrite) {
              // reset attributes and delete file
              RAPI(CeSetFileAttributes(dst_path.data(), FILE_ATTRIBUTE_NORMAL));
              RAPI(CeDeleteFile(dst_path.data()));
            }
          }
          if (!dst_file_exists || overwrite) {
            QueryPerformanceCounter((PLARGE_INTEGER) &progress.file_start_time);
            BEGIN_RETRY_BLOCK;
            CHECK_RAPI(RAPI(CeCopyFile(src_path.data(), dst_path.data(), (dst_file_exists && overwrite) ? FALSE : TRUE)) != 0);
            END_RETRY_BLOCK;
            // update progress data
            if (ui.update_needed()) {
              progress.src_path = src_path;
              progress.dst_path = dst_path;
              progress.file_size = src_file_list[i].size;
              progress.copied_file_size = src_file_list[i].size;
              draw_copy_files_progress(progress, stats, options.move_files);
            }
            progress.copied_total_size += src_file_list[i].size;
          }
        }
        // non-remote file copy
        else {
          // prepare file conversion if needed
          unsigned filter_idx = -1;
          std::string tmp_path;
          try {
            if (options.use_file_filters) {
              for (unsigned i = 0; i < filters.size(); i++) {
                unsigned ext_pos = src_path.rsearch('.');
                if ((ext_pos != -1) && (src_path.icompare(ext_pos + 1, src_path.size() - ext_pos - 1, filters[i].src_ext) == 0)) {
                  filter_idx = i;
                  break;
                }
              }
              if (filter_idx != -1) {
                // change destination file extension
                unsigned ext_pos = dst_path.rsearch('.');
                if ((ext_pos != -1) && (dst_path.icompare(ext_pos + 1, dst_path.size() - ext_pos - 1, filters[filter_idx].src_ext) == 0)) dst_path.replace(ext_pos + 1, dst_path.size() - ext_pos - 1, filters[filter_idx].dst_ext);
                else dst_path += L"." + filters[filter_idx].dst_ext;
                // create intermediate temporary file for conversion
                tmp_path = make_temp_file();
                // perform conversion
                if (!src_remote) {
                  far_message(c_progress_dialog_guid, far_get_msg(options.move_files ? MSG_MOVE_FILES_PROGRESS_TITLE : MSG_COPY_FILES_PROGRESS_TITLE) + '\n' + far_get_msg(MSG_PROGRESS_FILTER));
                  convert_file(filters[filter_idx].itf, src_path, tmp_path, true);
                }
              }
            }
            // open source file
            plugin->last_object = src_path;
            HANDLE h_src;
            BEGIN_RETRY_BLOCK;
            if (src_remote) {
              h_src = RAPI(CeCreateFile(src_path.data(), GENERIC_READ, FILE_SHARE_READ | (options.copy_shared ? FILE_SHARE_WRITE : 0), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
              CHECK_RAPI(h_src != INVALID_HANDLE_VALUE);
            }
            else {
              h_src = CreateFileW(long_path(filter_idx != -1 ? tmp_path : src_path).data(), GENERIC_READ, FILE_SHARE_READ | (options.copy_shared ? FILE_SHARE_WRITE : 0), NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
              CHECK_API(h_src != INVALID_HANDLE_VALUE);
            }
            END_RETRY_BLOCK;
            try {
              // check if destination file exists
              DWORD dst_attr;
              if (dst_remote) {
                CE_FIND_DATA dst_find_data;
                HANDLE h_dst_find = RAPI(CeFindFirstFile(dst_path.data(), &dst_find_data));
                if (h_dst_find != INVALID_HANDLE_VALUE) {
                  VERIFY(RAPI(CeFindClose(h_dst_find)) != 0);
                  dst_file_exists = true;
                  dst_attr = dst_find_data.dwFileAttributes;
                  overwrite_decision(dst_path, src_file_list[i], FileInfo(dst_find_data), overwrite, options.overwrite);
                }
              }
              else {
                WIN32_FIND_DATAW dst_find_data;
                HANDLE h_dst_find = FindFirstFileW(long_path(dst_path).data(), &dst_find_data);
                if (h_dst_find != INVALID_HANDLE_VALUE) {
                  VERIFY(FindClose(h_dst_find) != 0);
                  dst_file_exists = true;
                  dst_attr = dst_find_data.dwFileAttributes;
                  overwrite_decision(dst_path, src_file_list[i], FileInfo(dst_find_data), overwrite, options.overwrite);
                }
              }
              if (dst_file_exists && overwrite) {
                // reset destination file attributes if needed
                if ((dst_attr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) != 0) {
                  if (dst_remote) {
                    RAPI(CeSetFileAttributes(dst_path.data(), FILE_ATTRIBUTE_NORMAL));
                  }
                  else {
                    SetFileAttributesW(long_path(dst_path).data(), FILE_ATTRIBUTE_NORMAL);
                  }
                }
              }
              // create destination file
              if (!dst_file_exists || overwrite) {
                plugin->last_object = dst_path;
                HANDLE h_dst;
                BEGIN_RETRY_BLOCK;
                if (dst_remote) {
                  h_dst = RAPI(CeCreateFile(dst_path.data(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
                  CHECK_RAPI(h_dst != INVALID_HANDLE_VALUE);
                }
                else {
                  h_dst = CreateFileW(long_path(dst_path).data(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | (options.use_tmp_files ? FILE_ATTRIBUTE_TEMPORARY : 0), NULL);
                  CHECK_API(h_dst != INVALID_HANDLE_VALUE);
                }
                END_RETRY_BLOCK;
                try {
                  // copy file data
                  progress.copied_file_size = 0;
                  QueryPerformanceCounter((PLARGE_INTEGER) &progress.file_start_time);
                  DWORD size_read;
                  buffer.start_measurement();
                  while (true) {
                    // update progress bar
                    if (ui.update_needed()) {
                      progress.src_path = src_path;
                      progress.dst_path = dst_path;
                      progress.file_size = src_file_list[i].size;
                      draw_copy_files_progress(progress, stats, options.move_files);
                    }
                    // read block of data from source file
                    plugin->last_object = src_path;
                    BEGIN_RETRY_BLOCK;
                    if (src_remote) {
                      CHECK_RAPI(RAPI(CeReadFile(h_src, buffer.ptr(), buffer.size(), &size_read, NULL)) != 0);
                    }
                    else {
                      CHECK_API(ReadFile(h_src, buffer.ptr(), buffer.size(), &size_read, NULL) != 0);
                    }
                    END_RETRY_BLOCK;
                    if (size_read == 0) break;
                    // write block of data into destination file
                    plugin->last_object = dst_path;
                    DWORD size_written;
                    BEGIN_RETRY_BLOCK;
                    if (dst_remote) {
                      CHECK_RAPI(RAPI(CeWriteFile(h_dst, buffer.ptr(), size_read, &size_written, NULL)) != 0);
                    }
                    else {
                      CHECK_API(WriteFile(h_dst, buffer.ptr(), size_read, &size_written, NULL) != 0);
                    }
                    END_RETRY_BLOCK;
                    // update progress data
                    progress.copied_file_size += size_read;
                    progress.copied_total_size += size_read;
                    progress.processed_total_size += size_read;
                    // calc. buffer size
                    buffer.end_measurement(progress.copied_file_size);
                  }
                  // copy file date/time
                  plugin->last_object = dst_path;
                  BEGIN_RETRY_BLOCK;
                  if (dst_remote) {
                    CHECK_RAPI(RAPI(CeSetFileTime(h_dst, (FILETIME*) &src_file_list[i].creation_time, (FILETIME*) &src_file_list[i].access_time, (FILETIME*) &src_file_list[i].write_time)) != 0);
                  }
                  else {
                    CHECK_API(SetFileTime(h_dst, &src_file_list[i].creation_time, &src_file_list[i].access_time, &src_file_list[i].write_time) != 0);
                  }
                  END_RETRY_BLOCK;
                }
                catch (...) {
                  if (dst_remote) {
                    VERIFY(RAPI(CeCloseHandle(h_dst)) != 0);
                    // try to delete incomplete file
                    RAPI(CeDeleteFile(dst_path.data()));
                  }
                  else {
                    VERIFY(CloseHandle(h_dst) != 0);
                    // try to delete incomplete file
                    DeleteFileW(long_path(dst_path).data());
                  }
                  throw;
                }
                if (dst_remote) {
                  VERIFY(RAPI(CeCloseHandle(h_dst)) != 0);
                }
                else {
                  VERIFY(CloseHandle(h_dst) != 0);
                }
              }
            }
            finally (
              if (src_remote) {
                VERIFY(RAPI(CeCloseHandle(h_src)) != 0)
              }
              else {
                VERIFY(CloseHandle(h_src) != 0);
              }
            );
            // copy file attributes
            plugin->last_object = dst_path;
            BEGIN_RETRY_BLOCK;
            if (dst_remote) {
              CHECK_RAPI(RAPI(CeSetFileAttributes(dst_path.data(), src_file_list[i].attr & ATTR_MASK)) != 0);
            }
            else {
              CHECK_API(SetFileAttributesW(long_path(dst_path).data(), src_file_list[i].attr & ATTR_MASK) != 0);
            }
            END_RETRY_BLOCK;
          }
          finally (
            if (tmp_path.size() != 0) VERIFY(DeleteFileW(tmp_path.data()) != 0);
          );
        } // end of non-remote copy
        // update stats
        if (!dst_file_exists || overwrite) stats.files++;
        if (dst_file_exists && overwrite) stats.overwritten++;
        if (dst_file_exists && !overwrite) stats.skipped++;
      }
      catch (Error& e) {
        LOG_ERR(e);
        LOG_UMSG(plugin->last_object);
        if (ignore_errors || options.ignore_errors) {
          stats.errors++;
          log.add(lotCopyFile, plugin->last_object, e.message());
        }
        else throw;
      }
      // total progress bar is always updated
      progress.processed_total_size = progress_processed_total_size + src_file_list[i].size;
    } // end of file processing
    // finish directory processing
    while ((dir_stack.size() != 0) && (dir_stack[0] + src_file_list[dir_stack[0]].child_cnt == i)) {
      // update stats
      stats.dirs++;
      // remove last directory from current paths
      src_fp.remove(src_fp.size() - 1);
      dst_fp.remove(dst_fp.size() - 1);
      // remove processed directory from stack
      dir_stack.remove(0);
    }
  } // end src_file_list iteration
}

bool move_remote_file(const std::string& src_path, const std::string& dst_path, IRAPISession* session) {
  return RAPI(CeMoveFile(src_path.data(), dst_path.data())) != 0;
}

void delete_files(bool dst_remote, const std::string& dst_dir, const FileList& dst_file_list, DeleteFilesStats& stats, DeleteFilesProgress& progress, DeleteFilesOptions& options, UiLink& ui, Log& log, IRAPISession* session, PluginInstance* plugin) {
  Array<unsigned> dir_stack; // directory stack (prev. directory indices in src_file_list)
  FilePath dst_fp(dst_dir); // current destination path
  std::string dst_path;
  std::string dst_file_name;
  // start file list iteration
  for (unsigned i = 0; i < dst_file_list.size(); i++) {
    // calc. current file paths
    dst_file_name = dst_file_list[i].file_name;
    COMPOSE_PATH2(dst_path, dst_fp.get_full_path(), dst_file_name);
    // update progress bar
    if (ui.update_needed()) {
      progress.curr_path = dst_path;
      draw_delete_files_progress(progress, stats);
    }
    // reset READONLY attribute
    if ((dst_file_list[i].attr & FILE_ATTRIBUTE_READONLY) != 0) {
      if (dst_remote) {
        VERIFY(RAPI(CeSetFileAttributes(dst_path.data(), FILE_ATTRIBUTE_NORMAL)) != 0);
      }
      else {
        VERIFY(SetFileAttributesW(long_path(dst_path).data(), FILE_ATTRIBUTE_NORMAL) != 0);
      }
    }
    // start directory processing
    if (dst_file_list[i].is_dir()) {
      // include directory into current paths
      dst_fp += dst_file_name;
      // save directory into stack
      dir_stack.insert(0, i);
    }
    // process file
    else {
      bool ignore_errors = options.ignore_errors;
      try {
        // remove source file
        plugin->last_object = dst_path;
        BEGIN_RETRY_BLOCK;
        if (dst_remote) {
          CHECK_RAPI(RAPI(CeDeleteFile(dst_path.data())) != 0);
        }
        else {
          CHECK_API(DeleteFileW(long_path(dst_path).data()) != 0);
        }
        END_RETRY_BLOCK;
        stats.files++;
      }
      catch (Error& e) {
        LOG_ERR(e);
        LOG_UMSG(plugin->last_object);
        if (ignore_errors || options.ignore_errors) {
          stats.errors++;
          log.add(lotDeleteFile, plugin->last_object, e.message());
        }
        else throw;
      }
      progress.objects++;
    } // end of file processing
    // finish directory processing
    while ((dir_stack.size() != 0) && (dir_stack[0] + dst_file_list[dir_stack[0]].child_cnt == i)) {
      bool ignore_errors = options.ignore_errors;
      try {
        // delete directory
        dst_path = dst_fp.get_full_path();
        plugin->last_object = dst_path;
        BEGIN_RETRY_BLOCK;
        if (dst_remote) {
          CHECK_RAPI(RAPI(CeRemoveDirectory(dst_path.data())) != 0);
        }
        else {
          CHECK_API(RemoveDirectoryW(long_path(dst_path).data()) != 0);
        }
        END_RETRY_BLOCK;
        // update stats
        stats.dirs++;
      }
      catch (Error& e) {
        LOG_ERR(e);
        LOG_UMSG(plugin->last_object);
        if (ignore_errors || options.ignore_errors) {
          stats.errors++;
          log.add(lotDeleteDir, plugin->last_object, e.message());
        }
        else throw;
      }
      progress.objects++;
      // remove last directory from current paths
      dst_fp.remove(dst_fp.size() - 1);
      // remove processed directory from stack
      dir_stack.remove(0);
    }
  } // end dst_file_list iteration
}

void create_dir(const std::string& dst_dir, const std::string& dst_file_name, IRAPISession* session, PluginInstance* plugin) {
  FilePath fp_dst_path(dst_dir);
  fp_dst_path.combine(dst_file_name);
  for (unsigned i = 1; i <= fp_dst_path.size(); i++) {
    std::string partial_path = fp_dst_path.get_partial_path(i);
    if (!dir_exists(partial_path, session)) {
      plugin->last_object = partial_path;
      CHECK_RAPI(RAPI(CeCreateDirectory(partial_path.data(), NULL)) != 0);
    }
  }
}

void set_file_attr(const std::string& file_path, DWORD attr, IRAPISession* session, PluginInstance* plugin) {
  plugin->last_object = file_path;
  CHECK_RAPI(RAPI(CeSetFileAttributes(file_path.data(), attr)) != 0);
}

void find_real_file_path(FilePath& fp, IRAPISession* session) {
  CE_FIND_DATA find_data;
  for (unsigned i = 0; i < fp.size(); i++) {
    HANDLE h_find = RAPI(CeFindFirstFile(fp.get_partial_path(i + 1).data(), &find_data));
    if (h_find != INVALID_HANDLE_VALUE) {
      VERIFY(RAPI(CeFindClose(h_find)) != 0);
      fp.item(i) = find_data.cFileName;
    }
  }
}

// check if directory exists
bool dir_exists(const std::string& path) {
  WIN32_FIND_DATAW find_data;
  HANDLE h_find = FindFirstFileW(long_path(path).data(), &find_data);
  if (h_find == INVALID_HANDLE_VALUE) return false;
  VERIFY(FindClose(h_find) != 0);
  return IS_DIR(find_data);
}

bool dir_exists(const std::string& path, IRAPISession* session) {
  CE_FIND_DATA find_data;
  HANDLE h_find = RAPI(CeFindFirstFile(path.data(), &find_data));
  if (h_find == INVALID_HANDLE_VALUE) return false;
  VERIFY(RAPI(CeFindClose(h_find)) != 0);
  return IS_DIR(find_data);
}

void InfoPanel::clear() {
  Array<InfoPanelLine>::clear();
  info_lines.clear();
}

void InfoPanel::add_separator(const std::string& text) {
  InfoPanelLine ipl;
  memset(&ipl, 0, sizeof(ipl));
  info_lines += text;
  ipl.Text = info_lines.last().data();
  ipl.Flags = IPLFLAGS_SEPARATOR;
  add(ipl);
}

void InfoPanel::add_info(const std::string& name, const std::string& value) {
  InfoPanelLine ipl;
  memset(&ipl, 0, sizeof(ipl));
  info_lines += name;
  ipl.Text = info_lines.last().data();
  info_lines += value;
  ipl.Data = info_lines.last().data();
  add(ipl);
}

void refresh_system_info(PluginInstance* plugin) {
  IRAPISession*& session = plugin->session;

  // current directory free space
  ULARGE_INTEGER free_bytes;
  unsigned __int64 total_bytes;
  unsigned __int64 total_free_bytes;
  if  (RAPI(CeGetDiskFreeSpaceEx(plugin->current_dir.data(), &free_bytes, (ULARGE_INTEGER*) &total_bytes, (ULARGE_INTEGER*) &total_free_bytes)) != 0) {
    plugin->free_space = total_free_bytes;
  }
  else {
    plugin->free_space = 0;
  }

  InfoPanel& sys_info = plugin->sys_info;
  sys_info.clear();

  // OS version
  CEOSVERSIONINFO os_ver;
  os_ver.dwOSVersionInfoSize = sizeof(CEOSVERSIONINFO);
  if (RAPI(CeGetVersionEx(&os_ver)) != 0) {
    sys_info.add_separator(far_get_msg(MSG_SYSINFO_DEVICE_INFO));
    sys_info.add_info(far_get_msg(MSG_SYSINFO_OS_VERSION), std::string::format(L"%u.%u.%u", os_ver.dwMajorVersion, os_ver.dwMinorVersion, os_ver.dwBuildNumber).data());
  }

  // memory info
  MEMORYSTATUS mem;
  mem.dwLength = sizeof(MEMORYSTATUS);
  RAPI(CeGlobalMemoryStatus(&mem));
  if (SUCCEEDED(RAPI(CeRapiGetError()))) {
    sys_info.add_separator(far_get_msg(MSG_SYSINFO_PROGRAM_MEMORY));
    sys_info.add_info(far_get_msg(MSG_SYSINFO_TOTAL), format_data_size(mem.dwTotalPhys, get_size_suffixes()).data());
    sys_info.add_info(far_get_msg(MSG_SYSINFO_FREE), format_data_size(mem.dwAvailPhys, get_size_suffixes()).data());
  }

  // disk info
  if (RAPI(CeGetDiskFreeSpaceEx(L"\\", &free_bytes, (ULARGE_INTEGER*) &total_bytes, (ULARGE_INTEGER*) &total_free_bytes)) != 0) {
    sys_info.add_separator(far_get_msg(MSG_SYSINFO_STORAGE_MEMORY));
    sys_info.add_info(far_get_msg(MSG_SYSINFO_TOTAL), format_data_size(total_bytes, get_size_suffixes()).data());
    sys_info.add_info(far_get_msg(MSG_SYSINFO_FREE), format_data_size(total_free_bytes, get_size_suffixes()).data());
  }

  DWORD file_count;
  CE_FIND_DATA* find_data;
  std::vector<std::string> mount_points;
  CHECK_RAPI(RAPI(CeFindAllFiles(L"\\*", FAF_ATTRIBUTES | FAF_NAME, &file_count, &find_data)) != 0);
  try {
    for (unsigned i = 0; i < file_count; i++) {
      if ((find_data[i].dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY)) == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY)) {
        mount_points += find_data[i].cFileName;
      }
    }
  }
  finally (VERIFY(SUCCEEDED(RAPI(CeRapiFreeBuffer(find_data)))));
  for (unsigned i = 0; i < mount_points.size(); i++) {
    if (RAPI(CeGetDiskFreeSpaceEx((L"\\" + mount_points[i]).data(), &free_bytes, (ULARGE_INTEGER*) &total_bytes, (ULARGE_INTEGER*) &total_free_bytes)) != 0) {
      if (mount_points.size() == 1) {
        sys_info.add_separator(far_get_msg(MSG_SYSINFO_STORAGE_CARD));
      }
      else {
        sys_info.add_separator(mount_points[i].data());
      }
      sys_info.add_info(far_get_msg(MSG_SYSINFO_TOTAL), format_data_size(total_bytes, get_size_suffixes()).data());
      sys_info.add_info(far_get_msg(MSG_SYSINFO_FREE), format_data_size(total_free_bytes, get_size_suffixes()).data());
    }
  }

  // battery
  SYSTEM_POWER_STATUS_EX power_status;
  if (RAPI(CeGetSystemPowerStatusEx(&power_status, TRUE)) != 0) {
    if (power_status.ACLineStatus != AC_LINE_UNKNOWN) {
      sys_info.add_separator(far_get_msg(MSG_SYSINFO_POWER));
      std::string power_type;
      if (power_status.ACLineStatus == AC_LINE_ONLINE) power_type = far_get_msg(MSG_SYSINFO_POWER_ONLINE);
      else if (power_status.ACLineStatus == AC_LINE_OFFLINE) power_type = far_get_msg(MSG_SYSINFO_POWER_OFFLINE);
      else if (power_status.ACLineStatus == AC_LINE_BACKUP_POWER) power_type = far_get_msg(MSG_SYSINFO_POWER_BACKUP);
      sys_info.add_info(far_get_msg(MSG_SYSINFO_POWER_TYPE), power_type);
      if (power_status.BatteryLifePercent != BATTERY_PERCENTAGE_UNKNOWN) {
        sys_info.add_info(far_get_msg(MSG_SYSINFO_POWER_BATTERY_LEVEL), std::string::format(L"%u%%", power_status.BatteryLifePercent).data());
      }
      if (power_status.BackupBatteryLifePercent != BATTERY_PERCENTAGE_UNKNOWN) {
        sys_info.add_info(far_get_msg(MSG_SYSINFO_POWER_BACKUP_BATTERY_LEVEL), std::string::format(L"%u%%", power_status.BackupBatteryLifePercent).data());
      }
    }
  }
}

std::string get_open_command(const std::string& file_dir, const std::string& file_name, IRAPISession* session) {
  std::string cmd;
  // extract file extension
  unsigned pos = file_name.rsearch(L'.');
  if (pos == -1) FAIL(CustomError(L""));
  std::string file_ext = file_name.right(file_name.size() - pos);
  // open HKEY_CLASSES_ROOT\.ext key and read File Type ID
  std::string file_type_id;
  HKEY h_key;
  CHECK_ADVAPI(RAPI(CeRegOpenKeyEx(HKEY_CLASSES_ROOT, file_ext.data(), 0, 0, &h_key)));
  try {
    DWORD type;
    DWORD value_size = 256 * sizeof(wchar_t);
    CHECK_ADVAPI(RAPI(CeRegQueryValueEx(h_key, NULL, NULL, &type, (LPBYTE) file_type_id.buf(value_size / sizeof(wchar_t)), &value_size)));
    if (type != REG_SZ) FAIL(CustomError(L""));
    file_type_id.set_size(value_size / sizeof(wchar_t) - 1);
  }
  finally (VERIFY(RAPI(CeRegCloseKey(h_key)) == ERROR_SUCCESS));
  // open HKEY_CLASSES_ROOT\FileTypeID\shell\open\command and read command line
  HKEY h_key_tmp1;
  CHECK_ADVAPI(RAPI(CeRegOpenKeyEx(HKEY_CLASSES_ROOT, (file_type_id).data(), 0, 0, &h_key_tmp1)));
  try {
    HKEY h_key_tmp2;
    CHECK_ADVAPI(RAPI(CeRegOpenKeyEx(h_key_tmp1, L"shell", 0, 0, &h_key_tmp2)));
    try {
      HKEY h_key_tmp3;
      CHECK_ADVAPI(RAPI(CeRegOpenKeyEx(h_key_tmp2, L"open", 0, 0, &h_key_tmp3)));
      try {
        CHECK_ADVAPI(RAPI(CeRegOpenKeyEx(h_key_tmp3, L"command", 0, 0, &h_key)));
        try {
          DWORD type;
          DWORD value_size = 1024 * sizeof(wchar_t);
          CHECK_ADVAPI(RAPI(CeRegQueryValueEx(h_key, NULL, NULL, &type, (LPBYTE) cmd.buf(value_size / sizeof(wchar_t)), &value_size)));
          if (type != REG_SZ) FAIL(CustomError(L""));
          cmd.set_size(value_size / sizeof(wchar_t) - 1);
        }
        finally (VERIFY(RAPI(CeRegCloseKey(h_key)) == ERROR_SUCCESS));
      }
      finally (VERIFY(RAPI(CeRegCloseKey(h_key_tmp3)) == ERROR_SUCCESS));
    }
    finally (VERIFY(RAPI(CeRegCloseKey(h_key_tmp2)) == ERROR_SUCCESS));
  }
  finally (VERIFY(RAPI(CeRegCloseKey(h_key_tmp1)) == ERROR_SUCCESS));
  // replace %1 with file path; remove %*
  pos = cmd.search(L"%1");
  if (pos != -1) cmd.replace(pos, 2, COMPOSE_PATH(file_dir, file_name));
  pos = cmd.search(L"%*");
  if (pos != -1) cmd.remove(pos, 2);
  cmd.strip();
  if (cmd.size() == 0) FAIL(CustomError(L""));
  return cmd;
}

void create_process(const std::string& app_name, const std::string& params, IRAPISession* session) {
  PROCESS_INFORMATION pi;
  CHECK_RAPI(RAPI(CeCreateProcess(app_name.data(), params.data(), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi)) != 0);
  VERIFY(RAPI(CeCloseHandle(pi.hThread)) != 0);
  VERIFY(RAPI(CeCloseHandle(pi.hProcess)) != 0);
}

void panel_items_to_file_list(PluginPanelItem *PanelItem, size_t ItemsNumber, FileList& panel_file_list) {
  panel_file_list.extend(ItemsNumber);
  for (unsigned i = 0; i < ItemsNumber; i++) {
    panel_file_list += FileInfo(PanelItem[i]);
  }
}

void file_panel_items_to_file_list(const std::string& panel_path, PluginPanelItem *PanelItem, size_t ItemsNumber, PanelFileList& panel_file_list, UiLink& ui, PluginInstance* plugin) {
  panel_file_list.extend(ItemsNumber);
  PanelFileInfo fi;
  std::string file_name, file_path;
  WIN32_FIND_DATAW find_data;
  for (unsigned i = 0; i < ItemsNumber; i++) {
    if (ui.update_needed()) {
      draw_progress_msg(far_get_msg(MSG_PROGRESS_PREPARE));
    }
    FilePath fp(PanelItem[i].FileName);
    fi.file_dir = fp.get_dir_path();
    file_name = fp.get_file_name();
    if (!fp.is_absolute) {
      if (fi.file_dir.size() == 0) fi.file_dir = panel_path;
      else fi.file_dir.insert(0, L'\\').insert(0, panel_path);
    }
    COMPOSE_PATH2(file_path, fi.file_dir, file_name);
    plugin->last_object = file_path;
    HANDLE h_find = FindFirstFileW(long_path(file_path).data(), &find_data);
    if (h_find != INVALID_HANDLE_VALUE) {
      VERIFY(FindClose(h_find) != 0);
      fi.file_name = find_data.cFileName;
      fi.attr = find_data.dwFileAttributes;
      fi.creation_time = find_data.ftCreationTime;
      fi.access_time = find_data.ftLastAccessTime;
      fi.write_time = find_data.ftLastWriteTime;
      fi.size = FILE_SIZE(find_data);
    }
    else {
      fi.file_name = file_name;
      fi.attr = PanelItem[i].FileAttributes;
      fi.creation_time = PanelItem[i].CreationTime;
      fi.access_time = PanelItem[i].LastAccessTime;
      fi.write_time = PanelItem[i].LastWriteTime;
      fi.size = PanelItem[i].FileSize;
    }
    fi.child_cnt = 0;
    panel_file_list += fi;
  }
}
