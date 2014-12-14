/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __OSIMAGEREADER_H__

#include <string>
#include <vector>
#include "vectorutils.h"

class OsImageReader {
public:
	virtual ~OsImageReader();

    virtual bool Open()=0;
    virtual bool Close()=0;

	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)=0;
};

#define __OSIMAGEREADER_H__
#endif
