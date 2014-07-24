/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __SDOSIMAGEREADER_H__

#include "OsImageReader.h"
#include "SdCardDevice.h"
#include <string>
#include <vector>
#include "vectorutils.h"

class SdOsImageReader : public OsImageReader {
public:
    SdOsImageReader(SdCardDevice* sd);
    virtual ~SdOsImageReader();

    virtual bool Open();
    virtual bool Close();

	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer);

private:
	SdCardDevice* m_sd;
	DWORD m_baseoffset;
	bool m_bOpen;
};

#define __SDOSIMAGEREADER_H__
#endif
