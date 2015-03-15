// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once

#ifndef WDS_COM_HELPERS_H_INCLUDED
#define WDS_COM_HELPERS_H_INCLUDED

WDS_FILE_INCLUDE_MESSAGE


//several are from the "Show Shell Common File Dialog" sample

namespace {
	const COMDLG_FILTERSPEC c_rgSaveTypes[ ] = { { L"All folders", L"*" } };
	}




//
//   CLASS: CFileDialogEventHandler
//
//   PURPOSE: 
//   File Dialog Event Handler that responds to Events in Added Controls. The 
//   events handler provided by the calling process can implement 
//   IFileDialogControlEvents in addition to IFileDialogEvents. 
//   IFileDialogControlEvents enables the calling process to react to these events: 
//     1) PushButton clicked. 
//     2) CheckButton state changed. 
//     3) Item selected from a menu, ComboBox, or RadioButton list. 
//     4) Control activating. This is sent when a menu is about to display a 
//        drop-down list, in case the calling process wants to change the items in 
//        the list.
//
class CFileDialogEventHandler final : public IFileDialogEvents, public IFileDialogControlEvents {
	public:

	// IUnknown methods
	IFACEMETHODIMP QueryInterface( _In_ REFIID riid, _COM_Outptr_ void** ppv ) {
		static const QITAB qit[ ] =
			{
			QITABENT( CFileDialogEventHandler, IFileDialogEvents ),
			QITABENT( CFileDialogEventHandler, IFileDialogControlEvents ),
			{ 0 }
#pragma warning( suppress: 4365)
			};
		return QISearch( this, qit, riid, ppv );
		}

	IFACEMETHODIMP_( ULONG ) AddRef( ) {
#pragma warning( suppress: 4365 )
		return InterlockedIncrement( &m_cRef );
		}

	IFACEMETHODIMP_( ULONG ) Release( ) {
		const auto cRef = InterlockedDecrement( &m_cRef );
		if ( !cRef ) {
			delete this;
			}
#pragma warning( suppress: 4365 )
		return cRef;
		}

	// IFileDialogEvents methods
	//TODO: I think the MSDN sample might've done something wrong with all the S_OKs
	IFACEMETHODIMP OnFileOk( __RPC__in_opt IFileDialog* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnFolderChange( __RPC__in_opt IFileDialog* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnFolderChanging( __RPC__in_opt IFileDialog*, __RPC__in_opt IShellItem* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnHelp( IFileDialog* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnSelectionChange( __RPC__in_opt IFileDialog* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnTypeChange( __RPC__in_opt IFileDialog* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnShareViolation( __RPC__in_opt IFileDialog*, __RPC__in_opt IShellItem*, __RPC__out FDE_SHAREVIOLATION_RESPONSE* ) {
		return E_NOTIMPL;
		}
	IFACEMETHODIMP OnOverwrite( __RPC__in_opt IFileDialog*, __RPC__in_opt IShellItem*, __RPC__out FDE_OVERWRITE_RESPONSE* ) {
		return E_NOTIMPL;
		}

	// IFileDialogControlEvents methods
	IFACEMETHODIMP OnItemSelected( __RPC__in_opt IFileDialogCustomize* /*pfdc*/, DWORD /*dwIDCtl*/, DWORD /*dwIDItem*/ ) {
		return E_NOTIMPL;
		}


	IFACEMETHODIMP OnButtonClicked( __RPC__in_opt IFileDialogCustomize*, DWORD ) {
		return S_OK;
		}
	IFACEMETHODIMP OnControlActivating( __RPC__in_opt IFileDialogCustomize*, DWORD ) {
		return S_OK;
		}
	IFACEMETHODIMP OnCheckButtonToggled( __RPC__in_opt IFileDialogCustomize*, DWORD, BOOL ) {
		return S_OK;
		}

	CFileDialogEventHandler( ) : m_cRef { 1 } { }

	protected:

	~CFileDialogEventHandler( ) { }
	long m_cRef;
#pragma warning( suppress: 4265 )
	};




//https://code.msdn.microsoft.com/CppShellCommonFileDialog-17b20409/sourcecode?fileId=52757&pathId=1435971692
const HRESULT CFileDialogEventHandler_CreateInstance( _In_ REFIID riid, _COM_Outptr_ void** ppv );


//Keeping OnOpenAFolder in the implementation file means that we don't need to include ScopeGuard.h in the header.
std::wstring OnOpenAFolder( HWND /*hWnd*/ );



#else

#endif