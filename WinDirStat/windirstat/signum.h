// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#include "stdafx.h"

#pragma once

#ifndef WDS_SIGNUM_H_INCLUDED
#define WDS_SIGNUM_H_INCLUDED

WDS_FILE_INCLUDE_MESSAGE

//signum gets it's own file so that I can ask for inlining, while avoiding:
//  (a): warning (C4711, (function selected for automatic inlining))
//  (b): unnecessary code generation/unreferenced inline function removal at link time
template<class T>
inline INT signum( const T x ) noexcept {
	static_assert( std::is_pod<T>::value, "what the hell are you doing, trying to use a non-pod datatype??" );

	static_assert( std::is_arithmetic<T>::value, "need an arithmetic datatype!" );
	
	//This static_assert probably caught a bug!!! See CDriveItem::Compare - case column::COL_TOTAL & case column::COL_FREE were passing the result of an unsigned subtraction to this!!
	static_assert( std::is_signed<T>::value, "please don't try this with an unsigned number!" );

	
	if ( x < 0 ) {
		return -1;
		}
	if ( x == 0 ) {
		return 0;
		}
	return 1;
	//return ( x < 0 ) ? -1 : ( x == 0 ) ? 0 : 1;
	}



#endif