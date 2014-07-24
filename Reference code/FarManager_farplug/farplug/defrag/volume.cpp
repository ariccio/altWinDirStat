#include <windows.h>
#include <winioctl.h>

#include "col/UnicodeString.h"
#include "col/PlainArray.h"
using namespace col;

#define _ERROR_WINDOWS
#include "error.h"

#include "utils.h"
#include "ntfs.h"
#include "volume.h"

extern struct FarStandardFunctions g_fsf;

const unsigned c_max_rp = 256;

const wchar_t* c_invalid_symlink_path = L"Invalid symbolic link target";
const wchar_t* c_invalid_unc_path = L"Invalid UNC path";
const wchar_t* c_too_many_rp = L"Path contains too many reparse points";
const wchar_t* c_unknown_rp_tag = L"Unknown reparse tag";

void clean_path(UnicodeString& path) {
  if (path.equal(0, L"\\??\\") || path.equal(0, L"\\\\?\\")) path.remove(0, 4);
  unsigned i = 0;
  while (i < path.size()) {
    if (path[i] == L'/') path.item(i) = L'\\';
    if ((i > 1) && (path[i] == L'\\') && (path[i - 1] == L'\\')) path.remove(i, 1);
    else i++;
  }
  path = del_trailing_slash(path);
}

// expand symbolic links / mount points and find real file path
// possible return values:
// c:\path\...
// \\?\Volume{1edbf4ba-e9fe-11dc-86b3-005056c00008}\path\...
// \\?\UNC\server\share\path\...
UnicodeString get_real_path(const UnicodeString& fp) {
  UnicodeString path = fp;
  clean_path(path);

  // junction processing phase flag
  bool jnk_phase = false;

  // number of reparse points already processed
  unsigned rp_cnt = 0;

  while (true) {
    DBG_LOG(L"path = '" + path + L"'");

    // find volume name path component
    unsigned p = path.search(L'\\');
    // if path consists of volume name only
    if (p == -1) p = path.size();

    // check if DOS device (SUBST drive)
    if (p == 2) {
      const unsigned c_buf_size = MAX_PATH + 1;
      UnicodeString target_path;
      if (QueryDosDeviceW(path.left(p).data(), target_path.buf(c_buf_size), c_buf_size) != 0) {
        target_path.set_size();
        if (target_path.equal(0, L"\\??\\")) {
          DBG_LOG(L"QueryDosDevice('" + path.left(p) + L"') = '" + target_path + L"'");
          clean_path(target_path);
          // need to check for sym. links in target_path
          jnk_phase = false;
          // process new path
          path.replace(0, p, target_path);
          continue;
        }
      }
    }

    // process other file path components
    while (true) {

      // if entire path is processed
      if (p == path.size()) {
        if (jnk_phase) {
          // all reparse points are expanded
          if (path.equal(0, L"Volume")) {
            // try to replace volume name with drive letter
            p = path.search(L'\\');
            if (p == -1) p = path.size();

            wchar_t drive_str[MAX_PATH];
            CHECK_SYS(GetLogicalDriveStringsW(MAX_PATH, drive_str));

            UnicodeString volume, drive;

            unsigned i = 0;
            while (drive_str[i] != 0) {
              if (GetVolumeNameForVolumeMountPointW(drive_str + i, volume.buf(MAX_PATH), MAX_PATH) != 0) {
                volume.set_size();
                clean_path(volume);
                if (path.equal(0, p, volume)) {
                  drive = drive_str + i;
                  clean_path(drive);
                  path.replace(0, p, drive);
                  DBG_LOG(L"path = '" + path + L"'");
                  break;
                }
              }
              i += (unsigned) wcslen(drive_str + i) + 1;
            }
          }
          // check if path points to network drive
          // convert to UNC path if so
          if (path.equal(1, L':')) {
            Array<unsigned char> unc_buf;
            const unsigned c_unc_buf_size = 1024;
            DWORD unc_buf_size = c_unc_buf_size;
            if (WNetGetUniversalNameW(path.left(2).data(), UNIVERSAL_NAME_INFO_LEVEL, unc_buf.buf(unc_buf_size), &unc_buf_size) == NO_ERROR) {
              unc_buf.set_size(unc_buf_size);
              UNIVERSAL_NAME_INFOW* uni = (UNIVERSAL_NAME_INFOW*) unc_buf.data();
              path.replace(0, 2, uni->lpUniversalName);
              path.replace(0, 1, L"UNC");
              DBG_LOG(L"path = '" + path + L"'");
            }
          }
          // prepend special prefix if path is not starting with a disk letter
          if (!path.equal(1, L':')) path.insert(0, L"\\\\?\\");
          // done
          DBG_LOG(L"get_real_path() = '" + path + L"'");
          return path;
        }
        else {
          // all sym. links are expanded, proceed with junctions
          jnk_phase = true;
          break;
        }
      }

      // if UNC path
      if (path.equal(0, L"\\\\")) path.replace(0, 1, L"UNC");
      if (path.equal(0, L"UNC\\")) {
        // skip server
        unsigned s = path.search(4, L'\\');
        CHECK(s != -1, c_invalid_unc_path);
        // skip share
        s = path.search(s + 1, L'\\');
        if (s == -1) s = path.size();
        if (p < s) p = s;
        else {
          // select next path component
          p = path.search(p + 1, L'\\');
          if (p == -1) p = path.size();
        }
      }
      // if normal file path
      else {
        // select next path component
        p = path.search(p + 1, L'\\');
        if (p == -1) p = path.size();
      }

      DBG_LOG(L"path part = '" + path.left(p) + L"'");

      // get file path component information
      DWORD attr = GetFileAttributesW((L"\\\\?\\" + path.left(p)).data());
      CHECK_SYS(attr != INVALID_FILE_ATTRIBUTES);

      // if reparse point
      if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) {

        // error if path contains too many reparse points
        rp_cnt++;
        CHECK(rp_cnt <= c_max_rp, c_too_many_rp);

        Array<u8> reparse_buffer;
        REPARSE_DATA_BUFFER* reparse_data;
        DWORD reparse_data_size;

        // get reparse data
        HANDLE h_file = CreateFileW((L"\\\\?\\" + path.left(p)).data(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_POSIX_SEMANTICS, NULL);
        CHECK_SYS(h_file != INVALID_HANDLE_VALUE);
        try {
          unsigned reparse_buffer_max_size = MAXIMUM_REPARSE_DATA_BUFFER_SIZE + offsetof(REPARSE_DATA_BUFFER, GenericReparseBuffer);
          CHECK_SYS(DeviceIoControl(h_file, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse_buffer.buf(reparse_buffer_max_size), reparse_buffer_max_size, &reparse_data_size, NULL));
          reparse_buffer.set_size(reparse_data_size);
          reparse_data = (REPARSE_DATA_BUFFER*) reparse_buffer.data();
        }
        finally (CloseHandle(h_file));

        // process mount point
        if (reparse_data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
          if (jnk_phase) {
            UnicodeString link_path((wchar_t*) ((u8*) reparse_data +
              offsetof(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) +
              reparse_data->MountPointReparseBuffer.SubstituteNameOffset),
              reparse_data->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t));
            DBG_LOG(L"Mount Point = '" + link_path + L"'");
            clean_path(link_path);
            jnk_phase = false;
            path.replace(0, p, link_path);
            break;
          }
        } // IO_REPARSE_TAG_MOUNT_POINT
        // process symbolic link
        else if (reparse_data->ReparseTag == IO_REPARSE_TAG_SYMBOLIC_LINK) {
          if (!jnk_phase) {
            UnicodeString link_path((wchar_t*) ((u8*) reparse_data +
              offsetof(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) +
              reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset),
              reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t));
            DBG_LOG(L"Sym Link = '" + link_path + L"'");
            // if absolute symlink
            if (link_path.equal(0, L"\\??\\")) {
              clean_path(link_path);
              path.replace(0, p, link_path);
            }
            // if relative symlink
            else {
              clean_path(link_path);
              UnicodeString new_path = path.left(p);
              // link is relative to root directory
              if (link_path.equal(0, L'\\')) {
                // replace current file components with link path preserving volume name
                unsigned idx = new_path.search(L'\\');
                if (idx == -1) idx = new_path.size();
                // replace current path with link path preserving volume name
                new_path.replace(idx, new_path.size() - idx, link_path);
                path.replace(0, p, new_path);
              }
              // link is relative to current path
              else {
                // remove last component from path
                unsigned idx = new_path.rsearch(L'\\');
                CHECK(idx != -1, c_invalid_symlink_path);
                new_path.remove(idx, new_path.size() - idx);

                unsigned idx1 = 0;
                while (idx1 < link_path.size()) {
                  // extract rel. path component
                  unsigned idx2 = link_path.search(idx1, L'\\');
                  // if last path component found
                  if (idx2 == -1) idx2 = link_path.size();
                  // if current directory
                  if (link_path.equal(idx1, idx2 - idx1, L".")) {
                    // just skip
                  }
                  // if parent directory
                  else if (link_path.equal(idx1, idx2 - idx1, L"..")) {
                    // if UNC path
                    if ((new_path.size() >= 3) && new_path.equal(0, 3, L"UNC")) {
                      // check that there is at least one directory
                      unsigned idx = new_path.search(L'\\');
                      CHECK(idx != -1, c_invalid_symlink_path);
                      idx = new_path.search(idx + 1, L'\\');
                      CHECK(idx != -1, c_invalid_symlink_path);
                      idx = new_path.search(idx + 1, L'\\');
                      CHECK(idx != -1, c_invalid_symlink_path);
                      idx = new_path.search(idx + 1, L'\\');
                      CHECK(idx != -1, c_invalid_symlink_path);
                    }
                    // remove last abs. path component
                    unsigned idx = new_path.rsearch(L'\\');
                    CHECK(idx != -1, c_invalid_symlink_path);
                    new_path.remove(idx, new_path.size() - idx);
                  }
                  // other directory
                  else {
                    // add it to path
                    new_path.add(L'\\').add(link_path.data() + idx1, idx2 - idx1);
                  }
                  idx1 = idx2 + 1;
                }
                // process new path
                path.replace(0, p, new_path);
              }
            } // relative symlink
            break;
          }
        } // IO_REPARSE_TAG_SYMBOLIC_LINK
        else {
          // if unknown reparse tag
          CHECK(false, c_unknown_rp_tag);
        }
      } // if reparse point
    } // while
  } // while
}

void NtfsVolume::open(const UnicodeString& volume_name) {
  close();
  try {
    name = volume_name;

    /* get volume information */
    wchar_t vlm_label[MAX_PATH];
    DWORD vlm_comp_len;
    DWORD flags;
    wchar_t vlm_fs[MAX_PATH];
    CHECK_SYS(GetVolumeInformationW(add_trailing_slash(name).data(), vlm_label, sizeof(vlm_label), &serial, &vlm_comp_len, &flags, vlm_fs, sizeof(vlm_fs)));

    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD NumberOfFreeClusters;
    DWORD TotalNumberOfClusters;
    CHECK_SYS(GetDiskFreeSpaceW(add_trailing_slash(name).data(), &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters));
    cluster_size = SectorsPerCluster * BytesPerSector;

    /* allocate volume handle */
    handle = CreateFileW((name.equal(0, L"\\\\?\\") ? name : (L"\\\\.\\" + name)).data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    CHECK_SYS(handle != INVALID_HANDLE_VALUE);
  }
  catch (...) {
    close();
    throw;
  }
}
