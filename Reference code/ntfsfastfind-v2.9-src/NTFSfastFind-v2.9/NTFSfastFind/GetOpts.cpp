// ------------------------------------------------------------------------------------------------
// Standard unix like argument parsing.
//
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include "GetOpts.h"
#include <iostream>

// ------------------------------------------------------------------------------------------------
template <typename tchar>
bool GetOpts<tchar>::GetOpt()
{
    m_error = false;

	if (m_argSeq == 0 || !*m_argSeq) 
    { 
        // Update scanning pointer. 
		if (m_optIdx >= m_argc || *(m_argSeq = m_argv[m_optIdx]) != '-') 
        {
			m_argSeq = 0;
			return false;   // Done
		}

		if (m_argSeq[1] && *++m_argSeq == '-') 
        { 
            // Found "--", no more options allowed.
			++m_optIdx;
			m_argSeq = 0;
			return false;
		}
	}  

	const tchar* pOptLetterList;     // option letter list index 
    m_optOpt = *m_argSeq++;
	if (m_optOpt == ':' ||
		!(pOptLetterList = FindChr(m_optStr, m_optOpt))) 
    {
		//
		// For backwards compatibility: don't treat '-' as an
		// option letter unless caller explicitly asked for it.
		//
		if (m_optOpt == '-')
			return false;

		if (!*m_argSeq)
			++m_optIdx;

        m_error = true; // Illegal option.
		return false;   // Error.
	}

	if (*++pOptLetterList != ':') 
    { 
        // Don't need argument 
		m_optArg = NULL;
		if (!*m_argSeq)
			++m_optIdx;
	} 
    else 
    { 
        // Need an argument 
		if (*m_argSeq)   
        {
            //  No white space 
			m_optArg = m_argSeq;
        }
		else if (m_argc <= ++m_optIdx) 
        { 
			m_argSeq = 0;
            m_error = true;     // Missing option value
			return false;       // Error.
		} 
        else 
        {
            // White space
			m_optArg = m_argv[m_optIdx];
        }

		m_argSeq = 0;
		++m_optIdx;
	}

	return true; // Got a valid option.
}

// Force template to build.
template bool GetOpts<wchar_t>::GetOpt();
