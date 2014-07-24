/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __FILEOSIMAGEREADER_H__

#include "OsImageReader.h"

#include <string>
#include <vector>
#include "vectorutils.h"
#include "smarthandle.h"

// todo:
//     - rename this class to FileImageReader
class FileOsImageReader : public OsImageReader {
public:
    static FileOsImageReader* GetFileReader(const std::string& filename);

	FileOsImageReader(const std::string& filename, DWORD offset=0);
	virtual ~FileOsImageReader();

    virtual bool Open();
    virtual bool Close();

	//virtual bool ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)=0;

protected:
	DWORD m_baseoffset;
	std::string m_filename;
    SmartHandle m_h;
	bool m_bOpen;
};

#define __FILEOSIMAGEREADER_H__
#endif
