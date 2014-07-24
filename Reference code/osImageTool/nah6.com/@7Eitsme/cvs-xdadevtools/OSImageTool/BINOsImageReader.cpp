/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "BINOsImageReader.h"

#include <assert.h>

#include <vector>
#include <string>
#include "vectorutils.h"
#include "debug.h"
#include "errorhandler.h"

BINOsImageReader::BINOsImageReader(const std::string& filename, DWORD offset/*=0*/) : FileOsImageReader(filename, offset)
{
}

bool BINOsImageReader::Open()
{
    FileOsImageReader::Open();
    if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, 7+2*4+m_baseoffset, NULL, FILE_BEGIN))
    {
		g_err.Set(stringformat("error seeking to %08lx : %s", 7+2*4+m_baseoffset, LastError().c_str()));
        return false;
    }

    while(true)
    {
        struct {
            DWORD start;
            DWORD length;
            DWORD checksum;
        } blockHeader;
        DWORD bytes_read;
        if (!ReadFile(m_h, &blockHeader, sizeof(blockHeader), &bytes_read, NULL)
                || bytes_read!=sizeof(blockHeader))
        {
			g_err.Set(stringformat("error reading %08lx bytes from %08lx : %s", sizeof(blockHeader), 7+2*4+m_baseoffset, LastError().c_str()));
            return false;
        }

        if (blockHeader.start==0)
            break;

        m_blockmap[blockHeader.start].filepos= SetFilePointer(m_h, 0, NULL, FILE_CURRENT);
        m_blockmap[blockHeader.start].length= blockHeader.length;

        if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, blockHeader.length, NULL, FILE_CURRENT))
        {
			g_err.Set(stringformat("error seeking forward by %08lx : %s", blockHeader.length, LastError().c_str()));
            return false;
        }
    }

	return true;
}

bool BINOsImageReader::ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)
{
	buffer.resize(dwSize);

    ByteVector::iterator bufp= buffer.begin();

    // this iterates over all blocks, in order of memory offset
    for (OffsetToFilePosMap::iterator i= m_blockmap.begin() ; i!=m_blockmap.end() ; ++i)
    {
        DWORD blockoffset= (*i).first;
        MemoryBlock& block= (*i).second;

        if (blockoffset+block.length <= dwOffset)
		{
			//debug("skipping [%08lx : %08lx, %08lx]  o=%08lx s=%08lx\n", blockoffset, block.filepos, block.length, dwOffset, dwSize);
            continue;
		}
        if (dwOffset+dwSize <= blockoffset)
		{
			//debug("past end [%08lx : %08lx, %08lx]  o=%08lx s=%08lx\n", blockoffset, block.filepos, block.length, dwOffset, dwSize);
            break;
		}

        if (dwOffset < blockoffset)
        {
			//debug("filler until [%08lx : %08lx, %08lx]  o=%08lx s=%08lx\n", blockoffset, block.filepos, block.length, dwOffset, dwSize);

            fill_n(bufp, blockoffset-dwOffset, 0);

			bufp += blockoffset-dwOffset;
            dwSize -= blockoffset-dwOffset;
            dwOffset = blockoffset;
        }
        if (dwOffset < blockoffset+block.length)
        {
			if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, m_baseoffset+block.filepos+dwOffset-blockoffset, NULL, FILE_BEGIN))
			{
				g_err.Set(stringformat("error seeking to %08lx : %s", m_baseoffset+block.filepos+dwOffset-blockoffset, LastError().c_str()));
                return false;
			}

            DWORD bytes_read;
            DWORD bytes_wanted= min(dwSize, blockoffset+block.length-dwOffset);
            if (!ReadFile(m_h, iteratorptr(bufp), bytes_wanted, &bytes_read, NULL)
                    || bytes_read!=bytes_wanted)
            {
				g_err.Set(stringformat("error reading %08lx bytes from %08lx : %s", bytes_wanted, m_baseoffset+block.filepos+dwOffset-blockoffset, LastError().c_str()));
                return false;
            }
			//debug("chunk from [%08lx : %08lx, %08lx]  o=%08lx  s=%08lx l=%08lx\n", blockoffset, block.filepos, block.length, dwOffset, dwSize, bytes_wanted);
			
            bufp += bytes_wanted;
            dwSize -= bytes_wanted;
            dwOffset += bytes_wanted;
        }

        if (dwSize==0)
            break;
    }
    if (dwSize)
    {
        fill_n(bufp, dwSize, 0);

		bufp += dwSize;
		dwOffset += dwSize;
		dwSize= 0;
    }
	assert(bufp==buffer.end());

	return true;
}

