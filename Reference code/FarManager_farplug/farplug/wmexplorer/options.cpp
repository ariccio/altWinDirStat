#define _ERROR_WINDOWS
#include "error.h"
#include "guids.h"
#include "util.h"
#include "options.h"

extern struct PluginStartupInfo g_far;
extern struct FarStandardFunctions g_fsf;

class Options {
private:
  HANDLE handle;

private:
  INT_PTR control(FAR_SETTINGS_CONTROL_COMMANDS command, void* param = nullptr) {
    return g_far.SettingsControl(handle, command, 0, param);
  }

  void clean() {
    if (handle != INVALID_HANDLE_VALUE) {
      control(SCTL_FREE);
      handle = INVALID_HANDLE_VALUE;
    }
  }

protected:
  bool set(const wchar_t* name, unsigned __int64 value) {
    FarSettingsItem fsi = { sizeof(FarSettingsItem) };
    fsi.Root = 0;
    fsi.Name = name;
    fsi.Type = FST_QWORD;
    fsi.Number = value;
    return control(SCTL_SET, &fsi) != 0;
  }

  bool set(const wchar_t* name, const std::string& value) {
    FarSettingsItem fsi = { sizeof(FarSettingsItem) };
    fsi.Root = 0;
    fsi.Name = name;
    fsi.Type = FST_STRING;
    fsi.String = value.data();
    return control(SCTL_SET, &fsi) != 0;
  }

  bool get(size_t root, const wchar_t* name, unsigned __int64& value) {
    FarSettingsItem fsi = { sizeof(FarSettingsItem) };
    fsi.Root = root;
    fsi.Name = name;
    fsi.Type = FST_QWORD;
    if (!control(SCTL_GET, &fsi))
      return false;
    value = fsi.Number;
    return true;
  }

  bool get(size_t root, const wchar_t* name, std::string& value) {
    FarSettingsItem fsi = { sizeof(FarSettingsItem) };
    fsi.Root = root;
    fsi.Name = name;
    fsi.Type = FST_STRING;
    if (!control(SCTL_GET, &fsi))
      return false;
    value = fsi.String;
    return true;
  }

  bool del(const wchar_t* name) {
    FarSettingsValue fsv = { sizeof(FarSettingsValue) };
    fsv.Root = 0;
    fsv.Value = name;
    return control(SCTL_DELETE, &fsv) != 0;
  }

public:
  Options(): handle(INVALID_HANDLE_VALUE) {
  }

  ~Options() {
    clean();
  }

  bool create(bool app_settings = false) {
    clean();
    FarSettingsCreate fsc = { sizeof(FarSettingsCreate) };
    fsc.Guid = app_settings ? c_far_guid : c_plugin_guid;
    if (!control(SCTL_CREATE, &fsc))
      return false;
    handle = fsc.Handle;
    return true;
  }

  template<class Integer>
  Integer get_int(size_t root, const wchar_t* name, Integer def_value) {
    unsigned __int64 value;
    if (get(root, name, value))
      return static_cast<Integer>(value);
    else
      return def_value;
  }

  bool get_bool(size_t root, const wchar_t* name, bool def_value) {
    unsigned __int64 value;
    if (get(root, name, value))
      return value != 0;
    else
      return def_value;
  }

  std::string get_str(size_t root, const wchar_t* name, const std::string& def_value) {
    std::string value;
    if (get(root, name, value))
      return value;
    else
      return def_value;
  }

  template<class Integer>
  Integer get_int(const wchar_t* name, Integer def_value) {
    return get_int(0, name, def_value);
  }

  bool get_bool(const wchar_t* name, bool def_value) {
    return get_bool(0, name, def_value);
  }

  std::string get_str(const wchar_t* name, const std::string& def_value) {
    return get_str(0, name, def_value);
  }

  void set_int(const wchar_t* name, unsigned value, unsigned def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value);
  }

  void set_bool(const wchar_t* name, bool value, bool def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value ? 1 : 0);
  }

  void set_str(const wchar_t* name, const std::string& value, const std::string& def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value);
  }
};

PluginOptions::PluginOptions():
  add_to_plugin_menu(true),
  add_to_disk_menu(true),
  access_method(amRapi2),
  hide_copy_dlg(false),
  save_last_dir(false),
  copy_buf_size(-1),
  hide_rom_files(false),
  exit_on_dot_dot(true),
  prefix(L"wme"),
  show_free_space(true),
  ignore_errors(false),
  overwrite(ooAsk),
  show_stats(ssoIfError),
  use_file_filters(false),
  save_def_values(true),
  last_dev_type(dtPDA),
  key_attr(L"CtrlShiftA"),
  key_execute(L"CtrlShiftS"),
  key_hide_rom_files(L"CtrlShiftI") {
}

void load_plugin_options(PluginOptions& plugin_options) {
  Options options;
  if (!options.create())
    return;
  PluginOptions defaults;
  plugin_options.add_to_plugin_menu = options.get_bool(L"add_to_plugin_menu", defaults.add_to_plugin_menu);
  plugin_options.add_to_disk_menu = options.get_bool(L"add_to_disk_menu", defaults.add_to_disk_menu);
  plugin_options.access_method = (AccessMethod) options.get_int(L"access_method", defaults.access_method);
  plugin_options.hide_copy_dlg = options.get_bool(L"hide_copy_dlg", defaults.hide_copy_dlg);
  plugin_options.save_last_dir = options.get_bool(L"save_last_dir", defaults.save_last_dir);
  plugin_options.copy_buf_size = options.get_int(L"copy_buf_size", defaults.copy_buf_size);
  plugin_options.hide_rom_files = options.get_bool(L"hide_rom_files", defaults.hide_rom_files);
  plugin_options.exit_on_dot_dot = options.get_bool(L"exit_on_dot_dot", defaults.exit_on_dot_dot);
  plugin_options.prefix = options.get_str(L"prefix", defaults.prefix);
  plugin_options.show_free_space = options.get_bool(L"show_free_space", defaults.show_free_space);
  plugin_options.ignore_errors = options.get_bool(L"ignore_errors", defaults.ignore_errors);
  plugin_options.overwrite = (OverwriteOption) options.get_int(L"overwrite", defaults.overwrite);
  plugin_options.show_stats = (ShowStatsOption) options.get_int(L"show_op_stats", defaults.show_stats);
  plugin_options.use_file_filters = options.get_bool(L"use_file_filters", defaults.use_file_filters);
  plugin_options.save_def_values = options.get_bool(L"save_def_values", defaults.save_def_values);
  plugin_options.last_dev_type = (DevType) options.get_int(L"last_dev_type", defaults.last_dev_type);
  plugin_options.key_attr = options.get_str(L"key_attr", defaults.key_attr);
  plugin_options.key_execute = options.get_str(L"key_execute", defaults.key_execute);
  plugin_options.key_hide_rom_files = options.get_str(L"key_hide_rom_files", defaults.key_hide_rom_files);
}

void save_plugin_options(const PluginOptions& plugin_options) {
  Options options;
  if (!options.create())
    return;
  PluginOptions defaults;
  options.set_bool(L"add_to_plugin_menu", plugin_options.add_to_plugin_menu, defaults.add_to_plugin_menu);
  options.set_bool(L"add_to_disk_menu", plugin_options.add_to_disk_menu, defaults.add_to_disk_menu);
  options.set_int(L"access_method", plugin_options.access_method, defaults.access_method);
  options.set_int(L"hide_copy_dlg", plugin_options.hide_copy_dlg, defaults.hide_copy_dlg);
  options.set_bool(L"save_last_dir", plugin_options.save_last_dir, defaults.save_last_dir);
  options.set_int(L"copy_buf_size", plugin_options.copy_buf_size, defaults.copy_buf_size);
  options.set_bool(L"hide_rom_files", plugin_options.hide_rom_files, defaults.hide_rom_files);
  options.set_bool(L"exit_on_dot_dot", plugin_options.exit_on_dot_dot, defaults.exit_on_dot_dot);
  options.set_str(L"prefix", plugin_options.prefix, defaults.prefix);
  options.set_bool(L"show_free_space", plugin_options.show_free_space, defaults.show_free_space);
  options.set_bool(L"ignore_errors", plugin_options.ignore_errors, defaults.ignore_errors);
  options.set_int(L"overwrite", plugin_options.overwrite, defaults.overwrite);
  options.set_int(L"show_op_stats", plugin_options.show_stats, defaults.show_stats);
  options.set_bool(L"use_file_filters", plugin_options.use_file_filters, defaults.use_file_filters);
  options.set_bool(L"save_def_values", plugin_options.save_def_values, defaults.save_def_values);
  options.set_int(L"last_dev_type", plugin_options.last_dev_type, defaults.last_dev_type);
  options.set_str(L"key_attr", plugin_options.key_attr, defaults.key_attr);
  options.set_str(L"key_execute", plugin_options.key_execute, defaults.key_execute);
  options.set_str(L"key_hide_rom_files", plugin_options.key_hide_rom_files, defaults.key_hide_rom_files);
}

void save_def_option_values(const PluginOptions& plugin_options) {
  Options options;
  if (!options.create())
    return;
  PluginOptions defaults;
  options.set_bool(L"ignore_errors", plugin_options.ignore_errors, defaults.ignore_errors);
  options.set_int(L"overwrite", plugin_options.overwrite, defaults.overwrite);
  options.set_int(L"show_op_stats", plugin_options.show_stats, defaults.show_stats);
  options.set_bool(L"use_file_filters", plugin_options.use_file_filters, defaults.use_file_filters);
}

const wchar_t* c_root_dir = L"\\";

std::string load_last_dir(const std::string& id) {
  Options options;
  if (!options.create())
    return std::string(c_root_dir);
  return options.get_str((L"last_dir_" + id).data(), c_root_dir);
}

void save_last_dir(const std::string& id, const std::string& dir) {
  Options options;
  if (!options.create())
    return;
  options.set_str((L"last_dir_" + id).data(), dir, c_root_dir);
}

const wchar_t* c_copy_opened_files_option = L"CopyOpened";
const wchar_t* c_esc_confirmation_option = L"Esc";

bool get_app_option(size_t category, const wchar_t* name, bool def_value) {
  Options options;
  if (!options.create(true))
    return def_value;
  return options.get_bool(category, name, def_value);
}
