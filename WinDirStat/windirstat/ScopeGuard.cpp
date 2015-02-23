#include "stdafx.h"
#include "ScopeGuard.h"

#ifndef WDS_SCOPEGUARD_CPP_INCLUDED
#define WDS_SCOPEGUARD_CPP_INCLUDED

//intentionally NOT defined as part of ScopeGuard, to reduce code duplication. //Also, produces cleaner `TRACE` output.
void trace_out( _In_z_ PCSTR const file_name, _In_z_ PCSTR const func_name, _In_ _In_range_( 0, INT_MAX ) const int line_number ) {
	TRACE( L"Scope guard triggered!"
			L"\r\n\t\tScope guard initialized in file: `%S`,"
			L"\r\n\t\tfunction:                        `%S`,"
			L"\r\n\t\tline:                            `%i`\r\n", file_name, func_name, line_number );
	}

#else

#endif