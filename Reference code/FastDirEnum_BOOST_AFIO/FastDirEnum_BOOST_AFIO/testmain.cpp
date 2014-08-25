#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <boost/afio/afio.hpp>
#include <cstdint>
#include <future>
#include <regex>
#include <mutex>

using namespace boost::afio;

std::mutex coutMutex;
// Often it's easiest for a lot of nesting callbacks to carry state via a this pointer
class find_in_files {
	public:
	std::promise<int> finished;
	//std::regex regexpr; // The precompiled regular expression
	std::shared_ptr<async_file_io_dispatcher_base> dispatcher;
	//recursive_mutex opslock;
	//std::deque<async_io_op> ops; // For exception gathering
	std::atomic<size_t> /*bytesread, */ /*filesread,*/ /*filesmatched,*/ scheduled, completed, items, sizeTotal;
	std::vector<std::pair<std::filesystem::path, size_t>> filepaths;
	std::vector<std::wstring> files;
	std::vector<std::wstring> dirs;
	// Signals finish once all scheduled ops have completed
	void docompleted( size_t inc ) {
		size_t c = ( completed += inc );
		if ( c == scheduled )
			finished.set_value( 0 );
		};
	// Adds ops to the list of scheduled
	void doscheduled( std::initializer_list<async_io_op> list ) {
		scheduled += list.size( );
		}
	void doscheduled( std::vector<async_io_op> list ) {
		scheduled += list.size( );
		}

	//// A file searching completion, called when each file read completes
	//std::pair<bool, std::shared_ptr<async_io_handle>> file_read( size_t id,
	//	async_io_op op, std::shared_ptr < std::vector<char,
	//	detail::aligned_allocator<char, 4096, false >> > _buffer, size_t length ) {
	//	std::shared_ptr<async_io_handle> h( op.get( ) );
	//	std::cout << "R " << h->path() << std::endl;
	//	char *buffer = _buffer->data( );
	//	buffer[ length ] = 0;
	//	//dosearch( h, buffer, length );
	//	docompleted( 2 );
	//	// Throw away the buffer now rather than later to keep memory consumption down
	//	_buffer->clear( );
	//	return std::make_pair( true, h );
	//	}



//	// A file reading completion, called when each file open completes
//	std::pair<bool, std::shared_ptr<async_io_handle>> file_opened( size_t id,
//		async_io_op op, size_t length ) {
//		std::shared_ptr<async_io_handle> h( op.get( ) );
//		std::cout << "F " << h->path() << std::endl;
//#ifdef USE_MMAPS
//		if(!!(h->flags() & file_flags::OSMMap))
//			{
//			dosearch(h, (const char *) h->try_mapfile(), length);
//			}
//		else
//#endif
//			{
//			// Allocate a sufficient 4Kb aligned buffer
//			size_t _length = ( 4095 + length )&~4095;
//			auto buffer = std::make_shared < std::vector<char, detail::aligned_allocator<char, 4096, false >> >( _length + 1 );
//			// Schedule a read of the file
//			auto read = dispatcher->read( make_async_data_op_req( dispatcher->op_from_scheduled_id( id ), buffer->data( ), _length, 0 ) );
//			auto read_done = dispatcher->completion( read, std::make_pair( async_op_flags::none/*regex search might be slow*/, std::function<async_file_io_dispatcher_base::completion_t>( std::bind( &find_in_files::file_read, this, std::placeholders::_1, std::placeholders::_2, buffer, length ) ) ) );
//			doscheduled( { read, read_done } );
//			}
//		docompleted( 2 );
//		return std::make_pair( true, h );
//		}



	// An enumeration parsing completion, called when each directory enumeration completes
	std::pair<bool, std::shared_ptr<async_io_handle>> dir_enumerated( size_t id, async_io_op op, std::shared_ptr<future<std::pair<std::vector<directory_entry>, bool>>> listing ) {
		std::shared_ptr<async_io_handle> h( op.get( ) );
		async_io_op lastdir, thisop( dispatcher->op_from_scheduled_id( id ) );
		// Get the entries from the ready future
		std::vector<directory_entry> entries( std::move( listing->get( ).first ) );
		//std::cout << "E " << h->path() << std::endl;
		++items;
		// For each of the directories schedule an open and enumeration
		// Algorithm 2
		// The Windows NT kernel filing system driver gets upset with too much concurrency. when used with OSDirect so throttle directory enumerations to enforce some depth first traversal.
			{
			std::pair<async_op_flags, std::function<async_file_io_dispatcher_base::completion_t>> dir_openedf = std::make_pair( async_op_flags::none, std::bind( &find_in_files::dir_opened, this, std::placeholders::_1, std::placeholders::_2 ) );
			for ( auto &entry : entries ) {
				if ( entry.st_type( ) == std::filesystem::file_type::directory_file ) {
#ifdef DEBUG
					coutMutex.lock( );
					std::cout << "Found a directory: " << entry.name( ) << std::endl;
					coutMutex.unlock( );
#endif
					auto dir_open = dispatcher->dir( async_path_op_req( lastdir, h->path( ) / entry.name( ) ) );
					auto dir_opened = dispatcher->completion( dir_open, dir_openedf );
					doscheduled( { dir_open, dir_opened } );
					lastdir = dir_opened;
					++items;
					//dirs.push_back( entry.name( ).generic_wstring( ) );
					}
				if ( entry.st_type( ) == std::filesystem::file_type::regular_file ) {
					++items;
					sizeTotal += entry.st_size( );
					//files.push_back( entry.name( ).generic_wstring( ) );
					}
				}
				}

		// For each of the files schedule an open and search
		// Algorithm 2
//			{
//			for ( auto &entry : entries ) {
//				if ( entry.st_type( ) == std::filesystem::file_type::regular_file ) {
//					size_t length = ( size_t ) entry.st_size( );
//					if ( length ) {
//						file_flags flags = file_flags::Read;
//#ifdef USE_MMAPS
//						if(length>16384) flags=flags|file_flags::OSMMap;
//#endif
//						auto file_open = dispatcher->file( async_path_op_req( lastdir, h->path( ) / entry.name( ), flags ) );
//						//auto file_opened = dispatcher->completion( file_open, std::make_pair( async_op_flags::none, std::function<async_file_io_dispatcher_base::completion_t>( std::bind( &find_in_files::file_opened, this, std::placeholders::_1, std::placeholders::_2, length ) ) ) );
//						//doscheduled( { file_open, file_opened } );
//						//lastdir = file_opened;
//						}
//					}
//				}
//
//				}

		docompleted( 2 );
		return std::make_pair( true, h );
		}
	// A directory enumerating completion, called once per directory open in the tree
	std::pair<bool, std::shared_ptr<async_io_handle>> dir_opened( size_t id, async_io_op op ) {
		std::shared_ptr<async_io_handle> h( op.get( ) );
		//std::cout << "D " << h->path() << std::endl;
		++items;
		// Now we have an open directory handle, schedule an enumeration
		auto enumeration = dispatcher->enumerate( async_enumerate_op_req( dispatcher->op_from_scheduled_id( id ), metadata_flags::size, 1000 ) );
		auto listing = std::make_shared < future < std::pair<std::vector<directory_entry>, bool >> >( std::move( enumeration.first ) );
		auto enumeration_done = dispatcher->completion( enumeration.second, make_pair( async_op_flags::none, std::function<async_file_io_dispatcher_base::completion_t>( std::bind( &find_in_files::dir_enumerated, this, std::placeholders::_1, std::placeholders::_2, listing ) ) ) );
		doscheduled( { enumeration.second, enumeration_done } );
		docompleted( 2 );
		// Complete only if not the cur dir opened
		return std::make_pair( true, h );
		};
	void dowait( ) {
		// Prepare finished
		auto finished_waiter = finished.get_future( );
		finished_waiter.wait( );
		}
	// Constructor, which starts the ball rolling
	find_in_files( /*const char *_regexpr*/ ) : /*regexpr( _regexpr ),*/
		// Create an AFIO dispatcher that bypasses any filing system buffers
		dispatcher( make_async_file_io_dispatcher( process_threadpool( ), file_flags::WillBeSequentiallyAccessed/*|file_flags::OSDirect*/ ) ),
		/*bytesread( 0 ),*/ /*filesread( 0 ), filesmatched( 0 ),*/ scheduled( 0 ), completed( 0 ), sizeTotal( 0 ){
		//filepaths.reserve( 500000 );

		// Schedule the recursive enumeration of the current directory
		
		auto cur_dir = dispatcher->dir( async_path_op_req( "" ) );
		std::cout << "\n\nStarting directory enumerations ..." << std::endl;
		auto cur_dir_opened = dispatcher->completion( cur_dir, std::make_pair( async_op_flags::none, std::function<async_file_io_dispatcher_base::completion_t>( std::bind( &find_in_files::dir_opened, this, std::placeholders::_1, std::placeholders::_2 ) ) ) );
		doscheduled( { cur_dir, cur_dir_opened } );
		dowait( );
		}

	//	// An enumeration parsing completion, called when each directory enumeration completes
	//std::pair<bool, std::shared_ptr<async_io_handle>> Alt_dir_enumerated( size_t id, async_io_op op, std::vector<directory_entry>& listing ) {
	//	std::shared_ptr<async_io_handle> h( op.get( ) );
	//	async_io_op lastdir, thisop( dispatcher->op_from_scheduled_id( id ) );
	//	std::cout << "E " << h->path() << std::endl;
	//	// For each of the directories schedule an open and enumeration

	//	// Algorithm 2
	//	// The Windows NT kernel filing system driver gets upset with too much concurrency when used with OSDirect so throttle directory enumerations to enforce some depth first traversal.
	//			{
	//			std::pair<async_op_flags, std::function<async_file_io_dispatcher_base::completion_t>> dir_openedf = std::make_pair( async_op_flags::none, std::bind( &find_in_files::dir_opened, this, std::placeholders::_1, std::placeholders::_2 ) );
	//			for ( auto &entry : listing ) {
	//				if ( entry.st_type( ) == std::filesystem::file_type::directory_file ) {
	//					auto dir_open = dispatcher->dir( async_path_op_req( lastdir, h->path( ) / entry.name( ) ) );
	//					auto dir_opened = dispatcher->completion( dir_open, dir_openedf );
	//					doscheduled( { dir_open, dir_opened } );
	//					lastdir = dir_opened;
	//					}
	//				}
	//				}
	//	}

	};

int main( int argc, const char *argv[ ] ) {
	using std::placeholders::_1; using std::placeholders::_2;
	using namespace boost::afio;
	typedef chrono::duration<double, ratio<1>> secs_type;
	if ( argc < 2 ) {
		std::cerr << "ERROR: Specify a regular expression to search all files in the current directory." << std::endl;
		return 1;
		}
	for ( int i = 0; i < argc; ++i ) {
		std::cout << "arg #" << i << ": " << argv[ i ] << std::endl;
		}
	//char cwd[ 100 ];
	//auto resCWD = _getcwd( cwd, sizeof( char ) * 100 );
	//std::cout << "working directory: " << resCWD << std::endl;

	// Prime SpeedStep
	auto begin = chrono::high_resolution_clock::now( );
	//while ( chrono::duration_cast< secs_type >( chrono::high_resolution_clock::now( ) - begin ).count( ) < 1 );
	try {
		begin = chrono::high_resolution_clock::now( );
		find_in_files finder;
		auto end = chrono::high_resolution_clock::now( );
		auto diff = chrono::duration_cast< secs_type >( end - begin );
		//std::cout << "\n" << finder.filesmatched << " files matched out of " << finder.filesread << " files which was " << finder.bytesread << " bytes." << std::endl;
		std::cout << "The search in " << argv[ 1 ] << " found " << finder.items << " items in " << diff.count( ) << " seconds which is " << finder.items / diff.count( ) << " files per second" << std::endl << "Furthermore, the total size was: " << finder.sizeTotal << std::endl;

		}
	catch ( ... ) {
		std::cerr << boost::current_exception_diagnostic_information( true ) << std::endl;
		return 1;
		}
	return 0;
	}