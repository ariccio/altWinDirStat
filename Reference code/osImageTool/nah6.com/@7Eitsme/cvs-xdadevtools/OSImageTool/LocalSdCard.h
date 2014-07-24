/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __LOCALSDCARD_H__

#include "SdCardDevice.h"

#include "SmartHandle.h"

#include <string>
#include <vector>
#include "vectorutils.h"

#include "DiskInfo.h"

class LocalSdCard : public SdCardDevice {
public:
	LocalSdCard(const DiskInfo* di);
	virtual ~LocalSdCard();

    virtual bool OpenForReading();
    virtual bool OpenForWriting();
    virtual bool Close();

	virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer);
	virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer);
	virtual bool GetFriendlyName(std::string& name);
	virtual DWORD GetBlockSize();
	virtual DWORD GetTotalBlocks();

private:
	const DiskInfo* m_di;
    std::string m_friendlyname;
    
	bool m_bWriting;
	bool m_bReading;

	SmartHandle m_h;
};

#define __LOCALSDCARD_H__
#endif
