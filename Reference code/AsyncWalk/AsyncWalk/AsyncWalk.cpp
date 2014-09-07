#include "AsyncWalk.h"

//std::unique_ptr<File> m_rootFile;

std::wstring File::UpwardGetPathWithoutBackslash( ) const {
	std::wstring path;
	auto myParent = m_parent;
	if ( myParent != NULL ) {
		path = myParent->UpwardGetPathWithoutBackslash( );
		}
	else if ( myParent == nullptr ) {
		path = m_rootPath + m_FileInfo.name;
		return path;
		}
	switch ( m_type )
	{
		case IT_DIRECTORY:
			if ( !path.empty( ) ) {
				path += _T( "\\" );
				}
			path += m_FileInfo.name;
			break;

		case IT_FILE:
			path += _T("\\") + m_FileInfo.name;
			break;

	}
	return path; 
	}

std::wstring UpwardGetPathWithoutBackslash( File* aFile ) {
	std::wstring path;
	auto myParent = aFile->m_parent;
	if ( myParent != NULL ) {
		path = myParent->UpwardGetPathWithoutBackslash( );
		}
	else if ( myParent == nullptr ) {
		path = aFile->m_rootPath + aFile->m_FileInfo.name;
		return path;
		}
	switch ( aFile->m_type )
	{
		case IT_DIRECTORY:
			if ( !path.empty( ) ) {
				path += _T( "\\" );
				}
			path += aFile->m_FileInfo.name;
			break;

		case IT_FILE:
			path += _T("\\") + aFile->m_FileInfo.name;
			break;

	}
	return path; 
	}

void trace_fDataBits( _In_ const WIN32_FIND_DATA& fData, _In_ const std::wstring& normSzDir ) {
	if ( !( ( fData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) || (fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA) ) ) {
		//TOO MANY DAMN HIDDEN FILES!
		std::wcout << std::endl << L"\tWeird file encountered in " << normSzDir << std::endl << L"\tWeird file attributes:";

		std::wcout << TRACE_OUT( fData.cFileName ) << std::endl;
		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE              ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE              ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED           ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED           ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM    ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM    ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA       ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA       ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE             ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE             ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT       ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT       ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE         ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE         ) );
			}

		if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL             ) ) {
			std::wcout << TRACE_OUT( ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL             ) );
			}
			//std::wcout << std::endl;
		}
	}

std::int64_t stdRecurseFindFutures( _In_ std::wstring dir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, File* theFile );

std::int64_t descendDirectory( _Inout_ WIN32_FIND_DATA fData, _In_ const std::wstring normSzDir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, File* theFile ) {
	std::wstring newSzDir = normSzDir;//MUST operate on copy!
	newSzDir.reserve( MAX_PATH );
	newSzDir += L"\\";
	newSzDir += fData.cFileName;
	std::int64_t num = 0;
	num += stdRecurseFindFutures( newSzDir, isLargeFetch, isBasicInfo, theFile );
	return num;
	}

std::int64_t stdRecurseFindFutures( _In_ std::wstring dir, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, File* theFile ) {

	std::int64_t num = 0;
	dir.reserve( MAX_PATH );
	std::wstring normSzDir(dir);
	normSzDir.reserve( MAX_PATH );
	if ( ( dir.back( ) != L'*' ) && ( dir.at( dir.length( ) - 2 ) != L'\\' ) ) {
		dir += L"\\*";
		}
	else if ( dir.back( ) == L'\\' ) {
		dir += L"*";
		}
	std::vector<std::future<std::int64_t>> futureDirs;
	futureDirs.reserve( 100 );//pseudo-arbitrary number
	WIN32_FIND_DATA fData;
	HANDLE fDataHand = NULL;

	if ( isLargeFetch ) {
		if ( isBasicInfo ) {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic,    &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
			}
		else {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH );
			}
		}
	else {
		if ( isBasicInfo ) {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoBasic,    &fData, FindExSearchNameMatch, NULL, 0 );
			}
		else {
			fDataHand = FindFirstFileExW( dir.c_str( ), FindExInfoStandard, &fData, FindExSearchNameMatch, NULL, 0 );
			}
		}
	if ( fDataHand != INVALID_HANDLE_VALUE ) {
		if ( !( wcscmp( fData.cFileName, L".." ) == 0 ) ) {
			++num;
			}
		auto res = FindNextFileW( fDataHand, &fData );
		while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( res != 0 ) ) {
			auto newFile = std::make_unique<File>( );
			if ( !std::wcout.good()) {//Slower than it should be.
				auto badBits    = std::wcout.exceptions( );
				auto wasBadBit  = badBits & std::ios_base::badbit;
				auto wasFailBit = badBits & std::ios_base::failbit;
				auto wasEofBit  = badBits & std::ios_base::eofbit;
				std::wcout.clear( );
				std::wcout << L"wcout was in a bad state!" << std::endl;
				std::wcout << TRACE_OUT( wasBadBit );
				std::wcout << TRACE_OUT( wasFailBit );
				std::wcout << TRACE_OUT( wasEofBit );
				std::wcout << std::endl;
				}
			auto scmpVal = wcscmp( fData.cFileName, L".." );
			if ( ( !( scmpVal == 0 ) ) ) {
				newFile->m_done = false;
				newFile->m_FileInfo.attributes = fData.dwFileAttributes;
				newFile->m_FileInfo.lastWriteTime = fData.ftLastWriteTime;
				if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ) ) {
					ULARGE_INTEGER ret;
					ret.QuadPart = 0;//it's a union, but I'm being careful.
					std::wstring fileNameStr( dir.substr( 0, dir.length( ) - 1 ) + std::wstring( fData.cFileName ) );
					ret.LowPart = GetCompressedFileSize( fileNameStr.c_str( ), &ret.HighPart );
					if ( ( ret.LowPart == INVALID_FILE_SIZE ) ) {
						if ( ret.HighPart != NULL ) {
							if ( ( GetLastError( ) != NO_ERROR ) ) {
								newFile->m_FileInfo.m_size = ret.QuadPart;
								}
							else {
								newFile->m_FileInfo.m_size = ( std::uint64_t( fData.nFileSizeHigh ) * ( std::uint64_t( MAXDWORD )+ 1 ) ) + std::uint64_t( fData.nFileSizeLow );
								}
							}
						else if ( GetLastError( ) != NO_ERROR ) {
							newFile->m_FileInfo.m_size = ( std::uint64_t( fData.nFileSizeHigh ) * ( std::uint64_t( MAXDWORD )+ 1 ) ) + std::uint64_t( fData.nFileSizeLow );
							}
						}
					else {
						newFile->m_FileInfo.m_size = ret.QuadPart;
						}
					}
				else {
					newFile->m_FileInfo.m_size = ( std::uint64_t( fData.nFileSizeHigh ) * ( std::uint64_t( MAXDWORD )+ 1 ) ) + std::uint64_t( fData.nFileSizeLow );
					}
				
				
				newFile->m_FileInfo.name = fData.cFileName;
				newFile->m_files = 0;
				newFile->m_isRootItem = false;
				newFile->m_parent = theFile;
				newFile->m_readJobs = 0;
				newFile->m_subdirs = 0;
				newFile->m_readJobDone = false;
				++num;				
				}
			if ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && ( scmpVal != 0 ) ) {
				newFile->m_type = IT_DIRECTORY;
				//std::wcout << L"Directory: " << newFile->UpwardGetPathWithoutBackslash( ) << std::endl;
				theFile->AddChild( newFile.get( ) );
				theFile->m_children.emplace_back( std::move( newFile ) );
				futureDirs.emplace_back( std::async( std::launch::async | std::launch::deferred, descendDirectory, fData, normSzDir, isLargeFetch, isBasicInfo, theFile->m_children.back( ).get( ) ) );
				}
			else if ( ( scmpVal != 0 ) ) {
				newFile->m_type = IT_FILE;
				theFile->AddChild( newFile.get( ) );
				//std::wcout << L"File: " << newFile->UpwardGetPathWithoutBackslash( ) << std::endl;
				theFile->m_children.emplace_back( std::move( newFile ) );
				}
			else if ( ( scmpVal != 0 ) && ( ( ( fData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE ) || ( fData.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL ) ) ) ) {
				
				trace_fDataBits( fData, normSzDir );
				}

			res = FindNextFileW( fDataHand, &fData );
			}
		}
	for ( auto& a : futureDirs ) {
		num += a.get( );
		}
	FindClose( fDataHand );
	return num;
	}

void File::UpwardAddSize( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t bytes ) {
	if ( bytes < 0 ) {
		if ( ( bytes + std::int64_t( m_FileInfo.m_size ) ) < 0 ) {
			m_FileInfo.m_size = 0;
			}
		else {
			m_FileInfo.m_size -= std::uint64_t( bytes * ( -1 ) );
			}
		auto myParent = m_parent;
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		}
	else {
		m_FileInfo.m_size += std::uint64_t( bytes );
		auto myParent = m_parent;
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		//else `this` may be the root item.
		}
	}


void File::AddChild( File* aChild ) {
	if ( aChild->m_type == IT_FILE ) {
		++m_files;
		}
	if ( aChild->m_type == IT_DIRECTORY ) {
		++m_subdirs;
		}
	UpwardAddSize( aChild->m_FileInfo.m_size );
	}

//std::uint64_t sumChildren( File* aFile ) {
//	std::uint64_t size = 0;
//	size += aFile->m_FileInfo.m_size;
//	for ( auto& aChild : aFile->m_children ) {
//		if ( aChild->m_type == IT_FILE ) {
//			size += aChild->m_FileInfo.m_size;
//			}
//		else {
//			size += sumChildren( aChild.get( ) );
//			}
//		}
//	return size;
//	}
std::uint64_t File::CountDepth( ) {
	std::uint64_t depth = 0;
	auto parent = m_parent;
	if ( parent != NULL ) {
		return parent->CountDepth( ) + 1;
		}
	return depth;
	}

void printTree( File* treeRoot ) {
	auto depth = treeRoot->CountDepth( );
	std::wcout << L"-";
	for ( std::uint64_t i = 0; i < depth; ++i ) {
		std::wcout << L"-";
		}
	std::wcout << L">" << std::setw( 10 ) << treeRoot->m_FileInfo.m_size << L", " << treeRoot->m_FileInfo.name << std::endl;
	for ( auto& aChild : treeRoot->m_children ) {
		printTree( aChild.get( ) );
		}
	}


int stdWorkAsync( _In_ std::wstring arg, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, const bool DoPrintTree = false ) {
	std::wcout << L"Working on: `" << arg << L"`" << std::endl;
	std::int64_t numberFiles = 0;
	arg.reserve( MAX_PATH );
	WIN32_FIND_DATA fData;

	if ( arg.length( ) > 3 ) {
		auto strCmp = ( arg.compare( 0, 4, arg, 0, 4 ) );
		if ( strCmp != 0 ) {
			arg = L"\\\\?\\" + arg;
			std::wcout << L"prefixed `" << arg << L"`, value now: `" << arg << L"`" << std::endl << std::endl;
			}
		}
	else {
		arg = L"\\\\?\\" + arg;
		std::wcout << L"prefixed `" << arg << L"`, value now: `" << arg << L"`" << std::endl << std::endl;
		}
	HANDLE fDataHand = FindFirstFile( arg.c_str( ), &fData );
	auto m_rootFile = std::make_unique < File >( ) ;
	m_rootFile->m_done = false;
	m_rootFile->m_files = 0;
	m_rootFile->m_isRootItem = true;
	m_rootFile->m_parent = nullptr;
	m_rootFile->m_readJobDone = false;
	m_rootFile->m_readJobs = 1;
	m_rootFile->m_subdirs = 0;
	m_rootFile->m_rootPath = arg;
	m_rootFile->m_FileInfo.attributes = fData.dwFileAttributes;
	m_rootFile->m_FileInfo.lastWriteTime = fData.ftLastWriteTime;
	m_rootFile->m_FileInfo.name = fData.cFileName;
	m_rootFile->m_FileInfo.m_size = ( std::uint64_t( fData.nFileSizeHigh ) * ( std::uint64_t( MAXDWORD )+ 1 ) ) + std::uint64_t( fData.nFileSizeLow );
	if ( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		m_rootFile->m_type = IT_DIRECTORY;
		}
	else {
		std::wcerr << L"FATAL! " << arg << L" is not a directory!" << std::endl;
		return 666;
		}
	
	numberFiles = stdRecurseFindFutures( arg, isLargeFetch, isBasicInfo, m_rootFile.get( ) );
	std::wcout << std::endl << L"Number of items: " << numberFiles << std::endl;
	//std::wcout << L"Total size of children: " << sumChildren( m_rootFile.get( ) ) << std::endl;
	std::wcout << L"->" << m_rootFile->UpwardGetPathWithoutBackslash( ) << std::endl;
	if ( DoPrintTree ) {
		printTree( m_rootFile.get( ) );
		}
	return 0;
	}


const DOUBLE getAdjustedTimingFrequency( ) {
	LARGE_INTEGER timingFrequency;
	BOOL res1 = QueryPerformanceFrequency( &timingFrequency );
	if ( !res1 ) {
		std::wcout << L"QueryPerformanceFrequency failed!!!!!! Disregard any timing data!!" << std::endl;
		}
	const DOUBLE adjustedTimingFrequency = ( DOUBLE( 1.00 ) / DOUBLE( timingFrequency.QuadPart ) );
	return adjustedTimingFrequency;
	}



int iterate( _In_ const std::wstring& arg, _In_ const bool isLargeFetch, _In_ const bool isBasicInfo, _Inout_ std::wstringstream& ss ) {
	std::wcout << L"--------------------------------------------" << std::endl;
	if ( isLargeFetch ) {
		ss << L"Iteration WITH    FIND_FIRST_EX_LARGE_FETCH, ";
		}
	else {
		ss << L"Iteration without FIND_FIRST_EX_LARGE_FETCH, ";
		}
	if ( isBasicInfo ) {
		ss << L" WITH    isBasicInfo, ";
		}
	else {
		ss << L" without isBasicInfo, ";
		}

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;
	
	//std::int64_t fileSizeTotal = 0;
	auto adjustedTimingFrequency = getAdjustedTimingFrequency( );

	BOOL res2 = QueryPerformanceCounter( &startTime );
	auto res = stdWorkAsync( arg, isLargeFetch, isBasicInfo );
	BOOL res3 = QueryPerformanceCounter( &endTime );
	
	if ( ( !res2 ) || ( !res3 ) ) {
		std::wcout << L"QueryPerformanceCounter Failed!!!!!! Disregard any timing data!!" << std::endl;
		}

	auto totalTime = ( endTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;
	ss << L"Time in seconds:  " << totalTime << std::endl;
	std::wcout << std::endl;
	Sleep( 500 );
	return res;
	}

int main( ) {
	std::wstringstream ss;
	auto res = iterate( L"C:\\Users\\Alexander Riccio\\Documents", true, false, ss );
	std::wcout << ss.str( ) << std::endl;
	return res;
	}