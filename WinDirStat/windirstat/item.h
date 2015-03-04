// item.h	- Declaration of CItemBranch
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once
#include "stdafx.h"


#ifndef WDS_ITEM_H
#define WDS_ITEM_H

//class CItemBranch final : public CTreeListItem {
//	/*
//	  CItemBranch. This is the object, from which the whole tree is built. For every directory, file etc., we find [in the folder selected], there is one CItemBranch.
//	*/
//	static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "Bad parameter size! Check all functions that accept an unsigned long long or a std::uint64_t!" );
//
//	public:
//		CItemBranch  ( const std::uint64_t size, const FILETIME time, const DWORD attr, const bool done, _In_ CItemBranch* const parent, _In_z_ _Readable_elements_( length ) PCWSTR const name, const std::uint16_t length ) : CTreeListItem{ std::move( size ), std::move( time ), std::move( attr ), std::move( done ), std::move( parent ), std::move( name ), std::move( length ) } { }
//		//std::move( size ), std::move( time ), std::move( attr ), std::move( done ), std::move( parent ), std::move( name ), std::move( length )
//
//
//		//default constructor DOES NOT initialize.
//		__forceinline CItemBranch ( ) { }
//
//		virtual ~CItemBranch( ) final = default;
//
//		CItemBranch& operator=( const CItemBranch& in ) = delete;
//		CItemBranch( CItemBranch& in )  = delete;
//	};






#endif
