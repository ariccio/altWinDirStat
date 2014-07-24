#include <windows.h>

#include "col/UnicodeString.h"
using namespace col;

#include "utils.h"

// find path root component (drive letter / volume name / server share) and calculate its length
void locate_path_root(const UnicodeString& path, unsigned& path_root_len, bool& is_unc_path) {
  unsigned prefix_len = 0;
  is_unc_path = false;
  if (path.equal(0, L"\\\\?\\UNC\\")) {
    prefix_len = 8;
    is_unc_path = true;
  }
  else if (path.equal(0, L"\\\\?\\") || path.equal(0, L"\\??\\") || path.equal(0, L"\\\\.\\")) {
    prefix_len = 4;
  }
  else if (path.equal(0, L"\\\\")) {
    prefix_len = 2;
    is_unc_path = true;
  }
  if ((prefix_len == 0) && !path.equal(1, L':')) {
    path_root_len = 0;
  }
  else {
    unsigned p = path.search(prefix_len, L'\\');
    if (p == -1) p = path.size();
    if (is_unc_path) {
      p = path.search(p + 1, L'\\');
      if (p == -1) p = path.size();
    }
    path_root_len = p;
  }
}

UnicodeString extract_path_root(const UnicodeString& path) {
  unsigned path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  return path.left(path_root_len);
}

UnicodeString extract_file_name(const UnicodeString& path) {
  unsigned pos = path.rsearch('\\');
  if (pos == -1) pos = 0;
  else pos++;
  unsigned path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if (pos < path_root_len) return UnicodeString();
  return path.slice(pos);
}

UnicodeString extract_file_path(const UnicodeString& path) {
  unsigned pos = path.rsearch('\\');
  if (pos == -1) pos = 0;
  unsigned path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if (pos < path_root_len) return path.left(path_root_len);
  return path.left(pos);
}

UnicodeString long_path(const UnicodeString& path) {
  if (path.size() < MAX_PATH) return path;
  if (path.equal(0, L"\\\\?\\") || path.equal(0, L"\\\\.\\")) return path;
  if (path.equal(0, L"\\??\\")) return UnicodeString(path).replace(0, 4, L"\\\\?\\");
  if (path.equal(0, L"\\\\")) return UnicodeString(path).replace(0, 1, L"\\\\?\\UNC");
  return L"\\\\?\\" + path;
}

UnicodeString add_trailing_slash(const UnicodeString& file_path) {
  if ((file_path.size() == 0) || (file_path.last() == L'\\')) return file_path;
  else return file_path + L'\\';
}

UnicodeString del_trailing_slash(const UnicodeString& file_path) {
  if ((file_path.size() < 2) || (file_path.last() != L'\\')) return file_path;
  else return file_path.left(file_path.size() - 1);
}
