/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __XDAOSIMAGEWRITER_H__

#include "OsImageWriter.h"
#include "NBFFileOsImageWriter.h"

#include <vector>
#include "vectorutils.h"

class XDAOsImageWriter : public OsImageWriter {
public:
    XDAOsImageWriter();
	virtual ~XDAOsImageWriter();

    virtual bool Open();
    virtual bool Close();

	virtual bool WriteData(DWORD dwOffset, const ByteVector& buffer);
	virtual bool CheckSpace();

private:
    NBFFileOsImageWriter *m_nbf;
	std::string m_NBFFileName;
	bool m_bOpen;
};

#define __XDAOSIMAGEWRITER_H__
#endif
