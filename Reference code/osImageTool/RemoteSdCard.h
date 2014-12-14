/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __REMOTESDCARD_H__

#include "SdCardDevice.h"

#include <string>
#include <vector>
#include "vectorutils.h"

class RemoteSdCard : public SdCardDevice {
public:
	RemoteSdCard();
	virtual ~RemoteSdCard();

    virtual bool OpenForReading();
    virtual bool OpenForWriting();
    virtual bool Close();

	virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer);
	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer);
    virtual bool GetFriendlyName(std::string& name);
	virtual DWORD GetBlockSize();
	virtual DWORD GetTotalBlocks();

private:
    int m_diskNr;
	DWORD m_blockSize;
	DWORD m_totalBlocks;
	bool m_bOpen;
};

#define __REMOTESDCARD_H__
#endif
