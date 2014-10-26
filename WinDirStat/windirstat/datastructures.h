//Globally instantiable datastructures

#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"



class CSelectObject {
public:
	CSelectObject( CDC& pdc, CGdiObject& pObject ) {
		//ASSERT_VALID( pdc );
		m_pOldObject = pdc.SelectObject( &pObject );
		m_pdc = &pdc;
		}
	~CSelectObject() {
		m_pdc->SelectObject( m_pOldObject );
		}
protected:
	CDC* m_pdc;
	CGdiObject* m_pOldObject;
	};

class CSelectStockObject {
public:
	CSelectStockObject( CDC& pdc, INT nIndex ) {
		//ASSERT_VALID( pdc );
		m_pOldObject = pdc.SelectStockObject( nIndex );
		m_pdc = &pdc;
		}
	~CSelectStockObject( ) {
		m_pdc->SelectObject(m_pOldObject);
		}
protected:
	CDC*        m_pdc;
	CGdiObject* m_pOldObject;
	};

class CSetBkMode {
public:
	CSetBkMode(CDC& pdc, INT mode) {
		//ASSERT_VALID( pdc );
		m_pdc = &pdc;
		m_oldMode = pdc.SetBkMode( mode );
		}
	~CSetBkMode() {
		m_pdc->SetBkMode(m_oldMode);
		}
protected:
	CDC* m_pdc;
	INT  m_oldMode;
	};

class CSetTextColor {
public:
	CSetTextColor(CDC& pdc, COLORREF color) {
		//ASSERT_VALID( pdc );
		m_pdc = &pdc;
		m_oldColor = pdc.SetTextColor( color );
		}
	~CSetTextColor() {
		m_pdc->SetTextColor(m_oldColor);
		}
protected:
	CDC*     m_pdc;
	COLORREF m_oldColor;
	};


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
	int Width( ) {
		return right - left;
		}
	int Height( ) {
		return bottom - top;
		}

	std::int16_t left;
	std::int16_t top;
	std::int16_t right;
	std::int16_t bottom;
	};


#pragma pack(push, 1)
#pragma message( "Whoa there! I'm changing the natural data alignment for SExtensionRecord. Look for a message that says I'm restoring it!" )
struct SExtensionRecord {
	SExtensionRecord( ) : files( 0 ), color( COLORREF( 0 ) ), bytes( 0 ) { }
	SExtensionRecord( _In_ std::uint32_t files_in, _In_ COLORREF color_in, _In_ std::uint64_t bytes_in, _In_ PCWSTR ext_in ) : files( files_in ), color( color_in ), bytes( bytes_in ), ext( ext_in ) {
		ext.shrink_to_fit( );
		}
	/*
	  COMPARED BY BYTES!
	  Data stored for each extension.
	  4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
	  18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
	  */

	std::wstring ext;
	_Field_range_( 0, 4294967295 ) std::uint32_t files;//save 4 bytes :)
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t bytes;
	COLORREF color;

	//static bool compareSExtensionRecordByBytes( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.bytes < rhs.bytes ); }
	//bool compareSExtensionRecordByNumberFiles ( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.files < rhs.files ); }

	bool compareSExtensionRecordByExtensionAlpha( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.ext.compare( rhs.ext ) < 0 ); }

	};
#pragma message( "Restoring data alignment.... " )
#pragma pack(pop)

class CItemBranch;


struct s_compareSExtensionRecordByBytes {
	public:
	bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.bytes < rhs.bytes ); }
	};

struct s_compareSExtensionRecordByNumberFiles {
	public:
	bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) { return ( lhs.files < rhs.files ); }
	};

enum ITEMTYPE : std::uint8_t {
	//IT_MYCOMPUTER,		// Pseudo Container "My Computer"
	//IT_DRIVE,			// C:\, D:\ etc.
	IT_DIRECTORY,		// Folder
	IT_FILE,			// Regular file
	//IT_FILESFOLDER,		// Pseudo Folder "<Files>"
	};

enum TABTYPE : INT {//some MFC functions require an INT
	//SAL can't, for whatever reason, see these in an anonymous namespace
	TAB_ABOUT,
	TAB_LICENSE
	};


struct CItemSkeleton {
	CItemSkeleton*                              m_parent;
	ITEMTYPE                                    m_type; // Indicates our type. See ITEMTYPE.
	CString                                     m_name; // Display name
	std::vector<std::unique_ptr<CItemSkeleton>> m_children;
	std::uint64_t                               m_size;
	FILETIME                                    m_lastChange; // Last modification time OF SUBTREE
	DWORD                                       m_attributes;
	};


struct FILEINFO {
	std::uint64_t length;
	FILETIME      lastWriteTime;
	DWORD         attributes;
	CString       name;
	};

struct DIRINFO {
	DIRINFO( ) { }
	DIRINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_ CString name_, _In_ CString path_ ) : length( std::move( length_ ) ), lastWriteTime( std::move( lastWriteTime_ ) ), attributes( std::move( attributes_ ) ), name( std::move( name_ ) ), path( std::move( path_ ) ) { }
	std::uint64_t length;
	FILETIME      lastWriteTime;
	DWORD         attributes;
	CString       name;
	CString       path;
	};

// The dialog has these three radio buttons.
enum RADIO : INT {
	RADIO_ALLLOCALDRIVES,
	RADIO_SOMEDRIVES,
	RADIO_AFOLDER
	};


enum {	// length of internal buffer, [1, 16]
	SSO_THRESHOLD_BUF_SIZE = ( 16 / sizeof( wchar_t ) )
	};

enum ENUM_COL : int {
	COL_NAME,
	COL_TOTAL,
	COL_FREE,
	COL_GRAPH,
	COL_PERCENTUSED,
	COLUMN_COUNT
	};


//Boilerplate D2D code: http://msdn.microsoft.com/en-us/library/windows/desktop/dd370994(v=vs.85).aspx
template<class Interface>
void SafeRelease( Interface** ppInterfaceToRelease ) {
	if ( *ppInterfaceToRelease != NULL ) {
		( *ppInterfaceToRelease )->Release( );

		( *ppInterfaceToRelease ) = NULL;
		}
	}

struct AbstractItem {
	_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_ virtual AbstractItem* GetChild( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i  ) const = 0;
	};

//struct ItemCount {
//	virtual size_t GetChildrenCount( ) const = 0;
//	};


struct attribs {
	//attribs( ) : readonly( false ), hidden( false ), system( false ), archive( false ), compressed( false ), encrypted( false ), reparse( false ), invalid( false ) { }
	
	bool readonly   : 1;
	bool hidden     : 1;
	bool system     : 1;
	bool archive    : 1;
	bool compressed : 1;
	bool encrypted  : 1;
	bool reparse    : 1;
	bool invalid    : 1;
	};


//CRITICAL_SECTION treelist_critical_section;

//const CTreemap::Options _defaultOptions =    { KDirStatStyle, false, RGB( 0, 0, 0 ), 0.88, 0.38, 0.91, 0.13, -1.0, -1.0 };



const INT  TEXT_X_MARGIN    = 6;	// Horizontal distance of the text from the edge of the item rectangle
const UINT LABEL_INFLATE_CX = 3;// How much the label is enlarged, to get the selection and focus rectangle
const UINT LABEL_Y_MARGIN   = 2;
const UINT GENERAL_INDENT   = 5;

const LONG NODE_HEIGHT = 24;	// Height of IDB_NODES


const UINT WMU_OK = WM_USER + 100;
//#define WMU_WORKERTHREAD_FINISHED ( WM_USER + 102 )

//struct WorkerThreadData {
//	CItemBranch* theRootItem;
//	HWND*        theMainWindow;
//	};