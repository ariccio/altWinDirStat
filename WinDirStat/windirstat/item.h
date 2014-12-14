// item.h	- Declaration of CItemBranch
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#pragma once
#include "stdafx.h"
#include "Treelistcontrol.h"

#ifndef ITEM_H
#define ITEM_H

void AddFileExtensionData( _Out_ _Pre_satisfies_( (extensionRecords._Mylast - extensionRecords._Myfirst) == 0 ) std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap );

class CItemBranch;//God I hate C++
class CTreeListItem;
class CDirstatApp;

void    FindFilesLoop                 ( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring& path );

std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>> addFiles_returnSizesToWorkOn( _In_ CItemBranch* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path );

_Pre_satisfies_( !ThisCItem->m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>>    readJobNotDoneWork            ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app );

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) void    DoSomeWorkShim                ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse = false );
_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) int     DoSomeWork                    ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse = false );

class CItemBranch : public CTreeListItem {
	/*
	  CItemBranch. This is the object, from which the whole tree is built.
	  For every directory, file etc., we find on the Harddisks, there is one CItemBranch.
	  It is derived from CTreeListItem because it _may_ become "visible" and therefore may be inserted in the TreeList view (we don't clone any data).
 
	*/
	static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "Bad parameter size! Check all functions that accept an unsigned long long or a std::uint64_t!" );

	public:
		CItemBranch  ( ITEMTYPE type, _In_ std::wstring name, std::uint64_t size, FILETIME time, DWORD attr, bool done );
		
#ifdef ARRAYTEST
		CItemBranch( ) : m_size( 0 ), m_type( IT_FILE ), m_name( L"" ), m_done( false ), m_rect( 0, 0, 0, 0 ), m_children( nullptr ), m_childCount( 0 ) { }
#endif

		virtual ~CItemBranch (                                                         );

		//Don't copy these bastards around
		CItemBranch( CItemBranch& in )  = delete;



//		CItemBranch( CItemBranch&& in ) {
//			const auto parentIn = in.m_parent;
//			auto locInParent = SIZE_T_MAX;
//			if ( parentIn != NULL ) {
//				locInParent = findItemInChildren( &in );
//				if ( locInParent != SIZE_T_MAX ) {
//#ifdef ARRAYTEST
//
//#else
//
//#endif
//					}
//				}
//#ifdef ARRAYTEST
//			const auto childrenSize = in.m_childCount;
//
//#else
//			const auto childrenSize = in.m_children.size( );
//#endif
//			}

		_Success_( return < SIZE_T_MAX )
		size_t findItemInChildren( const CItemBranch* const theItem ) const {
#ifdef ARRAYTEST
			const auto childrenSize = m_childCount;
			for ( size_t i = 0; i < childrenSize; ++i ) {
				if ( ( ( m_children + i ) ) == theItem ) {
					return i;
					}
				}
#else
			const auto childrenSize = m_children.size( );
			for ( size_t i = 0; i < childrenSize; ++i ) {
				if ( m_children[ i ] == theItem ) {
					return i;
					}
				}
#endif
			return SIZE_T_MAX;
			}

		bool operator<( const CItemBranch& rhs ) const {
			return size_recurse( ) < rhs.size_recurse( );
			}

		void refresh_sizeCache( ) const {
			if ( m_type == IT_FILE ) {
				return;
				}
			if ( m_vi != NULL ) {
				if ( m_vi->sizeCache != UINT64_ERROR ) {
					m_vi->sizeCache = UINT64_ERROR;
					m_vi->sizeCache = size_recurse( );
					}
				}
			}

		_Ret_range_( 0, UINT64_MAX )
		std::uint64_t size_recurse( ) const {
			if ( m_type == IT_FILE ) {
				return m_size;
				}
			if ( m_vi != NULL ) {
				if ( m_vi->sizeCache != UINT64_ERROR ) {
					return m_vi->sizeCache;
					}
				}
			std::uint64_t total = m_size;
#ifdef ARRAYTEST
			const auto childCount = m_childCount;
#else
			const auto childCount = m_children.size( );
#endif
			//ASSERT( m_childCount == childCount );
			for ( size_t i = 0; i < childCount; ++i ) {
#ifdef ARRAYTEST
				total += ( m_children + ( i ) )->size_recurse( );
#else
				//using operator[ ] here because performance is critical
				total += m_children[ i ]->size_recurse( );
#endif
				}
			if ( m_vi != NULL ) {
				if ( m_vi->sizeCache == UINT64_ERROR ) {
					if ( total != 0 ) {
						m_vi->sizeCache = total;
						}
					}
				}
			return total;
			}

		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		_Ret_range_( 0, 4294967295 )
		std::uint32_t files_recurse( ) const {
			std::uint32_t total = 0;
#ifdef ARRAYTEST
			const auto childCount = m_childCount;
#else
			const auto childCount = m_children.size( );
#endif		//ASSERT( m_childCount == childCount );
			for ( size_t i = 0; i < childCount; ++i ) {
#ifdef ARRAYTEST
				total += ( m_children + ( i ) )->files_recurse( );
#else
				//using operator[ ] here because performance is critical
				total += m_children[ i ]->files_recurse( );
#endif
				}
			total += 1;
			return total;
			}

		FILETIME FILETIME_recurse( ) const;
		virtual COLORREF         ItemTextColor           ( ) const override final;
#ifdef ARRAYTEST
		virtual size_t           GetChildrenCount        ( ) const override final { return m_childCount; }
#else
		virtual size_t           GetChildrenCount        ( ) const override final { return m_children.size( ); }
#endif

		virtual std::wstring     Text                    ( _In_ _In_range_( 0, 7 ) const INT subitem ) const override final;
		
		//_When_( return == STRSAFE_E_INSUFFICIENT_BUFFER, _At_( sizeBuffNeed, _Out_ ) )
		virtual HRESULT          Text_WriteToStackBuffer ( _In_range_( 0, 7 ) const INT subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_text, const rsize_t strSize, rsize_t& sizeBuffNeed ) const override;
		INT CompareSibling                           ( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const;
#ifdef ITEM_DRAW_SUBITEM
		//virtual INT              GetImageToCache     ( ) const override;
		virtual bool             DrawSubitem         ( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem, _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* width, _Inout_ INT* focusLeft ) const;
		        COLORREF         GetPercentageColor  (                                          ) const;
#endif

		void             TmiSetRectangle     ( _In_ const CRect& rc          ) const;
		CRect            TmiGetRectangle     (                               ) const;

		// Branch/Leaf shared functions
		_Must_inspect_result_ _Ret_maybenull_    CItemBranch* GetParent                         (                                                  ) const { return static_cast< CItemBranch* >( m_parent ); };

		INT     GetSortAttributes             (                                                                   ) const;
		
		//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx : Note  The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be expanded to a longer string by the system at run time, and this expansion applies to the total length.
		_Ret_range_( 0, 33000 ) DOUBLE  averageNameLength             (                                                                   ) const;
		DOUBLE  GetFraction                   (                                                                   ) const;

		void    stdRecurseCollectExtensionData( _Inout_    std::map<std::wstring, SExtensionRecord>& extensionMap ) const;
		
		_Pre_satisfies_( this->m_type == IT_FILE )
		void    stdRecurseCollectExtensionData_FILE( _Inout_    std::map<std::wstring, SExtensionRecord>& extensionMap ) const;
		void    SetAttributes                 ( _In_ const DWORD         attr                                );
		std::wstring GetPath                       ( ) const;

		void    UpwardGetPathWithoutBackslash ( std::wstring& pathBuf ) const;

		_Pre_satisfies_(  this->m_type   == IT_FILE      )                                  const std::wstring GetExtension             ( ) const;
		_Pre_satisfies_(  this->m_type   == IT_FILE      )                                        PCWSTR       CStyle_GetExtensionStrPtr( ) const;
		_Pre_satisfies_(  this->m_type   == IT_FILE      ) _Success_( SUCCEEDED( return ) )       HRESULT      CStyle_GetExtension      (  _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_extension, const rsize_t strSize ) const;
		_Post_satisfies_( return->m_type == IT_DIRECTORY )                                        CItemBranch* AddDirectory             ( std::wstring thisFilePath, DWORD thisFileAttributes, std::wstring thisFileName, FILETIME thisFileTime );


		std::wstring GetTextCOL_ATTRIBUTES( ) const;
		std::wstring GetTextCOL_LASTCHANGE( ) const;
		std::wstring GetTextCOL_FILES( ) const;
		std::wstring GetTextCOL_ITEMS ( ) const;
		std::wstring GetTextCOL_PERCENTAGE( ) const;

	public:
		//Branch only functions
		//_Pre_satisfies_ isn't actually useful for static analysis, but including anyways
		CItemBranch* AddChild        ( _In_ _Post_satisfies_( child->m_parent == this ) CItemBranch*       const child       );
		void SortAndSetDone          (                                           );

		_Ret_notnull_ CItemBranch*    GetChildGuaranteedValid ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const;
		_Ret_notnull_ CItemBranch*    TmiGetChild             ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t c ) const { return GetChildGuaranteedValid( c ); }

		bool IsAncestorOf                ( _In_ const CItemBranch& item     ) const;
		
		_Pre_satisfies_( this->m_parent == NULL ) void AddChildren( );

		//Functions that should be virtually overridden for a Leaf
		//these `Has` and `Is` functions should be virtual when refactoring as branch
		
		bool IsTreeDone                      (                                  ) const { return m_done; };
	
		//data members//DON'T FUCK WITH LAYOUT! It's tweaked for good memory layout!
		

	public:
#ifdef PLACEMENT_NEW_DEBUGGING
												 char                           m_beginSentinel[ 6 ];
#endif
#ifdef ARRAYTEST
												 size_t                         m_childCount;
					_Field_size_( m_childCount ) CItemBranch*                   m_children;
												 std::vector<CItemBranch*>      m_children_vector;
#else
												 std::vector<CItemBranch*>      m_children;
#endif

		//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
		_Field_range_( 0, 18446744073709551615 ) std::uint64_t                  m_size;                // OwnSize, if IT_FILE or IT_FREESPACE, or IT_UNKNOWN; SubtreeTotal else.
		                                         ITEMTYPE                       m_type;                // Indicates our type. See ITEMTYPE.
												 attribs                        m_attr;
												 bool                           m_done        : 1;     // Whole Subtree is done.
		                                         std::wstring                   m_name;                // Display name
											     FILETIME                       m_lastChange;          // Last modification time OF SUBTREE
		                                 mutable SRECT                          m_rect;                // Finally, this is our coordinates in the Treemap view. (For GraphView)
#ifdef PLACEMENT_NEW_DEBUGGING
												 char                           m_bye[ 4 ];
#endif

	};


	_Ret_maybenull_ CItemBranch* const FindCommonAncestor( _In_ _Pre_satisfies_( item1->m_type != IT_FILE ) const CItemBranch* const item1, _In_ const CItemBranch& item2 );

	INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 );

#endif
