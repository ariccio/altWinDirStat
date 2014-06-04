#include "stdafx.h"

#include "OsImageCopier.h"
#include "OsImageToolDlg.h"
#include "OsImageReader.h"
#include "OsImageWriter.h"
#include "vectorutils.h"
#include "debug.h"
#include "ErrorHandler.h"

#define OSIMAGEBLOCKSIZE 32768
#define OSSTART 0x80040000
#define OSEND   0x81f00000

OsImageCopier::OsImageCopier(COSImageToolDlg *dlg)
{
    m_dlg= dlg;
}

void OsImageCopier::UpdateStatistics(DWORD t0, DWORD t1, DWORD offset)
{
    m_dlg->SetPercentage(100.0*(offset-OSSTART)/(OSEND-OSSTART));

    double bps= 1000.0*(offset-OSSTART)/(t1-t0);
    m_dlg->SetSpeed(bps);
    m_dlg->SetRemainingTime((long)((OSEND-offset)/bps));
}

bool OsImageCopier::copy(OsImageReader *reader, OsImageWriter *writer)
{
    ByteVector buffer;

	if (!reader->Open())		
	{
		g_err.Set("error opening input file");
		return false;
	}
	if (!writer->Open())
	{
		g_err.Set("error opening target device");
		return false;
	}

    DWORD t0= GetTickCount();
    DWORD t1= t0;
    m_dlg->SetPercentage(0);

    for (DWORD offset= OSSTART ; offset < OSEND ; offset += OSIMAGEBLOCKSIZE)
    {
        if (offset!=OSSTART && GetTickCount()/1000 != t1/1000)
        {
			t1= GetTickCount();
			UpdateStatistics(t0, t1, offset);
        }
        if (!reader->ReadData(offset, OSIMAGEBLOCKSIZE, buffer))
        {
			g_err.Set(stringformat("Reading osimage: %s", LastError().c_str()));
            return false;
        }
        if (m_dlg->m_bCancelled || m_dlg->PumpMessages(false))
            break;
        if (!writer->WriteData(offset, buffer))
        {
            g_err.Set(stringformat("Writing osimage: %s", LastError().c_str()));
            return false;
        }
        if (m_dlg->m_bCancelled || m_dlg->PumpMessages(false))
            break;
    }

	if (!m_dlg->m_bCancelled)
	{
		if (!reader->Close())		
		{
			g_err.Set("error closing input file");
			return false;
		}
		if (!writer->Close())
		{
			g_err.Set("error finishing osimage");
			return false;
		}
		t1= GetTickCount();
		UpdateStatistics(t0, t1, offset);

		m_dlg->MessageBox("Done");
	}
	else
		m_dlg->MessageBox("Cancelled");

	return true;
}

