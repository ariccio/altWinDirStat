#include "stdafx.h"
#include "ScopeGuard.h"

#ifndef WDS_SCOPEGUARD_CPP_INCLUDED
#define WDS_SCOPEGUARD_CPP_INCLUDED

#pragma message( "Including `" __FILE__ "`..." )

//intentionally NOT defined as part of ScopeGuard, to reduce code duplication. //Also, produces cleaner `TRACE` output.
#ifdef DEBUG
void trace_out( _In_z_ PCSTR const file_name, _In_z_ PCSTR const func_name, _In_ _In_range_( 0, INT_MAX ) const int line_number ) {
#ifdef WDS_SCOPE_GUARD_DEBUGGING
	TRACE( L"Scope guard triggered!"
			L"\r\n\t\tScope guard initialized in file: `%S`,"
			L"\r\n\t\tfunction:                        `%S`,"
			L"\r\n\t\tline:                            `%i`\r\n", file_name, func_name, line_number );
#else
	UNREFERENCED_PARAMETER( file_name );
	UNREFERENCED_PARAMETER( func_name );
	UNREFERENCED_PARAMETER( line_number );
#endif
	}
#endif

#else

#endif