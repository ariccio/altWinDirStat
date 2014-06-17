#define _ERROR_WINDOWS
#include "error.h"
#include <Windows.h>
#include <array>
#include <iostream>
#include <sstream>

#include "guids.h"
#include "dlgapi.h"
#include "options.h"
#include "log.h"
#include "utils.h"

extern struct PluginStartupInfo g_far;
extern struct FarStandardFunctions g_fsf;

//// unicode <-> oem codepage conversions
//void unicode_to_oem( AnsiString& oem_str, const std::string& u_str ) {
//	unsigned size = u_str.size( ) + 1;
//	int res = WideCharToMultiByte( CP_OEMCP, 0, u_str.data( ), size, oem_str.buf( u_str.size( ) ), size, NULL, NULL );
//	if ( res == 0 ) {
//		FAIL( SystemError( ) );
//		}
//	oem_str.set_size( res - 1 );
//	}

//void oem_to_unicode( std::string& u_str, const AnsiString& oem_str ) {
//	unsigned size = oem_str.size( ) + 1;
//	int res = MultiByteToWideChar( CP_OEMCP, 0, oem_str.data( ), size, u_str.buf( oem_str.size( ) ), size );
//	if ( res == 0 ) {
//		FAIL( SystemError( ) );
//		}
//	u_str.set_size( res - 1 );
//	}
//
//AnsiString unicode_to_oem( const std::string& u_str ) {
//	AnsiString oem_str;
//	unicode_to_oem( oem_str, u_str );
//	return oem_str;
//	}
//
//std::string oem_to_unicode( const AnsiString& oem_str ) {
//	std::string u_str;
//	oem_to_unicode( u_str, oem_str );
//	return u_str;
//	}
//
//AnsiString unicode_to_ansi( const std::string& u_str ) {
//	AnsiString ansi_str;
//	unsigned size = u_str.size( ) + 1;
//	int res = WideCharToMultiByte( CP_ACP, 0, u_str.data( ), size, ansi_str.buf( u_str.size( ) ), size, NULL, NULL );
//	CHECK_SYS( res );
//	ansi_str.set_size( res - 1 );
//	return ansi_str;
//	}
//
//std::string ansi_to_unicode( const AnsiString& a_str, unsigned code_page ) {
//	std::string u_str;
//	unsigned a_size = a_str.size( );
//	if ( a_size == 0 ) {
//		return u_str;
//		}
//	int u_size = MultiByteToWideChar( code_page, 0, a_str.data( ), a_size, NULL, 0 );
//	u_size = MultiByteToWideChar( code_page, 0, a_str.data( ), a_size, u_str.buf( u_size ), u_size );
//	u_str.set_size( u_size );
//	return u_str;
//	}
//
//AnsiString unicode_to_ansi( const std::string& u_str, unsigned code_page ) {
//	AnsiString a_str;
//	unsigned u_size = u_str.size( );
//	if ( u_size == 0 ) {
//		return a_str;
//		}
//	int a_size = WideCharToMultiByte( code_page, 0, u_str.data( ), u_size, NULL, 0, NULL, NULL );
//	a_size = WideCharToMultiByte( code_page, 0, u_str.data( ), u_size, a_str.buf( a_size ), a_size, NULL, NULL );
//	a_str.set_size( a_size );
//	return a_str;
//	}

// format amount of information
std::string format_inf_amount( u64 size ) {
	std::string str1, str2;
	str1.copy_fmt( L"%Lu", size );
	for ( unsigned i = 0; i < str1.size( ); i++ ) {
		if ( ( ( str1.size( ) - i ) % 3 == 0 ) && ( i != 0 ) ) {
			str2 += ',';
			}
		str2 += str1[ i ];
		}
	return str2;
	}

std::string format_inf_amount_short( u64 size, bool speed ) {
	return format_data_size( size, speed ? speed_suffixes : size_suffixes );
	}

std::vector<std::string> size_suffixes;
std::vector<std::string> speed_suffixes;
std::vector<std::string> short_size_suffixes;

void load_suffixes( ) {
	//size_suffixes.clear( );
	//size_suffixes += std::string( );
	//size_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SIZE_KB : MSG_SUFFIX_SIZE_KB );
	//size_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SIZE_MB : MSG_SUFFIX_SIZE_MB );
	//size_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SIZE_GB : MSG_SUFFIX_SIZE_GB );
	//size_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SIZE_TB : MSG_SUFFIX_SIZE_TB );
	//speed_suffixes.clear( );
	//speed_suffixes += far_get_msg( MSG_SUFFIX_SPEED_B );
	//speed_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SPEED_KB : MSG_SUFFIX_SPEED_KB );
	//speed_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SPEED_MB : MSG_SUFFIX_SPEED_MB );
	//speed_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SPEED_GB : MSG_SUFFIX_SPEED_GB );
	//speed_suffixes += far_get_msg( g_use_standard_inf_units ? MSG_SUFFIX_ALT_SPEED_TB : MSG_SUFFIX_SPEED_TB );
	//short_size_suffixes.clear( );
	//short_size_suffixes += std::string( L" " );
	//short_size_suffixes += far_get_msg( MSG_SUFFIX_SHORT_SIZE_KB );
	//short_size_suffixes += far_get_msg( MSG_SUFFIX_SHORT_SIZE_MB );
	//short_size_suffixes += far_get_msg( MSG_SUFFIX_SHORT_SIZE_GB );
	//short_size_suffixes += far_get_msg( MSG_SUFFIX_SHORT_SIZE_TB );
	}

std::string format_data_size( unsigned __int64 value, const std::vector<std::string>& suffixes ) {
	unsigned f = 0;
	unsigned __int64 div = 1;
	while ( ( value / div >= 1000 ) && ( f < 4 ) ) {
		f++;
		div *= 1024;
		}
	unsigned __int64 v1 = value / div;

	unsigned __int64 mul;
	if ( v1 < 10 ) {
		mul = 100;
		}
	else if ( v1 < 100 ) {
		mul = 10;
		}
	else {
		mul = 1;
		}

	unsigned __int64 v2 = value % div;
	unsigned __int64 d = v2 * mul * 10 / div % 10;
	v2 = v2 * mul / div;
	if ( d >= 5 ) {
		if ( v2 + 1 == mul ) {
			v2 = 0;
			if ( ( v1 == 999 ) && ( f < 4 ) ) {
				v1 = 0;
				v2 = 98;
				f += 1;
				}
			else {
				v1 += 1;
				}
			}
		else {
			v2 += 1;
			}
		}

	std::stringstream result;
	wchar_t buf[ 30 ];
	result <<  _ui64tow( v1, buf, 10 );
	if ( v2 != 0 ) {
		result << L'.';
		if ( ( v1 < 10 ) && ( v2 < 10 ) ) {
			result << L'0';
			}
		result << _ui64tow( v2, buf, 10 );
		}
	if ( suffixes[ f ].size( ) != 0 ) {
		result << L' ';
		result << suffixes[ f ];
		}
	return result.str();
	}

std::string format_time( u64 t /* ms */ ) {
	u64 ms = t % 1000;
	u64 s = ( t / 1000 ) % 60;
	u64 m = ( t / 1000 / 60 ) % 60;
	u64 h = t / 1000 / 60 / 60;
	std::stringstream ret;
	
	ret << h  << ":" <<  m  << ":" << s;
	return ret.str( );
	//return std::string::format( L"%02Lu:%02Lu:%02Lu", h, m, s );
	}

std::string format_time2( u64 t /* ms */ ) {
	u64 ms = t % 1000;
	u64 s = ( t / 1000 ) % 60;
	u64 m = ( t / 1000 / 60 ) % 60;
	u64 h = t / 1000 / 60 / 60;
	std::stringstream ret;
	if ( h != 0 ) {
		if ( m != 0 ) {
			ret << h << " h " << m << " m";
			return ret.str( );
			//return std::string::format( L"%Lu h %Lu m", h, m );
			}
		else {
			ret << h << " h";
			return ret.str( );
			//return std::string::format( L"%Lu h", h );
			}
		}
	else if ( m != 0 ) {
		if ( s != 0 ) {
			ret << m << " m " << s << " s";
			return ret.str( );
			//return std::string::format( L"%Lu m %Lu s", m, s );
			}
		else {
			ret << m << " m";
			return ret.str( );
			//return std::string::format( L"%Lu m", m );
			}
		}
	else if ( s != 0 ) {
		if ( ms != 0 ) {
			ret << s << "." << ms << " s";
			return ret.str( );
			//return std::string::format( L"%Lu.%03Lu s", s, ms );
			}
		else {
			ret << s << " s";
			return ret.str( );
			//return std::string::format( L"%Lu s", s );
			}
		}
	else {
		ret << ms << " ms";
		return ret.str( );
		//return std::string::format( L"%Lu ms", ms );
		}
	}

std::string format_hex_array( const std::array<u8, 4>& a ) {
	std::string str;
	for ( unsigned i = 0; i < a.size( ); i++ ) {
		str.add_fmt( L"%Bx", a[ i ] );
		}
	return str;
	}

// check if escape key is pressed
bool check_for_esc( void ) {
	bool res = false;
	HANDLE h_con = GetStdHandle( STD_INPUT_HANDLE );
	INPUT_RECORD rec;
	DWORD read_cnt;
	while ( true ) {
		PeekConsoleInput( h_con, &rec, 1, &read_cnt );
		if ( read_cnt == 0 ) {
			break;
			}
		ReadConsoleInput( h_con, &rec, 1, &read_cnt );
		if ( ( rec.EventType == KEY_EVENT ) &&
		  ( rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE ) &&
		  rec.Event.KeyEvent.bKeyDown &&
		  ( ( rec.Event.KeyEvent.dwControlKeyState &
		  ( LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED | RIGHT_CTRL_PRESSED | SHIFT_PRESSED ) ) == 0 )
		) {
			res = true;
			}
		}
	return res;
	}

std::string word_wrap( const std::string& message, unsigned wrap_bound ) {
	std::string msg = message;
	char newl = '\n';
	unsigned limit = wrap_bound;
	unsigned idx = -1;
	for ( unsigned i = 0; i < msg.size( ); i++ ) {
		if ( i >= limit ) {
			if ( idx != -1 ) {
				msg.insert( idx, &newl );
				i = idx + 1;
				limit = idx + 2 + wrap_bound;
				idx = -1;
				continue;
				}
			}
		if ( msg[ i ] == ' ' ) {
			idx = i;
			}
		}
	return msg;
	}

ModuleVersion get_module_version( HINSTANCE module ) {
	ModuleVersion version;
	memset( &version, 0, sizeof( version ) );
	LPWSTR file_name;
	unsigned size = GetModuleFileNameW( module, file_name, MAX_PATH );
	if ( size != 0 ) {
		//file_name.set_size( size );
		DWORD handle;
		size = GetFileVersionInfoSizeW( file_name, &handle );
		if ( size != 0 ) {
			//Array<char> ver_data;
			LPVOID ver_data;
			//const char* str_data = file_name.c_str( );
			if ( GetFileVersionInfoW( file_name, handle, size, ver_data ) != 0 ) {
				VS_FIXEDFILEINFO* ver;
				if ( VerQueryValueW( ver_data, L"\\", ( LPVOID* ) &ver, &size ) != 0 ) {
					version.major = HIWORD( ver->dwProductVersionMS );
					version.minor = LOWORD( ver->dwProductVersionMS );
					version.patch = HIWORD( ver->dwProductVersionLS );
					version.revision = LOWORD( ver->dwProductVersionLS );
					}
				}
			}
		}
	return version;
	}

// find path root component (drive letter / volume name / server share) and calculate its length
void locate_path_root( const std::string& path, unsigned& path_root_len, bool& is_unc_path ) {
	unsigned prefix_len = 0;
	is_unc_path = false;
	if ( path.equal( 0, L"\\\\?\\UNC\\" ) ) {
		prefix_len = 8;
		is_unc_path = true;
		}
	else if ( path.equal( 0, L"\\\\?\\" ) || path.equal( 0, L"\\??\\" ) || path.equal( 0, L"\\\\.\\" ) ) {
		prefix_len = 4;
		}
	else if ( path.equal( 0, L"\\\\" ) ) {
		prefix_len = 2;
		is_unc_path = true;
		}
	if ( ( prefix_len == 0 ) && !path.equal( 1, L':' ) ) {
		path_root_len = 0;
		}
	else {
		unsigned p = path.search( prefix_len, L'\\' );
		if ( p == -1 ) {
			p = path.size( );
			}
		if ( is_unc_path ) {
			p = path.search( p + 1, L'\\' );
			if ( p == -1 ) {
				p = path.size( );
				}
			}
		path_root_len = p;
		}
	}

std::string extract_path_root( const std::string& path ) {
	unsigned path_root_len;
	bool is_unc_path;
	locate_path_root( path, path_root_len, is_unc_path );
	return path.left( path_root_len );
	}

std::string extract_file_name( const std::string& path ) {
	unsigned pos = path.find_last_of( '\\' );
	if ( pos == -1 ) {
		pos = 0;
		}
	else {
		pos++;
		}
	unsigned path_root_len;
	bool is_unc_path;
	locate_path_root( path, path_root_len, is_unc_path );
	if ( pos < path_root_len ) {
		return std::string( );
		}
	
	return path.substr( pos, path.length()-pos );
	}

std::string extract_file_path( const std::string& path ) {
	unsigned pos = path.find_last_of( '\\' );
	if ( pos == -1 ) {
		pos = 0;
		}
	unsigned path_root_len;
	bool is_unc_path;
	locate_path_root( path, path_root_len, is_unc_path );
	if ( pos < path_root_len ) {
		return path.left( path_root_len );
		}
	return path.left( pos );
	}

std::string remove_path_root( const std::string& path ) {
	unsigned pos;
	bool is_unc_path;
	locate_path_root( path, pos, is_unc_path );
	if ( ( pos < path.size( ) ) && ( path[ pos ] == L'\\' ) ) {
		pos++;
		}
	return path.substr( pos, path.length()-pos );
	}

bool is_root_path( const std::string& path ) {
	unsigned path_root_len;
	bool is_unc_path;
	locate_path_root( path, path_root_len, is_unc_path );
	return ( path.size( ) == path_root_len ) || ( ( path.size( ) == path_root_len + 1 ) && ( path.last( ) == L'\\' ) );
	}

bool is_unc_path( const std::string& path ) {
	unsigned path_root_len;
	bool is_unc_path;
	locate_path_root( path, path_root_len, is_unc_path );
	return is_unc_path;
	}

std::string long_path( const std::string& path ) {
	if ( path.equal( 0, L"\\\\?\\" ) || path.equal( 0, L"\\\\.\\" ) ) {
		return path;
		}
	if ( path.equal( 0, L"\\??\\" ) ) {
		return std::string( path ).replace( 0, 4, L"\\\\?\\" );
		}
	if ( path.equal( 0, L"\\\\" ) ) {
		return std::string( path ).replace( 0, 1, L"\\\\?\\UNC" );
		}
	return L"\\\\?\\" + path;
	}

std::string add_trailing_slash( const std::string& file_path ) {
	if ( ( file_path.size( ) == 0 ) || ( file_path.at( ( ( file_path.length( ) > 0 ) ? file_path.length( ) - 1 : 0 ) ) == L'\\' ) ) {
		return file_path;
		}
	else {
		return file_path + L'\\';
		}
	}

std::string del_trailing_slash( const std::string& file_path ) {
	if ( ( file_path.size( ) < 2 ) || ( file_path.at( ( ( file_path.length( ) > 0 ) ? file_path.length( ) - 1 : 0 ) ) != L'\\' ) ) {
		return file_path;
		}
	else {
		return file_path.left( file_path.size( ) - 1 );
		}
	}

std::wstring add_trailing_slash( const std::wstring& path ) {
	if ( ( path.size( ) == 0 ) || ( path[ path.size( ) - 1 ] == L'\\' ) ) {
		return path;
		}
	else {
		return path + L'\\';
		}
	}

std::wstring del_trailing_slash( const std::wstring& path ) {
	if ( ( path.size( ) < 2 ) || ( path[ path.size( ) - 1 ] != L'\\' ) ) {
		return path;
		}
	else {
		return path.substr( 0, path.size( ) - 1 );
		}
	}

int str_to_int( const std::string& str ) {
	return _wtoi( str.data( ) );
	}

std::string int_to_str( int val ) {
	wchar_t str[ 64 ];
	return _itow( val, str, 10 );
	}

std::string center( const std::string& str, unsigned width ) {
	if ( str.size( ) >= width ) {
		return str;
		}
	unsigned lpad = ( width - str.size( ) ) / 2;
	unsigned rpad = width - str.size( ) - lpad;
	return std::string::format( L"%.*c%S%.*c", lpad, ' ', &str, rpad, ' ' );
	}

std::string fit_str( const std::string& path, unsigned size ) {
	if ( path.size( ) <= size ) {
		return path;
		}
	size -= 3; // place for ...
	unsigned ls = size / 2; // left part size
	unsigned rs = size - ls; // right part size
	return path.left( ls ) + L"..." + path.right( rs );
	}

void unquote( std::string& str ) {
	if ( ( str.size( ) >= 2 ) && ( str[ 0 ] == '"' ) && ( str.last( ) == '"' ) ) {
		str.remove( 0 );
		str.remove( str.size( ) - 1 );
		}
	}

std::vector<std::string> split_str( const std::string& str, wchar_t split_ch ) {
	std::vector<std::string> list;
	unsigned pos = 0;
	while ( pos < str.size( ) ) {
		unsigned pos2 = str.search( pos, split_ch );
		if ( pos2 == -1 ) {
			pos2 = str.size( );
			}
		list += str.slice( pos, pos2 - pos );
		pos = pos2 + 1;
		}
	return list;
	}

int round( double d ) {
	double a = fabs( d );
	int res = static_cast< int >( a );
	double frac = a - res;
	if ( frac >= 0.5 ) {
		res++;
		}
	if ( d >= 0 ) {
		return res;
		}
	else {
		return -res;
		}
	}

ProgressMonitor::ProgressMonitor( bool lazy ) : h_scr( NULL ) {
	QueryPerformanceCounter( ( PLARGE_INTEGER ) &t_curr );
	t_start = t_curr;
	QueryPerformanceFrequency( ( PLARGE_INTEGER ) &t_freq );
	if ( lazy ) {
		t_next = t_curr + t_freq / 2;
		}
	else {
		t_next = t_curr;
		}
	}

//ProgressMonitor::~ProgressMonitor( ) {
//	if ( h_scr != NULL ) {
//		g_far.RestoreScreen( h_scr );
//		SetConsoleTitleW( con_title.data( ) );
//		far_set_progress_state( TBPF_NOPROGRESS );
//		}
//	}

//void ProgressMonitor::update_ui( bool force ) {
//	QueryPerformanceCounter( ( PLARGE_INTEGER ) &t_curr );
//	if ( ( t_curr >= t_next ) || force ) {
//		if ( h_scr == NULL ) {
//			g_far.Text( 0, 0, 0, NULL ); // flush buffer hack
//			h_scr = g_far.SaveScreen( 0, 0, -1, -1 );
//			unsigned c_max_size = 10000;
//			con_title.set_size( GetConsoleTitleW( con_title.buf( c_max_size ), c_max_size ) ).compact( );
//			}
//		HANDLE h_con = GetStdHandle( STD_INPUT_HANDLE );
//		INPUT_RECORD rec;
//		DWORD read_cnt;
//		while ( true ) {
//			PeekConsoleInput( h_con, &rec, 1, &read_cnt );
//			if ( read_cnt == 0 ) {
//				break;
//				}
//			ReadConsoleInput( h_con, &rec, 1, &read_cnt );
//			if ( ( rec.EventType == KEY_EVENT ) && ( rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE ) && rec.Event.KeyEvent.bKeyDown && ( ( rec.Event.KeyEvent.dwControlKeyState & ( LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED | RIGHT_CTRL_PRESSED | SHIFT_PRESSED ) ) == 0 ) ) {
//				BREAK;
//				}
//			}
//		t_next = t_curr + t_freq / 2;
//		do_update_ui( );
//		}
//	}

//void Log::show( ) {
//	try {
//		// create file name for temporary file
//		std::string temp_file_path;
//		CHECK_SYS( GetTempFileNameW( get_temp_path( ).data( ), L"log", 0, temp_file_path.buf( MAX_PATH ) ) );
//		temp_file_path.set_size( );
//
//		// schedule file deletion
//		CLEAN( std::string, temp_file_path, DeleteFileW( temp_file_path.data( ) ) );
//			{
//			// open file for writing
//			HANDLE h_file = CreateFileW( temp_file_path.data( ), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
//			CHECK_SYS( h_file != INVALID_HANDLE_VALUE );
//			CLEAN( HANDLE, h_file, CHECK_SYS( CloseHandle( h_file ) ) );
//
//			// write unicode signature
//			const wchar_t sig = 0xFEFF;
//			DWORD size_written;
//			CHECK_SYS( WriteFile( h_file, &sig, sizeof( sig ), &size_written, NULL ) );
//			// write log data
//			std::string line;
//			for ( unsigned i = 0; i < size( ); i++ ) {
//				line.copy_fmt( L"'%S': %S\n", &citem( i ).object, &citem( i ).message );
//				CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ) * sizeof( wchar_t ), &size_written, NULL ) );
//				}
//			}
//
//		// open temporary file in Far viewer
//		far_viewer( temp_file_path, far_get_msg( MSG_LOG_TITLE ) );
//		}
//	catch ( ... ) {
//		}
//	}

//int far_control_int( HANDLE h_panel, FILE_CONTROL_COMMANDS command, int param ) {
//	return g_far.PanelControl( h_panel, command, param, 0 );
//	}
//
//int far_control_ptr( HANDLE h_panel, FILE_CONTROL_COMMANDS command, const void* param ) {
//	return g_far.PanelControl( h_panel, command, 0, const_cast< void* >( param ) );
//	}
//
//std::string far_get_panel_dir( HANDLE h_panel ) {
//	unsigned buf_size = 512;
//	std::unique_ptr<unsigned char> buf( new unsigned char[ buf_size ] );
//	reinterpret_cast< FarPanelDirectory* >( buf.get( ) )->StructSize = sizeof( FarPanelDirectory );
//	unsigned size = g_far.PanelControl( h_panel, FCTL_GETPANELDIRECTORY, buf_size, buf.get( ) );
//	if ( size > buf_size ) {
//		buf_size = size;
//		buf.reset( new unsigned char[ buf_size ] );
//		reinterpret_cast< FarPanelDirectory* >( buf.get( ) )->StructSize = sizeof( FarPanelDirectory );
//		size = g_far.PanelControl( h_panel, FCTL_GETPANELDIRECTORY, buf_size, buf.get( ) );
//		}
//	CHECK( size >= sizeof( FarPanelDirectory ) && size <= buf_size );
//	return reinterpret_cast< FarPanelDirectory* >( buf.get( ) )->Name;
//	}
//
//bool far_set_panel_dir( HANDLE h_panel, const std::string& dir ) {
//	FarPanelDirectory fpd;
//	memzero( fpd );
//	fpd.StructSize = sizeof( FarPanelDirectory );
//	fpd.Name = dir.data( );
//	return g_far.PanelControl( h_panel, FCTL_SETPANELDIRECTORY, 0, &fpd ) != 0;
//	}
//
//std::string far_get_full_path( const std::string& file_name ) {
//	std::string full_file_name;
//	const unsigned c_buf_size = 0x10000;
//	int size = g_fsf.ConvertPath( CPM_FULL, file_name.data( ), full_file_name.buf( c_buf_size ), c_buf_size );
//	if ( size > c_buf_size ) {
//		g_fsf.ConvertPath( CPM_FULL, file_name.data( ), full_file_name.buf( size ), size );
//		}
//	full_file_name.set_size( );
//	return full_file_name;
//	}

//PluginPanelItem* far_get_panel_item( HANDLE h_panel, size_t index ) {
//	static Array<unsigned char> ppi;
//	unsigned size = g_far.PanelControl( h_panel, FCTL_GETPANELITEM, index, nullptr );
//	FarGetPluginPanelItem gpi = { sizeof( FarGetPluginPanelItem ) };
//	gpi.Size = size;
//	gpi.Item = reinterpret_cast< PluginPanelItem* >( ppi.buf( size ) );
//	g_far.PanelControl( h_panel, FCTL_GETPANELITEM, index, &gpi );
//	ppi.set_size( size );
//	return reinterpret_cast< PluginPanelItem* >( ppi.buf( ) );
//	}
//
//PluginPanelItem* far_get_selected_panel_item( HANDLE h_panel, size_t index ) {
//	static Array<unsigned char> ppi;
//	unsigned size = g_far.PanelControl( h_panel, FCTL_GETSELECTEDPANELITEM, index, nullptr );
//	FarGetPluginPanelItem gpi = { sizeof( FarGetPluginPanelItem ) };
//	gpi.Size = size;
//	gpi.Item = reinterpret_cast< PluginPanelItem* >( ppi.buf( size ) );
//	g_far.PanelControl( h_panel, FCTL_GETSELECTEDPANELITEM, index, &gpi );
//	ppi.set_size( size );
//	return reinterpret_cast< PluginPanelItem* >( ppi.buf( ) );
//	}
//
//void far_set_progress_state( TBPFLAG state ) {
//	g_far.AdvControl( &c_plugin_guid, ACTL_SETPROGRESSSTATE, state, nullptr );
//	}
//
//void far_set_progress_value( unsigned __int64 completed, unsigned __int64 total ) {
//	ProgressValue pv = { sizeof( ProgressValue ) };
//	pv.Completed = completed;
//	pv.Total = total;
//	g_far.AdvControl( &c_plugin_guid, ACTL_SETPROGRESSVALUE, 0, &pv );
//	}

#ifdef _M_X64
#  define PLUGIN_TYPE L"uni x64"
#else
#  define PLUGIN_TYPE L"uni"
#endif

ModuleVersion g_version;

void error_dlg( const Error& e ) {
	std::string msg;
	//msg.add( far_get_msg( MSG_PLUGIN_NAME ) ).add( '\n' );
	std::string err_msg = word_wrap( e.message( ), get_msg_width( ) );
	if ( err_msg.size( ) != 0 ) {
		msg.add( err_msg ).add( '\n' );
		}
	msg.add_fmt( L"%S:%u v.%u.%u.%u.%u "PLUGIN_TYPE, &extract_file_name( oem_to_unicode( e.file ) ), e.line, g_version.major, g_version.minor, g_version.patch, g_version.revision );
	far_message( c_error_dialog_guid, msg, 0, FMSG_WARNING | FMSG_MB_OK );
	}

void error_dlg( const std::exception& e ) {
	std::string msg;
	//msg.add( far_get_msg( MSG_PLUGIN_NAME ) ).add( '\n' );
	std::string err_msg = word_wrap( oem_to_unicode( e.what( ) ), get_msg_width( ) );
	if ( err_msg.size( ) != 0 ) {
		msg.add( err_msg ).add( '\n' );
		}
	msg.add_fmt( L"v.%u.%u.%u.%u "PLUGIN_TYPE, g_version.major, g_version.minor, g_version.patch, g_version.revision );
	far_message( c_error_dialog_guid, msg, 0, FMSG_WARNING | FMSG_MB_OK );
	}

// get system directory for temporary files
std::string get_temp_path( ) {
	std::string temp_path;
	DWORD temp_path_size = MAX_PATH;
	DWORD len = GetTempPathW( temp_path_size, temp_path.buf( temp_path_size ) );
	if ( len > temp_path_size ) {
		temp_path_size = len;
		len = GetTempPathW( temp_path_size, temp_path.buf( temp_path_size ) );
		}
	CHECK_SYS( len != 0 );
	temp_path.set_size( len );
	return temp_path;
	}

Event::Event( bool manual_reset, bool initial_state ) {
	h_event = CreateEvent( NULL, manual_reset, initial_state, NULL );
	CHECK_SYS( h_event );
	}

Event::~Event( ) {
	CloseHandle( h_event );
	}

Semaphore::Semaphore( unsigned init_cnt, unsigned max_cnt ) {
	h_sem = CreateSemaphore( NULL, init_cnt, max_cnt, NULL );
	CHECK_SYS( h_sem );
	}

Semaphore::~Semaphore( ) {
	CloseHandle( h_sem );
	}

File::File( const std::string& file_path, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes ) {
	h_file = CreateFileW( long_path( file_path ).data( ), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL );
	CHECK_SYS( h_file != INVALID_HANDLE_VALUE );
	}

File::~File( ) {
	CloseHandle( h_file );
	}

unsigned __int64 File::pos( ) {
	LARGE_INTEGER p;
	p.QuadPart = 0;
	LARGE_INTEGER file_pos;
	CHECK_SYS( SetFilePointerEx( h_file, p, &file_pos, FILE_CURRENT ) );
	return file_pos.QuadPart;
	}

unsigned __int64 File::size( ) {
	LARGE_INTEGER file_size;
	CHECK_SYS( GetFileSizeEx( h_file, &file_size ) );
	return file_size.QuadPart;
	}

unsigned File::read( void* data, unsigned size ) {
	DWORD size_read;
	CHECK_SYS( ReadFile( h_file, data, size, &size_read, NULL ) );
	return size_read;
	}

FileEnum::FileEnum( const std::string& dir_path ) : dir_path( dir_path ), h_find( INVALID_HANDLE_VALUE ) { }

FileEnum::~FileEnum( ) {
	if ( h_find != INVALID_HANDLE_VALUE ) {
		FindClose( h_find );
		}
	}

bool FileEnum::next( ) {
	while ( true ) {
		if ( h_find == INVALID_HANDLE_VALUE ) {
			h_find = FindFirstFileW( long_path( add_trailing_slash( dir_path ) + L'*' ).data( ), &find_data );
			if ( h_find == INVALID_HANDLE_VALUE ) {
				if ( GetLastError( ) == ERROR_NO_MORE_FILES ) {
					return false;
					}
				CHECK_SYS( false );
				}
			}
		else {
			if ( !FindNextFileW( h_find, &find_data ) ) {
				if ( GetLastError( ) == ERROR_NO_MORE_FILES ) {
					return false;
					}
				CHECK_SYS( false );
				}
			}
		if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( ( find_data.cFileName[ 0 ] == L'.' ) && ( ( find_data.cFileName[ 1 ] == 0 ) || ( ( find_data.cFileName[ 1 ] == L'.' ) && ( find_data.cFileName[ 2 ] == 0 ) ) ) ) {
				continue;
				}
			}
		return true;
		}
	}

FindData get_find_data( const std::string& path ) {
	FindData find_data;
	HANDLE h_find = FindFirstFileW( long_path( path ).data( ), &find_data );
	CHECK_SYS( h_find != INVALID_HANDLE_VALUE );
	FindClose( h_find );
	return find_data;
	}

FILETIME time_t_to_FILETIME( time_t t ) {
	FILETIME ft;
	LONGLONG ll = Int32x32To64( t, 10000000 ) + 116444736000000000;
	ft.dwLowDateTime = static_cast< DWORD >( ll );
	ft.dwHighDateTime = ll >> 32;
	return ft;
	}

void enable_lfh( ) {
	typedef BOOL( WINAPI *PHeapSetInformation )( HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength );
	PHeapSetInformation FHeapSetInformation = reinterpret_cast< PHeapSetInformation >( GetProcAddress( GetModuleHandle( "kernel32" ), "HeapSetInformation" ) );
	if ( FHeapSetInformation ) {
		ULONG heap_info = 2;
		FHeapSetInformation( reinterpret_cast< HANDLE >( _get_heap_handle( ) ), HeapCompatibilityInformation, &heap_info, sizeof( heap_info ) );
		}
	}