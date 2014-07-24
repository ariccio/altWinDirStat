/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __LinearFileIMAGEREADER_H__

#include "FileOsImageReader.h"
#include <string>
#include <vector>
#include "vectorutils.h"

class LinearFileImageReader : public FileOsImageReader {
public:
    LinearFileImageReader(
			const std::string& filename, DWORD filestart, DWORD wantedsize, DWORD offset);

	
	virtual bool Open();
	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer);

private:
    DWORD m_filestart;
	DWORD m_wantedsize;
};

#define __LinearFileIMAGEREADER_H__
#endif
