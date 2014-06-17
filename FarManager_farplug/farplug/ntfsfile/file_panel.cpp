#define _ERROR_WINDOWS
#include "error.h"

//#include "msg.h"
#include "guids.h"
#include "options.h"
#include "utils.h"
#include "dlgapi.h"
#include "ntfs.h"
#include "volume.h"
#include "ntfs_file.h"
#include "file_panel.h"

#define IS_DIR(find_data) (((find_data).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
#define IS_REPARSE(find_data) (((find_data).dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
#define IS_DOT_DIR(find_data) (IS_DIR(find_data) && ((wcscmp(find_data.cFileName, L".") == 0) || (wcscmp(find_data.cFileName, L"..") == 0)))
#define FILE_SIZE(find_data) ((((unsigned __int64) (find_data).nFileSizeHigh) << 32) + (find_data).nFileSizeLow)

extern struct PluginStartupInfo g_far;

FilePanelMode g_file_panel_mode;
//Array<FilePanel*> FilePanel::g_file_panels;
//
//PanelState save_state( HANDLE h_panel ) {
//	PanelState state;
//	PanelInfo pi = { sizeof( PanelInfo ) };
//	if ( far_control_ptr( h_panel, FCTL_GETPANELINFO, &pi ) ) {
//		state.directory = far_get_panel_dir( h_panel );
//		if ( pi.CurrentItem < pi.ItemsNumber ) {
//			PluginPanelItem* ppi = far_get_panel_item( h_panel, pi.CurrentItem );
//			if ( ppi ) {
//				state.current_file = ppi->FileName;
//				}
//			}
//		if ( pi.TopPanelItem < pi.ItemsNumber ) {
//			PluginPanelItem* ppi = far_get_panel_item( h_panel, pi.TopPanelItem );
//			if ( ppi ) {
//				state.top_panel_file = ppi->FileName;
//				}
//			}
//		state.selected_files.extend( pi.SelectedItemsNumber );
//		for ( size_t i = 0; i < pi.SelectedItemsNumber; i++ ) {
//			PluginPanelItem* ppi = far_get_selected_panel_item( h_panel, i );
//			if ( ppi ) {
//				if ( ppi->Flags & PPIF_SELECTED ) {
//					state.selected_files += ppi->FileName;
//					}
//				}
//			}
//		state.selected_files.sort( );
//		}
//	return state;
//	}
//
//void restore_state( HANDLE h_panel, const PanelState& state ) {
//	PanelInfo pi = { sizeof( PanelInfo ) };
//	if ( far_control_ptr( h_panel, FCTL_GETPANELINFO, &pi ) ) {
//		if ( state.selected_files.size( ) ) {
//			g_far.PanelControl( h_panel, FCTL_BEGINSELECTION, 0, nullptr );
//			CLEAN( HANDLE, h_panel, g_far.PanelControl( h_panel, FCTL_ENDSELECTION, 0, nullptr ) );
//			for ( size_t i = 0; i < pi.ItemsNumber; i++ ) {
//				PluginPanelItem* ppi = far_get_panel_item( h_panel, i );
//				if ( ppi ) {
//					if ( state.selected_files.bsearch( ppi->FileName ) != -1 ) {
//						g_far.PanelControl( h_panel, FCTL_SETSELECTION, i, reinterpret_cast< void* >( true ) );
//						}
//					}
//				}
//			}
//		PanelRedrawInfo pri = { sizeof( PanelRedrawInfo ) };
//		for ( size_t i = 0; i < pi.ItemsNumber; i++ ) {
//			PluginPanelItem* ppi = far_get_panel_item( h_panel, i );
//			if ( ppi ) {
//				if ( state.current_file == ppi->FileName ) {
//					pri.CurrentItem = i;
//					}
//				if ( state.top_panel_file == ppi->FileName ) {
//					pri.TopPanelItem = i;
//					}
//				}
//			}
//		far_control_ptr( h_panel, FCTL_REDRAWPANEL, &pri );
//		}
//	}
//
//FilePanel* FilePanel::open( ) {
//	FilePanel* panel = new FilePanel( );
//	try {
//		panel->flat_mode = false;
//		panel->mft_mode = false;
//		panel->saved_state = save_state( INVALID_HANDLE_VALUE );
//		if ( g_file_panel_mode.default_mft_mode ) {
//			panel->current_dir = panel->saved_state.directory;
//			panel->toggle_mft_mode( );
//			}
//		else {
//			panel->change_directory( panel->saved_state.directory, false );
//			}
//		// signal to restore selection & current item after panel is created
//		g_far.AdvControl( &c_plugin_guid, ACTL_SYNCHRO, 0, panel );
//		g_file_panels += panel;
//		}
//	catch ( ... ) {
//		delete panel;
//		throw;
//		}
//	return panel;
//	}
//
//void FilePanel::apply_saved_state( ) {
//	far_control_int( this, FCTL_SETSORTMODE, g_file_panel_mode.sort_mode );
//	far_control_int( this, FCTL_SETSORTORDER, g_file_panel_mode.reverse_sort );
//	far_control_int( this, FCTL_SETNUMERICSORT, g_file_panel_mode.numeric_sort );
//	far_control_int( this, FCTL_SETDIRECTORIESFIRST, g_file_panel_mode.sort_dirs_first );
//	restore_state( this, saved_state );
//	}
//
//void FilePanel::close( ) {
//	PanelState state = save_state( this );
//	far_set_panel_dir( INVALID_HANDLE_VALUE, state.directory );
//	restore_state( INVALID_HANDLE_VALUE, state );
//	}

//void FilePanel::on_close( ) {
//	if ( g_file_panel_mode.use_usn_journal && is_journal_used( ) && g_file_panel_mode.use_cache ) {
//		try {
//			store_mft_index( );
//			}
//		catch ( ... ) {
//			}
//		}
//	delete_usn_journal( );
//	PanelInfo pi = { sizeof( PanelInfo ) };
//	if ( far_control_ptr( this, FCTL_GETPANELINFO, &pi ) ) {
//		g_file_panel_mode.sort_mode = pi.SortMode;
//		g_file_panel_mode.reverse_sort = pi.Flags & PFLAGS_REVERSESORTORDER ? 1 : 0;
//		g_file_panel_mode.numeric_sort = pi.Flags & PFLAGS_NUMERICSORT ? 1 : 0;
//		g_file_panel_mode.sort_dirs_first = pi.Flags & PFLAGS_DIRECTORIESFIRST ? 1 : 0;
//		}
//	store_plugin_options( );
//	g_file_panels.remove( g_file_panels.search( this ) );
//	delete this;
//	}
//
//FilePanel* FilePanel::get_active_panel( ) {
//	for ( unsigned i = 0; i < g_file_panels.size( ); i++ ) {
//		PanelInfo pi = { sizeof( PanelInfo ) };
//		if ( far_control_ptr( g_file_panels[ i ], FCTL_GETPANELINFO, &pi ) ) {
//			if ( pi.Flags & PFLAGS_FOCUS ) {
//				return g_file_panels[ i ];
//				}
//			}
//		}
//	return NULL;
//	}

//void FileListProgress::do_update_ui( ) {
//	const unsigned c_client_xs = 40;
//	std::vector<std::string> lines;
//	//lines += center( std::string::format( far_get_msg( MSG_FILE_PANEL_READ_DIR_PROGRESS_MESSAGE ).data( ), count ), c_client_xs );
//	//draw_text_box( far_get_msg( MSG_FILE_PANEL_READ_DIR_PROGRESS_TITLE ), lines, c_client_xs );
//	//SetConsoleTitleW( far_get_msg( MSG_FILE_PANEL_READ_DIR_PROGRESS_TITLE ).data( ) );
//	far_set_progress_state( TBPF_INDETERMINATE );
//	}
//
//std::string& fit_col_str( std::string& str, unsigned size ) {
//	if ( str.size( ) < size ) {
//		str.extend( size );
//		unsigned cnt = size - str.size( );
//		wchar_t* buf = str.buf( );
//		wmemmove( buf + cnt, buf, str.size( ) );
//		wmemset( buf, L' ', cnt );
//		str.set_size( size );
//		}
//	else if ( str.size( ) > size ) {
//		str.set_size( size );
//		if ( size != 0 ) {
//			str.last_item( ) = L'}';
//			}
//		}
//	return str;
//	}

//PluginItemList FilePanel::create_panel_items( const std::list<PanelItemData>& pid_list, bool search_mode ) {
//	PluginItemList pi_list;
//	unsigned sz = static_cast< unsigned >( pid_list.size( ) );
//	pi_list.extend( sz );
//	pi_list.names.extend( sz * 2 );
//	pi_list.col_str.extend( sz * col_indices.size( ) );
//	pi_list.col_data.extend( sz );
//	Array<const wchar_t*> col_data;
//	col_data.extend( col_indices.size( ) );
//	for ( std::list<PanelItemData>::const_iterator pid = pid_list.begin( ); pid != pid_list.end( ); pid++ ) {
//		PluginPanelItem pi;
//		memset( &pi, 0, sizeof( pi ) );
//		pi_list.names += pid->file_name;
//		pi.FileName = const_cast< wchar_t* >( pi_list.names.last( ).data( ) );
//		if ( pid->alt_file_name.size( ) != 0 ) {
//			pi_list.names += pid->alt_file_name;
//			pi.AlternateFileName = const_cast< wchar_t* >( pi_list.names.last( ).data( ) );
//			}
//		pi.FileAttributes = pid->file_attr;
//		pi.CreationTime = pid->creation_time;
//		pi.LastAccessTime = pid->last_access_time;
//		pi.LastWriteTime = pid->last_write_time;
//		pi.FileSize = pid->data_size;
//		pi.AllocationSize = pid->disk_size;
//		pi.NumberOfLinks = pid->hard_link_cnt;
//		// custom columns
//		col_data.clear( );
//		for ( unsigned i = 0; i < col_indices.size( ); i++ ) {
//			if ( search_mode ) {
//				col_data += L"";
//				}
//			else if ( pid->error ) {
//				//col_data += far_msg_ptr( MSG_FILE_PANEL_ERROR_MARKER );
//				}
//			else {
//				switch ( col_indices[ i ] ) {
//						case 0: // data size
//							if ( ( pid->stream_cnt == 0 ) && !pid->ntfs_attr ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( format_data_size( pid->data_size, short_size_suffixes ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						case 1: // disk size
//							if ( pid->resident ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( format_data_size( pid->disk_size, short_size_suffixes ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						case 2: // fragment count
//							if ( pid->resident ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( int_to_str( pid->fragment_cnt ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						case 3: // stream count
//							if ( pid->ntfs_attr ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( int_to_str( pid->stream_cnt ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						case 4: // hard links
//							if ( pid->ntfs_attr ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( int_to_str( pid->hard_link_cnt ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						case 5: // mft record count
//							if ( pid->ntfs_attr ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( int_to_str( pid->mft_rec_cnt ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						case 6: // valid size
//							if ( ( pid->stream_cnt == 0 ) && !pid->ntfs_attr ) {
//								col_data += L"";
//								}
//							else {
//								pi_list.col_str += fit_col_str( format_data_size( pid->valid_size, short_size_suffixes ), col_sizes[ i ] );
//								col_data += pi_list.col_str.last( ).data( );
//								}
//							break;
//						default:
//							assert( false );
//					}
//				}
//			}
//		pi_list.col_data += col_data;
//		pi.CustomColumnData = ( wchar_t** ) pi_list.col_data.last( ).data( );
//		pi.CustomColumnNumber = pi_list.col_data.last( ).size( );
//		pi_list += pi;
//		}
//	return pi_list;
//	}

void FilePanel::scan_dir( const std::string& root_path, const std::string& rel_path, std::list<PanelItemData>& pid_list, FileListProgress& progress ) {
	std::string path = add_trailing_slash( root_path ) + rel_path;
	bool more = true;
	WIN32_FIND_DATAW find_data;
	HANDLE h_find = FindFirstFileW( long_path( add_trailing_slash( path ) + L"*" ).data( ), &find_data );
	try {
		if ( h_find == INVALID_HANDLE_VALUE ) {
			// special case: symlink that denies access to directory, try real path
			if ( GetLastError( ) == ERROR_ACCESS_DENIED ) {
				DWORD attr = GetFileAttributesW( long_path( path ).data( ) );
				CHECK_SYS( attr != INVALID_FILE_ATTRIBUTES );
				if ( attr & FILE_ATTRIBUTE_REPARSE_POINT ) {
					h_find = FindFirstFileW( long_path( add_trailing_slash( get_real_path( path ) ) + L"*" ).data( ), &find_data );
					}
				else CHECK_SYS( false );
				}
			}
		if ( h_find == INVALID_HANDLE_VALUE ) {
			CHECK_SYS( GetLastError( ) == ERROR_NO_MORE_FILES );
			more = false;
			}
		}
	catch ( ... ) {
		if ( flat_mode ) {
			more = false;
			}
		else throw;
		}
	ALLOC_RSRC( ; );
	while ( more ) {
		if ( !IS_DOT_DIR( find_data ) ) {
			std::string file_path = add_trailing_slash( path ) + find_data.cFileName;
			std::string rel_file_path = add_trailing_slash( rel_path ) + find_data.cFileName;

			std::string file_name = flat_mode ? rel_file_path : find_data.cFileName;
			u64 data_size = 0;
			u64 nr_disk_size = 0;
			u64 valid_size = 0;
			unsigned stream_cnt = 0;
			unsigned fragment_cnt = 0;
			unsigned hard_link_cnt = 0;
			unsigned mft_rec_cnt = 0;
			bool error = false;
			FileInfo file_info;
			volume.synced = false;
			try {
				BY_HANDLE_FILE_INFORMATION h_file_info;
				HANDLE h_file = CreateFileW( long_path( file_path ).data( ), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_POSIX_SEMANTICS, NULL );
				CHECK_SYS( h_file != INVALID_HANDLE_VALUE );
				ALLOC_RSRC( ; );
				CHECK_SYS( GetFileInformationByHandle( h_file, &h_file_info ) );
				FREE_RSRC( CloseHandle( h_file ) );

				u64 file_ref_num = ( ( u64 ) h_file_info.nFileIndexHigh << 32 ) + h_file_info.nFileIndexLow;
				file_info.volume = &volume;
				file_info.process_file( file_ref_num );

				hard_link_cnt = h_file_info.nNumberOfLinks;
				mft_rec_cnt = file_info.mft_rec_cnt;
				for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
					const AttrInfo& attr_info = file_info.attr_list[ i ];
					if ( !attr_info.resident ) {
						nr_disk_size += attr_info.disk_size;
						}
					if ( attr_info.type == AT_DATA ) {
						data_size += attr_info.data_size;
						valid_size += attr_info.valid_size;
						stream_cnt++;
						}
					if ( attr_info.fragments > 1 ) {
						fragment_cnt += ( unsigned ) ( attr_info.fragments - 1 );
						}
					}
				}
			catch ( ... ) {
				error = true;
				}

			// is file fully resident?
			bool fully_resident = true;
			for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
				if ( !file_info.attr_list[ i ].resident ) {
					fully_resident = false;
					break;
					}
				}

			PanelItemData pid;
			pid.file_name = file_name;
			pid.alt_file_name = find_data.cAlternateFileName;
			pid.file_attr = find_data.dwFileAttributes;
			pid.creation_time = find_data.ftCreationTime;
			pid.last_access_time = find_data.ftLastAccessTime;
			pid.last_write_time = find_data.ftLastWriteTime;
			pid.data_size = data_size;
			pid.disk_size = nr_disk_size;
			pid.valid_size = valid_size;
			pid.fragment_cnt = fragment_cnt;
			pid.stream_cnt = stream_cnt;
			pid.hard_link_cnt = hard_link_cnt;
			pid.mft_rec_cnt = mft_rec_cnt;
			pid.error = error;
			pid.ntfs_attr = false;
			pid.resident = fully_resident;
			pid_list.push_back( pid );

			if ( g_file_panel_mode.show_streams && !error ) {
				unsigned cnt = 0;
				bool named_data = false;
				for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
					const AttrInfo& attr = file_info.attr_list[ i ];
					if ( !attr.resident || ( attr.type == AT_DATA ) ) {
						cnt++;
						}
					if ( ( attr.type == AT_DATA ) && ( attr.name.size( ) != 0 ) ) {
						named_data = true;
						}
					}
				// multiple non-resident/data attributes or at least one named data attribute
				if ( ( cnt > 1 ) || named_data ) {
					for ( unsigned i = 0; i < file_info.attr_list.size( ); i++ ) {
						const AttrInfo& attr = file_info.attr_list[ i ];
						if ( attr.resident && ( attr.type != AT_DATA ) ) {
							continue;
							}
						if ( !g_file_panel_mode.show_main_stream && ( attr.type == AT_DATA ) && ( attr.name.size( ) == 0 ) ) {
							continue;
							}

						std::string file_name = std::string( flat_mode ? rel_file_path : find_data.cFileName ) + L":" + attr.name + L":$" + attr.type_name( );

						unsigned fragment_cnt = ( unsigned ) attr.fragments;
						if ( fragment_cnt != 0 ) {
							fragment_cnt--;
							}

						DWORD file_attr = find_data.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY & ~FILE_ATTRIBUTE_REPARSE_POINT;
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

						PanelItemData pid;
						pid.file_name = file_name;
						pid.alt_file_name = L"";
						pid.file_attr = file_attr;
						pid.creation_time = find_data.ftCreationTime;
						pid.last_access_time = find_data.ftLastAccessTime;
						pid.last_write_time = find_data.ftLastWriteTime;
						pid.data_size = attr.data_size;
						pid.disk_size = attr.disk_size;
						pid.valid_size = attr.valid_size;
						pid.fragment_cnt = fragment_cnt;
						pid.stream_cnt = 0;
						pid.hard_link_cnt = 0;
						pid.mft_rec_cnt = 0;
						pid.error = false;
						pid.ntfs_attr = true;
						pid.resident = attr.resident;
						pid_list.push_back( pid );
						}
					}
				}

			progress.count++;
			progress.update_ui( );

			if ( flat_mode && IS_DIR( find_data ) && !IS_REPARSE( find_data ) ) {
				scan_dir( root_path, rel_file_path, pid_list, progress );
				}
			}
		if ( FindNextFileW( h_find, &find_data ) == 0 ) {
			CHECK_SYS( GetLastError( ) == ERROR_NO_MORE_FILES );
			more = false;
			}
		}
	FREE_RSRC( if ( h_find != INVALID_HANDLE_VALUE ) { VERIFY( FindClose( h_find ) ) } );
	}

void FilePanel::sort_file_list( std::list<PanelItemData>& pid_list ) {
	switch ( g_file_panel_mode.custom_sort_mode ) {
			case 1:
				struct DataSizeCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.data_size < item2.data_size;
						}
					};
				pid_list.sort( DataSizeCmp( ) );
				break;
			case 2:
				struct DiskSizeCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.disk_size < item2.disk_size;
						}
					};
				pid_list.sort( DiskSizeCmp( ) );
				break;
			case 3:
				struct FragmentsCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.fragment_cnt < item2.fragment_cnt;
						}
					};
				pid_list.sort( FragmentsCmp( ) );
				break;
			case 4:
				struct StreamsCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.stream_cnt < item2.stream_cnt;
						}
					};
				pid_list.sort( StreamsCmp( ) );
				break;
			case 5:
				struct HardLinksCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.hard_link_cnt < item2.hard_link_cnt;
						}
					};
				pid_list.sort( HardLinksCmp( ) );
				break;
			case 6:
				struct MftRecordsCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.mft_rec_cnt < item2.mft_rec_cnt;
						}
					};
				pid_list.sort( MftRecordsCmp( ) );
				break;
			case 7:
				struct FragmLevelCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						if ( ( item1.fragment_cnt == 0 ) && ( item2.fragment_cnt == 0 ) ) {
							return false;
							}
						else if ( item1.fragment_cnt == 0 ) {
							return true;
							}
						else if ( item2.fragment_cnt == 0 ) {
							return false;
							}
						else {
							assert( item2.disk_size != 0 );
							double r1 = ( double ) ( item1.fragment_cnt + 1 ) / ( item2.fragment_cnt + 1 );
							double r2 = ( double ) ( item2.fragment_cnt + 1 ) / ( item1.fragment_cnt + 1 ) * item1.disk_size / item2.disk_size;
							return r1 < r2;
							}
						}
					};
				pid_list.sort( FragmLevelCmp( ) );
				break;
			case 8:
				struct ValidSizeCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.valid_size < item2.valid_size;
						}
					};
				pid_list.sort( ValidSizeCmp( ) );
				break;
			case 9:
				struct NameLenCmp {
					bool operator()( const PanelItemData& item1, const PanelItemData& item2 ) const {
						return item1.file_name.size( ) < item2.file_name.size( );
						}
					};
				pid_list.sort( NameLenCmp( ) );
				break;
		}
	}

//void FilePanel::new_file_list( PluginPanelItem*& panel_items, size_t& item_num, bool search_mode ) {
//	FileListProgress progress;
//	std::list<PanelItemData> pid_list;
//	if ( current_dir.size( ) == 0 ) {
//		file_lists += create_volume_items( );
//		}
//	else {
//		if ( mft_mode ) {
//			if ( g_file_panel_mode.use_usn_journal && is_journal_used( ) && !search_mode ) {
//				try {
//					update_mft_index_from_usn( );
//					}
//				catch ( ... ) {
//					create_mft_index( );
//					}
//				}
//			mft_scan_dir( mft_find_path( current_dir ), L"", pid_list, progress );
//			}
//		else {
//			scan_dir( current_dir, L"", pid_list, progress );
//			}
//		if ( !search_mode ) {
//			sort_file_list( pid_list );
//			}
//		file_lists += create_panel_items( pid_list, search_mode );
//		}
//	panel_items = ( PluginPanelItem* ) file_lists.last( ).data( );
//	item_num = file_lists.last( ).size( );
//	}

//void FilePanel::clear_file_list( void* file_list_ptr ) {
//	for ( unsigned i = 0; i < file_lists.size( ); i++ ) {
//		if ( file_lists[ i ].data( ) == file_list_ptr ) {
//			file_lists.remove( i );
//			return;
//			}
//		}
//	assert( false );
//	}

void FilePanel::change_directory( const std::string& target_dir, bool search_mode ) {
	std::string new_cur_dir;
	if ( target_dir == L"\\" ) { // root directory
		new_cur_dir = extract_path_root( current_dir );
		}
	else if ( target_dir == L".." ) { // parent directory
		if ( is_root_path( current_dir ) ) {
			new_cur_dir.clear( );
			}
		else {
			new_cur_dir = extract_file_path( current_dir );
			}
		}
	else if ( extract_path_root( target_dir ).size( ) != 0 ) { // absolute path
		new_cur_dir = del_trailing_slash( target_dir );
		}
	else { // subdirectory name
		new_cur_dir = add_trailing_slash( current_dir ) + target_dir;
		}
	if ( new_cur_dir.size( ) == 0 ) {
		invalidate_mft_index( );
		delete_usn_journal( );
		}
	else {
		if ( is_root_path( new_cur_dir ) ) {
			new_cur_dir = add_trailing_slash( new_cur_dir );
			}
		if ( mft_mode ) {
			if ( current_dir.size( ) == 0 ) {
				open_volume( new_cur_dir );
				}
			if ( g_file_panel_mode.use_usn_journal && is_journal_used( ) && !search_mode ) {
				try {
					update_mft_index_from_usn( );
					}
				catch ( ... ) {
					create_mft_index( );
					}
				}
			mft_find_path( new_cur_dir );
			if ( !search_mode ) {
				SetCurrentDirectoryW( new_cur_dir.data( ) );
				}
			}
		else {
			WIN32_FIND_DATAW find_data;
			HANDLE h_find = FindFirstFileW( long_path( add_trailing_slash( new_cur_dir ) + L"*" ).data( ), &find_data );
			CHECK_SYS( h_find != INVALID_HANDLE_VALUE );
			FindClose( h_find );
			volume.open( extract_path_root( get_real_path( new_cur_dir ) ) );
			SetCurrentDirectoryW( new_cur_dir.data( ) );
			}
		}
	current_dir = new_cur_dir;
	if ( g_file_panel_mode.flat_mode_auto_off ) {
		flat_mode = false;
		}
	}
//
//void FilePanel::fill_plugin_info( OpenPanelInfo* info ) {
//	info->StructSize = sizeof( OpenPanelInfo );
//	info->Flags = OPIF_ADDDOTS | OPIF_REALNAMES;
//	if ( flat_mode && !g_file_panel_mode.use_highlighting ) {
//		info->Flags |= OPIF_USEATTRHIGHLIGHTING;
//		}
//	//panel_title = far_msg_ptr( MSG_FILE_PANEL_TITLE_PREFIX );
//	std::string flags;
//	if ( flat_mode ) {
//		flags += L'*';
//		}
//	if ( mft_mode ) {
//		if ( is_journal_used( ) ) {
//			flags += L'J';
//			}
//		else {
//			flags += L'M';
//			}
//		}
//	if ( flags.size( ) ) {
//		panel_title += L'(' + flags + L')';
//		}
//	panel_title += L':';
//	info->CurDir = current_dir.data( );
//	panel_title += current_dir;
//	info->PanelTitle = panel_title.data( );
//
//	col_indices.clear( );
//	col_sizes.clear( );
//	memzero( panel_mode );
//	if ( current_dir.size( ) ) {
//		parse_column_spec( g_file_panel_mode.col_types, g_file_panel_mode.col_widths, col_types, col_widths, true );
//		parse_column_spec( g_file_panel_mode.status_col_types, g_file_panel_mode.status_col_widths, status_col_types, status_col_widths, false );
//		panel_mode.ColumnTypes = const_cast< wchar_t* >( col_types.data( ) );
//		panel_mode.ColumnWidths = const_cast< wchar_t* >( col_widths.data( ) );
//		panel_mode.ColumnTitles = const_cast< wchar_t** >( col_titles.data( ) );
//		panel_mode.StatusColumnTypes = const_cast< wchar_t* >( status_col_types.data( ) );
//		panel_mode.StatusColumnWidths = const_cast< wchar_t* >( status_col_widths.data( ) );
//		}
//	else {
//		panel_mode.ColumnTypes = L"N,C0,C1";
//		panel_mode.ColumnWidths = const_cast< wchar_t* >( col_widths.data( ) );
//		}
//	panel_mode.Flags = g_file_panel_mode.wide ? PMFLAGS_FULLSCREEN : 0;
//	info->PanelModesArray = &panel_mode;
//	info->PanelModesNumber = 1;
//	info->StartPanelMode = '0';
//	}
//
//class FilePanelModeDialog : public FarDialog {
//	private:
//	enum {
//		c_client_xs = 60
//		};
//
//	FilePanelMode& mode;
//
//	int col_types_ctrl_id;
//	int status_col_types_ctrl_id;
//	int col_widths_ctrl_id;
//	int status_col_widths_ctrl_id;
//	int wide_ctrl_id;
//	int show_streams_ctrl_id;
//	int show_main_stream_ctrl_id;
//	int sort_mode_ctrl_id;
//	int use_mft_index_ctrl_id;
//	int use_highlighting_ctrl_id;
//	int use_usn_journal_ctrl_id;
//	int use_existing_usn_journal_ctrl_id;
//	int delete_usn_journal_ctrl_id;
//	int delete_own_usn_journal_ctrl_id;
//	int use_cache_ctrl_id;
//	int default_mft_mode_ctrl_id;
//	int backward_mft_scan_ctrl_id;
//	int cache_dir_lbl_id;
//	int cache_dir_ctrl_id;
//	int flat_mode_auto_off_ctrl_id;
//	int ok_ctrl_id;
//	int cancel_ctrl_id;
//
//	static intptr_t WINAPI dialog_proc( HANDLE h_dlg, intptr_t msg, intptr_t param1, void* param2 ) {
//		BEGIN_ERROR_HANDLER;
//		FilePanelModeDialog* dlg = ( FilePanelModeDialog* ) FarDialog::get_dlg( h_dlg );
//		if ( ( msg == DN_CLOSE ) && ( param1 >= 0 ) && ( param1 != dlg->cancel_ctrl_id ) ) {
//			dlg->mode.col_types = dlg->get_text( dlg->col_types_ctrl_id );
//			dlg->mode.status_col_types = dlg->get_text( dlg->status_col_types_ctrl_id );
//			dlg->mode.col_widths = dlg->get_text( dlg->col_widths_ctrl_id );
//			dlg->mode.status_col_widths = dlg->get_text( dlg->status_col_widths_ctrl_id );
//			dlg->mode.wide = dlg->get_check( dlg->wide_ctrl_id );
//			dlg->mode.custom_sort_mode = dlg->get_list_pos( dlg->sort_mode_ctrl_id );
//			dlg->mode.show_streams = dlg->get_check( dlg->show_streams_ctrl_id );
//			dlg->mode.show_main_stream = dlg->get_check( dlg->show_main_stream_ctrl_id );
//			dlg->mode.use_highlighting = dlg->get_check( dlg->use_highlighting_ctrl_id );
//			dlg->mode.use_usn_journal = dlg->get_check( dlg->use_usn_journal_ctrl_id );
//			dlg->mode.use_existing_usn_journal = dlg->get_check( dlg->use_existing_usn_journal_ctrl_id );
//			dlg->mode.delete_usn_journal = dlg->get_check( dlg->delete_usn_journal_ctrl_id );
//			dlg->mode.delete_own_usn_journal = dlg->get_check( dlg->delete_own_usn_journal_ctrl_id );
//			dlg->mode.use_cache = dlg->get_check( dlg->use_cache_ctrl_id );
//			dlg->mode.default_mft_mode = dlg->get_check( dlg->default_mft_mode_ctrl_id );
//			dlg->mode.backward_mft_scan = dlg->get_check( dlg->backward_mft_scan_ctrl_id );
//			dlg->mode.cache_dir = dlg->get_text( dlg->cache_dir_ctrl_id );
//			dlg->mode.flat_mode_auto_off = dlg->get_check( dlg->flat_mode_auto_off_ctrl_id );
//			}
//		else if ( ( msg == DN_BTNCLICK ) && ( param1 == dlg->show_streams_ctrl_id ) ) {
//			dlg->enable( dlg->show_main_stream_ctrl_id, param2 != 0 );
//			}
//		else if ( ( msg == DN_BTNCLICK ) && ( param1 == dlg->use_usn_journal_ctrl_id ) ) {
//			dlg->enable( dlg->use_existing_usn_journal_ctrl_id, param2 != 0 );
//			dlg->enable( dlg->delete_usn_journal_ctrl_id, param2 != 0 );
//			dlg->enable( dlg->delete_own_usn_journal_ctrl_id, param2 != 0 && dlg->get_check( dlg->delete_usn_journal_ctrl_id ) );
//			dlg->enable( dlg->use_cache_ctrl_id, param2 != 0 );
//			dlg->enable( dlg->cache_dir_lbl_id, param2 != 0 && dlg->get_check( dlg->use_cache_ctrl_id ) );
//			dlg->enable( dlg->cache_dir_ctrl_id, param2 != 0 && dlg->get_check( dlg->use_cache_ctrl_id ) );
//			}
//		else if ( ( msg == DN_BTNCLICK ) && ( param1 == dlg->use_cache_ctrl_id ) ) {
//			dlg->enable( dlg->cache_dir_lbl_id, param2 != 0 );
//			dlg->enable( dlg->cache_dir_ctrl_id, param2 != 0 );
//			}
//		else if ( ( msg == DN_BTNCLICK ) && ( param1 == dlg->delete_usn_journal_ctrl_id ) ) {
//			dlg->enable( dlg->delete_own_usn_journal_ctrl_id, dlg->get_check( dlg->use_usn_journal_ctrl_id ) && param2 != 0 );
//			}
//		END_ERROR_HANDLER( ; , ; );
//		return g_far.DefDlgProc( h_dlg, msg, param1, param2 );
//		}
//
//	public:
//	//FilePanelModeDialog( FilePanelMode& mode ) : FarDialog( c_file_panel_mode_dialog_guid, far_get_msg( MSG_FILE_PANEL_MODE_TITLE ), c_client_xs ), mode( mode ) { }
//
//	bool show( ) {
//		//unsigned col_types_lbl_id = label( far_get_msg( MSG_FILE_PANEL_MODE_COL_TYPES ) );
//		pad( c_client_xs / 2 );
//		//unsigned status_col_types_lbl_id = label( far_get_msg( MSG_FILE_PANEL_MODE_STATUS_COL_TYPES ) );
//		new_line( );
//		col_types_ctrl_id = var_edit_box( mode.col_types, c_client_xs / 2 - 1 );
//		//link_label( col_types_ctrl_id, col_types_lbl_id );
//		pad( c_client_xs / 2 );
//		status_col_types_ctrl_id = var_edit_box( mode.status_col_types, c_client_xs / 2 - 1 );
//		//link_label( status_col_types_ctrl_id, status_col_types_lbl_id );
//		new_line( );
//		//unsigned col_widths_lbl_id = label( far_get_msg( MSG_FILE_PANEL_MODE_COL_WIDTHS ) );
//		pad( c_client_xs / 2 );
//		//unsigned status_col_widths_lbl_id = label( far_get_msg( MSG_FILE_PANEL_MODE_STATUS_COL_WIDTHS ) );
//		new_line( );
//		col_widths_ctrl_id = var_edit_box( mode.col_widths, c_client_xs / 2 - 1 );
//		//link_label( col_widths_ctrl_id, col_widths_lbl_id );
//		pad( c_client_xs / 2 );
//		status_col_widths_ctrl_id = var_edit_box( mode.status_col_widths, c_client_xs / 2 - 1 );
//		//link_label( status_col_widths_ctrl_id, status_col_widths_lbl_id );
//		new_line( );
//		separator( );
//		new_line( );
//
//		//wide_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_MODE_WIDE ), mode.wide );
//		new_line( );
//		separator( );
//		new_line( );
//
//		//show_streams_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_SHOW_STREAMS ), mode.show_streams );
//		spacer( 2 );
//		//show_main_stream_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_SHOW_MAIN_STREAM ), mode.show_main_stream, mode.show_streams ? 0 : DIF_DISABLE );
//		new_line( );
//		//label( far_get_msg( MSG_FILE_PANEL_MODE_SORT ) );
//		spacer( 1 );
//		std::vector<std::string> items;
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_NOTHING );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_DATA_SIZE );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_DISK_SIZE );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_FRAGMENTS );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_STREAMS );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_HARD_LINKS );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_MFT_RECORDS );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_FRAGM_LEVEL );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_VALID_SIZE );
//		//items += far_get_msg( MSG_FILE_PANEL_MODE_SORT_NAME_LEN );
//		unsigned max_size = 0;
//		for ( unsigned i = 0; i < items.size( ); i++ ) {
//			max_size = max( max_size, items[ i ].size( ) );
//			}
//		sort_mode_ctrl_id = combo_box( items, mode.custom_sort_mode, max_size + 1, DIF_DROPDOWNLIST );
//		new_line( );
//		//default_mft_mode_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_DEFAULT_MFT_MODE ), mode.default_mft_mode );
//		spacer( 2 );
//		//backward_mft_scan_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_BACKWARD_MFT_SCAN ), mode.backward_mft_scan );
//		new_line( );
//		separator( );
//		new_line( );
//		//label( far_get_msg( MSG_FILE_PANEL_FLAT_MODE_PARAMS ) );
//		new_line( );
//		//flat_mode_auto_off_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_FLAT_MODE_AUTO_OFF ), mode.flat_mode_auto_off );
//		new_line( );
//		//use_highlighting_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_USE_HIGHLIGHTING ), mode.use_highlighting );
//		new_line( );
//		separator( );
//		new_line( );
//		//use_usn_journal_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_USE_USN_JOURNAL ), mode.use_usn_journal );
//		spacer( 2 );
//		//use_existing_usn_journal_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_USE_EXISTING_USN_JOURNAL ), mode.use_existing_usn_journal, mode.use_usn_journal ? 0 : DIF_DISABLE );
//		new_line( );
//		//delete_usn_journal_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_DELETE_USN_JOURNAL ), mode.delete_usn_journal, mode.use_usn_journal ? 0 : DIF_DISABLE );
//		spacer( 2 );
//		//delete_own_usn_journal_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_DELETE_OWN_USN_JOURNAL ), mode.delete_own_usn_journal, mode.use_usn_journal && mode.delete_usn_journal ? 0 : DIF_DISABLE );
//		new_line( );
//		//use_cache_ctrl_id = check_box( far_get_msg( MSG_FILE_PANEL_USE_CACHE ), mode.use_cache, mode.use_usn_journal ? 0 : DIF_DISABLE );
//		spacer( 2 );
//		//cache_dir_lbl_id = label( far_get_msg( MSG_FILE_PANEL_CACHE_DIR ), AUTO_SIZE, mode.use_cache && mode.use_usn_journal ? 0 : DIF_DISABLE );
//		spacer( 1 );
//		cache_dir_ctrl_id = var_edit_box( mode.cache_dir, AUTO_SIZE, mode.use_cache && mode.use_usn_journal ? 0 : DIF_DISABLE );
//		new_line( );
//		separator( );
//		new_line( );
//
//		//ok_ctrl_id = def_button( far_get_msg( MSG_BUTTON_OK ), DIF_CENTERGROUP );
//		//cancel_ctrl_id = button( far_get_msg( MSG_BUTTON_CANCEL ), DIF_CENTERGROUP );
//		new_line( );
//
//		int item = FarDialog::show( dialog_proc, L"file_panel_mode" );
//
//		return ( item != -1 ) && ( item != cancel_ctrl_id );
//		}
//	};
//
//bool show_file_panel_mode_dialog( FilePanelMode& mode ) {
//	//return FilePanelModeDialog( mode ).show( );
//	return false;
//	}
//
//void FilePanel::parse_column_spec( const std::string& src_col_types, const std::string& src_col_widths, std::string& col_types, std::string& col_widths, bool title ) {
//	const wchar_t* c_col_names[ c_cust_col_cnt ] = { L"DSZ", L"RSZ", L"FRG", L"STM", L"LNK", L"MFT", L"VSZ" };
//	const unsigned c_def_col_sizes[ c_cust_col_cnt ] = { 7, 7, 5, 3, 3, 3, 7 };
//	//const wchar_t* c_col_titles[ c_cust_col_cnt ] = { far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_DATA_SIZE ), far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_DISK_SIZE ), far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_FRAGMENTS ), far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_STREAMS ), far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_HARD_LINKS ), far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_MFT_RECORDS ), far_msg_ptr( MSG_FILE_PANEL_MODE_COL_TITLE_VALID_SIZE ) };
//
//	col_types.clear( );
//	col_widths.clear( );
//	if ( title ) {
//		col_titles.clear( );
//		}
//	std::vector<std::string> col_types_lst = split_str( src_col_types, L',' );
//	std::vector<std::string> col_widths_lst = split_str( src_col_widths, L',' );
//	for ( unsigned i = 0; i < col_types_lst.size( ); i++ ) {
//		std::string name = col_types_lst[ i ];
//		unsigned size = 0;
//		if ( i < col_widths_lst.size( ) ) {
//			size = str_to_int( col_widths_lst[ i ] );
//			}
//		unsigned col_idx = -1;
//		for ( unsigned j = 0; j < c_cust_col_cnt; j++ ) {
//			if ( name == c_col_names[ j ] ) {
//				col_idx = j;
//				break;
//				}
//			}
//		if ( col_idx != -1 ) {
//			// custom column
//			if ( size == 0 ) {
//				size = c_def_col_sizes[ col_idx ];
//				}
//			unsigned idx = col_indices.search( col_idx );
//			if ( idx == -1 ) {
//				// new column
//				col_indices += col_idx;
//				col_sizes += size;
//				name = L"C" + int_to_str( col_indices.size( ) - 1 );
//				}
//			else {
//				// column is referenced several times
//				if ( col_sizes[ idx ] > size ) {
//					col_sizes.item( idx ) = size;
//					}
//				name = L"C" + int_to_str( idx );
//				}
//			if ( title ) {
//				//col_titles += c_col_titles[ col_idx ];
//				}
//			}
//		else {
//			// standard column
//			if ( title ) {
//				col_titles += NULL;
//				}
//			}
//		if ( col_types.size( ) != 0 ) {
//			col_types += L',';
//			}
//		col_types += name;
//		if ( col_widths.size( ) != 0 ) {
//			col_widths += L',';
//			}
//		col_widths += int_to_str( size );
//		}
//	}