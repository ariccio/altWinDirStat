#ifndef _NTFS_H
#define _NTFS_H

#pragma pack (push, 1)

typedef enum {
  magic_FILE = 0x454c4946,
  magic_INDX = 0x58444e49,
  magic_HOLE = 0x454c4f48,
  magic_RSTR = 0x52545352,
  magic_RCRD = 0x44524352,
  magic_CHKD = 0x444b4843,
  magic_BAAD = 0x44414142,
  magic_empty = 0xffffffff,
} NTFS_RECORD_TYPES;

typedef enum {
  MFT_RECORD_IN_USE = 0x0001,
  MFT_RECORD_IS_DIRECTORY = 0x0002,
  MFT_RECORD_IS_4 = 0x0004,
  MFT_RECORD_IS_VIEW_INDEX = 0x0008,
  MFT_REC_SPACE_FILLER = 0xffff,
} MFT_RECORD_FLAGS;

typedef struct {
  u32 magic;
  u16 usa_ofs;
  u16 usa_count;
  u64 lsn;
  u16 sequence_number;
  u16 link_count;
  u16 attrs_offset;
  u16 flags;
  u32 bytes_in_use;
  u32 bytes_allocated;
  u64 base_mft_record;
  u16 next_attr_instance;
//  u16 reserved;
//  u32 mft_record_number;
} MFT_RECORD;

typedef enum {
  AT_UNUSED = 0,
  AT_STANDARD_INFORMATION = 0x10,
  AT_ATTRIBUTE_LIST = 0x20,
  AT_FILE_NAME = 0x30,
  AT_OBJECT_ID = 0x40,
  AT_SECURITY_DESCRIPTOR = 0x50,
  AT_VOLUME_NAME = 0x60,
  AT_VOLUME_INFORMATION = 0x70,
  AT_DATA = 0x80,
  AT_INDEX_ROOT = 0x90,
  AT_INDEX_ALLOCATION = 0xa0,
  AT_BITMAP = 0xb0,
  AT_REPARSE_POINT = 0xc0,
  AT_EA_INFORMATION = 0xd0,
  AT_EA = 0xe0,
  AT_PROPERTY_SET = 0xf0,
  AT_LOGGED_UTILITY_STREAM = 0x100,
  AT_FIRST_USER_DEFINED_ATTRIBUTE = 0x1000,
  AT_END = 0xffffffff,
} ATTR_TYPES;

#define ATTR_TYPE_DEF(name) { AT_##name, L#name },

const struct {
  u32 type;
  const wchar_t* name;
} c_attr_types[] = {
  ATTR_TYPE_DEF(STANDARD_INFORMATION)
  ATTR_TYPE_DEF(ATTRIBUTE_LIST)
  ATTR_TYPE_DEF(FILE_NAME)
  ATTR_TYPE_DEF(OBJECT_ID)
  ATTR_TYPE_DEF(SECURITY_DESCRIPTOR)
  ATTR_TYPE_DEF(VOLUME_NAME)
  ATTR_TYPE_DEF(VOLUME_INFORMATION)
  ATTR_TYPE_DEF(DATA)
  ATTR_TYPE_DEF(INDEX_ROOT)
  ATTR_TYPE_DEF(INDEX_ALLOCATION)
  ATTR_TYPE_DEF(BITMAP)
  ATTR_TYPE_DEF(REPARSE_POINT)
  ATTR_TYPE_DEF(EA_INFORMATION)
  ATTR_TYPE_DEF(EA)
  ATTR_TYPE_DEF(PROPERTY_SET)
  ATTR_TYPE_DEF(LOGGED_UTILITY_STREAM)
};

#undef ATTR_TYPE_DEF

typedef enum {
  ATTR_IS_COMPRESSED = 0x0001,
  ATTR_IS_ENCRYPTED = 0x4000,
  ATTR_IS_SPARSE = 0x8000,
} ATTR_FLAGS;

typedef enum {
  RESIDENT_ATTR_IS_INDEXED = 0x01,
} RESIDENT_ATTR_FLAGS;

typedef struct {
  u32 type;
  u32 length;
  u8 non_resident;
  u8 name_length;
  u16 name_offset;
  u16 flags;
  u16 instance;
} ATTR_HEADER;

typedef struct {
  u32 value_length;
  u16 value_offset;
  u8 resident_flags;
  u8 padding;
} ATTR_RESIDENT;

typedef struct {
  u64 lowest_vcn;
  u64 highest_vcn;
  u16 mapping_pairs_offset;
  u8 compression_unit;
  u8 padding[5];
  u64 allocated_size;
  u64 data_size;
  u64 initialized_size;
} ATTR_NONRESIDENT;

typedef struct {
  u64 lowest_vcn;
  u64 highest_vcn;
  u16 mapping_pairs_offset;
  u8 compression_unit;
  u8 padding[5];
  u64 allocated_size;
  u64 data_size;
  u64 initialized_size;
  u64 compressed_size;
} ATTR_COMPRESSED;

typedef struct {
  u32 type;
  u16 length;
  u8 name_length;
  u8 name_offset;
  u64 lowest_vcn;
  u64 mft_reference;
  u16 instance;
} ATTR_LIST_ENTRY;

typedef enum {
  FILE_ATTR_READONLY = 0x00000001,
  FILE_ATTR_HIDDEN = 0x00000002,
  FILE_ATTR_SYSTEM = 0x00000004,
  FILE_ATTR_DIRECTORY = 0x00000010,
  FILE_ATTR_ARCHIVE = 0x00000020,
  FILE_ATTR_DEVICE = 0x00000040,
  FILE_ATTR_NORMAL = 0x00000080,
  FILE_ATTR_TEMPORARY = 0x00000100,
  FILE_ATTR_SPARSE_FILE = 0x00000200,
  FILE_ATTR_REPARSE_POINT = 0x00000400,
  FILE_ATTR_COMPRESSED = 0x00000800,
  FILE_ATTR_OFFLINE = 0x00001000,
  FILE_ATTR_NOT_CONTENT_INDEXED = 0x00002000,
  FILE_ATTR_ENCRYPTED = 0x00004000,
  FILE_ATTR_VALID_FLAGS = 0x00007fb7,
  FILE_ATTR_VALID_SET_FLAGS = 0x000031a7,
  FILE_ATTR_I30_INDEX_PRESENT = 0x10000000,
  FILE_ATTR_VIEW_INDEX_PRESENT = 0x20000000,
} FILE_ATTR_FLAGS;

typedef enum {
  FILE_NAME_POSIX = 0x00,
  FILE_NAME_WIN32 = 0x01,
  FILE_NAME_DOS = 0x02,
  FILE_NAME_WIN32_AND_DOS = 0x03,
} FILE_NAME_TYPE_FLAGS;

typedef struct {
  u64 parent_directory;
  u64 creation_time;
  u64 last_data_change_time;
  u64 last_mft_change_time;
  u64 last_access_time;
  u64 allocated_size;
  u64 data_size;
  u32 file_attributes;
  union {
    struct {
      u16 packed_ea_size;
      u16 reserved;
    };
    u32 reparse_point_tag;
  };
  u8 file_name_length;
  u8 file_name_type;
} FILE_NAME_ATTR;

typedef struct {
  u64 creation_time;
  u64 last_data_change_time;
  u64 last_mft_change_time;
  u64 last_access_time;
  u32 file_attributes;
} STANDARD_INFORMATION_ATTR;

typedef enum {
  IO_REPARSE_TAG_IS_ALIAS = 0x20000000,
  IO_REPARSE_TAG_IS_HIGH_LATENCY = 0x40000000,
  IO_REPARSE_TAG_IS_MICROSOFT = 0x80000000,

//  IO_REPARSE_TAG_RESERVED_ZERO = 0x00000000,
//  IO_REPARSE_TAG_RESERVED_ONE = 0x00000001,
//  IO_REPARSE_TAG_RESERVED_RANGE = 0x00000001,

  IO_REPARSE_TAG_NSS = 0x68000005,
  IO_REPARSE_TAG_NSS_RECOVER = 0x68000006,
//  IO_REPARSE_TAG_SIS = 0x68000007,
//  IO_REPARSE_TAG_DFS = 0x68000008,

//  IO_REPARSE_TAG_MOUNT_POINT = 0x88000003,

//  IO_REPARSE_TAG_HSM = 0xa8000004,

  IO_REPARSE_TAG_SYMBOLIC_LINK = 0xa000000c,

  IO_REPARSE_TAG_VALID_VALUES = 0xe000ffff,
} PREDEFINED_REPARSE_TAGS;

typedef struct {
  u32 reparse_tag;
  u16 reparse_data_length;
  u16 reserved;
} REPARSE_POINT;

typedef struct {
  u16 subst_name_off;
  u16 subst_name_len;
  u16 print_name_off;
  u16 print_name_len;
} MOUNT_POINT;

typedef struct {
  u16 subst_name_off;
  u16 subst_name_len;
  u16 print_name_off;
  u16 print_name_len;
  u32 flags; // ??
} SYMBOLIC_LINK;

typedef struct _REPARSE_DATA_BUFFER {
  u32 ReparseTag;
  u16 ReparseDataLength;
  u16 Reserved;
  union {
    struct {
      u16 SubstituteNameOffset;
      u16 SubstituteNameLength;
      u16 PrintNameOffset;
      u16 PrintNameLength;
      WCHAR PathBuffer[1];
    } MountPointReparseBuffer;
    struct {
      u16 SubstituteNameOffset;
      u16 SubstituteNameLength;
      u16 PrintNameOffset;
      u16 PrintNameLength;
      u32 flags;
      WCHAR PathBuffer[1];
    } SymbolicLinkReparseBuffer;
    struct {
      u8 DataBuffer[1];
    } GenericReparseBuffer;
  };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define FILE_REF(ref) ((ref) & 0x0000FFFFFFFFFFFFl)
#define U64_TO_FILETIME(ft, dt) { (ft).dwLowDateTime = static_cast<DWORD>((dt) & 0xFFFFFFFF); (ft).dwHighDateTime = static_cast<DWORD>(((dt) >> 32) & 0xFFFFFFFF); }

#pragma pack (pop)

#endif /* _NTFS_H */
