#define _ERROR_WINDOWS
#include "error.h"
#include <Windows.h>


#include <initguid.h>
#include "guids.h"
#include "utils.h"
#include "ntfs.h"
#include "volume.h"
#include "options.h"
#include "content.h"
#include "dlgapi.h"
#include "ntfs_file.h"
#include "file_panel.h"
#include "log.h"
#include "defragment.h"
#include "filever.h"
#include "compress_files.h"

struct PluginStartupInfo g_far;
struct FarStandardFunctions g_fsf;

//Array<FarColor> g_colors;

HINSTANCE g_h_module;

struct CtrlIds {
	unsigned linked;
	unsigned link; // displays text from 'linked'
	CtrlIds( ) : linked( -1 ) { // no linked control by default
		}
	};

const int c_update_time = 1;

struct FileTotals {
	u64 unnamed_data_size;
	u64 named_data_size;
	u64 nr_data_size;
	u64 unnamed_disk_size;
	u64 named_disk_size;
	u64 nr_disk_size;
	u64 unnamed_hl_size;
	u64 named_hl_size;
	u64 nr_hl_size;
	u64 excess_fragments;
	unsigned file_cnt;
	unsigned hl_cnt;
	unsigned dir_cnt;
	unsigned file_rp_cnt;
	unsigned dir_rp_cnt;
	unsigned err_cnt;
	FileTotals( ) {
		memset( this, 0, sizeof( *this ) );
		}
	};

struct FileAnalyzer {
	std::vector<std::string> file_list;
	FileInfo file_info;
	std::vector<FileInfo> hard_links;
	NtfsVolume volume;
	FileTotals totals;
	HANDLE h_dlg;
	//Array<FarDialogItem> dlg_items;
	std::vector<std::string> dlg_text;
	//Array<FAR_CHAR_INFO> dlg_ci;
	CtrlIds ctrl;
	HANDLE h_thread;
	HANDLE h_stop_event;
	time_t update_timer;
	void display_file_info( bool partial = false );
	void update_totals( const FileInfo& file_info, bool hard_link );
	void process_file( FileInfo& file_info, bool full_info );
	void process_recursive( const std::string& dir_name );
	void process( );
	static unsigned __stdcall th_proc( void* param );
	intptr_t dialog_handler( intptr_t msg, intptr_t param1, void* param2 );
	static intptr_t WINAPI dlg_proc( HANDLE h_dlg, intptr_t msg, intptr_t param1, void* param2 );
	FileAnalyzer( ) : h_dlg( NULL ), h_thread( NULL ), h_stop_event( NULL ), update_timer( 0 ) { }
	~FileAnalyzer( ) {
		if ( h_stop_event ) {
			CloseHandle( h_stop_event );
			}
		if ( h_thread ) {
			CloseHandle( h_thread );
			}
		}
	};
//
//Array<FAR_CHAR_INFO> str_to_char_info( const std::string& str, const Array<FarColor>& colors ) {
//	Array<FAR_CHAR_INFO> out;
//	unsigned color_idx = 0;
//	out.extend( str.size( ) );
//	FAR_CHAR_INFO ci;
//	memset( &ci, 0, sizeof( ci ) );
//	for ( unsigned i = 0; i < str.size( ); i++ ) {
//		if ( str[ i ] == '\1' ) {
//			ci.Attributes = colors[ color_idx++ ];
//			}
//		else {
//			ci.Char = str[ i ];
//			out.add( ci );
//			}
//		}
//	return out;
//	}
//
unsigned fmt_char_cnt( const std::string& str ) {
	unsigned cnt = 0;
	for ( unsigned i = 0; i < str.size( ); i++ ) {
		if ( str[ i ] == '\1' ) {
			cnt++;
			}
		}
	return cnt;
	}

//std::string far_get_text( HANDLE h_dlg, unsigned ctrl_id ) {
//	FarDialogItemData item = { sizeof( FarDialogItemData ) };
//	item.PtrLength = g_far.SendDlgMessage( h_dlg, DM_GETTEXT, ctrl_id, nullptr );
//	std::string text;
//	item.PtrData = text.buf( item.PtrLength + 1 );
//	g_far.SendDlgMessage( h_dlg, DM_GETTEXT, ctrl_id, &item );
//	text.set_size( item.PtrLength );
//	return text;
//	}

void FileAnalyzer::display_file_info( bool partial ) {
	const std::string empty_str;

	/* flag - show size summary for directories / multiple files */
	bool show_totals = ( file_info.directory && !file_info.reparse ) || ( file_list.size( ) > 1 );
	/* flag - single file is selected */
	bool single_file = file_list.size( ) == 1;
	/* current control index */
	unsigned ctrl_idx = 0;
	/* dialog line counter */
	unsigned line = 0;
	/* current hot key index */
	unsigned hot_key_idx = 0;

	//if ( h_dlg != NULL ) {
	//	g_far.SendDlgMessage( h_dlg, DM_ENABLEREDRAW, FALSE, nullptr );
	//	}

	/* FAR window size */
	SMALL_RECT console_rect;
	//CHECK( g_far.AdvControl( &c_plugin_guid, ACTL_GETFARRECT, 0, &console_rect ) );
	unsigned con_width = console_rect.Right - console_rect.Left + 1;
	unsigned con_height = console_rect.Bottom - console_rect.Top + 1;

	/* fixed width columns */
	const unsigned c_col_type_len = 21;
	const unsigned c_col_flags_len = 5;
	/* variable width columns */
	//unsigned col_data_size_min_len = far_get_msg( MSG_METADATA_COL_DATA_SIZE ).size( );
	//unsigned col_data_size_max_len = col_data_size_min_len;
	//unsigned col_data_size_len;
	//unsigned col_disk_size_min_len = far_get_msg( MSG_METADATA_COL_DISK_SIZE ).size( );
	//unsigned col_disk_size_max_len = col_disk_size_min_len;
	//unsigned col_disk_size_len;
	//unsigned col_fragments_min_len = far_get_msg( MSG_METADATA_COL_FRAGMENTS ).size( );
	//unsigned col_fragments_max_len = col_fragments_min_len;
	//unsigned col_fragments_len;
	//unsigned col_name_min_len = far_get_msg( MSG_METADATA_COL_NAME_DATA ).size( );
	//unsigned col_name_max_len = col_name_min_len;
	//unsigned col_name_len;

	//std::string file_name_label = far_get_msg( MSG_METADATA_FILE_NAME );
	//std::string file_name_text = L" " + file_name_label + L" ";
	//for ( unsigned p; ( ( p = file_name_text.search( L'&' ) ) != -1 ); ) {
	//	file_name_text.remove( p );
	//	}

	/* horizontal dialog border size */
	const unsigned c_hframe_width = 3;

	/* minimum allowed dialog width */
	//unsigned min_dlg_width = c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len + 1 +
	//  col_data_size_min_len + 1 + col_disk_size_min_len + 1 + col_fragments_min_len + 1 + col_name_min_len +
	//  1 + c_hframe_width;

	//if ( con_width < min_dlg_width ) {
	//	FAIL( MsgError( L"Console window size is too small" ) );
	//	}

	/* useful macro to measure required column width */
	std::string tmp;
#define MEASURE_COLUMN(col_size, value) { \
  tmp = format_inf_amount(value); \
  if (tmp.size() > col_size) col_size = tmp.size(); \
  tmp = format_inf_amount_short(value); \
  if (tmp.size() > col_size) col_size = tmp.size(); \
		}
#define MEASURE_COLUMN2(col_size, value1, value2) { \
  if (value2 == 0) { \
	MEASURE_COLUMN(col_size, value1); \
		} \
	  else { \
	tmp = format_inf_amount(value1) + '+' + format_inf_amount(value2); \
	if (tmp.size() > col_size) col_size = tmp.size(); \
	tmp = format_inf_amount_short(value1) + '+' + format_inf_amount_short(value2); \
	if (tmp.size() > col_size) col_size = tmp.size(); \
		  } \
		}

	u64 total_data_size = 0; // file total non-resident data size
	u64 total_disk_size = 0; // file total non-resident disk size
	//if ( single_file ) {
	//	for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
	//		const AttrInfo& attr = file_info.attr_list[ i ];
	//		MEASURE_COLUMN( col_data_size_max_len, attr.data_size );
	//		if ( !attr.resident ) {
	//			total_data_size += attr.data_size;
	//			total_disk_size += attr.disk_size;

	//			MEASURE_COLUMN( col_disk_size_max_len, attr.disk_size );
	//			MEASURE_COLUMN( col_fragments_max_len, attr.fragments );
	//			}
	//		if ( attr.name.size( ) + 2 > col_name_max_len ) col_name_max_len = attr.name.size( ) + 2;
	//		}
	//	MEASURE_COLUMN( col_data_size_max_len, total_data_size );
	//	MEASURE_COLUMN( col_disk_size_max_len, total_disk_size );
	//	}
	//if ( show_totals ) {
	//	MEASURE_COLUMN2( col_data_size_max_len, totals.unnamed_data_size, totals.unnamed_hl_size );
	//	MEASURE_COLUMN( col_disk_size_max_len, totals.unnamed_disk_size );
	//	MEASURE_COLUMN2( col_data_size_max_len, totals.named_data_size, totals.named_hl_size );
	//	MEASURE_COLUMN( col_disk_size_max_len, totals.named_disk_size );
	//	MEASURE_COLUMN2( col_data_size_max_len, totals.nr_data_size, totals.nr_hl_size );
	//	MEASURE_COLUMN( col_disk_size_max_len, totals.nr_disk_size );
	//	MEASURE_COLUMN( col_fragments_max_len, totals.excess_fragments );
	//	}

	/* max. width of attribute table */
	//unsigned attr_table_width = c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len +
	//  1 + col_data_size_max_len + 1 + col_disk_size_max_len + 1 + col_fragments_max_len +
	//  1 + col_name_max_len + 1 + c_hframe_width;
	/* max. width of file information panel */
	//unsigned info_panel_width = c_hframe_width + 1 + file_name_text.size( ) + ( single_file ? ( file_info.file_name.size( ) + 1 ) : far_get_msg( MSG_METADATA_MULTIPLE ).size( ) ) + 1 + c_hframe_width;
	/* max. dialog width */
	//unsigned max_dlg_width = attr_table_width > info_panel_width ? attr_table_width : info_panel_width;

#define DIALOG_WIDTH (c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len + \
	1 + col_data_size_len + 1 + col_disk_size_len + 1 + col_fragments_len + \
	1 + col_name_len + 1 + c_hframe_width)

	/* calculate real dialog width */
	//unsigned dlg_width;
	//if ( max_dlg_width > con_width ) {
	//	dlg_width = con_width;
	//	col_data_size_len = col_data_size_min_len;
	//	col_disk_size_len = col_disk_size_min_len;
	//	col_fragments_len = col_fragments_min_len;
	//	col_name_len = col_name_min_len;
	//	/* try to maximize size columns first */
	//	while ( DIALOG_WIDTH < dlg_width ) {
	//		bool no_change = true;
	//		if ( col_data_size_len < col_data_size_max_len ) {
	//			col_data_size_len++;
	//			no_change = false;
	//			if ( DIALOG_WIDTH == dlg_width ) break;
	//			}
	//		if ( col_disk_size_len < col_disk_size_max_len ) {
	//			col_disk_size_len++;
	//			no_change = false;
	//			if ( DIALOG_WIDTH == dlg_width ) break;
	//			}
	//		if ( col_fragments_len < col_fragments_max_len ) {
	//			col_fragments_len++;
	//			no_change = false;
	//			if ( DIALOG_WIDTH == dlg_width ) break;
	//			}
	//		if ( no_change ) break;
	//		}
	//	}
	//else {
	//	dlg_width = max_dlg_width;
	//	col_data_size_len = col_data_size_max_len;
	//	col_disk_size_len = col_disk_size_max_len;
	//	col_fragments_len = col_fragments_max_len;
	//	col_name_len = col_name_max_len;
	//	}
	/* set Name/Data column to max. possible size */
	//while ( DIALOG_WIDTH < dlg_width ) col_name_len++;

	/* column X coord. */
	//unsigned col_data_size_x = c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len + 1;
	//unsigned col_disk_size_x = c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len + 1 + col_data_size_len + 1;
	//unsigned col_fragments_x = c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len + 1 + col_data_size_len + 1 + col_disk_size_len + 1;
	//unsigned col_name_x = c_hframe_width + 1 + c_col_type_len + 1 + c_col_flags_len + 1 + col_data_size_len + 1 + col_disk_size_len + 1 + col_fragments_len + 1;

	/* width of the dialog client area */
	//unsigned dlg_cl_width = dlg_width - 2 * c_hframe_width - 2;

	std::string dlg_str;
	//Array<FarColor> dlg_colors;
	////dlg_str.set_inc( dlg_width * 2 );
	//FarDialogItem di;

	/* useful macros to form dialog contents */
#define SET_DATA(str) \
  if (h_dlg == NULL) { \
	dlg_text += str; \
	di.Data = dlg_text.last().data(); \
		} \
	  else { \
	dlg_text.item(ctrl_idx) = str; \
	di.Data = dlg_text[ctrl_idx].data(); \
		  }

#define ADD_STR_LINE(str) { \
  unsigned fmt_cnt = fmt_char_cnt(str); \
  dlg_str.add_fmt(L"\1%.*c\1%c\1%-*.*S\1%c\1%.*c", c_hframe_width, ' ', \
	c_vert2, dlg_cl_width + fmt_cnt, dlg_cl_width + fmt_cnt, &(str), \
	c_vert2, c_hframe_width, ' '); \
  dlg_colors.add(g_colors[COL_DIALOGTEXT]).add(g_colors[COL_DIALOGBOX]); \
  dlg_colors.add(g_colors[COL_DIALOGTEXT]).add(g_colors[COL_DIALOGBOX]).add(g_colors[COL_DIALOGTEXT]); \
  line++; \
		}

#define ADD_HORIZ_LINE(chl, chm, chr, cho) { \
  dlg_str.add_fmt(L"\1%.*c\1%c%.*c%c%.*c%c%.*c%c%.*c%c%.*c%c%.*c%c\1%.*c", \
	c_hframe_width, ' ', \
	chl, c_col_type_len, cho, chm, c_col_flags_len, cho, chm, \
	col_data_size_len, cho, chm, col_disk_size_len, cho, chm, \
	col_fragments_len, cho, chm, col_name_len, cho, chr, \
	c_hframe_width, ' '); \
  dlg_colors.add(g_colors[COL_DIALOGTEXT]).add(g_colors[COL_DIALOGBOX]).add(g_colors[COL_DIALOGTEXT]); \
  line++; \
		}

#define ADD_TABLE_LINE(type, flags, data_size, disk_size, fragments, name) { \
  ADD_STR_LINE(std::string::format(L"%*S\1%c\1%*S\1%c\1%*.*S\1%c\1%*.*S\1%c\1%*.*S\1%c\1%-*.*S", \
	c_col_type_len, &(type), c_vert1, \
	c_col_flags_len, &(flags), c_vert1, \
	col_data_size_len, col_data_size_len, &(data_size), c_vert1, \
	col_disk_size_len, col_disk_size_len, &(disk_size), c_vert1, \
	col_fragments_len, col_fragments_len, &(fragments), c_vert1, \
	col_name_len, col_name_len, &(name))); \
  dlg_colors.add(g_colors[COL_DIALOGBOX]).add(g_colors[COL_DIALOGTEXT]).add(g_colors[COL_DIALOGBOX]); \
  dlg_colors.add(g_colors[COL_DIALOGTEXT]).add(g_colors[COL_DIALOGBOX]); \
  dlg_colors.add(g_colors[COL_DIALOGTEXT]).add(g_colors[COL_DIALOGBOX]).add(g_colors[COL_DIALOGTEXT]); \
  dlg_colors.add(g_colors[COL_DIALOGBOX]).add(g_colors[COL_DIALOGTEXT]); \
		}

#define ADD_CTRL(val, x, len, show) { \
  memset(&di, 0, sizeof(di)); \
  di.Type = DI_EDIT; \
  di.Flags = DIF_READONLY | DIF_SELECTONENTRY; \
  di.X1 = (x); \
  di.X2 = di.X1 + (len) - 1; \
  di.Y1 = di.Y2 = line; \
  SET_DATA(val); \
  if (h_dlg != NULL) { \
	g_far.SendDlgMessage(h_dlg, DM_SETDLGITEM, ctrl_idx, &di); \
	g_far.SendDlgMessage(h_dlg, DM_SHOWITEM, ctrl_idx, reinterpret_cast<void*>(((val).size() > (len)) && (show) ? 1 : 0)); \
		} \
	  else { \
	dlg_items += di; \
		  } \
  ctrl_idx++; \
		}

	// edit control + hot key label for Name/Data column
#define ADD_ND_CTRL(val, show) { \
  bool active = (val.size() != 0) && (show); \
\
  memset(&di, 0, sizeof(di)); \
  di.Type = DI_TEXT; \
  di.X1 = col_name_x; \
  di.X2 = di.X1; \
  di.Y1 = di.Y2 = line; \
  if (hot_key_idx <= 8) { \
	SET_DATA(std::string::format(L"&%u", hot_key_idx + 1)); \
		} \
	  else { \
	SET_DATA(empty_str); \
		  } \
  if (h_dlg != NULL) { \
	g_far.SendDlgMessage(h_dlg, DM_SETDLGITEM, ctrl_idx, &di); \
	g_far.SendDlgMessage(h_dlg, DM_SHOWITEM, ctrl_idx, reinterpret_cast<void*>(active ? 1 : 0)); \
		} \
	  else { \
	dlg_items += di; \
		  } \
  ctrl_idx++; \
  if (active) hot_key_idx++; \
\
  memset(&di, 0, sizeof(di)); \
  di.Type = DI_EDIT; \
  di.Flags = DIF_READONLY | DIF_SELECTONENTRY; \
  di.X1 = col_name_x + 1; \
  di.X2 = di.X1 + col_name_len - 2; \
  di.Y1 = di.Y2 = line; \
  SET_DATA(val); \
  if (h_dlg != NULL) { \
	g_far.SendDlgMessage(h_dlg, DM_SETDLGITEM, ctrl_idx, &di); \
	g_far.SendDlgMessage(h_dlg, DM_SHOWITEM, ctrl_idx, reinterpret_cast<void*>(active ? 1 : 0)); \
		} \
	  else { \
	dlg_items += di; \
		  } \
  ctrl_idx++; \
		}

#define ADD_CTRL_LINE(data_size, disk_size, fragments, name, show) { \
  ADD_CTRL(data_size, col_data_size_x, col_data_size_len, show); \
  ADD_CTRL(disk_size, col_disk_size_x, col_disk_size_len, show); \
  ADD_CTRL(fragments, col_fragments_x, col_fragments_len, show); \
  ADD_ND_CTRL(name, show); \
		}

#define ADD_SIZE_TOTALS(title, data_size, hl_size, disk_size, fragments) { \
  std::string data_size_fmt; \
  std::string data_size_fmt_short; \
  if ((hl_size) == 0) { \
	data_size_fmt = format_inf_amount(data_size); \
	data_size_fmt_short = format_inf_amount_short(data_size); \
		} \
	  else { \
	data_size_fmt = format_inf_amount(data_size) + '+' + format_inf_amount(hl_size); \
	data_size_fmt_short = format_inf_amount_short(data_size) + '+' + format_inf_amount_short(hl_size); \
		  } \
  std::string disk_size_fmt = format_inf_amount(disk_size); \
  std::string disk_size_fmt_short = format_inf_amount_short(disk_size); \
  std::string fragments_fmt = (fragments) == 0 ? empty_str : std::string::format(far_get_msg(MSG_METADATA_TOTALS_EXCESS_FRAGMENTS).data(), &format_inf_amount(fragments)); \
  ADD_CTRL_LINE(data_size_fmt, disk_size_fmt, fragments_fmt, empty_str, true); \
  ADD_TABLE_LINE(title, empty_str, data_size_fmt, disk_size_fmt, fragments_fmt, empty_str); \
  bool second_line = (data_size_fmt != data_size_fmt_short) || (disk_size_fmt != disk_size_fmt_short); \
  ADD_CTRL_LINE(data_size_fmt_short, disk_size_fmt_short, empty_str, empty_str, second_line); \
  if (second_line) ADD_TABLE_LINE(std::string(), empty_str, data_size_fmt_short, disk_size_fmt_short, empty_str, empty_str); \
		}

	// reserve place for table
	//if ( h_dlg == NULL ) {
	//	dlg_items = di;
	//	dlg_text = std::string( );
	//	}
	ctrl_idx++;

	///* dialog border */
	//dlg_str.add_fmt( L"\1%.*c", dlg_width, ' ' );
	//dlg_colors.add( g_colors[ COL_DIALOGTEXT ] );
	//line++;

	///* dialog header */
	//unsigned plugin_title_len = far_get_msg( MSG_PLUGIN_NAME ).size( );
	//if ( plugin_title_len > dlg_cl_width - 2 ) plugin_title_len = dlg_cl_width - 2;
	//unsigned hl1 = ( dlg_cl_width - plugin_title_len ) / 2;
	//unsigned hl2 = dlg_cl_width - plugin_title_len - hl1;
	//if ( hl1 != 0 ) hl1--;
	//if ( hl2 != 0 ) hl2--;
	//dlg_str.add_fmt( L"\1%.*c\1%c%.*c \1%.*s\1 %.*c%c\1%.*c", c_hframe_width, ' ', c_top2_left2, hl1, c_horiz2, plugin_title_len, far_get_msg( MSG_PLUGIN_NAME ).data( ), hl2, c_horiz2, c_top2_right2, c_hframe_width, ' ' );
	//dlg_colors.add( g_colors[ COL_DIALOGTEXT ] ).add( g_colors[ COL_DIALOGBOX ] ).add( g_colors[ COL_DIALOGBOXTITLE ] );
	//dlg_colors.add( g_colors[ COL_DIALOGBOX ] ).add( g_colors[ COL_DIALOGTEXT ] );

	//memset( &di, 0, sizeof( di ) );
	//di.Type = DI_TEXT;
	////di.X1 = c_hframe_width + 1 + hl1 + 1;
	////di.X2 = di.X1 + far_get_msg( MSG_PLUGIN_NAME ).size( ) - 1;
	//di.Y1 = di.Y2 = line;
	////SET_DATA( far_get_msg( MSG_PLUGIN_NAME ) );
	//if ( h_dlg != NULL ) {
	//	g_far.SendDlgMessage( h_dlg, DM_SETDLGITEM, ctrl_idx, &di );
	//	}
	//else {
	//	dlg_items += di;
	//	}
	ctrl_idx++;
	line++;

	/* file information panel */
	if ( single_file ) {
		/* file name read-only EDIT control + hot key label */
		if ( h_dlg == NULL ) {
			//memset( &di, 0, sizeof( di ) );
			//di.Type = DI_TEXT;
			//di.X1 = c_hframe_width + 1 + 1;
			////di.X2 = di.X1 + get_label_len( file_name_label ) - 1;
			//di.Y1 = di.Y2 = line;
			////SET_DATA( file_name_label );
			//dlg_items += di;
			}
		ctrl_idx++;

		//memset( &di, 0, sizeof( di ) );
		//di.Type = DI_EDIT;
		//di.Flags = DIF_READONLY | DIF_SELECTONENTRY | DIF_FOCUS;
		//di.X1 = c_hframe_width + 1 + file_name_text.size( );
		//di.X2 = di.X1 + dlg_cl_width - file_name_text.size( ) - 1;
		//di.Y1 = di.Y2 = line;
		//SET_DATA( file_info.file_name );
		//if ( h_dlg != NULL ) {
		//	g_far.SendDlgMessage( h_dlg, DM_SETDLGITEM, ctrl_idx, &di );
		//	}
		//else {
		//	dlg_items += di;
		//	}
		ctrl_idx++;

		//ADD_STR_LINE( file_name_text + file_info.file_name );

		/* other file info */
		std::string str;
		if ( file_info.mft_rec_cnt > 1 ) {
			//str.add( L" " ).add_fmt( far_get_msg( MSG_METADATA_MFT_RECS ).data( ), file_info.mft_rec_cnt );
			}
		if ( file_info.hard_link_cnt > 1 ) {
			//str.add( L" " ).add_fmt( far_get_msg( MSG_METADATA_HARD_LINKS ).data( ), file_info.hard_link_cnt );
			}
		//if ( str.size( ) != 0 ) ADD_STR_LINE( str );
		}
	else {
		/* multiple files are selected */
		//ADD_STR_LINE( file_name_text + far_get_msg( MSG_METADATA_MULTIPLE ) );
		}

	//ADD_HORIZ_LINE( c_left2_horiz1, c_top1_vert1, c_right2_horiz1, c_horiz1 );

	/* attribute table header */
	//ADD_TABLE_LINE( far_get_msg( MSG_METADATA_COL_TYPE ), far_get_msg( MSG_METADATA_COL_FLAGS ), far_get_msg( MSG_METADATA_COL_DATA_SIZE ),
	  //far_get_msg( MSG_METADATA_COL_DISK_SIZE ), far_get_msg( MSG_METADATA_COL_FRAGMENTS ), far_get_msg( MSG_METADATA_COL_NAME_DATA ) );

	if ( single_file ) {
		//ADD_HORIZ_LINE( c_left2_horiz1, c_cross1, c_right2_horiz1, c_horiz1 );

		/* attribute list */
		std::string flags;
		for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
			const AttrInfo& attr = file_info.attr_list[ i ];

			/* attribute type name */
			std::string attr_type = attr.type_name( );

			/* attribute flags */
			//flags.copy( attr.resident ? 'R' : ' ' ).add( attr.mft_ext_rec ? 'X' : ' ' ).add( attr.compressed ? 'C' : ' ' ).add( attr.encrypted ? 'E' : ' ' ).add( attr.sparse ? 'S' : ' ' );

			/* attribute table line */
			std::string data_size_fmt = format_inf_amount( attr.data_size );
			std::string disk_size_fmt;
			std::string fragments_fmt;
			std::string data_size_fmt_short = format_inf_amount_short( attr.data_size );
			std::string disk_size_fmt_short;
			if ( !attr.resident ) {
				disk_size_fmt = format_inf_amount( attr.disk_size );
				fragments_fmt = format_inf_amount( attr.fragments );
				disk_size_fmt_short = format_inf_amount_short( attr.disk_size );
				}
			//ADD_CTRL_LINE( data_size_fmt, disk_size_fmt, fragments_fmt, attr.name, true );
			//ADD_TABLE_LINE( attr_type, flags, data_size_fmt, disk_size_fmt, fragments_fmt, attr.name );
			//bool second_line = ( data_size_fmt != data_size_fmt_short ) || ( disk_size_fmt != disk_size_fmt_short ); \
			//ADD_CTRL_LINE( data_size_fmt_short, disk_size_fmt_short, empty_str, empty_str, second_line );
			//if ( second_line ) ADD_TABLE_LINE( std::string( ), empty_str, data_size_fmt_short, disk_size_fmt_short, empty_str, empty_str );
			}

		/* attribute table totals */
		//ADD_HORIZ_LINE( c_left2_horiz1, c_cross1, c_right2_horiz1, c_horiz1 );
		//ADD_SIZE_TOTALS( far_get_msg( MSG_METADATA_ROW_NR_TOTAL ), total_data_size, 0, total_disk_size, 0 );
		}

	/* display totals for directory / multiple files */
	if ( show_totals ) {
		//ADD_HORIZ_LINE( c_left2_horiz1, c_bottom1_vert1, c_right2_horiz1, c_horiz1 );

		/* summary: count of files, dirs, etc. */
		std::string str;
		if ( partial ) {
			//str.add_fmt( L" \1*\1" );
			//dlg_colors.add( g_colors[ COL_DIALOGHIGHLIGHTTEXT ] ).add( g_colors[ COL_DIALOGTEXT ] );
			}
		if ( ( totals.file_cnt != 0 ) || ( totals.file_rp_cnt != 0 ) ) {
			//str.add( L" " ).add_fmt( far_get_msg( MSG_METADATA_TOTALS_FILES ).data( ), totals.file_cnt );
			}
		if ( totals.hl_cnt != 0 ) {
			//str.add_fmt( far_get_msg( MSG_METADATA_TOTALS_HL ).data( ), totals.hl_cnt );
			}
		if ( totals.file_rp_cnt != 0 ) {
			//str.add_fmt( far_get_msg( MSG_METADATA_TOTALS_RP_FILES ).data( ), totals.file_rp_cnt );
			}
		if ( ( totals.dir_cnt != 0 ) || ( totals.dir_rp_cnt != 0 ) ) {
			//str.add( L" " ).add_fmt( far_get_msg( MSG_METADATA_TOTALS_DIRS ).data( ), totals.dir_cnt );
			}
		if ( totals.dir_rp_cnt != 0 ) {
			//str.add_fmt( far_get_msg( MSG_METADATA_TOTALS_RP_DIRS ).data( ), totals.dir_rp_cnt );
			}
		if ( totals.err_cnt != 0 ) {
			//FarColor color = g_colors[ COL_DIALOGTEXT ];
			//color.ForegroundColor = FOREGROUND_RED;
			//color.Flags |= FCF_FG_4BIT;
			////str.add( L" " ).add_fmt( far_get_msg( MSG_METADATA_TOTALS_ERRORS ).data( ), &std::string::format( L"\1%u\1", totals.err_cnt ) );
			//dlg_colors.add( color ).add( g_colors[ COL_DIALOGTEXT ] );
			}
		//ADD_STR_LINE( str );

		/* total size of directory contents / multiple files */
		//ADD_HORIZ_LINE( c_left2_horiz1, c_top1_vert1, c_right2_horiz1, c_horiz1 );
		//ADD_SIZE_TOTALS( far_get_msg( MSG_METADATA_ROW_UNNAMED_TOTAL ), totals.unnamed_data_size, totals.unnamed_hl_size, totals.unnamed_disk_size, 0 );
		//ADD_SIZE_TOTALS( far_get_msg( MSG_METADATA_ROW_NAMED_TOTAL ), totals.named_data_size, totals.named_hl_size, totals.named_disk_size, 0 );
		//ADD_SIZE_TOTALS( far_get_msg( MSG_METADATA_ROW_NR_TOTAL ), totals.nr_data_size, totals.nr_hl_size, totals.nr_disk_size, totals.excess_fragments );
		}

	/* link control */
	if ( ctrl.linked != -1 ) {
		//ADD_HORIZ_LINE( c_left2_horiz1, c_bottom1_vert1, c_right2_horiz1, c_horiz1 );
		}
	else {
		//ADD_HORIZ_LINE( c_bottom2_left2, c_bottom2_vert1, c_bottom2_right2, c_horiz2 );
		}

	// hot key
	//memset( &di, 0, sizeof( di ) );
	//di.Type = DI_TEXT;
	//di.X1 = c_hframe_width + 1;
	//di.X2 = di.X1;
	//di.Y1 = di.Y2 = line;
	//SET_DATA( std::string( L"&0" ) );
	//if ( h_dlg != NULL ) {
	//	g_far.SendDlgMessage( h_dlg, DM_SETDLGITEM, ctrl_idx, &di );
	//	g_far.SendDlgMessage( h_dlg, DM_SHOWITEM, ctrl_idx, reinterpret_cast< void* >( ctrl.linked != -1 ? 1 : 0 ) );
	//	}
	//else {
	//	dlg_items += di;
	//	}
	ctrl_idx++;

	//memset( &di, 0, sizeof( di ) );
	//di.Type = DI_EDIT;
	//di.Flags = DIF_READONLY | DIF_SELECTONENTRY;
	//di.X1 = c_hframe_width + 2;
	//di.X2 = di.X1 + dlg_cl_width - 2;
	//di.Y1 = di.Y2 = line;
	//if ( ( ctrl.linked != -1 ) && ( h_dlg != NULL ) ) {
	//	std::string str = far_get_text( h_dlg, ctrl.linked );
	//	SET_DATA( str );
	//	}
	//else {
	//	SET_DATA( empty_str );
	//	}
	//if ( h_dlg != NULL ) {
	//	g_far.SendDlgMessage( h_dlg, DM_SETDLGITEM, ctrl_idx, &di );
	//	g_far.SendDlgMessage( h_dlg, DM_SHOWITEM, ctrl_idx, reinterpret_cast< void* >( ctrl.linked != -1 ? 1 : 0 ) );
	//	}
	//else {
	//	dlg_items += di;
	//	ctrl.link = ctrl_idx;
	//	}
	ctrl_idx++;

	if ( ctrl.linked != -1 ) {
		//ADD_HORIZ_LINE( c_vert2, ' ', c_vert2, ' ' );
		//ADD_HORIZ_LINE( c_bottom2_left2, c_horiz2, c_bottom2_right2, c_horiz2 );
		}

	/* dialog border */
	//dlg_str.add_fmt( L"\1%.*c", dlg_width, ' ' );
	//dlg_colors.add( g_colors[ COL_DIALOGTEXT ] );
	line++;

	/* custom control - dialog contents */
	ctrl_idx = 0;
	//memset( &di, 0, sizeof( di ) );
	//di.Type = DI_USERCONTROL;
	//di.Flags = DIF_NOFOCUS;
	//di.X1 = 0;
	//di.Y1 = 0;
	////di.X2 = dlg_width - 1;
	//di.Y2 = line - 1;
	//dlg_ci = str_to_char_info( dlg_str, dlg_colors );
	//di.VBuf = const_cast< FAR_CHAR_INFO* >( dlg_ci.data( ) );
	//if ( h_dlg != NULL ) {
	//	g_far.SendDlgMessage( h_dlg, DM_SETDLGITEM, ctrl_idx, &di );
	//	}
	//else {
	//	dlg_items.item( 0 ) = di;
	//	}

	unsigned dlg_height = line;

	if ( h_dlg != NULL ) {
		SMALL_RECT dlg_rect;
		//g_far.SendDlgMessage( h_dlg, DM_GETDLGRECT, 0, &dlg_rect );
		//if ( ( dlg_rect.Right - dlg_rect.Left + 1 != dlg_width ) || ( dlg_rect.Bottom - dlg_rect.Top + 1 != dlg_height ) ) {
			//COORD dlg_size;
			//dlg_size.X = dlg_width;
			//dlg_size.Y = dlg_height;
			//g_far.SendDlgMessage( h_dlg, DM_RESIZEDIALOG, 0, &dlg_size );
			//COORD dlg_pos = { -1, -1 };
			//g_far.SendDlgMessage( h_dlg, DM_MOVEDIALOG, TRUE, &dlg_pos );
			//}
		//g_far.SendDlgMessage( h_dlg, DM_ENABLEREDRAW, TRUE, nullptr );
		}
	else {
		//HANDLE h = g_far.DialogInit( &c_plugin_guid, &c_metadata_dialog_guid, -1, -1, dlg_width, dlg_height, L"metadata", dlg_items.data( ), dlg_items.size( ), 0, 0, dlg_proc, this );
		//if ( h != INVALID_HANDLE_VALUE ) {
		//	//g_far.DialogRun( h );
		//	//g_far.DialogFree( h );
		//	}
		//}
	}

void FileAnalyzer::update_totals( const FileInfo& file_info, bool hard_link ) {
	if ( file_info.reparse && file_info.directory ) {
		totals.dir_rp_cnt++;
		}
	else if ( file_info.directory ) {
		totals.dir_cnt++;
		}
	else if ( hard_link ) {
		totals.hl_cnt++;
		}
	else if ( file_info.reparse ) {
		totals.file_rp_cnt++;
		}
	else {
		totals.file_cnt++;
		}
	for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
		const AttrInfo& attr_info = file_info.attr_list[ i ];
		if ( !attr_info.resident ) {
			if ( hard_link ) {
				totals.nr_hl_size += attr_info.data_size;
				}
			else {
				totals.nr_data_size += attr_info.data_size;
				totals.nr_disk_size += attr_info.disk_size;
				}
			}
		if ( attr_info.type == AT_DATA ) {
			if ( attr_info.name.size( ) == 0 ) {
				if ( hard_link ) {
					totals.unnamed_hl_size += attr_info.data_size;
					}
				else {
					totals.unnamed_data_size += attr_info.data_size;
					totals.unnamed_disk_size += attr_info.disk_size;
					}
				}
			else {
				if ( hard_link ) {
					totals.named_hl_size += attr_info.data_size;
					}
				else {
					totals.named_data_size += attr_info.data_size;
					totals.named_disk_size += attr_info.disk_size;
					}
				}
			}
		if ( !hard_link && ( attr_info.fragments > 1 ) ) {
			totals.excess_fragments += attr_info.fragments - 1;
			}
		}
	}

//void FileAnalyzer::process_file( FileInfo& file_info, bool full_info ) {
//	BY_HANDLE_FILE_INFORMATION h_file_info;
//	//HANDLE h_file = CreateFileW( long_path( file_info.file_name ).data( ), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_POSIX_SEMANTICS, NULL );
//	if ( h_file == INVALID_HANDLE_VALUE ) {
//		FAIL( SystemError( ) );
//		}
//	try {
//		if ( GetFileInformationByHandle( h_file, &h_file_info ) == 0 ) {
//			FAIL( SystemError( ) );
//			}
//		}
//	finally ( CloseHandle( h_file ) );
//
//	u64 file_ref_num = ( ( u64 ) h_file_info.nFileIndexHigh << 32 ) + h_file_info.nFileIndexLow;
//	file_info.hard_link_cnt = h_file_info.nNumberOfLinks;
//	file_info.reparse = ( h_file_info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT;
//	file_info.directory = ( h_file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY;
//	if ( volume.serial != h_file_info.dwVolumeSerialNumber ) { // volume changed
//		volume.open( extract_path_root( get_real_path( extract_file_path( file_info.file_name ) ) ) );
//		}
//	file_info.volume = &volume;
//
//	if ( file_info.hard_link_cnt > 1 ) {
//		unsigned idx = hard_links.bsearch( file_info );
//		if ( idx != -1 ) {
//			update_totals( hard_links[ idx ], true );
//			}
//		else {
//			file_info.process_file( file_ref_num );
//			if ( full_info ) {
//				file_info.find_full_paths( );
//				}
//			update_totals( file_info, false );
//			hard_links += file_info;
//			hard_links.sort( );
//			}
//		}
//	else {
//		file_info.process_file( file_ref_num );
//		if ( full_info ) {
//			file_info.find_full_paths( );
//			}
//		update_totals( file_info, false );
//		}
//	}

void FileAnalyzer::process_recursive( const std::string& dir_name ) {
	try {
		bool root_dir = dir_name.last( ) == L'\\';
		WIN32_FIND_DATAW find_data;
		HANDLE h_find = FindFirstFileW( long_path( dir_name + ( root_dir ? L"*" : L"\\*" ) ).data( ), &find_data );
		if ( h_find == INVALID_HANDLE_VALUE ) {
			FAIL( SystemError( ) );
			}
		try {
			while ( true ) {
				if ( WaitForSingleObject( h_stop_event, 0 ) == WAIT_OBJECT_0 ) {
					break;
					}
				if ( FindNextFileW( h_find, &find_data ) == 0 ) {
					if ( GetLastError( ) == ERROR_NO_MORE_FILES ) {
						break;
						}
					else {
						FAIL( SystemError( ) );
						}
					}
				if ( ( wcscmp( find_data.cFileName, L"." ) == 0 ) || ( wcscmp( find_data.cFileName, L".." ) == 0 ) ) {
					continue;
					}

				FileInfo file_info;
				file_info.file_name = dir_name + ( root_dir ? L"" : L"\\" ) + find_data.cFileName;
				try {
					process_file( file_info, false );
					if ( file_info.directory && !file_info.reparse ) {
						process_recursive( file_info.file_name );
						}
					}
				catch ( Error& ) {
					totals.err_cnt++;
					}

				time_t ctime = time( NULL );
				if ( ctime > update_timer + c_update_time ) {
					display_file_info( true );
					update_timer = ctime;
					}
				}
			}
		finally ( FindClose( h_find ) );
		}
	catch ( Error& ) {
		totals.err_cnt++;
		}
	}

// set current file on active panel to file_name
//bool panel_go_to_file( const std::string& file_name ) {
//	std::string dir = file_name;
//	if ( ( dir.size( ) >= 4 ) && ( dir.equal( 0, 4, L"\\\\?\\" ) || dir.equal( 0, 4, L"\\??\\" ) ) ) {
//		dir.remove( 0, 4 );
//		}
//	if ( ( dir.size( ) < 2 ) || ( dir[ 1 ] != ':' ) ) {
//		return false;
//		}
//	unsigned pos = dir.rsearch( '\\' );
//	if ( pos == -1 ) {
//		return false;
//		}
//	std::string file = dir.slice( pos + 1, dir.size( ) - pos - 1 );
//	dir.remove( pos, dir.size( ) - pos );
//	if ( dir.size( ) == 2 ) {
//		dir += '\\';
//		}
//	if ( !far_set_panel_dir( INVALID_HANDLE_VALUE, dir ) ) {
//		return false;
//		}
//	PanelInfo panel_info = { sizeof( PanelInfo ) };
//	if ( !far_control_ptr( INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &panel_info ) ) {
//		return false;
//		}
//	PanelRedrawInfo panel_ri = { sizeof( PanelRedrawInfo ) };
//	size_t i;
//	for ( i = 0; i < panel_info.ItemsNumber; i++ ) {
//		PluginPanelItem* ppi = far_get_panel_item( INVALID_HANDLE_VALUE, i );
//		if ( !ppi ) {
//			return false;
//			}
//		if ( file == ppi->FileName ) {
//			panel_ri.CurrentItem = i;
//			panel_ri.TopPanelItem = 0;
//			break;
//			}
//		}
//	if ( i == panel_info.ItemsNumber ) {
//		return false;
//		}
//	if ( !far_control_ptr( INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, &panel_ri ) ) {
//		return false;
//		}
//	return true;
//	}

void WINAPI GetGlobalInfoW( GlobalInfo* info ) {
	info->StructSize = sizeof( GlobalInfo );
	info->MinFarVersion = FARMANAGERVERSION;
	//info->Version = PLUGIN_VERSION;
	//info->Guid = c_plugin_guid;
	//info->Title = PLUGIN_NAME;
	//info->Description = PLUGIN_DESCRIPTION;
	//info->Author = PLUGIN_AUTHOR;
	}

void WINAPI SetStartupInfoW( const PluginStartupInfo* info ) {
	enable_lfh( );
	g_far = *info;
	g_fsf = *info->FSF;
	g_far.FSF = &g_fsf;
	g_version = get_module_version( g_h_module );
	}

const wchar_t* c_command_prefix = L"nfi:nfc:defrag:nfv";
//void WINAPI GetPluginInfoW( PluginInfo* info ) {
//	static const wchar_t* plugin_menu[ 1 ];
//
//	info->StructSize = sizeof( struct PluginInfo );
//	info->Flags = PF_FULLCMDLINE | PF_VIEWER;
//	//plugin_menu[ 0 ] = far_msg_ptr( MSG_PLUGIN_NAME );
//	info->PluginMenu.Guids = &c_plugin_menu_guid;
//	info->PluginMenu.Strings = plugin_menu;
//	info->PluginMenu.Count = ARRAYSIZE( plugin_menu );
//	info->PluginConfig.Guids = &c_plugin_config_guid;
//	info->PluginConfig.Strings = plugin_menu;
//	info->PluginConfig.Count = ARRAYSIZE( plugin_menu );
//	info->CommandPrefix = c_command_prefix;
//	}

//void FileAnalyzer::process( ) {
//	if ( file_info.directory && !file_info.reparse ) {
//		process_recursive( file_info.file_name );
//		}
//	for ( unsigned i = 1; i < file_list.size( ); i++ ) {
//		FileInfo fi;
//		fi.file_name = file_list[ i ];
//		try {
//			process_file( fi, false );
//			if ( fi.directory && !fi.reparse ) {
//				process_recursive( fi.file_name );
//				}
//			}
//		catch ( Error& ) {
//			totals.err_cnt++;
//			}
//		time_t ctime = time( NULL );
//		if ( ctime > update_timer + c_update_time ) {
//			display_file_info( true );
//			update_timer = ctime;
//			}
//		}
//	display_file_info( );
//	}

unsigned __stdcall FileAnalyzer::th_proc( void* param ) {
	try {
		reinterpret_cast< FileAnalyzer* >( param )->process( );
		}
	catch ( Error& e ) {
		error_dlg( e );
		return FALSE;
		}
	return TRUE;
	}

//intptr_t FileAnalyzer::dialog_handler( intptr_t msg, intptr_t param1, void* param2 ) {
//	/* do we need background processing? */
//	bool bg_proc = ( file_info.directory && !file_info.reparse ) || ( file_list.size( ) > 1 );
//	if ( msg == DN_INITDIALOG ) {
//		NOFAIL( display_file_info( ) );
//		if ( bg_proc ) {
//			h_stop_event = CreateEvent( NULL, TRUE, FALSE, NULL );
//			CHECK_SYS( h_stop_event != NULL );
//			unsigned th_id;
//			h_thread = ( HANDLE ) _beginthreadex( NULL, 0, th_proc, this, 0, &th_id );
//			CHECK_SYS( h_thread != NULL );
//			}
//		return TRUE;
//		}
//	else if ( msg == DN_CLOSE ) {
//		if ( bg_proc ) {
//			SetEvent( h_stop_event );
//			if ( WaitForSingleObject( h_thread, 0 ) == WAIT_OBJECT_0 ) {
//				return TRUE;
//				}
//			else {
//				return FALSE;
//				}
//			}
//		else {
//			return TRUE;
//			}
//		}
//	else if ( msg == DN_CONTROLINPUT ) {
//		bool is_enter_or_dblclick = false;
//		if ( param1 != -1 ) {
//			const INPUT_RECORD* input_record = reinterpret_cast< const INPUT_RECORD* >( param2 );
//			if ( input_record->EventType == MOUSE_EVENT ) {
//				const MOUSE_EVENT_RECORD& mouse_evt = input_record->Event.MouseEvent;
//				if ( mouse_evt.dwEventFlags == DOUBLE_CLICK && ( mouse_evt.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED ) == FROM_LEFT_1ST_BUTTON_PRESSED ) {
//					is_enter_or_dblclick = true;
//					}
//				}
//			if ( input_record->EventType == KEY_EVENT ) {
//				const KEY_EVENT_RECORD& key_evt = input_record->Event.KeyEvent;
//				if ( key_evt.wVirtualKeyCode == VK_RETURN && key_evt.bKeyDown && ( key_evt.dwControlKeyState & ( LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED | RIGHT_CTRL_PRESSED | SHIFT_PRESSED ) ) == 0 ) {
//					is_enter_or_dblclick = true;
//					}
//				}
//			}
//		if ( is_enter_or_dblclick ) {
//			unsigned ctrl_id = param1;
//			if ( ( dlg_items[ ctrl_id ].Type == DI_EDIT ) || ( dlg_items[ ctrl_id ].Type == DI_FIXEDIT ) ) {
//				if ( ctrl_id == ctrl.linked ) {
//					ctrl.linked = -1;
//					NOFAIL( display_file_info( ) );
//					}
//				else if ( ctrl_id == ctrl.link ) {
//					std::string file_name = far_get_text( h_dlg, ctrl.link );
//					if ( panel_go_to_file( file_name ) ) g_far.SendDlgMessage( h_dlg, DM_CLOSE, -1, nullptr );
//					}
//				else {
//					ctrl.linked = ctrl_id;
//					NOFAIL( display_file_info( ) );
//					}
//				}
//			return TRUE;
//			}
//		else {
//			return FALSE;
//			}
//		}
//	else if ( msg == DN_RESIZECONSOLE ) {
//		NOFAIL( display_file_info( ) );
//		return TRUE;
//		}
//	else {
//		return g_far.DefDlgProc( h_dlg, msg, param1, param2 );
//		}
//	}
//
//intptr_t WINAPI FileAnalyzer::dlg_proc( HANDLE h_dlg, intptr_t msg, intptr_t param1, void* param2 ) {
//	BEGIN_ERROR_HANDLER;
//	FileAnalyzer* fa = reinterpret_cast< FileAnalyzer* >( g_far.SendDlgMessage( h_dlg, DM_GETDLGDATA, 0, nullptr ) );
//	fa->h_dlg = h_dlg;
//	return fa->dialog_handler( msg, param1, param2 );
//	END_ERROR_HANDLER( ; , ; );
//	return g_far.DefDlgProc( h_dlg, msg, param1, param2 );
//	}

// get file name from command line (when plugin prefix is used)
//bool file_list_from_cmdline( const std::string& cmd, std::vector<std::string>& file_list, std::string& prefix ) {
//	unsigned prefix_pos = cmd.search( L':' );
//	if ( prefix_pos == -1 ) {
//		return false;
//		}
//	prefix = cmd.left( prefix_pos );
//	std::string file_name = cmd.slice( prefix_pos + 1 );
//	unquote( file_name );
//	file_list = far_get_full_path( file_name );
//	return true;
//	}
//
//std::string get_unicode_file_path( const PluginPanelItem& panel_item, const std::string& dir_path, bool own_panel ) {
//	return dir_path + panel_item.FileName;
//	}
//
//// get list of the select files from panel and place it into global file list
//bool file_list_from_panel( std::vector<std::string>& file_list, bool own_panel ) {
//	if ( !far_control_int( INVALID_HANDLE_VALUE, FCTL_CHECKPANELSEXIST, 0 ) ) {
//		return false;
//		}
//	PanelInfo p_info = { sizeof( PanelInfo ) };
//	if ( !far_control_ptr( INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &p_info ) ) {
//		return false;
//		}
//	bool file_panel = ( p_info.PanelType == PTYPE_FILEPANEL ) && ( ( p_info.Flags & PFLAGS_REALNAMES ) == PFLAGS_REALNAMES );
//	bool tree_panel = ( p_info.PanelType == PTYPE_TREEPANEL ) && ( ( p_info.Flags & PFLAGS_REALNAMES ) == PFLAGS_REALNAMES );
//	if ( !file_panel && !tree_panel ) {
//		return false;
//		}
//
//	unsigned sel_file_cnt = 0;
//	if ( file_panel ) {
//		for ( size_t i = 0; i < p_info.SelectedItemsNumber; i++ ) {
//			PluginPanelItem* ppi = far_get_selected_panel_item( INVALID_HANDLE_VALUE, i );
//			if ( !ppi ) {
//				return false;
//				}
//			if ( ( ppi->Flags & PPIF_SELECTED ) == PPIF_SELECTED ) {
//				sel_file_cnt++;
//				}
//			}
//		}
//
//	file_list.clear( );
//	std::string cur_dir = far_get_panel_dir( INVALID_HANDLE_VALUE );
//	if ( sel_file_cnt != 0 ) {
//		file_list.extend( sel_file_cnt );
//		cur_dir = add_trailing_slash( cur_dir );
//		for ( size_t i = 0; i < p_info.SelectedItemsNumber; i++ ) {
//			PluginPanelItem* ppi = far_get_selected_panel_item( INVALID_HANDLE_VALUE, i );
//			if ( !ppi ) {
//				return false;
//				}
//			if ( ( ppi->Flags & PPIF_SELECTED ) == PPIF_SELECTED ) {
//				file_list += get_unicode_file_path( *ppi, cur_dir, own_panel );
//				}
//			}
//		}
//	else {
//		if ( file_panel ) {
//			if ( ( p_info.CurrentItem < 0 ) || ( p_info.CurrentItem >= p_info.ItemsNumber ) ) {
//				return false;
//				}
//			PluginPanelItem* ppi = far_get_panel_item( INVALID_HANDLE_VALUE, p_info.CurrentItem );
//			if ( !ppi ) {
//				return false;
//				}
//			if ( wcscmp( ppi->FileName, L".." ) == 0 ) { // current directory selected
//				if ( cur_dir.size( ) == 0 ) {
//					return false; // directory is invalid (plugin panel)
//					}
//				else {
//					file_list += cur_dir;
//					}
//				}
//			else {
//				cur_dir = add_trailing_slash( cur_dir );
//				file_list += get_unicode_file_path( *ppi, cur_dir, own_panel );
//				}
//			}
//		else { // Tree panel
//			file_list += cur_dir;
//			}
//		}
//
//	file_list.sort( );
//	return true;
//	}

enum FileType {
	ftFile,
	ftDirectory,
	ftReparse,
	};

FileType get_file_type( const std::string& file_name ) {
	DWORD attr = GetFileAttributesW( file_name.data( ) );
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		return ftFile;
		}
	if ( ( attr & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT ) {
		return ftReparse;
		}
	else if ( ( attr & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY ) {
		return ftDirectory;
		}
	else {
		return ftFile;
		}
	}

/* show file information dialog */
void plugin_show_metadata( const std::vector<std::string>& file_list ) {
	FileAnalyzer fa;
	fa.file_list = file_list;
	fa.volume.open( extract_path_root( get_real_path( extract_file_path( file_list[ 0 ] ) ) ) );
	fa.file_info.file_name = file_list[ 0 ];
	fa.process_file( fa.file_info, true );
	fa.display_file_info( );
	}

void plugin_process_contents( const std::vector<std::string>& file_list ) {
	bool single_file = ( file_list.size( ) == 1 ) && ( get_file_type( file_list[ 0 ] ) == ftFile );
	if ( show_options_dialog( g_content_options, single_file ) ) {
		store_plugin_options( );
		if ( single_file ) {
			ContentInfo content_info;
			process_file_content( file_list[ 0 ], g_content_options, content_info );
			show_result_dialog( file_list[ 0 ], g_content_options, content_info );
			}
		else {
			CompressionStats stats;
			compress_files( file_list, stats );
			show_result_dialog( stats );
			}
		}
	}

//HANDLE WINAPI OpenW( const OpenInfo* info ) {
//	HANDLE handle = nullptr;
//	BEGIN_ERROR_HANDLER;
//	/* load plugin configuration settings from registry */
//	load_suffixes( );
//	load_plugin_options( );
//
//	/* load Far color table */
//	unsigned colors_size = ( unsigned ) g_far.AdvControl( &c_plugin_guid, ACTL_GETARRAYCOLOR, 0, nullptr );
//	g_far.AdvControl( &c_plugin_guid, ACTL_GETARRAYCOLOR, colors_size, g_colors.buf( colors_size ) );
//	g_colors.set_size( colors_size );
//
//	std::vector<std::string> file_list;
//
//	if ( info->OpenFrom == OPEN_COMMANDLINE ) {
//		std::string prefix;
//		if ( file_list_from_cmdline( ( const wchar_t* ) reinterpret_cast< OpenCommandLineInfo* >( info->Data )->CommandLine, file_list, prefix ) ) {
//			if ( prefix == L"nfi" ) {
//				plugin_show_metadata( file_list );
//				}
//			else if ( prefix == L"nfc" ) {
//				plugin_process_contents( file_list );
//				}
//			else if ( prefix == L"defrag" ) {
//				defragment( file_list, Log( ) );
//				far_control_int( INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 1 );
//				far_control_int( PANEL_PASSIVE, FCTL_UPDATEPANEL, 1 );
//				far_control_ptr( INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, nullptr );
//				far_control_ptr( PANEL_PASSIVE, FCTL_REDRAWPANEL, NULL );
//				}
//			else if ( prefix == L"nfv" ) {
//				plugin_show_file_version( file_list[ 0 ] );
//				}
//			}
//		}
//	else { // OPEN_PLUGINSMENU or OPEN_VIEWER
//		bool from_viewer = info->OpenFrom == OPEN_VIEWER;
//
//		// check if current panel belongs to our plugin
//		FilePanel* active_panel = nullptr;
//		if ( !from_viewer ) {
//			active_panel = FilePanel::get_active_panel( );
//			}
//
//		/* display plugin menu */
//		std::vector<std::string> menu_items;
//		unsigned metadata_menu_id = -1;
//		if ( !from_viewer ) {
//			//menu_items += far_get_msg( MSG_MENU_METADATA );
//			metadata_menu_id = menu_items.size( ) - 1;
//			}
//		//menu_items += far_get_msg( MSG_MENU_CONTENT );
//		unsigned content_menu_id = menu_items.size( ) - 1;
//		unsigned toggle_panel_menu_id = -1;
//		unsigned defragment_menu_id = -1;
//		if ( !from_viewer ) {
//			//menu_items += far_get_msg( active_panel == NULL ? MSG_MENU_PANEL_ON : MSG_MENU_PANEL_OFF );
//			toggle_panel_menu_id = menu_items.size( ) - 1;
//			//menu_items += far_get_msg( MSG_MENU_DEFRAGMENT );
//			defragment_menu_id = menu_items.size( ) - 1;
//			}
//		//menu_items += far_get_msg( MSG_MENU_FILE_VERSION );
//		unsigned version_menu_id = menu_items.size( ) - 1;
//		unsigned compress_files_menu_id = -1;
//		if ( !from_viewer ) {
//			//menu_items += far_get_msg( MSG_MENU_COMPRESS_FILES );
//			compress_files_menu_id = menu_items.size( ) - 1;
//			}
//		unsigned flat_mode_menu_id = -1;
//		unsigned mft_mode_menu_id = -1;
//		unsigned show_totals_menu_id = -1;
//		if ( active_panel && active_panel->current_dir.size( ) ) {
//			//menu_items += far_get_msg( active_panel->flat_mode ? MSG_MENU_FLAT_MODE_OFF : MSG_MENU_FLAT_MODE_ON );
//			flat_mode_menu_id = menu_items.size( ) - 1;
//			//menu_items += far_get_msg( active_panel->mft_mode ? MSG_MENU_MFT_MODE_OFF : MSG_MENU_MFT_MODE_ON );
//			mft_mode_menu_id = menu_items.size( ) - 1;
//			if ( active_panel->mft_mode ) {
//				//menu_items += far_get_msg( MSG_MENU_SHOW_TOTALS );
//				show_totals_menu_id = menu_items.size( ) - 1;
//				}
//			}
//		//int item_idx = far_menu( c_main_menu_guid, far_get_msg( MSG_PLUGIN_NAME ), menu_items, L"plugin_menu" );
//		//if ( item_idx == -1 ) {
//		//  BREAK;
//		//}
//
//		//if ( item_idx == content_menu_id || item_idx == version_menu_id ) {
//		//	if ( from_viewer ) {
//		//		int len = g_far.ViewerControl( -1, VCTL_GETFILENAME, 0, nullptr );
//		//		if ( len == 0 ) {
//		//			return nullptr;
//		//			}
//		//		std::string file_name;
//		//		g_far.ViewerControl( -1, VCTL_GETFILENAME, len, file_name.buf( len ) );
//		//		file_name.set_size( len - 1 );
//		//		file_list = file_name;
//		//		}
//		//	else {
//		//		if ( !file_list_from_panel( file_list, active_panel != NULL ) ) {
//		//			return nullptr;
//		//			}
//		//		}
//		//	}
//
//		if ( item_idx == metadata_menu_id ) {
//			if ( file_list_from_panel( file_list, active_panel != NULL ) ) {
//				plugin_show_metadata( file_list );
//				}
//			}
//		else if ( item_idx == content_menu_id ) {
//			plugin_process_contents( file_list );
//			}
//		else if ( item_idx == toggle_panel_menu_id ) {
//			if ( active_panel == NULL ) {
//				handle = FilePanel::open( );
//				}
//			else {
//				active_panel->close( );
//				}
//			}
//		else if ( item_idx == defragment_menu_id ) {
//			if ( file_list_from_panel( file_list, active_panel != NULL ) ) {
//				Log log;
//				defragment( file_list, log );
//				if ( log.size( ) != 0 ) {
//					if ( far_message( c_defrag_errors_dialog_guid, far_get_msg( MSG_PLUGIN_NAME ) + L"\n" + word_wrap( far_get_msg( MSG_DEFRAG_ERRORS ), get_msg_width( ) ) + L"\n" + far_get_msg( MSG_BUTTON_OK ) + L"\n" + far_get_msg( MSG_LOG_SHOW ), 2, FMSG_WARNING ) == 1 ) {
//						log.show( );
//						}
//					}
//				far_control_int( INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 1 );
//				far_control_int( PANEL_PASSIVE, FCTL_UPDATEPANEL, 1 );
//				}
//			}
//		else if ( item_idx == version_menu_id ) {
//			plugin_show_file_version( file_list[ 0 ] );
//			}
//		else if ( item_idx == compress_files_menu_id ) {
//			if ( file_list_from_panel( file_list, active_panel != NULL ) ) {
//				if ( show_compress_files_dialog( g_compress_files_params ) ) {
//					store_plugin_options( );
//					Log log;
//					plugin_compress_files( file_list, g_compress_files_params, log );
//					if ( log.size( ) ) {
//						if ( far_message( c_compress_errors_dialog_guid, far_get_msg( MSG_PLUGIN_NAME ) + L"\n" + word_wrap( far_get_msg( MSG_COMPRESS_FILES_ERRORS ), get_msg_width( ) ) + L"\n" + far_get_msg( MSG_BUTTON_OK ) + L"\n" + far_get_msg( MSG_LOG_SHOW ), 2, FMSG_WARNING ) == 1 ) {
//							log.show( );
//							}
//						}
//					far_control_int( INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 1 );
//					far_control_int( PANEL_PASSIVE, FCTL_UPDATEPANEL, 1 );
//					}
//				}
//			}
//		else if ( item_idx == flat_mode_menu_id ) {
//			active_panel->flat_mode = !active_panel->flat_mode;
//			far_control_int( active_panel, FCTL_UPDATEPANEL, 1 );
//			}
//		else if ( item_idx == mft_mode_menu_id ) {
//			active_panel->toggle_mft_mode( );
//			far_control_int( active_panel, FCTL_UPDATEPANEL, 1 );
//			}
//		else if ( item_idx == show_totals_menu_id ) {
//			if ( file_list_from_panel( file_list, true ) ) {
//				FilePanel::Totals totals = active_panel->mft_get_totals( file_list );
//				std::string msg;
//				msg.add( far_get_msg( MSG_SHOW_TOTALS_TITLE ) ).add( L"\n" );
//				msg.add_fmt( far_get_msg( MSG_SHOW_TOTALS_DATA_SIZE ).data( ), &format_data_size( totals.data_size, size_suffixes ), totals.data_size ).add( L"\n" );
//				msg.add_fmt( far_get_msg( MSG_SHOW_TOTALS_DISK_SIZE ).data( ), &format_data_size( totals.disk_size, size_suffixes ), totals.disk_size ).add( L"\n" );
//				if ( totals.file_cnt + totals.dir_cnt != 0 ) {
//					msg.add_fmt( far_get_msg( MSG_SHOW_TOTALS_FRAGMENTS ).data( ), totals.fragment_cnt, totals.fragment_cnt / ( totals.file_cnt + totals.dir_cnt ) ).add( L"\n" );
//					}
//				msg.add_fmt( far_get_msg( MSG_SHOW_TOTALS_FILES ).data( ), totals.file_cnt, totals.hl_cnt, totals.file_rp_cnt ).add( L"\n" );
//				msg.add_fmt( far_get_msg( MSG_SHOW_TOTALS_DIRS ).data( ), totals.dir_cnt, totals.dir_rp_cnt ).add( L"\n" );
//				msg.add( far_get_msg( MSG_BUTTON_OK ) );
//				far_message( c_show_totals_dialog_guid, msg, 1, FMSG_LEFTALIGN );
//				}
//			}
//		}
//	END_ERROR_HANDLER( return handle, return nullptr );
//	}

void WINAPI ClosePanelW( const ClosePanelInfo* info ) {
	BEGIN_ERROR_HANDLER;
	FilePanel* panel = reinterpret_cast< FilePanel* >( info->hPanel );
	panel->on_close( );
	END_ERROR_HANDLER( ; , ; );
	}

void WINAPI GetOpenPanelInfoW( OpenPanelInfo* info ) {
	BEGIN_ERROR_HANDLER;
	FilePanel* panel = reinterpret_cast< FilePanel* >( info->hPanel );
	panel->fill_plugin_info( info );
	END_ERROR_HANDLER( ; , ; );
	}

intptr_t WINAPI GetFindDataW( GetFindDataInfo* info ) {
	BEGIN_ERROR_HANDLER;
	FilePanel* panel = reinterpret_cast< FilePanel* >( info->hPanel );
	try {
		panel->new_file_list( info->PanelItem, info->ItemsNumber, ( info->OpMode & OPM_FIND ) != 0 );
		}
	catch ( ... ) {
		if ( info->OpMode & ( OPM_FIND | OPM_SILENT ) ) {
			BREAK;
			}
		throw;
		}
	END_ERROR_HANDLER( return TRUE, return FALSE );
	}

void WINAPI FreeFindDataW( const FreeFindDataInfo* info ) {
	BEGIN_ERROR_HANDLER;
	FilePanel* panel = reinterpret_cast< FilePanel* >( info->hPanel );
	panel->clear_file_list( info->PanelItem );
	END_ERROR_HANDLER( ; , ; );
	}

intptr_t WINAPI SetDirectoryW( const SetDirectoryInfo* info ) {
	BEGIN_ERROR_HANDLER;
	FilePanel* panel = reinterpret_cast< FilePanel* >( info->hPanel );
	try {
		panel->change_directory( std::string( info->Dir ), ( info->OpMode & OPM_FIND ) != 0 );
		}
	catch ( ... ) {
		if ( info->OpMode & ( OPM_FIND | OPM_SILENT ) ) {
			BREAK;
			}
		throw;
		}
	END_ERROR_HANDLER( return TRUE, return FALSE );
	}

intptr_t WINAPI ConfigureW( const ConfigureInfo* info ) {
	BEGIN_ERROR_HANDLER;
	load_plugin_options( );
	FilePanelMode file_panel_mode = g_file_panel_mode;
	if ( show_file_panel_mode_dialog( file_panel_mode ) ) {
		bool need_reload = ( file_panel_mode.show_streams != g_file_panel_mode.show_streams ) || ( file_panel_mode.show_main_stream != g_file_panel_mode.show_main_stream );
		g_file_panel_mode = file_panel_mode;
		store_plugin_options( );
		if ( need_reload ) {
			FilePanel::reload_mft_all( );
			}
		}
	else {
		return FALSE;
		}
	END_ERROR_HANDLER( return TRUE, return FALSE );
	}

intptr_t WINAPI ProcessPanelInputW( const ProcessPanelInputInfo* info ) {
	BEGIN_ERROR_HANDLER;
	FilePanel* panel = reinterpret_cast< FilePanel* >( info->hPanel );
	if ( info->Rec.EventType == KEY_EVENT ) {
		const KEY_EVENT_RECORD& key_event = info->Rec.Event.KeyEvent;
		if ( ( key_event.dwControlKeyState & ( LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED ) ) != 0 && key_event.wVirtualKeyCode == 'R' ) {
			if ( !g_file_panel_mode.use_usn_journal ) {
				panel->reload_mft( );
				}
			return FALSE;
			}
		}
	return FALSE;
	END_ERROR_HANDLER( return TRUE, return TRUE );
	}

intptr_t WINAPI ProcessSynchroEventW( const ProcessSynchroEventInfo* info ) {
	BEGIN_ERROR_HANDLER;
	if ( info->Event == SE_COMMONSYNCHRO ) {
		FilePanel* panel = reinterpret_cast< FilePanel* >( info->Param );
		panel->apply_saved_state( );
		}
	END_ERROR_HANDLER( return 0, return 0 );
	}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {
	if ( fdwReason == DLL_PROCESS_ATTACH ) {
		g_h_module = hinstDLL;
		}
	return TRUE;
	}