#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>
#include <strsafe.h>
#include <vector>
#include <cstdint>
#include <memory>
#include <ios>
#include <exception>
#include <future>
#include <queue>

#define TRACE_OUT(x) std::endl << L"\t\t" << #x << L" = `" << x << L"` "//awesomely useful macro, included now, just in case I need it later.

#define TRACE_STR(x) << L" " << #x << L" = `" << x << L"`;"

enum ITEMTYPE : std::uint8_t {
	IT_DIRECTORY,		// Folder
	IT_FILE,			// Regular file
	};

struct FILEINFO {
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t m_size;
	FILETIME      lastWriteTime;
	DWORD         attributes;
	std::wstring  name;
	};

class File {
	public:
		File*                    m_parent = nullptr;
		ITEMTYPE                 m_type;
		bool                     m_readJobDone : 1;
		bool                     m_done        : 1;     // Whole Subtree is done.
		bool                     m_isRootItem  : 1;
		FILEINFO                 m_FileInfo;
		_Field_range_( 0, 4294967295 ) std::uint32_t                           m_files;			// # Files in subtree
		_Field_range_( 0, 4294967295 ) std::uint32_t                           m_subdirs;		// # Folder in subtree
		_Field_range_( 0, 4294967295 ) std::uint32_t                           m_readJobs;		// # "read jobs" in subtree.
		                               std::vector<std::unique_ptr<File>>      m_children;

									   std::wstring m_rootPath;
	std::wstring  UpwardGetPathWithoutBackslash (                                          ) const;
	void AddChild( File* child );
	void UpwardAddSize( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t      bytes );
	std::uint64_t CountDepth( );
	//void UpwardAddFiles( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t      fileCount );
	};