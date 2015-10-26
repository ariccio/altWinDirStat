// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_DATASTRUCTURES_CPP
#define WDS_DATASTRUCTURES_CPP

#include "datastructures.h"



CSelectObject::CSelectObject( _In_ const HDC hDC, _In_ const HGDIOBJ hObject ) : m_hDC{ hDC } {
	m_pOldObject = ::SelectObject( m_hDC, hObject );
	if ( m_pOldObject == NULL ) {
		std::terminate( );
		}
	if ( m_pOldObject == HGDI_ERROR ) {
		std::terminate( );
		}
	}

CSelectObject::~CSelectObject( ) {
	const HGDIOBJ retval = ::SelectObject( m_hDC, m_pOldObject );
	if ( retval == NULL ) {
		std::terminate( );
		}
	if ( retval == HGDI_ERROR ) {
		std::terminate( );
		}
	}


CSelectStockObject::CSelectStockObject( _In_ HDC hDC, _In_ _In_range_( 0, 16 ) const INT nIndex ) : m_hDC { hDC } {
	//"Return Value: A pointer to the CGdiObject object that was replaced if the function is successful. The actual object pointed to is a CPen, CBrush, or CFont object. If the call is unsuccessful, the return value is NULL."
	/*
	CGdiObject* CDC::SelectStockObject(int nIndex)
	{
		ASSERT(m_hDC != NULL);

		HGDIOBJ hObject = ::GetStockObject(nIndex);
		HGDIOBJ hOldObj = NULL;

		ASSERT(hObject != NULL);
		if (m_hDC != m_hAttribDC)
			hOldObj = ::SelectObject(m_hDC, hObject);
		if (m_hAttribDC != NULL)
			hOldObj = ::SelectObject(m_hAttribDC, hObject);
		return CGdiObject::FromHandle(hOldObj);
	}

	
	*/
	if ( m_hDC == NULL ) {
		std::terminate( );
		}
	HGDIOBJ hStockObj = ::GetStockObject( nIndex );
	if ( hStockObj == NULL ) {
		std::terminate( );
		abort( );
		}
	m_pOldObject = ::SelectObject( m_hDC, hStockObj );

	//m_pOldObject = pdc.SelectStockObject( nIndex );
	ASSERT( m_pOldObject != NULL );
	}

CSelectStockObject::~CSelectStockObject( ) {
	//"Return Value: A pointer to the object being replaced. This is a pointer to an object of one of the classes derived from CGdiObject, such as CPen, depending on which version of the function is used. The return value is NULL if there is an error. This function may return a pointer to a temporary object. This temporary object is only valid during the processing of one Windows message. For more information, see CGdiObject::FromHandle."
	const auto retval = ::SelectObject( m_hDC, m_pOldObject );
	if ( retval == NULL ) {
		std::terminate( );
		}
	if ( retval == HGDI_ERROR ) {
		std::terminate( );
		}
	}

_Pre_satisfies_( ( mode == OPAQUE) || ( mode == TRANSPARENT ) )
CSetBkMode::CSetBkMode( _In_ HDC hDC, _In_ const INT mode ) : m_hDC { hDC } {
	if ( hDC == NULL ) {
		std::terminate( );
		}
	m_oldMode = ::SetBkMode( m_hDC, mode );
	//m_oldMode = pdc.SetBkMode( mode );
	}

CSetBkMode::~CSetBkMode( ) {
	if ( m_hDC == NULL ) {
		std::terminate( );
		abort( );
		}
	::SetBkMode( m_hDC, m_oldMode );
	//m_pdc->SetBkMode( m_oldMode );
	}

CSetTextColor::CSetTextColor( _In_ HDC hDC, _In_ const COLORREF color ) : m_hDC { hDC } {
	if ( hDC == NULL ) {
		std::terminate( );
		}
	//ASSERT_VALID( pdc );
	//m_oldColor = pdc.SetTextColor( color );
	m_oldColor = ::SetTextColor( hDC, color );
	ASSERT( m_oldColor != CLR_INVALID );
	}


CSetTextColor::~CSetTextColor( ) {
	if ( m_hDC == NULL ) {
		std::terminate( );
		abort( );
		}
	const COLORREF result = ::SetTextColor( m_hDC, m_oldColor );
	if ( result == CLR_INVALID ) {
		std::terminate( );
		}
	//m_pdc->SetTextColor( m_oldColor );
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