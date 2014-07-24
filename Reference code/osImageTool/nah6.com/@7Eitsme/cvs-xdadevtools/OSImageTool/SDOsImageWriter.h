/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __SDOSIMAGEWRITER_H__

#include "OsImageWriter.h"
#include "SdCardDevice.h"

#include <vector>
#include "vectorutils.h"

class SdOsImageWriter : public OsImageWriter {
public:
    SdOsImageWriter(SdCardDevice *sd);
    virtual ~SdOsImageWriter();

    virtual bool Open();
    virtual bool Close();

	virtual bool CheckSpace();
    virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer);
private:
    SdCardDevice* m_sd;
    DWORD m_checksum;
	bool m_bOpen;

};

#define __SDOSIMAGEWRITER_H__
#endif
