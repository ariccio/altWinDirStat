#define _ERROR_WINDOWS
#include "error.h"
#include "guids.h"
#include "util.h"

extern struct PluginStartupInfo g_far;
extern struct FarStandardFunctions g_fsf;

// unicode <-> oem codepage conversions
void unicode_to_oem(AnsiString& oem_str, const std::string& u_str) {
  unsigned size = u_str.size() + 1;
  int res = WideCharToMultiByte(CP_OEMCP, 0, u_str.data(), size, oem_str.buf(u_str.size()), size, NULL, NULL);
  if (res == 0) FAIL(SystemError());
  oem_str.set_size(res - 1);
}

void oem_to_unicode(std::string& u_str, const AnsiString& oem_str) {
  unsigned size = oem_str.size() + 1;
  int res = MultiByteToWideChar(CP_OEMCP, 0, oem_str.data(), size, u_str.buf(oem_str.size()), size);
  if (res == 0) FAIL(SystemError());
  u_str.set_size(res - 1);
}

AnsiString unicode_to_oem(const std::string& u_str) {
  AnsiString oem_str;
  unicode_to_oem(oem_str, u_str);
  return oem_str;
}

std::string oem_to_unicode(const AnsiString& oem_str) {
  std::string u_str;
  oem_to_unicode(u_str, oem_str);
  return u_str;
}

void unicode_to_ansi(AnsiString& a_str, const std::string& u_str) {
  unsigned size = u_str.size() + 1;
  int res = WideCharToMultiByte(CP_ACP, 0, u_str.data(), size, a_str.buf(u_str.size()), size, NULL, NULL);
  if (res == 0) FAIL(SystemError());
  a_str.set_size(res - 1);
}

void ansi_to_unicode(std::string& u_str, const AnsiString& a_str) {
  unsigned size = a_str.size() + 1;
  int res = MultiByteToWideChar(CP_ACP, 0, a_str.data(), size, u_str.buf(a_str.size()), size);
  if (res == 0) FAIL(SystemError());
  u_str.set_size(res - 1);
}

AnsiString unicode_to_ansi(const std::string& u_str) {
  AnsiString a_str;
  unicode_to_ansi(a_str, u_str);
  return a_str;
}

std::string ansi_to_unicode(const AnsiString& a_str) {
  std::string u_str;
  ansi_to_unicode(u_str, a_str);
  return u_str;
}

std::string word_wrap(const std::string& message, unsigned wrap_bound) {
  std::string msg = message;
  unsigned limit = wrap_bound;
  unsigned idx = -1;
  for (unsigned i = 0; i < msg.size(); i++) {
    if (i >= limit) {
      if (idx != -1) {
        msg.insert(idx, '\n');
        i = idx + 1;
        limit = idx + 2 + wrap_bound;
        idx = -1;
        continue;
      }
    }
    if (msg[i] == ' ') idx = i;
  }
  return msg;
}

std::string format_data_size(unsigned __int64 size, const std::vector<std::string>& suffixes) {
  unsigned f = 0;
  double v = (double) size;
  while ((v >= 1024) && (f < 4)) {
    f++;
    v /= 1024;
  }

  double mul;
  if (v < 10) mul = 100;
  else if (v < 100) mul = 10;
  else mul = 1;

  v *= mul;
  if (v - floor(v) >= 0.5) v += 1;
  v = floor(v) / mul;
  if ((v == 1024) && (f < 4)) {
    v = 1;
    f++;
  }

  wchar_t buf[16];
  swprintf(buf, L"%g %s", v, suffixes[f].data());
  return std::string(buf);
}

std::string format_time(unsigned __int64 t /* ms */) {
  unsigned __int64 ms = t % 1000;
  unsigned __int64 s = (t / 1000) % 60;
  unsigned __int64 m = (t / 1000 / 60) % 60;
  unsigned __int64 h = t / 1000 / 60 / 60;
  return std::string::format(L"%02Lu:%02Lu:%02Lu", h, m, s);
}

std::string fit_str(const std::string& path, unsigned size) {
  if (path.size() <= size) return path;
  size -= 3; // place for ...
  unsigned ls = size / 2; // left part size
  unsigned rs = size - ls; // right part size
  return path.left(ls) + L"..." + path.right(rs);
}

std::string center(const std::string& str, unsigned width) {
  if (str.size() >= width) return str;
  unsigned lpad = (width - str.size()) / 2;
  unsigned rpad = width - str.size() - lpad;
  return std::string::format(L"%.*c%S%.*c", lpad, ' ', &str, rpad, ' ');
}

unsigned __int64 mul_div(unsigned __int64 a, unsigned __int64 b, unsigned __int64 c) {
  double r_double = (double) a * ((double) b / (double) c);
  assert(r_double < 0xFFFFFFFFFFFFFFFFl);
  unsigned __int64 r_int = (unsigned __int64) r_double;
  if (r_double - r_int >= 0.5) r_int++;
  return r_int;
}

ModuleVersion get_module_version(HINSTANCE module) {
  ModuleVersion version;
  memset(&version, 0, sizeof(version));
  std::string file_name;
  unsigned size = GetModuleFileNameW(module, file_name.buf(MAX_PATH), MAX_PATH);
  if (size != 0) {
    file_name.set_size(size);
    DWORD handle;
    size = GetFileVersionInfoSizeW(file_name.data(), &handle);
    if (size != 0) {
      Array<char> ver_data;
      if (GetFileVersionInfoW(file_name.data(), handle, size, ver_data.buf(size)) != 0) {
        VS_FIXEDFILEINFO* ver;
        if (VerQueryValueW((const LPVOID) ver_data.data(), L"\\", (LPVOID*) &ver, &size) != 0) {
          version.major = HIWORD(ver->dwProductVersionMS);
          version.minor = LOWORD(ver->dwProductVersionMS);
          version.patch = HIWORD(ver->dwProductVersionLS);
          version.revision = LOWORD(ver->dwProductVersionLS);
        }
      }
    }
  }
  return version;
}

const std::string extract_file_name(const std::string& file_path) {
  unsigned pos = file_path.rsearch('\\');
  if (pos == -1) pos = 0;
  else pos++;
  return file_path.slice(pos, file_path.size() - pos);
}

void unquote(std::string& str) {
  if ((str.size() >= 2) && (str[0] == '"') && (str.last() == '"')) {
    str.remove(0);
    str.remove(str.size() - 1);
  }
}

std::string make_temp_file() {
  std::string temp_path;
  CHECK_API(GetTempPathW(MAX_PATH, temp_path.buf(MAX_PATH)) != 0);
  temp_path.set_size();
  std::string temp_file_name;
  CHECK_API(GetTempFileNameW(temp_path.data(), L"wme", 0, temp_file_name.buf(MAX_PATH)) != 0);
  temp_file_name.set_size();
  return temp_file_name;
}

std::string format_file_time(const FILETIME& file_time) {
  FILETIME local_ft;
  if (FileTimeToLocalFileTime(&file_time, &local_ft) == 0) return std::string();
  SYSTEMTIME st;
  if (FileTimeToSystemTime(&local_ft, &st) == 0) return std::string();
  const unsigned c_size = 1024;
  std::string date_str;
  if (GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, date_str.buf(c_size), c_size) == 0) return std::string();
  date_str.set_size();
  std::string time_str;
  if (GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &st, NULL, time_str.buf(c_size), c_size) == 0) return std::string();
  time_str.set_size();
  return date_str + L' ' + time_str;
}

std::string add_trailing_slash(const std::string& file_path) {
  if ((file_path.size() == 0) || (file_path.last() == L'\\')) return file_path;
  else return file_path + L'\\';
}

std::string del_trailing_slash(const std::string& file_path) {
  if ((file_path.size() < 2) || (file_path.last() != L'\\')) return file_path;
  else return file_path.left(file_path.size() - 1);
}

intptr_t far_control_int(HANDLE h_panel, FILE_CONTROL_COMMANDS command, intptr_t param) {
  return g_far.PanelControl(h_panel, command, param, nullptr);
}

intptr_t far_control_ptr(HANDLE h_panel, FILE_CONTROL_COMMANDS command, const void* param) {
  return g_far.PanelControl(h_panel, command, 0, const_cast<void*>(param));
}

PluginPanelItem* far_get_panel_item(HANDLE h_panel, intptr_t index, const PanelInfo& pi) {
  static Array<unsigned char> ppi;
  unsigned size = static_cast<unsigned>(g_far.PanelControl(h_panel, FCTL_GETPANELITEM, index, nullptr));
  FarGetPluginPanelItem gpi = { sizeof(FarGetPluginPanelItem) };
  gpi.Size = size;
  gpi.Item = reinterpret_cast<PluginPanelItem*>(ppi.buf(size));
  g_far.PanelControl(h_panel, FCTL_GETPANELITEM, index, &gpi);
  ppi.set_size(size);
  return reinterpret_cast<PluginPanelItem*>(ppi.buf());
}

PluginPanelItem* far_get_selected_panel_item(HANDLE h_panel, intptr_t index, const PanelInfo& pi) {
  static Array<unsigned char> ppi;
  unsigned size = static_cast<unsigned>(g_far.PanelControl(h_panel, FCTL_GETSELECTEDPANELITEM, index, nullptr));
  FarGetPluginPanelItem gpi = { sizeof(FarGetPluginPanelItem) };
  gpi.Size = size;
  gpi.Item = reinterpret_cast<PluginPanelItem*>(ppi.buf(size));
  g_far.PanelControl(h_panel, FCTL_GETSELECTEDPANELITEM, index, &gpi);
  ppi.set_size(size);
  return reinterpret_cast<PluginPanelItem*>(ppi.buf());
}

std::string far_get_current_dir() {
  std::string curr_dir;
  int size = static_cast<int>(g_fsf.GetCurrentDirectory(0, NULL));
  g_fsf.GetCurrentDirectory(size, curr_dir.buf(size));
  curr_dir.set_size();
  return del_trailing_slash(curr_dir);
}

std::string far_get_full_path(const std::string& file_path) {
  const unsigned c_buf_size = 0x10000;
  std::string full_path;
  int size = static_cast<int>(g_fsf.ConvertPath(CPM_FULL, file_path.data(), full_path.buf(c_buf_size), c_buf_size));
  if (size > c_buf_size) g_fsf.ConvertPath(CPM_FULL, file_path.data(), full_path.buf(size), size);
  full_path.set_size();
  return full_path;
}

void far_set_progress_state(TBPFLAG state) {
  g_far.AdvControl(&c_plugin_guid, ACTL_SETPROGRESSSTATE, state, nullptr);
}

void far_set_progress_value(unsigned __int64 completed, unsigned __int64 total) {
  ProgressValue pv = { sizeof(ProgressValue) };
  pv.Completed = completed;
  pv.Total = total;
  g_far.AdvControl(&c_plugin_guid, ACTL_SETPROGRESSVALUE, 0, &pv);
}
