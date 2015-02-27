#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <climits>
#include <memory>

struct children_heap_block_allocation;
struct SomeObject;

struct SomeObject {
	SomeObject( ) : m_some_number( UINT64_MAX ), m_children_allocation( nullptr ) { }
	~SomeObject( );
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t m_some_number;
	children_heap_block_allocation* m_children_allocation;
	char end[ 4 ];
	};


//If a class contains an unsized array, it cannot be used as the base class for another class.
//In addition, a class containing an unsized array cannot be used to declare any member except the last member of another class.
//A class containing an unsized array cannot have a direct or indirect virtual base class.
//The sizeof operator, when applied to a class containing an unsized array, returns the amount of storage required for all members except the unsized array.
//Implementors of classes that contain unsized arrays should provide alternate methods for obtaining the correct size of the class.
//You cannot declare arrays of objects that have unsized array components.
//Also, performing pointer arithmetic on pointers to such objects generates an error message.
struct children_heap_block_allocation {
	children_heap_block_allocation( ) : m_childCount { 0u } { }
	children_heap_block_allocation( const children_heap_block_allocation& in ) = delete;
	children_heap_block_allocation& operator=( const children_heap_block_allocation& in ) = delete;


	_Field_range_( 0, 4294967295 )
		std::uint32_t m_childCount;
#pragma warning( suppress: 4200 )//yes, this is Microsoft-specific
		SomeObject    m_children[ ];

	};
