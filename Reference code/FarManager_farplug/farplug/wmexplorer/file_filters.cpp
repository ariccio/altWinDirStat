#define _ERROR_WINDOWS
#include "error.h"

#include "util.h"
#include "as_api.h"
#include "file_filters.h"

std::vector<FileFilters> export_filter_list;
std::vector<FileFilters> import_filter_list;

const GUID IID_ICeFileFilter = { 0x6C5C05E1, 0x97A2, 0x11CF, { 0x80, 0x11, 0x00, 0xA0, 0xC9, 0x0A, 0x8F, 0x78 } };
const GUID IID_ICeFileFilterSite = { 0x6C5C05E0, 0x97A2, 0x11CF, { 0x80, 0x11, 0x00, 0xA0, 0xC9, 0x0A, 0x8F, 0x78 } };

std::string reg_read_string(HKEY hkey, const std::string& name, const std::string& def_value) {
  std::string value = def_value;
  DWORD type = REG_SZ;
  DWORD data_size;
  // get string size
  LONG res = RegQueryValueExW(hkey, name.data(), NULL, &type, NULL, &data_size);
  if (res == ERROR_SUCCESS) {
    std::string data;
    // get string value
    res = RegQueryValueExW(hkey, name.data(), NULL, &type, (LPBYTE) data.buf(data_size / sizeof(wchar_t)), &data_size);
    if (res == ERROR_SUCCESS) {
      data.set_size(data_size / sizeof(wchar_t) - 1); // throw away terminating NULL
      value = data;
    }
  }
  return value;
}

void load_file_filters(HCESVC hkey_filters) {
  unsigned ext_key_idx = 0;
  std::string ext_key_name;
  bool more = true;
  while (more) {
    // enum all file extensions
    DWORD key_name_size = 256;
    LONG res = RegEnumKeyExW(hkey_filters, ext_key_idx, ext_key_name.buf(key_name_size), &key_name_size, NULL, NULL, NULL, NULL);
    if (res == ERROR_NO_MORE_ITEMS) more = false;
    if (more) {
      CHECK_ADVAPI(res);
      ext_key_idx++;
      ext_key_name.set_size(key_name_size);
      // verify that key is really a file extension (starts with .)
      if ((ext_key_name.size() != 0) && (ext_key_name[0] == '.')) {
        FileFilters import_filters;
        FileFilters export_filters;
        export_filters.src_ext = import_filters.src_ext = ext_key_name.right(ext_key_name.size() - 1);
        // open file extension key
        HKEY hkey_ext;
        CHECK_ADVAPI(RegOpenKeyExW(hkey_filters, ext_key_name.data(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkey_ext));
        try {
          // default export and import filters
          std::string default_export = reg_read_string(hkey_ext, L"DefaultExport", L"Binary Copy");
          std::string default_import = reg_read_string(hkey_ext, L"DefaultImport", L"Binary Copy");
          // list of installed filters for current file extension
          HKEY hkey_list;
          LONG res = RegOpenKeyExW(hkey_ext, L"InstalledFilters", 0, KEY_QUERY_VALUE, &hkey_list);
          if (res != ERROR_FILE_NOT_FOUND) {
            CHECK_ADVAPI(res);
            try {
              unsigned value_idx = 0;
              // enum all installed filters for current file extension
              bool more = true;
              while (more) {
                Filter filter;
                DWORD value_name_size = 256;
                DWORD value_type;
                LONG res = RegEnumValueW(hkey_list, value_idx, filter.guid.buf(value_name_size), &value_name_size, NULL, &value_type, NULL, NULL);
                if (res == ERROR_NO_MORE_ITEMS) more = false;
                if (more) {
                  CHECK_ADVAPI(res);
                  filter.guid.set_size(value_name_size);
                  value_idx++;
                  // value is a string and not a key default value
                  if ((value_type == REG_SZ) && (filter.guid.size() != 0)) {
                    filter.guid.set_size(value_name_size);
                    // open filter class information key
                    HKEY hkey_pegasus;
                    LONG res = RegOpenKeyExW(HKEY_CLASSES_ROOT, (L"CLSID\\" + filter.guid + L"\\PegasusFilter").data(), 0, KEY_QUERY_VALUE, &hkey_pegasus);
                    // does filter class exist in registry?
                    if (res != ERROR_FILE_NOT_FOUND) {
                      CHECK_ADVAPI(res);
                      try {
                        // target file extension
                        filter.dst_ext = reg_read_string(hkey_pegasus, L"NewExtension", L"");
                        // filter GUID
                        if (filter.dst_ext.size() != 0) {
                          filter.description = reg_read_string(hkey_pegasus, L"Description", filter.dst_ext);
                          if (RegQueryValueExW(hkey_pegasus, L"Export", NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                            // this is export filter
                            if (filter.guid.icompare(default_export) == 0) export_filters.insert(0, filter);
                            else export_filters += filter;
                          }
                          if (RegQueryValueExW(hkey_pegasus, L"Import", NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                            // this is import filter
                            if (filter.guid.icompare(default_import) == 0) import_filters.insert(0, filter);
                            else import_filters += filter;
                          }
                        }
                      }
                      finally (RegCloseKey(hkey_pegasus));
                    }
                  }
                }
              } // end enum of installed filters
            }
            finally (RegCloseKey(hkey_list));
          }
        }
        finally (RegCloseKey(hkey_ext));
        if ((export_filters.size() != 0) && (export_filter_list.bsearch(export_filters) == -1)) {
          export_filter_list += export_filters;
          export_filter_list.sort();
        }
        if ((import_filters.size() != 0) && (import_filter_list.bsearch(import_filters) == -1)) {
          import_filter_list += import_filters;
          import_filter_list.sort();
        }
      }
    }
  } // end enum of file extensions
}

void load_file_filters() {
  static bool filters_loaded = false;
  if (filters_loaded) return;
  export_filter_list.clear();
  import_filter_list.clear();
  // open HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows CE Services\Filters
  HCESVC hkey_filters;
  CHECK_COM(ceutil.CeSvcOpenW(CESVC_FILTERS, L"", FALSE, &hkey_filters));
  try {
    load_file_filters(hkey_filters);
  }
  finally (VERIFY(SUCCEEDED(ceutil.CeSvcClose(hkey_filters))));
  // enumerate registered devices
  HCESVC h_enum = NULL;
  try {
    unsigned dev_idx = 0;
    DWORD dev_id;
    while (true) {
      HRESULT hr = ceutil.CeSvcEnumProfiles(&h_enum, dev_idx, &dev_id);
      if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
        h_enum = NULL;
        break;
      }
      CHECK_COM(hr);
      // load filter list for each registered device
      HCESVC hkey_dev;
      CHECK_COM(ceutil.CeSvcOpenW(CESVC_DEVICEX, (LPWSTR) dev_id, FALSE, &hkey_dev));
      try {
        CHECK_COM(ceutil.CeSvcOpenExW(hkey_dev, L"Filters", FALSE, &hkey_filters));
        try {
          load_file_filters(hkey_filters);
        }
        finally (VERIFY(SUCCEEDED(ceutil.CeSvcClose(hkey_filters))));
      }
      finally (VERIFY(SUCCEEDED(ceutil.CeSvcClose(hkey_dev))));
      dev_idx++;
    }
  }
  finally (
    if (h_enum != NULL) VERIFY(SUCCEEDED(ceutil.CeSvcClose(h_enum)));
  );
  filters_loaded = true;
}

ICeFileFilter* create_interface(const std::string& guid_str) {
  ICeFileFilter* itf;
  GUID guid;
  CHECK_COM(IIDFromString(const_cast<LPOLESTR>(guid_str.data()), &guid));
  CHECK_COM(CoCreateInstance(guid, NULL, CLSCTX_INPROC_SERVER, IID_ICeFileFilter, (void**) &itf));
  return itf;
}

FilterInterface::FilterInterface(const std::string& src_ext, const std::string& dst_ext, const std::string& guid_str): src_ext(src_ext), dst_ext(dst_ext), itf(create_interface(guid_str)) {
}

FilterInterface::FilterInterface(const FilterInterface& filter_itf) {
  src_ext = filter_itf.src_ext;
  dst_ext = filter_itf.dst_ext;
  itf = filter_itf.itf;
  itf->AddRef();
}

FilterInterface::~FilterInterface() {
  itf->Release();
}

FilterInterface& FilterInterface::operator=(const FilterInterface& filter_itf) {
  src_ext = filter_itf.src_ext;
  dst_ext = filter_itf.dst_ext;
  itf = filter_itf.itf;
  itf->AddRef();
  return *this;
}

#define BEGIN_ERROR_HANDLER try {
#define END_ERROR_HANDLER \
  } \
  catch (SystemError& e) { \
    return HRESULT_FROM_WIN32(e.error()); \
  } \
  catch (ComError& e) { \
    return e.error(); \
  } \
  catch (...) { \
    return E_FAIL; \
  }

//implement filestream that derives from IStream
class FileStream: public IStream {
private:
  std::string file_name;
  bool read_only;
  HANDLE h_file;
  LONG ref_cnt;
  void open_if_needed() {
    if (h_file == INVALID_HANDLE_VALUE) {
      h_file = ::CreateFileW(file_name.data(), read_only ? GENERIC_READ : GENERIC_WRITE, FILE_SHARE_READ, NULL, read_only ? OPEN_EXISTING : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      CHECK_API(h_file != INVALID_HANDLE_VALUE);
    }
  }
public:
  FileStream(const std::string& file_name, bool read_only): file_name(file_name), read_only(read_only), h_file(INVALID_HANDLE_VALUE), ref_cnt(1) {
  }
  ~FileStream() {
    if (h_file != INVALID_HANDLE_VALUE) {
      ::CloseHandle(h_file);
    }
  }
  // IUnknown Interface
  STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj) {
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IStream) || IsEqualIID(riid, IID_ISequentialStream)) {
      *ppvObj = static_cast<IStream*>(this);
      AddRef();
      return S_OK;
    }
    else {
      *ppvObj = NULL;
      return E_NOINTERFACE;
    }
  }
  STDMETHODIMP_(ULONG) AddRef(THIS) {
    return InterlockedIncrement(&ref_cnt);
  }
  STDMETHODIMP_(ULONG) Release(THIS) {
    ULONG cnt = InterlockedDecrement(&ref_cnt);
    if (cnt == 0) delete this;
    return cnt;
  }
  // ISequentialStream Interface
  STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) {
    BEGIN_ERROR_HANDLER;
    open_if_needed();
    CHECK_API(ReadFile(h_file, pv, cb, pcbRead, NULL) != 0);
    return S_OK;
    END_ERROR_HANDLER;
  }
  STDMETHODIMP Write(void const* pv, ULONG cb, ULONG* pcbWritten) {
    BEGIN_ERROR_HANDLER;
    open_if_needed();
    CHECK_API(WriteFile(h_file, pv, cb, pcbWritten, NULL) != 0);
    return S_OK;
    END_ERROR_HANDLER;
  }
  // IStream Interface
  STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) {
    BEGIN_ERROR_HANDLER;
    open_if_needed();
    DWORD dwMoveMethod;
    switch (dwOrigin) {
    case STREAM_SEEK_SET:
      dwMoveMethod = FILE_BEGIN;
      break;
    case STREAM_SEEK_CUR:
      dwMoveMethod = FILE_CURRENT;
      break;
    case STREAM_SEEK_END:
      dwMoveMethod = FILE_END;
      break;
    default:
      return STG_E_INVALIDFUNCTION;
      break;
    }
    CHECK_API(SetFilePointerEx(h_file, dlibMove, (PLARGE_INTEGER) plibNewPosition, dwMoveMethod) != 0);
    return S_OK;
    END_ERROR_HANDLER;
  }
  STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) {
    return E_NOTIMPL;
  }
  STDMETHODIMP CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) {
    return E_NOTIMPL;
  }
  STDMETHODIMP Commit(DWORD grfCommitFlags) {
    return E_NOTIMPL;
  }
  STDMETHODIMP Revert(void) {
    return E_NOTIMPL;
  }
  STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {
    return E_NOTIMPL;
  }
  STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {
    return E_NOTIMPL;
  }
  STDMETHODIMP Stat(STATSTG* pstatstg, DWORD grfStatFlag) {
    BEGIN_ERROR_HANDLER;
    open_if_needed();
    memset(pstatstg, 0, sizeof(STATSTG));
    BY_HANDLE_FILE_INFORMATION file_info;
    CHECK_API(GetFileInformationByHandle(h_file, &file_info) != 0);
    if ((grfStatFlag & STATFLAG_NONAME) == 0) {
      pstatstg->pwcsName = (LPOLESTR) CoTaskMemAlloc((file_name.size() + 1) * sizeof(wchar_t));
      if (pstatstg->pwcsName == NULL) FAIL(ComError(STG_E_INSUFFICIENTMEMORY));
      wcscpy(pstatstg->pwcsName, file_name.data());
    }
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.LowPart = file_info.nFileSizeLow;
    pstatstg->cbSize.HighPart = file_info.nFileSizeHigh;
    pstatstg->mtime = file_info.ftLastWriteTime;
    pstatstg->ctime = file_info.ftCreationTime;
    pstatstg->atime = file_info.ftLastAccessTime;
    pstatstg->grfMode = (read_only ? STGM_READ : (STGM_WRITE | STGM_CREATE)) | STGM_SHARE_DENY_WRITE;
    return S_OK;
    END_ERROR_HANDLER;
  }
  STDMETHODIMP Clone(IStream** ppstm) {
    return E_NOTIMPL;
  }
};

class FileFilterSite: public ICeFileFilterSite {
private:
  std::string src_file_name;
  std::string dst_file_name;
public:
  FileFilterSite(std::string src_file_name, std::string dst_file_name): src_file_name(src_file_name), dst_file_name(dst_file_name) {
  }
  // IUnknown
  STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj) {
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICeFileFilterSite)) {
      *ppvObj = static_cast<ICeFileFilterSite*>(this);
      return S_OK;
    }
    else {
      *ppvObj = NULL;
      return E_NOINTERFACE;
    }
  }
  STDMETHODIMP_(ULONG) AddRef(THIS) {
    return 0;
  }
  STDMETHODIMP_(ULONG) Release(THIS) {
    return 0;
  }
  // ICeFileFilterSite
  STDMETHODIMP OpenSourceFile(THIS_ int nHowToOpenFile, LPVOID *ppObj) {
    if (nHowToOpenFile != CF_OPENFLAT) return E_INVALIDARG;
    *ppObj = new FileStream(src_file_name, true);
    if (*ppObj == NULL) return E_OUTOFMEMORY;
    return S_OK;
  }
  STDMETHODIMP OpenDestinationFile(THIS_ int nHowToOpenFile, LPCTSTR pszFullpath, LPVOID *ppObj) {
    if (nHowToOpenFile != CF_OPENFLAT) return E_INVALIDARG;
    *ppObj = new FileStream(dst_file_name, false);
    if (*ppObj == NULL) return E_OUTOFMEMORY;
    return S_OK;
  }
  STDMETHODIMP CloseSourceFile(THIS_ LPUNKNOWN pObj) {
    ((FileStream*) pObj)->Release();
    return S_OK;
  }
  STDMETHODIMP CloseDestinationFile(THIS_ BOOL bKeepFile, LPUNKNOWN pObj) {
    ((FileStream*) pObj)->Release();
    return S_OK;
  }
  STDMETHODIMP ReportProgress(THIS_ UINT nPercent) {
    return S_OK;
  }
  STDMETHODIMP ReportLoss(THIS_ DWORD dw, LPCTSTR psz, va_list args) {
    return S_OK;
  }
};

struct FilePathParts {
  std::string dir;
  std::string name;
  std::string ext;
};

FilePathParts split_path(const std::string& path) {
  FilePathParts parts;
  std::string full_path;
  LPWSTR name;
  DWORD res = GetFullPathNameW(path.data(), MAX_PATH, full_path.buf(MAX_PATH), &name);
  if (res > MAX_PATH) {
    res = GetFullPathNameW(path.data(), res, full_path.buf(res), &name);
  }
  CHECK_API(res != 0);
  full_path.set_size();
  if (name == NULL) {
    parts.dir = full_path;
  }
  else {
    parts.dir = full_path.left((unsigned) (name - full_path.data()));
    parts.name = name;
    unsigned p = parts.name.rsearch('.');
    if (p != -1) parts.ext = parts.name.right(parts.name.size() - p - 1);
  }
  return parts;
}

void convert_file(ICeFileFilter* filter_itf, const std::string& src_file_name, const std::string& dst_file_name, bool import) {
  FileFilterSite ffs(src_file_name, dst_file_name);

  CFF_CONVERTINFO cff_info;
  cff_info.bImport = import ? TRUE : FALSE;
  cff_info.hwndParent = GetDesktopWindow();
  cff_info.bYesToAll = FALSE;
  cff_info.pffs = &ffs;

  FilePathParts src_path_parts = split_path(src_file_name);
  CFF_SOURCEFILE cff_src;
  if (src_file_name.size() < _MAX_PATH) strcpy(cff_src.szFullpath, unicode_to_ansi(src_file_name).data());
  else strcpy(cff_src.szFullpath, unicode_to_ansi(src_file_name.left(_MAX_PATH - 1)).data());
  if (src_path_parts.dir.size() < _MAX_PATH) strcpy(cff_src.szPath, unicode_to_ansi(src_path_parts.dir).data());
  else strcpy(cff_src.szPath, unicode_to_ansi(src_path_parts.dir.left(_MAX_PATH - 1)).data());
  if (src_path_parts.name.size() < _MAX_FNAME) strcpy(cff_src.szFilename, unicode_to_ansi(src_path_parts.name).data());
  else strcpy(cff_src.szFilename, unicode_to_ansi(src_path_parts.name.left(_MAX_FNAME - 1)).data());
  if (src_path_parts.ext.size() < _MAX_EXT) strcpy(cff_src.szExtension, unicode_to_ansi(src_path_parts.ext).data());
  else strcpy(cff_src.szExtension, unicode_to_ansi(src_path_parts.ext.left(_MAX_EXT - 1)).data());
  WIN32_FIND_DATAW find_data;
  HANDLE h_find = FindFirstFileW(src_file_name.data(), &find_data);
  CHECK_API(h_find != INVALID_HANDLE_VALUE);
  FindClose(h_find);
  cff_src.cbSize = find_data.nFileSizeLow;
  cff_src.ftCreated = find_data.ftCreationTime;
  cff_src.ftModified = find_data.ftLastWriteTime;

  FilePathParts dst_path_parts = split_path(dst_file_name);
  CFF_DESTINATIONFILE cff_dst;
  if (dst_file_name.size() < _MAX_PATH) strcpy(cff_dst.szFullpath, unicode_to_ansi(dst_file_name).data());
  else strcpy(cff_dst.szFullpath, unicode_to_ansi(dst_file_name.left(_MAX_PATH - 1)).data());
  if (dst_path_parts.dir.size() < _MAX_PATH) strcpy(cff_dst.szPath, unicode_to_ansi(dst_path_parts.dir).data());
  else strcpy(cff_dst.szPath, unicode_to_ansi(dst_path_parts.dir.left(_MAX_PATH - 1)).data());
  if (dst_path_parts.name.size() < _MAX_FNAME) strcpy(cff_dst.szFilename, unicode_to_ansi(dst_path_parts.name).data());
  else strcpy(cff_dst.szFilename, unicode_to_ansi(dst_path_parts.name.left(_MAX_FNAME - 1)).data());
  if (dst_path_parts.ext.size() < _MAX_EXT) strcpy(cff_dst.szExtension, unicode_to_ansi(dst_path_parts.ext).data());
  else strcpy(cff_dst.szExtension, unicode_to_ansi(dst_path_parts.ext.left(_MAX_EXT - 1)).data());

  volatile BOOL cancel = FALSE;
  CF_ERROR cf_error;
  int nconv = 0;
  while (true) {
    HRESULT hr = filter_itf->NextConvertFile(nconv, &cff_info, &cff_src, &cff_dst, &cancel, &cf_error);
    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) break;
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) break;
    if (hr == E_FAIL) {
      char* msg;
      DWORD cb;
      filter_itf->FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, cf_error, 0, (LPTSTR) &msg, 0, NULL, &cb);
      try {
        FAIL(CustomError(L"Filter error: " + ansi_to_unicode(msg).strip(), src_file_name));
      }
      finally (LocalFree(msg));
    }
    CHECK_COM(hr);
    nconv++;
  }
}
