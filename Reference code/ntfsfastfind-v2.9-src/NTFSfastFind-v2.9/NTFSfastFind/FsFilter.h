// ------------------------------------------------------------------------------------------------
// FileSystem filter classes used to limit output of file system scan.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------


#pragma once

#include "BaseTypes.h"
#include "NtfsTypes.h"
#include "FsTime.h"

#include <string>
#include <time.h>

// ------------------------------------------------------------------------------------------------
// Example usage:
//      MultiFilter mFilter;
//
//      mFilter.List().push_back(new MatchName("*.txt", IsNameIcase));
//
//      double days = -2;
//      FILETIME  daysAgo = FsTime::TodayUTC() - FsTime::TimeSpan::Days(days);
//      mFilter.List().push_back(new MatchDate(daysAgo));

// ------------------------------------------------------------------------------------------------
class Match {
public:
	Match( bool matchOn = true ) : m_matchOn( matchOn ) { }

	virtual bool IsMatch( const MFT_STANDARD & attr, const MFT_FILEINFO& name, const void* pData ) = 0;

	bool m_matchOn;
};

//
// Date matching Test filters:
//
extern bool IsDateModifyGreater( const MFT_STANDARD &, const FILETIME& );
extern bool IsDateModifyEqual( const MFT_STANDARD &, const FILETIME& );
extern bool IsDateModifyLess( const MFT_STANDARD &, const FILETIME& );

// ------------------------------------------------------------------------------------------------
class MatchDate : public Match
{
public:
	typedef bool (*Test)(const MFT_STANDARD &, const FILETIME& );
	MatchDate( const FILETIME& fileTime, Test test = IsDateModifyGreater, bool matchOn = true ) : Match( matchOn ), m_fileTime( fileTime ), m_test( test ) { }

	virtual bool IsMatch( const MFT_STANDARD & attr, const MFT_FILEINFO&, const void* ) {
		return m_test( attr, m_fileTime ) == m_matchOn;
		}

	FILETIME m_fileTime;
	Test     m_test;
};

//
// Name matching Test filters:
//

extern bool IsNameIcase( const MFT_FILEINFO&, const std::wstring& name );    // Ignore case
extern bool IsName( const MFT_FILEINFO&, const std::wstring& name );     // currently not working.

// ------------------------------------------------------------------------------------------------
class MatchName : public Match
{
public:
	typedef bool (*Test)(const MFT_FILEINFO&, const std::wstring& name);

	MatchName( const std::wstring& name, Test test = IsNameIcase, bool matchOn = true ) : Match( matchOn ), m_name( name ), m_test( test ) { }

	virtual ~MatchName( ) { }

	virtual bool IsMatch( const MFT_STANDARD &, const MFT_FILEINFO& name, const void* ) {
		return ( ( name.chFileNameLength != 0 ) && m_test( name, m_name ) ) == m_matchOn;
		}

	std::wstring m_name;
	Test         m_test;
};



//
// Size matching Test filters:
//

extern bool IsSizeGreater( const MFT_FILEINFO&, LONGLONG size );
extern bool IsSizeEqual( const MFT_FILEINFO&, LONGLONG size );
extern bool IsSizeLess( const MFT_FILEINFO&, LONGLONG size );

// ------------------------------------------------------------------------------------------------
class MatchSize : public Match
{
public:
	typedef bool (*Test)(const MFT_FILEINFO&, LONGLONG size);

	MatchSize(LONGLONG size, Test test = IsSizeGreater, bool matchOn = true) : Match(matchOn), m_size(size), m_test(test) { }

	virtual bool IsMatch( const MFT_STANDARD &, const MFT_FILEINFO& name, const void* ) {
		return m_test(name, m_size) == m_matchOn;
		}

	LONGLONG     m_size;
	Test         m_test;
};


// ------------------------------------------------------------------------------------------------
class FsFilter
{
public:
	virtual bool IsMatch( const MFT_STANDARD &, const MFT_FILEINFO&, const void* pData ) const = 0;
	virtual bool IsValid( ) const = 0;
};

// ------------------------------------------------------------------------------------------------
class StreamFilter
{
public:
	// TODO  - filter and store stream names.
	virtual bool IsMatch(const wchar_t* pwFilename, const wchar_t* pwStreamName, DWORD streamLength) const {
		// std::wcout << pwFilename << pwStreamName << " StreamSize:" << streamLength << std::endl;
		return false;   // TODO - add logic
		}
};

// ------------------------------------------------------------------------------------------------
//  Single filter rule
//  Ex:
//      OneFilter oneFilter(new MatchName("*.txt", IsNameIcase));
//      ...user filter
//      FILETIME today = ...
//      oneFilter.SetMatch(new MatchDate(daysAgo, IsDateModifyGreater));
//
class OneFilter : public FsFilter
{
public:
	OneFilter() { }

	OneFilter( SharePtr<Match>& rMatch ) : m_rMatch( rMatch ) { }

	virtual ~OneFilter( ) { }

	void SetMatch( SharePtr<Match>& rMatch ) { m_rMatch = rMatch; }

	virtual bool IsMatch( const MFT_STANDARD & attr, const MFT_FILEINFO& name, const void* pData ) const {
		return m_rMatch->IsMatch(attr, name, pData);
		}

	virtual bool IsValid( ) const { return !m_rMatch.IsNull( );  }

private:
	SharePtr<Match> m_rMatch;
};

// ------------------------------------------------------------------------------------------------
//  Multiple filter rules
//  Ex:
//      MultiFilter mFilter;
//      mFilter.List().push_back(new MatchName(L"foo"));
//      mFilter.List().push_back(new MatchName(L"*.txt", IsNameIcase, false));  // reverse match
//      mFilter.List().push_back(new MatchDate(Today, IsDateModifyGreater);
//
class MultiFilter : public FsFilter
{
public:
	typedef std::vector<SharePtr<Match>> MatchList;

	MultiFilter( ) { }

	MultiFilter( const MatchList& matchList ) : m_testList( matchList ) { }

	virtual ~MultiFilter( ) { }

	void SetMatch( const MatchList& matchList ) { m_testList = matchList; }

	MatchList& List( ) { return m_testList; }
   
	virtual bool IsMatch( const MFT_STANDARD & attr, const MFT_FILEINFO& name, const void* pData ) const {
		for ( unsigned mIdx = 0; mIdx < m_testList.size( ); mIdx++ ) {
			if ( !m_testList[ mIdx ]->IsMatch( attr, name, pData ) ) {
				return false;
				}
			}
		return true;
		}

	virtual bool IsValid( ) const { return m_testList.size( ) != 0; }

private:
	MatchList  m_testList;
};


