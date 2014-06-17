#ifndef _UTIL_H
#define _UTIL_H

#ifdef DEBUG
#define LOG_MSG(msg) OutputDebugStringA((msg + '\n').data());
#define LOG_UMSG(msg) OutputDebugStringW((msg + '\n').data());
#define LOG_ERR(e) OutputDebugStringW(std::string::format(L"File: %s Line: %u \"%s\"\n", oem_to_unicode(e.file).data(), e.line, e.message().data()).data());
#else // DEBUG
#define LOG_MSG(msg)
#define LOG_UMSG(msg)
#define LOG_ERR(e) (e)
#endif // DEBUG

void unicode_to_oem(AnsiString& oem_str, const std::string& u_str);
void oem_to_unicode(std::string& u_str, const AnsiString& oem_str);
AnsiString unicode_to_oem(const std::string& u_str);
std::string oem_to_unicode(const AnsiString& oem_str);
void unicode_to_ansi(AnsiString& a_str, const std::string& u_str);
void ansi_to_unicode(std::string& u_str, const AnsiString& a_str);
AnsiString unicode_to_ansi(const std::string& u_str);
std::string ansi_to_unicode(const AnsiString& a_str);

std::string word_wrap(const std::string& message, unsigned wrap_bound);
std::string format_data_size(unsigned __int64 size, const std::vector<std::string>& suffixes);
std::string format_time(unsigned __int64 t /* ms */);
std::string fit_str(const std::string& path, unsigned size);
std::string center(const std::string& str, unsigned width);
unsigned __int64 mul_div(unsigned __int64 a, unsigned __int64 b, unsigned __int64 c);

#ifdef _M_X64
#  define PLUGIN_TYPE L" uni x64"
#else
#  define PLUGIN_TYPE L" uni"
#endif

struct ModuleVersion {
  unsigned major;
  unsigned minor;
  unsigned patch;
  unsigned revision;
};
ModuleVersion get_module_version(HINSTANCE module);
const std::string extract_file_name(const std::string& file_path);
void unquote(std::string& str);

std::string make_temp_file();
std::string format_file_time(const FILETIME& file_time);

std::string add_trailing_slash(const std::string& file_path);
std::string del_trailing_slash(const std::string& file_path);

intptr_t far_control_int(HANDLE h_panel, FILE_CONTROL_COMMANDS command, intptr_t param);
intptr_t far_control_ptr(HANDLE h_panel, FILE_CONTROL_COMMANDS command, const void* param);
PluginPanelItem* far_get_panel_item(HANDLE h_panel, intptr_t index, const PanelInfo& pi);
PluginPanelItem* far_get_selected_panel_item(HANDLE h_panel, intptr_t index, const PanelInfo& pi);
std::string far_get_current_dir();
std::string far_get_full_path(const std::string& file_path);
void far_set_progress_state(TBPFLAG state);
void far_set_progress_value(unsigned __int64 completed, unsigned __int64 total);

#endif // _UTIL_H
