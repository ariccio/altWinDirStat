// Bugfix for Linux and NT 4.0 drives prior to NTFS 3.0 or 
// files written by Linux NTFS drivers

#include "ntfs_struct.h"


// limit: 2^32 files

// LinkedList
struct LINKITEM
{
	unsigned int data;
	unsigned int entry;
	LINKITEM     *next;

#ifdef TRACING
	unsigned long long num;
#endif
};


void AddToFixList( int entry, int data );
void CreateFixList( );
void ProcessFixList( PDISKHANDLE disk );
