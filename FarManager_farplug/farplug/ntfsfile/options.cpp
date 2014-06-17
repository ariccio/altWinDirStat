#define _ERROR_WINDOWS
#include "error.h"

#include "guids.h"
#include "utils.h"
#include "options.h"

extern struct PluginStartupInfo g_far;

bool g_use_standard_inf_units;
ContentOptions g_content_options;

class Options {
	private:
	HANDLE handle;

	private:
	//INT_PTR control( FAR_SETTINGS_CONTROL_COMMANDS command, void* param = nullptr ) {
	//	return g_far.SettingsControl( handle, command, 0, param );
	//	}

	//void clean( ) {
	//	if ( handle != INVALID_HANDLE_VALUE ) {
	//		control( SCTL_FREE );
	//		handle = INVALID_HANDLE_VALUE;
	//		}
	//	}

	//protected:
	//bool set( const wchar_t* name, unsigned __int64 value ) {
	//	FarSettingsItem fsi = { sizeof( FarSettingsItem ) };
	//	fsi.Root = 0;
	//	fsi.Name = name;
	//	fsi.Type = FST_QWORD;
	//	fsi.Number = value;
	//	return control( SCTL_SET, &fsi ) != 0;
	//	}

	//bool set( const wchar_t* name, const std::string& value ) {
	//	FarSettingsItem fsi = { sizeof( FarSettingsItem ) };
	//	fsi.Root = 0;
	//	fsi.Name = name;
	//	fsi.Type = FST_STRING;
	//	fsi.String = value.data( );
	//	return control( SCTL_SET, &fsi ) != 0;
	//	}

	//bool get( const wchar_t* name, unsigned __int64& value ) {
	//	FarSettingsItem fsi = { sizeof( FarSettingsItem ) };
	//	fsi.Root = 0;
	//	fsi.Name = name;
	//	fsi.Type = FST_QWORD;
	//	if ( !control( SCTL_GET, &fsi ) ) {
	//		return false;
	//		}
	//	value = fsi.Number;
	//	return true;
	//	}

	//bool get( const wchar_t* name, std::string& value ) {
	//	FarSettingsItem fsi = { sizeof( FarSettingsItem ) };
	//	fsi.Root = 0;
	//	fsi.Name = name;
	//	fsi.Type = FST_STRING;
	//	if ( !control( SCTL_GET, &fsi ) ) {
	//		return false;
	//		}
	//	value = fsi.String;
	//	return true;
	//	}

	//bool del( const wchar_t* name ) {
	//	FarSettingsValue fsv = { sizeof( FarSettingsValue ) };
	//	fsv.Root = 0;
	//	fsv.Value = name;
	//	return control( SCTL_DELETE, &fsv ) != 0;
	//	}

	public:
	Options( ) : handle( INVALID_HANDLE_VALUE ) { }

	~Options( ) {
		clean( );
		}

	bool create( ) {
		clean( );
		//FarSettingsCreate fsc = { sizeof( FarSettingsCreate ) };
		//fsc.Guid = c_plugin_guid;
		//if ( !control( SCTL_CREATE, &fsc ) ) {
			//return false;
			//}
		//handle = fsc.Handle;
		return true;
		}

	template<class Integer>
	Integer get_int( const wchar_t* name, Integer def_value ) {
		unsigned __int64 value;
		if ( get( name, value ) ) {
			return static_cast< Integer >( value );
			}
		else {
			return def_value;
			}
		}

	//bool get_bool( const wchar_t* name, bool def_value ) {
	//	unsigned __int64 value;
	//	if ( get( name, value ) ) {
	//		return value != 0;
	//		}
	//	else {
	//		return def_value;
	//		}
	//	}

	//std::string get_str( const wchar_t* name, const std::string& def_value ) {
	//	std::string value;
	//	if ( get( name, value ) ) {
	//		return value;
	//		}
	//	else {
	//		return def_value;
	//		}
	//	}

	//void set_int( const wchar_t* name, unsigned value, unsigned def_value ) {
	//	if ( value == def_value ) {
	//		del( name );
	//		}
	//	else {
	//		set( name, value );
	//		}
	//	}

	//void set_bool( const wchar_t* name, bool value, bool def_value ) {
	//	if ( value == def_value ) {
	//		del( name );
	//		}
	//	else {
	//		set( name, value ? 1 : 0 );
	//		}
	//	}

	//void set_str( const wchar_t* name, const std::string& value, const std::string& def_value ) {
	//	if ( value == def_value ) {
	//		del( name );
	//		}
	//	else {
	//		set( name, value );
	//		}
	//	}
	};

ContentOptions::ContentOptions( ) :
compression( false ),
  crc32( false ),
  md5( false ),
  sha1( false ),
  sha256( false ),
  ed2k( false ),
  crc16( false ) { }

FilePanelMode::FilePanelMode( ) :
//col_types( L"N,DSZ,RSZ,FRG,STM,LNK,MFT" ),
  //col_widths( L"0,7,7,5,3,3,3" ),
  //status_col_types( L"NR,A,D,T" ),
  //status_col_widths( L"0,10,0,0" ),
  wide( false ),
  //sort_mode( SM_UNSORTED ),
  reverse_sort( 0 ),
  numeric_sort( 0 ),
  sort_dirs_first( 0 ),
  custom_sort_mode( 3 ),
  show_streams( true ),
  show_main_stream( false ),
  use_highlighting( true ),
  use_usn_journal( true ),
  use_existing_usn_journal( false ),
  delete_usn_journal( true ),
  delete_own_usn_journal( true ),
  use_cache( false ),
  default_mft_mode( true ),
  backward_mft_scan( true ),
  flat_mode_auto_off( true )
  //cache_dir( L"%TEMP%" )
	{ }

CompressFilesParams::CompressFilesParams( ) :
min_file_size( 10 ),
  max_compression_ratio( 80 ),
  min_file_age( 30 ),
  defragment_after_compression( true ) { }

//void load_plugin_options( ) {
//	Options options;
//	if ( !options.create( ) ) {
//		return;
//		}
//	g_use_standard_inf_units = options.get_bool( L"StandardInformationUnits", false );
//	ContentOptions def_content_options;
//	g_content_options.compression = options.get_bool( L"ContentOptionsCompression", def_content_options.compression );
//	g_content_options.crc32 = options.get_bool( L"ContentOptionsCRC32", def_content_options.crc32 );
//	g_content_options.md5 = options.get_bool( L"ContentOptionsMD5", def_content_options.md5 );
//	g_content_options.sha1 = options.get_bool( L"ContentOptionsSHA1", def_content_options.sha1 );
//	g_content_options.sha256 = options.get_bool( L"ContentOptionsSHA256", def_content_options.sha256 );
//	g_content_options.ed2k = options.get_bool( L"ContentOptionsED2K", def_content_options.ed2k );
//	g_content_options.crc16 = options.get_bool( L"ContentOptionsCRC16", def_content_options.crc16 );
//	FilePanelMode def_file_panel_mode;
//	g_file_panel_mode.col_types = options.get_str( L"FilePanelColTypes", def_file_panel_mode.col_types );
//	g_file_panel_mode.col_widths = options.get_str( L"FilePanelColWidths", def_file_panel_mode.col_widths );
//	g_file_panel_mode.status_col_types = options.get_str( L"FilePanelStatusColTypes", def_file_panel_mode.status_col_types );
//	g_file_panel_mode.status_col_widths = options.get_str( L"FilePanelStatusColWidths", def_file_panel_mode.status_col_widths );
//	g_file_panel_mode.wide = options.get_bool( L"FilePanelWide", def_file_panel_mode.wide );
//	g_file_panel_mode.sort_mode = options.get_int( L"FilePanelSortMode", def_file_panel_mode.sort_mode );
//	g_file_panel_mode.reverse_sort = options.get_int( L"FilePanelReverseSort", def_file_panel_mode.reverse_sort );
//	g_file_panel_mode.numeric_sort = options.get_int( L"FilePanelNumericSort", def_file_panel_mode.numeric_sort );
//	g_file_panel_mode.sort_dirs_first = options.get_int( L"FilePanelSortDirsFirst", def_file_panel_mode.sort_dirs_first );
//	g_file_panel_mode.custom_sort_mode = options.get_int( L"CustomSortMode", def_file_panel_mode.custom_sort_mode );
//	g_file_panel_mode.show_streams = options.get_bool( L"FilePanelShowStreams", def_file_panel_mode.show_streams );
//	g_file_panel_mode.show_main_stream = options.get_bool( L"FilePanelShowMainStream", def_file_panel_mode.show_main_stream );
//	g_file_panel_mode.use_highlighting = options.get_bool( L"FilePanelUseHighlighting", def_file_panel_mode.use_highlighting );
//	g_file_panel_mode.use_usn_journal = options.get_bool( L"FilePanelUseUsnJournal", def_file_panel_mode.use_usn_journal );
//	g_file_panel_mode.use_existing_usn_journal = options.get_bool( L"FilePanelUseExistingUsnJournal", def_file_panel_mode.use_existing_usn_journal );
//	g_file_panel_mode.delete_usn_journal = options.get_bool( L"FilePanelDeleteUsnJournal", def_file_panel_mode.delete_usn_journal );
//	g_file_panel_mode.delete_own_usn_journal = options.get_bool( L"FilePanelDeleteOwnUsnJournal", def_file_panel_mode.delete_own_usn_journal );
//	g_file_panel_mode.use_cache = options.get_bool( L"FilePanelUseCache", def_file_panel_mode.use_cache );
//	g_file_panel_mode.default_mft_mode = options.get_bool( L"FilePanelDefaultMftMode", def_file_panel_mode.default_mft_mode );
//	g_file_panel_mode.backward_mft_scan = options.get_bool( L"FilePanelBackwardMftScan", def_file_panel_mode.backward_mft_scan );
//	g_file_panel_mode.cache_dir = options.get_str( L"FilePanelCacheDir", def_file_panel_mode.cache_dir );
//	g_file_panel_mode.flat_mode_auto_off = options.get_bool( L"FilePanelFlatModeAutoOff", def_file_panel_mode.flat_mode_auto_off );
//	CompressFilesParams def_compress_files_params;
//	g_compress_files_params.min_file_size = options.get_int( L"CompressFilesMinFileSize", def_compress_files_params.min_file_size );
//	g_compress_files_params.max_compression_ratio = options.get_int( L"CompressFilesMaxCompressionRatio", def_compress_files_params.max_compression_ratio );
//	g_compress_files_params.min_file_age = options.get_int( L"CompressFilesMinFileAge", def_compress_files_params.min_file_age );
//	g_compress_files_params.defragment_after_compression = options.get_bool( L"CompressFilesDefragmentAfterCompression", def_compress_files_params.defragment_after_compression );
//	};
//
//void store_plugin_options( ) {
//	Options options;
//	if ( !options.create( ) ) {
//		return;
//		}
//	ContentOptions def_content_options;
//	options.set_bool( L"ContentOptionsCompression", g_content_options.compression, def_content_options.compression );
//	options.set_bool( L"ContentOptionsCRC32", g_content_options.crc32, def_content_options.crc32 );
//	options.set_bool( L"ContentOptionsMD5", g_content_options.md5, def_content_options.md5 );
//	options.set_bool( L"ContentOptionsSHA1", g_content_options.sha1, def_content_options.sha1 );
//	options.set_bool( L"ContentOptionsSHA256", g_content_options.sha256, def_content_options.sha256 );
//	options.set_bool( L"ContentOptionsED2K", g_content_options.ed2k, def_content_options.ed2k );
//	options.set_bool( L"ContentOptionsCRC16", g_content_options.crc16, def_content_options.crc16 );
//	FilePanelMode def_file_panel_mode;
//	options.set_str( L"FilePanelColTypes", g_file_panel_mode.col_types, def_file_panel_mode.col_types );
//	options.set_str( L"FilePanelColWidths", g_file_panel_mode.col_widths, def_file_panel_mode.col_widths );
//	options.set_str( L"FilePanelStatusColTypes", g_file_panel_mode.status_col_types, def_file_panel_mode.status_col_types );
//	options.set_str( L"FilePanelStatusColWidths", g_file_panel_mode.status_col_widths, def_file_panel_mode.status_col_widths );
//	options.set_bool( L"FilePanelWide", g_file_panel_mode.wide, def_file_panel_mode.wide );
//	options.set_int( L"FilePanelSortMode", g_file_panel_mode.sort_mode, def_file_panel_mode.sort_mode );
//	options.set_int( L"FilePanelReverseSort", g_file_panel_mode.reverse_sort, def_file_panel_mode.reverse_sort );
//	options.set_int( L"FilePanelNumericSort", g_file_panel_mode.numeric_sort, def_file_panel_mode.numeric_sort );
//	options.set_int( L"FilePanelSortDirsFirst", g_file_panel_mode.sort_dirs_first, def_file_panel_mode.sort_dirs_first );
//	options.set_int( L"CustomSortMode", g_file_panel_mode.custom_sort_mode, def_file_panel_mode.custom_sort_mode );
//	options.set_bool( L"FilePanelShowStreams", g_file_panel_mode.show_streams, def_file_panel_mode.show_streams );
//	options.set_bool( L"FilePanelShowMainStream", g_file_panel_mode.show_main_stream, def_file_panel_mode.show_main_stream );
//	options.set_bool( L"FilePanelUseHighlighting", g_file_panel_mode.use_highlighting, def_file_panel_mode.use_highlighting );
//	options.set_bool( L"FilePanelUseUsnJournal", g_file_panel_mode.use_usn_journal, def_file_panel_mode.use_usn_journal );
//	options.set_bool( L"FilePanelUseExistingUsnJournal", g_file_panel_mode.use_existing_usn_journal, def_file_panel_mode.use_existing_usn_journal );
//	options.set_bool( L"FilePanelDeleteUsnJournal", g_file_panel_mode.delete_usn_journal, def_file_panel_mode.delete_usn_journal );
//	options.set_bool( L"FilePanelDeleteOwnUsnJournal", g_file_panel_mode.delete_own_usn_journal, def_file_panel_mode.delete_own_usn_journal );
//	options.set_bool( L"FilePanelUseCache", g_file_panel_mode.use_cache, def_file_panel_mode.use_cache );
//	options.set_bool( L"FilePanelDefaultMftMode", g_file_panel_mode.default_mft_mode, def_file_panel_mode.default_mft_mode );
//	options.set_bool( L"FilePanelBackwardMftScan", g_file_panel_mode.backward_mft_scan, def_file_panel_mode.backward_mft_scan );
//	options.set_str( L"FilePanelCacheDir", g_file_panel_mode.cache_dir, def_file_panel_mode.cache_dir );
//	options.set_bool( L"FilePanelFlatModeAutoOff", g_file_panel_mode.flat_mode_auto_off, def_file_panel_mode.flat_mode_auto_off );
//	CompressFilesParams def_compress_files_params;
//	options.set_int( L"CompressFilesMinFileSize", g_compress_files_params.min_file_size, def_compress_files_params.min_file_size );
//	options.set_int( L"CompressFilesMaxCompressionRatio", g_compress_files_params.max_compression_ratio, def_compress_files_params.max_compression_ratio );
//	options.set_int( L"CompressFilesMinFileAge", g_compress_files_params.min_file_age, def_compress_files_params.min_file_age );
//	options.set_bool( L"CompressFilesDefragmentAfterCompression", g_compress_files_params.defragment_after_compression, def_compress_files_params.defragment_after_compression );
//	}