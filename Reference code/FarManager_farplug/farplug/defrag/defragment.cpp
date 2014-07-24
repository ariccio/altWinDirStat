#include <windows.h>
#include <winioctl.h>

#include "col/UnicodeString.h"
#include "col/PlainArray.h"
using namespace col;

#define _ERROR_WINDOWS
#include "error.h"

#include "utils.h"
#include "volume.h"
#include "defragment.h"

struct ClusterChain {
  unsigned __int64 lcn;
  unsigned __int64 cnt;
};

unsigned get_chain_count(const Array<ClusterChain>& cluster_chains, unsigned first_chain, unsigned __int64 total_clusters) {
  unsigned __int64 clusters_count = 0;
  unsigned chains_count = 0;

  for (unsigned i = first_chain; i < cluster_chains.size(); i++) {
    if (clusters_count >= total_clusters) {
      chains_count = i - first_chain;
      break;
    }
    clusters_count += cluster_chains[i].cnt;
  }
  return chains_count;
}

void defragment(const UnicodeString& file_name) {
  UnicodeString real_path = add_trailing_slash(get_real_path(extract_file_path(file_name))) + extract_file_name(file_name);
  NtfsVolume volume;
  volume.open(extract_path_root(real_path));
  // file fragments
  HANDLE h_file = CreateFileW(long_path(real_path).data(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_POSIX_SEMANTICS, NULL);
  CHECK_SYS(h_file != INVALID_HANDLE_VALUE);
  CLEAN(HANDLE, h_file, CHECK_SYS(CloseHandle(h_file)));
  STARTING_VCN_INPUT_BUFFER vcn_buf;
  vcn_buf.StartingVcn.QuadPart = 0;
  Array<unsigned char> extent_buf;
  Array<ClusterChain> file_extents;
  bool more = true;
  while (more) {
    DWORD out_size = 10 * 1024;
    BOOL ret = DeviceIoControl(h_file, FSCTL_GET_RETRIEVAL_POINTERS, &vcn_buf, sizeof(vcn_buf), extent_buf.buf(out_size), out_size, &out_size, NULL);
    if (ret == 0) {
      if (GetLastError() == ERROR_HANDLE_EOF) break; // resident file
      CHECK_SYS(GetLastError() == ERROR_MORE_DATA);
    }
    else more = false;
    if (out_size < sizeof(RETRIEVAL_POINTERS_BUFFER)) break; // out_size == 0 for resident directory - Windows bug?
    extent_buf.set_size(out_size);
    const RETRIEVAL_POINTERS_BUFFER* retr_ptr = (const RETRIEVAL_POINTERS_BUFFER*) extent_buf.data();
    if (retr_ptr->ExtentCount == 0) break; // just in case ...
    file_extents.extend(file_extents.size() + retr_ptr->ExtentCount);
    ClusterChain chain;
    for (unsigned i = 0; i < retr_ptr->ExtentCount; i++) {
      chain.lcn = retr_ptr->Extents[i].Lcn.QuadPart;
      chain.cnt = retr_ptr->Extents[i].NextVcn.QuadPart - (i == 0 ? retr_ptr->StartingVcn.QuadPart : retr_ptr->Extents[i - 1].NextVcn.QuadPart);
      file_extents += chain;
    }
    vcn_buf.StartingVcn = retr_ptr->Extents[retr_ptr->ExtentCount - 1].NextVcn;
  }
  unsigned __int64 extent_clusters = 0;
  unsigned total_extents = 0;
  unsigned __int64 last_lcn = -1;
  for (unsigned i = 0; i < file_extents.size(); i++) {
    if (file_extents[i].lcn != -1) {
      extent_clusters += file_extents[i].cnt;
      if (file_extents[i].lcn != last_lcn) total_extents++;
      last_lcn = file_extents[i].lcn + file_extents[i].cnt;
    }
  }
  if (total_extents > 1) {
    // volume bitmap
    STARTING_LCN_INPUT_BUFFER lcn_buf;
    lcn_buf.StartingLcn.QuadPart = 0;
    Array<unsigned char> bitmap_buf;
    DWORD out_size = sizeof(VOLUME_BITMAP_BUFFER);
    BOOL ret = DeviceIoControl(volume.handle, FSCTL_GET_VOLUME_BITMAP, &lcn_buf, sizeof(lcn_buf), bitmap_buf.buf(out_size), out_size, &out_size, NULL);
    if (ret == 0) {
      CHECK_SYS(GetLastError() == ERROR_MORE_DATA);
      bitmap_buf.set_size(out_size);
      const VOLUME_BITMAP_BUFFER* bitmap = (const VOLUME_BITMAP_BUFFER*) bitmap_buf.data();
      out_size = (DWORD) (bitmap->BitmapSize.QuadPart / 8 + (bitmap->BitmapSize.QuadPart % 8 ? 1 : 0) + offsetof(VOLUME_BITMAP_BUFFER, Buffer));
      CHECK_SYS(DeviceIoControl(volume.handle, FSCTL_GET_VOLUME_BITMAP, &lcn_buf, sizeof(lcn_buf), bitmap_buf.buf(out_size), out_size, &out_size, NULL));
    }
    bitmap_buf.set_size(out_size);
    const VOLUME_BITMAP_BUFFER* bitmap = (const VOLUME_BITMAP_BUFFER*) bitmap_buf.data();
    Array<ClusterChain> cluster_chains;
    cluster_chains.set_inc(10000);
    ClusterChain chain = { 0, 0 };
    for (unsigned i = 0; i < bitmap->BitmapSize.QuadPart / 8; i++) {
      if (bitmap->Buffer[i] == 0x00) {
        if (chain.cnt == 0) chain.lcn = (unsigned __int64) i * 8;
        chain.cnt += 8;
      }
      else if (bitmap->Buffer[i] == 0xFF) {
        if (chain.cnt != 0) {
          cluster_chains += chain;
          chain.cnt = 0;
        }
      }
      else {
        for (unsigned j = 0; j < 8; j++) {
          if (bitmap->Buffer[i] & (1 << j)) {
            if (chain.cnt != 0) {
              cluster_chains += chain;
              chain.cnt = 0;
            }
          }
          else {
            if (chain.cnt == 0) chain.lcn = (unsigned __int64) i * 8 + j;
            chain.cnt++;
          }
        }
      }
    }
    for (unsigned j = 0; j < bitmap->BitmapSize.QuadPart % 8; j++) {
      if (bitmap->Buffer[bitmap->BitmapSize.QuadPart / 8] & (1 << j)) {
        if (chain.cnt != 0) {
          cluster_chains += chain;
          chain.cnt = 0;
        }
      }
      else {
        if (chain.cnt == 0) chain.lcn = bitmap->BitmapSize.QuadPart / 8 * 8 + j;
        chain.cnt++;
      }
    }
    if (chain.cnt != 0) cluster_chains += chain;
    // find suitable sequence of free cluster chains
    struct ClusterChain_CompareCnt {
      int operator()(const ClusterChain& item1, const ClusterChain& item2) {
        if (item1.cnt > item2.cnt) return -1;
        else if (item1.cnt == item2.cnt) {
          if (item1.lcn > item2.lcn) return -1;
          else if (item1.lcn == item2.lcn) return 0;
          else return 1;
        }
        else return 1;
      }
    };
    cluster_chains.sort<ClusterChain_CompareCnt>();
    unsigned total_chains = get_chain_count(cluster_chains, 0, extent_clusters);
    if ((total_chains != 0) && (total_chains < total_extents)) {
      // mark file change in the USN
      CLEAN(HANDLE, h_file,
        USN usn;
        DWORD bytes_ret;
        DeviceIoControl(h_file, FSCTL_WRITE_USN_CLOSE_RECORD, NULL, 0, &usn, sizeof(usn), &bytes_ret, NULL);
      );
      // find smallest cluster chains
      unsigned first_chain;
      for (first_chain = 1; first_chain < cluster_chains.size(); first_chain++) {
        if (get_chain_count(cluster_chains, first_chain, extent_clusters) != total_chains) break;
      }
      first_chain--;
      cluster_chains = cluster_chains.slice(first_chain, total_chains);
      // move clusters
      assert(cluster_chains.size() != 0);
      unsigned __int64 extent_vcn = 0;
      unsigned __int64 x1 = 0;
      unsigned __int64 max_cnt = 8 * 1024 * 1024 / volume.cluster_size;
      for (unsigned i = 0; i < file_extents.size(); i++) {
        if (file_extents[i].lcn != -1) {
          for (unsigned __int64 fe_x = 0; fe_x < file_extents[i].cnt; fe_x += max_cnt) {
            unsigned __int64 fe_cnt = max_cnt;
            if (fe_x + fe_cnt > file_extents[i].cnt) fe_cnt = file_extents[i].cnt - fe_x;
            unsigned __int64 y1 = x1 + fe_cnt - 1;
            unsigned __int64 x2 = 0;
            for (unsigned j = 0; j < cluster_chains.size(); j++) {
              unsigned __int64 y2 = x2 + cluster_chains[j].cnt - 1;
              unsigned __int64 vcn;
              unsigned __int64 lcn;
              unsigned __int64 cnt;
              bool f = true;
              if (x2 > y1) {
                f = false;
              }
              else if (x2 < x1) {
                vcn = extent_vcn + fe_x;
                lcn = cluster_chains[j].lcn + (x1 - x2);
                if (y2 < x1) {
                  f = false;
                }
                else if (y2 > y1) {
                  cnt = y1 - x1 + 1;
                }
                else {
                  assert((y2 >= x1) && (y2 <= y1));
                  cnt = y2 - x1 + 1;
                }
              }
              else {
                assert((x2 >= x1) && (x2 <= y1));
                vcn = extent_vcn + fe_x + (x2 - x1);
                lcn = cluster_chains[j].lcn;
                if (y2 > y1) {
                  cnt = y1 - x2 + 1;
                }
                else {
                  assert((y2 >= x1) && (y2 <= y1));
                  cnt = y2 - x2 + 1;
                }
              }
              if (f) {
                MOVE_FILE_DATA move_data;
                move_data.FileHandle = h_file;
                move_data.StartingVcn.QuadPart = vcn;
                move_data.StartingLcn.QuadPart = lcn;
                assert(cnt <= max_cnt);
                move_data.ClusterCount = (DWORD) cnt;
                DWORD bytes_ret;
                CHECK_SYS(DeviceIoControl(volume.handle, FSCTL_MOVE_FILE, &move_data, sizeof(move_data), NULL, 0, &bytes_ret, NULL));
              }
              x2 += cluster_chains[j].cnt;
            }
            x1 += fe_cnt;
          }
        }
        extent_vcn += file_extents[i].cnt;
      }
    }
  }
}
