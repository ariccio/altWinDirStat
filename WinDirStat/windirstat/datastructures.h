//Globally instantiable datastructures

#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#else
#error ass
#endif
#include "stdafx.h"

static_assert( sizeof( short ) == sizeof( std::int16_t ), "y'all ought to check SRECT" );
struct SRECT {
	/*
	  short-based RECT, saves 8 bytes compared to tagRECT
	  */
	SRECT( ) : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) { }
	SRECT( std::int16_t iLeft, std::int16_t iTop, std::int16_t iRight, std::int16_t iBottom ) : left { iLeft }, top { iTop }, right { iRight }, bottom { iBottom } { }
	SRECT( const SRECT& in ) {
		left = in.left;
		top = in.top;
		right = in.right;
		bottom = in.bottom;
		}
	SRECT( const CRect& in ) {
		left = std::int16_t( in.right );
		top = std::int16_t( in.top );
		right = std::int16_t( in.right );
		bottom = std::int16_t( in.bottom );
		}
	static CRect BuildCRect( const SRECT& in ) {
		//ASSERT( ( in.left != -1 ) && ( in.top != -1 ) && ( in.right != -1 ) && ( in.bottom != -1 ) );
		ASSERT( ( in.right + 1 ) >= in.left );
		ASSERT( in.bottom >= in.top );
		CRect out;
		out.left = LONG( in.left );
		out.top = LONG( in.top );
		out.right = LONG( in.right );
		out.bottom = LONG( in.bottom );
		ASSERT( out.left == in.left );
		ASSERT( out.top == in.top );
		ASSERT( out.right == in.right );
		ASSERT( out.bottom == in.bottom );
		out.NormalizeRect( );
		ASSERT( out.right >= out.left );
		ASSERT( out.bottom >= out.top );
		return std::move( out );
		}
	std::int16_t left;
	std::int16_t top;
	std::int16_t right;
	std::int16_t bottom;
	};

//struct ExtensionString {
//	ExtensionString( ) {
//		ext[ 0 ] = 0;
//		}
//	_Field_z_ wchar_t ext[ MAX_PATH + 1 ];
//	bool operator<( const ExtensionString& rhs ) {	
//		return ( ext < rhs.ext );
//		}
//	friend bool operator<( const ExtensionString& lhs, const ExtensionString& rhs ) {	
//		return ( lhs.ext < rhs.ext );
//		}
//	};

#pragma pack(push, 1)
#pragma message( "Whoa there! I'm changing the natural data alignment for SExtensionRecord. Look for a message that says I'm restoring it!" )
struct SExtensionRecord {
	SExtensionRecord( ) : files( 0 ), color( COLORREF( 0 ) ), bytes( 0 ) { }
	SExtensionRecord( _In_ std::uint32_t files_in, _In_ COLORREF color_in, _In_ std::uint64_t bytes_in, _In_ CString ext_in ) : files( files_in ), color( color_in ), bytes( bytes_in ), ext( ext_in ) { }
	/*
	  COMPARED BY BYTES!
	  Data stored for each extension.
	  4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
	  18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	  */

	CString ext;
	_Field_range_( 0, 4294967295 ) std::uint32_t files;//save 4 bytes :)
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t bytes;
	COLORREF color;

	//static bool compareSExtensionRecordByBytes( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.bytes < rhs.bytes ); }
	//bool compareSExtensionRecordByNumberFiles ( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.files < rhs.files ); }

	bool compareSExtensionRecordByExtensionAlpha( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.ext.Compare( rhs.ext ) < 0 ); }

	};
#pragma message( "Restoring data alignment.... " )
#pragma pack(pop)

struct s_compareSExtensionRecordByBytes {
	public:
	bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.bytes < rhs.bytes ); }
	};

struct s_compareSExtensionRecordByNumberFiles {
	public:
	bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.files < rhs.files ); }
	};

enum ITEMTYPE : std::uint8_t {
	IT_MYCOMPUTER,		// Pseudo Container "My Computer"
	IT_DRIVE,			// C:\, D:\ etc.
	IT_DIRECTORY,		// Folder
	IT_FILE,			// Regular file
	IT_FILESFOLDER,		// Pseudo Folder "<Files>"
	//IT_FREESPACE,		// Pseudo File "<Free Space>"
	//IT_UNKNOWN,			// Pseudo File "<Unknown>"
	};

//Some global constants




struct FILEINFO {
	std::uint64_t length;
	FILETIME      lastWriteTime;
	DWORD         attributes;
	CString       name;
	};


// The dialog has these three radio buttons.
enum RADIO {
	RADIO_ALLLOCALDRIVES,
	RADIO_SOMEDRIVES,
	RADIO_AFOLDER
	};

// If I DO NOT mark these two operator overloads as `inline `, then we hit ODR violations. TODO: investigate
inline bool operator< ( const FILETIME& t1, const FILETIME& t2 ) {
	const auto u1 = ( const ULARGE_INTEGER& ) t1;
	const auto u2 = ( const ULARGE_INTEGER& ) t2;

	return ( u1.QuadPart < u2.QuadPart );
	}
inline bool operator== ( const FILETIME& t1, const FILETIME& t2 ) {
	return t1.dwLowDateTime == t2.dwLowDateTime && t1.dwHighDateTime == t2.dwHighDateTime;
	}



const UINT WMU_OK = WM_USER + 100;
//#define WMU_WORKERTHREAD_FINISHED ( WM_USER + 102 )

//struct WorkerThreadData {
//	CItemBranch* theRootItem;
//	HWND*        theMainWindow;
//	};