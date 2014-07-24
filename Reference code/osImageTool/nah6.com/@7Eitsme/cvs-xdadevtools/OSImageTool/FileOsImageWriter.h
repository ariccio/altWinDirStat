/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __FILEOSIMAGEWRITER_H__

#include <vector>
#include "vectorutils.h"
#include "OsImageWriter.h"
#include "SmartHandle.h"

class FileOsImageWriter : public OsImageWriter {
public:
    static FileOsImageWriter* GetFileWriter(const std::string& filename);

	FileOsImageWriter(const std::string& filename, DWORD baseoffset);
	virtual ~FileOsImageWriter();

    virtual bool Open();
    virtual bool Close();

    bool DeleteFile();
    bool ClearFile();
	virtual bool CheckSpace();
	//virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer)=0;
protected:
	DWORD m_baseoffset;
	std::string m_filename;
    SmartHandle m_h;
	bool m_bOpen;
};

#define __FILEOSIMAGEWRITER_H__
#endif
