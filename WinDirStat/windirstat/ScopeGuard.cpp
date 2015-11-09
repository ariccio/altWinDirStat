// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#include "stdafx.h"
#include "ScopeGuard.h"

#ifndef WDS_SCOPEGUARD_CPP_INCLUDED
#define WDS_SCOPEGUARD_CPP_INCLUDED

WDS_FILE_INCLUDE_MESSAGE

//intentionally NOT defined as part of ScopeGuard, to reduce code duplication. //Also, produces cleaner `TRACE` output.
#ifdef WDS_SCOPE_GUARD_DEBUGGING
void trace_out( _In_z_ PCSTR const func_expr, _In_z_ PCSTR const file_name, _In_z_ PCSTR const func_name, _In_ _In_range_( 0, INT_MAX ) const int line_number ) {
	TRACE( L"Scope guard triggered!"
			L"\r\n\t\tScope guard initialized in file: `%S`,"
			L"\r\n\t\tfunction:                        `%S`,"
			L"\r\n\t\tline:                            `%i`,"
			L"\r\n\t\tActual expression:"
			L"\r\n%S\r\n",
			file_name, func_name, line_number, func_expr );
	}
#endif

#else

#endif