/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __SDCARDWRITER_H__

#include <string>
#include <vector>
#include "vectorutils.h"

class SdCardDevice {
public:
    virtual ~SdCardDevice()=0;

    virtual bool OpenForReading()=0;
    virtual bool OpenForWriting()=0;
    virtual bool Close()=0;

    virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer)=0;
    virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)=0;
    virtual bool GetFriendlyName(std::string& name)=0;
	std::string GetFriendlyName();
	virtual DWORD GetBlockSize()=0;
	virtual DWORD GetTotalBlocks()=0;

};

#define __SDCARDWRITER_H__
#endif
