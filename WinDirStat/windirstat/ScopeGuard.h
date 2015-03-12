#pragma once


#include "stdafx.h"

#ifndef WDS_SCOPEGUARD_H_INCLUDED
#define WDS_SCOPEGUARD_H_INCLUDED

#pragma message( "Including `" __FILE__ "`..." )

void trace_out( _In_z_ PCSTR const file_name, _In_z_ PCSTR const func_name, _In_ _In_range_( 0, INT_MAX ) const int line_number );

#ifdef new
#pragma push_macro("new")
#define WDS_SCOPEGUARD_PUSHED_MACRO_NEW
#undef new
#endif



//based on Andrei Alexandrescu's "Systematic Error Handling in C++"
// To initialize, use something like: auto g1 = scopeGuard( [ ]{ fclose( fd ); unlink( name ); }, __FILE__, __FUNCSIG__, __LINE__ );
// To dismiss
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
	_Field_z_ PCSTR const file_name;
	_Field_z_ PCSTR const func_name;
	_Field_range_( 0, INT_MAX ) const int line_number;
#endif

	public:
	ScopeGuard( Fun f, _In_z_ PCSTR const file_name_in, _In_z_ PCSTR const func_name_in, _In_ _In_range_( 0, INT_MAX ) const int line_number_in ) : function_to_call_on_scope_exit{ std::move( f ) }, active_{ true }
#ifdef DEBUG
		,
		file_name{ file_name_in },
		func_name{ func_name_in },
		line_number{ line_number_in }
#endif
		{
#ifndef DEBUG
		UNREFERENCED_PARAMETER( file_name_in );
		UNREFERENCED_PARAMETER( func_name_in );
		UNREFERENCED_PARAMETER( line_number_in );
#endif
		}

	//intentionally asked to NOT inline, to reduce code duplication.
	__declspec(noinline)
	~ScopeGuard( ) {
		if ( active_ ) {
	#ifdef DEBUG
			trace_out( file_name, func_name, line_number );
			//TRACE( L"Scope guard triggered!"
			//		L"\r\n\t\tScope guard initialized in file: `%S`,"
			//		L"\r\n\t\tfunction:                        `%S`,"
			//		L"\r\n\t\tline:                            `%i`\r\n", file_name, func_name, line_number );
	#endif
#pragma warning( suppress: 4711 )//C4711: function 'void __cdecl <lambda_[...]>::operator()(void)const __ptr64' selected for automatic inline expansion
			function_to_call_on_scope_exit( );
			}
		}

	//intentionally ASKING for inlining.
	inline void dismiss( ) {
		ASSERT( active_ == true );
		active_ = false;
		}


	ScopeGuard( ) = delete;
	ScopeGuard( const ScopeGuard& ) = delete;
	ScopeGuard& operator=( const ScopeGuard& ) = delete;

	//intentionally asked to NOT inline, to reduce code duplication.
	__declspec(noinline)
	ScopeGuard( ScopeGuard&& rhs ) : function_to_call_on_scope_exit( std::move( rhs.function_to_call_on_scope_exit ) ), active_( rhs.active_ ) {
		rhs.dismiss( );
		}

	};


//intentionally ASKING for inlining.
template <class Fun>
inline ScopeGuard<Fun> scopeGuard( Fun f, _In_z_ PCSTR const file_name_in, _In_z_ PCSTR const func_name_in, _In_ _In_range_( 0, INT_MAX ) const int line_number_in ) {
	return ScopeGuard<Fun>( std::move( f ), file_name_in, func_name_in, line_number_in );
	}

#ifdef WDS_SCOPEGUARD_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_SCOPEGUARD_PUSHED_MACRO_NEW
#endif

#else

#endif