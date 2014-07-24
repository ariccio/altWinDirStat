#pragma once

struct NtfsVolume {
  UnicodeString name;
  DWORD serial;
  unsigned cluster_size;
  HANDLE handle;
  NtfsVolume(): handle(INVALID_HANDLE_VALUE), serial(0) {
  }
  ~NtfsVolume() {
    close();
  }
  void close() {
    if (handle != INVALID_HANDLE_VALUE) {
      CHECK_SYS(CloseHandle(handle));
      handle = INVALID_HANDLE_VALUE;
    }
    name.clear();
    serial = 0;
  }
  void open(const UnicodeString& volume_name);
};

UnicodeString get_real_path(const UnicodeString& fp);
