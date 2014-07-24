#pragma once

struct PluginItemList : public Array < PluginPanelItem > {
	std::vector<std::string> names;
	std::vector<std::string> col_str;
	std::vector<Array<const wchar_t*> > col_data;
	};

struct PanelState {
	std::string directory;
	std::string current_file;
	std::string top_panel_file;
	std::vector<std::string> selected_files;
	};

class FileListProgress : public ProgressMonitor {
	protected:
	void do_update_ui( );
	public:
	unsigned count;
	FileListProgress( ) : ProgressMonitor( true ), count( 0 ) { }
	};

class FilePanel {
	private:
	enum {
		c_cust_col_cnt = 7
		};
	struct PanelItemData {
		std::string file_name;
		std::string alt_file_name;
		DWORD file_attr;
		FILETIME creation_time;
		FILETIME last_access_time;
		FILETIME last_write_time;
		u64 data_size;
		u64 disk_size;
		u64 valid_size;
		unsigned fragment_cnt;
		unsigned stream_cnt;
		unsigned hard_link_cnt;
		unsigned mft_rec_cnt;
		bool error;
		bool ntfs_attr;
		bool resident;
		};
	NtfsVolume volume;
	std::vector<PluginItemList> file_lists;
	PanelMode panel_mode;
	std::string panel_title;
	std::string col_types;
	std::string col_widths;
	std::string status_col_types;
	std::string status_col_widths;
	Array<const wchar_t*> col_titles;
	Array<unsigned> col_sizes;
	Array<unsigned> col_indices;
	PanelState saved_state;
	static Array<FilePanel*> g_file_panels;
	void parse_column_spec( const std::string& src_col_types, const std::string& src_col_widths, std::string& col_types, std::string& col_widths, bool title );
	PluginItemList create_panel_items( const std::list<PanelItemData>& pid_list, bool search_mode );
	PluginItemList create_volume_items( );
	void scan_dir( const std::string& root_path, const std::string& rel_path, std::list<PanelItemData>& pid_list, FileListProgress& progress );
	void sort_file_list( std::list<PanelItemData>& pid_list );
	struct FileRecord {
		u64 file_ref_num;
		u64 parent_ref_num;
		std::string file_name;
		DWORD file_attr;
		FILETIME creation_time;
		FILETIME last_access_time;
		FILETIME last_write_time;
		u64 data_size;
		u64 disk_size;
		u64 valid_size;
		u32 fragment_cnt;
		u32 mft_rec_cnt;
		u16 stream_cnt;
		u16 hard_link_cnt;
		u8 flags;
		bool ntfs_attr( ) const { return ( flags & 1 ) != 0; }
		bool resident( ) const { return ( flags & 2 ) != 0; }
		void set_flags( bool ntfs_attr, bool resident ) { flags = ( ntfs_attr ? 1 : 0 ) | ( resident ? 2 : 0 ); }
		};
	struct FileRecordCompare;
	DWORDLONG usn_journal_id;
	USN next_usn;
	bool is_journal_created;
	bool is_journal_used( ) const {
		return usn_journal_id != 0;
		}
	std::vector<FileRecord> mft_index;
	void invalidate_mft_index( ) {
		mft_index.clear( );
		usn_journal_id = 0;
		}
	u64 root_dir_ref_num;
	void add_file_records( std::list<FileRecord>& file_list, const FileInfo& file_info );
	void prepare_usn_journal( );
	void delete_usn_journal( );
	void create_mft_index( );
	void update_mft_index_from_usn( );
	void mft_scan_dir( u64 parent_file_index, const std::string& rel_path, std::list<PanelItemData>& pid_list, FileListProgress& progress );
	u64 mft_find_root( ) const;
	u64 mft_find_path( const std::string& path );
	void store_mft_index( );
	void load_mft_index( );
	std::string get_mft_index_cache_name( );
	void open_volume( const std::string& dir );
	FilePanel( ) : usn_journal_id( 0 ), is_journal_created( false ) { }
	public:
	std::string current_dir;
	bool flat_mode;
	bool mft_mode;
	static FilePanel* open( );
	void apply_saved_state( );
	void close( );
	void on_close( );
	static FilePanel* FilePanel::get_active_panel( );
	void new_file_list( PluginPanelItem*& panel_items, size_t& item_num, bool search_mode );
	void clear_file_list( void* file_list_ptr );
	void change_directory( const std::string& target_dir, bool search_mode );
	void fill_plugin_info( OpenPanelInfo* info );
	void toggle_mft_mode( );
	void reload_mft( );
	static void reload_mft_all( );
	struct Totals {
		u64 data_size;
		u64 disk_size;
		u64 fragment_cnt;
		unsigned file_cnt;
		unsigned dir_cnt;
		unsigned hl_cnt;
		unsigned file_rp_cnt;
		unsigned dir_rp_cnt;
		Totals( ) {
			memset( this, 0, sizeof( *this ) );
			}
		};
	Totals mft_get_totals( const std::vector<std::string>& file_list );
	};

bool show_file_panel_mode_dialog( FilePanelMode& mode );
