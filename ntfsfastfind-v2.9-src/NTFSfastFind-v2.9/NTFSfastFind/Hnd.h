// ------------------------------------------------------------------------------------------------
// Smart file handle class. Close when it goes out of scope.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>


class Hnd {
public:
	Hnd( HANDLE handle = INVALID_HANDLE_VALUE ) : m_handle( handle ) { }

	~Hnd( ) {
		if ( IsValid( ) ) {
			CloseHandle( m_handle );
			}
		}

	Hnd& operator=( HANDLE other ) {
		if ( other != m_handle && IsValid( ) ) {
			CloseHandle( m_handle );
			}
		m_handle = other;
		return *this;
		}

#if 0
	HANDLE Duplicate() const
	{
		if (IsValid())
		{
			HANDLE handle;
			if (::DuplicateHandle(
					::GetCurrentProcess(), 
					m_handle, 
					::GetCurrentProcess(),
					&handle, 
					0,
					FALSE,
					DUPLICATE_SAME_ACCESS))
			{
				return handle;
			}
		}

		return INVALID_HANDLE_VALUE;
	}
#endif

	bool IsValid( ) const { return m_handle != INVALID_HANDLE_VALUE; }

	operator HANDLE& ( ) {  return m_handle; }

	HANDLE m_handle;
};

