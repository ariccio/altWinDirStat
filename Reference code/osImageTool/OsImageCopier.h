/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __OSIMAGECOPIER_H__

#include "OSImageToolDlg.h"
#include "OsImageReader.h"
#include "OsImageWriter.h"

class OsImageCopier {
public:
	OsImageCopier(COSImageToolDlg *dlg);
	
	bool copy(OsImageReader *reader, OsImageWriter *writer);

private:
	COSImageToolDlg *m_dlg;

	void UpdateStatistics(DWORD t0, DWORD t1, DWORD offset);
};

#define __OSIMAGECOPIER_H__
#endif
