/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __LinearFileIMAGEWRITER_H__

#include <vector>
#include "vectorutils.h"
#include "FileOsImageWriter.h"

class LinearFileImageWriter : public FileOsImageWriter {
public:
	LinearFileImageWriter(const std::string& filename, DWORD filestart, DWORD baseoffset);

	virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer);

private:
    DWORD m_filestart;
};

#define __LinearFileIMAGEWRITER_H__
#endif
