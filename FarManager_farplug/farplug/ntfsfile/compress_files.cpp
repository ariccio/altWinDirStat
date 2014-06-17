#define _ERROR_WINDOWS
#include "error.h"
#include <Windows.h>
#include <vector>


//#include "msg.h"
#include "guids.h"
#include "utils.h"
#include "dlgapi.h"
#include "log.h"
#include "options.h"
#include "volume.h"
#include "defragment.h"
#include "compress_files.h"

extern struct PluginStartupInfo g_far;
extern Array<FarColor> g_colors;

CompressFilesParams g_compress_files_params;

typedef NTSTATUS( NTAPI *PRtlCompressBuffer )( USHORT CompressionFormatAndEngine, PUCHAR UncompressedBuffer, ULONG UncompressedBufferSize, PUCHAR CompressedBuffer, ULONG CompressedBufferSize, ULONG UncompressedChunkSize, PULONG FinalCompressedSize, PVOID WorkSpace );
typedef NTSTATUS( NTAPI *PRtlGetCompressionWorkSpaceSize )( USHORT CompressionFormatAndEngine, PULONG CompressBufferWorkSpaceSize, PULONG CompressFragmentWorkSpaceSize );
PRtlCompressBuffer RtlCompressBuffer;
PRtlGetCompressionWorkSpaceSize RtlGetCompressionWorkSpaceSize;

void init_comp_api( ) {
	RtlCompressBuffer = reinterpret_cast< PRtlCompressBuffer >( GetProcAddress( GetModuleHandle( "ntdll" ), "RtlCompressBuffer" ) );
	CHECK_SYS( RtlCompressBuffer );
	RtlGetCompressionWorkSpaceSize = reinterpret_cast< PRtlGetCompressionWorkSpaceSize >( GetProcAddress( GetModuleHandle( "ntdll" ), "RtlGetCompressionWorkSpaceSize" ) );
	CHECK_SYS( RtlGetCompressionWorkSpaceSize );
	}

unsigned get_comp_work_buffer_size( ) {
	ULONG compress_buffer_work_space_size, compress_fragment_work_space_size;
	if ( RtlGetCompressionWorkSpaceSize( COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD, &compress_buffer_work_space_size, &compress_fragment_work_space_size ) != STATUS_SUCCESS )
	  FAIL( MsgError( L"RtlGetCompressionWorkSpaceSize" ) );
	return compress_buffer_work_space_size;
	}

unsigned get_cpu_count( ) {
	SYSTEM_INFO sys_info;
	GetSystemInfo( &sys_info );
	return sys_info.dwNumberOfProcessors;
	}

unsigned get_cluster_size( const std::string& file_name ) {
	try {
		return VolumeInfo( file_name ).cluster_size;
		}
	catch ( ... ) {
		return 4 * 1024;
		}
	}

u64 get_time( ) {
	LARGE_INTEGER li;
	CHECK_SYS( QueryPerformanceCounter( &li ) );
	return li.QuadPart;
	}

u64 get_time_freq( ) {
	LARGE_INTEGER li;
	CHECK_SYS( QueryPerformanceFrequency( &li ) );
	return li.QuadPart;
	}

enum BufState {
	bs_io_ready,
	bs_proc_ready,
	bs_processing,
	};

struct Buffer : private NonCopyable {
	BufState state;
	unsigned data_size; // valid data size in io_buffer
	u8* io_buffer; // I/O buffer
	u8* comp_buffer; // compression buffer
	u8* comp_work_buffer;  // compression work buffer
	Buffer( ) : state( bs_io_ready ), data_size( 0 ), io_buffer( NULL ), comp_buffer( NULL ), comp_work_buffer( NULL ) { }
	~Buffer( ) {
		if ( io_buffer ) {
			VirtualFree( io_buffer, 0, MEM_RELEASE );
			}
		if ( comp_buffer ) {
			delete[ ] comp_buffer;
			}
		if ( comp_work_buffer ) {
			delete[ ] comp_work_buffer;
			}
		}
	void allocate( unsigned io_buffer_size, unsigned comp_buffer_size, unsigned comp_work_buffer_size ) {
		io_buffer = static_cast< u8* >( VirtualAlloc( NULL, io_buffer_size, MEM_COMMIT, PAGE_READWRITE ) );
		CHECK_SYS( io_buffer );
		comp_buffer = new u8[ comp_buffer_size ];
		comp_work_buffer = new u8[ comp_work_buffer_size ];
		}
	};

class BufferArray : private NonCopyable {
	private:
	Buffer* buffers;
	unsigned num_buf;
	public:
	unsigned io_buffer_size; // I/O buffer size
	unsigned comp_buffer_size;
	unsigned comp_work_buffer_size;
	BufferArray( unsigned num_buf, unsigned cluster_size ) :
	  num_buf( num_buf ),
	  io_buffer_size( 16 * cluster_size ), // NTFS compression unit = 16 clusters
	  comp_buffer_size( io_buffer_size + io_buffer_size / 16 + 64 + 3 ),
	  comp_work_buffer_size( get_comp_work_buffer_size( ) ) {
		buffers = new Buffer[ num_buf ];
		try {
			for ( unsigned i = 0; i < num_buf; i++ ) {
				buffers[ i ].allocate( io_buffer_size, comp_buffer_size, comp_work_buffer_size );
				}
			}
		catch ( ... ) {
			delete[ ] buffers;
			throw;
			}
		}
	~BufferArray( ) {
		delete[ ] buffers;
		}
	Buffer* find( BufState state ) {
		for ( unsigned i = 0; i < num_buf; i++ ) {
			if ( buffers[ i ].state == state ) {
				return buffers + i;
				}
			}
		return NULL;
		}
	};

enum ProgressPhase {
	phase_enum,
	phase_estimate,
	phase_compress,
	phase_defragment,
	};

struct CompressFiles : private NonCopyable, private ProgressMonitor, public IDefragProgress {
	const CompressFilesParams& params;
	Log& log;

	unsigned num_th; // number of worker threads
	unsigned num_buf; // number of I/O buffers
	unsigned cluster_size;
	BufferArray buffers; // I/O buffers
	Event stop_event;
	CriticalSection sync;
	Semaphore io_ready_sem;
	Semaphore proc_ready_sem;

	ProgressPhase progress_phase;

	std::string current_file_name; // currently processed file name

	u64 file_comp_size; // current file compressed size
	u64 file_proc_size; // current file processed size
	u64 file_size; // current file size

	u64 total_proc_size; // total processed data size
	u64 total_size; // total file size (estimated)

	unsigned file_cnt; // number of files processed
	unsigned total_file_cnt; // total number of files (estimated)

	unsigned err_cnt; // number of files/dirs skipped because of errors

	u64 start_time; // operation start time

	std::vector<HANDLE> wait_handles; // for compress_file()

	ULONGLONG now; // current system time for file filter

	virtual void do_update_ui( );
	void update_progress( ProgressPhase phase, bool force = false ) {
		progress_phase = phase;
		update_ui( force );
		}
	virtual void update_defrag_ui( bool force ) {
		update_progress( phase_defragment, force );
		}
	u64 clustered_size( u64 size );
	void run_compression_thread( );
	bool is_file_accepted_by_filter( const FindData& find_data ) const;
	void estimate_file_size( const FindData& find_data );
	void estimate_directory_size( const std::string& dir_name );
	void compress_file( const std::string& file_name, const FindData& find_data );
	void compress_directory( const std::string& dir_name );
	CompressFiles( const CompressFilesParams& params, Log& log, unsigned num_th, unsigned cluster_size ) : ProgressMonitor( true ), params( params ), log( log ), num_th( num_th ), num_buf( num_th * 2 ), cluster_size( cluster_size ), stop_event( true, false ), io_ready_sem( num_buf, num_buf ), proc_ready_sem( 0, num_buf ), buffers( num_buf, cluster_size ) { }
	void process( const std::vector<std::string>& file_list );
	};

class WorkerThreads : private NonCopyable, public std::vector< HANDLE > {
	private:
	CompressFiles& cf;
	void stop_threads( ) {
		SetEvent( cf.stop_event.handle( ) );
		WaitForMultipleObjects( static_cast< DWORD >( size( ) ), to_array( *this ), TRUE, INFINITE );
		for ( unsigned i = 0; i < size( ); i++ ) {
			CloseHandle( at( i ) );
			}
		}
	static unsigned __stdcall thread_proc( void* wth_param ) {
		try {
			CompressFiles* cf = static_cast< CompressFiles* >( wth_param );
			cf->run_compression_thread( );
			return TRUE;
			}
		catch ( ... ) {
			return FALSE;
			}
		}
	public:
	WorkerThreads( CompressFiles& cf ) : cf( cf ) {
		reserve( cf.num_th );
		for ( unsigned i = 0; i < cf.num_th; i++ ) {
			try {
				unsigned th_id;
				HANDLE h_thread = reinterpret_cast< HANDLE >( _beginthreadex( NULL, 0, thread_proc, &cf, 0, &th_id ) );
				CHECK_SYS( h_thread );
				push_back( h_thread ); // should not fail
				}
			catch ( ... ) {
				stop_threads( );
				throw;
				}
			}
		}
	~WorkerThreads( ) {
		stop_threads( );
		}
	};

void CompressFiles::do_update_ui( ) {
	const unsigned c_client_xs = 60;
	std::vector<std::string> lines;

	if ( progress_phase == phase_enum ) {
		if ( total_size ) {
			//lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_SIZE ).data( ), &format_inf_amount_short( total_size ) );
			}
		if ( total_size && total_file_cnt ) {
			lines += L"\x1";
			}
		if ( total_file_cnt ) {
			//lines += std::string::format( far_get_msg( MSG_ESTIMATE_PROGRESS_FILES ).data( ), total_file_cnt );
			}
		//draw_text_box( far_get_msg( MSG_ESTIMATE_PROGRESS_TITLE ), lines, c_client_xs );
		//SetConsoleTitleW( far_get_msg( MSG_ESTIMATE_PROGRESS_TITLE ).data( ) );
		far_set_progress_state( TBPF_INDETERMINATE );
		}
	else if ( progress_phase == phase_estimate || progress_phase == phase_compress || progress_phase == phase_defragment ) {
		u64 local_file_comp_size, local_file_proc_size, local_total_proc_size;
			{
			CriticalSectionLock lock( sync );
			local_file_comp_size = file_comp_size;
			local_file_proc_size = file_proc_size;
			local_total_proc_size = total_proc_size;
			}

		// file name
		//std::string file_name_label( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_FILE_NAME ) );
		lines += file_name_label + ' ' + fit_str( current_file_name, c_client_xs - file_name_label.size( ) - 1 );
		lines += L"\x1";

		if ( progress_phase == phase_estimate ) {
			// file compression ratio
			unsigned compression_ratio;
			if ( local_file_proc_size == 0 ) {
				compression_ratio = 100;
				}
			else {
				compression_ratio = round( static_cast< double >( local_file_comp_size ) / local_file_proc_size * 100 );
				}
			//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_ESTIMATION ).data( ), &format_inf_amount_short( local_file_comp_size ), &format_inf_amount_short( local_file_proc_size ), compression_ratio );

			// file percent done
			unsigned file_percent_done;
			if ( file_size == 0 ) {
				file_percent_done = 100;
				}
			else {
				file_percent_done = round( static_cast< double >( local_file_proc_size ) / file_size * 100 );
				}
			if ( file_percent_done > 100 ) {
				file_percent_done = 100;
				}
			//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_ESTIMATION_SIZE ).data( ), &format_inf_amount_short( local_file_proc_size ), &format_inf_amount_short( file_size ), file_percent_done );

			// processing speed
			u64 time = get_time( ) - start_time;
			if ( time ) {
				lines.item( lines.size( ) - 1 ).add( L' ' ).add_fmt( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_ESTIMATION_SPEED ).data( ), &format_inf_amount_short( round( static_cast< double >( local_file_proc_size ) / time * get_time_freq( ) ), true ) );
				}
			// file progress bar
			if ( file_size ) {
				unsigned len1 = round( static_cast< double >( local_file_proc_size ) / file_size * c_client_xs );
				if ( len1 > c_client_xs ) {
					len1 = c_client_xs;
					}
				unsigned len2 = c_client_xs - len1;
				lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
				}
			}
		else if ( progress_phase == phase_compress ) {
			//lines += far_get_msg( MSG_COMPRESS_FILES_PROGRESS_COMPRESSION );

			u64 file_remain_size = file_size - local_file_proc_size;
			int worst_ratio = round( static_cast< double >( local_file_comp_size + file_remain_size ) / file_size * 100 );
			int best_ratio = round( static_cast< double >( local_file_comp_size + 0 ) / file_size * 100 );
			//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_COMPRESSION_RATIO ).data( ), best_ratio, worst_ratio );

			lines += std::string::format( L"%.*c", c_client_xs, c_pb_white );
			}
		else if ( progress_phase == phase_defragment ) {
			if ( total_clusters ) {
				//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_DEFRAGMENT ).data( ), extents_before, extents_after );
				//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_DEFRAGMENT_CLUSTERS ).data( ), moved_clusters, total_clusters );
				unsigned len1 = round( static_cast< double >( moved_clusters ) / total_clusters * c_client_xs );
				if ( len1 > c_client_xs ) {
					len1 = c_client_xs;
					}
				unsigned len2 = c_client_xs - len1;
				lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
				}
			else {
				//lines += far_get_msg( MSG_COMPRESS_FILES_PROGRESS_DEFRAGMENT_ANALYZE );
				lines += std::string( );
				lines += std::string::format( L"%.*c", c_client_xs, c_pb_white );
				}
			}

		lines += L"\x1";

		// total percent done
		unsigned total_percent_done;
		if ( total_size == 0 ) {
			total_percent_done = 100;
			}
		else {
			total_percent_done = round( static_cast< double >( local_total_proc_size ) / total_size * 100 );
			}
		if ( total_percent_done > 100 ) {
			total_percent_done = 100;
			}

		// number of files processed
		//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_FILES ).data( ), file_cnt, total_file_cnt );
		// total progress bar
		if ( total_size ) {
			unsigned len1 = round( static_cast< double >( local_total_proc_size ) / total_size * c_client_xs );
			if ( len1 > c_client_xs ) {
				len1 = c_client_xs;
				}
			unsigned len2 = c_client_xs - len1;
			lines += std::string::format( L"%.*c%.*c", len1, c_pb_black, len2, c_pb_white );
			}

		if ( err_cnt ) {
			lines += L"\x1";
			//lines += std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_ERRORS ).data( ), err_cnt );
			}

		//draw_text_box( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_TITLE ), lines, c_client_xs );
		//SetConsoleTitleW( std::string::format( far_get_msg( MSG_COMPRESS_FILES_PROGRESS_CONSOLE_TITLE ).data( ), total_percent_done ).data( ) );
		far_set_progress_state( TBPF_NORMAL );
		far_set_progress_value( total_percent_done, 100 );
		}
	}

//bool CompressFiles::is_file_accepted_by_filter( const FindData& find_data ) const {
//	if ( file_size < params.min_file_size * 1024 * 1024 ) {
//		return false;
//		}
//	if ( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
//		return false;
//		}
//	ULONGLONG last_write_time = ( static_cast< ULONGLONG >( find_data.ftLastWriteTime.dwHighDateTime ) << 32 ) + find_data.ftLastWriteTime.dwLowDateTime;
//	const ULONGLONG one_day = static_cast< ULONGLONG >( 10000000 ) * 60 * 60 * 24;
//	if ( last_write_time + params.min_file_age * one_day >= now ) {
//		return false;
//		}
//	return true;
//	}

void CompressFiles::estimate_file_size( const FindData& find_data ) {
	if ( is_file_accepted_by_filter( find_data ) ) {
		total_size += clustered_size( find_data.size( ) );
		total_file_cnt++;
		}
	update_progress( phase_enum );
	}

void CompressFiles::estimate_directory_size( const std::string& dir_name ) {
	std::string file_name;
	FileEnum file_enum( dir_name );
	while ( file_enum.next( ) ) {
		if ( file_enum.data( ).dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
			}
		else if ( file_enum.data( ).is_dir( ) ) {
			try {
				estimate_directory_size( add_trailing_slash( dir_name ) + file_enum.data( ).cFileName );
				}
			catch ( ... ) {
				}
			}
		else {
			estimate_file_size( file_enum.data( ) );
			}
		update_progress( phase_enum );
		}
	}

u64 CompressFiles::clustered_size( u64 size ) {
	return size / cluster_size * cluster_size + ( size % cluster_size ? cluster_size : 0 );
	}

void CompressFiles::run_compression_thread( ) {
	while ( true ) {
		HANDLE h[ 2 ] = { stop_event.handle( ), proc_ready_sem.handle( ) };
		DWORD w = WaitForMultipleObjects( 2, h, FALSE, INFINITE );
		CHECK_SYS( w != WAIT_FAILED );
		if ( w == WAIT_OBJECT_0 ) {
			break;
			}
		else if ( w == WAIT_OBJECT_0 + 1 ) {
			Buffer* buf;
				{
				CriticalSectionLock sync( sync );
				buf = buffers.find( bs_proc_ready );
				if ( buf ) {
					buf->state = bs_processing;
					}
				}

			ULONG final_compressed_size;
			NTSTATUS status = RtlCompressBuffer( COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD, buf->io_buffer, buf->data_size, buf->comp_buffer, buffers.comp_buffer_size, cluster_size, &final_compressed_size, buf->comp_work_buffer );
			if ( status != STATUS_SUCCESS && status != STATUS_BUFFER_ALL_ZEROS ) {
				FAIL( MsgError( L"RtlCompressBuffer" ) );
				}
			u64 comp_size = clustered_size( final_compressed_size );
			u64 data_size = clustered_size( buf->data_size );

				{
				CriticalSectionLock sync( sync );
				// update stats
				total_proc_size += data_size;
				file_comp_size += min( comp_size, data_size );
				file_proc_size += data_size;
				// mark buffer ready for I/O
				buf->state = bs_io_ready;
				}

			// signal I/O thread
			CHECK_SYS( ReleaseSemaphore( io_ready_sem.handle( ), 1, NULL ) );
			}
		}
	}

class ReadOnlyFileAccess {
	private:
	DWORD attr;
	const std::string& file_name;
	public:
	ReadOnlyFileAccess( const std::string& file_name ) : file_name( file_name ) {
		attr = GetFileAttributesW( long_path( file_name ).data( ) );
		CHECK_SYS( attr != INVALID_FILE_ATTRIBUTES );
		if ( attr & FILE_ATTRIBUTE_READONLY ) {
			CHECK_SYS( SetFileAttributesW( long_path( file_name ).data( ), FILE_ATTRIBUTE_NORMAL ) );
			}
		}
	~ReadOnlyFileAccess( ) {
		if ( attr & FILE_ATTRIBUTE_READONLY ) {
			SetFileAttributesW( long_path( file_name ).data( ), attr );
			}
		}
	};

void CompressFiles::compress_file( const std::string& file_name, const FindData& find_data ) {
	if ( !is_file_accepted_by_filter( find_data ) ) {
		return;
		}
	bool fatal_error = false;
	try {
		ReadOnlyFileAccess ro_access( file_name );

		File file( file_name, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN );

		current_file_name = file_name;
		file_comp_size = file_proc_size = 0;
		file_size = clustered_size( file.size( ) );
		start_time = get_time( );
		bool good_ratio = false;

		fatal_error = true;
		bool eof = false;
		while ( !eof ) {
			DWORD w = WaitForMultipleObjects( static_cast< DWORD >( wait_handles.size( ) ), to_array( wait_handles ), FALSE, INFINITE );
			CHECK_SYS( w != WAIT_FAILED );
			if ( w == WAIT_OBJECT_0 ) {
				BREAK;
				}
			else if ( w == WAIT_OBJECT_0 + 1 ) {
				Buffer* buf;
					{
					CriticalSectionLock lock( sync );
					// try to predict comp. ratio
					if ( file_size ) {
						u64 file_remain_size = file_size - file_proc_size;
						double worst_ratio = static_cast< double >( file_comp_size + file_remain_size ) / file_size * 100;
						double best_ratio = static_cast< double >( file_comp_size + 0 ) / file_size * 100;
						good_ratio = worst_ratio <= params.max_compression_ratio;
						if ( good_ratio || best_ratio > params.max_compression_ratio ) {
							eof = true;
							}
						}
					// find buffer ready for I/O
					buf = buffers.find( bs_io_ready );
					assert( buf );
					buf->state = bs_processing;
					}

				try {
					if ( !eof ) {
						unsigned buffer_data_size = file.read( buf->io_buffer, buffers.io_buffer_size );
						buf->data_size = buffer_data_size;
						eof = buffer_data_size == 0;
						}
					}
				catch ( ... ) {
						{
						CriticalSectionLock lock( sync );
						buf->state = bs_io_ready;
						}
					// signal worker threads
					CHECK_SYS( ReleaseSemaphore( io_ready_sem.handle( ), 1, NULL ) );
					fatal_error = false;
					throw;
					}

			{
			CriticalSectionLock lock( sync );
			// mark buffer ready for processing
			buf->state = eof ? bs_io_ready : bs_proc_ready;
				}
				// signal worker threads
				CHECK_SYS( ReleaseSemaphore( eof ? io_ready_sem.handle( ) : proc_ready_sem.handle( ), 1, NULL ) );
				}
			else {
				FAIL( MsgError( L"Compression thread failure" ) );
				}

			update_progress( phase_estimate );
			} // end file read loop

		// wait for all buffers to be processed
		for ( unsigned i = 0; i < num_buf; i++ ) {
			CHECK_SYS( WaitForSingleObject( io_ready_sem.handle( ), INFINITE ) != WAIT_FAILED );
			}
		// release IO buffers
		CHECK_SYS( ReleaseSemaphore( io_ready_sem.handle( ), num_buf, NULL ) );

		fatal_error = false;

		if ( file_proc_size < file_size ) {
			total_proc_size += file_size - file_proc_size;
			}
		if ( file_proc_size && static_cast< double >( file_comp_size ) / file_proc_size * 100 <= params.max_compression_ratio ) {
			good_ratio = true;
			}
		if ( good_ratio ) {
			// compress file
			update_progress( phase_compress, true );
			USHORT format = COMPRESSION_FORMAT_LZNT1;
			DWORD bytes_ret;
			CHECK_SYS( DeviceIoControl( file.handle( ), FSCTL_SET_COMPRESSION, &format, sizeof( format ), NULL, 0, &bytes_ret, NULL ) );

			if ( params.defragment_after_compression ) {
				defragment( file_name, *this );
				}
			}

		file_cnt++;
		}
	catch ( const Error& e ) {
		log.add( file_name, e.message( ) );
		err_cnt++;
		if ( fatal_error ) {
			throw;
			}
		}
	}

void CompressFiles::compress_directory( const std::string& dir_name ) {
	std::string file_name;
	FileEnum file_enum( dir_name );
	while ( true ) {
		try {
			if ( !file_enum.next( ) ) {
				break;
				}
			}
		catch ( const Error& e ) {
			log.add( dir_name, e.message( ) );
			err_cnt++;
			break;
			}
		file_name = add_trailing_slash( dir_name ) + file_enum.data( ).cFileName;

		if ( file_enum.data( ).dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
			}
		else if ( file_enum.data( ).is_dir( ) ) {
			compress_directory( file_name );
			}
		else {
			compress_file( file_name, file_enum.data( ) );
			}

		update_progress( phase_compress );
		}
	}

void CompressFiles::process( const std::vector<std::string>& file_list ) {
		{
		start_time = get_time( );
		total_size = 0;
		total_file_cnt = 0;

		SYSTEMTIME system_time;
		GetSystemTime( &system_time );
		FILETIME system_time_as_file_time;
		SystemTimeToFileTime( &system_time, &system_time_as_file_time );
		now = ( static_cast< ULONGLONG >( system_time_as_file_time.dwHighDateTime ) << 32 ) + system_time_as_file_time.dwLowDateTime;

		// estimate total file size
		for ( unsigned i = 0; i < file_list.size( ); i++ ) {
			const std::string& file_name = file_list[ i ];
			FindData find_data;
			try {
				find_data = get_find_data( file_name );
				}
			catch ( ... ) {
				continue;
				}
			if ( find_data.is_dir( ) ) {
				estimate_directory_size( file_name );
				}
			else {
				estimate_file_size( find_data );
				}
			}
		}

	  {
	  total_proc_size = 0;
	  file_cnt = err_cnt = 0;

	  WorkerThreads workers( *this );

	  wait_handles.clear( );
	  wait_handles.reserve( 2 + workers.size( ) );
	  wait_handles.push_back( stop_event.handle( ) );
	  wait_handles.push_back( io_ready_sem.handle( ) );
	  wait_handles.insert( wait_handles.end( ), workers.begin( ), workers.end( ) );

	  for ( unsigned i = 0; i < file_list.size( ); i++ ) {
		  const std::string& file_name = file_list[ i ];
		  FindData find_data;
		  try {
			  find_data = get_find_data( file_name );
			  }
		  catch ( const Error& e ) {
			  log.add( file_name, e.message( ) );
			  err_cnt++;
			  continue;
			  }
		  if ( find_data.is_dir( ) ) {
			  compress_directory( file_name );
			  }
		  else {
			  compress_file( file_name, find_data );
			  }
		  }
		  }
	}

void plugin_compress_files( const std::vector<std::string>& file_list, const CompressFilesParams& params, Log& log ) {
	init_comp_api( );
	CompressFiles compress_files( params, log, get_cpu_count( ), get_cluster_size( file_list[ 0 ] ) );
	compress_files.process( file_list );
	}

//class CompressFilesDialog : public FarDialog {
//	private:
//	enum {
//		c_client_xs = 40
//		};
//
//	CompressFilesParams& params;
//
//	int min_file_size_ctrl_id;
//	int max_compression_ratio_ctrl_id;
//	int min_file_age_ctrl_id;
//	int defragment_after_compression_ctrl_id;
//	int ok_ctrl_id;
//	int cancel_ctrl_id;
//
//	static intptr_t WINAPI dialog_proc( HANDLE h_dlg, intptr_t msg, intptr_t param1, void* param2 ) {
//		BEGIN_ERROR_HANDLER;
//		CompressFilesDialog* dlg = static_cast< CompressFilesDialog* >( FarDialog::get_dlg( h_dlg ) );
//		if ( ( msg == DN_CLOSE ) && ( param1 >= 0 ) && ( param1 != dlg->cancel_ctrl_id ) ) {
//			dlg->params.min_file_size = str_to_int( dlg->get_text( dlg->min_file_size_ctrl_id ) );
//			dlg->params.max_compression_ratio = str_to_int( dlg->get_text( dlg->max_compression_ratio_ctrl_id ) );
//			dlg->params.min_file_age = str_to_int( dlg->get_text( dlg->min_file_age_ctrl_id ) );
//			dlg->params.defragment_after_compression = dlg->get_check( dlg->defragment_after_compression_ctrl_id );
//			}
//		END_ERROR_HANDLER( ; , ; );
//		return g_far.DefDlgProc( h_dlg, msg, param1, param2 );
//		}
//
//	public:
//	CompressFilesDialog( CompressFilesParams& params ) : FarDialog( c_compress_files_dialog_guid, far_get_msg( MSG_COMPRESS_FILES_TITLE ), c_client_xs ), params( params ) { }
//
//	bool show( ) {
//		//label( far_get_msg( MSG_COMPRESS_FILES_MIN_FILE_SIZE ) );
//		spacer( 1 );
//		min_file_size_ctrl_id = var_edit_box( int_to_str( params.min_file_size ), 5 );
//		new_line( );
//		//label( far_get_msg( MSG_COMPRESS_FILES_MAX_COMPRESSION_RATIO ) );
//		spacer( 1 );
//		max_compression_ratio_ctrl_id = var_edit_box( int_to_str( params.max_compression_ratio ), 5 );
//		new_line( );
//		//label( far_get_msg( MSG_COMPRESS_FILES_MIN_FILE_AGE ) );
//		spacer( 1 );
//		min_file_age_ctrl_id = var_edit_box( int_to_str( params.min_file_age ), 5 );
//		new_line( );
//		//defragment_after_compression_ctrl_id = check_box( far_get_msg( MSG_COMPRESS_FILES_DEFRAGMENT_AFTER_COMPRESSION ), params.defragment_after_compression );
//		new_line( );
//		separator( );
//		new_line( );
//
//		//ok_ctrl_id = def_button( far_get_msg( MSG_BUTTON_OK ), DIF_CENTERGROUP );
//		//cancel_ctrl_id = button( far_get_msg( MSG_BUTTON_CANCEL ), DIF_CENTERGROUP );
//		new_line( );
//
//		int item = FarDialog::show( dialog_proc, L"compress_files" );
//
//		return ( item != -1 ) && ( item != cancel_ctrl_id );
//		}
//	};
//
//bool show_compress_files_dialog( CompressFilesParams& params ) {
//	return CompressFilesDialog( params ).show( );
//	}