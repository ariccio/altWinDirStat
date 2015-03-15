// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

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





SExtensionRecord::SExtensionRecord( ) : files { 0u }, color { 0u }, bytes { 0u } { }

SExtensionRecord::SExtensionRecord( _In_ std::uint32_t files_in, _In_ std::uint64_t bytes_in, _In_ std::wstring ext_in ) : files { std::move( files_in ) }, bytes { std::move( bytes_in ) }, ext( std::move( ext_in ) ) { }

SExtensionRecord::SExtensionRecord( const SExtensionRecord& in ) {
	/*
	std::wstring ext;
	_Field_range_( 0, 4294967295 ) std::uint32_t files;//save 4 bytes :)
	_Field_range_( 0, 18446744073709551615 ) std::uint64_t bytes;
	COLORREF color;
	*/
	ext = in.ext;
	files = in.files;
	bytes = in.bytes;
	color = in.color;
	}

const bool SExtensionRecord::compareSExtensionRecordByExtensionAlpha( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const {
	return ( lhs.ext.compare( rhs.ext ) < 0 );
	}

minimal_SExtensionRecord::minimal_SExtensionRecord( ) : files { 0u }, bytes { 0u } { }



#endif