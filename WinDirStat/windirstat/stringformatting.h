// stringformatting.h
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_STRINGFORMATTING_H_INCLUDED
#define WDS_STRINGFORMATTING_H_INCLUDED


WDS_FILE_INCLUDE_MESSAGE

#include "macros_that_scare_small_children.h"


//WinDirStat string-formatting functions
namespace wds_fmt {

	_Success_( SUCCEEDED( return ) ) HRESULT FormatBytes                ( _In_ const std::uint64_t n, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_formatted_bytes, _In_range_( 38, 64 ) const rsize_t strSize, _Out_ rsize_t& chars_written, _On_failure_( _Post_valid_ ) rsize_t& size_needed );

	std::wstring FormatBytes( _In_ const std::uint64_t n, const bool humanFormat );

	void write_BAD_FMT      ( _Out_writes_z_( 8 )  _Pre_writable_size_( 8 ) _Post_readable_size_( 8 ) PWSTR pszFMT, _Out_ rsize_t& chars_written );
	}

#endif