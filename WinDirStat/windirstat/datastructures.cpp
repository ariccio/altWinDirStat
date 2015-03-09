#pragma once

#include "stdafx.h"

#ifndef WDS_DATASTRUCTURES_CPP
#define WDS_DATASTRUCTURES_CPP

#include "datastructures.h"



CSelectObject::CSelectObject( _In_ CDC& pdc, _In_ CGdiObject& pObject ) : m_pdc{ &pdc } {
	//"Return Value: A pointer to the object being replaced. This is a pointer to an object of one of the classes derived from CGdiObject, such as CPen, depending on which version of the function is used. The return value is NULL if there is an error. This function may return a pointer to a temporary object. This temporary object is only valid during the processing of one Windows message. For more information, see CGdiObject::FromHandle."
	m_pOldObject = pdc.SelectObject( &pObject );
	/*
_AFXWIN_INLINE CGdiObject* CDC::SelectObject(CGdiObject* pObject)
{ ASSERT(m_hDC != NULL); return SelectGdiObject(m_hDC, pObject->GetSafeHandle()); }
		
CGdiObject* PASCAL CDC::SelectGdiObject(HDC hDC, HGDIOBJ h)
{
return CGdiObject::FromHandle(::SelectObject(hDC, h));
}

CGdiObject* PASCAL CGdiObject::FromHandle(HGDIOBJ h)
{
CHandleMap* pMap = afxMapHGDIOBJ(TRUE); //create map if not exist
ASSERT(pMap != NULL);
CGdiObject* pObject = (CGdiObject*)pMap->FromHandle(h);
ASSERT(pObject == NULL || pObject->m_hObject == h);
return pObject;
}
	*/

	ASSERT( m_pOldObject != NULL );
	}

CSelectObject::~CSelectObject( ) {
	const auto retval = m_pdc->SelectObject( m_pOldObject );
#ifdef DEBUG
	ASSERT( retval != NULL );
#else
	UNREFERENCED_PARAMETER( retval );
#endif
	}


CSelectStockObject::CSelectStockObject( _In_ CDC& pdc, _In_ _In_range_( 0, 16 ) const INT nIndex ) : m_pdc { &pdc } {
	//"Return Value: A pointer to the CGdiObject object that was replaced if the function is successful. The actual object pointed to is a CPen, CBrush, or CFont object. If the call is unsuccessful, the return value is NULL."
	m_pOldObject = pdc.SelectStockObject( nIndex );
	ASSERT( m_pOldObject != NULL );
	}

CSelectStockObject::~CSelectStockObject( ) {
	//"Return Value: A pointer to the object being replaced. This is a pointer to an object of one of the classes derived from CGdiObject, such as CPen, depending on which version of the function is used. The return value is NULL if there is an error. This function may return a pointer to a temporary object. This temporary object is only valid during the processing of one Windows message. For more information, see CGdiObject::FromHandle."
	const auto retval = m_pdc->SelectObject( m_pOldObject );
#ifdef DEBUG
	ASSERT( retval != NULL );
#else
	UNREFERENCED_PARAMETER( retval );
#endif
	}

_Pre_satisfies_( ( mode == OPAQUE) || ( mode == TRANSPARENT ) )
CSetBkMode::CSetBkMode( _In_ CDC& pdc, _In_ const INT mode ) : m_pdc { &pdc } {
	m_oldMode = pdc.SetBkMode( mode );
	}

CSetBkMode::~CSetBkMode( ) {
	m_pdc->SetBkMode( m_oldMode );
	}

CSetTextColor::CSetTextColor( _In_ CDC& pdc, _In_ const COLORREF color ) : m_pdc { &pdc } {
	//ASSERT_VALID( pdc );
	m_oldColor = pdc.SetTextColor( color );
	}


CSetTextColor::~CSetTextColor( ) {
	m_pdc->SetTextColor( m_oldColor );
	}


//Yes, this is used!
SExtensionRecord::SExtensionRecord( SExtensionRecord&& in ) {
	ext = std::move( in.ext );
	files = std::move( in.files );
	bytes = std::move( in.bytes );
	color = std::move( in.color );
	}

FILEINFO& FILEINFO::operator=( FILEINFO&& in ) {
	length = std::move( in.length );
	lastWriteTime = std::move( in.lastWriteTime );
	attributes = std::move( in.attributes );
	name = std::move( in.name );
	return ( *this );
	}

FILEINFO::FILEINFO( FILEINFO&& in ) {
	length = std::move( in.length );
	lastWriteTime = std::move( in.lastWriteTime );
	attributes = std::move( in.attributes );
	name = std::move( in.name );
	}

void FILEINFO::reset( ) {
	length = 0;
	lastWriteTime.dwLowDateTime  = 0;
	lastWriteTime.dwHighDateTime = 0;
	attributes = INVALID_FILE_ATTRIBUTES;
	name.clear( );
	}


DIRINFO::DIRINFO( DIRINFO&& in ) {
	length = std::move( in.length );
	lastWriteTime = std::move( in.lastWriteTime );
	attributes = std::move( in.attributes );
	name = std::move( in.name );
	path = std::move( in.path );
	}

SRECT::SRECT( ) : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) { }
SRECT::SRECT( std::int16_t iLeft, std::int16_t iTop, std::int16_t iRight, std::int16_t iBottom ) : left { iLeft }, top { iTop }, right { iRight }, bottom { iBottom } { }

SRECT::SRECT( const RECT& in ) {
	left   = static_cast<std::int16_t>( in.right );
	top    = static_cast<std::int16_t>( in.top );
	right  = static_cast<std::int16_t>( in.right );
	bottom = static_cast<std::int16_t>( in.bottom );
	}

#endif