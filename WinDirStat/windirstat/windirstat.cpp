// windirstat.cpp	- Implementation of CDirstatApp and some globals
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


#ifndef WDS_WINDIRSTAT_CPP
#define WDS_WINDIRSTAT_CPP

WDS_FILE_INCLUDE_MESSAGE


#include "macros_that_scare_small_children.h"
#include "graphview.h"
//#include "SelectDrivesDlg.h"
#include "dirstatdoc.h"
#include "options.h"
#include "windirstat.h"
#include "mainframe.h"
#include "globalhelpers.h"
#include "ScopeGuard.h"
#include "COM_helpers.h"

#include "stringformatting.h"


CMainFrame* GetMainFrame( ) {
	// Not: `return (CMainFrame *)AfxGetMainWnd();` because CWinApp::m_pMainWnd is set too late.
	return CMainFrame::GetTheFrame( );
	}

CDirstatApp* GetApp( ) {
	return static_cast< CDirstatApp* >( AfxGetApp( ) );
	}


namespace {

#ifdef DEBUG
	void setFlags( ) {
		const auto flag = ::_CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n\t_CRTDBG_DELAY_FREE_MEM_DEF: %i\r\n" ), flag, ( flag & _CRTDBG_ALLOC_MEM_DF ), ( flag & _CRTDBG_CHECK_CRT_DF ), ( flag & _CRTDBG_LEAK_CHECK_DF ), ( flag & _CRTDBG_DELAY_FREE_MEM_DF ) );
		
		::_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF bitor _CRTDBG_LEAK_CHECK_DF );
		const auto flag2 = ::_CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n\t_CRTDBG_DELAY_FREE_MEM_DEF: %i\r\n" ), flag2, ( flag2 & _CRTDBG_ALLOC_MEM_DF ), ( flag2 & _CRTDBG_CHECK_CRT_DF ), ( flag2 & _CRTDBG_LEAK_CHECK_DF ), ( flag2 & _CRTDBG_DELAY_FREE_MEM_DF )  );
		}
#endif

	/*
WINBASEAPI
BOOL
WINAPI
SetProcessMitigationPolicy(
    _In_ PROCESS_MITIGATION_POLICY MitigationPolicy,
    _In_reads_bytes_(dwLength) PVOID lpBuffer,
    _In_ SIZE_T dwLength
    );

	*/


	typedef WINBASEAPI BOOL( WINAPI* SetProcessMitigationPolicy_t )( _In_ _Const_ PROCESS_MITIGATION_POLICY MitigationPolicy, _In_reads_bytes_( dwLength ) _Const_ PVOID lpBuffer, _In_ _Const_ SIZE_T dwLength );


	struct HMODULE_RAII final {
		HMODULE_RAII( _In_ std::pair<HMODULE, BOOL> pair_in ) : the_module { pair_in.first }, result { pair_in.second } { }
		~HMODULE_RAII( ) {
			if ( result == 0 ) {
				return;
				}
			const BOOL free_result = ::FreeLibrary( the_module );
			ASSERT( free_result != 0 );
			if ( free_result != 0 ) {
				return;
				}
			displayWindowsMsgBoxWithMessage( L"FreeLibrary( the_module ) failed!" );
			displayWindowsMsgBoxWithError( );
			}
		DISALLOW_COPY_AND_ASSIGN( HMODULE_RAII );
		const HMODULE the_module;
		const BOOL    result;
		};

	void handle_mitigation_failure_doublefault( _In_z_ PCWSTR const mitigation_specific_error_message, _In_z_ PCWSTR const non_mitigation_specific_message ) {
		displayWindowsMsgBoxWithMessage( non_mitigation_specific_message );
		displayWindowsMsgBoxWithMessage( mitigation_specific_error_message );
		}

	//CStyle_GetLastErrorAsFormattedMessage appends a newline (`\r\n`). Clobber that new fucking line:
	void clobber_the_damned_new_line( _Inout_z_ PWSTR str_err_buff, _In_ const rsize_t chars_written ) {
		if ( chars_written > 1 ) {
			str_err_buff[ chars_written - 1 ] = 0;
			str_err_buff[ chars_written - 2 ] = 0;
			}
		}

	std::wstring get_last_error_no_newline( _In_z_ PCWSTR const mitigation_specific_error_message ) {
		const rsize_t str_buff_size = 256u;
		wchar_t str_err_buff[ str_buff_size ] = { 0 };
		rsize_t chars_written_1 = 0u;
		const HRESULT err_fmt_res = CStyle_GetLastErrorAsFormattedMessage( str_err_buff, str_buff_size, chars_written_1 );
		ASSERT( SUCCEEDED( err_fmt_res ) );
		if ( !SUCCEEDED( err_fmt_res ) ) {
			handle_mitigation_failure_doublefault( mitigation_specific_error_message, L"Ran into an error while formatting another error! This happened in the handler for enhanced-security mitigation initializations!" );
			return std::wstring( L"" );
			}
		clobber_the_damned_new_line( str_err_buff, chars_written_1 );
		return std::wstring( str_err_buff );
		}

	//TODO: BUGBUG: refactor when not half-asleep
	void handle_mitigation_enable_failure( _In_z_ PCWSTR const mitigation_specific_error_message ) {
		const std::wstring dyn_str( mitigation_specific_error_message + get_last_error_no_newline( mitigation_specific_error_message ) + L" It's totally safe to continue execution (we will), but if you see this, please report it to me." );
		//dyn_str += L" It's totally safe to continue execution (we will), but if you see this, please report it to me.";
		displayWindowsMsgBoxWithMessage( dyn_str.c_str( ) );
		}


	//Tell windows that we WANT to crash if the shit hits the fan. This is a security feature.
	void enable_heap_security_crash_on_corruption( ) {

		//If the function succeeds, the return value is nonzero.
		BOOL heap_set_info_result = ::HeapSetInformation( NULL, HeapEnableTerminationOnCorruption, NULL, 0u );
		if ( heap_set_info_result == 0 ) {
			TRACE( _T( "HeapSetInformation failed!\r\n" ) );
			}
		else {
			TRACE( _T( "Enabled HeapEnableTerminationOnCorruption!\r\n" ) );
			}
		}


	void enable_ASLR_mitigation( SetProcessMitigationPolicy_t SetProcessMitigationPolicy_f ) {
		PROCESS_MITIGATION_ASLR_POLICY ASLR_policy = { 0 };
		ASLR_policy.EnableBottomUpRandomization = true;
		ASLR_policy.EnableForceRelocateImages   = true;
#ifdef _WIN64
		ASLR_policy.EnableHighEntropy           = true;
#endif
		ASLR_policy.DisallowStrippedImages = true;
		
		auto guard = WDS_SCOPEGUARD_INSTANCE( [&]{ handle_mitigation_enable_failure( L"Failed to set enhanced/forced ASLR: " ); } );

		const BOOL set_aslr_policy_res = SetProcessMitigationPolicy_f( ProcessASLRPolicy, &ASLR_policy, sizeof( ASLR_policy ) );
		if ( set_aslr_policy_res == TRUE ) {
			TRACE( _T( "Successfully enabled bottom-up randomization, forcible image relocation, and refusal to load images without a `.reloc` section (DisallowStrippedImages).\r\n" ) );
			guard.dismiss( );
			return;
			}
		}

	//This one seems to be causing trouble. Disable for now.
	void enable_DEP_mitigation( SetProcessMitigationPolicy_t SetProcessMitigationPolicy_f ) {
		PROCESS_MITIGATION_DEP_POLICY DEP_policy = { 0 };
		DEP_policy.Enable                   = true;
		DEP_policy.Permanent                = true;
		DEP_policy.DisableAtlThunkEmulation = true;

		auto guard = WDS_SCOPEGUARD_INSTANCE( [ &] { handle_mitigation_enable_failure( L"Failed to set enhanced/forced DEP: " ); } );

		const BOOL set_DEP_policy_res = SetProcessMitigationPolicy_f( ProcessDEPPolicy, &DEP_policy, sizeof( DEP_policy ) );
		if ( set_DEP_policy_res == TRUE ) {
			TRACE( _T( "Successfully enabled Permanent DEP, and successfully disabled AtlThunkEmulation.\r\n" ) );
			guard.dismiss( );
			return;
			}
		}


	void enable_EXTENSION_POINT_mitigation( SetProcessMitigationPolicy_t SetProcessMitigationPolicy_f ) {
		PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY EXTEND_POINT_policy = { 0 };
		EXTEND_POINT_policy.DisableExtensionPoints = true;
		
		auto guard = WDS_SCOPEGUARD_INSTANCE( [ &] { handle_mitigation_enable_failure( L"Failed to disable insecure Extension Points: " ); } );

		const BOOL set_EXTEND_policy_res = SetProcessMitigationPolicy_f( ProcessExtensionPointDisablePolicy, &EXTEND_POINT_policy, sizeof( EXTEND_POINT_policy ) );
		if ( set_EXTEND_policy_res == TRUE ) {
			TRACE( _T( "Successfully disabled Extension Points, ancient & insecure extension points are now forbidden.\r\n" ) );
			guard.dismiss( );
			return;
			}
		}

	void enable_strict_HANDLE_check_mitigation( SetProcessMitigationPolicy_t SetProcessMitigationPolicy_f ) {
		PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY HANDLE_policy = { 0 };
		HANDLE_policy.RaiseExceptionOnInvalidHandleReference = true;
		HANDLE_policy.HandleExceptionsPermanentlyEnabled = true;

		auto guard = WDS_SCOPEGUARD_INSTANCE( [&]{ handle_mitigation_enable_failure( L"Failed to enable strict invalid handle checking: " ); } );

		const BOOL set_HANDLE_policy_res = SetProcessMitigationPolicy_f( ProcessStrictHandleCheckPolicy, &HANDLE_policy, sizeof( HANDLE_policy ) );
		if ( set_HANDLE_policy_res == TRUE ) {
			TRACE( _T( "Successfully enabled strict invalid handle checking.\r\n" ) );
			guard.dismiss( );
			return;
			}
		}

	std::pair<const HMODULE, const BOOL> init_kernel32( ) {
		HMODULE kernel32_temp;
		const BOOL module_handle_result = ::GetModuleHandleExW( 0, L"kernel32.dll", &kernel32_temp );
		if ( module_handle_result == 0 ) {
			TRACE( _T( "Failed to get handle to kernel32.dll!\r\n" ) );
			}
		return std::make_pair( kernel32_temp, module_handle_result );
		}

	//Security is a matter determined by the weakest link in the chain. Let's NOT be that link. Let's be respectful of our operating environment.
	void enable_aggressive_process_mitigations( ) {
		
		HMODULE_RAII module_scope_manager { init_kernel32( ) };
		if ( module_scope_manager.result == 0 ) {
			return;
			}
		const SetProcessMitigationPolicy_t SetProcessMitigationPolicy_f = reinterpret_cast< SetProcessMitigationPolicy_t >( GetProcAddress( module_scope_manager.the_module, "SetProcessMitigationPolicy" ) );

		ASSERT( ::IsWindows8OrGreater( ) );
		enable_ASLR_mitigation( SetProcessMitigationPolicy_f );
		
		
		//this one seems to be causing trouble.
		//enable_DEP_mitigation( SetProcessMitigationPolicy_f );
		
		
		enable_EXTENSION_POINT_mitigation( SetProcessMitigationPolicy_f );
		enable_strict_HANDLE_check_mitigation( SetProcessMitigationPolicy_f );
		//TODO:
			//ProhibitDynamicCode: https://msdn.microsoft.com/en-us/library/windows/desktop/mt706243.aspx
			//EnableControlFlowGuard: https://msdn.microsoft.com/en-us/library/windows/desktop/mt654121.aspx
			//

		//(Win 10 only):
			//MicrosoftSignedOnly:https://msdn.microsoft.com/en-us/library/windows/desktop/mt706242.aspx
			//DisableNonSystemFonts: https://msdn.microsoft.com/en-us/library/windows/desktop/mt706244.aspx

		}


	std::wstring test_file_open( ) {
		TRACE( _T( "Displaying shell folder selection dialog...\r\n" ) );
		return OnOpenAFolder( NULL );
		}

	}


// CDirstatApp

BEGIN_MESSAGE_MAP(CDirstatApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &( CDirstatApp::OnAppAbout ) )
	ON_COMMAND(ID_FILE_OPEN, &( CDirstatApp::OnFileOpen ) )
	ON_COMMAND(ID_FILE_NEW, &( CDirstatApp::OnFileOpenLight ) )
END_MESSAGE_MAP()


WTL::CAppModule _Module;	// add this line
CDirstatApp _theApp;


CDirstatApp::CDirstatApp( ) : m_workingSet( 0 ), m_lastPeriodicalRamUsageUpdate( GetTickCount64( ) ), m_altEncryptionColor( GetAlternativeColor( RGB( 0x00, 0x80, 0x00 ), _T( "AltEncryptionColor" ) ) ) { }

CDirstatApp::~CDirstatApp( ) {
	m_pDocTemplate = { NULL };
	}

// Get the alternative colors for compressed and encrypted files/folders. This function uses either the value defined in the Explorer configuration or the default color values.
_Success_( return != clrDefault ) COLORREF CDirstatApp::GetAlternativeColor( _In_ const COLORREF clrDefault, _In_z_ PCWSTR const which ) {
	COLORREF x;
	ULONG cbValue = sizeof( x );
	ATL::CRegKey key;

	// Open the explorer key
	key.Open( HKEY_CURRENT_USER, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" ), KEY_READ );

	// Try to read the REG_BINARY value
	if ( ERROR_SUCCESS == key.QueryBinaryValue( which, &x, &cbValue ) ) {
		return x;
		}
	return clrDefault;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CDirstatApp::GetCurrentProcessMemoryInfo( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_usage, _In_range_( 50, 64 ) const rsize_t strSize ) {
	const auto Memres = UpdateMemoryInfo( );
	if ( !Memres ) {
		wds_fmt::write_MEM_INFO_ERR( psz_formatted_usage );
		return STRSAFE_E_INVALID_PARAMETER;
		}
	wds_fmt::write_RAM_USAGE( psz_formatted_usage );
	rsize_t chars_written = 0;
	rsize_t size_buff_needed = 0;
	const HRESULT res = wds_fmt::FormatBytes( m_workingSet, &( psz_formatted_usage[ 11 ] ), ( strSize - 12 ), chars_written, size_buff_needed );
	if ( !SUCCEEDED( res ) ) {
		return StringCchPrintfW( psz_formatted_usage, strSize, L"RAM Usage: %s", wds_fmt::FormatBytes( m_workingSet, GetOptions( )->m_humanFormat ).c_str( ) );
		}
	return res;
	}

_Success_( return == true ) bool CDirstatApp::UpdateMemoryInfo( ) {
	//auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	PROCESS_MEMORY_COUNTERS pmc = { };

	pmc.cb = sizeof( pmc );

	if ( !::GetProcessMemoryInfo( ::GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		return false;
		}	

	m_workingSet = pmc.WorkingSetSize;

	return true;
	}

BOOL CDirstatApp::InitInstance( ) {
	//Program entry point

	TRACE( _T( "------>Program entry point!<------\r\n" ) );
	if ( IsWindows8OrGreater( ) ) {
		enable_heap_security_crash_on_corruption( );
		enable_aggressive_process_mitigations( );
		}

	//uses ~29K memory
	if ( !SUCCEEDED( ::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ) ) {
		::AfxMessageBox( _T( "CoInitializeEx Failed!" ) );
		return FALSE;
		}

	
#ifdef DEBUG
	setFlags( );
#endif

	// Initialize ATL
	_Module.Init( NULL, ::AfxGetInstanceHandle( ) );

	VERIFY( CWinApp::InitInstance( ) );
	::InitCommonControls( );          // InitCommonControls() is necessary for Windows XP.
	if ( ::AfxOleInit( ) == FALSE ) { // For SHBrowseForFolder()
		::AfxMessageBox( _T( "AfxOleInit Failed!" ) );
		return FALSE;
		}
	
	

	CWinApp::SetRegistryKey( _T( "Seifert" ) );
	//LoadStdProfileSettings( 4 );

	GetOptions( )->LoadFromRegistry( );
	
	m_pDocTemplate = new CSingleDocTemplate { IDR_MAINFRAME, RUNTIME_CLASS( CDirstatDoc ), RUNTIME_CLASS( CMainFrame ), RUNTIME_CLASS( CGraphView ) };
	if ( !m_pDocTemplate ) {
		return FALSE;
		}

	CWinApp::AddDocTemplate( m_pDocTemplate );
	
	CCommandLineInfo cmdInfo;
	CWinApp::ParseCommandLine( cmdInfo );

	m_nCmdShow = SW_HIDE;


	if ( !CWinApp::ProcessShellCommand( cmdInfo ) ) {
		return FALSE;
		}

	m_frameptr = GetMainFrame( );
	m_frameptr->m_appptr = this;
	
	
	m_frameptr->InitialShowWindow( );
	m_pMainWnd->UpdateWindow( );

	// When called by setup.exe, windirstat remained in the background, so we do a
	m_pMainWnd->BringWindowToTop( );
	m_pMainWnd->SetForegroundWindow( );

	//if ( cmdInfo.m_nShellCommand != CCommandLineInfo::FileOpen ) { <------WTF WAS GOING ON HERE!!! TODO: BUGBUG: WTF!!!
	if ( cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen ) {
		//OnFileOpen( );
		OnFileOpenLight( );
		}
	return TRUE;
	}

INT CDirstatApp::ExitInstance( ) {
	// Terminate ATL
	_Module.Term( );	
	const auto retval = CWinApp::ExitInstance( );
	return retval;
	}

void CDirstatApp::OnAppAbout( ) {
	displayWindowsMsgBoxWithMessage( global_strings::about_text );
	}

void CDirstatApp::OnFileOpen( ) {
	const auto path_str = test_file_open( );
	if ( !( path_str.empty( ) ) ) {
		m_pDocTemplate->OpenDocumentFile( path_str.c_str( ), true );
		}
	}

void CDirstatApp::OnFileOpenLight( ) {
	const UINT flags = ( BIF_RETURNONLYFSDIRS bitor BIF_USENEWUI bitor BIF_NONEWFOLDERBUTTON );
	WTL::CFolderDialog bob { NULL, global_strings::select_folder_dialog_title_text, flags };
	//ASSERT( m_folder_name_heap.compare( m_folderName ) == 0 );
	auto resDoModal = bob.DoModal( );
	if ( resDoModal == IDOK ) {
		PCWSTR const m_folder_name_heap( bob.GetFolderPath( ) );
		if ( wcslen( m_folder_name_heap ) > 0 ) {

			//Here, calls CSingleDocTemplate::OpenDocumentFile (in docsingl.cpp)
			m_pDocTemplate->OpenDocumentFile( m_folder_name_heap, TRUE );
			}
		}
	}

BOOL CDirstatApp::OnIdle( _In_ LONG lCount ) {
	BOOL more = FALSE;
	ASSERT( lCount >= 0 );
	const auto ramDiff = ( ::GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate );
	auto doc = GetDocument( );
	
	if ( doc != NULL ) {
		if ( !doc->Work( ) ) {
			//ASSERT( doc->m_workingItem != NULL );
			more = TRUE;
			}
		}
	if ( ramDiff > RAM_USAGE_UPDATE_INTERVAL ) {
		more = CWinApp::OnIdle( lCount );
		if ( !more ) {
			CDirstatApp::PeriodicalUpdateRamUsage( );
			}
		else {
			more = CWinThread::OnIdle( 0 );
			}
		}
	return more;
	}


#else

#endif