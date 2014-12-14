/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __NBFFILEOSIMAGEWRITER_H__

#include <vector>
#include "vectorutils.h"
#include "LinearFileImageWriter.h"

class NBFFileOsImageWriter : public LinearFileImageWriter {
public:
	NBFFileOsImageWriter(const std::string& filename, DWORD baseoffset=0);

	virtual bool Open();
    virtual bool Close();
	virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer);
private:
    DWORD m_bootloaderBytes;
	DWORD m_checksum;
};

#define __NBFFILEOSIMAGEWRITER_H__
#endif
