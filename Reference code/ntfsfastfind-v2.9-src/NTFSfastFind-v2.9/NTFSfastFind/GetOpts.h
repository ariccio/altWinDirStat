// ------------------------------------------------------------------------------------------------
// Standard unix like argument parsing.
//
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#pragma once

template <typename tchar>
class GetOpts
{
public:
    // Pass in argc and argv from main()
    // optStr is optional switches
    //      "bd:eg:h"
    // colon indicates those switch letter which tag an argument
    //   -b  -d foo -e -g bar -h
	GetOpts( int argc, const tchar* argv[ ], const tchar* optStr ) : m_argc( argc ), m_argv( argv ), m_optStr( optStr ), m_optArg( 0 ), m_optIdx( 1 ), m_optOpt( 0 ), m_argSeq( L"" ), m_error( false ) { }
       
    int             m_argc;
    const tchar**   m_argv;
    const tchar*    m_optStr;

    const tchar*    m_optArg;   // Argument associated with option  
    int             m_optIdx;   // Index into parent argv vector 
    tchar           m_optOpt;   // Character option being processed.
	const tchar*    m_argSeq;   // Argv token of characters (sequence).
    bool            m_error;    // True if error detected.

    // Return true if option detected.
	bool GetOpt( );

    // Return option character just processed by GetOpt().
	tchar Opt( ) const { return m_optOpt; }

	bool Error( ) const { return m_error; }

    // Return option's argument value.
	const tchar* OptArg( ) const { return m_optArg; }

    // Return next index after last processed by GetOpt();
	int NextIdx( ) const { return m_optIdx; }

	const tchar* FindChr( const tchar* str, tchar chr ) {
		while ( *str && *str != chr ) {
			str++;
			}
        return (*str == chr) ? str : 0;
		}
};

