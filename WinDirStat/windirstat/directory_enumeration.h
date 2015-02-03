#pragma once

#include "stdafx.h"
#include "item.h"


#ifndef WDS_DIRECTOY_ENUMERATION_H
#define WDS_DIRECTOY_ENUMERATION_H

void    FindFilesLoop                 ( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring path );

std::vector<std::pair<CItemBranch*, std::wstring>> addFiles_returnSizesToWorkOn( _In_ CItemBranch* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path );

_Pre_satisfies_( !ThisCItem->m_attr.m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, std::wstring>>>    readJobNotDoneWork            ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app );

DOUBLE    DoSomeWorkShim                ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse = false );


//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
void DoSomeWork                    ( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse = false );

#else

#endif