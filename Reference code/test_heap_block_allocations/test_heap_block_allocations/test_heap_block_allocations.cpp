
#include "test_heap_block_allocation.h"



_At_( return, _Writable_bytes_( bytes_allocated ) )
_Ret_notnull_ children_heap_block_allocation* allocate_enough_memory_for_children_block( _In_ _In_range_( 1, UINT32_MAX ) const std::uint32_t number_of_children, _Out_ size_t& bytes_allocated, _Out_ size_t& address_start ) {
	const rsize_t base_memory_size_in_bytes = sizeof( decltype( children_heap_block_allocation::m_childCount ) );
	
	
	const rsize_t size_of_a_single_child_in_bytes = sizeof( SomeObject );
	const size_t size_of_children_needed_in_bytes = ( size_of_a_single_child_in_bytes * static_cast<size_t>( number_of_children + 1 ) );

	const size_t total_size_needed = ( base_memory_size_in_bytes + size_of_children_needed_in_bytes );
	void* const memory_block = malloc( total_size_needed );
	if ( memory_block == NULL ) {
		abort( );
		}
	bytes_allocated = total_size_needed;
	children_heap_block_allocation* const new_block = static_cast< children_heap_block_allocation* const>( memory_block );
	memset( new_block, 0, total_size_needed );
	new_block->m_childCount = number_of_children;
	address_start = reinterpret_cast<size_t>( memory_block );
	return new_block;
	}
void done( ) {
	printf( "done!\r\n" );
	}

SomeObject::~SomeObject( ) {
	if ( m_children_allocation != nullptr ) {
		for ( size_t i = 0; i < m_children_allocation->m_childCount; ++i ) {
			m_children_allocation->m_children[ i ].~SomeObject( );
			}
		}
	free( m_children_allocation );
	m_children_allocation = nullptr;
	}


int main( ) {
		{
		SomeObject the_object;
		the_object.end[ 0 ] = 'e';
		the_object.end[ 1 ] = 'n';
		the_object.end[ 2 ] = 'd';
		the_object.end[ 3 ] = 0;


		the_object.m_some_number = 25u;

		size_t bytes_allocated = 0;
		size_t address_start = 0;

		the_object.m_children_allocation = allocate_enough_memory_for_children_block( 5000u, bytes_allocated, address_start );

		SomeObject* const block = the_object.m_children_allocation->m_children;
		printf( "Block, size: %I64u, allocated at: %p\r\n", std::uint64_t( bytes_allocated ), block );

		//void* block_end = ( std::ptrdiff_t( block ) + std::ptrdiff_t( bytes_allocated ) );
		printf( "Block start: %I64u\r\n", std::uint64_t( address_start ) );
		printf( "Block end  : %I64u\r\n", std::uint64_t( address_start + bytes_allocated ) );


		for ( size_t i = 0; i < the_object.m_children_allocation->m_childCount; ++i ) {
			SomeObject* const new_obj_location = ( block + i );
			printf( "constructing object at: %p\r\n", new_obj_location );
			new ( new_obj_location ) SomeObject;
			new_obj_location->end[ 0 ] = 'e';
			new_obj_location->end[ 1 ] = 'n';
			new_obj_location->end[ 2 ] = 'd';
			new_obj_location->end[ 3 ] = 0;
			new_obj_location->m_children_allocation = allocate_enough_memory_for_children_block( 500u, bytes_allocated, address_start );
			}

		printf( "object construction complete!\r\n" );
		}
	done( );
	}