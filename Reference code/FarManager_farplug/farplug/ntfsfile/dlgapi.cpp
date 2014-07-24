#define _ERROR_WINDOWS
#include "error.h"

#include "guids.h"
#include "utils.h"
#include "dlgapi.h"

extern struct PluginStartupInfo g_far;
extern Array<FarColor> g_colors;

// dialog frame size
const unsigned c_x_frame = 5;
const unsigned c_y_frame = 2;

// frame characters
const wchar_t c_horiz1 = 9472;
const wchar_t c_vert1 = 9474;
const wchar_t c_horiz2 = 9552;
const wchar_t c_vert2 = 9553;
const wchar_t c_top2_left2 = 9556;
const wchar_t c_top2_right2 = 9559;
const wchar_t c_bottom2_left2 = 9562;
const wchar_t c_bottom2_right2 = 9565;
const wchar_t c_left2_horiz1 = 9567;
const wchar_t c_right2_horiz1 = 9570;
const wchar_t c_top1_vert1 = 9516;
const wchar_t c_bottom1_vert1 = 9524;
const wchar_t c_bottom2_vert1 = 9575;
const wchar_t c_cross1 = 9532;

// progress bar chars
const wchar_t c_pb_black = 9608;
const wchar_t c_pb_white = 9617;

std::string far_get_msg( int id ) {
	return g_far.GetMsg( &c_plugin_guid, id );
	}

const wchar_t* far_msg_ptr( int id ) {
	return g_far.GetMsg( &c_plugin_guid, id );
	}

// optimum client area width of message dialogs
unsigned get_msg_width( ) {
	SMALL_RECT console_rect;
	if ( g_far.AdvControl( &c_plugin_guid, ACTL_GETFARRECT, 0, &console_rect ) ) {
		unsigned con_width = console_rect.Right - console_rect.Left + 1;
		if ( con_width >= 80 ) {
			return con_width - 20;
			}
		}
	return 60;
	}

int far_message( const GUID& guid, const std::string& msg, int button_cnt, FARMESSAGEFLAGS flags ) {
	return g_far.Message( &c_plugin_guid, &guid, flags | FMSG_ALLINONE, nullptr, reinterpret_cast< const wchar_t* const* >( msg.data( ) ), 0, button_cnt );
	}

int far_menu( const GUID& guid, const std::string& title, const std::vector<std::string>& items, const wchar_t* help ) {
	Array<FarMenuItem> menu_items;
	FarMenuItem mi;
	for ( unsigned i = 0; i < items.size( ); i++ ) {
		memset( &mi, 0, sizeof( mi ) );
		mi.Text = items[ i ].data( );
		menu_items += mi;
		}
	return g_far.Menu( &c_plugin_guid, &guid, -1, -1, 0, FMENU_WRAPMODE, title.data( ), NULL, help, NULL, NULL, menu_items.data( ), menu_items.size( ) );
	}

int far_viewer( const std::string& file_name, const std::string& title ) {
	return g_far.Viewer( file_name.data( ), title.data( ), 0, 0, -1, -1, VF_DISABLEHISTORY | VF_ENABLE_F6 | VF_NONMODAL, CP_UNICODE );
	}

unsigned get_label_len( const std::string& str ) {
	unsigned cnt = 0;
	for ( unsigned i = 0; i < str.size( ); i++ ) {
		if ( str[ i ] != '&' ) {
			cnt++;
			}
		}
	return cnt;
	}

void draw_text_box( const std::string& title, const std::vector<std::string>& lines, unsigned client_xs ) {
	unsigned size = 0;
	for ( unsigned i = 0; i < lines.size( ); i++ ) {
		size += min( lines[ i ].size( ), client_xs ) + 1;
		}
	std::string text;
	text.extend( title.size( ) + 1 + size );
	text += title;
	text += L'\n';
	for ( unsigned i = 0; i < lines.size( ); i++ ) {
		if ( lines[ i ].size( ) <= client_xs ) {
			text += lines[ i ];
			}
		else {
			text += lines[ i ].left( client_xs );
			}
		text += L'\n';
		}
	far_message( c_progress_dialog_guid, text, 0, FMSG_LEFTALIGN );
	}

void FarDialog::set_text( FarDialogItem& di, const std::string& text ) {
	values += text;
	di.Data = values.last( ).data( );
	}

void FarDialog::set_buf( FarDialogItem& di, const std::string& text ) {
	set_text( di, text );
	}

void FarDialog::set_list_text( FarListItem& li, const std::string& text ) {
	values += text;
	li.Text = values.last( ).data( );
	}

void FarDialog::frame( const std::string& text ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_DOUBLEBOX;
	di.X1 = c_x_frame - 2;
	di.Y1 = c_y_frame - 1;
	di.X2 = c_x_frame + client_xs + 1;
	di.Y2 = c_y_frame + client_ys;
	set_text( di, text );
	new_item( di );
	}

void FarDialog::calc_frame_size( ) {
	client_ys = y - c_y_frame;
	FarDialogItem& di = items.item( index[ 0 ] ); // dialog frame
	di.X2 = c_x_frame + client_xs + 1;
	di.Y2 = c_y_frame + client_ys;
	}

unsigned FarDialog::new_item( const FarDialogItem& di ) {
	items += di;
	index += items.size( ) - 1;
	return index.size( ) - 1;
	}

FarDialog::FarDialog( const GUID& guid, const std::string& title, unsigned width ) : guid( guid ), client_xs( width ), x( c_x_frame ), y( c_y_frame ) {
	frame( title );
	}

void FarDialog::new_line( ) {
	x = c_x_frame;
	y++;
	}

void FarDialog::spacer( unsigned size ) {
	x += size;
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	}

void FarDialog::pad( unsigned pos ) {
	if ( pos > x - c_x_frame ) {
		spacer( pos - ( x - c_x_frame ) );
		}
	}

void FarDialog::same_pos( ) {
	if ( items.size( ) ) {
		x = items.last( ).X1;
		y = items.last( ).Y1;
		}
	}

unsigned FarDialog::separator( ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_TEXT;
	di.Y1 = y;
	di.Y2 = y;
	di.Flags = DIF_SEPARATOR;
	return new_item( di );
	}

unsigned FarDialog::separator( const std::string& text ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_TEXT;
	di.Y1 = y;
	di.Y2 = y;
	di.Flags = DIF_SEPARATOR;
	set_text( di, text );
	return new_item( di );
	}

unsigned FarDialog::label( const std::string& text, unsigned boxsize, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_TEXT;
	di.X1 = x;
	di.Y1 = y;
	if ( boxsize == AUTO_SIZE ) {
		x += get_label_len( text );
		}
	else x += boxsize;
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.X2 = x - 1;
	di.Y2 = y;
	di.Flags = flags;
	set_text( di, text );
	return new_item( di );
	}

unsigned FarDialog::var_edit_box( const std::string& text, unsigned boxsize, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_EDIT;
	di.X1 = x;
	di.Y1 = y;
	if ( boxsize == AUTO_SIZE ) {
		x = c_x_frame + client_xs;
		}
	else {
		x += boxsize;
		}
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.X2 = x - 1;
	di.Y2 = y;
	di.Flags = flags;
	set_buf( di, text );
	return new_item( di );
	}

unsigned FarDialog::mask_edit_box( const std::string& text, const std::string& mask, unsigned boxsize, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_FIXEDIT;
	di.X1 = x;
	di.Y1 = y;
	if ( boxsize == AUTO_SIZE ) {
		x += mask.size( );
		}
	else {
		x += boxsize;
		}
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.X2 = x - 1;
	di.Y2 = y;
	values += mask;
	di.Mask = values.last( ).data( );
	di.Flags = DIF_MASKEDIT | flags;
	set_buf( di, text );
	return new_item( di );
	}

unsigned FarDialog::fix_edit_box( const std::string& text, unsigned boxsize, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_FIXEDIT;
	di.X1 = x;
	di.Y1 = y;
	if ( boxsize == AUTO_SIZE ) {
		x += text.size( );
		}
	else {
		x += boxsize;
		}
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.X2 = x - 1;
	di.Y2 = y;
	di.Flags = flags;
	set_buf( di, text );
	return new_item( di );
	}

unsigned FarDialog::button( const std::string& text, DWORD flags, bool def ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_BUTTON;
	di.X1 = x;
	di.Y1 = y;
	x += get_label_len( text ) + 4;
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.Y2 = y;
	di.Flags = flags | ( def ? DIF_DEFAULTBUTTON : 0 );
	set_text( di, text );
	return new_item( di );
	}

unsigned FarDialog::check_box( const std::string& text, int value, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_CHECKBOX;
	di.X1 = x;
	di.Y1 = y;
	x += get_label_len( text ) + 4;
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.Y2 = y;
	di.Flags = flags;
	di.Selected = value;
	set_text( di, text );
	return new_item( di );
	}

unsigned FarDialog::radio_button( const std::string& text, bool value, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_RADIOBUTTON;
	di.X1 = x;
	di.Y1 = y;
	x += get_label_len( text ) + 4;
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.Y2 = y;
	di.Flags = flags;
	di.Selected = value ? 1 : 0;
	set_text( di, text );
	return new_item( di );
	}

unsigned FarDialog::combo_box( const std::vector<std::string>& list_items, unsigned sel_idx, unsigned boxsize, DWORD flags ) {
	FarDialogItem di;
	memset( &di, 0, sizeof( di ) );
	di.Type = DI_COMBOBOX;
	di.X1 = x;
	di.Y1 = y;
	if ( boxsize == AUTO_SIZE ) {
		x = c_x_frame + client_xs;
		}
	else {
		x += boxsize;
		}
	if ( x - c_x_frame > client_xs ) {
		client_xs = x - c_x_frame;
		}
	di.X2 = x - 2;
	di.Y2 = y;
	di.Flags = flags;
	ListData list_data;
	for ( unsigned i = 0; i < list_items.size( ); i++ ) {
		FarListItem li;
		memset( &li, 0, sizeof( li ) );
		set_list_text( li, list_items[ i ] );
		if ( i == sel_idx ) {
			li.Flags = LIF_SELECTED;
			}
		list_data.items += li;
		}
	FarList fl = { sizeof( FarList ) };
	fl.ItemsNumber = list_data.items.size( );
	fl.Items = const_cast< FarListItem* >( list_data.items.data( ) );
	list_data.list = fl;
	lists += list_data;
	di.ListItems = const_cast< FarList* >( lists.last( ).list.data( ) );
	set_buf( di, L"" );
	return new_item( di );
	}

int FarDialog::show( FARWINDOWPROC dlg_proc, const wchar_t* help ) {
	calc_frame_size( );
	int res = -1;
	HANDLE h_dlg = g_far.DialogInit( &c_plugin_guid, &guid, -1, -1, client_xs + 2 * c_x_frame, client_ys + 2 * c_y_frame, help, items.buf( ), items.size( ), 0, 0, dlg_proc, this );
	if ( h_dlg != INVALID_HANDLE_VALUE ) {
		res = g_far.DialogRun( h_dlg );
		g_far.DialogFree( h_dlg );
		}
	return res;
	}

FarDialog* FarDialog::get_dlg( HANDLE h_dlg ) {
	FarDialog* dlg = ( FarDialog* ) g_far.SendDlgMessage( h_dlg, DM_GETDLGDATA, 0, 0 );
	dlg->h_dlg = h_dlg;
	return dlg;
	}

std::string FarDialog::get_text( unsigned ctrl_id ) {
	FarDialogItemData item = { sizeof( FarDialogItemData ) };
	item.PtrLength = g_far.SendDlgMessage( h_dlg, DM_GETTEXT, index[ ctrl_id ], nullptr );
	std::string text;
	item.PtrData = text.buf( item.PtrLength + 1 );
	g_far.SendDlgMessage( h_dlg, DM_GETTEXT, index[ ctrl_id ], &item );
	text.set_size( item.PtrLength );
	return text;
	}

void FarDialog::set_text( unsigned ctrl_id, const std::string& text ) {
	g_far.SendDlgMessage( h_dlg, DM_SETTEXTPTR, index[ ctrl_id ], const_cast< wchar_t* >( text.data( ) ) );
	}

bool FarDialog::get_check( unsigned ctrl_id ) {
	return DlgItem_GetCheck( g_far, h_dlg, index[ ctrl_id ] ) == BSTATE_CHECKED;
	}

void FarDialog::set_check( unsigned ctrl_id, bool check ) {
	g_far.SendDlgMessage( h_dlg, DM_SETCHECK, index[ ctrl_id ], reinterpret_cast< void* >( check ? BSTATE_CHECKED : BSTATE_UNCHECKED ) );
	}

unsigned FarDialog::get_list_pos( unsigned ctrl_id ) {
	return ( unsigned ) g_far.SendDlgMessage( h_dlg, DM_LISTGETCURPOS, index[ ctrl_id ], 0 );
	}

void FarDialog::set_focus( unsigned ctrl_id ) {
	g_far.SendDlgMessage( h_dlg, DM_SETFOCUS, index[ ctrl_id ], 0 );
	}

void FarDialog::enable( unsigned ctrl_id, bool enable ) {
	g_far.SendDlgMessage( h_dlg, DM_ENABLE, index[ ctrl_id ], reinterpret_cast< void* >( enable ? TRUE : FALSE ) );
	}

void FarDialog::set_visible( unsigned ctrl_id, bool visible ) {
	g_far.SendDlgMessage( h_dlg, DM_SHOWITEM, ctrl_id, reinterpret_cast< void* >( visible ? 1 : 0 ) );
	}

void FarDialog::link_label( unsigned ctrl_id, unsigned label_id ) {
	assert( ctrl_id != label_id );
	unsigned label_idx = index[ label_id ];
	FarDialogItem label = items[ label_idx ];
	items.remove( label_idx );
	for ( unsigned i = 0; i < index.size( ); i++ ) {
		if ( index[ i ] > label_idx ) {
			index.item( i )--;
			}
		}
	unsigned ctrl_idx = index[ ctrl_id ];
	items.insert( ctrl_idx, label );
	for ( unsigned i = 0; i < index.size( ); i++ ) {
		if ( index[ i ] >= ctrl_idx ) {
			index.item( i )++;
			}
		}
	index.item( label_id ) = ctrl_idx;
	}