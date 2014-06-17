#define _ERROR_WINDOWS
#include "error.h"
#include <Windows.h>

#include "guids.h"
#include "utils.h"
#include "options.h"
#include "dlgapi.h"
#include "content.h"

#include "crc16.cpp"

extern struct PluginStartupInfo g_far;
//extern Array<FarColor> g_colors;

// LZO library exception
class LzoError : public Error {
	private:
	int code;
	public:
	LzoError( int code ) : code( code ) { }
	virtual std::string message( ) const {
		std::string message;
		//if ( code == LZO_E_OK ) {
		//	message = L"LZO_E_OK";
		//	}
		//else if ( code == LZO_E_ERROR ) {
		//	message = L"LZO_E_ERROR";
		//	}
		//else if ( code == LZO_E_OUT_OF_MEMORY ) {
		//	message = L"LZO_E_OUT_OF_MEMORY";
		//	}
		//else if ( code == LZO_E_NOT_COMPRESSIBLE ) {
		//	message = L"LZO_E_NOT_COMPRESSIBLE";
		//	}
		//else if ( code == LZO_E_INPUT_OVERRUN ) {
		//	message = L"LZO_E_INPUT_OVERRUN";
		//	}
		//else if ( code == LZO_E_OUTPUT_OVERRUN ) {
		//	message = L"LZO_E_OUTPUT_OVERRUN";
		//	}
		//else if ( code == LZO_E_LOOKBEHIND_OVERRUN ) {
		//	message = L"LZO_E_LOOKBEHIND_OVERRUN";
		//	}
		//else if ( code == LZO_E_EOF_NOT_FOUND ) {
		//	message = L"LZO_E_EOF_NOT_FOUND";
		//	}
		//else if ( code == LZO_E_INPUT_NOT_CONSUMED ) {
		//	message = L"LZO_E_INPUT_NOT_CONSUMED";
		//	}
		//else if ( code == LZO_E_NOT_YET_IMPLEMENTED ) {
		//	message = L"LZO_E_NOT_YET_IMPLEMENTED";
		//	}
		//else {
		//	message.copy_fmt( L"LZO code %i", code );
		//	}
		//return message;
		return message;
		}
	};

#define CHECK_LZO(code) { int __err = (code); if (__err != LZO_E_OK) FAIL(LzoError(__err)); }

struct OptionsDlgData {
	// control ids
	int compression_ctrl_id;
	int crc32_ctrl_id;
	int md5_ctrl_id;
	int sha1_ctrl_id;
	int sha256_ctrl_id;
	int ed2k_ctrl_id;
	int crc16_ctrl_id;
	int set_all_ctrl_id;
	int reset_all_ctrl_id;
	int ok_ctrl_id;
	int cancel_ctrl_id;
	};

//intptr_t WINAPI options_dlg_proc( HANDLE h_dlg, intptr_t msg, intptr_t param1, void* param2 ) {
//	BEGIN_ERROR_HANDLER;
//	FarDialog* dlg = FarDialog::get_dlg( h_dlg );
//	const OptionsDlgData* dlg_data = ( const OptionsDlgData* ) dlg->get_dlg_data( 0 );
//	ContentOptions* options = ( ContentOptions* ) dlg->get_dlg_data( 1 );
//	bool single_file = *( bool* ) dlg->get_dlg_data( 2 );
//	if ( single_file ) {
//		if ( msg == DN_BTNCLICK ) {
//			if ( param1 == dlg_data->set_all_ctrl_id ) {
//				dlg->set_check( dlg_data->compression_ctrl_id, true );
//				dlg->set_check( dlg_data->crc32_ctrl_id, true );
//				dlg->set_check( dlg_data->md5_ctrl_id, true );
//				dlg->set_check( dlg_data->sha1_ctrl_id, true );
//				dlg->set_check( dlg_data->sha256_ctrl_id, true );
//				dlg->set_check( dlg_data->ed2k_ctrl_id, true );
//				dlg->set_check( dlg_data->crc16_ctrl_id, true );
//				dlg->set_focus( dlg_data->ok_ctrl_id );
//				return TRUE;
//				}
//			else if ( param1 == dlg_data->reset_all_ctrl_id ) {
//				dlg->set_check( dlg_data->compression_ctrl_id, false );
//				dlg->set_check( dlg_data->crc32_ctrl_id, false );
//				dlg->set_check( dlg_data->md5_ctrl_id, false );
//				dlg->set_check( dlg_data->sha1_ctrl_id, false );
//				dlg->set_check( dlg_data->sha256_ctrl_id, false );
//				dlg->set_check( dlg_data->ed2k_ctrl_id, false );
//				dlg->set_check( dlg_data->crc16_ctrl_id, false );
//				dlg->set_focus( dlg_data->ok_ctrl_id );
//				return TRUE;
//				}
//			}
//		else if ( ( msg == DN_CLOSE ) && ( param1 >= 0 ) && ( param1 != dlg_data->cancel_ctrl_id ) ) {
//			// fill options structrure
//			options->compression = dlg->get_check( dlg_data->compression_ctrl_id );
//			options->crc32 = dlg->get_check( dlg_data->crc32_ctrl_id );
//			options->md5 = dlg->get_check( dlg_data->md5_ctrl_id );
//			options->sha1 = dlg->get_check( dlg_data->sha1_ctrl_id );
//			options->sha256 = dlg->get_check( dlg_data->sha256_ctrl_id );
//			options->ed2k = dlg->get_check( dlg_data->ed2k_ctrl_id );
//			options->crc16 = dlg->get_check( dlg_data->crc16_ctrl_id );
//			}
//		}
//	else {
//		if ( msg == DN_INITDIALOG ) {
//			dlg->set_check( dlg_data->compression_ctrl_id, true );
//			dlg->enable( dlg_data->compression_ctrl_id, false );
//			return FALSE;
//			}
//		}
//	END_ERROR_HANDLER( ; , ; );
//	return g_far.DefDlgProc( h_dlg, msg, param1, param2 );
//	}
//
//// show content analysis options dialog
//// returns false if user cancelled dialog
//// fills 'options' structure otherwise
//bool show_options_dialog( ContentOptions& options, bool single_file ) {
//	//FarDialog dlg( c_content_options_dialog_guid, far_get_msg( MSG_CONTENT_SETTINGS_TITLE ), 30 );
//	//OptionsDlgData dlg_data;
//	//dlg.add_dlg_data( &dlg_data );
//	//dlg.add_dlg_data( &options );
//	//dlg.add_dlg_data( &single_file );
//
//	////dlg_data.compression_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_COMPRESSION ), options.compression );
//	//dlg.new_line( );
//	//if ( single_file ) {
//	//	dlg_data.crc32_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_CRC32 ), options.crc32 );
//	//	dlg.new_line( );
//	//	dlg_data.md5_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_MD5 ), options.md5 );
//	//	dlg.new_line( );
//	//	dlg_data.sha1_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_SHA1 ), options.sha1 );
//	//	dlg.new_line( );
//	//	dlg_data.sha256_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_SHA256 ), options.sha256 );
//	//	dlg.new_line( );
//	//	dlg_data.ed2k_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_ED2K ), options.ed2k );
//	//	dlg.new_line( );
//	//	dlg_data.crc16_ctrl_id = dlg.check_box( far_get_msg( MSG_CONTENT_SETTINGS_CRC16 ), options.crc16 );
//	//	dlg.new_line( );
//
//	//	// Set & Reset All buttons
//	//	dlg_data.set_all_ctrl_id = dlg.button( far_get_msg( MSG_CONTENT_SETTINGS_SET_ALL ), DIF_CENTERGROUP | DIF_BTNNOCLOSE );
//	//	dlg_data.reset_all_ctrl_id = dlg.button( far_get_msg( MSG_CONTENT_SETTINGS_RESET_ALL ), DIF_CENTERGROUP | DIF_BTNNOCLOSE );
//	//	dlg.new_line( );
//	//	}
//
//	//dlg.separator( );
//	//dlg.new_line( );
//
//	//// Ok & Cancel buttons
//	//dlg_data.ok_ctrl_id = dlg.def_button( far_get_msg( MSG_BUTTON_OK ), DIF_CENTERGROUP );
//	//dlg_data.cancel_ctrl_id = dlg.button( far_get_msg( MSG_BUTTON_CANCEL ), DIF_CENTERGROUP );
//	//dlg.new_line( );
//
//	//int item = dlg.show( options_dlg_proc );
//
//	//return ( item != -1 ) && ( item != dlg_data.cancel_ctrl_id );
//	return false;
//	}

std::string ed2k_extract_hash_from_url( const std::string& url ) {
	const wchar_t* c_ed2k_url_start = L"ed2k://|file|";
	unsigned p = url.search( c_ed2k_url_start );
	if ( p != 0 ) {
		return std::string( );
		}
	p = url.search( p + ( unsigned ) wcslen( c_ed2k_url_start ), '|' ); // skip file name
	if ( p == -1 ) {
		return std::string( );
		}
	p = url.search( p + 1, '|' ); // skip file size
	if ( p == -1 ) {
		return std::string( );
		}
	p++;
	unsigned p_end = url.search( p, '|' );
	if ( p_end == -1 ) {
		return std::string( );
		}
	return std::string( url.data( ) + p, p_end - p );
	}

//void save_hashes_to_file( const std::string& file_name, const ContentOptions& options, const ContentInfo& info ) {
//	std::string hashes_file_name = file_name + L".hashes";
//	if ( GetFileAttributesW( hashes_file_name.data( ) ) != INVALID_FILE_ATTRIBUTES ) { // file already exists
//		//if ( far_message( c_file_exists_dialog_guid, far_get_msg( MSG_CONTENT_RESULT_TITLE ) + L"\n" + word_wrap( far_get_msg( MSG_CONTENT_RESULT_FILE_EXISTS ), get_msg_width( ) ) + L"\n" + far_get_msg( MSG_BUTTON_OK ) + L"\n" + far_get_msg( MSG_BUTTON_CANCEL ), 2 ) != 0 ) {
//		//	return;
//		//	}
//		}
//	HANDLE h_file = CreateFileW( hashes_file_name.data( ), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
//	CHECK_SYS( h_file != INVALID_HANDLE_VALUE );
//	CLEAN( HANDLE, h_file, CloseHandle( h_file ) );
//	DWORD bw;
//	if ( options.crc32 ) {
//		AnsiString line = "CRC32: " + unicode_to_oem( format_hex_array( info.crc32 ) ) + "\n";
//		CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ), &bw, NULL ) );
//		}
//	if ( options.md5 ) {
//		AnsiString line = "MD5: " + unicode_to_oem( format_hex_array( info.md5 ) ) + "\n";
//		CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ), &bw, NULL ) );
//		}
//	if ( options.sha1 ) {
//		AnsiString line = "SHA1: " + unicode_to_oem( format_hex_array( info.sha1 ) ) + "\n";
//		CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ), &bw, NULL ) );
//		}
//	if ( options.sha256 ) {
//		AnsiString line = "SHA256: " + unicode_to_oem( format_hex_array( info.sha256 ) ) + "\n";
//		CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ), &bw, NULL ) );
//		}
//	if ( options.ed2k ) {
//		AnsiString line = "ED2K: " + unicode_to_oem( format_hex_array( info.ed2k ) ) + "\n";
//		CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ), &bw, NULL ) );
//		}
//	if ( options.crc16 ) {
//		AnsiString line = "CRC16: " + unicode_to_oem( format_hex_array( info.crc16 ) ) + "\n";
//		CHECK_SYS( WriteFile( h_file, line.data( ), line.size( ), &bw, NULL ) );
//		}
//	//far_message( c_file_saved_dialog_guid, far_get_msg( MSG_CONTENT_RESULT_TITLE ) + L"\n" + word_wrap( far_get_msg( MSG_CONTENT_RESULT_FILE_SAVED ), get_msg_width( ) ) + L"\n" + far_get_msg( MSG_BUTTON_OK ), 1 );
//	}

struct ResultDlgData {
	int verify_edit_ctrl_id;
	int correct_result_label_ctrl_id;
	int wrong_result_label_ctrl_id;
	int save_file_ctrl_id;
	};

//intptr_t WINAPI result_dlg_proc( HANDLE h_dlg, intptr_t msg, intptr_t param1, void* param2 ) {
//	BEGIN_ERROR_HANDLER;
//	FarDialog* dlg = FarDialog::get_dlg( h_dlg );
//	const ResultDlgData* dlg_data = ( const ResultDlgData* ) dlg->get_dlg_data( 0 );
//	const std::string* file_name = ( const std::string* ) dlg->get_dlg_data( 1 );
//	const ContentOptions* options = ( const ContentOptions* ) dlg->get_dlg_data( 2 );
//	const ContentInfo* info = ( const ContentInfo* ) dlg->get_dlg_data( 3 );
//	if ( msg == DN_EDITCHANGE ) {
//		if ( param1 == dlg_data->verify_edit_ctrl_id ) {
//			std::string user_hash = dlg->get_text( dlg_data->verify_edit_ctrl_id );
//			user_hash.strip( );
//			bool result =
//			  ( ( options->crc32 ) && ( format_hex_array( info->crc32 ).icompare( user_hash ) == 0 ) ) ||
//			  ( ( options->md5 ) && ( format_hex_array( info->md5 ).icompare( user_hash ) == 0 ) ) ||
//			  ( ( options->sha1 ) && ( format_hex_array( info->sha1 ).icompare( user_hash ) == 0 ) ) ||
//			  ( ( options->sha256 ) && ( format_hex_array( info->sha256 ).icompare( user_hash ) == 0 ) ) ||
//			  ( ( options->ed2k ) && ( ( format_hex_array( info->ed2k ).icompare( user_hash ) == 0 ) ||
//			  ( format_hex_array( info->ed2k ).icompare( ed2k_extract_hash_from_url( user_hash ) ) == 0 ) ) ||
//			  ( ( options->crc16 ) && ( format_hex_array( info->crc16 ).icompare( user_hash ) == 0 ) ) );
//			dlg->set_visible( dlg_data->correct_result_label_ctrl_id, result && user_hash.size( ) );
//			dlg->set_visible( dlg_data->wrong_result_label_ctrl_id, !result && user_hash.size( ) );
//			}
//		}
//	else if ( msg == DN_BTNCLICK ) {
//		if ( param1 == dlg_data->save_file_ctrl_id ) {
//			save_hashes_to_file( *file_name, *options, *info );
//			dlg->set_focus( dlg_data->verify_edit_ctrl_id );
//			}
//		}
//	else if ( msg == DN_CTLCOLORDLGITEM ) {
//		FarDialogItemColors* item_colors = static_cast< FarDialogItemColors* >( param2 );
//		if ( param1 == dlg_data->correct_result_label_ctrl_id ) {
//			item_colors->Colors[ 0 ].ForegroundColor = FOREGROUND_GREEN;
//			item_colors->Colors[ 0 ].Flags |= FCF_FG_4BIT;
//			}
//		else if ( param1 == dlg_data->wrong_result_label_ctrl_id ) {
//			item_colors->Colors[ 0 ].ForegroundColor = FOREGROUND_RED;
//			item_colors->Colors[ 0 ].Flags |= FCF_FG_4BIT;
//			}
//		}
//
//	END_ERROR_HANDLER( ; , ; );
//	return g_far.DefDlgProc( h_dlg, msg, param1, param2 );
//	}

// show content analysis result dialog
void show_result_dialog( const std::string& file_name, const ContentOptions& options, const ContentInfo& info ) {
	//ResultDlgData dlg_data;
	//FarDialog dlg( c_content_result_dialog_guid, far_get_msg( MSG_CONTENT_RESULT_TITLE ), 30 );
	//dlg.add_dlg_data( &dlg_data );
	//dlg.add_dlg_data( ( void* ) &file_name );
	//dlg.add_dlg_data( ( void* ) &options );
	//dlg.add_dlg_data( ( void* ) &info );

	//if ( info.time != 0 ) {
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_RESULT_PROCESSED1 ).data( ), &format_inf_amount_short( info.file_size ), &format_time2( info.time ), &format_inf_amount_short( info.file_size * 1000 / info.time, true ) ) );
	//	}
	//else {
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_RESULT_PROCESSED2 ).data( ), &format_inf_amount_short( info.file_size ), &format_time2( info.time ) ) );
	//	}
	//dlg.new_line( );

	//if ( options.compression ) {
	//	dlg.separator( );
	//	dlg.new_line( );
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_RESULT_COMPRESSION ).data( ), &format_inf_amount_short( info.comp_size ), &format_inf_amount_short( info.file_size ) ) );
	//	dlg.spacer( 1 );
	//	if ( info.file_size != 0 ) {
	//		dlg.fix_edit_box( std::string::format( L"%Lu", info.comp_size * 100 / info.file_size ), AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		}
	//	else {
	//		dlg.fix_edit_box( std::string::format( L"%u", 100 ), AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		}
	//	dlg.label( L"%" );
	//	dlg.new_line( );
	//	}

	//bool hash_opt = options.crc32 || options.md5 || options.sha1 || options.sha256 || options.ed2k || options.crc16;
	//if ( hash_opt ) {
	//	dlg.separator( );
	//	dlg.new_line( );

	//	unsigned pad_size = max( max( max( max( max( max( get_label_len( far_get_msg( MSG_CONTENT_RESULT_CRC32 ) ), get_label_len( far_get_msg( MSG_CONTENT_RESULT_MD5 ) ) ), get_label_len( far_get_msg( MSG_CONTENT_RESULT_SHA1 ) ) ), get_label_len( far_get_msg( MSG_CONTENT_RESULT_SHA256 ) ) ), get_label_len( far_get_msg( MSG_CONTENT_RESULT_ED2K ) ) ), get_label_len( far_get_msg( MSG_CONTENT_RESULT_VERIFY ) ) ), get_label_len( far_get_msg( MSG_CONTENT_RESULT_CRC16 ) ) ) + 1;
	//	unsigned verify_box_size = 0;

	//	if ( options.crc32 ) {
	//		dlg.label( far_get_msg( MSG_CONTENT_RESULT_CRC32 ) );
	//		dlg.pad( pad_size );
	//		std::string hash_str = format_hex_array( info.crc32 );
	//		dlg.fix_edit_box( hash_str, AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		dlg.new_line( );
	//		if ( verify_box_size < hash_str.size( ) ) verify_box_size = hash_str.size( );
	//		}

	//	if ( options.md5 ) {
	//		dlg.label( far_get_msg( MSG_CONTENT_RESULT_MD5 ) );
	//		dlg.pad( pad_size );
	//		std::string hash_str = format_hex_array( info.md5 );
	//		dlg.fix_edit_box( hash_str, AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		dlg.new_line( );
	//		if ( verify_box_size < hash_str.size( ) ) verify_box_size = hash_str.size( );
	//		}

	//	if ( options.sha1 ) {
	//		dlg.label( far_get_msg( MSG_CONTENT_RESULT_SHA1 ) );
	//		dlg.pad( pad_size );
	//		std::string hash_str = format_hex_array( info.sha1 );
	//		dlg.fix_edit_box( hash_str, AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		dlg.new_line( );
	//		if ( verify_box_size < hash_str.size( ) ) verify_box_size = hash_str.size( );
	//		}

	//	if ( options.sha256 ) {
	//		dlg.label( far_get_msg( MSG_CONTENT_RESULT_SHA256 ) );
	//		dlg.pad( pad_size );
	//		std::string hash_str = format_hex_array( info.sha256 );
	//		dlg.fix_edit_box( hash_str, AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		dlg.new_line( );
	//		if ( verify_box_size < hash_str.size( ) ) verify_box_size = hash_str.size( );
	//		}

	//	if ( options.ed2k ) {
	//		dlg.label( far_get_msg( MSG_CONTENT_RESULT_ED2K ) );
	//		dlg.pad( pad_size );
	//		std::string hash_str = format_hex_array( info.ed2k );
	//		dlg.fix_edit_box( hash_str, AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		dlg.new_line( );
	//		if ( verify_box_size < hash_str.size( ) ) verify_box_size = hash_str.size( );
	//		}

	//	if ( options.crc16 ) {
	//		dlg.label( far_get_msg( MSG_CONTENT_RESULT_CRC16 ) );
	//		dlg.pad( pad_size );
	//		std::string hash_str = format_hex_array( info.crc16 );
	//		dlg.fix_edit_box( hash_str, AUTO_SIZE, DIF_READONLY | DIF_SELECTONENTRY );
	//		dlg.new_line( );
	//		if ( verify_box_size < hash_str.size( ) ) verify_box_size = hash_str.size( );
	//		}

	//	// hash check if apropriate
	//	dlg.separator( );
	//	dlg.new_line( );

	//	dlg.label( far_get_msg( MSG_CONTENT_RESULT_VERIFY ) );
	//	dlg.pad( pad_size );
	//	dlg_data.verify_edit_ctrl_id = dlg.var_edit_box( L"", verify_box_size + 1 );
	//	dlg.new_line( );

	//	dlg.label( far_get_msg( MSG_CONTENT_RESULT_RESULT ) );
	//	dlg.spacer( 1 );
	//	dlg_data.correct_result_label_ctrl_id = dlg.label( far_get_msg( MSG_CONTENT_RESULT_CORRECT ), AUTO_SIZE, DIF_HIDDEN );
	//	dlg.same_pos( );
	//	dlg_data.wrong_result_label_ctrl_id = dlg.label( far_get_msg( MSG_CONTENT_RESULT_WRONG ), AUTO_SIZE, DIF_HIDDEN );
	//	dlg.new_line( );
	//	}

	//dlg.separator( );
	//dlg.new_line( );
	//dlg.def_button( far_get_msg( MSG_CONTENT_RESULT_CLOSE ), DIF_CENTERGROUP );
	//dlg_data.save_file_ctrl_id = hash_opt ? dlg.button( far_get_msg( MSG_CONTENT_RESULT_SAVE_FILE ), DIF_CENTERGROUP | DIF_BTNNOCLOSE ) : -1;
	//dlg.new_line( );

	//dlg.show( result_dlg_proc );
	}

//void ed2k_update_block_hashes( const u8* buffer, unsigned buffer_size, Array<u8>& block_hashes, unsigned& last_block_slack, MD4_CTX& md4_ctx ) {
//	const unsigned c_ed2k_block_size = 9500 * 1024;
//	if ( last_block_slack > buffer_size ) {
//		MD4_Update( &md4_ctx, buffer, buffer_size );
//		last_block_slack -= buffer_size;
//		}
//	else {
//		if ( last_block_slack != 0 ) {
//			// finish previous partial block
//			MD4_Update( &md4_ctx, buffer, last_block_slack );
//			u8 md4[ MD4_DIGEST_LENGTH ];
//			MD4_Final( md4, &md4_ctx );
//			block_hashes.add( md4, sizeof( md4 ) );
//			}
//		// process whole blocks
//		unsigned idx = last_block_slack;
//		while ( idx + c_ed2k_block_size <= buffer_size ) {
//			MD4_Init( &md4_ctx );
//			MD4_Update( &md4_ctx, buffer + idx, c_ed2k_block_size );
//			u8 md4[ MD4_DIGEST_LENGTH ];
//			MD4_Final( md4, &md4_ctx );
//			block_hashes.add( md4, sizeof( md4 ) );
//			idx += c_ed2k_block_size;
//			}
//		if ( idx < buffer_size ) {
//			// process last partial block
//			MD4_Init( &md4_ctx );
//			MD4_Update( &md4_ctx, buffer + idx, buffer_size - idx );
//			last_block_slack = c_ed2k_block_size - ( buffer_size - idx );
//			}
//		else {
//			// there is no partial block present at the end
//			last_block_slack = 0;
//			}
//		}
//	}
//
//Array<u8> ed2k_finalize_block_hashes( Array<u8>& block_hashes, unsigned last_block_slack, MD4_CTX& md4_ctx ) {
//	u8 md4[ MD4_DIGEST_LENGTH ];
//	// if there is unfinished block
//	if ( last_block_slack != 0 ) {
//		MD4_Final( md4, &md4_ctx );
//		block_hashes.add( md4, sizeof( md4 ) );
//		}
//	// add last zero length block
//	else {
//		MD4_Init( &md4_ctx );
//		MD4_Update( &md4_ctx, NULL, 0 );
//		MD4_Final( md4, &md4_ctx );
//		block_hashes.add( md4, sizeof( md4 ) );
//		}
//	// if there is only one block
//	if ( block_hashes.size( ) == sizeof( md4 ) ) {
//		// then ed2k hash = block hash
//		return block_hashes;
//		}
//	// there are several blocks
//	else {
//		// ed2k hash = md4 of all the block hashes
//		MD4_Init( &md4_ctx );
//		MD4_Update( &md4_ctx, block_hashes.data( ), block_hashes.size( ) );
//		MD4_Final( md4, &md4_ctx );
//		return Array<u8>( md4, sizeof( md4 ) );
//		}
//	}

enum BufState {
	bs_io_ready,
	bs_io_in_progress,
	bs_proc_ready,
	bs_processing,
	};

struct SharedData {
	unsigned num_th;
	unsigned num_buf;
	Array<BufState> buffer_state;
	u8* buffer;
	unsigned buffer_size;
	Array<unsigned> buffer_data_size;
	u8* comp_buffer;
	unsigned comp_buffer_size;
	u8* comp_work_buffer;
	unsigned comp_work_buffer_size;
	HANDLE h_stop_event;
	CRITICAL_SECTION sync;
	HANDLE h_io_ready_sem;
	HANDLE h_proc_ready_sem;
	u64 data_size;
	u64 comp_size;
	};

template<typename Data> bool compress_buffer( Data* d ) {
	unsigned buf_idx;
	EnterCriticalSection( &d->sync );
	try {
		buf_idx = d->buffer_state.search( bs_proc_ready );
		if ( buf_idx != -1 ) {
			d->buffer_state.item( buf_idx ) = bs_processing;
			}
		}
	finally ( LeaveCriticalSection( &d->sync ) );
	if ( buf_idx == -1 ) {
		return false;
		}

	lzo_uint comp_size;
	CHECK_LZO( lzo1x_1_compress( d->buffer + buf_idx * d->buffer_size, d->buffer_data_size[ buf_idx ], d->comp_buffer + buf_idx * d->comp_buffer_size, &comp_size, d->comp_work_buffer + buf_idx * d->comp_work_buffer_size ) );

	EnterCriticalSection( &d->sync );
	try {
		// update stats
		d->comp_size += min( comp_size, d->buffer_data_size[ buf_idx ] );
		d->data_size += d->buffer_data_size[ buf_idx ];
		// mark buffer ready for I/O
		d->buffer_state.item( buf_idx ) = bs_io_ready;
		}
	finally ( LeaveCriticalSection( &d->sync ) );

	// signal I/O thread
	CHECK_SYS( ReleaseSemaphore( d->h_io_ready_sem, 1, NULL ) != 0 );
	return true;
	}

template<typename Data> unsigned __stdcall wth_proc( void* wth_param ) {
	try {
		Data* d = ( Data* ) wth_param;
		bool running = true;
		while ( running ) {
			HANDLE h[ 2 ] = { d->h_stop_event, d->h_proc_ready_sem };
			DWORD w = WaitForMultipleObjects( 2, h, FALSE, INFINITE );
			CHECK_SYS( w != WAIT_FAILED );
			if ( w == WAIT_OBJECT_0 ) {
				while ( compress_buffer( d ) );
				running = false;
				}
			else if ( w == WAIT_OBJECT_0 + 1 ) {
				VERIFY( compress_buffer( d ) );
				}
			}
		return TRUE;
		}
	catch ( ... ) {
		return FALSE;
		}
	}
//
//class ProcessFileProgress : public ProgressMonitor {
//	protected:
//	virtual void do_update_ui( ) {
//		const unsigned c_client_xs = 55;
//		std::vector<std::string> lines;
//
//		u64 data_size;
//		u64 comp_size;
//		if ( sd.num_th != 0 ) {
//			EnterCriticalSection( &sd.sync );
//			}
//		try {
//			data_size = sd.data_size;
//			comp_size = sd.comp_size;
//			}
//		finally ( if ( sd.num_th != 0 ) { LeaveCriticalSection( &sd.sync ) } );
//		u64 file_size = result.file_size;
//		u64 time = time_elapsed( );
//
//		// percent done
//		unsigned percent_done;
//		if ( file_size == 0 ) {
//			percent_done = 100;
//			}
//		else {
//			percent_done = ( unsigned ) ( data_size * 100 / file_size );
//			}
//
//		// if too little time to estimate speed
//		if ( time != 0 ) {
//			//lines += std::string::format( far_get_msg( MSG_CONTENT_PROGRESS_PROCESSED1 ).data( ), &format_inf_amount_short( data_size ),
//			//  &format_inf_amount_short( file_size ), percent_done,
//			//  &format_inf_amount_short( data_size * 1000 / time, true ) );
//			}
//		else {
//			//lines += std::string::format( far_get_msg( MSG_CONTENT_PROGRESS_PROCESSED2 ).data( ), &format_inf_amount_short( data_size ),
//			//  &format_inf_amount_short( file_size ), percent_done );
//			}
//
//		// progress bar
//		if ( file_size != 0 ) {
//			unsigned len1;
//			if ( file_size == 0 ) {
//				len1 = c_client_xs;
//				}
//			else {
//				len1 = ( unsigned ) ( data_size * c_client_xs / file_size );
//				}
//			if ( len1 > c_client_xs ) {
//				len1 = c_client_xs;
//				}
//			unsigned len2 = c_client_xs - len1;
//			lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
//			}
//
//		// time left
//		if ( ( time != 0 ) && ( data_size != 0 ) ) {
//			u64 total_time = file_size * time / data_size;
//			//lines += std::string::format( far_get_msg( MSG_CONTENT_PROGRESS_ELAPSED ).data( ), &format_time( time ),
//			//  &format_time( total_time - time ), &format_time( total_time ) );
//			}
//
//		if ( ( options.compression ) && ( data_size != 0 ) ) {
//			//lines += std::string::format( far_get_msg( MSG_CONTENT_PROGRESS_COMPRESSION ).data( ), &format_inf_amount_short( comp_size ),
//			//  &format_inf_amount_short( data_size ), 100 * comp_size / data_size );
//			}
//
//		//draw_text_box( far_get_msg( MSG_CONTENT_PROGRESS_TITLE ), lines, c_client_xs );
//		//SetConsoleTitleW( std::string::format( far_get_msg( MSG_CONTENT_PROGRESS_CONSOLE_TITLE ).data( ), percent_done ).data( ) );
//		far_set_progress_state( TBPF_NORMAL );
//		far_set_progress_value( percent_done, 100 );
//		}
//	public:
//	SharedData& sd;
//	const ContentInfo& result;
//	const ContentOptions& options;
//	ProcessFileProgress( SharedData& sd, const ContentInfo& result, const ContentOptions& options ) : ProgressMonitor( true ), sd( sd ), result( result ), options( options ) { }
//	};

//void process_file_content( const std::string& file_name, const ContentOptions& options, ContentInfo& result ) {
//	ALLOC_RSRC( HANDLE h_scr = g_far.SaveScreen( 0, 0, -1, -1 ) );
//
//	SharedData sd;
//
//	sd.buffer_size = 16 * 4 * 1024; // NTFS compression unit = 16 clusters
//
//	SYSTEM_INFO sys_info;
//	GetSystemInfo( &sys_info );
//	sd.num_th = options.compression ? ( sys_info.dwNumberOfProcessors > 1 ? sys_info.dwNumberOfProcessors : 0 ) : 0;
//	sd.num_buf = options.compression ? sys_info.dwNumberOfProcessors * 2 : 2;
//
//	ALLOC_RSRC( if ( sd.num_th != 0 ) { sd.h_stop_event = CreateEvent( NULL, TRUE, FALSE, NULL ); CHECK_SYS( sd.h_stop_event != NULL ); } );
//	ALLOC_RSRC( if ( sd.num_th != 0 ) InitializeCriticalSection( &sd.sync ); );
//	ALLOC_RSRC( if ( sd.num_th != 0 ) { sd.h_io_ready_sem = CreateSemaphore( NULL, sd.num_buf, sd.num_buf, NULL ); CHECK_SYS( sd.h_io_ready_sem != NULL ); } );
//	ALLOC_RSRC( if ( sd.num_th != 0 ) { sd.h_proc_ready_sem = CreateSemaphore( NULL, 0, sd.num_buf, NULL ); CHECK_SYS( sd.h_proc_ready_sem != NULL ); } );
//	ALLOC_RSRC( HANDLE h_io_event = CreateEvent( NULL, TRUE, FALSE, NULL ); CHECK_SYS( h_io_event != NULL ) ); // async. I/O event
//	ALLOC_RSRC( sd.buffer = ( u8* ) VirtualAlloc( NULL, sd.buffer_size * sd.num_buf, MEM_COMMIT, PAGE_READWRITE ); CHECK_SYS( sd.buffer != NULL ) );
//	sd.comp_buffer_size = sd.buffer_size + sd.buffer_size / 16 + 64 + 3;
//	ALLOC_RSRC( if ( options.compression ) { sd.comp_buffer = new u8[ sd.comp_buffer_size * ( sd.num_th != 0 ? sd.num_buf : 1 ) ]; } );
//	sd.comp_work_buffer_size = LZO1X_1_MEM_COMPRESS;
//	ALLOC_RSRC( if ( options.compression ) { sd.comp_work_buffer = new u8[ sd.comp_work_buffer_size * ( sd.num_th != 0 ? sd.num_buf : 1 ) ]; } );
//
//	for ( unsigned i = 0; i < sd.num_buf; i++ ) {
//		sd.buffer_state += bs_io_ready;
//		sd.buffer_data_size += 0;
//		}
//
//	sd.data_size = 0; // file data size
//	sd.comp_size = 0; // compressed file size
//
//	// crc32 checksum
//	u32 crc32 = 0;
//	// MD5 hash
//	MD5_CTX md5_ctx;
//	MD5_Init( &md5_ctx );
//	// SHA1 hash
//	SHA_CTX sha1_ctx;
//	SHA1_Init( &sha1_ctx );
//	// SHA256 hash
//	SHA256_CTX sha256_ctx;
//	SHA256_Init( &sha256_ctx );
//	// ed2k hash
//	Array<u8> ed2k_block_hashes;
//	unsigned ed2k_last_block_slack = 0;
//	MD4_CTX md4_ctx;
//	// crc16 checksum
//	u16 crc16 = CRC16::init( );
//
//	ProcessFileProgress progress( sd, result, options );
//
//	// create compression threads
//	ALLOC_RSRC( Array<HANDLE> h_wth );
//	for ( unsigned i = 0; i < sd.num_th; i++ ) {
//		unsigned th_id;
//		HANDLE h = ( HANDLE ) _beginthreadex( NULL, 0, wth_proc<SharedData>, &sd, 0, &th_id );
//		CHECK_SYS( h != NULL );
//		h_wth += h;
//		}
//	try {
//		if ( sd.num_th != 0 ) CHECK_SYS( SetThreadPriority( h_wth.last( ), THREAD_PRIORITY_BELOW_NORMAL ) != 0 );
//
//		ALLOC_RSRC( HANDLE h_file = CreateFileW( long_path( file_name ).data( ), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED | FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN, NULL ); CHECK_SYS( h_file != INVALID_HANDLE_VALUE ) );
//
//		// determine file size
//		DWORD fsize_hi;
//		DWORD fsize_lo = GetFileSize( h_file, &fsize_hi );
//		CHECK_SYS( ( fsize_lo != INVALID_FILE_SIZE ) || ( GetLastError( ) == NO_ERROR ) );
//		result.file_size = ( ( u64 ) fsize_hi << 32 ) | fsize_lo;
//
//		unsigned prev_buf_idx = -1; // previous I/O buffer
//		u64 file_ptr = 0; // file pointer
//		OVERLAPPED ov; // overlapped I/O structure
//		DWORD last_error; // file read operation result
//
//		// file read loop
//		Array<HANDLE> h = sd.h_io_ready_sem + h_wth;
//		bool eof = false;
//		while ( !eof ) {
//			if ( sd.num_th != 0 ) {
//				DWORD w = WaitForMultipleObjects( h.size( ), h.data( ), FALSE, INFINITE );
//				CHECK_SYS( w != WAIT_FAILED );
//				CHECK_MSG( w == WAIT_OBJECT_0, L"Unexpected thread death" );
//				}
//
//			// find buffer ready for I/O
//			unsigned buf_idx = 0;
//			if ( sd.num_th != 0 ) {
//				EnterCriticalSection( &sd.sync );
//				try {
//					buf_idx = sd.buffer_state.search( bs_io_ready );
//					assert( buf_idx != -1 );
//					sd.buffer_state.item( buf_idx ) = bs_io_in_progress;
//					}
//				finally ( LeaveCriticalSection( &sd.sync ) );
//				}
//			else {
//				if ( prev_buf_idx == 0 ) buf_idx = 1;
//				}
//
//			// specify file read offset
//			memset( &ov, 0, sizeof( ov ) );
//			ov.Offset = ( DWORD ) ( file_ptr & 0xFFFFFFFF );
//			ov.OffsetHigh = ( DWORD ) ( ( file_ptr >> 32 ) & 0xFFFFFFFF );
//			ov.hEvent = h_io_event;
//
//			unsigned buffer_data_size;
//			ReadFile( h_file, sd.buffer + buf_idx * sd.buffer_size, sd.buffer_size, ( LPDWORD ) &buffer_data_size, &ov );
//			last_error = GetLastError( );
//
//			CHECK_SYS( ( last_error == NO_ERROR ) || ( last_error == ERROR_IO_PENDING ) || ( last_error == ERROR_HANDLE_EOF ) );
//
//			// process previous I/O buffer while async. I/O is in progress
//			if ( prev_buf_idx != -1 ) {
//				u8* buffer = sd.buffer + prev_buf_idx * sd.buffer_size;
//				unsigned size = sd.buffer_data_size[ prev_buf_idx ];
//				if ( sd.num_th == 0 ) {
//					if ( options.compression ) {
//						lzo_uint comp_size;
//						CHECK_LZO( lzo1x_1_compress( buffer, size, sd.comp_buffer, &comp_size, sd.comp_work_buffer ) );
//						sd.comp_size += min( comp_size, size );
//						}
//					sd.data_size += size;
//					}
//				if ( options.crc32 ) crc32 = lzo_crc32( crc32, buffer, size );
//				if ( options.md5 ) MD5_Update( &md5_ctx, buffer, size );
//				if ( options.sha1 ) SHA1_Update( &sha1_ctx, buffer, size );
//				if ( options.sha256 ) SHA256_Update( &sha256_ctx, buffer, size );
//				if ( options.ed2k ) ed2k_update_block_hashes( buffer, size, ed2k_block_hashes, ed2k_last_block_slack, md4_ctx );
//				if ( options.crc16 ) crc16 = CRC16::update( crc16, buffer, size );
//				}
//
//			progress.update_ui( );
//
//			// check for end of file
//			eof = last_error == ERROR_HANDLE_EOF;
//			// determine number of bytes returned by last read operation
//			if ( last_error == ERROR_IO_PENDING ) {
//				// asynchronous operation
//				if ( GetOverlappedResult( h_file, &ov, ( LPDWORD ) &buffer_data_size, TRUE ) == 0 ) {
//					// check for end of file
//					if ( GetLastError( ) == ERROR_HANDLE_EOF ) eof = true;
//					// else async. operation failed
//					else CHECK_SYS( false );
//					}
//				}
//			sd.buffer_data_size.item( buf_idx ) = buffer_data_size;
//			assert( eof || ( !eof && ( ( prev_buf_idx == -1 ) || ( sd.buffer_data_size[ prev_buf_idx ] == sd.buffer_size ) ) ) );
//
//			if ( !eof ) {
//				// mark buffer ready for processing and signal worker threads
//				if ( sd.num_th != 0 ) {
//					EnterCriticalSection( &sd.sync );
//					try {
//						sd.buffer_state.item( buf_idx ) = bs_proc_ready;
//						}
//					finally ( LeaveCriticalSection( &sd.sync ) );
//					CHECK_SYS( ReleaseSemaphore( sd.h_proc_ready_sem, 1, NULL ) != 0 );
//					}
//				}
//
//			// advance file pointer
//			file_ptr += sd.buffer_size;
//
//			// this buffer will be processed next
//			prev_buf_idx = buf_idx;
//			}
//		FREE_RSRC( VERIFY( CloseHandle( h_file ) != 0 ) );
//		}
//	finally (
//	  if ( sd.num_th != 0 ) {
//		VERIFY( SetEvent( sd.h_stop_event ) != 0 );
//		VERIFY( WaitForMultipleObjects( h_wth.size( ), h_wth.data( ), TRUE, INFINITE ) != WAIT_FAILED );
//		  }
//	);
//	FREE_RSRC(
//	  // must use h_wth.size() instead of sd.num_th
//	  for ( unsigned i = 0; i < h_wth.size( ); i++ ) {
//		VERIFY( CloseHandle( h_wth[ i ] ) != 0 );
//		  }
//	);
//
//	// populate result structure
//	assert( sd.data_size == result.file_size );
//	progress.update_ui( );
//	result.time = progress.time_elapsed( );
//	if ( options.compression ) result.comp_size = sd.comp_size;
//	if ( options.crc32 ) {
//		const u8* c = ( const u8* ) &crc32;
//		result.crc32.copy( c[ 3 ] ).add( c[ 2 ] ).add( c[ 1 ] ).add( c[ 0 ] );
//		}
//	if ( options.md5 ) {
//		u8 md5[ MD5_DIGEST_LENGTH ];
//		MD5_Final( md5, &md5_ctx );
//		result.md5.copy( md5, sizeof( md5 ) );
//		}
//	if ( options.sha1 ) {
//		u8 sha1[ SHA_DIGEST_LENGTH ];
//		SHA1_Final( sha1, &sha1_ctx );
//		result.sha1.copy( sha1, sizeof( sha1 ) );
//		}
//	if ( options.sha256 ) {
//		u8 sha256[ SHA256_DIGEST_LENGTH ];
//		SHA256_Final( sha256, &sha256_ctx );
//		result.sha256.copy( sha256, sizeof( sha256 ) );
//		}
//	if ( options.ed2k ) {
//		result.ed2k = ed2k_finalize_block_hashes( ed2k_block_hashes, ed2k_last_block_slack, md4_ctx );
//		}
//	if ( options.crc16 ) {
//		const u8* c = ( const u8* ) &crc16;
//		result.crc16.copy( c[ 1 ] ).add( c[ 0 ] );
//		}
//
//	FREE_RSRC( if ( options.compression ) delete[ ] sd.comp_work_buffer );
//	FREE_RSRC( if ( options.compression ) delete[ ] sd.comp_buffer );
//	FREE_RSRC( VERIFY( VirtualFree( sd.buffer, 0, MEM_RELEASE ) != 0 ) );
//	FREE_RSRC( VERIFY( CloseHandle( h_io_event ) != 0 ) );
//	FREE_RSRC( if ( sd.num_th != 0 ) VERIFY( CloseHandle( sd.h_proc_ready_sem ) != 0 ) );
//	FREE_RSRC( if ( sd.num_th != 0 ) VERIFY( CloseHandle( sd.h_io_ready_sem ) != 0 ) );
//	FREE_RSRC( if ( sd.num_th != 0 ) DeleteCriticalSection( &sd.sync ) );
//	FREE_RSRC( if ( sd.num_th != 0 ) VERIFY( CloseHandle( sd.h_stop_event ) != 0 ) );
//	FREE_RSRC( g_far.RestoreScreen( NULL ); g_far.RestoreScreen( h_scr ) );
//	}

// show content analysis result dialog
void show_result_dialog( const CompressionStats& stats ) {
	std::string str;
	//FarDialog dlg( c_content_result_dialog_guid, far_get_msg( MSG_CONTENT_MULTI_RESULT_TITLE ), 30 );
	//if ( stats.file_cnt != 0 ) {
	//	// processing speed
	//	str.copy_fmt( far_get_msg( MSG_CONTENT_MULTI_RESULT_PROCESSED ).data( ), &format_inf_amount_short( stats.data_size ), &format_time2( stats.time ) );
	//	if ( stats.time != 0 ) {
	//		str.add( L' ' ).add_fmt( far_get_msg( MSG_CONTENT_MULTI_RESULT_SPEED ).data( ), &format_inf_amount_short( stats.data_size * 1000 / stats.time, true ) );
	//		}
	//	dlg.label( str );
	//	dlg.new_line( );

	//	// compression ratio
	//	str.copy_fmt( far_get_msg( MSG_CONTENT_MULTI_RESULT_COMPRESSION ).data( ), &format_inf_amount_short( stats.comp_size ), &format_inf_amount_short( stats.data_size ) );
	//	if ( stats.data_size != 0 ) {
	//		str.add_fmt( L" %Lu", stats.comp_size * 100 / stats.data_size );
	//		}
	//	else {
	//		str.add_fmt( L" %u", 100 );
	//		}
	//	str.add( '%' );
	//	dlg.label( str );
	//	dlg.new_line( );
	//	dlg.separator( );
	//	dlg.new_line( );
	//	}

	//// object counts
	//if ( stats.file_cnt != 0 ) {
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_MULTI_RESULT_FILES ).data( ), stats.file_cnt ) );
	//	dlg.new_line( );
	//	}
	//if ( stats.dir_cnt != 0 ) {
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_MULTI_RESULT_DIRS ).data( ), stats.dir_cnt ) );
	//	dlg.new_line( );
	//	}
	//if ( stats.reparse_cnt != 0 ) {
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_MULTI_RESULT_REPARSE ).data( ), stats.reparse_cnt ) );
	//	dlg.new_line( );
	//	}
	//if ( stats.err_cnt != 0 ) {
	//	dlg.label( std::string::format( far_get_msg( MSG_CONTENT_MULTI_RESULT_ERRORS ).data( ), stats.err_cnt ) );
	//	dlg.new_line( );
	//	}
	//dlg.separator( );
	//dlg.new_line( );

	//dlg.def_button( far_get_msg( MSG_CONTENT_MULTI_RESULT_CLOSE ), DIF_CENTERGROUP );
	//dlg.new_line( );

	//dlg.show( );
	}

struct CompressionState : public CompressionStats {
	unsigned num_th; // number of worker threads
	unsigned num_buf; // number of I/O buffers
	Array<BufState> buffer_state;
	u8* buffer; // I/O buffers
	unsigned buffer_size; // I/O buffer size
	Array<unsigned> buffer_data_size; // valid data size in each buffer
	u8* comp_buffer; // compression buffers
	unsigned comp_buffer_size;
	u8* comp_work_buffer;
	unsigned comp_work_buffer_size;
	HANDLE h_aio_event; // async. I/O event
	Array<HANDLE> h_wth; // worker thread handles
	HANDLE h_stop_event;
	CRITICAL_SECTION sync;
	HANDLE h_io_ready_sem;
	HANDLE h_proc_ready_sem;
	u64 est_size; // estimated total file data size
	unsigned est_file_cnt; // estimated number of files processed
	unsigned est_dir_cnt; // estimated number of dirs processed
	unsigned est_reparse_cnt; // estimated number of reparse points skipped
	unsigned est_err_cnt; // estimated number of files/dirs skipped because of errors
	};

class CompressFilesProgress : public ProgressMonitor {
	protected:
	virtual void do_update_ui( ) {
		const unsigned c_client_xs = 55;
		std::vector<std::string> lines;

		u64 data_size, est_size, comp_size;
		u64 time = time_elapsed( );
		unsigned file_cnt, dir_cnt, reparse_cnt, err_cnt;
		unsigned est_file_cnt, est_dir_cnt;
		if ( st.num_th != 0 ) {
			EnterCriticalSection( &st.sync );
			}
		try {
			data_size = st.data_size;
			est_size = st.est_size;
			comp_size = st.comp_size;
			file_cnt = st.file_cnt;
			dir_cnt = st.dir_cnt;
			reparse_cnt = st.reparse_cnt;
			err_cnt = st.err_cnt;
			est_file_cnt = st.est_file_cnt;
			est_dir_cnt = st.est_dir_cnt;
			}
		finally ( if ( st.num_th != 0 ) { LeaveCriticalSection( &st.sync ) } );

		// percent done
		unsigned percent_done;
		if ( est_size == 0 ) {
			percent_done = 100;
			}
		else {
			percent_done = ( unsigned ) ( data_size * 100 / est_size );
			}
		if ( percent_done > 100 ) {
			percent_done = 100;
			}

		//lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_PROCESSED ).data( ),
		  &format_inf_amount_short( data_size ),
		  &format_inf_amount_short( est_size ), percent_done );
		if ( time != 0 ) {
			//lines.at( lines.size( ) - 1 ).add( L' ' ).add_fmt( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_SPEED ).data( ), &format_inf_amount_short( data_size * 1000 / time, true ) );
			}

		if ( data_size != 0 ) {
			//lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_COMPRESSION ).data( ), &format_inf_amount_short( comp_size ),
			  &format_inf_amount_short( data_size ), 100 * comp_size / data_size );
			}
		lines.push_back(std::string("\x1"));

		// progress bar
		if ( est_size != 0 ) {
			unsigned len1;
			if ( est_size == 0 ) {
				len1 = c_client_xs;
				}
			else {
				len1 = ( unsigned ) ( data_size * c_client_xs / est_size );
				}
			if ( len1 > c_client_xs ) {
				len1 = c_client_xs;
				}
			unsigned len2 = c_client_xs - len1;
			lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
			}

		// time left
		if ( ( time != 0 ) && ( data_size != 0 ) ) {
			u64 total_time = est_size * time / data_size;
			//lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_ELAPSED ).data( ), &format_time( time ),
			  &format_time( total_time - time ), &format_time( total_time ) );
			}

		if ( ( file_cnt != 0 ) || ( dir_cnt != 0 ) || ( reparse_cnt != 0 ) || ( err_cnt != 0 ) ) {
			lines.push_back(std::string("\x1"));
			}
		//if ( file_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_FILES ).data( ), file_cnt, est_file_cnt );
		//if ( dir_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_DIRS ).data( ), dir_cnt, est_dir_cnt );
		//if ( reparse_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_REPARSE ).data( ), reparse_cnt );
		//if ( err_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_ERRORS ).data( ), err_cnt );

		//draw_text_box( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_TITLE ), lines, c_client_xs );
		//SetConsoleTitleW( std::string::format( far_get_msg( MSG_CONTENT_MULTI_PROGRESS_CONSOLE_TITLE ).data( ), percent_done ).data( ) );
		//far_set_progress_state( TBPF_NORMAL );
		//far_set_progress_value( percent_done, 100 );
		}
	public:
	CompressionState& st;
	CompressFilesProgress( CompressionState& st ) : ProgressMonitor( false ), st( st ) { }
	};

//class EstimationProgress : public ProgressMonitor {
//	protected:
//	virtual void do_update_ui( ) {
//		const unsigned c_client_xs = 35;
//		std::vector<std::string> lines;
//		//if ( st.est_size != 0 ) lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_SIZE ).data( ), &format_inf_amount_short( st.est_size ) );
//		if ( ( st.est_size != 0 ) && ( ( st.est_file_cnt != 0 ) || ( st.est_dir_cnt != 0 ) || ( st.est_reparse_cnt != 0 ) || ( st.est_err_cnt != 0 ) ) ) {
//			lines += L"\x1";
//			}
//		//if ( st.est_file_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_FILES ).data( ), st.est_file_cnt );
//		//if ( st.est_dir_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_DIRS ).data( ), st.est_dir_cnt );
//		//if ( st.est_reparse_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_REPARSE ).data( ), st.est_reparse_cnt );
//		//if ( st.est_err_cnt != 0 ) lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_ERRORS ).data( ), st.est_err_cnt );
//		//draw_text_box( far_get_msg( MSG_ESTIMATE_PROGRESS_TITLE ), lines, c_client_xs );
//		//SetConsoleTitleW( far_get_msg( MSG_ESTIMATE_PROGRESS_TITLE ).data( ) );
//		far_set_progress_state( TBPF_INDETERMINATE );
//		}
//	public:
//	const CompressionState& st;
//	EstimationProgress( const CompressionState& st ) : ProgressMonitor( true ), st( st ) { }
//	};

void estimate_file_size( const std::string& file_name, CompressionState& st, EstimationProgress& progress ) {
	WIN32_FIND_DATAW find_data;
	HANDLE h_find = FindFirstFileW( long_path( file_name ).data( ), &find_data );
	if ( h_find == INVALID_HANDLE_VALUE ) {
		st.est_err_cnt++;
		}
	else {
		ALLOC_RSRC( ; );
		st.est_size += ( ( u64 ) find_data.nFileSizeHigh << 32 ) | find_data.nFileSizeLow;
		progress.update_ui( );
		FREE_RSRC( FindClose( h_find ) );
		st.est_file_cnt++;
		}
	}

void estimate_directory_size( const std::string& dir_name, CompressionState& st, EstimationProgress& progress ) {
	bool root_dir = dir_name.last( ) == L'\\';
	std::string file_name;
	WIN32_FIND_DATAW find_data;
	bool more = true;
	HANDLE h_find = FindFirstFileW( long_path( dir_name + ( root_dir ? L"*" : L"\\*" ) ).data( ), &find_data );
	if ( h_find == INVALID_HANDLE_VALUE ) {
		if ( GetLastError( ) != ERROR_NO_MORE_FILES ) {
			st.est_err_cnt++;
			}
		}
	else {
		ALLOC_RSRC( ; );
		while ( more ) {
			if ( ( wcscmp( find_data.cFileName, L"." ) != 0 ) && ( wcscmp( find_data.cFileName, L".." ) != 0 ) ) {
				file_name = dir_name + ( root_dir ? L"" : L"\\" ) + find_data.cFileName;

				if ( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT ) {
					st.est_reparse_cnt++;
					}
				else if ( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY ) {
					estimate_directory_size( file_name, st, progress );
					}
				else {
					st.est_file_cnt++;
					st.est_size += ( ( u64 ) find_data.nFileSizeHigh << 32 ) | find_data.nFileSizeLow;
					}
				}

			progress.update_ui( );

			if ( FindNextFileW( h_find, &find_data ) == 0 ) {
				CHECK_SYS( GetLastError( ) == ERROR_NO_MORE_FILES );
				more = false;
				}
			} // while
		FREE_RSRC( VERIFY( FindClose( h_find ) != 0 ) );
		st.est_dir_cnt++;
		}
	}

void compress_file( const std::string& file_name, CompressionState& st, CompressFilesProgress& progress ) {
	HANDLE h_file = CreateFileW( long_path( file_name ).data( ), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED | FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( h_file == INVALID_HANDLE_VALUE ) {
		st.err_cnt++;
		}
	else {
		ALLOC_RSRC( ; );

		unsigned prev_buf_idx = -1; // previous I/O buffer
		u64 file_ptr = 0; // file pointer
		OVERLAPPED ov; // overlapped I/O structure
		DWORD last_error; // file read operation result

		// determine file size
		DWORD fsize_hi;
		DWORD fsize_lo = GetFileSize( h_file, &fsize_hi );
		CHECK_SYS( ( fsize_lo != INVALID_FILE_SIZE ) || ( GetLastError( ) == NO_ERROR ) );
		u64 file_size = ( ( u64 ) fsize_hi << 32 ) | fsize_lo;

		// file read loop
		Array<HANDLE> h = st.h_io_ready_sem + st.h_wth;
		bool eof = false;
		while ( !eof ) {
			if ( st.num_th != 0 ) {
				DWORD w = WaitForMultipleObjects( h.size( ), h.data( ), FALSE, INFINITE );
				CHECK_SYS( w != WAIT_FAILED );
				CHECK_MSG( w == WAIT_OBJECT_0, L"Unexpected thread death" );
				}

			// find buffer ready for I/O
			unsigned buf_idx = 0;
			if ( st.num_th != 0 ) {
				EnterCriticalSection( &st.sync );
				try {
					buf_idx = st.buffer_state.search( bs_io_ready );
					assert( buf_idx != -1 );
					st.buffer_state.item( buf_idx ) = bs_io_in_progress;
					}
				finally ( LeaveCriticalSection( &st.sync ) );
				}
			else {
				if ( prev_buf_idx == 0 ) {
					buf_idx = 1;
					}
				}

			// specify file read offset
			memset( &ov, 0, sizeof( ov ) );
			ov.Offset = ( DWORD ) ( file_ptr & 0xFFFFFFFF );
			ov.OffsetHigh = ( DWORD ) ( ( file_ptr >> 32 ) & 0xFFFFFFFF );
			ov.hEvent = st.h_aio_event;

			unsigned buffer_data_size;
			ReadFile( h_file, st.buffer + buf_idx * st.buffer_size, st.buffer_size, ( LPDWORD ) &buffer_data_size, &ov );
			last_error = GetLastError( );

			CHECK_SYS( ( last_error == NO_ERROR ) || ( last_error == ERROR_IO_PENDING ) || ( last_error == ERROR_HANDLE_EOF ) );

			// process previous I/O buffer while async. I/O is in progress
			if ( prev_buf_idx != -1 ) {
				if ( st.num_th == 0 ) {
					// compress buffer
					u8* buffer = st.buffer + prev_buf_idx * st.buffer_size;
					unsigned size = st.buffer_data_size[ prev_buf_idx ];
					//lzo_uint comp_size;
					//CHECK_LZO( lzo1x_1_compress( buffer, size, st.comp_buffer, &comp_size, st.comp_work_buffer ) );
					//st.comp_size += min( comp_size, size );
					st.data_size += size;
					}
				}

			progress.update_ui( );

			// check for end of file
			eof = last_error == ERROR_HANDLE_EOF;
			// determine number of bytes returned by last read operation
			if ( last_error == ERROR_IO_PENDING ) {
				// asynchronous operation
				if ( GetOverlappedResult( h_file, &ov, ( LPDWORD ) &buffer_data_size, TRUE ) == 0 ) {
					// check for end of file
					if ( GetLastError( ) == ERROR_HANDLE_EOF ) {
						eof = true;
						}
					// else async. operation failed
					else {
						CHECK_SYS( false );
						}
					}
				}
			st.buffer_data_size.item( buf_idx ) = buffer_data_size;
			assert( eof || ( !eof && ( ( prev_buf_idx == -1 ) || ( st.buffer_data_size[ prev_buf_idx ] == st.buffer_size ) ) ) );

			// mark buffer ready for processing and signal worker threads
			if ( st.num_th != 0 ) {
				EnterCriticalSection( &st.sync );
				try {
					st.buffer_state.item( buf_idx ) = eof ? bs_io_ready : bs_proc_ready;
					}
				finally ( LeaveCriticalSection( &st.sync ) );
				CHECK_SYS( ReleaseSemaphore( eof ? st.h_io_ready_sem : st.h_proc_ready_sem, 1, NULL ) != 0 );
				}

			// advance file pointer
			file_ptr += st.buffer_size;

			// this buffer will be processed next
			prev_buf_idx = buf_idx;
			} // end file read loop
		FREE_RSRC( VERIFY( CloseHandle( h_file ) != 0 ) );
		st.file_cnt++;
		}
	}

void compress_directory( const std::string& dir_name, CompressionState& st, CompressFilesProgress& progress ) {
	bool root_dir = dir_name.last( ) == L'\\';
	std::string file_name;
	WIN32_FIND_DATAW find_data;
	bool more = true;
	HANDLE h_find = FindFirstFileW( long_path( dir_name + ( root_dir ? L"*" : L"\\*" ) ).data( ), &find_data );
	if ( h_find == INVALID_HANDLE_VALUE ) {
		if ( GetLastError( ) != ERROR_NO_MORE_FILES ) {
			st.err_cnt++;
			}
		}
	else {
		ALLOC_RSRC( ; );
		while ( more ) {
			if ( ( wcscmp( find_data.cFileName, L"." ) != 0 ) && ( wcscmp( find_data.cFileName, L".." ) != 0 ) ) {
				file_name = dir_name + ( root_dir ? L"" : L"\\" ) + find_data.cFileName;

				if ( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT ) {
					st.reparse_cnt++;
					}
				else if ( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY ) {
					compress_directory( file_name, st, progress );
					}
				else {
					compress_file( file_name, st, progress );
					}
				}

			progress.update_ui( );

			if ( FindNextFileW( h_find, &find_data ) == 0 ) {
				CHECK_SYS( GetLastError( ) == ERROR_NO_MORE_FILES );
				more = false;
				}
			} // while
		FREE_RSRC( VERIFY( FindClose( h_find ) != 0 ) );
		st.dir_cnt++;
		}
	}

void compress_files( const std::vector<std::string>& file_list, CompressionStats& result ) {
	CompressionState st;

	// save Far creen
	ALLOC_RSRC( HANDLE h_scr = g_far.SaveScreen( 0, 0, -1, -1 ) );

	st.buffer_size = 16 * 4 * 1024; // NTFS compression unit = 16 clusters

	SYSTEM_INFO sys_info;
	GetSystemInfo( &sys_info );
	st.num_th = sys_info.dwNumberOfProcessors > 1 ? sys_info.dwNumberOfProcessors : 0;
	st.num_buf = sys_info.dwNumberOfProcessors * 2;

	ALLOC_RSRC( if ( st.num_th != 0 ) { st.h_stop_event = CreateEvent( NULL, TRUE, FALSE, NULL ); CHECK_SYS( st.h_stop_event != NULL ); } );
	ALLOC_RSRC( if ( st.num_th != 0 ) InitializeCriticalSection( &st.sync ); );
	ALLOC_RSRC( if ( st.num_th != 0 ) { st.h_io_ready_sem = CreateSemaphore( NULL, st.num_buf, st.num_buf, NULL ); CHECK_SYS( st.h_io_ready_sem != NULL ); } );
	ALLOC_RSRC( if ( st.num_th != 0 ) { st.h_proc_ready_sem = CreateSemaphore( NULL, 0, st.num_buf, NULL ); CHECK_SYS( st.h_proc_ready_sem != NULL ); } );
	ALLOC_RSRC( st.h_aio_event = CreateEvent( NULL, TRUE, FALSE, NULL ); CHECK_SYS( st.h_aio_event != NULL ) ); // async. I/O event
	ALLOC_RSRC( st.buffer = ( u8* ) VirtualAlloc( NULL, st.buffer_size * st.num_buf, MEM_COMMIT, PAGE_READWRITE ); CHECK_SYS( st.buffer != NULL ) );
	st.comp_buffer_size = st.buffer_size + st.buffer_size / 16 + 64 + 3;
	ALLOC_RSRC( st.comp_buffer = new u8[ st.comp_buffer_size * ( st.num_th != 0 ? st.num_buf : 1 ) ] );
	//st.comp_work_buffer_size = LZO1X_1_MEM_COMPRESS;
	//ALLOC_RSRC( st.comp_work_buffer = new u8[ st.comp_work_buffer_size * ( st.num_th != 0 ? st.num_buf : 1 ) ] );

	for ( unsigned i = 0; i < st.num_buf; i++ ) {
		st.buffer_state += bs_io_ready;
		st.buffer_data_size += 0;
		}

	  {
	  EstimationProgress progress( st );

	  st.est_size = st.est_file_cnt = st.est_dir_cnt = st.est_reparse_cnt = st.est_err_cnt = 0;

	  // estimate total file size
	  for ( unsigned i = 0; i < file_list.size( ); i++ ) {
		  const std::string& file_name = file_list[ i ];
		  DWORD fattr = GetFileAttributesW( file_name.data( ) );
		  if ( fattr == INVALID_FILE_ATTRIBUTES ) {
			  st.est_err_cnt++;
			  }
		  else if ( ( fattr & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT ) {
			  st.est_reparse_cnt++;
			  }
		  else if ( ( fattr & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY ) {
			  estimate_directory_size( file_name, st, progress );
			  }
		  else {
			  estimate_file_size( file_name, st, progress );
			  }
		  }
		}

	st.data_size = 0;
	st.comp_size = 0;
	st.time = 0;
	st.file_cnt = 0;
	st.dir_cnt = 0;
	st.reparse_cnt = 0;
	st.err_cnt = 0;

	CompressFilesProgress progress( st );

	// create compression threads
	ALLOC_RSRC( st.h_wth );
	for ( unsigned i = 0; i < st.num_th; i++ ) {
		unsigned th_id;
		HANDLE h = ( HANDLE ) _beginthreadex( NULL, 0, wth_proc<CompressionState>, &st, 0, &th_id );
		CHECK_SYS( h != NULL );
		st.h_wth += h;
		}
	try {
		if ( st.num_th != 0 ) {
			CHECK_SYS( SetThreadPriority( st.h_wth.last( ), THREAD_PRIORITY_BELOW_NORMAL ) != 0 );
			}

		unsigned i;
		for ( i = 0; i < file_list.size( ); i++ ) {
			const std::string& file_name = file_list[ i ];
			DWORD fattr = GetFileAttributesW( file_name.data( ) );
			if ( fattr == INVALID_FILE_ATTRIBUTES ) {
				st.err_cnt++;
				}
			else if ( ( fattr & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT ) {
				st.reparse_cnt++;
				}
			else if ( ( fattr & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY ) {
				compress_directory( file_name, st, progress );
				}
			else {
				compress_file( file_name, st, progress );
				}
			}
		}
	finally (
	  if ( st.num_th != 0 ) {
		VERIFY( SetEvent( st.h_stop_event ) != 0 );
		VERIFY( WaitForMultipleObjects( st.h_wth.size( ), st.h_wth.data( ), TRUE, INFINITE ) != WAIT_FAILED );
		  }
	);
	FREE_RSRC(
	  // must use h_wth.size() instead of sd.num_th
	  for ( unsigned i = 0; i < st.h_wth.size( ); i++ ) {
		VERIFY( CloseHandle( st.h_wth[ i ] ) != 0 );
		  }
	);

	// populate result structure
	progress.update_ui( );
	st.time = progress.time_elapsed( );
	result = st;

	FREE_RSRC( delete[ ] st.comp_work_buffer );
	FREE_RSRC( delete[ ] st.comp_buffer );
	FREE_RSRC( VERIFY( VirtualFree( st.buffer, 0, MEM_RELEASE ) != 0 ) );
	FREE_RSRC( VERIFY( CloseHandle( st.h_aio_event ) != 0 ) );
	FREE_RSRC( if ( st.num_th != 0 ) VERIFY( CloseHandle( st.h_proc_ready_sem ) != 0 ) );
	FREE_RSRC( if ( st.num_th != 0 ) VERIFY( CloseHandle( st.h_io_ready_sem ) != 0 ) );
	FREE_RSRC( if ( st.num_th != 0 ) DeleteCriticalSection( &st.sync ) );
	FREE_RSRC( if ( st.num_th != 0 ) VERIFY( CloseHandle( st.h_stop_event ) != 0 ) );
	//FREE_RSRC( g_far.RestoreScreen( NULL ); g_far.RestoreScreen( h_scr ) );
	//FREE_RSRC( g_far.RestoreScreen( h_scr ) );
	}