#pragma once

#include "stdafx.h"



#ifndef WDS_DIRECTOY_ENUMERATION_H
#define WDS_DIRECTOY_ENUMERATION_H

#pragma message( "Including `" __FILE__ "`..." )

#include "datastructures.h"

class CTreeListItem;
class CDirstatApp;


//defined in directory_enumeration.cpp, in an anonymous namespace.
//struct FILEINFO;






//_Pre_satisfies_( !ThisCItem->m_attr.m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, std::wstring>>>    readJobNotDoneWork            ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app );

DOUBLE    DoSomeWorkShim                ( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse = false );





_Success_( return < UINT64_ERROR )
const std::uint64_t get_uncompressed_file_size( const CTreeListItem* const item );

#else

#endif