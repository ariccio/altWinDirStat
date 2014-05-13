/* Heap

*/
typedef struct HEAPBLOCK
{
	PUCHAR    data;
	DWORD     current;
	DWORD     size;
	HEAPBLOCK *end;
	HEAPBLOCK *next;
}HEAPBLOCK, *PHEAPBLOCK;

/* Heaps*/


PHEAPBLOCK CreateHeap ( DWORD      size );
BOOL       FreeHeap   ( PHEAPBLOCK block);

PUCHAR AllocData          ( PHEAPBLOCK block, DWORD size              );
LPTSTR AllocString        ( PHEAPBLOCK block, DWORD size              );
LPTSTR AllocAndCopyString ( PHEAPBLOCK block, LPTSTR HEAP, DWORD size );

BOOL FreeAllBlocks ( PHEAPBLOCK block             );
BOOL ReUseBlocks   ( PHEAPBLOCK block, BOOL clear );
