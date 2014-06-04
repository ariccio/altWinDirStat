/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __BINOSIMAGEREADER_H__

#include "FileOsImageReader.h"
#include <string>
#include <vector>
#include "vectorutils.h"

#include <map>

typedef struct {
    DWORD filepos;
    DWORD length;
} MemoryBlock;

typedef std::map<DWORD,MemoryBlock> OffsetToFilePosMap;

class BINOsImageReader : public FileOsImageReader {
public:
    BINOsImageReader(const std::string& filename, DWORD offset=0);

    virtual bool Open();
	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer);
private:
    OffsetToFilePosMap m_blockmap;
};

#define __BINOSIMAGEREADER_H__
#endif
