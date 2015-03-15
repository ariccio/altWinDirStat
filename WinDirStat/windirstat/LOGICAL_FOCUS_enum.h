// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
// This enum is defined in its OWN header, as many different files depend on it. Else, we'd have to include the whole `datastructures.h` file.


#pragma once

#include "stdafx.h"

#ifndef WDS_LOGICAL_FOCUS_ENUM_H_INCLUDED
#define WDS_LOGICAL_FOCUS_ENUM_H_INCLUDED

WDS_FILE_INCLUDE_MESSAGE

// The "logical focus" can be 
// - on the Directory List
// - on the Extension List
// Although these windows can loose the real focus, for instance when a dialog box is opened, the logical focus will not be lost.
enum class LOGICAL_FOCUS {
	LF_NONE,
	LF_DIRECTORYLIST,
	LF_EXTENSIONLIST
	};


#endif