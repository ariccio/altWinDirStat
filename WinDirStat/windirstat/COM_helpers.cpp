// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#include "stdafx.h"

#pragma once

#ifndef WDS_COM_HELPERS_CPP_INCLUDED
#define WDS_COM_HELPERS_CPP_INCLUDED


WDS_FILE_INCLUDE_MESSAGE

#include "COM_helpers.h"
#include "macros_that_scare_small_children.h"

#include "ScopeGuard.h"

//Keeping OnOpenAFolder in the implementation file means that we don't need to include ScopeGuard.h in the header.
std::wstring OnOpenAFolder( HWND /*hWnd*/ ) {
	IFileDialog* file_dialog = nullptr;
	const HRESULT create_file_dialog_result = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &file_dialog ) );
	if ( !SUCCEEDED( create_file_dialog_result ) ) {
		TRACE( _T( "create_file_dialog_result-> FAILED!\r\n" ) );
		return L"";
		}
	auto file_dialog_guard = WDS_SCOPEGUARD_INSTANCE( [ &] { file_dialog->Release( ); } );

	IFileDialogEvents* file_dialog_event = NULL;
	const HRESULT dialog_event_handler_create_instance_result = CFileDialogEventHandler_CreateInstance( IID_PPV_ARGS( &file_dialog_event ) );
	if ( !SUCCEEDED( dialog_event_handler_create_instance_result ) ) {
		TRACE( _T( "CFileDialogEventHandler_CreateInstance-> FAILED!\r\n" ) );
		return L"";
		}

	auto file_dialog_event_guard = WDS_SCOPEGUARD_INSTANCE( [ &] { file_dialog_event->Release( ); } );
	
	DWORD dwCookie_temp = 0;
	const HRESULT file_dialog_advise_cookie_result = file_dialog->Advise( file_dialog_event, &dwCookie_temp );
	if ( !SUCCEEDED( file_dialog_advise_cookie_result ) ) {
		TRACE( _T( "file_dialog->Advise FAILED!\r\n" ) );
		return L"";
		}

	const DWORD dwCookie = dwCookie_temp;

	auto file_dialog_advise_cookie_guard = WDS_SCOPEGUARD_INSTANCE( [ &] { file_dialog->Unadvise( dwCookie ); } );

	DWORD dialog_options_flags = 0;

	const HRESULT get_file_dialog_options_result = file_dialog->GetOptions( &dialog_options_flags );
	if ( !SUCCEEDED( get_file_dialog_options_result ) ) {
		TRACE( _T( "file_dialog->GetOptions FAILED!\r\n" ) );
		return L"";
		}

	const HRESULT set_file_dialog_options_result = file_dialog->SetOptions( dialog_options_flags bitor FOS_FORCEFILESYSTEM bitor FOS_PICKFOLDERS bitor FOS_FORCESHOWHIDDEN );
	if ( !SUCCEEDED( set_file_dialog_options_result ) ) {
		TRACE( _T( "file_dialog->SetOptions FAILED!\r\n" ) );
		return L"";
		}

	//const HRESULT set_file_dialog_types_result = file_dialog->SetFileTypes( ARRAYSIZE( c_rgSaveTypes ), c_rgSaveTypes );
	//if ( !SUCCEEDED( set_file_dialog_types_result ) ) {
	//	TRACE( _T( "file_dialog->SetFileTypes FAILED!\r\n" ) );
	//	return;
		//}

	//const HRESULT set_file_type_index_result = file_dialog->SetFileTypeIndex( 1 );
	//if ( !SUCCEEDED( set_file_type_index_result ) ) {
	//	TRACE( _T( "file_dialog->SetFileTypeIndex FAILED!\r\n" ) );
	//	return;
	//	}

	//const HRESULT set_file_type_default_extension = file_dialog->SetDefaultExtension( L"d" );
	//if ( !SUCCEEDED( set_file_type_default_extension ) ) {
	//	TRACE( _T( "file_dialog->SetDefaultExtension FAILED!\r\n" ) );
	//	return;
	//	}

	const HRESULT file_dialog_show_dialog_result = file_dialog->Show( NULL );
	if ( !SUCCEEDED( file_dialog_show_dialog_result ) ) {
		TRACE( _T( "file_dialog->Show FAILED!\r\n" ) );
		return L"";
		}

	IShellItem* shell_item_result = nullptr;
	const HRESULT file_dialog_get_result_result = file_dialog->GetResult( &shell_item_result );
	if ( !SUCCEEDED( file_dialog_get_result_result ) ) {
		TRACE( _T( "file_dialog->GetResult FAILED!\r\n" ) );
		return L"";
		}

	auto psi_result_guard = WDS_SCOPEGUARD_INSTANCE( [ &] { shell_item_result->Release( ); } );

	PWSTR file_path_temp = nullptr;
	const HRESULT shell_item_get_display_name_result = shell_item_result->GetDisplayName( SIGDN_FILESYSPATH, &file_path_temp );
	if ( !SUCCEEDED( shell_item_get_display_name_result ) ) {
		TRACE( _T( "shell_item_result->GetDisplayName FAILED!\r\n" ) );
		return L"";
		}
	
	PWSTR const file_path = file_path_temp;

	auto file_path_guard = WDS_SCOPEGUARD_INSTANCE( [ &] { CoTaskMemFree( file_path ); } );
	std::wstring path_string( file_path );
	return path_string;

	//const HRESULT task_dialog_result = TaskDialog( NULL, NULL, L"CommonFileDialogApp", file_path, NULL, TDCBF_OK_BUTTON, TD_INFORMATION_ICON, NULL );
	//if ( !SUCCEEDED( task_dialog_result ) ) {
	//	TRACE( _T( "TaskDialog FAILED!\r\n" ) );
	//	return;
	//	}


	//TRACE( _T( "OpenAFolder is returning SUCESSFULLY!\r\n" ) );
	}


//https://code.msdn.microsoft.com/CppShellCommonFileDialog-17b20409/sourcecode?fileId=52757&pathId=1435971692
const HRESULT CFileDialogEventHandler_CreateInstance( _In_ REFIID riid, _COM_Outptr_ void** ppv ) {
	*ppv = NULL;
	auto pFileDialogEventHandler = new CFileDialogEventHandler( );
	const HRESULT file_dialog_event_handler_query_interface_result = pFileDialogEventHandler->QueryInterface( riid, ppv );
	pFileDialogEventHandler->Release( );
	return file_dialog_event_handler_query_interface_result;
	}




#else

#endif