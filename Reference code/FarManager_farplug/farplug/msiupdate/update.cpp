#include "msg.h"
#include "guids.hpp"
#include "utils.hpp"
#include "farutils.hpp"
#include "sysutils.hpp"
#include "iniparse.hpp"
#include "utils.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "inet.hpp"
#include "trayicon.hpp"
#include "options.hpp"
#include "update.hpp"

namespace Update {

const wchar_t* c_param_last_check_time = L"last_check_time";
const wchar_t* c_param_last_check_version = L"last_check_version";
#ifdef _M_IX86
const wchar_t* c_upgrade_code = L"{795A974D-CE83-476D-9C1F-FC59F8C60709}";
const char* c_platform = "x86";
const wchar_t* c_update_script = L"update3.php?p=32";
#endif
#ifdef _M_X64
const wchar_t* c_upgrade_code = L"{A37EC11A-94EE-41FC-B116-E6FF79545B70}";
const char* c_platform = "x64";
const wchar_t* c_update_script = L"update3.php?p=64";
#endif
const wchar_t* c_changelog_url = L"http://farmanager.com/svn/trunk/unicode_far/changelog";
const unsigned c_exit_wait = 6;
const unsigned c_update_period = 12 * 60 * 60;
const wchar_t* c_param_changelog_path = L"changelog_path";

VersionInfo current_version;

struct UpdateInfo {
  VersionInfo version;
  wstring package_name;
};

UpdateInfo parse_update_info(const wstring& text) {
  Ini::File update_ini;
  update_ini.parse(text);
  wstring ver_major = update_ini.get(L"far", L"major");
  wstring ver_minor = update_ini.get(L"far", L"minor");
  wstring ver_build = update_ini.get(L"far", L"build");
  UpdateInfo update_info;
  update_info.version = current_version;
  update_info.version.Major = str_to_int(ver_major);
  update_info.version.Minor = str_to_int(ver_minor);
  update_info.version.Build = str_to_int(ver_build);
  update_info.package_name = update_ini.get(L"far", L"msi");
  return update_info;
}

void check_product_installed() {
  wchar_t product_guid[39];
  if (MsiEnumRelatedProducts(c_upgrade_code, 0, 0, product_guid) != ERROR_SUCCESS)
    FAIL_MSG(Far::get_msg(MSG_ERROR_NO_MSI));
}

wstring get_update_url() {
  wstring update_url = L"http://www.farmanager.com/";
  if (g_options.update_stable_builds)
    update_url += L"files/";
  else
    update_url += L"nightly/";
  return update_url;
}

wstring save_changelog(unsigned build1, unsigned build2) {
  wstring file_path;
  Far::Settings plugin_key;
  if (!plugin_key.create())
    return file_path;
  if (!plugin_key.get(c_param_changelog_path, file_path))
    file_path = add_trailing_slash(get_temp_path()) + create_guid() + L".txt";

  wstring text = ansi_to_unicode(load_url(c_changelog_url, g_options.http), 1251);
  const wchar_t* c_expr_prefix = L"[^\\s]+\\s+\\d+\\.\\d+\\.\\d+\\s+\\d+:\\d+:\\d+\\s+[+-]?\\d+\\s+-\\s+build\\s+";
  const wchar_t* c_expr_suffix = L"\\s*";
  Far::Regex regex;
  size_t pos1 = regex.search(c_expr_prefix + int_to_str(build1) + c_expr_suffix, text);
  if (pos1 == -1)
    pos1 = text.size();
  size_t pos2 = regex.search(c_expr_prefix + int_to_str(build2) + c_expr_suffix, text);
  if (pos2 == -1)
    pos2 = 0;
  CHECK(pos1 > pos2);

  File file(file_path, FILE_WRITE_DATA, FILE_SHARE_READ, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
  const wchar_t sig = 0xFEFF;
  file.write(&sig, sizeof(sig));
  file.write(text.data() + pos2, static_cast<unsigned>((pos1 - pos2) * sizeof(wchar_t)));

  plugin_key.set(c_param_changelog_path, file_path);

  return file_path;
}

void delete_changelog() {
  wstring file_path;
  Far::Settings plugin_key;
  if (plugin_key.get(c_param_changelog_path, file_path)) {
    if (File::exists(file_path))
      File::delete_file(file_path);
    plugin_key.del(c_param_changelog_path);
  }
}

const GUID c_update_dialog_guid = { /* BBE496E8-B5A8-48AC-B0D1-2141951B2C80 */
  0xBBE496E8,
  0xB5A8,
  0x48AC,
  {0xB0, 0xD1, 0x21, 0x41, 0x95, 0x1B, 0x2C, 0x80}
};

enum UpdateDialogResult {
  udrYes,
  udrNo,
  udrCancel,
};

class UpdateDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 40
  };

  const UpdateInfo& update_info;

  int changelog_ctrl_id;
  int yes_ctrl_id;
  int no_ctrl_id;
  int cancel_ctrl_id;

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) {
    if (msg == DN_INITDIALOG) {
      set_focus(yes_ctrl_id);
      return TRUE;
    }
    else if ((msg == DN_BTNCLICK) && (param1 == changelog_ctrl_id)) {
      wstring changelog_path = save_changelog(current_version.Build, update_info.version.Build);
      Far::viewer(changelog_path, L"Changelog", VF_DISABLEHISTORY | VF_ENABLE_F6 | VF_DELETEONLYFILEONCLOSE);
      Far::flush_screen();
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  UpdateDialog(const UpdateInfo& update_info): Far::Dialog(Far::get_msg(MSG_PLUGIN_NAME), &c_update_dialog_guid, c_client_xs, L"update"), update_info(update_info) {
  }

  UpdateDialogResult show() {
    wostringstream st;
    st << Far::get_msg(MSG_UPDATE_NEW_VERSION) << L' ' << update_info.version.Major << L'.' << update_info.version.Minor << L'.' << update_info.version.Build;
    label(st.str());
    new_line();
    label(Far::get_msg(MSG_UPDATE_QUESTION));
    new_line();
    changelog_ctrl_id = button(Far::get_msg(MSG_UPDATE_CHANGELOG), DIF_BTNNOCLOSE);
    new_line();
    separator();
    new_line();
    yes_ctrl_id = def_button(Far::get_msg(MSG_BUTTON_YES), DIF_CENTERGROUP);
    no_ctrl_id = button(Far::get_msg(MSG_BUTTON_NO), DIF_CENTERGROUP);
    cancel_ctrl_id = button(Far::get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
    new_line();

    int item = Far::Dialog::show();
    if (item == yes_ctrl_id) return udrYes;
    else if (item == no_ctrl_id) return udrNo;
    else return udrCancel;
  }
};

void prepare_directory(const wstring& dir) {
  if (GetFileAttributesW(dir.c_str()) == INVALID_FILE_ATTRIBUTES) {
    if (!is_root_path(dir)) {
      prepare_directory(extract_file_path(dir));
      CHECK_SYS(CreateDirectoryW(dir.c_str(), NULL));
    }
  }
}

void save_to_cache(const string& package, const wstring& cache_dir, const wstring& package_name) {
  prepare_directory(cache_dir);
  const wchar_t* c_param_cache_index = L"cache_index";
  Far::Settings plugin_key;

  list<wstring> cache_index;
  wstring cache_index_str;
  if (plugin_key.get(c_param_cache_index, cache_index_str))
    cache_index = split(cache_index_str, L'\n');

  while (cache_index.size() && cache_index.size() >= g_options.cache_max_size) {
    File::delete_file(add_trailing_slash(cache_dir) + cache_index.front());
    cache_index.erase(cache_index.begin());
  }

  File package_file((add_trailing_slash(cache_dir) + package_name).c_str(), GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
  package_file.write(package.data(), static_cast<unsigned>(package.size()));

  cache_index.push_back(package_name);
  plugin_key.set(c_param_cache_index, combine(cache_index, L'\n'));
}

void execute(bool ask) {
  check_product_installed();
  string update_url_text = load_url(get_update_url() + c_update_script, g_options.http);
  UpdateInfo update_info = parse_update_info(ansi_to_unicode(update_url_text, CP_ACP));
  if (Far::compare_versions(update_info.version, current_version) <= 0) {
    Far::info_dlg(c_no_new_version_dialog_guid, Far::get_msg(MSG_PLUGIN_NAME), Far::get_msg(MSG_UPDATE_NO_NEW_VERSION));
    return;
  }
  UpdateDialogResult res = ask ? UpdateDialog(update_info).show() : udrYes;
  if (res == udrYes) {
    if (!ask && g_options.open_changelog) {
      wstring changelog_path;
      IGNORE_ERRORS(changelog_path = save_changelog(current_version.Build, update_info.version.Build))
      if (!changelog_path.empty()) {
        SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFO) };
        sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE | SEE_MASK_ASYNCOK;
        sei.lpFile = changelog_path.c_str();
        sei.nShow = SW_SHOWDEFAULT;
        ShellExecuteExW(&sei);
      }
    }

    wstring cache_dir;
    if (g_options.cache_enabled) {
      cache_dir = expand_env_vars(g_options.cache_dir);
      string package = load_url(get_update_url() + update_info.package_name, g_options.http);
      save_to_cache(package, cache_dir, update_info.package_name);
    }

    wostringstream st;
    st << L"msiexec /promptrestart ";
    if (!g_options.use_full_install_ui) {
      st << L"/qr ";
    }
    if (g_options.logged_install) {
      wchar_t temp_path[MAX_PATH];
      CHECK_SYS(GetTempPathW(ARRAYSIZE(temp_path), temp_path));
      st << L"/log \"" << add_trailing_slash(temp_path) << L"MsiUpdate_" << current_version.Major << L"_" << widen(c_platform) << L".log\" ";
    }
    if (g_options.cache_enabled)
      st << "/i \"" << add_trailing_slash(cache_dir) + update_info.package_name + L"\"";
    else
      st << "/i \"" << get_update_url() + update_info.package_name + L"\"";
    if (!g_options.install_properties.empty())
      st << L" " << g_options.install_properties;
    wstring command = st.str();

    STARTUPINFOW si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    CHECK_SYS(CreateProcessW(NULL, const_cast<LPWSTR>(command.c_str()), NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi));
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    Far::quit();
  }
  else if (res == udrNo) {
    Far::Settings settings;
    if (settings.create())
      settings.set(c_param_last_check_version, Far::version_to_string(update_info.version));
  }
}

class AutoUpdate: public Thread, public Event {
private:
  wstring update_url;
  HttpOptions http_options;
  wstring window_name;
  wstring msg_tray_title;
  wstring msg_tray_version;
  wstring msg_tray_update;
  virtual void run() {
    struct Clean {
      ~Clean() {
        Far::call_user_apc(reinterpret_cast<void*>(cmdClean));
      }
    };
    Clean clean;
    string update_url_text = load_url(update_url, http_options, h_event);
    UpdateInfo update_info = parse_update_info(ansi_to_unicode(update_url_text, CP_ACP));
    wstring last_check_version;
    Far::Settings settings;
    CHECK(settings.create())
    if (!settings.get(c_param_last_check_version, last_check_version))
      last_check_version = L"0.0.0";
    if ((Far::compare_versions(update_info.version, current_version) > 0) && (Far::compare_versions(update_info.version, Far::string_to_version(last_check_version)) > 0)) {
      wostringstream st;
      st << msg_tray_version << L' ' << update_info.version.Major << L'.' << update_info.version.Minor << L'.' << update_info.version.Build << L'\n' << msg_tray_update;
      TrayIcon tray_icon(window_name, msg_tray_title, st.str());
      if (tray_icon.message_loop(h_event))
        Far::call_user_apc(reinterpret_cast<void*>(cmdExecute));
    }
  }
public:
  AutoUpdate(const wstring& update_url, const HttpOptions& http_options): Event(true, false), update_url(update_url), http_options(http_options), window_name(Far::get_msg(MSG_PLUGIN_NAME)), msg_tray_title(Far::get_msg(MSG_TRAY_TITLE)), msg_tray_version(Far::get_msg(MSG_TRAY_VERSION)), msg_tray_update(Far::get_msg(MSG_TRAY_UPDATE)) {
  }
  ~AutoUpdate() {
    set();
  }
};

AutoUpdate* g_auto_update = NULL;

void init() {
  current_version = Far::get_version();

  IGNORE_ERRORS(delete_changelog());

  check_product_installed();

  time_t curr_time = time(NULL);
  CHECK(curr_time != -1);
  unsigned __int64 last_check_time;
  Far::Settings settings;
  CHECK(settings.create());
  if (!settings.get(c_param_last_check_time, last_check_time))
      last_check_time = 0;
  if (fabs(static_cast<double>(curr_time) - last_check_time) < c_update_period)
    return;
  settings.set(c_param_last_check_time, curr_time);

  g_auto_update = new AutoUpdate(get_update_url() + c_update_script, g_options.http);
  try {
    g_auto_update->start();
  }
  catch (...) {
    delete g_auto_update;
    g_auto_update = NULL;
    throw;
  }
}

void clean() {
  if (g_auto_update) {
    delete g_auto_update;
    g_auto_update = NULL;
  }
}

}
