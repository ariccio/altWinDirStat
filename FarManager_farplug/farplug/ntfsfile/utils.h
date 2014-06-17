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

class NonCopyable {
	protected:
	NonCopyable( ) { }
	~NonCopyable( ) { }
	private:
	NonCopyable( const NonCopyable& );
	NonCopyable& operator=( const NonCopyable& );
	};

//void unicode_to_oem( AnsiString& oem_str, const std::string& u_str );
//AnsiString unicode_to_oem( const std::string& str );
//void oem_to_unicode( std::string& u_str, const AnsiString& oem_str );
//std::string oem_to_unicode( const AnsiString& str );
//std::string ansi_to_unicode( const AnsiString& a_str, unsigned code_page );
//AnsiString unicode_to_ansi( const std::string& u_str, unsigned code_page );
std::string format_inf_amount( u64 size );
std::string format_inf_amount_short( u64 size, bool speed = false );
std::string format_time( u64 t /* ms */ );
std::string format_time2( u64 t /* ms */ );
std::string format_hex_array( const Array<u8>& a );
bool check_for_esc( void );
std::string word_wrap( const std::string& message, unsigned wrap_bound );
struct ModuleVersion {
	unsigned major;
	unsigned minor;
	unsigned patch;
	unsigned revision;
	};
ModuleVersion get_module_version( HINSTANCE module );
std::string extract_path_root( const std::string& path );
std::string extract_file_name( const std::string& path );
std::string extract_file_path( const std::string& path );
std::string remove_path_root( const std::string& path );
bool is_root_path( const std::string& path );
bool is_unc_path( const std::string& path );

std::string long_path( const std::string& path );
std::string add_trailing_slash( const std::string& file_path );
std::string del_trailing_slash( const std::string& file_path );
std::wstring add_trailing_slash( const std::wstring& path );
std::wstring del_trailing_slash( const std::wstring& path );
int str_to_int( const std::string& str );
std::string int_to_str( int val );
std::string center( const std::string& str, unsigned width );
std::string fit_str( const std::string& path, unsigned size );
void unquote( std::string& str );
std::vector<std::string> split_str( const std::string& str, wchar_t split_ch );

extern std::vector<std::string> size_suffixes;
extern std::vector<std::string> speed_suffixes;
extern std::vector<std::string> short_size_suffixes;
void load_suffixes( );
std::string format_data_size( unsigned __int64 value, const std::vector<std::string>& suffixes );

int round( double d );

template<class T> const T* to_array( const vector<T>& v ) {
	if ( v.size( ) )
	  return &v[ 0 ];
	else
	  return NULL;
	}

class ProgressMonitor {
	private:
	HANDLE h_scr;
	std::string con_title;
	unsigned __int64 t_start;
	unsigned __int64 t_curr;
	unsigned __int64 t_next;
	unsigned __int64 t_freq;
	protected:
	virtual void do_update_ui( ) = 0;
	public:
	ProgressMonitor( bool lazy = true );
	virtual ~ProgressMonitor( );
	void update_ui( bool force = false );
	unsigned __int64 time_elapsed( ) const {
		return ( t_curr - t_start ) / ( t_freq / 1000 );
		}
	};

//int far_control_int( HANDLE h_panel, FILE_CONTROL_COMMANDS command, int param );
//int far_control_ptr( HANDLE h_panel, FILE_CONTROL_COMMANDS command, const void* param );
std::string far_get_panel_dir( HANDLE h_panel );
bool far_set_panel_dir( HANDLE h_panel, const std::string& dir );
std::string far_get_full_path( const std::string& file_name );
//PluginPanelItem* far_get_panel_item( HANDLE h_panel, size_t index );
//PluginPanelItem* far_get_selected_panel_item( HANDLE h_panel, size_t index );
//void far_set_progress_state( TBPFLAG state );
void far_set_progress_value( unsigned __int64 completed, unsigned __int64 total );

#define BEGIN_ERROR_HANDLER try {
#define END_ERROR_HANDLER(success, failure) \
	success; \
	  } \
  catch (Break&) { \
	failure; \
	  } \
  catch (Error& e) { \
	error_dlg(e); \
	failure; \
	  } \
  catch (std::exception& e) { \
	error_dlg(e); \
	failure; \
	  } \
  catch (...) { \
	far_message(c_error_dialog_guid, L"\nFailure!", 0, FMSG_WARNING | FMSG_MB_OK); \
	failure; \
	  }

extern ModuleVersion g_version;

void error_dlg( const Error& e );
void error_dlg( const std::exception& e );

std::string get_temp_path( );

class CriticalSection : private NonCopyable, private CRITICAL_SECTION {
	public:
	CriticalSection( ) {
		InitializeCriticalSection( this );
		}
	virtual ~CriticalSection( ) {
		DeleteCriticalSection( this );
		}
	friend class CriticalSectionLock;
	};

class CriticalSectionLock : private NonCopyable {
	private:
	CriticalSection& cs;
	public:
	CriticalSectionLock( CriticalSection& cs ) : cs( cs ) {
		EnterCriticalSection( &cs );
		}
	~CriticalSectionLock( ) {
		LeaveCriticalSection( &cs );
		}
	};

class Event : private NonCopyable {
	protected:
	HANDLE h_event;
	public:
	Event( bool manual_reset, bool initial_state );
	~Event( );
	HANDLE handle( ) const {
		return h_event;
		}
	};

class Semaphore : private NonCopyable {
	protected:
	HANDLE h_sem;
	public:
	Semaphore( unsigned init_cnt, unsigned max_cnt );
	~Semaphore( );
	HANDLE handle( ) const {
		return h_sem;
		}
	};

class File : private NonCopyable {
	protected:
	HANDLE h_file;
	public:
	File( const std::string& file_path, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes );
	File( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTransaction );
	~File( );
	HANDLE handle( ) const {
		return h_file;
		}
	unsigned __int64 pos( );
	unsigned __int64 size( );
	unsigned read( void* data, unsigned size );
	void write( const void* data, unsigned size );
	};

struct FindData : public WIN32_FIND_DATAW {
	bool is_dir( ) const {
		return ( dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
		}
	unsigned __int64 size( ) const {
		return ( static_cast< unsigned __int64 >( nFileSizeHigh ) << 32 ) | nFileSizeLow;
		}
	};

class FileEnum : private NonCopyable {
	protected:
	std::string dir_path;
	HANDLE h_find;
	FindData find_data;
	public:
	FileEnum( const std::string& dir_path );
	~FileEnum( );
	bool next( );
	const FindData& data( ) const {
		return find_data;
		}
	};

FindData get_find_data( const std::string& path );
FILETIME time_t_to_FILETIME( time_t t );
void enable_lfh( );

template<typename T> void memzero( T& v ) {
	memset( &v, 0, sizeof( T ) );
	}

#define NT_MAX_PATH 32768
