// Code to support wildchars

#include "stdafx.h"
#include "SimplePattern.h"

int __cdecl wcsnrcmp ( const wchar_t * first, const wchar_t * last, size_t count );

// Prepares the searchpattern struct
SEARCHP* StartSearch(wchar_t* string, int len)
{
	wchar_t* res;
	if ( len > 1 ) {
		SEARCHP* ptr;
		ptr = new SEARCHP;
		memset( ptr, 0, sizeof( SEARCHP ) );
		ptr->mode = 0;
		if ( string[ len - 1 ] == L'*' ) {
			ptr->mode += 2;
			string[ len - 1 ] = 0;
			len--;
			}
		if ( string[ 0 ] == L'*' ) {
			ptr->mode += 1;
			string++;
			len--;
			}

		ptr->string = string;
		ptr->len = len;
		ptr->totallen = ptr->len;
		if ( ptr->mode == 0 ) {
			res = wcschr( string, L'*' );
			if ( res != NULL ) {
				ptr->mode = 42;
				*res = L'\0';
				ptr->len = res-string;
				ptr->extra = &res[ 1 ];
				ptr->extralen = len - ptr->len-1;
				ptr->totallen = ptr->len + ptr->extralen;
				}
			}
		return ptr;	
		}
	return NULL;
}

// does the actual search
int SearchStr(SEARCHP* pattern, wchar_t* string, int len)
{
	if ( pattern->totallen > len ) {
		return FALSE;
		}
	switch(pattern->mode)
	{
		case 0:
			if ( wcscmp( string, pattern->string ) == 0 ) {
				return TRUE;
				}
			break;
		case 1:
			if ( wcsnrcmp( string + len, pattern->string + pattern->len, pattern->len + 1 ) == 0 ) {
				return TRUE;
				}
			break;
		case 2:
			if ( wcsncmp( string, pattern->string, pattern->len ) == 0 ) {
				return TRUE;
				}
			break;
		case 3:
			if ( wcsstr( string, pattern->string ) != NULL ) {
				return TRUE;
				}
			break;
		case 42:
			if ( wcsnrcmp( string + len, pattern->extra + pattern->extralen, pattern->extralen + 1 ) == 0 ) {
				if ( wcsncmp( string, pattern->string, pattern->len ) == 0 ) {
					return TRUE;
					}
				}
			break;
	}
	return FALSE;
}
// just the simple cleanup, because no copies were made
int EndSearch(SEARCHP* pattern)
{
	delete pattern;
	return 0;
}

int __cdecl wcsnrcmp ( const wchar_t * first, const wchar_t * last, size_t count )
{
	if ( !count ) {
		return( 0 );
		}
	while ( --count && *first == *last ) {//CANNOT be vectorized.
			first--;
			last--;
		}
	return ( ( int ) ( *first - *last ) );
}