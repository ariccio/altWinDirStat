#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <boost/afio/afio.hpp>
#include <cstdint>
#include <future>
#include <regex>
#include <mutex>
#include <filesystem>

//using namespace boost::afio;

std::mutex coutMutex;
// Often it's easiest for a lot of nesting callbacks to carry state via a this pointer


class find_in_files {
void doscheduled( std::initializer_list<boost::afio::async_io_op> list ) {
	scheduled += list.size( );
	}
void doscheduled( std::vector<boost::afio::async_io_op> list ) {
	scheduled += list.size( );
	}
	public:
	std::promise<int> finished;
	std::shared_ptr<boost::afio::async_file_io_dispatcher_base> dispatcher;
	//async_file_io_dispatcher_base internal code:
	//struct detail::async_io_handle_windows;
	//class detail::async_file_io_dispatcher_windows;
	//detail::async_file_io_dispatcher_base_p *p;
//    detail::async_file_io_dispatcher_base_p *p;
//    BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC void int_add_io_handle(void *key, std::shared_ptr<async_io_handle> h);
//    BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC void int_del_io_handle(void *key);
//    BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC async_io_op int_op_from_scheduled_id(size_t id) const;
//protected:
//    BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC async_file_io_dispatcher_base(std::shared_ptr<thread_source> threadpool, file_flags flagsforce, file_flags flagsmask);
	//BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC file_flags fileflags(file_flags flags) const;
	//BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC async_io_op op_from_scheduled_id(size_t id) const;
	//typedef completion_returntype completion_t(size_t, async_io_op);
	//BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::vector<async_io_op> completion(const std::vector<async_io_op> &ops, const std::vector<std::pair<async_op_flags, std::function<async_file_io_dispatcher_base::completion_t>>> &callbacks);
	//inline async_io_op completion(const async_io_op &req, const std::pair<async_op_flags, std::function<async_file_io_dispatcher_base::completion_t>> &callback);
	//BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC std::vector<async_io_op> dir(const std::vector<async_path_op_req> &reqs) BOOST_AFIO_HEADERS_ONLY_VIRTUAL_UNDEFINED_SPEC
	//inline async_io_op dir(const async_path_op_req &req);
	//inline async_io_op rmdir(const async_path_op_req &req);
	//BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC std::vector<async_io_op> file(const std::vector<async_path_op_req> &reqs) BOOST_AFIO_HEADERS_ONLY_VIRTUAL_UNDEFINED_SPEC
	//inline async_io_op file(const async_path_op_req &req);
	//BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC std::vector<async_io_op> close(const std::vector<async_io_op> &ops) BOOST_AFIO_HEADERS_ONLY_VIRTUAL_UNDEFINED_SPEC
	//inline async_io_op close(const async_io_op &req);
	//BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC std::pair<std::vector<future<std::pair<std::vector<directory_entry>, bool>>>, std::vector<async_io_op>> enumerate(const std::vector<async_enumerate_op_req> &reqs) BOOST_AFIO_HEADERS_ONLY_VIRTUAL_UNDEFINED_SPEC
	//inline std::pair<future<std::pair<std::vector<directory_entry>, bool>>, async_io_op> enumerate(const async_enumerate_op_req &req);
	//static BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC size_t page_size() BOOST_NOEXCEPT_OR_NOTHROW;
	//BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC void complete_async_op(size_t id, std::shared_ptr<async_io_handle> h, exception_ptr e=exception_ptr());
	//void complete_async_op(size_t id, exception_ptr e) { complete_async_op(id, std::shared_ptr<async_io_handle>(), e); }
    //BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC completion_returntype invoke_user_completion_fast(size_t id, async_io_op h, completion_t *callback);
    //BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC completion_returntype invoke_user_completion_slow(size_t id, async_io_op h, std::function<completion_t> callback);
    //template<class F, class T> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::vector<async_io_op> chain_async_ops(int optype, const std::vector<async_io_op> &preconditions, const std::vector<T> &container, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, T));
    //template<class F, class T> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::vector<async_io_op> chain_async_ops(int optype, const std::vector<T> &container, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, T));
    //template<class F> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::vector<async_io_op> chain_async_ops(int optype, const std::vector<async_io_op> &container, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, async_io_op));
    //template<class F> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::vector<async_io_op> chain_async_ops(int optype, const std::vector<async_path_op_req> &container, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, async_path_op_req));
    //template<class F, bool iswrite> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::vector<async_io_op> chain_async_ops(int optype, const std::vector<detail::async_data_op_req_impl<iswrite>> &container, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, detail::async_data_op_req_impl<iswrite>));
    //template<class F> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::pair<std::vector<future<std::pair<std::vector<directory_entry>, bool>>>, std::vector<async_io_op>> chain_async_ops(int optype, const std::vector<async_enumerate_op_req> &container, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, async_enumerate_op_req, std::shared_ptr<promise<std::pair<std::vector<directory_entry>, bool>>>));
    //template<class T> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC async_file_io_dispatcher_base::completion_returntype dobarrier(size_t id, async_io_op h, T);
    //template<class F, class... Args> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC std::shared_ptr<async_io_handle> invoke_async_op_completions(size_t id, async_io_op h, completion_returntype(F::*f)(size_t, async_io_op, Args...), Args... args);
    //template<class F, class... Args> BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC async_io_op chain_async_op(detail::immediate_async_ops &immediates, int optype, const async_io_op &precondition, async_op_flags flags, completion_returntype(F::*f)(size_t, async_io_op, Args...), Args... args);
	//BOOST_AFIO_HEADERS_ONLY_FUNC_SPEC std::shared_ptr<async_file_io_dispatcher_base> make_async_file_io_dispatcher(std::shared_ptr<thread_source> threadpool=process_threadpool(), file_flags flagsforce=file_flags::None, file_flags flagsmask=file_flags::None);



	//recursive_mutex opslock;
	//std::deque<async_io_op> ops; // For exception gathering
	std::atomic<size_t> /*bytesread, */ /*filesread,*/ /*filesmatched,*/ scheduled, completed, items, sizeTotal;
	std::vector<std::pair<std::filesystem::path, size_t>> filepaths;
	std::vector<std::wstring> files;
	std::vector<std::wstring> dirs;
	
	void docompleted( size_t inc ) {
		//Signals finish once all scheduled ops have completed
		auto c = ( completed += inc );
		if ( c == scheduled )
			finished.set_value( 0 );
		};
	// Adds ops to the list of scheduled

	//// A file searching completion, called when each file read completes
	//std::pair<bool, std::shared_ptr<async_io_handle>> file_read( size_t id,


	// An enumeration parsing completion, called when each directory enumeration completes
	std::pair<bool, std::shared_ptr<boost::afio::async_io_handle>> dir_enumerated( size_t id, boost::afio::async_io_op op, std::shared_ptr<boost::afio::future<std::pair<std::vector<boost::afio::directory_entry>, bool>>> listing ) {
		std::shared_ptr<boost::afio::async_io_handle> h( op.get( ) );
		boost::afio::async_io_op lastdir, thisop( dispatcher->op_from_scheduled_id( id ) );
		// Get the entries from the ready future
		std::vector<boost::afio::directory_entry> entries( std::move( listing->get( ).first ) );
#ifdef DEBUG
		coutMutex.lock( );
		std::cout << "E " << h->path() << std::endl;
		coutMutex.unlock( );
#endif

		++items;
		// For each of the directories schedule an open and enumeration
		// Algorithm 2. The Windows NT kernel filing system driver gets upset with too much concurrency. when used with OSDirect so throttle directory enumerations to enforce some depth first traversal.
			{
			std::pair<boost::afio::async_op_flags, std::function<boost::afio::async_file_io_dispatcher_base::completion_t>> dir_openedf = std::make_pair( boost::afio::async_op_flags::none, std::bind( &find_in_files::dir_opened, this, std::placeholders::_1, std::placeholders::_2 ) );
			for ( auto &entry : entries ) {
				if ( entry.st_type( ) == std::filesystem::file_type::directory_file ) {
#ifdef DEBUG
					coutMutex.lock( );
					//std::cout << "Found a directory: " << entry.name( ) << std::endl;
					coutMutex.unlock( );
#endif
					auto dir_open = dispatcher->dir( boost::afio::async_path_op_req( lastdir, h->path( ) / entry.name( ) ) );
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

		docompleted( 2 );
		return std::make_pair( true, h );
		}
	// A directory enumerating completion, called once per directory open in the tree
	std::pair<bool, std::shared_ptr<boost::afio::async_io_handle>> dir_opened( size_t id, boost::afio::async_io_op op ) {
		std::shared_ptr<boost::afio::async_io_handle> h( op.get( ) );
		
#ifdef DEBUG
		coutMutex.lock( );
		std::cout << "D " << h->path() << std::endl;
		coutMutex.unlock( );
#endif

		++items;
		// Now we have an open directory handle, schedule an enumeration
		auto enumeration = dispatcher->enumerate( boost::afio::async_enumerate_op_req( dispatcher->op_from_scheduled_id( id ), boost::afio::metadata_flags::size, 1000 ) );
		auto listing = std::make_shared < boost::afio::future < std::pair<std::vector<boost::afio::directory_entry>, bool >> >( std::move( enumeration.first ) );
		auto enumeration_done = dispatcher->completion( enumeration.second, make_pair( boost::afio::async_op_flags::none, std::function<boost::afio::async_file_io_dispatcher_base::completion_t>( std::bind( &find_in_files::dir_enumerated, this, std::placeholders::_1, std::placeholders::_2, listing ) ) ) );
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
		dispatcher( make_async_file_io_dispatcher( boost::afio::process_threadpool( ), boost::afio::file_flags::WillBeSequentiallyAccessed/*|file_flags::OSDirect*/ ) ),
		/*bytesread( 0 ),*/ /*filesread( 0 ), filesmatched( 0 ),*/ scheduled( 0 ), completed( 0 ), sizeTotal( 0 ){
		//filepaths.reserve( 500000 );

		// Schedule the recursive enumeration of the current directory


		//async_io_op async_file_io_dispatcher_base::dir(const async_path_op_req &req)
		//  std::vector<async_path_op_req> i;
		//i.reserve(1);
		//i.push_back(req);
		//return std::move(dir(i).front());

		std::vector<boost::afio::async_path_op_req> asyncPathOpRec;
		asyncPathOpRec.emplace_back( boost::afio::async_path_op_req( "" ) );
		
		auto cur_dir = dispatcher->dir( boost::afio::async_path_op_req( "" ) );
		std::cout << "\n\nStarting directory enumerations ..." << std::endl;
		auto cur_dir_opened = dispatcher->completion( cur_dir, std::make_pair( boost::afio::async_op_flags::none, std::function<boost::afio::async_file_io_dispatcher_base::completion_t>( std::bind( &find_in_files::dir_opened, this, std::placeholders::_1, std::placeholders::_2 ) ) ) );
		doscheduled( { cur_dir, cur_dir_opened } );
		dowait( );
		}


	};


int helloWorldFunction( std::string text ) {
	std::cout << text << std::endl;
	return 42;
	}

int addToValueFunction( boost::afio::shared_future<int> v ) {
		return v.get( ) + 1;
	}

class myClass {
	public:
	std::promise<int> finished;
	std::shared_ptr<boost::afio::async_file_io_dispatcher_base> dispatcher;
	std::atomic<size_t> /*bytesread, */ /*filesread,*/ /*filesmatched,*/ scheduled, completed, items, sizeTotal;
	std::vector<std::pair<std::filesystem::path, size_t>> filepaths;
	std::vector<std::wstring> files;
	std::vector<std::wstring> dirs;
	//std::pair<boost::afio::shared_future<int>,boost::afio::async_io_op>
	void doscheduled( std::initializer_list<boost::afio::async_io_op> list ) {
		scheduled += list.size( );
		}
	void doscheduled( std::vector<boost::afio::async_io_op> list ) {
		scheduled += list.size( );
		}
	void doscheduled( std::pair<boost::afio::shared_future<int>, boost::afio::async_io_op> in ) {
		scheduled += 1;
		}
	void docompleted( size_t inc ) {
		//Signals finish once all scheduled ops have completed
		auto c = ( completed += inc );
		if ( c == scheduled )
			finished.set_value( 0 );
		};
	// An enumeration parsing completion, called when each directory enumeration completes
	std::pair<bool, std::shared_ptr<boost::afio::async_io_handle>> alt_dir_enumerated( size_t id, boost::afio::async_io_op op, std::shared_ptr<boost::afio::future<std::pair<std::vector<boost::afio::directory_entry>, bool>>> listing ) {
		std::shared_ptr<boost::afio::async_io_handle> h( op.get( ) );
		boost::afio::async_io_op lastdir, thisop( dispatcher->op_from_scheduled_id( id ) );
		// Get the entries from the ready future
		std::vector<boost::afio::directory_entry> entries( std::move( listing->get( ).first ) );
#ifdef DEBUG
		coutMutex.lock( );
		std::cout << "E " << h->path( ) << std::endl;
		coutMutex.unlock( );
#endif

		++items;
		// For each of the directories schedule an open and enumeration
		// Algorithm 2. The Windows NT kernel filing system driver gets upset with too much concurrency. when used with OSDirect so throttle directory enumerations to enforce some depth first traversal.
			{
			std::pair<boost::afio::async_op_flags, std::function<boost::afio::async_file_io_dispatcher_base::completion_t>> dir_openedf = std::make_pair( boost::afio::async_op_flags::none, std::bind( &find_in_files::dir_opened, this, std::placeholders::_1, std::placeholders::_2 ) );
			for ( auto &entry : entries ) {
				if ( entry.st_type( ) == std::filesystem::file_type::directory_file ) {
#ifdef DEBUG
					coutMutex.lock( );
					//std::cout << "Found a directory: " << entry.name( ) << std::endl;
					coutMutex.unlock( );
#endif
					auto dir_open = dispatcher->dir( boost::afio::async_path_op_req( lastdir, h->path( ) / entry.name( ) ) );
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

		docompleted( 2 );
		return std::make_pair( true, h );
		}


	//void alt_completion_returntype boost::afio::detail::async_file_io_dispatcher_windows::dofile(size_t id, async_io_op, async_path_op_req req)
	void alt_enum_test( ) {
		// Create a dispatcher instance
		auto dispatcher = boost::afio::make_async_file_io_dispatcher( );

		// Schedule an asynchronous call of some function with some bound set of arguments
		//auto helloworld = dispatcher->call( boost::afio::async_io_op( ) /* no precondition */, [] ( std::string text ) -> int {
		//	std::cout << text << std::endl;
		//	return 42;
		//	}, std::string( "Hello world" ) ); // Returns a pair of <future, op ref>

		auto helloworld = dispatcher->call( boost::afio::async_io_op( ) /* no precondition */, helloWorldFunction, std::string( "Hello world" ) ); // Returns a pair of <future, op ref>


		// Schedule as asynchronous call of some function to occur only after helloworld completes
		//auto addtovalue = dispatcher->call( helloworld.second, [] ( boost::afio::shared_future<int> v ) -> int {
		//	// v is highly likely to be ready very soon by the time we are called
		//	return v.get( ) + 1;
		//	}, helloworld.first );

		auto addtovalue = dispatcher->call( helloworld.second, addToValueFunction, helloworld.first );

		// Create a boost::future<> representing the ops passed to when_all()
		auto future = boost::afio::when_all( addtovalue.second );
		// ... and wait for it to complete
		future.wait( );

		// Print the result returned by the future for the lambda, which will be 43
		std::cout << "addtovalue() returned " << addtovalue.first.get( ) << std::endl;
		}

	// A directory enumerating completion, called once per directory open in the tree
	std::pair<bool, std::shared_ptr<boost::afio::async_io_handle>> alt_dir_opened( size_t id, boost::afio::async_io_op op ) {
		std::shared_ptr<boost::afio::async_io_handle> h( op.get( ) );

#ifdef DEBUG
		coutMutex.lock( );
		std::cout << "D " << h->path( ) << std::endl;
		coutMutex.unlock( );
#endif

		//++items;
		// Now we have an open directory handle, schedule an enumeration
		auto enumeration = dispatcher->enumerate( boost::afio::async_enumerate_op_req( dispatcher->op_from_scheduled_id( id ), boost::afio::metadata_flags::size, 1000 ) );
		auto listing = std::make_shared < boost::afio::future < std::pair<std::vector<boost::afio::directory_entry>, bool >> >( std::move( enumeration.first ) );
		//dir_enumerated( size_t id, boost::afio::async_io_op op, std::shared_ptr<boost::afio::future<std::pair<std::vector<boost::afio::directory_entry>, bool>>> listing )
		auto enumeration_done = dispatcher->call( enumeration.second, &myClass::alt_dir_enumerated, boost::afio::async_io_op( ), std::placeholders::_1, std::placeholders::_2, listing );
		//doscheduled( enumeration.second, enumeration_done );
		++scheduled;
		docompleted( 2 );
		// Complete only if not the cur dir opened
		return std::make_pair( true, h );
		};
	void dowait( ) {
		// Prepare finished
		auto finished_waiter = finished.get_future( );
		finished_waiter.wait( );
		}
	void alt_enum( ) {
		dispatcher = ( make_async_file_io_dispatcher( boost::afio::process_threadpool( ), boost::afio::file_flags::WillBeSequentiallyAccessed/*|file_flags::OSDirect*/ ) );
		auto cur_dir = dispatcher->dir( boost::afio::async_path_op_req( "" ) );
		auto cur_dir_opened = dispatcher->call( cur_dir, &myClass::alt_dir_opened, boost::afio::async_io_op( ), std::placeholders::_1, std::placeholders::_2 );
		++scheduled;
		//doscheduled( { cur_dir, cur_dir_opened } );
		dowait( );
		}

	};

int main( int argc, const char *argv[ ] ) {
	//using std::placeholders::_1;
	//using std::placeholders::_2;
	//using namespace boost::afio;
	typedef boost::afio::chrono::duration<double, boost::afio::ratio<1>> secs_type;
	if ( argc < 2 ) {
		std::cerr << "ERROR: Specify a regular expression to search all files in the current directory." << std::endl;
		return 1;
		}
	for ( int i = 0; i < argc; ++i ) {
		std::cout << "arg #" << i << ": " << argv[ i ] << std::endl;
		}
	//alt_enum_test( );
	myClass tester;
	tester.alt_enum( );

	auto begin = boost::afio::chrono::high_resolution_clock::now( );
	std::tr2::sys::path lePath;
	//while ( chrono::duration_cast< secs_type >( chrono::high_resolution_clock::now( ) - begin ).count( ) < 1 );
	try {
		begin = boost::afio::chrono::high_resolution_clock::now( );
		find_in_files finder;
		auto end = boost::afio::chrono::high_resolution_clock::now( );
		auto diff = boost::afio::chrono::duration_cast< secs_type >( end - begin );
		//std::cout << "\n" << finder.filesmatched << " files matched out of " << finder.filesread << " files which was " << finder.bytesread << " bytes." << std::endl;
		std::cout << "The search in " << argv[ 1 ] << " found " << finder.items << " items in " << diff.count( ) << " seconds which is " << finder.items / diff.count( ) << " files per second" << std::endl << "Furthermore, the total size was: " << finder.sizeTotal << std::endl;

		}
	catch ( ... ) {
		std::cerr << boost::current_exception_diagnostic_information( true ) << std::endl;
		return 1;
		}
	return 0;
	}