#define _ERROR_WINDOWS
#include "error.h"
#include <Windows.h>
#include <map>
#include <list>
#include <set>
#include <array>


#include "utils.h"
#include "ntfs.h"
#include "volume.h"
#include "ntfs_file.h"
#include "options.h"
#include "dlgapi.h"
#include "file_panel.h"

#define NTFS_FILE_REC_HEADER_SIZE offsetof(NTFS_FILE_RECORD_OUTPUT_BUFFER, FileRecordBuffer)

struct FilePanel::FileRecordCompare {
	int operator()( const FileRecord& item1, const FileRecord& item2 ) {
		if ( item1.parent_ref_num > item2.parent_ref_num ) {
			return 1;
			}
		else if ( item1.parent_ref_num == item2.parent_ref_num ) {
			return item1.file_name.compare( item2.file_name );
			}
		else return -1;
		}
	};

void FilePanel::add_file_records( std::list<FileRecord>& file_list, const FileInfo& file_info ) {
	u64 data_size = 0;
	u64 nr_disk_size = 0;
	u64 valid_size = 0;
	unsigned stream_cnt = 0;
	unsigned fragment_cnt = 0;
	unsigned hard_link_cnt = 0;
	bool fully_resident = true;
	for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
		const AttrInfo& attr_info = file_info.attr_list[ i ];
		if ( !attr_info.resident ) {
			nr_disk_size += attr_info.disk_size;
			fully_resident = false;
			}
		if ( attr_info.type == AT_DATA ) {
			data_size += attr_info.data_size;
			valid_size += attr_info.valid_size;
			stream_cnt++;
			}
		if ( attr_info.fragments > 1 ) {
			fragment_cnt += static_cast< unsigned >( attr_info.fragments - 1 );
			}
		}
	for ( unsigned i = 0; i < file_info.file_name_list.size( ); i++ ) {
		if ( file_info.file_name_list[ i ].file_name_type != FILE_NAME_DOS ) {
			hard_link_cnt++;
			}
		}
	DWORD file_attr = file_info.std_info.file_attributes;
	if ( file_info.base_mft_rec( )->flags & MFT_RECORD_IS_DIRECTORY ) {
		file_attr |= FILE_ATTRIBUTE_DIRECTORY;
		}

	for ( unsigned i = 0; i < file_info.file_name_list.size( ); i++ ) {
		const FileNameAttr& name_attr = file_info.file_name_list[ i ];
		if ( name_attr.file_name_type != FILE_NAME_DOS ) {
			FileRecord rec;
			rec.file_ref_num = file_info.file_ref_num( );
			rec.parent_ref_num = name_attr.parent_directory;
			rec.file_name = name_attr.name;
			rec.file_attr = file_attr;
			U64_TO_FILETIME( rec.creation_time, file_info.std_info.creation_time );
			U64_TO_FILETIME( rec.last_access_time, file_info.std_info.last_access_time );
			U64_TO_FILETIME( rec.last_write_time, file_info.std_info.last_data_change_time );
			rec.data_size = data_size;
			rec.disk_size = nr_disk_size;
			rec.valid_size = valid_size;
			rec.fragment_cnt = fragment_cnt;
			rec.stream_cnt = stream_cnt;
			rec.hard_link_cnt = hard_link_cnt;
			rec.mft_rec_cnt = file_info.mft_rec_cnt;
			rec.set_flags( false, fully_resident );
			file_list.push_back( rec );
			}
		}

	if ( g_file_panel_mode.show_streams ) {
		unsigned data_or_nr_cnt = 0;
		bool named_data = false;
		for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
			const AttrInfo& attr = file_info.attr_list[ i ];
			if ( !attr.resident || ( attr.type == AT_DATA ) ) {
				data_or_nr_cnt++;
				}
			if ( ( attr.type == AT_DATA ) && ( attr.name.size( ) != 0 ) ) {
				named_data = true;
				}
			}
		// multiple non-resident/data attributes or at least one named data attribute
		if ( ( data_or_nr_cnt > 1 ) || named_data ) {
			for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
				const AttrInfo& attr = file_info.attr_list[ i ];
				if ( attr.resident && ( attr.type != AT_DATA ) ) {
					continue;
					}
				if ( !g_file_panel_mode.show_main_stream && ( attr.type == AT_DATA ) && ( attr.name.size( ) == 0 ) ) {
					continue;
					}

				for ( unsigned i = 0; i < file_info.file_name_list.size( ); i++ ) {
					const FileNameAttr& name_attr = file_info.file_name_list[ i ];
					if ( name_attr.file_name_type != FILE_NAME_DOS ) {
						unsigned fragment_cnt = ( unsigned ) attr.fragments;
						if ( fragment_cnt != 0 ) {
							fragment_cnt--;
							}

						file_attr &= ~FILE_ATTRIBUTE_DIRECTORY & ~FILE_ATTRIBUTE_REPARSE_POINT;
						if ( attr.compressed ) {
							file_attr |= FILE_ATTRIBUTE_COMPRESSED;
							}
						else {
							file_attr &= ~FILE_ATTRIBUTE_COMPRESSED;
							}
						if ( attr.encrypted ) {
							file_attr |= FILE_ATTRIBUTE_ENCRYPTED;
							}
						else {
							file_attr &= ~FILE_ATTRIBUTE_ENCRYPTED;
							}
						if ( attr.sparse ) {
							file_attr |= FILE_ATTRIBUTE_SPARSE_FILE;
							}
						else {
							file_attr &= ~FILE_ATTRIBUTE_SPARSE_FILE;
							}

						FileRecord rec;
						rec.file_ref_num = file_info.file_ref_num( );
						rec.parent_ref_num = name_attr.parent_directory;
						rec.file_name = name_attr.name + ":" + attr.name + ":$" + attr.type_name( );
						rec.file_attr = file_attr;
						U64_TO_FILETIME( rec.creation_time, file_info.std_info.creation_time );
						U64_TO_FILETIME( rec.last_access_time, file_info.std_info.last_access_time );
						U64_TO_FILETIME( rec.last_write_time, file_info.std_info.last_data_change_time );
						rec.data_size = attr.data_size;
						rec.disk_size = attr.disk_size;
						rec.valid_size = attr.valid_size;
						rec.fragment_cnt = fragment_cnt;
						rec.stream_cnt = 0;
						rec.hard_link_cnt = 0;
						rec.mft_rec_cnt = 0;
						rec.set_flags( true, attr.resident );
						file_list.push_back( rec );
						}
					}
				}
			}
		}
	}

void FilePanel::prepare_usn_journal( ) {
	if ( !g_file_panel_mode.use_usn_journal ) {
		return;
		}
	DWORD bytes_ret;
	USN_JOURNAL_DATA journal_data;
	if ( !DeviceIoControl( volume.handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journal_data, sizeof( journal_data ), &bytes_ret, NULL ) ) {
		if ( ( GetLastError( ) == ERROR_JOURNAL_NOT_ACTIVE ) || ( GetLastError( ) == ERROR_JOURNAL_DELETE_IN_PROGRESS ) ) {
			if ( g_file_panel_mode.use_existing_usn_journal ) {
				return;
				}
			if ( GetLastError( ) == ERROR_JOURNAL_DELETE_IN_PROGRESS ) {
				DELETE_USN_JOURNAL_DATA delete_journal_data;
				delete_journal_data.UsnJournalID = 0;
				delete_journal_data.DeleteFlags = USN_DELETE_FLAG_NOTIFY;
				CHECK_SYS( DeviceIoControl( volume.handle, FSCTL_DELETE_USN_JOURNAL, &delete_journal_data, sizeof( delete_journal_data ), NULL, 0, &bytes_ret, NULL ) );
				}
			CREATE_USN_JOURNAL_DATA create_journal_data;
			memset( &create_journal_data, 0, sizeof( create_journal_data ) );
			CHECK_SYS( DeviceIoControl( volume.handle, FSCTL_CREATE_USN_JOURNAL, &create_journal_data, sizeof( create_journal_data ), NULL, 0, &bytes_ret, NULL ) );
			CHECK_SYS( DeviceIoControl( volume.handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journal_data, sizeof( journal_data ), &bytes_ret, NULL ) );
			is_journal_created = true;
			}
		else CHECK_SYS( false );
		}
	else {
		if ( is_journal_used( ) && usn_journal_id != journal_data.UsnJournalID ) {
			is_journal_created = false;
			}
		}
	usn_journal_id = journal_data.UsnJournalID;
	next_usn = journal_data.NextUsn;
	}

void FilePanel::delete_usn_journal( ) {
	if ( !g_file_panel_mode.use_usn_journal || !g_file_panel_mode.delete_usn_journal ) {
		return;
		}
	if ( g_file_panel_mode.delete_own_usn_journal && !is_journal_created ) {
		return;
		}
	if ( is_journal_used( ) ) {
		DWORD bytes_ret;
		if ( g_file_panel_mode.delete_own_usn_journal ) {
			USN_JOURNAL_DATA journal_data;
			if ( !DeviceIoControl( volume.handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journal_data, sizeof( journal_data ), &bytes_ret, NULL ) ) {
				return;
				}
			if ( journal_data.UsnJournalID != usn_journal_id ) {
				return;
				}
			}
		DELETE_USN_JOURNAL_DATA delete_journal_data;
		delete_journal_data.UsnJournalID = usn_journal_id;
		delete_journal_data.DeleteFlags = USN_DELETE_FLAG_DELETE;
		DeviceIoControl( volume.handle, FSCTL_DELETE_USN_JOURNAL, &delete_journal_data, sizeof( delete_journal_data ), NULL, 0, &bytes_ret, NULL );
		usn_journal_id = 0;
		}
	}

void FilePanel::create_mft_index( ) {
	prepare_usn_journal( );

	class VolumeListProgress : public ProgressMonitor {
		protected:
		//virtual void do_update_ui( ) {
		//	const unsigned c_client_xs = 60;
		//	std::vector<std::string> lines;
		//	//lines += center(std::string::format(far_get_msg(MSG_FILE_PANEL_READ_VOLUME_PROGRESS_MESSAGE).data(), count), c_client_xs);
		//	unsigned len1 = static_cast< unsigned >( curr_file_index * c_client_xs / max_file_index );
		//	if ( len1 > c_client_xs ) {
		//		len1 = c_client_xs;
		//		}
		//	unsigned len2 = c_client_xs - len1;
		//	lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
		//	//draw_text_box(far_get_msg(MSG_FILE_PANEL_READ_VOLUME_PROGRESS_TITLE), lines, c_client_xs);
		//	//SetConsoleTitleW(std::string::format(far_get_msg(MSG_FILE_PANEL_READ_VOLUME_PROGRESS_CONSOLE_TITLE).data(), curr_file_index * 100 / max_file_index).data());
		//	//far_set_progress_state( TBPF_NORMAL );
		//	//far_set_progress_value( curr_file_index, max_file_index );
		//	}
		public:
		unsigned count;
		u64 max_file_index;
		u64 curr_file_index;
		VolumeListProgress( ) : ProgressMonitor( true ), count( 0 ) { }
		};
	VolumeListProgress progress;

	FileInfo file_info;
	file_info.volume = &volume;
	volume.synced = false;
	u64 max_file_index = file_info.load_base_file_rec( volume.mft_size / volume.file_rec_size - 1 );
	std::list<FileRecord> file_list;
	progress.max_file_index = max_file_index;

	if ( g_file_panel_mode.backward_mft_scan ) {
		u64 file_index = max_file_index + 1;
		do {
			file_index--;

			progress.curr_file_index = max_file_index - file_index;
			//progress.update_ui( );

			file_index = file_info.load_base_file_rec( file_index );

			if ( file_info.base_mft_rec( )->base_mft_record == 0 ) {
				file_info.process_base_file_rec( );
				add_file_records( file_list, file_info );
				progress.count++;
				}
			}
		while ( file_index != 0 );
		}
	else {
		for ( u64 file_index = 0; file_index <= max_file_index; file_index++ ) {
			progress.curr_file_index = file_index;
			//progress.update_ui( );

			if ( ( file_index == file_info.load_base_file_rec( file_index ) ) && ( file_info.base_mft_rec( )->base_mft_record == 0 ) ) {
				file_info.process_base_file_rec( );
				add_file_records( file_list, file_info );
				progress.count++;
				}
			}
		}

	try {
		mft_index.clear( ).extend( static_cast< unsigned >( file_list.size( ) ) );
		for ( std::list<FileRecord>::const_iterator file_rec = file_list.begin( ); file_rec != file_list.end( ); file_rec++ ) {
			mft_index += *file_rec;
			}
		mft_index.sort<FileRecordCompare>( );
		root_dir_ref_num = mft_find_root( );
		}
	catch ( ... ) {
		invalidate_mft_index( );
		throw;
		}
	}

void FilePanel::update_mft_index_from_usn( ) {
	class Progress : public ProgressMonitor {
		protected:
		//virtual void do_update_ui( ) {
		//	const unsigned c_client_xs = 60;
		//	std::vector<std::string> lines;
		//	unsigned len1 = static_cast< unsigned >( current * c_client_xs / total );
		//	if ( len1 > c_client_xs ) {
		//		len1 = c_client_xs;
		//		}
		//	unsigned len2 = c_client_xs - len1;
		//	lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
		//	//draw_text_box( far_get_msg( MSG_FILE_PANEL_UPDATE_CACHE_PROGRESS_TITLE ), lines, c_client_xs );
		//	//SetConsoleTitleW( std::string::format( far_get_msg( MSG_FILE_PANEL_UPDATE_CACHE_PROGRESS_CONSOLE_TITLE ).data( ), static_cast< unsigned >( current * 100 / total ) ).data( ) );
		//	//far_set_progress_state( TBPF_NORMAL );
		//	//far_set_progress_value( current, total );
		//	}
		public:
		u64 current, total;
		Progress( ) : ProgressMonitor( true ), current( 0 ) { }
		};
	Progress progress;

	READ_USN_JOURNAL_DATA read_usn_data;
	read_usn_data.StartUsn = next_usn;
	read_usn_data.ReasonMask = 0xFFFFFFFF;
	read_usn_data.ReturnOnlyOnClose = FALSE;
	read_usn_data.Timeout = 0;
	read_usn_data.BytesToWaitFor = 0;
	read_usn_data.UsnJournalID = usn_journal_id;

	std::set<u64> upd_file_refs;
	Array<unsigned char> usn_buffer;
	const unsigned c_usn_buffer_size = 0x1000;
	while ( true ) {
		DWORD bytes_ret;
		CHECK_SYS( DeviceIoControl( volume.handle, FSCTL_READ_USN_JOURNAL, &read_usn_data, sizeof( read_usn_data ), usn_buffer.buf( c_usn_buffer_size ), c_usn_buffer_size, &bytes_ret, NULL ) );
		usn_buffer.set_size( bytes_ret );
		if ( usn_buffer.size( ) < sizeof( USN ) ) {
			break;
			}
		read_usn_data.StartUsn = *reinterpret_cast< const USN* >( usn_buffer.data( ) );
		if ( usn_buffer.size( ) == sizeof( USN ) ) {
			break;
			}
		unsigned pos = sizeof( USN );
		while ( pos < usn_buffer.size( ) ) {
			const USN_RECORD* usn_rec = reinterpret_cast< const USN_RECORD* >( usn_buffer.data( ) + pos );
			upd_file_refs.insert( FILE_REF( usn_rec->FileReferenceNumber ) );
			pos += usn_rec->RecordLength;
			}
		}
	if ( upd_file_refs.size( ) == 0 ) {
		return;
		}

	progress.total = upd_file_refs.size( );
	std::list<FileRecord> file_list;
	FileInfo file_info;
	file_info.volume = &volume;
	volume.synced = false;
	for ( std::set<u64>::const_iterator file_index = upd_file_refs.begin( ); file_index != upd_file_refs.end( ); file_index++ ) {
		DBG_LOG( std::string::format( L"mft_update_index(): %Lx", *file_index ) );
		progress.current++;
		progress.update_ui( );
		if ( ( *file_index == file_info.load_base_file_rec( *file_index ) ) && ( file_info.base_mft_rec( )->base_mft_record == 0 ) ) {
			file_info.process_base_file_rec( );
			add_file_records( file_list, file_info );
			}
		}

	try {
		next_usn = read_usn_data.StartUsn;

		unsigned i = 0;
		while ( i < mft_index.size( ) ) {
			if ( upd_file_refs.count( mft_index[ i ].file_ref_num ) ) {
				// "fast" remove
				std::swap( mft_index.item( i ), mft_index.last_item( ) );
				mft_index.remove( mft_index.size( ) - 1 );
				}
			else i++;
			}

		mft_index.extend( mft_index.size( ) + static_cast< unsigned >( file_list.size( ) ) );
		for ( std::list<FileRecord>::const_iterator file_rec = file_list.begin( ); file_rec != file_list.end( ); file_rec++ ) {
			mft_index += *file_rec;
			}
		mft_index.sort<FileRecordCompare>( );
		root_dir_ref_num = mft_find_root( );
		}
	catch ( ... ) {
		invalidate_mft_index( );
		throw;
		}
	}

void FilePanel::mft_scan_dir( u64 parent_file_index, const std::string& rel_path, std::list<PanelItemData>& pid_list, FileListProgress& progress ) {
	progress.update_ui( );
	struct ParentFileIndexCompare {
		int operator()( u64 item1, const FileRecord& item2 ) {
			if ( item1 > item2.parent_ref_num ) {
				return 1;
				}
			else if ( item1 == item2.parent_ref_num ) {
				return 0;
				}
			else {
				return -1;
				}
			}
		};
	unsigned idx = mft_index.bsearch<ParentFileIndexCompare>( parent_file_index );
	if ( idx == -1 ) {
		return; // empty dir
		}
	while ( ( idx != 0 ) && ( mft_index[ idx - 1 ].parent_ref_num == parent_file_index ) ) {
		idx--; // find first item
		}
	while ( ( idx < mft_index.size( ) ) && ( mft_index[ idx ].parent_ref_num == parent_file_index ) ) {
		const FileRecord& file_rec = mft_index[ idx ];
		PanelItemData pid;
		if ( rel_path.size( ) != 0 ) {
			pid.file_name = rel_path + L'\\' + file_rec.file_name;
			}
		else {
			pid.file_name = file_rec.file_name;
			}
		pid.alt_file_name.clear( );
		pid.file_attr = file_rec.file_attr;
		pid.creation_time = file_rec.creation_time;
		pid.last_access_time = file_rec.last_access_time;
		pid.last_write_time = file_rec.last_write_time;
		pid.data_size = file_rec.data_size;
		pid.disk_size = file_rec.disk_size;
		pid.valid_size = file_rec.valid_size;
		pid.fragment_cnt = file_rec.fragment_cnt;
		pid.stream_cnt = file_rec.stream_cnt;
		pid.hard_link_cnt = file_rec.hard_link_cnt;
		pid.mft_rec_cnt = file_rec.mft_rec_cnt;
		pid.error = false;
		pid.ntfs_attr = file_rec.ntfs_attr( );
		pid.resident = file_rec.resident( );
		pid_list.push_back( pid );

		progress.count++;

		if ( flat_mode && ( file_rec.file_attr & FILE_ATTRIBUTE_DIRECTORY ) && ( file_rec.file_ref_num != root_dir_ref_num ) ) {
			mft_scan_dir( file_rec.file_ref_num, pid.file_name, pid_list, progress );
			}

		idx++;
		}
	}

u64 FilePanel::mft_find_root( ) const {
	for ( unsigned i = 0; i < mft_index.size( ); i++ ) {
		if ( mft_index[ i ].file_ref_num == mft_index[ i ].parent_ref_num ) {
			return mft_index[ i ].file_ref_num;
			}
		}
	FAIL( SystemError( ERROR_FILE_NOT_FOUND ) );
	}

u64 FilePanel::mft_find_path( const std::string& path ) {
	std::vector<std::string> path_parts = split_str( remove_path_root( del_trailing_slash( path ) ), L'\\' );
	u64 file_ref_num = root_dir_ref_num;
	for ( unsigned i = 0; i < path_parts.size( ); i++ ) {
		FileRecord fr;
		fr.parent_ref_num = file_ref_num;
		fr.file_name = path_parts[ i ];
		unsigned idx = mft_index.bsearch<FileRecordCompare>( fr );
		if ( idx == -1 ) {
			FAIL( SystemError( ERROR_FILE_NOT_FOUND ) );
			}
		file_ref_num = mft_index[ idx ].file_ref_num;
		}
	return file_ref_num;
	}

void FilePanel::open_volume( const std::string& dir ) {
	volume.open( extract_path_root( dir ) );
	invalidate_mft_index( );
	prepare_usn_journal( );
	if ( g_file_panel_mode.use_usn_journal && is_journal_used( ) && g_file_panel_mode.use_cache ) {
		try {
			load_mft_index( );
			update_mft_index_from_usn( );
			}
		catch ( ... ) {
			invalidate_mft_index( );
			}
		}
	if ( mft_index.size( ) == 0 ) {
		create_mft_index( );
		}
	}

void FilePanel::toggle_mft_mode( ) {
	if ( !mft_mode ) {
		current_dir = get_real_path( current_dir );
		if ( is_root_path( current_dir ) ) {
			current_dir = add_trailing_slash( current_dir );
			}
		open_volume( current_dir );
		mft_find_path( current_dir );
		mft_mode = true;
		}
	else {
		mft_mode = false;
		if ( g_file_panel_mode.use_usn_journal && is_journal_used( ) && g_file_panel_mode.use_cache ) {
			try {
				store_mft_index( );
				}
			catch ( ... ) {
				}
			}
		mft_index.clear( ).compact( );
		volume.open( extract_path_root( get_real_path( current_dir ) ) );
		}
	}

void FilePanel::reload_mft( ) {
	if ( mft_mode ) {
		create_mft_index( );
		}
	}

void FilePanel::reload_mft_all( ) {
	for ( unsigned i = 0; i < g_file_panels.size( ); i++ ) {
		g_file_panels[ i ]->reload_mft( );
		}
	}

const u8 c_cache_version = 0;

void FilePanel::store_mft_index( ) {
	if ( mft_index.size( ) == 0 ) {
		return;
		}

	//class Progress : public ProgressMonitor {
	//	protected:
	//	virtual void do_update_ui( ) {
	//		const unsigned c_client_xs = 60;
	//		std::vector<std::string> lines;
	//		unsigned len1 = static_cast< unsigned >( percent * c_client_xs / 100 );
	//		if ( len1 > c_client_xs ) {
	//			len1 = c_client_xs;
	//			}
	//		unsigned len2 = c_client_xs - len1;
	//		lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
	//		//draw_text_box(far_get_msg(MSG_FILE_PANEL_WRITE_CACHE_PROGRESS_TITLE), lines, c_client_xs);
	//		//SetConsoleTitleW(std::string::format(far_get_msg(MSG_FILE_PANEL_WRITE_CACHE_PROGRESS_CONSOLE_TITLE).data(), percent).data());
	//		far_set_progress_state( TBPF_NORMAL );
	//		far_set_progress_value( percent, 100 );
	//		}
	//	public:
	//	unsigned percent;
	//	Progress( ) : ProgressMonitor( true ), percent( 0 ) { }
	//	};
	Progress progress;

	u32 buffer_size = sizeof( usn_journal_id ) + sizeof( next_usn ) + sizeof( unsigned );
	unsigned file_record_size = sizeof( mft_index[ 0 ].file_ref_num ) + sizeof( mft_index[ 0 ].parent_ref_num ) + sizeof( mft_index[ 0 ].file_attr ) + sizeof( mft_index[ 0 ].creation_time ) + sizeof( mft_index[ 0 ].last_access_time ) + sizeof( mft_index[ 0 ].last_write_time ) + sizeof( mft_index[ 0 ].data_size ) + sizeof( mft_index[ 0 ].disk_size ) + sizeof( mft_index[ 0 ].valid_size ) + sizeof( mft_index[ 0 ].fragment_cnt ) + sizeof( mft_index[ 0 ].mft_rec_cnt ) + sizeof( mft_index[ 0 ].stream_cnt ) + sizeof( mft_index[ 0 ].hard_link_cnt ) + sizeof( mft_index[ 0 ].flags );
	buffer_size += file_record_size * mft_index.size( );
	for ( unsigned i = 0; i < mft_index.size( ); i++ ) {
		buffer_size += sizeof( unsigned ) + mft_index[ i ].file_name.size( ) * sizeof( mft_index[ i ].file_name[ 0 ] );
		}
	std::vector<unsigned char> buffer;
	buffer.reserve( buffer_size );
//#define ENCODE(var) buffer.add(reinterpret_cast<const unsigned char*>(&var), sizeof(var));


	#define ENCODE(var) buffer.push_back( ( (unsigned char) (var) ) );
	ENCODE( usn_journal_id );
	ENCODE( next_usn );
	unsigned count = mft_index.size( );
	ENCODE( count );
	for ( unsigned i = 0; i < mft_index.size( ); i++ ) {
		//progress.percent = i * 30 / count;
		//progress.update_ui( );

		ENCODE( mft_index[ i ].file_ref_num );
		ENCODE( mft_index[ i ].parent_ref_num );
		unsigned file_name_size = mft_index[ i ].file_name.size( );
		ENCODE( file_name_size );
		buffer.add( reinterpret_cast< const unsigned char* >( mft_index[ i ].file_name.data( ) ), mft_index[ i ].file_name.size( ) * sizeof( mft_index[ i ].file_name[ 0 ] ) );
		ENCODE( mft_index[ i ].file_attr );
		ENCODE( mft_index[ i ].creation_time );
		ENCODE( mft_index[ i ].last_access_time );
		ENCODE( mft_index[ i ].last_write_time );
		ENCODE( mft_index[ i ].data_size );
		ENCODE( mft_index[ i ].disk_size );
		ENCODE( mft_index[ i ].valid_size );
		ENCODE( mft_index[ i ].fragment_cnt );
		ENCODE( mft_index[ i ].mft_rec_cnt );
		ENCODE( mft_index[ i ].stream_cnt );
		ENCODE( mft_index[ i ].hard_link_cnt );
		ENCODE( mft_index[ i ].flags );
		}
	//assert( buffer.size( ) == buffer_size );

	//Array<unsigned char> comp_buffer;
	u32 comp_buffer_size = buffer.size( ) + buffer.size( ) / 16 + 64 + 3;
	//comp_buffer.extend( comp_buffer_size );
	//Array<unsigned char> comp_work_buffer;
	//comp_work_buffer.extend( LZO1X_1_MEM_COMPRESS );
	//lzo_uint sz = comp_buffer_size;
	//if ( lzo1x_1_compress( buffer.buf( ), buffer.size( ), comp_buffer.buf( ), &sz, comp_work_buffer.buf( ) ) != LZO_E_OK ) {
	//	FAIL( MsgError( L"Compressor failure" ) );
	//	}
	//comp_buffer_size = static_cast< unsigned >( sz );
	//comp_buffer.set_size( comp_buffer_size );

	//progress.percent = 70;
	//progress.update_ui( );

	//lzo_uint32 header_checksum = lzo_crc32( 0, reinterpret_cast< const lzo_bytep >( &c_cache_version ), sizeof( c_cache_version ) );
	//header_checksum = lzo_crc32( header_checksum, reinterpret_cast< const lzo_bytep >( &buffer_size ), sizeof( buffer_size ) );
	//header_checksum = lzo_crc32( header_checksum, reinterpret_cast< const lzo_bytep >( &comp_buffer_size ), sizeof( comp_buffer_size ) );
	//lzo_uint32 comp_buffer_checksum = lzo_crc32( 0, comp_buffer.data( ), comp_buffer.size( ) );
	std::string mft_index_cache_name = get_mft_index_cache_name( );
	auto mft_index_cache_name_char = mft_index_cache_name.c_str( );
	HANDLE h_file = CreateFileW( ( (LPCWSTR) (mft_index_cache_name_char)), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	CHECK_SYS( h_file != INVALID_HANDLE_VALUE );
	CLEAN( HANDLE, h_file, CloseHandle( h_file ) );
	CHECK_SYS( SetFilePointer( h_file, sizeof( header_checksum ) + sizeof( buffer_size ) + sizeof( comp_buffer_size ) + sizeof( comp_buffer_checksum ) + comp_buffer.size( ), NULL, FILE_BEGIN ) != INVALID_SET_FILE_POINTER );
	CHECK_SYS( SetEndOfFile( h_file ) );
	CHECK_SYS( SetFilePointer( h_file, 0, NULL, FILE_BEGIN ) != INVALID_SET_FILE_POINTER );
	DWORD bw;
	//CHECK_SYS( WriteFile( h_file, &header_checksum, sizeof( header_checksum ), &bw, NULL ) );
	CHECK_SYS( WriteFile( h_file, &c_cache_version, sizeof( c_cache_version ), &bw, NULL ) );
	CHECK_SYS( WriteFile( h_file, &buffer_size, sizeof( buffer_size ), &bw, NULL ) );
	//CHECK_SYS( WriteFile( h_file, &comp_buffer_size, sizeof( comp_buffer_size ), &bw, NULL ) );
	//CHECK_SYS( WriteFile( h_file, &comp_buffer_checksum, sizeof( comp_buffer_checksum ), &bw, NULL ) );
	//CHECK_SYS( WriteFile( h_file, comp_buffer.data( ), comp_buffer.size( ), &bw, NULL ) );

	//progress.percent = 100;
	//progress.update_ui( );
	}

void FilePanel::load_mft_index( ) {
	class Progress : public ProgressMonitor {
		protected:
		virtual void do_update_ui( ) {
			const unsigned c_client_xs = 60;
			std::vector<std::string> lines;
			unsigned len1 = static_cast< unsigned >( percent * c_client_xs / 100 );
			if ( len1 > c_client_xs ) {
				len1 = c_client_xs;
				}
			unsigned len2 = c_client_xs - len1;
			lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
			//draw_text_box( far_get_msg( MSG_FILE_PANEL_READ_CACHE_PROGRESS_TITLE ), lines, c_client_xs );
			//SetConsoleTitleW( std::string::format( far_get_msg( MSG_FILE_PANEL_READ_CACHE_PROGRESS_CONSOLE_TITLE ).data( ), percent ).data( ) );
			far_set_progress_state( TBPF_NORMAL );
			far_set_progress_value( percent, 100 );
			}
		public:
		unsigned percent;
		Progress( ) : ProgressMonitor( true ), percent( 0 ) { }
		};
	Progress progress;

	const wchar_t* c_corrupted_msg = L"Corrupted cache file";
	const wchar_t* c_wrong_version_msg = L"Wrong cache file version";
	HANDLE h_file = CreateFileW( get_mft_index_cache_name( ).data( ), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	CHECK_SYS( h_file != INVALID_HANDLE_VALUE );
	CLEAN( HANDLE, h_file, CloseHandle( h_file ) );

	u8 cache_version;
	u32 buffer_size;
	u32 comp_buffer_size;
	lzo_uint32 saved_header_checksum, saved_comp_buffer_checksum;
	DWORD br;
	CHECK_SYS( ReadFile( h_file, &saved_header_checksum, sizeof( saved_header_checksum ), &br, NULL ) );
	if ( br != sizeof( saved_header_checksum ) ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	CHECK_SYS( ReadFile( h_file, &cache_version, sizeof( cache_version ), &br, NULL ) );
	if ( br != sizeof( cache_version ) ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	CHECK_SYS( ReadFile( h_file, &buffer_size, sizeof( buffer_size ), &br, NULL ) );
	if ( br != sizeof( buffer_size ) ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	CHECK_SYS( ReadFile( h_file, &comp_buffer_size, sizeof( comp_buffer_size ), &br, NULL ) );
	if ( br != sizeof( comp_buffer_size ) ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}

	lzo_uint32 header_checksum = lzo_crc32( 0, reinterpret_cast< const lzo_bytep >( &cache_version ), sizeof( cache_version ) );
	header_checksum = lzo_crc32( header_checksum, reinterpret_cast< const lzo_bytep >( &buffer_size ), sizeof( buffer_size ) );
	header_checksum = lzo_crc32( header_checksum, reinterpret_cast< const lzo_bytep >( &comp_buffer_size ), sizeof( comp_buffer_size ) );
	if ( header_checksum != saved_header_checksum ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	if ( cache_version != c_cache_version ) {
		FAIL( MsgError( c_wrong_version_msg ) );
		}

	CHECK_SYS( ReadFile( h_file, &saved_comp_buffer_checksum, sizeof( saved_comp_buffer_checksum ), &br, NULL ) );
	if ( br != sizeof( saved_comp_buffer_checksum ) ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	Array<unsigned char> comp_buffer;
	comp_buffer.extend( comp_buffer_size );
	CHECK_SYS( ReadFile( h_file, comp_buffer.buf( ), comp_buffer_size, &br, NULL ) );
	if ( br != comp_buffer_size ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	comp_buffer.set_size( comp_buffer_size );

	lzo_uint32 comp_buffer_checksum = lzo_crc32( 0, comp_buffer.data( ), comp_buffer.size( ) );
	if ( comp_buffer_checksum != saved_comp_buffer_checksum ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}

	progress.percent = 30;
	progress.update_ui( );

	std::vector<unsigned char> buffer;
	buffer.reserve( buffer_size + 3 );
#ifdef _M_X64
#  define decompress lzo1x_decompress
#else
#  define decompress lzo1x_decompress_asm_fast
#endif
	lzo_uint sz = buffer_size;
	if ( decompress( comp_buffer.data( ), comp_buffer_size, buffer.buf( ), &sz, NULL ) != LZO_E_OK ) {
		FAIL( MsgError( c_corrupted_msg ) );
		}
	assert( sz == buffer_size );
	buffer.set_size( buffer_size );

	progress.percent = 70;
	progress.update_ui( );

	try {
#define DECODE(var) memcpy(&var, buffer.data() + pos, sizeof(var)); pos += sizeof(var);
		unsigned pos = 0;
		DECODE( usn_journal_id );
		DECODE( next_usn );
		unsigned count;
		DECODE( count );
		mft_index.reserve( count );
		FileRecord rec;
		unsigned file_name_size;
		for ( unsigned i = 0; i < count; i++ ) {
			progress.percent = 70 + i * 30 / count;
			progress.update_ui( );

			DECODE( rec.file_ref_num );
			DECODE( rec.parent_ref_num );
			DECODE( file_name_size );
			rec.file_name = std::string( reinterpret_cast< const wchar_t* >( buffer.data( ) + pos ), file_name_size );
			pos += file_name_size * sizeof( wchar_t );
			DECODE( rec.file_attr );
			DECODE( rec.creation_time );
			DECODE( rec.last_access_time );
			DECODE( rec.last_write_time );
			DECODE( rec.data_size );
			DECODE( rec.disk_size );
			DECODE( rec.valid_size );
			DECODE( rec.fragment_cnt );
			DECODE( rec.mft_rec_cnt );
			DECODE( rec.stream_cnt );
			DECODE( rec.hard_link_cnt );
			DECODE( rec.flags );
			mft_index += rec;
			}
		assert( pos == buffer_size );
		root_dir_ref_num = mft_find_root( );
		}
	catch ( ... ) {
		invalidate_mft_index( );
		throw;
		}
	}

std::string FilePanel::get_mft_index_cache_name( ) {
	std::string cache_dir;
	unsigned cache_dir_size = MAX_PATH;
	cache_dir_size = ExpandEnvironmentStringsW( g_file_panel_mode.cache_dir.data( ), cache_dir.buf( cache_dir_size ), cache_dir_size );
	if ( cache_dir_size > MAX_PATH ) {
		cache_dir_size = ExpandEnvironmentStringsW( g_file_panel_mode.cache_dir.data( ), cache_dir.buf( cache_dir_size ), cache_dir_size );
		}
	CHECK_SYS( cache_dir_size != 0 );
	cache_dir.set_size( cache_dir_size - 1 );
	return add_trailing_slash( cache_dir ) + get_volume_guid( volume.name ) + L".ntfsfile";
	}

FilePanel::Totals FilePanel::mft_get_totals( const std::vector<std::string>& file_list ) {
	std::set<u64> file_set;
	for ( unsigned i = 0; i < file_list.size( ); i++ ) {
		file_set.insert( mft_find_path( file_list[ i ] ) );
		}

	std::map<u64, unsigned> file_ptrs;
	std::vector<bool> tmp_array;
	bool* untested = tmp_array.buf( mft_index.size( ) );

	// find $BadClus
	FileRecord fr;
	fr.parent_ref_num = root_dir_ref_num;
	fr.file_name = L"$BadClus";
	unsigned bad_clus_ref_num = mft_index.bsearch<FileRecordCompare>( fr );

	for ( unsigned i = 0; i < mft_index.size( ); i++ ) {
		if ( mft_index[ i ].ntfs_attr( ) ) {
			untested[ i ] = false; // do not count streams
			}
		else if ( i == bad_clus_ref_num ) {
			untested[ i ] = false; // do not count $BadClus
			}
		else if ( file_set.count( mft_index[ i ].file_ref_num ) ) {
			file_ptrs.insert( std::pair<u64, unsigned>( mft_index[ i ].file_ref_num, i ) );
			untested[ i ] = false;
			}
		else {
			untested[ i ] = true;
			}
		}

	while ( true ) {
		size_t size = file_ptrs.size( );
		for ( unsigned i = 0; i < mft_index.size( ); i++ ) {
			if ( untested[ i ] && file_ptrs.count( mft_index[ i ].parent_ref_num ) ) {
				untested[ i ] = false;
				file_ptrs.insert( std::pair<u64, unsigned>( mft_index[ i ].file_ref_num, i ) );
				}
			}
		if ( size == file_ptrs.size( ) ) {
			break; // no change in size -> all files are found
			}
		}

	Totals totals;
	for ( std::map<u64, unsigned>::const_iterator i = file_ptrs.begin( ); i != file_ptrs.end( ); i++ ) {
		totals.data_size += mft_index[ i->second ].data_size;
		totals.disk_size += mft_index[ i->second ].disk_size;
		totals.fragment_cnt += mft_index[ i->second ].fragment_cnt;
		if ( mft_index[ i->second ].file_attr & FILE_ATTRIBUTE_DIRECTORY ) {
			totals.dir_cnt++;
			if ( mft_index[ i->second ].file_attr & FILE_ATTRIBUTE_REPARSE_POINT ) {
				totals.dir_rp_cnt++;
				}
			}
		else {
			totals.file_cnt++;
			if ( mft_index[ i->second ].file_attr & FILE_ATTRIBUTE_REPARSE_POINT ) {
				totals.file_rp_cnt++;
				}
			}
		if ( mft_index[ i->second ].hard_link_cnt > 1 ) {
			totals.hl_cnt++;
			}
		}
	return totals;
	}