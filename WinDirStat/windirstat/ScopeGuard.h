// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once


#include "stdafx.h"

#ifndef WDS_SCOPEGUARD_H_INCLUDED
#define WDS_SCOPEGUARD_H_INCLUDED

WDS_FILE_INCLUDE_MESSAGE

void trace_out( _In_z_ PCSTR const func_expr, _In_z_ PCSTR const file_name, _In_z_ PCSTR const func_name, _In_ _In_range_( 0, INT_MAX ) const int line_number ) noexcept;

#ifdef new
#pragma push_macro("new")
#define WDS_SCOPEGUARD_PUSHED_MACRO_NEW
#undef new
#endif



//based on Andrei Alexandrescu's "Systematic Error Handling in C++"
// To initialize, use something like: auto g1 = scopeGuard( [ ]{ fclose( fd ); unlink( name ); }, __FILE__, __FUNCSIG__, __LINE__ );
template <class Fun>
class ScopeGuard final {
	Fun function_to_call_on_scope_exit;
	bool active_;

	static void* operator new( size_t ) = delete;
	static void* operator new[]( size_t ) = delete;


	//static_assert( std::is_function<Fun>::value, "This template NEEDS a function to call. `Fun` is not callable." );
	static_assert( std::is_move_constructible<Fun>::value, "It's important that `Fun` be move-constructable, as ScopeGuard has a move constructor" );
	
#if !_HAS_EXCEPTIONS
	static_assert( std::is_nothrow_move_constructible<Fun>::value, "It's important that `Fun` be move-constructable WITHOUT throwing exceptions, as ScopeGuard has a move constructor, and I have exceptions disabled." );
#endif

#ifdef DEBUG
	_Field_z_ PCSTR const func_expr;
	_Field_z_ PCSTR const file_name;
	_Field_z_ PCSTR const func_name;
	_Field_range_( 0, INT_MAX ) const int line_number;
#endif

	public:
	__forceinline ScopeGuard( Fun f, _In_z_ PCSTR const func_expr_in, _In_z_ PCSTR const file_name_in, _In_z_ PCSTR const func_name_in, _In_ _In_range_( 0, INT_MAX ) const int line_number_in ) : function_to_call_on_scope_exit{ std::move( f ) }, active_{ true }
#ifdef DEBUG
		,
		func_expr{ func_expr_in },
		file_name{ file_name_in },
		func_name{ func_name_in },
		line_number{ line_number_in }
#endif
		{
#ifndef DEBUG
		UNREFERENCED_PARAMETER( func_expr_in );
		UNREFERENCED_PARAMETER( file_name_in );
		UNREFERENCED_PARAMETER( func_name_in );
		UNREFERENCED_PARAMETER( line_number_in );
#else
		ASSERT( line_number >= 0 );
#endif
		}

	__forceinline
	~ScopeGuard( ) {
		if ( !active_ ) {
			return;
			}
#ifdef WDS_SCOPE_GUARD_DEBUGGING
		trace_out( func_expr, file_name, func_name, line_number );
#endif

#pragma warning( suppress: 4711 )//C4711: function 'void __cdecl <lambda_[...]>::operator()(void)const __ptr64' selected for automatic inline expansion
		function_to_call_on_scope_exit( );
		}

	//intentionally ASKING for inlining.
	__forceinline void dismiss( ) noexcept {
		ASSERT( active_ == true );
		active_ = false;
		}


	ScopeGuard( ) = delete;
	ScopeGuard( const ScopeGuard& ) = delete;
	ScopeGuard& operator=( const ScopeGuard& ) = delete;

	__forceinline
	ScopeGuard( ScopeGuard&& rhs ) : function_to_call_on_scope_exit( std::move( rhs.function_to_call_on_scope_exit ) ), active_( rhs.active_ ) {
		rhs.dismiss( );
		}

	};


//intentionally ASKING for inlining.
template <class Fun>
/*__forceinline*/ ScopeGuard<Fun> scopeGuard( Fun f, _In_z_ PCSTR const func_expr_in, _In_z_ PCSTR const file_name_in, _In_z_ PCSTR const func_name_in, _In_ _In_range_( 0, INT_MAX ) const int line_number_in ) noexcept {
	static_assert( std::is_move_constructible<Fun>::value, "It's important that `Fun` be move-constructable, as ScopeGuard has a move constructor" );
	
#if !_HAS_EXCEPTIONS
	static_assert( std::is_nothrow_move_constructible<Fun>::value, "It's important that `Fun` be move-constructable WITHOUT throwing exceptions, as ScopeGuard has a move constructor, and I have exceptions disabled." );
#endif

	return ScopeGuard<Fun>( std::move( f ), func_expr_in, file_name_in, func_name_in, line_number_in );
	}

#ifdef WDS_SCOPEGUARD_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_SCOPEGUARD_PUSHED_MACRO_NEW
#endif

#else

#endif