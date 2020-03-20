// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


#ifndef WDS_DATASTRUCTURES_CPP
#define WDS_DATASTRUCTURES_CPP

#include "datastructures.h"



SelectObject_wrapper::SelectObject_wrapper( _In_ const HDC hDC, _In_ const HGDIOBJ hObject ) : m_hDC{ hDC } {
	//SelectObject function: https://msdn.microsoft.com/en-us/library/dd162957.aspx
	//If the selected object is not a region and the function succeeds, the return value is a handle to the object being replaced.
	//If the selected object is a region and the function succeeds, the return value is one of the following values.
		//SIMPLEREGION
		//COMPLEXREGION
		//NULLREGION
	//If an error occurs and the selected object is not a region, the return value is NULL.
	//Otherwise, it is HGDI_ERROR.
	m_pOldObject = ::SelectObject( m_hDC, hObject );
	if ( m_pOldObject == nullptr) {
		std::terminate( );
		}
	if ( m_pOldObject == HGDI_ERROR ) {
		std::terminate( );
		}
	}

SelectObject_wrapper::~SelectObject_wrapper( ) {
	//SelectObject function: https://msdn.microsoft.com/en-us/library/dd162957.aspx
	//If the selected object is not a region and the function succeeds, the return value is a handle to the object being replaced.
	//If the selected object is a region and the function succeeds, the return value is one of the following values.
		//SIMPLEREGION
		//COMPLEXREGION
		//NULLREGION
	//If an error occurs and the selected object is not a region, the return value is NULL.
	//Otherwise, it is HGDI_ERROR.
	const HGDIOBJ retval = ::SelectObject( m_hDC, m_pOldObject );
	if ( retval == nullptr) {
		std::terminate( );
		}
	if ( retval == HGDI_ERROR ) {
		std::terminate( );
		}
	}


SelectStockObject_wrapper::SelectStockObject_wrapper( _In_ HDC hDC, _In_ _In_range_( 0, 16 ) const INT nIndex ) : m_hDC { hDC } {
	if ( m_hDC == nullptr) {
		std::terminate( );
		}

	//GetStockObject function: https://msdn.microsoft.com/en-us/library/dd144925.aspx
	//If the function succeeds, the return value is a handle to the requested logical object.
	//If the function fails, the return value is NULL.
	//It is not necessary (but it is not harmful) to delete stock objects by calling DeleteObject.
	const HGDIOBJ hStockObj = ::GetStockObject( nIndex );
	if ( hStockObj == nullptr) {
		std::terminate( );
		}

	//SelectObject function: https://msdn.microsoft.com/en-us/library/dd162957.aspx
	//If the selected object is not a region and the function succeeds, the return value is a handle to the object being replaced.
	//If the selected object is a region and the function succeeds, the return value is one of the following values.
		//SIMPLEREGION
		//COMPLEXREGION
		//NULLREGION
	//If an error occurs and the selected object is not a region, the return value is NULL.
	//Otherwise, it is HGDI_ERROR.
	m_pOldObject = ::SelectObject( m_hDC, hStockObj );

	//m_pOldObject = pdc.SelectStockObject( nIndex );
	ASSERT( m_pOldObject != NULL );
	}

SelectStockObject_wrapper::~SelectStockObject_wrapper( ) {
	//SelectObject function: https://msdn.microsoft.com/en-us/library/dd162957.aspx
	//If the selected object is not a region and the function succeeds, the return value is a handle to the object being replaced.
	//If the selected object is a region and the function succeeds, the return value is one of the following values.
		//SIMPLEREGION
		//COMPLEXREGION
		//NULLREGION
	//If an error occurs and the selected object is not a region, the return value is NULL.
	//Otherwise, it is HGDI_ERROR.
	const auto retval = ::SelectObject( m_hDC, m_pOldObject );
	if ( retval == nullptr) {
		std::terminate( );
		}
	if ( retval == HGDI_ERROR ) {
		std::terminate( );
		}
	}

HGDIOBJ_wrapper::HGDIOBJ_wrapper( _In_ HGDIOBJ hObject ) : m_hObject( hObject ) {
	ASSERT( hObject != nullptr);
	}

_At_( this->m_hObject, _Post_ptr_invalid_ )
HGDIOBJ_wrapper::~HGDIOBJ_wrapper( ) {
	//DeleteObject function: https://msdn.microsoft.com/en-us/library/dd183539.aspx
	//The DeleteObject function deletes a logical pen, brush, font, bitmap, region, or palette, freeing all system resources associated with the object.
	//After the object is deleted, the specified handle is no longer valid.
	//If the function succeeds, the return value is nonzero.
	//If the specified handle is not valid or is currently selected into a DC, the return value is zero.
	//Do not delete a drawing object (pen or brush) while it is still selected into a DC.
	const BOOL result = ::DeleteObject( m_hObject );
	if ( result == 0 ) {
		std::terminate( );
		}
	}


_Pre_satisfies_( ( mode == OPAQUE) || ( mode == TRANSPARENT ) )
CSetBkMode::CSetBkMode( _In_ HDC hDC, _In_ const INT mode ) : m_hDC { hDC } {
	if ( hDC == nullptr) {
		std::terminate( );
		}

	//SetBkMode function: https://msdn.microsoft.com/en-us/library/dd162965.aspx
	//If the function succeeds, the return value specifies the previous background mode.
	//If the function fails, the return value is zero.
	m_oldMode = ::SetBkMode( m_hDC, mode );
	//m_oldMode = pdc.SetBkMode( mode );
	ASSERT( m_oldMode != 0 );
	}

CSetBkMode::~CSetBkMode( ) {
	if ( m_hDC == nullptr) {
		std::terminate( );
		}
	ASSERT( m_oldMode != 0 );
	//SetBkMode function: https://msdn.microsoft.com/en-us/library/dd162965.aspx
	//If the function succeeds, the return value specifies the previous background mode.
	//If the function fails, the return value is zero.
	VERIFY( ::SetBkMode( m_hDC, m_oldMode ) );
	//m_pdc->SetBkMode( m_oldMode );
	}

CSetTextColor::CSetTextColor( _In_ HDC hDC, _In_ const COLORREF color ) : m_hDC { hDC } {
	if ( hDC == NULL ) {
		std::terminate( );
		}
	//ASSERT_VALID( pdc );
	//m_oldColor = pdc.SetTextColor( color );
	
	//SetTextColor function: https://msdn.microsoft.com/en-us/library/dd145093.aspx
	//If the function succeeds, the return value is a color reference for the previous text color as a COLORREF value.
	//If the function fails, the return value is CLR_INVALID.
	m_oldColor = ::SetTextColor( hDC, color );
	ASSERT( m_oldColor != CLR_INVALID );
	}


CSetTextColor::~CSetTextColor( ) {
	if ( m_hDC == NULL ) {
		std::terminate( );
		}
	//SetTextColor function: https://msdn.microsoft.com/en-us/library/dd145093.aspx
	//If the function succeeds, the return value is a color reference for the previous text color as a COLORREF value.
	//If the function fails, the return value is CLR_INVALID.
	const COLORREF result = ::SetTextColor( m_hDC, m_oldColor );
	if ( result == CLR_INVALID ) {
		std::terminate( );
		}
	//m_pdc->SetTextColor( m_oldColor );
	}


SExtensionRecord::SExtensionRecord( ) : files { 0u }, bytes { 0u }, color{ 0u } { }

SExtensionRecord::SExtensionRecord( _In_ std::uint32_t files_in, _In_ std::uint64_t bytes_in, _In_ std::wstring ext_in ) : files { std::move( files_in ) }, bytes { std::move( bytes_in ) }, ext( std::move( ext_in ) ) { }

//SExtensionRecord::SExtensionRecord( const SExtensionRecord& in ) {
//	/*
//	std::wstring ext;
//	_Field_range_( 0, 4294967295 ) std::uint32_t files;//save 4 bytes :)
//	_Field_range_( 0, 18446744073709551615 ) std::uint64_t bytes;
//	COLORREF color;
//	*/
//	ext = in.ext;
//	files = in.files;
//	bytes = in.bytes;
//	color = in.color;
//	}


SExtensionRecord& SExtensionRecord::operator=( SExtensionRecord&& in ) noexcept {
	files = std::move( in.files );
	bytes = std::move( in.bytes );
	ext = std::move( in.ext );
	color = std::move( in.color );
	return *this;
	}

const bool SExtensionRecord::compareSExtensionRecordByExtensionAlpha( const SExtensionRecord& lhs, const SExtensionRecord& rhs ) const noexcept {
	return ( lhs.ext.compare( rhs.ext ) < 0 );
	}

//Yes, this is used!
SExtensionRecord::SExtensionRecord( SExtensionRecord&& in ) noexcept {
	ext = std::move( in.ext );
	files = std::move( in.files );
	bytes = std::move( in.bytes );
	color = std::move( in.color );
	}

minimal_SExtensionRecord::minimal_SExtensionRecord( ) : files { 0u }, bytes { 0u } { }



#endif