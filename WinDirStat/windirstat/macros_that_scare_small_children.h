// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#pragma once

#ifndef MACROS_THAT_SCARE_SMALL_CHILDREN_H_INCLUDED
#define MACROS_THAT_SCARE_SMALL_CHILDREN_H_INCLUDED

WDS_FILE_INCLUDE_MESSAGE


#ifndef WDS_WRITES_TO_STACK
#define WDS_WRITES_TO_STACK( strSize, chars_written ) _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) _Pre_satisfies_( strSize >= chars_written ) _Post_satisfies_( _Old_( chars_written ) <= chars_written )
#else
#error already defined!
#endif

#ifndef WDS_SCOPEGUARD_INSTANCE
#define WDS_SCOPEGUARD_INSTANCE( func ) scopeGuard( (func), __FILE__, __FUNCSIG__, __LINE__ )
#else
#error already defined!!
#endif

#ifndef WDS_ASSERT_NEVER_REACHED
//this line of code should NEVER be reached. ASSERT( false ) on reaching in Debug build.
#define WDS_ASSERT_NEVER_REACHED( ) ASSERT( false )
#else
#error already defined??!?
#endif



#ifndef WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT

//I don't think anything in strsafe actually ever returns STRSAFE_E_END_OF_FILE,
//so I use this after I've handled the other error conditions (STRSAFE_E_INSUFFICIENT_BUFFER, STRSAFE_E_INVALID_PARAMETER),
//to catch unexpected errors. NOTE that these are still handled by the calling function via SUCCESSS( ),
//but this macro helps catch the issue closer to the function that returned the unexpected value;
#define WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res ) {                                                \
	static_assert( SUCCEEDED( S_OK ), "This macro depends on SUCCEEDED( S_OK ) returning true" );                 \
	static_assert( std::is_same<decltype( res ), const HRESULT>::value, "This macro depends on an HRESULT res" ); \
	ASSERT( ( res ) != STRSAFE_E_END_OF_FILE );                                                                   \
	ASSERT( FAILED( res ) );                                                                                      \
	ASSERT( !SUCCEEDED( res ) );                                                                                  \
	}

#else
#error already defined!
#endif



#ifndef WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER
//std::terminate( )s if ( res == STRSAFE_E_INVALID_PARAMETER ), as this is usually an issue with an incorrect compile-time constant.
//Is a macro and not an inline function because of the use of file name, function signature, and line number.
#define WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, strsafe_func_name )                                             \
	if ( ( res ) == STRSAFE_E_INVALID_PARAMETER ) {                                                                   \
		static_assert( std::is_same<decltype( res ), const HRESULT>::value, "This macro depends on an HRESULT res" ); \
		unexpected_strsafe_invalid_parameter_handler( ( strsafe_func_name ), __FILE__, __FUNCSIG__, __LINE__ );       \
		}
#else
#error already defined!!
#endif


#ifndef WDS_DECLSPEC_NOTHROW
#define WDS_DECLSPEC_NOTHROW __declspec(nothrow)
#else
#error already defined!
#endif

#define WDS_INT_BASE 1024
#define WDS_HALF_BASE WDS_INT_BASE/2

#endif