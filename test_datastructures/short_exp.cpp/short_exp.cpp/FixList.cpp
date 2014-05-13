// Code to fix the structures but only for filenames
// All this pointer passing seems like a bad idea!

#include "stdafx.h"
#include "FixList.h"

LINKITEM *fixlist=NULL;
LINKITEM *curfix=NULL;
unsigned long long counterList = 0;

void AddToFixList(int entry, int data)
{
//#ifdef TRACING
//	TRACE( _T("AddToFixList: entry: %i, data: %i\r\n"), entry, data);
//	if ( curfix != NULL ) {
//		TRACE( _T( "addr curfix->next: %lld\r\n" ),  &( curfix->next ) );
//		}
//#endif

	curfix->entry = entry;
	curfix->data = data;
	curfix->next = new LINKITEM;
	#ifdef TRACING
		//curfix->num = counterList;
		//counterList += 1;
		
		//TRACE(_T("addr (2) curfix->next: %lld\r\n"), &(curfix->next) );
	#endif

	curfix = curfix->next;
	//#ifdef TRACING
	//	TRACE( _T( "addr (3) curfix->next: %lld\r\n" ),  &( curfix->next ) );
	//#endif

	curfix->next = NULL;

//	curfix->next = new LINKITEM;
//
//
//
//	curfix = new LINKITEM;
//
//
//
//	curfix->next = NULL;
}

void CreateFixList()
{
#ifdef TRACING
	TRACE( _T( "CreateFixList\r\n" ) );
#endif
	fixlist = new LINKITEM;
	fixlist->next = NULL;
	curfix = fixlist;

#ifdef TRACING
	TRACE( _T( "size of fixlist: %llu\r\n" ), (ULONGLONG)sizeof(fixlist) );
#endif

}

void ProcessFixList(PDISKHANDLE disk)
{
#ifdef TRACING
	TRACE( _T( "ProcessFixList\r\n" ) );
#endif
	curfix = NULL;
	delete curfix;
	SEARCHFILEINFO *info, *src;
	while (fixlist->next!=NULL) {//STOPS BEFORE LAST ITEM IN LIST, caused leak of single LINKITEM struct (12 bytes)
#ifdef TRACING
		
		TRACE( _T("counterList: %llu\r\n" ), counterList );
		TRACE( _T("LINKITEM #: %llu\r\n" ), fixlist->num );
		counterList -= 1;
#endif
		info = &disk->fFiles[fixlist->entry];
		src  = &disk->fFiles[fixlist->data];
		info->FileName = src->FileName;
		info->FileNameLength = src->FileNameLength;
		
		info->ParentId = src->ParentId;
		// hide all that we used for cleanup
		src->ParentId.QuadPart = 0;
		LINKITEM *item;
		item = fixlist;
		//fixlist = NULL;
		//fixlist = fixlist->next;
		fixlist = item->next;
		delete item;
		}

	fixlist->data  = NULL;
	fixlist->entry = NULL;
	fixlist->next  = NULL;
#ifdef TRACING
#endif
	delete fixlist;
	
	//if ( fixlist != NULL ) {
	//	delete fixlist;
	//	}
	//if ( curfix != NULL && curfix->next != NULL ) {
	//	delete curfix;
	//	}
}

