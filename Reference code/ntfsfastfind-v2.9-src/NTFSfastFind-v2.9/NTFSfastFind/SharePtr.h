// ------------------------------------------------------------------------------------------------
// Simple smart pointer to delete shared pointer when last instance goes out of scope.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#pragma once

#include <assert.h>

/// Simple smart pointer with reference counting.
template < typename T > 
class SharePtr {
public:
	SharePtr( ) : m_pShareItem( 0 ) { }

	SharePtr( T* pItem ) : m_pShareItem( new ShareItem( pItem ) ) { }

	SharePtr( const SharePtr& rhs ) : m_pShareItem( rhs.m_pShareItem != NULL ? rhs.m_pShareItem->Add( ) : NULL ) { }

	SharePtr& operator=( const SharePtr& rhs ) {
		if ( this != &rhs ) {
				m_pShareItem = rhs.m_pShareItem != NULL ? rhs.m_pShareItem->Add() : NULL;
			}
		return *this;
		}

	~SharePtr( ) {
		if ( m_pShareItem != NULL ) {
			int refCnt = m_pShareItem->Dec( );
			assert( refCnt >= 0 );
			if ( 0 == refCnt ) {
				delete m_pShareItem;
				}
			}
		}

	class ShareItem {
	public:
		ShareItem( T* pItem ) : m_pItem( pItem ), m_refCnt( 1 ), m_marker( 123456 ) { }
		~ShareItem( ) {
#ifdef TRACING
			std::wcout << std::endl << "\t~ShareItem" << std::endl;
#endif

			assert( m_refCnt == 0 );
			assert( m_marker == 123456 );

			m_refCnt = 0;
			m_marker = 654321;
			
			delete m_pItem;
			m_pItem = 0;
			}

		ShareItem* Add( ) {
			m_refCnt++;
			return this;
			}    // not thread safe

		int Dec() { return --m_refCnt; }          // not thread safe

		T*          m_pItem;
		mutable int m_refCnt;
		int         m_marker;
	};


	bool IsNull( ) const { return m_pShareItem == NULL || m_pShareItem->m_pItem == NULL; }

	operator T*( ) const {  return m_pShareItem->m_pItem;  }

	T* operator-> ( ) const {  return m_pShareItem->m_pItem;   }

	T* Ptr( ) {  return m_pShareItem->m_pItem; }

private:
	ShareItem*  m_pShareItem;
};

