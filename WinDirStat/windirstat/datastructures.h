// Globally instantiable datastructures
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_DATASTRUCTURES_H
#define WDS_DATASTRUCTURES_H

#pragma message( "Including `" __FILE__ "`..." )


class CSelectObject final {
public:
	CSelectObject( _In_ CDC& pdc, _In_ CGdiObject& pObject );
	~CSelectObject( );

	CSelectObject( const CSelectObject& in ) = delete;
	CSelectObject& operator=( const CSelectObject& rhs ) = delete;
protected:
	CDC* const m_pdc;
	CGdiObject* m_pOldObject;
	};

class CSelectStockObject final {
public:
	CSelectStockObject( _In_ CDC& pdc, _In_ _In_range_( 0, 16 ) const INT nIndex );

	~CSelectStockObject( );

	CSelectStockObject( const CSelectStockObject& in ) = delete;
	CSelectStockObject& operator=( const CSelectStockObject& rhs ) = delete;
protected:
	CDC* const  m_pdc;
	CGdiObject* m_pOldObject;
	};

class CSetBkMode final {
public:
	_Pre_satisfies_( ( mode == OPAQUE ) || ( mode == TRANSPARENT ) )
	CSetBkMode( _In_ CDC& pdc, _In_ const INT mode );
	
	~CSetBkMode( );

	CSetBkMode( const CSetBkMode& in ) = delete;
	CSetBkMode& operator=( const CSetBkMode& rhs ) = delete;
protected:
	CDC* const m_pdc;
	//C4820: 'CSetBkMode' : '4' bytes padding added after data member 'CSetBkMode::m_oldMode'
	int  m_oldMode;
	};

class CSetTextColor final {
public:
	CSetTextColor( _In_ CDC& pdc, _In_ const COLORREF color );

	~CSetTextColor( );

	CSetTextColor( const CSetTextColor& in ) = delete;
	CSetTextColor& operator=( const CSetTextColor& rhs ) = delete;
protected:
	CDC* const m_pdc;
	//C4820: 'CSetTextColor' : '4' bytes padding added after data member 'CSetTextColor::m_oldColor'
	COLORREF m_oldColor;
	};



#pragma pack(push, 1)
#pragma message( "Whoa there! I'm changing the natural data alignment for SExtensionRecord. Look for a message that says I'm restoring it!" )
struct SExtensionRecord final {
	SExtensionRecord( );

	SExtensionRecord( const SExtensionRecord& in );
	//SExtensionRecord( SExtensionRecord& in ) = delete;

	//Yes, this is used!
	SExtensionRecord( SExtensionRecord&& in );

	SExtensionRecord( _In_ std::uint32_t files_in, _In_ std::uint64_t bytes_in, _In_ std::wstring ext_in );
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

	const bool compareSExtensionRecordByExtensionAlpha( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const;

	};
#pragma message( "Restoring data alignment.... " )
#pragma pack(pop)

//Used for mapping std::wstring -> files + bytes
struct minimal_SExtensionRecord final {
	minimal_SExtensionRecord( );
	_Field_range_( 0, 4294967295 ) std::uint32_t files;
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t bytes;
	};

class CTreeListItem;



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





//static const Treemap_Options  _defaultOptions;				// Good values. Default for WinDirStat 1.0.2






// The dialog has these three radio buttons.
//enum RADIO : INT {
//	//RADIO_ALLLOCALDRIVES,
//	RADIO_SOMEDRIVES,
//	RADIO_AFOLDER
//	};


////Boilerplate D2D code: http://msdn.microsoft.com/en-us/library/windows/desktop/dd370994(v=vs.85).aspx
//template<class Interface>
//void SafeRelease( _In_ Interface** const ppInterfaceToRelease ) {
//	if ( *ppInterfaceToRelease != NULL ) {
//		( *ppInterfaceToRelease )->Release( );
//
//		( *ppInterfaceToRelease ) = { NULL };
//		}
//	}



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


// The "logical focus" can be 
// - on the Directory List
// - on the Extension List
// Although these windows can loose the real focus, for instance when a dialog box is opened, the logical focus will not be lost.
enum class LOGICAL_FOCUS {
	LF_NONE,
	LF_DIRECTORYLIST,
	LF_EXTENSIONLIST
	};






const UINT GENERAL_INDENT   = 5;

const LONG NODE_HEIGHT = 24;	// Height of IDB_NODES


//const UINT WMU_OK = WM_USER + 100;
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
