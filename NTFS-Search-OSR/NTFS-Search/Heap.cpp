// Heaps

#include "stdafx.h"
#include "Heap.h"

PHEAPBLOCK CreateHeap(DWORD size)
{
#ifdef TRACING
	TRACE( _T( "CreateHeap (size: %lld)\r\n" ), size );
#endif
	PHEAPBLOCK tmp = new HEAPBLOCK;
	//SecureZeroMemory( &tmp, sizeof( HEAPBLOCK ) );
	//tmp = (PHEAPBLOCK) malloc(sizeof(HEAPBLOCK));
	tmp->current = 0;
	tmp->size = size;
	tmp->next = NULL;
	//tmp->data = (PUCHAR) malloc(size);

	tmp->data = new UCHAR[ size*sizeof(UCHAR) ];
	if (tmp->data != NULL) {
		//currentBlock = tmp;
		tmp->end = tmp;
		return tmp;
		}
	else {
		//free(tmp);
		delete tmp;
		return NULL;
		}
}

BOOL FreeHeap(PHEAPBLOCK block)
{
	if (block != NULL) {
		FreeAllBlocks( block );
		return TRUE;
		}
	return FALSE;
}

LPTSTR AllocAndCopyString(PHEAPBLOCK block, LPTSTR string, DWORD size)
{
	PHEAPBLOCK tmp, back = NULL;
	PUCHAR ret = NULL;
	int t;
	int asize;
	DWORD rsize = ( size + 1 )*sizeof( TCHAR );
	asize = ( ( rsize ) & 0xfffffff8 ) + 8;

	if ( asize <= rsize ) {
		DebugBreak( );
		}
	tmp = block->end;

	if (tmp!=NULL) {
		t = tmp->size - tmp->current;
		if ( t > asize ) {
			goto copy;
			}
		back = tmp;
		tmp  = tmp->next;
		if ( tmp != NULL ) {
			t = tmp->size - tmp->current;
			if ( t > asize ) {
				block->end = tmp;
				goto copy;
			}
			back = tmp;
			tmp  = tmp->next;
			}
		}
	tmp = ( PHEAPBLOCK ) malloc( sizeof( HEAPBLOCK ) );

#ifdef TRACING
	TRACE( _T("tmp address: %llu\r\n"),(ULONGLONG)&tmp);
#endif
	memset( tmp, 0, sizeof( HEAPBLOCK ) );
	tmp->data =	(PUCHAR) malloc(block->size);
#ifdef TRACING
	TRACE( _T( "tmp->data address: %llu\r\n" ), ( ULONGLONG ) &(tmp->data) );
#endif

	if (tmp->data!=NULL) {
		tmp->size = block->size;
		tmp->next = NULL;
		
		if ( back == NULL ) {
			back = block->end;
			}
		tmp->end = block;
		if ( back != NULL ) {
			back->next = tmp;
			}
		block->end = tmp;
		goto copy;
		}
	else {
		DebugBreak();
		return NULL;
		}

copy:
	ret              = &tmp->data[ tmp->current ];
	memcpy( ret, string, rsize );
	ret[ rsize ]     = 0;
	ret[ rsize + 1 ] = 0;
	tmp->current    += asize;
	return ( LPTSTR ) ret;
}

BOOL FreeAllBlocks(PHEAPBLOCK block)
{
	PHEAPBLOCK tmp;
	PHEAPBLOCK back;
	tmp = block;
#ifdef TRACING
	TRACE( _T( "Freeing all blocks!\r\n" ) );
#endif
	while ( tmp != NULL ) {
		//free(tmp->data);	
		delete tmp->data;
		back = tmp;
		tmp = tmp->next;
		//free(back);
		delete back;
		}
	return TRUE;
}

BOOL ReUseBlocks(PHEAPBLOCK block, BOOL clear)
{
	if ( block != NULL ) {
		PHEAPBLOCK tmp;
		PHEAPBLOCK back;
		tmp = block;
		while ( tmp != NULL ) {
			tmp->current = 0;	
			tmp = tmp->next;
			if ( clear ) {
				delete tmp->data;
				//memset( tmp->data, 0, tmp->size*sizeof( TCHAR ) );
				}
			}
		block->end = block;
		}
	return TRUE;
}