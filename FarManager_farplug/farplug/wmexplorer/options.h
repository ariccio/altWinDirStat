#ifndef _OPTIONS_H
#define _OPTIONS_H

enum ShowStatsOption {
  ssoAlways = 0,
  ssoNever = 1,
  ssoIfError = 2,
};

enum OverwriteOption {
  ooSkip = 0,
  ooOverwrite = 1,
  ooAsk = 2,
};

enum AccessMethod {
  amRapi = 0,
  amRapi2 = 1,
};

enum DevType {
  dtPDA = 0,
  dtSmartPhone = 1,
};

struct PluginOptions {
  // plugin configuration
  bool add_to_plugin_menu;
  bool add_to_disk_menu;
  AccessMethod access_method;
  bool hide_copy_dlg;
  bool save_last_dir;
  unsigned copy_buf_size;
  bool hide_rom_files;
  bool exit_on_dot_dot;
  std::string prefix;
  bool show_free_space;
  // default option values
  bool ignore_errors;
  OverwriteOption overwrite;
  ShowStatsOption show_stats;
  bool use_file_filters;
  bool save_def_values;
  // hidden options
  DevType last_dev_type;
  // plugin keys
  std::string key_attr;
  std::string key_execute;
  std::string key_hide_rom_files;
  PluginOptions();
};

void load_plugin_options(PluginOptions& options);
void save_plugin_options(const PluginOptions& options);
void save_def_option_values(const PluginOptions& options);
std::string load_last_dir(const std::string& id);
void save_last_dir(const std::string& id, const std::string& dir);

extern const wchar_t* c_copy_opened_files_option;
extern const wchar_t* c_esc_confirmation_option;
bool get_app_option(size_t category, const wchar_t* name, bool def_value);

bool virt_key_from_name(const std::string& name, unsigned& key, unsigned& state);

#endif // _OPTIONS_H
