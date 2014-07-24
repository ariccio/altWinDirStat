/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __XDAOSIMAGEREADER_H__

#include "OsImageReader.h"
#include <string>
#include <vector>
#include "vectorutils.h"

class XDAOsImageReader : public OsImageReader {
public:
    XDAOsImageReader();
	virtual ~XDAOsImageReader();

    virtual bool Open();
    virtual bool Close();

	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer);

private:
	bool m_bOpen;
};

#define __XDAOSIMAGEREADER_H__
#endif
