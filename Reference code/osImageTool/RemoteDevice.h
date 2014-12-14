/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __REMOTEDEVICE_H__

#include <string>
class RemoteDevice {
public:
    RemoteDevice();
    ~RemoteDevice();

    bool Connect();
    void Disconnect();

private:
    bool m_bConnected;

    bool CheckITSDll();
    bool WaitForDevice();
};

bool ConnectToDevice();
bool ITSDCardInfo(DWORD dwDiskNr, DWORD& dwBlockSize, DWORD& dwTotalBlocks);
bool ITReadSDCard(DWORD dwDiskNr, DWORD dwOffset, BYTE *buffer, DWORD dwBytesWanted, DWORD *pdwNumberOfBytesRead);
bool ITWriteSDCard(DWORD dwDiskNr, DWORD dwOffset, const BYTE *buffer, DWORD dwBufferSize, DWORD *pdwNumberOfBytesWritten);
bool ITReadProcessMemory(HANDLE hProc, DWORD dwOffset, BYTE *buffer, DWORD dwBytesWanted, DWORD *pdwNumberOfBytesRead);
bool CeCopyFileToDevice(const std::string& srcfile, const std::string& dstfile);

#define __REMOTEDEVICE_H__
#endif
