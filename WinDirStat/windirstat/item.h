// item.h	- Declaration of CItemBranch
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once
#include "stdafx.h"


#ifndef WDS_ITEM_H
#define WDS_ITEM_H

#include "Treelistcontrol.h"

class CItemBranch;//God I hate C++
class CTreeListItem;
class CDirstatApp;



// `/d1reportSingleClassLayoutCItemBranch`
class CItemBranch final : public CTreeListItem {
	/*
	  CItemBranch. This is the object, from which the whole tree is built. For every directory, file etc., we find [in the folder selected], there is one CItemBranch.
	*/
	static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "Bad parameter size! Check all functions that accept an unsigned long long or a std::uint64_t!" );

	public:
		CItemBranch  ( const std::uint64_t size, const FILETIME time, const DWORD attr, const bool done, _In_ CItemBranch* const parent, _In_z_ _Readable_elements_( length ) PCWSTR const name, const std::uint16_t length );
		
		//default constructor DOES NOT initialize.
		CItemBranch  ( ) { }

		virtual ~CItemBranch( ) final = default;

		CItemBranch& operator=( const CItemBranch& in ) = delete;

		CItemBranch( CItemBranch& in )  = delete;

		_Success_( return < SIZE_T_MAX )
		size_t findItemInChildren( const CItemBranch* const theItem ) const;

		//bool operator<( const CItemBranch& rhs ) const {
		//	return size_recurse( ) < rhs.size_recurse( );
		//	}

		void refresh_sizeCache( );

		_Ret_range_( 0, UINT64_MAX )
		std::uint64_t compute_size_recurse( ) const;

		_Ret_range_( 0, UINT64_MAX )
		std::uint64_t size_recurse( ) const;

		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		_Ret_range_( 0, 4294967295 )
		std::uint32_t files_recurse( ) const;

		FILETIME FILETIME_recurse( ) const;

	private:
		
		const COLORREF Concrete_ItemTextColor( ) const;

		//ItemTextColor __should__ be private!
		virtual COLORREF ItemTextColor( ) const override final {
			return Concrete_ItemTextColor( );
			}
		
		
	public:
		
		_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
		virtual HRESULT Text_WriteToStackBuffer ( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const override final;

		_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
		  const HRESULT WriteToStackBuffer_COL_PERCENTAGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_NTCOMPRESS ) _Success_( SUCCEEDED( return ) )
		  const HRESULT WriteToStackBuffer_COL_NTCOMPRESS( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_SUBTREETOTAL ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_SUBTREETOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_FILES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_LASTCHANGE ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_LASTCHANGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		_Pre_satisfies_( subitem == column::COL_ATTRIBUTES ) _Success_( SUCCEEDED( return ) )
		 inline const HRESULT WriteToStackBuffer_COL_ATTRIBUTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

		INT CompareSibling                           ( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const;

		void            TmiSetRectangle     ( _In_ const RECT& rc          ) const;
		RECT            TmiGetRectangle     (                               ) const;

		// Branch/Leaf shared functions
		_Must_inspect_result_ _Ret_maybenull_ 
		const CItemBranch* GetParentItem            (                                                  ) const { return static_cast< const CItemBranch* >( m_parent ); };

		INT     GetSortAttributes             (                                                                   ) const;
		
		//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx : Note  The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be expanded to a longer string by the system at run time, and this expansion applies to the total length.
		_Ret_range_( 0, 33000 ) DOUBLE  averageNameLength             (                                                                   ) const;
		DOUBLE  GetFraction                   (                                                                   ) const;

		void    stdRecurseCollectExtensionData( _Inout_    std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) const;
		
		_Pre_satisfies_( this->m_children._Myptr == nullptr ) 
		void    stdRecurseCollectExtensionData_FILE( _Inout_    std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) const;
		
		//unconditionally called only ONCE, so we ask for inlining.
		inline void    SetAttributes                 ( _In_ const DWORD attr );
		

		void    UpwardGetPathWithoutBackslash ( std::wstring& pathBuf ) const;

		_Pre_satisfies_( this->m_children._Myptr == nullptr ) 
			const std::wstring GetExtension             ( ) const;
		
		_Pre_satisfies_( this->m_children._Myptr == nullptr )
			PCWSTR const CStyle_GetExtensionStrPtr( ) const;
		
		_Pre_satisfies_( this->m_children._Myptr == nullptr )
		_Success_( SUCCEEDED( return ) )
			HRESULT      CStyle_GetExtension      (  WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_extension, const rsize_t strSize, _Out_ rsize_t& chars_written ) const;


		std::wstring GetPath                       ( ) const;

		//Branch only functions

		std::vector<CTreeListItem*> size_sorted_vector_of_children( ) const;

		
		_Pre_satisfies_( this->m_parent == NULL )
		void AddChildren( _In_ CTreeListControl* const tree_list_control );

		//data members - DON'T FUCK WITH LAYOUT! It's tweaked for good memory layout!

		//4,294,967,295 ( 4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		//We can exploit this fact to use a 4-byte unsigned integer for the size of the array, which saves us 4 bytes on 64-bit architectures
				  _Field_range_( 0, 4294967295 ) std::uint32_t                  m_childCount;
												 //C4820: 'CItemBranch' : '3' bytes padding added after data member 'CItemBranch::m_attr'
												 attribs                        m_attr;
					_Field_size_( m_childCount ) std::unique_ptr<CItemBranch[]> m_children;
		//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
		_Field_range_( 0, 18446744073709551615 ) std::uint64_t                  m_size;                // OwnSize
											     FILETIME                       m_lastChange;          // Last modification time OF SUBTREE
		                                 mutable SRECT                          m_rect;                // Finally, this is our coordinates in the Treemap view. (For GraphView)



		//static_assert( sizeof( m_attr ) <= 1, "" );

	};

INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 );


//If a class contains an unsized array, it cannot be used as the base class for another class. In addition, a class containing an unsized array cannot be used to declare any member except the last member of another class. A class containing an unsized array cannot have a direct or indirect virtual base class.
//The sizeof operator, when applied to a class containing an unsized array, returns the amount of storage required for all members except the unsized array. Implementors of classes that contain unsized arrays should provide alternate methods for obtaining the correct size of the class.
//You cannot declare arrays of objects that have unsized array components. Also, performing pointer arithmetic on pointers to such objects generates an error message.
//newer, more efficient allocation strategy will use a struct with the child count, child name heap manager, and an UNSIZED ARRAY as the children array.
//may need to merge CTreeListItem & CItemBranch to make this work in a clean manner.
//It'll have to use a struct that'll look something like this:
struct children_heap_block_allocation {
	children_heap_block_allocation( ) : m_childCount { 0u } { }
	children_heap_block_allocation( const children_heap_block_allocation& in ) = delete;
	children_heap_block_allocation& operator=( const children_heap_block_allocation& in ) = delete;

	~children_heap_block_allocation( ) {
		for ( size_t i = 0u; i < m_childCount; ++i ) {
			m_children[ i ].~CItemBranch( );
			}
		}

	_Field_range_( 0, 4294967295 )
		std::uint32_t                m_childCount;
		Children_String_Heap_Manager m_name_pool;
		//maybe we could also store the NON-NTFS-compressed folder size here?

#pragma warning( suppress: 4200 )//yes, this is Microsoft-specific
		CItemBranch                  m_children[ ];//unsized array is a MICROSOFT-SPECIFIC extension to C++ that emulates C's Flexible Array Member.
		//there are VERY active discussions in the C++ CWG (core working group) to develop some standardized version of array data members of runtime-bound.
	};

_At_( return, _Writable_bytes_( bytes_allocated ) )
_Ret_notnull_ children_heap_block_allocation* allocate_enough_memory_for_children_block( _In_ const std::uint32_t number_of_children, _Out_ size_t& bytes_allocated );


#endif
