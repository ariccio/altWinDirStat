#pragma once

typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
typedef __int8 s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

#ifdef DEBUG
#  define DBG_LOG(msg) OutputDebugStringW(((msg) + L'\n').data())
#else
#  define DBG_LOG(msg)
#endif

UnicodeString extract_path_root(const UnicodeString& path);
UnicodeString extract_file_name(const UnicodeString& path);
UnicodeString extract_file_path(const UnicodeString& path);
UnicodeString long_path(const UnicodeString& path);
UnicodeString add_trailing_slash(const UnicodeString& file_path);
UnicodeString del_trailing_slash(const UnicodeString& file_path);
