// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_CHILDREN_HEAP_MANAGER_H_INCLUDED
#define WDS_CHILDREN_HEAP_MANAGER_H_INCLUDED


WDS_FILE_INCLUDE_MESSAGE

#pragma pack(push, 1)
#pragma message( "Whoa there! I'm changing the natural data alignment for (all of) ChildrenHeapManager.h - Look for a message that says I'm restoring it!" )



//See N4188
//If a class contains an unsized array, it cannot be used as the base class for another class. In addition, a class containing an unsized array cannot be used to declare any member except the last member of another class. A class containing an unsized array cannot have a direct or indirect virtual base class.
//The sizeof operator, when applied to a class containing an unsized array, returns the amount of storage required for all members except the unsized array. Implementors of classes that contain unsized arrays should provide alternate methods for obtaining the correct size of the class.
//You cannot declare arrays of objects that have unsized array components. Also, performing pointer arithmetic on pointers to such objects generates an error message.
//newer, more efficient allocation strategy will use a struct with the child count, child name heap manager, and an UNSIZED ARRAY as the children array.
//See also: "MEM54-CPP. Provide placement new with properly-aligned pointers to sufficient storage capacity"
//           https://www.securecoding.cert.org/confluence/display/cplusplus/MEM54-CPP.+Provide+placement+new+with+properly-aligned+pointers+to+sufficient+storage+capacity
struct Children_String_Heap_Manager_Impl final {
	Children_String_Heap_Manager_Impl& operator=( const Children_String_Heap_Manager_Impl& in ) = delete;
	Children_String_Heap_Manager_Impl( const Children_String_Heap_Manager_Impl& in ) = delete;

	//TODO: inline these?
	__forceinline Children_String_Heap_Manager_Impl( ) noexcept : m_buffer_size { 0u }, m_buffer_filled { 0u }/*, m_string_buffer { nullptr }*/ { }
	__forceinline Children_String_Heap_Manager_Impl( _In_ const rsize_t number_of_characters_needed ) noexcept : m_buffer_size( number_of_characters_needed ), m_buffer_filled( 0 )/*, m_string_buffer( new wchar_t[ number_of_characters_needed ] )*/ { }

	//__forceinline void reset( const rsize_t number_of_characters_needed ) {
	//	m_buffer_filled = 0u;
	//	//m_string_buffer.reset( new wchar_t[ number_of_characters_needed ] );
	//	m_buffer_size = number_of_characters_needed;
	//	}

	_Success_( SUCCEEDED( return ) )
	const HRESULT copy_name_str_into_buffer( _Pre_invalid_ _Always_(_Post_z_) _Out_writes_( new_name_length ) wchar_t*& new_name_ptr, _In_ _In_range_( 0, UINT16_MAX ) const rsize_t& new_name_length, const std::wstring& name ) noexcept;

	_Field_range_(0, SIZE_T_MAX) const size_t m_buffer_size;
	_Field_range_(<=, m_buffer_size) size_t m_buffer_filled;
#pragma warning( suppress: 4200 )//yes, this is Microsoft-specific
	_Field_size_part_( m_buffer_size, m_buffer_filled ) wchar_t m_string_buffer[ ];

	};


#ifdef new
#pragma push_macro("new")
#define WDS_CHILDREN_HEAP_MANAGER_PUSHED_MACRO_NEW
#undef new
#endif


class Children_String_Heap_Manager final {
	Children_String_Heap_Manager& operator=( const Children_String_Heap_Manager& in ) = delete;
	Children_String_Heap_Manager( const Children_String_Heap_Manager& in ) = delete;

	//std::unique_ptr<wchar_t[ ]> buffer_ptr;
	
public:
	Children_String_Heap_Manager( _In_ const rsize_t number_of_characters_needed ) /* : buffer_ptr { new wchar_t[ number_of_characters_needed ] }*/ {
		constexpr rsize_t size_of_base_struct = sizeof( Children_String_Heap_Manager_Impl );
		const rsize_t size_total_needed = ( size_of_base_struct + ( sizeof( wchar_t ) * number_of_characters_needed ) );
		m_buffer_impl = static_cast<Children_String_Heap_Manager_Impl*>( malloc( size_total_needed ) );
#pragma message("This is temporary!")
#pragma warning(suppress: 6386)
		new ( m_buffer_impl ) Children_String_Heap_Manager_Impl( number_of_characters_needed );
		}
	~Children_String_Heap_Manager( ) {
		free( m_buffer_impl );
		m_buffer_impl = nullptr;
		}

	Children_String_Heap_Manager_Impl* m_buffer_impl;
	};


class CTreeListItem;

struct child_info final {

	child_info( _In_ const rsize_t number_of_characters_needed, _In_ const rsize_t child_count );
	
	~child_info( );

	//please always pass a size for m_name_pool. you should know this before construction, that's the point of child_info
	child_info( ) = delete;

	child_info( const child_info& in ) = delete;
	child_info& operator=( const child_info& in ) = delete;
	
	_Field_size_( m_childCount )   std::unique_ptr<CTreeListItem[]>      m_children;
	_Field_range_( 0, 4294967295 ) std::uint32_t                         m_childCount;
	                               Children_String_Heap_Manager_Impl     m_name_pool;
	};

struct child_info_block_manager final {
	child_info* m_child_info_ptr;
	child_info_block_manager( ) noexcept : m_child_info_ptr( nullptr ) { }
	child_info_block_manager( _In_ const rsize_t number_of_characters_needed, _In_ const rsize_t child_count ) {
		const rsize_t size_of_a_single_struct_in_bytes = sizeof( child_info );
		const rsize_t character_bytes_needed = ( number_of_characters_needed * sizeof( wchar_t ) );
		
		//maybe /analyze thinks we will overflow?
		//apparently not, but this is a good idea anyways.
		if ( ( number_of_characters_needed * sizeof( wchar_t ) ) < number_of_characters_needed ) {
			std::terminate( );
			} 
		if ( ( character_bytes_needed + size_of_a_single_struct_in_bytes ) < size_of_a_single_struct_in_bytes ) {
			std::terminate( );
			}
		if ( ( character_bytes_needed + size_of_a_single_struct_in_bytes ) < character_bytes_needed ) {
			std::terminate( );
			}

		const size_t total_size_needed = ( size_of_a_single_struct_in_bytes + character_bytes_needed );
		ASSERT( total_size_needed > 0 );
		if ( total_size_needed < sizeof( child_info ) ) {
			std::terminate( );
			}
		void* const memory_block = malloc( total_size_needed );
		if ( memory_block == nullptr ) {
			std::terminate( );
			}

		m_child_info_ptr = new ( memory_block ) child_info( number_of_characters_needed, child_count );
		}
	~child_info_block_manager( ) {
		if ( m_child_info_ptr != nullptr ) {
			m_child_info_ptr->~child_info( );
			free( m_child_info_ptr );
			m_child_info_ptr = nullptr;
			}
		}
	};


#ifdef WDS_CHILDREN_HEAP_MANAGER_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_CHILDREN_HEAP_MANAGER_PUSHED_MACRO_NEW
#endif


#pragma message( "Restoring data alignment.... " )
#pragma pack(pop)


#else

#endif