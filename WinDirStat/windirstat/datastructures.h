// Globally instantiable datastructures
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_DATASTRUCTURES_H
#define WDS_DATASTRUCTURES_H

class CSelectObject {
public:
	CSelectObject( _In_ CDC& pdc, _In_ CGdiObject& pObject ) : m_pdc{ &pdc } {
		//"Return Value: A pointer to the object being replaced. This is a pointer to an object of one of the classes derived from CGdiObject, such as CPen, depending on which version of the function is used. The return value is NULL if there is an error. This function may return a pointer to a temporary object. This temporary object is only valid during the processing of one Windows message. For more information, see CGdiObject::FromHandle."
		m_pOldObject = pdc.SelectObject( &pObject );
		/*
_AFXWIN_INLINE CGdiObject* CDC::SelectObject(CGdiObject* pObject)
	{ ASSERT(m_hDC != NULL); return SelectGdiObject(m_hDC, pObject->GetSafeHandle()); }
		
CGdiObject* PASCAL CDC::SelectGdiObject(HDC hDC, HGDIOBJ h)
{
	return CGdiObject::FromHandle(::SelectObject(hDC, h));
}

CGdiObject* PASCAL CGdiObject::FromHandle(HGDIOBJ h)
{
	CHandleMap* pMap = afxMapHGDIOBJ(TRUE); //create map if not exist
	ASSERT(pMap != NULL);
	CGdiObject* pObject = (CGdiObject*)pMap->FromHandle(h);
	ASSERT(pObject == NULL || pObject->m_hObject == h);
	return pObject;
}
		*/

		ASSERT( m_pOldObject != NULL );
		}
	~CSelectObject( ) {
		const auto retval = m_pdc->SelectObject( m_pOldObject );
#ifdef DEBUG
		ASSERT( retval != NULL );
#else
		UNREFERENCED_PARAMETER( retval );
#endif
		}
	CSelectObject( const CSelectObject& in ) = delete;
	CSelectObject& operator=( const CSelectObject& rhs ) = delete;
protected:
	CDC* const m_pdc;
	CGdiObject* m_pOldObject;
	};

class CSelectStockObject {
public:
	CSelectStockObject( _In_ CDC& pdc, _In_ _In_range_( 0, 16 ) const INT nIndex ) : m_pdc { &pdc } {
		//"Return Value: A pointer to the CGdiObject object that was replaced if the function is successful. The actual object pointed to is a CPen, CBrush, or CFont object. If the call is unsuccessful, the return value is NULL."
		m_pOldObject = pdc.SelectStockObject( nIndex );
		ASSERT( m_pOldObject != NULL );
		}
	~CSelectStockObject( ) {
		//"Return Value: A pointer to the object being replaced. This is a pointer to an object of one of the classes derived from CGdiObject, such as CPen, depending on which version of the function is used. The return value is NULL if there is an error. This function may return a pointer to a temporary object. This temporary object is only valid during the processing of one Windows message. For more information, see CGdiObject::FromHandle."
		const auto retval = m_pdc->SelectObject( m_pOldObject );
#ifdef DEBUG
		ASSERT( retval != NULL );
#else
		UNREFERENCED_PARAMETER( retval );
#endif
		}
	CSelectStockObject( const CSelectStockObject& in ) = delete;
	CSelectStockObject& operator=( const CSelectStockObject& rhs ) = delete;
protected:
	CDC* const  m_pdc;
	CGdiObject* m_pOldObject;
	};

class CSetBkMode {
public:
	_Pre_satisfies_( ( mode == OPAQUE) || ( mode == TRANSPARENT ) )
	CSetBkMode( _In_ CDC& pdc, _In_ const INT mode ) : m_pdc { &pdc } {
		m_oldMode = pdc.SetBkMode( mode );
		}
	~CSetBkMode( ) {
		m_pdc->SetBkMode( m_oldMode );
		}
	CSetBkMode( const CSetBkMode& in ) = delete;
	CSetBkMode& operator=( const CSetBkMode& rhs ) = delete;
protected:
	CDC* const m_pdc;
	//C4820: 'CSetBkMode' : '4' bytes padding added after data member 'CSetBkMode::m_oldMode'
	int  m_oldMode;
	};

class CSetTextColor {
public:
	CSetTextColor( _In_ CDC& pdc, _In_ const COLORREF color ) : m_pdc { &pdc } {
		//ASSERT_VALID( pdc );
		m_oldColor = pdc.SetTextColor( color );
		}
	~CSetTextColor( ) {
		m_pdc->SetTextColor( m_oldColor );
		}
	CSetTextColor( const CSetTextColor& in ) = delete;
	CSetTextColor& operator=( const CSetTextColor& rhs ) = delete;
protected:
	CDC* const m_pdc;
	//C4820: 'CSetTextColor' : '4' bytes padding added after data member 'CSetTextColor::m_oldColor'
	COLORREF m_oldColor;
	};


static_assert( sizeof( short ) == sizeof( std::int16_t ), "y'all ought to check SRECT" );
struct SRECT {
	/*
	  short-based RECT, saves 8 bytes compared to tagRECT
	  */
	SRECT( ) : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) { }
	SRECT( std::int16_t iLeft, std::int16_t iTop, std::int16_t iRight, std::int16_t iBottom ) : left { iLeft }, top { iTop }, right { iRight }, bottom { iBottom } { }
	//SRECT( const SRECT& in ) {
	//	left   = in.left;
	//	top    = in.top;
	//	right  = in.right;
	//	bottom = in.bottom;
	//	}
	
	SRECT( const SRECT& in ) = default;

	//SRECT( const CRect& in ) {
	//	left   = static_cast<std::int16_t>( in.right );
	//	top    = static_cast<std::int16_t>( in.top );
	//	right  = static_cast<std::int16_t>( in.right );
	//	bottom = static_cast<std::int16_t>( in.bottom );
	//	}
	SRECT( const RECT& in ) {
		left   = static_cast<std::int16_t>( in.right );
		top    = static_cast<std::int16_t>( in.top );
		right  = static_cast<std::int16_t>( in.right );
		bottom = static_cast<std::int16_t>( in.bottom );
		}

	int Width( ) const {
		return right - left;
		}

	int Height( ) const {
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
	SExtensionRecord( ) : files { 0u }, color { 0u }, bytes { 0u } { }

	SExtensionRecord( const SExtensionRecord& in ) = default;
	//SExtensionRecord( SExtensionRecord& in ) = delete;

	//Yes, this is used!
	SExtensionRecord( SExtensionRecord&& in ) {
		ext = std::move( in.ext );
		files = std::move( in.files );
		bytes = std::move( in.bytes );
		color = std::move( in.color );
		}

	SExtensionRecord( _In_ std::uint32_t files_in, _In_ std::uint64_t bytes_in, _In_ std::wstring ext_in ) : files { std::move( files_in ) }, bytes { std::move( bytes_in ) }, ext( std::move( ext_in ) ) { }
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

	bool compareSExtensionRecordByExtensionAlpha( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const { return ( lhs.ext.compare( rhs.ext ) < 0 ); }

	};
#pragma message( "Restoring data alignment.... " )
#pragma pack(pop)

//Used for mapping std::wstring -> files + bytes
struct minimal_SExtensionRecord {
	minimal_SExtensionRecord( ) : files { 0u }, bytes { 0u } { }
	_Field_range_( 0, 4294967295 ) std::uint32_t files;
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t bytes;
	};

class CTreeListItem;


//Yes, used
struct s_compareSExtensionRecordByBytes {
	public:
	bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const { return ( lhs.bytes < rhs.bytes ); }
	};

//struct s_compareSExtensionRecordByNumberFiles {
//	public:
//	bool operator()( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const { return ( lhs.files < rhs.files ); }
//	};

//enum ITEMTYPE : std::uint8_t {
//	IT_DIRECTORY,		// Folder
//	IT_FILE,			// Regular file
//	};

//enum TABTYPE : INT {//some MFC functions require an INT
//	//SAL can't, for whatever reason, see these in an anonymous namespace
//	TAB_ABOUT,
//	TAB_LICENSE
//	};

enum class Treemap_STYLE {
	KDirStatStyle,		// Children are layed out in rows. Similar to the style used by KDirStat.
	SequoiaViewStyle	// The 'classical' squarification as described in `squarified treemaps` (stm.pdf)
	};


template<class T>
INT signum(T x) {
	static_assert( std::is_arithmetic<T>::value, "need an arithmetic datatype!" );
	
	//This static_assert probably caught a bug!!! See CDriveItem::Compare - case column::COL_TOTAL & case column::COL_FREE were passing the result of an unsigned subtraction to this!!
	static_assert( std::is_signed<T>::value, "please don't try this with an unsigned number!" );

	if ( x < 0 ) {
		return -1;
		}
	if ( x == 0 ) {
		return 0;
		}
	return 1;
	//return ( x < 0 ) ? -1 : ( x == 0 ) ? 0 : 1;
	}


// Collection of all treemap options.
struct Treemap_Options {
	                                Treemap_STYLE style;        // Squarification method
									//C4820: 'Treemap_Options' : '3' bytes padding added after data member 'Treemap_Options::grid'
	                                bool          grid;         // Whether or not to draw grid lines
									//C4820: 'Treemap_Options' : '4' bytes padding added after data member 'Treemap_Options::gridColor'
	                                COLORREF      gridColor;    // Color of grid lines
	_Field_range_(  0, 1          ) DOUBLE        brightness;   // (default = 0.84)
	_Field_range_(  0, UINT64_MAX ) DOUBLE        height;       // (default = 0.40)  Factor "H (really range should be 0...std::numeric_limits<double>::max/100"
	_Field_range_(  0, 1          ) DOUBLE        scaleFactor;  // (default = 0.90)  Factor "F"
	_Field_range_(  0, 1          ) DOUBLE        ambientLight; // (default = 0.15)  Factor "Ia"
	_Field_range_( -4, 4          ) DOUBLE        lightSourceX; // (default = -1.0), negative = left
	_Field_range_( -4, 4          ) DOUBLE        lightSourceY; // (default = -1.0), negative = top

	_Ret_range_( 0, 100 ) INT    GetBrightnessPercent  ( ) const { return RoundDouble( brightness   * 100 );                               }
	_Ret_range_( 0, 100 ) INT    GetHeightPercent      ( ) const { return RoundDouble( height       * 100 );                               }
	_Ret_range_( 0, 100 ) INT    GetScaleFactorPercent ( ) const { return RoundDouble( scaleFactor  * 100 );                               }
	_Ret_range_( 0, 100 ) INT    GetAmbientLightPercent( ) const { return RoundDouble( ambientLight * 100 );                               }
	_Ret_range_( 0, 100 ) INT    GetLightSourceXPercent( ) const { return RoundDouble( lightSourceX * 100 );                               }
	_Ret_range_( 0, 100 ) INT    GetLightSourceYPercent( ) const { return RoundDouble( lightSourceY * 100 );                               }
		                  POINT  GetLightSourcePoint   ( ) const { return POINT { GetLightSourceXPercent( ), GetLightSourceYPercent( ) }; }

	_Ret_range_( 0, 100 ) INT    RoundDouble ( const DOUBLE d ) const { return signum( d ) * INT( abs( d ) + 0.5 ); }

	void SetBrightnessPercent  ( const INT    n   ) { brightness   = n / 100.0; }
	void SetHeightPercent      ( const INT    n   ) { height       = n / 100.0; }
	void SetScaleFactorPercent ( const INT    n   ) { scaleFactor  = n / 100.0; }
	void SetAmbientLightPercent( const INT    n   ) { ambientLight = n / 100.0; }
	void SetLightSourceXPercent( const INT    n   ) { lightSourceX = n / 100.0; }
	void SetLightSourceYPercent( const INT    n   ) { lightSourceY = n / 100.0; }
	void SetLightSourcePoint   ( const POINT  pt  ) {
			SetLightSourceXPercent( pt.x );
			SetLightSourceYPercent( pt.y );
		}
	};

//static const Treemap_Options  _defaultOptions;				// Good values. Default for WinDirStat 1.0.2
static const Treemap_Options _defaultOptions = { Treemap_STYLE::KDirStatStyle, false, RGB( 0, 0, 0 ), 0.88, 0.38, 0.91, 0.13, -1.0, -1.0 };


struct FILEINFO {
	FILEINFO( ) { }
	FILEINFO( const FILEINFO& in ) = delete;
	FILEINFO& operator=( const FILEINFO& in ) = delete;
	FILEINFO& operator=( FILEINFO&& in ) {
		length = std::move( in.length );
		lastWriteTime = std::move( in.lastWriteTime );
		attributes = std::move( in.attributes );
		name = std::move( in.name );
		return ( *this );
		}
	FILEINFO( FILEINFO&& in ) {
		length = std::move( in.length );
		lastWriteTime = std::move( in.lastWriteTime );
		attributes = std::move( in.attributes );
		name = std::move( in.name );
		}

	FILEINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_z_ wchar_t( &cFileName )[ MAX_PATH ] ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( cFileName ) {
#ifdef DEBUG
		if ( length > 34359738368 ) {
			_CrtDbgBreak( );
			}
#endif
		}
	void reset( ) {
		length = 0;
		lastWriteTime.dwLowDateTime  = 0;
		lastWriteTime.dwHighDateTime = 0;
		attributes = INVALID_FILE_ATTRIBUTES;
		name.clear( );
		}
	bool operator<( const FILEINFO& rhs ) const {
		return length < rhs.length;
		}

	std::uint64_t length;
	FILETIME      lastWriteTime;
	//C4820: 'FILEINFO' : '4' bytes padding added after data member 'FILEINFO::attributes'
	DWORD         attributes;
	std::wstring  name;
	};

struct DIRINFO {
	DIRINFO( ) { }
	DIRINFO( DIRINFO&& in ) {
		length = std::move( in.length );
		lastWriteTime = std::move( in.lastWriteTime );
		attributes = std::move( in.attributes );
		name = std::move( in.name );
		path = std::move( in.path );
		}

	DIRINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_z_ wchar_t( &cFileName )[ MAX_PATH ], _In_ std::wstring path_ ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( cFileName ), path( std::move( path_ ) ) { }

	std::uint64_t length;
	FILETIME      lastWriteTime;
	//C4820: 'DIRINFO' : '4' bytes padding added after data member 'DIRINFO::attributes'
	DWORD         attributes;
	std::wstring       name;
	std::wstring       path;
	};

// The dialog has these three radio buttons.
enum RADIO : INT {
	//RADIO_ALLLOCALDRIVES,
	RADIO_SOMEDRIVES,
	RADIO_AFOLDER
	};


//Boilerplate D2D code: http://msdn.microsoft.com/en-us/library/windows/desktop/dd370994(v=vs.85).aspx
template<class Interface>
void SafeRelease( _In_ Interface** const ppInterfaceToRelease ) {
	if ( *ppInterfaceToRelease != NULL ) {
		( *ppInterfaceToRelease )->Release( );

		( *ppInterfaceToRelease ) = { NULL };
		}
	}

struct attribs {	
	bool readonly   : 1;
	bool hidden     : 1;
	bool system     : 1;
  //bool archive    : 1;//Nobody actually cares about the archive attribute!
	bool compressed : 1;
	bool encrypted  : 1;
	bool reparse    : 1;
	bool invalid    : 1;

	//Not a file attribute, but a member of attribs to enable better packing
	bool m_done     : 1;
	};

void copy_attribs( _Out_ attribs& out, _In_ const attribs& in ) {
	static_assert( std::is_trivially_copyable<attribs>::value, "can't use memcpy!" );
	const auto res = memcpy_s( &out, sizeof( attribs ), &in, 1 );
	ASSERT( res == 0 );
	if ( res != 0 ) {
		WTL::AtlMessageBox( NULL, L"copy_attribs error!", TEXT( "Error" ), MB_OK );
		std::terminate( );
		}
	return;
	}




namespace UpdateAllViews_ENUM {
	// Hints for UpdateAllViews()
	enum {
		HINT_NULL,				        // General update
		HINT_NEWROOT,			        // Root item has changed - clear everything.
		HINT_SELECTIONCHANGED,	        // The selection has changed, EnsureVisible.
		HINT_SHOWNEWSELECTION,	        // The selection has changed, Show Path
		HINT_SELECTIONSTYLECHANGED,	    // Only update selection in Graphview
		HINT_EXTENSIONSELECTIONCHANGED,	// Type list selected a new extension
		//HINT_ZOOMCHANGED,		        // Only zoom item has changed.
		HINT_REDRAWWINDOW,		        // Only graphically redraw views.
		//HINT_SOMEWORKDONE,		    // Directory list shall process mouse messages first, then re-sort.
		HINT_LISTSTYLECHANGED,	        // Options: List style (grid/stripes) or treelist colors changed
		HINT_TREEMAPSTYLECHANGED,	    // Options: Treemap style (grid, colors etc.) changed
		};

	}

struct pair_of_item_and_path {
	CTreeListItem* ptr;
	std::wstring path;
	};


#ifndef RANGE_ENUM_COL
#define RANGE_ENUM_COL _In_ _In_range_( 0, 6 )
#else
#error already defined!
#endif


namespace column {
	enum ENUM_COL {
		//Equivalent to COL_EXTENSION in typeview.cpp
		COL_NAME,

		//COL_SUBTREEPERCENTAGE,//not used
		COL_PERCENTAGE,
		COL_SUBTREETOTAL,
		COL_ITEMS,
		COL_FILES,
		//COL_SUBDIRS,//not used
		COL_LASTCHANGE,
		COL_ATTRIBUTES,
		COL_TOTAL = COL_PERCENTAGE,
		COL_FREE  = COL_SUBTREETOTAL,
		COL_EXTENSION = COL_NAME,
		COL_COLOR     = COL_PERCENTAGE,
		COL_DESCRIPTION = COL_SUBTREETOTAL,
		COL_BYTES       = COL_ITEMS,
		COL_BYTESPERCENT = COL_FILES,
		COL_NTCOMPRESS = COL_FILES,
		COL_FILES_TYPEVIEW = COL_LASTCHANGE
		};
	static_assert( COL_BYTESPERCENT == 4, "typeview will break!" );
	}

// SSorting. A sorting specification. We sort by column1, and if two items equal in column1, we sort them by column2.
struct SSorting {
	SSorting( ) : column1 { column::COL_NAME }, column2 { column::COL_NAME }, ascending1 { false }, ascending2 { true } { }
	_Field_range_( 0, 8 ) column::ENUM_COL  column1;
	_Field_range_( 0, 8 ) column::ENUM_COL  column2;
	                      bool              ascending2 : 1;
						  //C4820: 'SSorting' : '3' bytes padding added after data member 'SSorting::ascending1'
	                      bool              ascending1 : 1;
	};

// The "logical focus" can be 
// - on the Directory List
// - on the Extension List
// Although these windows can loose the real focus, for instance when a dialog box is opened, the logical focus will not be lost.
enum class LOGICAL_FOCUS {
	LF_NONE,
	LF_DIRECTORYLIST,
	LF_EXTENSIONLIST
	};

// Sequence within IDB_NODES
enum class ENUM_NODE {
	NODE_PLUS_SIBLING,
	NODE_PLUS_END,
	NODE_MINUS_SIBLING,
	NODE_MINUS_END,
	NODE_SIBLING,
	NODE_END,
	NODE_LINE
	};


const INT  TEXT_X_MARGIN    = 6;	// Horizontal distance of the text from the edge of the item rectangle
const UINT LABEL_INFLATE_CX = 3;// How much the label is enlarged, to get the selection and focus rectangle
const UINT LABEL_Y_MARGIN   = 2;
const UINT GENERAL_INDENT   = 5;

const LONG NODE_HEIGHT = 24;	// Height of IDB_NODES


const UINT WMU_OK = WM_USER + 100;
//#define WMU_WORKERTHREAD_FINISHED ( WM_USER + 102 )

//typedef std::shared_ptr<std::tuple<std::shared_ptr<promise<std::pair<std::vector<directory_entry>, bool>>>, std::unique_ptr<windows_nt_kernel::FILE_ID_FULL_DIR_INFORMATION[]>, async_enumerate_op_req>> enumerate_state_t;

namespace global_strings {
	_Null_terminated_ const wchar_t write_to_stackbuffer_err[ ] = { L"GetText_WriteToStackBuffer - SERIOUS ERROR!" };
	
	_Null_terminated_ const wchar_t child_guaranteed_valid_err[ ] = { L"GetChildGuaranteedValid couldn't find a valid child! This should never happen!" };
	
	_Null_terminated_ const wchar_t eight_dot_three_gen_notif1[ ] = { L"Your computer is set to create short (8.3 style) names for files on " };
	_Null_terminated_ const wchar_t eight_dot_three_gen_notif2[ ] = { L"- As a result, the amount of time required to perform a directory listing increases with the square of the number of files in the directory! For more, see Microsoft KnowledgeBase article ID: 130694" };
	_Null_terminated_ const wchar_t eight_dot_three_all_volume[ ] = { L"all NTFS volumes. This can TREMENDOUSLY slow directory operations " };
	_Null_terminated_ const wchar_t eight_dot_three_per_volume[ ] = { L"NTFS volumes, on a per-volume-setting basis. Shore file name creation can TREMENDOUSLY slow directory operations " };
	_Null_terminated_ const wchar_t eight_dot_three_sys_volume[ ] = { L"the system volume. If you're running WinDirStat against any other volume you can safely ignore this warning. Short file name creation can TREMENDOUSLY slow directory operations " };

	_Null_terminated_ const wchar_t gen_performance_warning[ ] = { L"Performance warning!" };
	
	_Null_terminated_ const wchar_t get_date_format_buffer_err[ ] = { L"The supplied buffer size ( to GetDateFormatW ) was too small/NULL.\r\n" };
	_Null_terminated_ const wchar_t get_time_format_buffer_err[ ] = { L"The supplied buffer size ( to GetTimeFormatW ) was too small/NULL.\r\n" };

	_Null_terminated_ const wchar_t get_date_format_flags_err[ ] = { L"The specified flags ( to GetDateFormatW ) were not valid." };
	_Null_terminated_ const wchar_t get_time_format_flags_err[ ] = { L"The specified flags ( to GetTimeFormatW ) were not valid." };

	_Null_terminated_ const wchar_t get_date_format_param_err[ ] = { L"Any of the parameter values ( for GetDateFormatW ) was invalid." };
	_Null_terminated_ const wchar_t get_time_format_param_err[ ] = { L"Any of the parameter values ( for GetTimeFormatW ) was invalid." };

	_Null_terminated_ const wchar_t get_time_format_err_OUTOFMEMORY[ ] = { L"GetTimeFormatW failed because not enough memory was available to complete this operation! (unrecoverable)" };

	_Null_terminated_ const wchar_t write_to_stackbuffer_file[ ] = { L"Not implemented yet. Try normal GetText." };

	_Null_terminated_ const wchar_t global_alloc_failed[ ] = { L"GlobalAlloc failed! Cannot copy to clipboard!" };

	_Null_terminated_ const wchar_t string_cch_copy_failed[ ] = { L"StringCchCopyW failed!" };

	_Null_terminated_ const wchar_t cannot_set_clipboard_data[ ] = { L"Cannot set clipboard data! Cannot copy to clipboard!" };

	_Null_terminated_ const wchar_t main_split[ ] = { L"main" };

	_Null_terminated_ const wchar_t sub_split[ ] = { L"sub" };

	_Null_terminated_ const wchar_t treelist_str[ ] = { L"treelist" };

	_Null_terminated_ const wchar_t type_str[ ] = { L"types" };

	_Null_terminated_ const wchar_t drives_str[ ] = { L"drives" };

	_Null_terminated_ const wchar_t select_drives_dialog_layout[ ] = { L"sddlg" };

	_Null_terminated_ const wchar_t name[ ] = { L"Name" };

	_Null_terminated_ const wchar_t total[ ] = { L"Total" };

	_Null_terminated_ const wchar_t free[ ] = { L"Free" };
	_Null_terminated_ const wchar_t output_dbg_string_error[ ] = { L"WDS: OutputDebugStringW error!" };
	_Null_terminated_ const wchar_t COwnerDrawnListCtrl_handle_LvnGetdispinfo_err[ ] = { L"COwnerDrawnListCtrl::handle_LvnGetdispinfo serious error!" };

	_Null_terminated_ const wchar_t about_text[ ] = { L"\r\naltWinDirStat - a fork of 'WinDirStat' Windows Directory Statistics\r\n\r\nShows where all your disk space has gone\r\nand helps you clean it up.\r\n\r\n(originally)Re-programmed for MS Windows by\r\nBernhard Seifert,\r\n\r\nbased on Stefan Hundhammer's KDE (Linux) program KDirStat\r\n(http://kdirstat.sourceforge.net/).\r\n\r\n\r\n\r\n\r\n\r\nLATER modified by Alexander Riccio\r\n\r\nabout.me/ariccio or ariccio.com\r\nsee gpl-2.0.txt for license ( GNU GENERAL PUBLIC LICENSE Version 2, June 1991 )" };
	
	_Null_terminated_ const wchar_t select_folder_dialog_title_text[ ] = L"WinDirStat - Select Folder";

	_Null_terminated_ const wchar_t GetDriveInformation_failed_fmt_str[ ] = L"WDS: thread (%p)->GetDriveInformation failed!, name: %s, total: %I64u, free: %I64u\r\n";

	_Null_terminated_ const wchar_t GetDriveInformation_succeed_fmt_str[ ] = L"thread (%p)->GetDriveInformation succeeded!, name:%s, total: %I64u, free: %I64u\r\n";
	}

#else
#error ass
#endif
