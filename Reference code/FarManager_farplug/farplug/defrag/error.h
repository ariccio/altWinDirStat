#pragma once

#define finally(code) catch(...) { code; throw; } code;

#define FAIL(err) { \
  try { \
    throw err; \
  } \
  catch (Error& e) { \
    e.file = __FILE__; \
    e.line = __LINE__; \
    throw; \
  } \
}

#define NOFAIL(code) try { code; } catch (...) { }

#define CHECK(code, msg) { if (!(code)) FAIL(MsgError(msg)); }
#define CHECK_STD(code) { if (!(code)) FAIL(StdIoError()); }
#define CHECK_SYS(code) { if (!(code)) FAIL(SystemError()); }
#define CHECK_ADVSYS(code) { DWORD __ret; if ((__ret = (code)) != ERROR_SUCCESS) FAIL(SystemError(__ret)); }
#define CHECK_COM(code) { HRESULT __ret; if (FAILED(__ret = (code))) FAIL(ComError(__ret)); }

#define VERIFY(code) { if (!(code)) assert(false); }

#define BREAK FAIL(Break());

#define ALLOC_RSRC(code) code; try {
#define FREE_RSRC(code) } catch (...) { code; throw; } code;

#define CLEAN(type, object, code) \
  class Clean_##object { \
  private: \
    type& object; \
  public: \
    Clean_##object(type& object): object(object) { \
    } \
    ~Clean_##object() { \
      code; \
    } \
  }; \
  Clean_##object clean_##object(object);

class Error {
public:
  const char* file;
  unsigned line;
  virtual UnicodeString message() const = 0;
};

class Break {
};

class MsgError: public Error {
private:
  UnicodeString msg;
public:
  MsgError(const UnicodeString& msg): msg(msg) {
  }
  MsgError(const wchar_t* msg): msg(msg) {
  }
  virtual UnicodeString message() const {
    return msg;
  }
};

#ifdef _ERROR_STDIO

class StdIoError: public Error {
private:
  int code;
public:
  int error() const {
    return code;
  }
  StdIoError(): code(errno) {
  }
  virtual UnicodeString message() const {
    return _wcserror(code);
  }
};

#endif // _ERROR_STDIO

#ifdef _ERROR_WINDOWS

inline UnicodeString get_system_message(DWORD code) {
  UnicodeString message;
  const wchar_t* sys_msg;
  DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, code, 0, (LPWSTR) &sys_msg, 0, NULL);
  if (len != 0) {
    try {
      message = sys_msg;
      message.strip();
      message.add_fmt(L" (%u)", code);
    }
    finally (LocalFree((HLOCAL) sys_msg));
  }
  return message;
}

class SystemError: public Error {
private:
  DWORD code;
public:
  SystemError(): code(GetLastError()) {
  }
  SystemError(DWORD code): code(code) {
  }
  DWORD error() const {
    return code;
  }
  virtual UnicodeString message() const {
    return get_system_message(code);
  }
};

class ComError: public Error {
private:
  HRESULT hr;
public:
  ComError(HRESULT hr): hr(hr) {
  }
  HRESULT error() const {
    return hr;
  }
  virtual UnicodeString message() const {
    unsigned facility = HRESULT_FACILITY(hr);
    unsigned code = HRESULT_CODE(hr);
    UnicodeString facility_name;
    switch (facility) {
    case FACILITY_DISPATCH:
      facility_name = L"DISPATCH";
      break;
    case FACILITY_ITF:
      facility_name = L"ITF";
      break;
    case FACILITY_NULL:
      facility_name = L"NULL";
      break;
    case FACILITY_RPC:
      facility_name = L"RPC";
      break;
    case FACILITY_STORAGE:
      facility_name = L"STORAGE";
      break;
    case FACILITY_WIN32:
      facility_name = L"WIN32";
      break;
    case FACILITY_WINDOWS:
      facility_name = L"WINDOWS";
      break;
    default:
      facility_name.copy_fmt(L"%u", facility);
      break;
    }
    if ((facility == FACILITY_WIN32) || (facility == FACILITY_WINDOWS)) {
      return UnicodeString::format(L"COM error: 0x%x (facility = %S): %S", hr, &facility_name, &get_system_message(code));
    }
    else {
      return UnicodeString::format(L"COM error: 0x%x (facility = %S, code = %u)", hr, &facility_name, code);
    }
  }
};

#ifdef MMRESULT

class MMError: public Error {
private:
  MMRESULT code;
public:
  MMError(MMRESULT code): code(code) {
  }
  virtual UnicodeString message() const {
    return UnicodeString::format(L"MMSYSTEM error: %u", code);
  }
};
#endif // MMRESULT

#endif // _ERROR_WINDOWS
