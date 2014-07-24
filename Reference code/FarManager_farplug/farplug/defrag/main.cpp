#include <windows.h>

#include <stdio.h>

#include "col/UnicodeString.h"
using namespace col;

#define _ERROR_WINDOWS
#include "error.h"

#include "utils.h"
#include "defragment.h"

void process_dir(const UnicodeString& path) {
  WIN32_FIND_DATAW find_data;
  HANDLE h_find = FindFirstFileW(long_path(add_trailing_slash(path) + L'*').data(), &find_data);
  CHECK_SYS(h_find != INVALID_HANDLE_VALUE);
  CLEAN(HANDLE, h_find, FindClose(h_find));
  while (true) {
    if ((wcscmp(find_data.cFileName, L".") != 0) && (wcscmp(find_data.cFileName, L"..") != 0)) {
      UnicodeString file_name = add_trailing_slash(path) + find_data.cFileName;
      try {
        defragment(file_name);
      }
      catch (Error& e) {
        fwprintf(stderr, L"%s: %s\n", file_name.data(), e.message().data());
      }
      catch (std::exception& e) {
        fwprintf(stderr, L"%s: ", file_name.data());
        fprintf(stderr, "%s\n", e.what());
      }
      catch (...) {
        fwprintf(stderr, L"%s: unknown error\n", file_name.data());
      }
      if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        process_dir(file_name);
      }
    }
    if (FindNextFileW(h_find, &find_data) == 0) {
      CHECK_SYS(GetLastError() == ERROR_NO_MORE_FILES);
      break;
    }
  }
}

int wmain(int argc, wchar_t* argv[]) {
  UnicodeString path;
  bool recursive = false;
  if ((argc == 3) && (wcscmp(argv[1], L"-r") == 0)) {
    recursive = true;
    path = argv[2];
  }
  else if (argc == 2) {
    path = argv[1];
  }
  else {
    wprintf(L"Usage: defrag [-r] <path>\n");
    return 2;
  }
  try {
    UnicodeString full_path;
    unsigned full_path_size = MAX_PATH;
    full_path_size = GetFullPathNameW(path.data(), full_path_size, full_path.buf(full_path_size), NULL);
    if (full_path_size > MAX_PATH) {
      full_path_size = GetFullPathNameW(path.data(), full_path_size, full_path.buf(full_path_size), NULL);
    }
    CHECK_SYS(full_path_size != 0);
    full_path.set_size(full_path_size);

    WIN32_FIND_DATAW find_data;
    HANDLE h_find = FindFirstFileW(long_path(full_path).data(), &find_data);
    CHECK_SYS(h_find != INVALID_HANDLE_VALUE);
    CLEAN(HANDLE, h_find, FindClose(h_find));

    defragment(full_path);
    if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && recursive) process_dir(full_path);
    return 0;
  }
  catch (Error& e) {
    fwprintf(stderr, L"%s: %s\n", path.data(), e.message().data());
  }
  catch (std::exception& e) {
    fwprintf(stderr, L"%s: ", path.data());
    fprintf(stderr, "%s\n", e.what());
  }
  catch (...) {
    fwprintf(stderr, L"%s: unknown error\n", path.data());
  }
  return 1;
}
