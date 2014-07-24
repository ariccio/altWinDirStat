/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __OSIMAGEWRITER_H__

#include <vector>
#include "vectorutils.h"

class OsImageWriter {
public:
	virtual ~OsImageWriter() {}

    virtual bool Open()=0;
    virtual bool Close()=0;

	virtual bool CheckSpace()=0;
	virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer)=0;
};

#define __OSIMAGEWRITER_H__
#endif
