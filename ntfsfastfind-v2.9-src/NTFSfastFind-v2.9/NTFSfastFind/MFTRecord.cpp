// ------------------------------------------------------------------------------------------------
// Read Master File Table from Windows NTFS file system.
//
// Original code from T.YogaRamanan's Undelete project posted to CodeProject 13-Jan-2005.
// http://www.codeproject.com/KB/files/NTFSUndelete.aspx
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include "MFTRecord.h"
#include <assert.h>

char* MFTRecord::sMFTRecordTypeStr[] =
{
    "0x00",
    "Standard Information",  //  0x10
    "Attribute List",        //  0x20
    "File Name",             //  0x30
    "Volume Version",        //  0x40
    "Security Descriptor",   //  0x50
    "Volume Name",           //  0x60
    "Volume Information",    //  0x70
    "Data",                  //  0x80
    "Index Root",            //  0x90
    "Index Allocation",      //  0xa0
    "Bitmap",                //  0xb0
    "Symbolic Link",         //  0xc0
    "Extended Attribute",    //  0xd0
    "EA",                    //  0xe0
    "0xf0"
};

// ------------------------------------------------------------------------------------------------
static DWORD ReturnError(DWORD error)
{
    return error;   // handy place to set break point.
}

// ------------------------------------------------------------------------------------------------
MFTRecord::MFTRecord() :
    m_bInUse(false),
    m_bSparse(false),
    m_nameCnt(0),
    m_streamCnt(0),
    m_fragCnt(0),
    m_hDrive(INVALID_HANDLE_VALUE),
    m_dwMFTRecSize(1023),   // usual size
	m_dwCurPos(0),
    m_dwBytesPerCluster(0)
{
    ZeroMemory(&m_attrStandard, sizeof(m_attrStandard));
	ZeroMemory(&m_attrFilename,sizeof(m_attrFilename));
	ZeroMemory(m_typeCnt, sizeof(m_typeCnt));
}

// ------------------------------------------------------------------------------------------------
MFTRecord::~MFTRecord()
{
}

// ------------------------------------------------------------------------------------------------
// Set the internal details:
//  n64StartPos is the byte from the starting of the physical disk
//  dwRecSize is the record size in the MFT table
//  dwBytesPerCluster is the bytes per cluster

int MFTRecord::SetRecordInfo(LONGLONG  n64StartPos, DWORD dwRecSize, DWORD dwBytesPerCluster)
{
	if (!dwRecSize)
		return ReturnError(ERROR_INVALID_PARAMETER);

	if (dwRecSize % 2)
		return ReturnError(ERROR_INVALID_PARAMETER);

	if (!dwBytesPerCluster)
		return ReturnError(ERROR_INVALID_PARAMETER);

	if (dwBytesPerCluster % 2)
		return ReturnError(ERROR_INVALID_PARAMETER);

	m_dwMFTRecSize      = dwRecSize;
	m_dwBytesPerCluster = dwBytesPerCluster;
	m_n64StartPos       = n64StartPos;

	return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
/// inMFTBlock is the MFT record which defines the file to load.
/// If it points to $MFT, the Master File Table is loaded (loadData==true) and optionally
/// filtered (pMFTFilter!=NULL)

int MFTRecord::ExtractFileOrMFT(
        const Block& inMFTBlock, bool loadData, size_t maxSize, 
        const FsFilter* pMFTFilter,
        const StreamFilter* pStreamFilter)
{
	if (inMFTBlock.size() < m_dwMFTRecSize)
		return ReturnError(ERROR_INVALID_PARAMETER);

	int nRet;
	
    // If 'loadData' is true, the files contents are stroed in m_outFileData.
    m_MFTBlock   = inMFTBlock;
	m_dwCurPos   = 0;
    m_outFileData.resize(0);
    Buffer tmpBuffer;

	// read the record header in MFT table
	const MFT_FILE_HEADER* pNtfsMFT = m_MFTBlock.OutPtr<MFT_FILE_HEADER>(0);

	if (memcmp(pNtfsMFT->szSignature, "FILE", 4))
		return ReturnError(ERROR_INVALID_PARAMETER);     // not the right signature

    // 1=nonResident attributes, 2=record is directory.
	m_bInUse = (pNtfsMFT->wFlags & 0x01);   // mask 0x01  Record is in use
									        // mask 0x02  Record is a directory
    m_bSparse = false;
    m_fragCnt = 0;

    /*
    http://hetmanrecovery.com/file_system/ntfs_recovery/ntfs_mft.htm

    NTFS incorporates fixup values into data structures that are over one sector in length. With fixup values,
    the last two bytes of each sector in large data structures are replaced with a signature value when the 
    data structure is written to disk. The signature is later used to verify the integrity of the data by 
    verifying that all sectors have the same signature. Note that fixups are used only in data structures 
    and not in sectors that contain file content.

    The data structures that use fixups have header fields that identify the current 16-bit signature value 
    and an array that contains the original values. When the data structure is written to disk, the signature 
    value is incremented by one, the last two bytes of each sector are copied to the array, and the signature 
    value is written to the last two bytes of each sector. When reading the data structure, the OS should 
    verify that the last two bytes of each sector are equal to the signature value, and the original values 
    are then replaced from the array.

    Fixups are used to detect damaged sectors and corrupt data structures. If only one sector of a multi-sector 
    data structure was written, the fixup will be different from the signature, and the OS will know that 
    the data are corrupt. When we dissect our example file system, we will need to 
    */

    WORD  fixSize = pNtfsMFT->wFixupSize;
    if (fixSize != 0)
    {
        const WORD* pFixUp =  m_MFTBlock.OutPtr<WORD>(pNtfsMFT->wFixupOffset);
        const WORD sig = *pFixUp++;
        fixSize--;
        WORD* pData = (WORD*)pNtfsMFT + 256-1;
        while (fixSize-- != 0)
        {
            // assert(*pData == sig);
            *pData = *pFixUp++;
            pData += 256;
        }
    }

	m_dwCurPos = pNtfsMFT->wAttribOffset;

    m_nameCnt   = 0;
    m_streamCnt = 0;

    const NTFS_ATTRIBUTE* pNtfsAttr = NULL;

	do
	{	// Extract the attribute header, only the first field is required.
        try
        {
	        pNtfsAttr = m_MFTBlock.OutPtr<NTFS_ATTRIBUTE>(m_dwCurPos, sizeof(pNtfsAttr->dwType));
        } 
        catch (const BlockException&)
        {
            // TODO - how should we deal with this error ?
			return ERROR_SUCCESS;
        }

        /*

        Table 5.3. The fourteen system-defined attributes in NTFS. [DUC98 /attribute.html] Type code  Attribute name  Description  
        10  $STANDARD_INFORMATION   Contains timestamps for the time when the file was created, last modified and last access. Also holds a flag telling if it is a directory, if the file is read-only, if it should be hidden and all the other traditional MS-DOS attributes. [DUC98 /standard.html]  
        20  $ATTRIBUTE_LIST         This attribute is used to find the extension file record and to see what attributes are stored in it when all attributes cannot fit into one file record. It is very rarely used and is mostly observed in heavily fragmented files. [CUS94, 23]  
        30  $FILE_NAME              Holds the file name of the file record. A file record can have multiple file name attributes to support MS-DOS based file names. [CUS94, 23]  
        40  $VOLUME_VERSION         This have I not ever encountered, [DUC98 /volumever.html] states that it could have something to do with what version of NTFS that created the volume.  
        50  $SECURITY_DESCRIPTOR    Holds information on: who owns the file, who can access it and what the different users can do with it. [CUS94, 23]  
        60  $VOLUME_NAME            Just the name of the volume. [DUC98 /volumename.html]  
        70  $VOLUME_INFORMATION     Stores the volume dirty flag. [DUC98 /volumeinf.html]  
        80  $DATA                   The usual content of a data file. [DUC98 /data.html]  
        90  $INDEX_ROOT             These three attributes together store the content of a directory. [CUS94, 23]  
        a0  $INDEX_ALLOCATION  
        b0  $BITMAP  
        c0  $SYMBOLIC_LINK          Unused. [RUS98Ja]  
        d0  $EA_INFORMATION         Used to implement HPFS style extended attribute for OS/2 subsystems and for OS/2 clients of Windows NT file servers. [CUS94, 23]  
        e0  $EA  

        */
        m_typeCnt[(pNtfsAttr->dwType >> 4) & 0xf]++;
        // m_mftSize += (pNtfsAttr->wFullLength);

		switch(pNtfsAttr->dwType) // extract the attribute data 
		{
		// I have not implemented the processing of all the attributes.
		// I have implemented attributes necessary for file & file data extraction

		case 0: //UNUSED
			break;

		case 0x10: // STANDARD_INFORMATION
	        memset(&m_attrStandard, 0, sizeof(MFT_STANDARD ));
            tmpBuffer.clear();
			nRet = ExtractData(*pNtfsAttr, tmpBuffer, 512);
			if (nRet)
				return nRet;
            if (tmpBuffer.size() < sizeof(m_attrStandard))
            {
                std::wcout << "Error Attribute bufferSize=" << tmpBuffer.size() << " expect min size of " << sizeof(m_attrStandard) << std::endl;
                return ReturnError(ERROR_INVALID_PARAMETER);
            }
            memcpy(&m_attrStandard, &tmpBuffer[0], min(tmpBuffer.size(), sizeof(m_attrStandard)));
			break;

		case 0x30: // FILE_NAME
	        memset(&m_attrFilename, 0, sizeof(MFT_FILEINFO));
            tmpBuffer.clear();
			nRet = ExtractData(*pNtfsAttr, tmpBuffer, 4096);
			if (nRet)
				return nRet;

            if (tmpBuffer.size() < sizeof(m_attrFilename) - sizeof(m_attrFilename.wFilename))
                return ReturnError(ERROR_INVALID_PARAMETER);
            
            memcpy(&m_attrFilename, &tmpBuffer[0], min(tmpBuffer.size(), sizeof(m_attrFilename)));
          
            if (m_attrFilename.chFileNameLength < ARRAYSIZE(m_attrFilename.wFilename))
                m_attrFilename.wFilename[m_attrFilename.chFileNameLength] = 0;
            m_nameCnt++;
			break;

		case 0x40: // OBJECT_ID
			break;
		case 0x50: // SECURITY_DESCRIPTOR
			break;
		case 0x60: // VOLUME_NAME
			break;
		case 0x70: // VOLUME_INFORMATION
			break;
		case 0x80: // DATA
            m_streamCnt++;
            if (loadData)
            {
                // Append to buffer
			    nRet = ExtractData(*pNtfsAttr, m_outFileData, maxSize, pMFTFilter);
		     	if (nRet)
		    		return nRet;
            }
            else
            {
                // Note: $BadClus has two DATA records, 2nd record has Stream name $Bad
                // and allocate and realSize span entire volume. DataRun Offset is non-zero
                // to indicate a spares file.

                if (pNtfsAttr->uchNonResFlag)
                {
                    if (pNtfsAttr->uchNameLength != 0)
                    {
                        if (pStreamFilter)
                        {
                            // Get Stream Name (currently not passed back to caller)
                            std::vector<wchar_t> streamName(pNtfsAttr->uchNameLength+1);
                            memcpy(&streamName[0], (char*)pNtfsAttr + pNtfsAttr->wNameOffset, pNtfsAttr->uchNameLength * 2);
                            const wchar_t* pStreamName = &streamName[0];
                            pStreamFilter->IsMatch(m_attrFilename.wFilename, pStreamName, pNtfsAttr->wFullLength);
                        }
                    }

                    // NonResidence file data.
                    // Get actual 'data' size from this chunk of resident file data.
		            LONGLONG realSize = pNtfsAttr->Attr.NonResident.n64RealSize;
                    m_attrFilename.n64RealSize = realSize;

			        if (m_fileOnDisk.empty())
                        ExtractDataPos(*pNtfsAttr, m_outFileData, maxSize, pMFTFilter);

                    if (pNtfsAttr->Attr.NonResident.wDatarunOffset != 0)
                    {
                        // Sparse file have data runs (I think)
                        //
                        // DataRuns    [[OL] [DataSize...] [Offset...] ]...
                        //  First byte, low nibble is byte length of DataSize value
                        //              high nibble is byte length of Offset value
                        //  Repeat until OL is zero.
                        // ( [LengthOfSizes] [DataSize] [Offset] ) ... repeat until LengthOfSizes is zero.
                        const BYTE* pRunList = (const BYTE*)pNtfsAttr + pNtfsAttr->Attr.NonResident.wDatarunOffset;

#if 1
                        // Parse allocation list and count fragments and flag sparse files.
                        m_fragCnt = 0;
                        m_bSparse = false;
                        DWORD64 totalSize = 0;

                        while (*pRunList != 0)
                        {
                            unsigned char dataSizeLen = pRunList[0] & 0x0f;   
                            unsigned char offsetLen   = (pRunList[0] & 0xf0) >> 4;

                            m_bSparse |= (offsetLen == 0);     // Sparse if no offset.

                            pRunList++;
                            DWORD64 dataSize = 0;
                            unsigned byteIdx = 0;
                            while (dataSizeLen-- != 0)
                            {
                                ((BYTE*)&dataSize)[byteIdx++] = *pRunList++;
                            }
                            DWORD64 offset = 0;
                            byteIdx = 0;
                            while (offsetLen-- != 0)
                            {
                                ((BYTE*)&offset)[byteIdx++] = *pRunList++;

                            }

                            if (offset != 0)
                                m_fragCnt++;

                            totalSize += dataSize;
                            // std::wcout << m_attrFilename.wFilename << " offset=" << offset << " size=" << dataSize * m_dwBytesPerCluster << std::endl;
                        }
#endif
                    }
                
                 
                }
                else
	            {
                    // Residence file data.
                    // Get actual 'data' size from this chunk of resident file data.
		            LONGLONG realSize = pNtfsAttr->Attr.Resident.dwLength;
                    m_attrFilename.n64RealSize = realSize;
                }
            }
			break;

		case 0x90: //INDEX_ROOT
            break;
		case 0xa0: //INDEX_ALLOCATION
			break;
		case 0xb0: //BITMAP
			break;

		case 0xc0: //REPARSE_POINT
			break;
		case 0xd0: //EA_INFORMATION
			break;
		case 0xe0: //EA
			break;
		case 0xf0: //PROPERTY_SET
			break;
		case 0x100: //LOGGED_UTILITY_STREAM
			break;
		case 0x1000: //FIRST_USER_DEFINED_ATTRIBUTE
			break;

		case 0xFFFFFFFF: // END 
			return ERROR_SUCCESS;

		default:
			break;
		};

		m_dwCurPos += pNtfsAttr->wFullLength; // go to the next location of attribute

	} while (pNtfsAttr->wFullLength != 0);

	return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
/// inMFTBlock is the MFT record which defines the file to load.

int MFTRecord::ExtractItems(const Block& inMFTBlock, ItemList& itemList, size_t maxSize)
{
	if (inMFTBlock.size() < m_dwMFTRecSize)
		return ReturnError(ERROR_INVALID_PARAMETER);

    m_MFTBlock   = inMFTBlock;
	m_dwCurPos   = 0;
    m_outFileData.resize(0);

	// read the record header in MFT table
	const MFT_FILE_HEADER* pNtfsMFT = inMFTBlock.OutPtr<MFT_FILE_HEADER>(m_dwCurPos);

	if (memcmp(pNtfsMFT->szSignature, "FILE", 4))
		return ReturnError(ERROR_INVALID_PARAMETER);     // not the right signature

    // 1=nonResident attributes, 2=record is directory.
	m_bInUse = (pNtfsMFT->wFlags & 0x01);   //0x01  Record is in use
									        //0x02 	Record is a directory

	//m_dwCurPos = (ntfsMFT.wFixupOffset + ntfsMFT.wFixupSize*2); 
	m_dwCurPos = pNtfsMFT->wAttribOffset;

    const NTFS_ATTRIBUTE* pNtfsAttr = NULL;

	do
	{	// Extract the attribute header, only the first field is required.
        try
        {
	        pNtfsAttr = inMFTBlock.OutPtr<NTFS_ATTRIBUTE>(m_dwCurPos, sizeof(pNtfsAttr->dwType));
        } 
        catch (const BlockException&)
        {
		    return ReturnError(ERROR_INVALID_PARAMETER);
        }

        /*

        Table 5.3. The fourteen system-defined attributes in NTFS. [DUC98 /attribute.html] Type code  Attribute name  Description  
        10  $STANDARD_INFORMATION   Contains timestamps for the time when the file was created, last modified and last access. Also holds a flag telling if it is a directory, if the file is read-only, if it should be hidden and all the other traditional MS-DOS attributes. [DUC98 /standard.html]  
        20  $ATTRIBUTE_LIST         This attribute is used to find the extension file record and to see what attributes are stored in it when all attributes cannot fit into one file record. It is very rarely used and is mostly observed in heavily fragmented files. [CUS94, 23]  
        30  $FILE_NAME              Holds the file name of the file record. A file record can have multiple file name attributes to support MS-DOS based file names. [CUS94, 23]  
        40  $VOLUME_VERSION         This have I not ever encountered, [DUC98 /volumever.html] states that it could have something to do with what version of NTFS that created the volume.  
        50  $SECURITY_DESCRIPTOR    Holds information on: who owns the file, who can access it and what the different users can do with it. [CUS94, 23]  
        60  $VOLUME_NAME            Just the name of the volume. [DUC98 /volumename.html]  
        70  $VOLUME_INFORMATION     Stores the volume dirty flag. [DUC98 /volumeinf.html]  
        80  $DATA                   The usual content of a data file. [DUC98 /data.html]  
        90  $INDEX_ROOT             These three attributes together store the content of a directory. [CUS94, 23]  
        a0  $INDEX_ALLOCATION  
        b0  $BITMAP  
        c0  $SYMBOLIC_LINK          Unused. [RUS98Ja]  
        d0  $EA_INFORMATION         Used to implement HPFS style extended attribute for OS/2 subsystems and for OS/2 clients of Windows NT file servers. [CUS94, 23]  
        e0  $EA  

        */
        MFTitem mftItem;
        mftItem.type = pNtfsAttr->dwType;
        mftItem.pNTFSAttribute = pNtfsAttr;

		switch (pNtfsAttr->dwType) // extract the attribute data 
		{
		case 0: //UNUSED
			break;
        case 0x80: // DATA
            break; //   skip saving data.

		case 0x10: // STANDARD_INFORMATION
		case 0x30: // FILE_NAME
        case 0x40: // OBJECT_ID
		case 0x50: // SECURITY_DESCRIPTOR
		case 0x60: // VOLUME_NAME
		case 0x70: // VOLUME_INFORMATION
		
        case 0x90: //INDEX_ROOT
		case 0xa0: //INDEX_ALLOCATION
		case 0xb0: // BITMAP
		case 0xc0: // REPARSE_POINT
		case 0xd0: // EA_INFORMATION
		case 0xe0: // EA
		case 0xf0: // PROPERTY_SET
		case 0x100:  // LOGGED_UTILITY_STREAM
		case 0x1000: // FIRST_USER_DEFINED_ATTRIBUTE
        default:
            if (!pNtfsAttr->uchNonResFlag)
            {
                // Set Block to point to Resident data.
            	DWORD dwDataLen = pNtfsAttr->Attr.Resident.dwLength;
                mftItem.data.Set(inMFTBlock.OutPtr<char>(
                    m_dwCurPos + pNtfsAttr->Attr.Resident.wAttrOffset, dwDataLen), dwDataLen);
            }
            else
            {
                // Save current buffer size, append data, push offset and size as a Block.
                size_t offset = m_outFileData.size();
                int nRet = ExtractData(*pNtfsAttr, m_outFileData, maxSize, NULL);
                if (nRet)
		            return nRet;

                // NOTE: Block does not have valid pointer, but rather an offset and length.
                mftItem.data.Set((void*)offset, m_outFileData.size() - offset);
            }
            itemList.push_back(mftItem);
			break;

		case 0xFFFFFFFF: // END 
            // assert(pNtfsAttr->wFullLength == 0);
			// return ERROR_SUCCESS;
            break;
		};

		m_dwCurPos += (pNtfsAttr->wFullLength); // go to the next location of attribute

	} while (pNtfsAttr->dwType != 0xFFFFFFFF && pNtfsAttr->wFullLength != 0);

    // Adjust all 'nonResident' Block pointers to be relative to base of m_outFileData.
    for (unsigned itemIdx = 0; itemIdx != itemList.size(); itemIdx++)
    {
        if (itemList[itemIdx].pNTFSAttribute->uchNonResFlag)
        {
            Block& data = itemList[itemIdx].data;
            data.Set(m_outFileData.Data() + (size_t)data.OutVPtr(0), data.size());
        }
    }


	return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// Extract the attribute data from the MFT table and append to buffer.
// Data can be Resident & non-resident
int MFTRecord::ExtractData(
        const NTFS_ATTRIBUTE& ntfsAttr, 
        Buffer& outBuffer,              
        size_t maxSize,
        const FsFilter* pMFTFilter)     // Only set when reading MFT table.
{
	DWORD dwCurPos = m_dwCurPos;

	if (!ntfsAttr.uchNonResFlag)
	{
        // Residence attribute, this always resides in the MFT table itself.
		DWORD dwDataLen = ntfsAttr.Attr.Resident.dwLength;
        size_t offset = outBuffer.size();
        if (offset + dwDataLen > maxSize)
            return ReturnError(ERROR_NOT_ENOUGH_MEMORY);
        outBuffer.resize(offset + dwDataLen);
        m_MFTBlock.Copy(&outBuffer[offset], dwCurPos+ntfsAttr.Attr.Resident.wAttrOffset, dwDataLen);
	}
	else
	{
        // Non-residence attribute, this resides in the other part of the physical drive

        LONGLONG dwDataLen = ntfsAttr.Attr.NonResident.n64AllocSize;
		if (!ntfsAttr.Attr.NonResident.n64AllocSize) // i don't know Y, but fails when its zero
			dwDataLen = (ntfsAttr.Attr.NonResident.n64EndVCN - ntfsAttr.Attr.NonResident.n64StartVCN) + 1;

		LONGLONG n64Len;    // Data length
        LONGLONG n64Offset; // Data offset
		LONGLONG n64LCN =0; // File offset to data on disk (LCN logical clustter number on physical drive)
        
        m_fileOnDisk.clear();

		dwCurPos += ntfsAttr.Attr.NonResident.wDatarunOffset;

        //  Iterator over non-resident data.
        // ( [LengthOfSizes] [DataSize] [Offset] ) ... repeat until LengthOfSizes is zero.
        //  Where LenghOfSizes is a byte with low nibble the size of the DataSize and
        //  the upper nibble the size of the Offset value.
        //  Ex:     0x04     = 4byte DataSize and 0byte Offset 
        //          0x44     = 4byte DataSize and 4byte Offset
		for (;;)
		{
			// ---- Read the length of LCN/VCN and length  
			BYTE lenOffSz = 0;
            m_MFTBlock.Copy(&lenOffSz, dwCurPos, sizeof(BYTE));
			dwCurPos += sizeof(BYTE);

			if (!lenOffSz)
				break;

            // Split into Length Size and Offset Size
			BYTE lenSize     = lenOffSz & 0x0F;
			BYTE offSize	 = (lenOffSz & 0xF0) >> 4;

			// ---- Read the data length  
			n64Len = 0;
            m_MFTBlock.Copy(&n64Len, dwCurPos, lenSize);
			dwCurPos += lenSize;

			// ---- Read the LCN/VCN offset 
			n64Offset = 0;
			m_MFTBlock.Copy(&n64Offset, dwCurPos, offSize);
			dwCurPos += offSize;

			//  If the last bit of n64Offset is 1 then fill remainder with bits on.
			if ((((char*)&n64Offset)[offSize-1])&0x80)
				for (int i=sizeof(LONGLONG)-1; i > (offSize-1); i--)
					((char*)&n64Offset)[i] = (char)0xff;
			
			n64LCN += n64Offset;
			n64Len *= m_dwBytesPerCluster;

            // Store file's disk layout for later use, ex: when loading directory names.
            m_fileOnDisk.push_back(std::pair<LONGLONG,LONGLONG>(n64LCN, n64Len));

            if (outBuffer.size() > maxSize)
                return ReturnError(ERROR_NOT_ENOUGH_MEMORY);

			// Data is available out side the MFT table, physical drive should be accessed
			int nRet = ReadRaw(n64LCN, outBuffer, (DWORD)n64Len, pMFTFilter);
			if (nRet)
				return nRet;
		}
	}

	return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// Extract the attribute data from the MFT table and append to buffer.
// Data can be Resident & non-resident
int MFTRecord::ExtractDataPos(
        const NTFS_ATTRIBUTE& ntfsAttr, 
        Buffer& outBuffer,              
        size_t maxSize,
        const FsFilter* pMFTFilter)     // Only set when reading MFT table.
{
	DWORD dwCurPos = m_dwCurPos;

	if (!ntfsAttr.uchNonResFlag)
	{
        m_fileOnDisk.clear();
	}
	else
	{
        // Non-residence attribute, this resides in the other part of the physical drive

        LONGLONG dwDataLen = ntfsAttr.Attr.NonResident.n64AllocSize;
		if (!ntfsAttr.Attr.NonResident.n64AllocSize) // i don't know Y, but fails when its zero
			dwDataLen = (ntfsAttr.Attr.NonResident.n64EndVCN - ntfsAttr.Attr.NonResident.n64StartVCN) + 1;

		LONGLONG n64Len;    // Data length
        LONGLONG n64Offset; // Data offset
		LONGLONG n64LCN =0; // File offset to data on disk (LCN logical clustter number on physical drive)
        
        m_fileOnDisk.clear();

		dwCurPos += ntfsAttr.Attr.NonResident.wDatarunOffset;

        //  Iterator over non-resident data.
        // ( [LengthOfSizes] [DataSize] [Offset] ) ... repeat until LengthOfSizes is zero.
        //  Where LenghOfSizes is a byte with low nibble the size of the DataSize and
        //  the upper nibble the size of the Offset value.
        //  Ex:     0x04     = 4byte DataSize and 0byte Offset 
        //          0x44     = 4byte DataSize and 4byte Offset
		for (;;)
		{
			// ---- Read the length of LCN/VCN and length  
			BYTE lenOffSz = 0;
            m_MFTBlock.Copy(&lenOffSz, dwCurPos, sizeof(BYTE));
			dwCurPos += sizeof(BYTE);

			if (!lenOffSz)
				break;

            // Split into Length Size and Offset Size
			BYTE lenSize     = lenOffSz & 0x0F;
			BYTE offSize	 = (lenOffSz & 0xF0) >> 4;

			// ---- Read the data length  
			n64Len = 0;
            m_MFTBlock.Copy(&n64Len, dwCurPos, lenSize);
			dwCurPos += lenSize;

			// ---- Read the LCN/VCN offset 
			n64Offset = 0;
			m_MFTBlock.Copy(&n64Offset, dwCurPos, offSize);
			dwCurPos += offSize;

			//  If the last bit of n64Offset is 1 then fill remainder with bits on.
			if ((((char*)&n64Offset)[offSize-1])&0x80)
				for (int i=sizeof(LONGLONG)-1; i > (offSize-1); i--)
					((char*)&n64Offset)[i] = (char)0xff;
			
			n64LCN += n64Offset;
			n64Len *= m_dwBytesPerCluster;

            // Store file's disk layout for later use, ex: when loading directory names.
            m_fileOnDisk.push_back(std::pair<LONGLONG,LONGLONG>(n64LCN, n64Len));

            if (outBuffer.size() > maxSize)
                return ReturnError(ERROR_NOT_ENOUGH_MEMORY);
		}
	}

	return ERROR_SUCCESS;
}
// ------------------------------------------------------------------------------------------------
// Read the data from the physical drive.
int MFTRecord::ReadRaw(LONGLONG n64LCN, Buffer& buffer, DWORD dwLen, const FsFilter* pMFTFilter)
{
    DWORD chunkSize = m_dwBytesPerCluster * 16;  
	LARGE_INTEGER n64Pos;
	n64Pos.QuadPart = (n64LCN)*m_dwBytesPerCluster;
	n64Pos.QuadPart += m_n64StartPos;

	// Data is available in the relative sector from the begining of the drive	
	// so point that data.
	int nRet = SetFilePointer(m_hDrive, n64Pos.LowPart, &n64Pos.HighPart, FILE_BEGIN);
	if (nRet == -1)
		return GetLastError();

	DWORD dwBytesRead  = 0;
	DWORD dwBytes	   = 0;
	DWORD dwTotRead	   = 0;
	DWORD dwTotSaved   = 0;
    bool haveFilter = (pMFTFilter != NULL) && pMFTFilter->IsValid();
    
	while (dwTotRead < dwLen)
	{
		// dwBytesRead = m_dwBytesPerCluster;
        dwBytesRead = haveFilter ? min(chunkSize, dwLen) : dwLen;  
        size_t begSize = buffer.size();
        buffer.resize(begSize + dwBytesRead);
	    BYTE *pTmp = &buffer[begSize];

		// Read chunk of data.
		nRet = ReadFile(m_hDrive, pTmp, dwBytesRead, &dwBytes, NULL);
		if (!nRet)
        {
            buffer.resize(begSize);
			return GetLastError();
        }

#if 0
        // Fix checksum
        const unsigned sSectorSize = 512;
        unsigned numSectors = dwBytes / sSectorSize;
        WORD* pData = (WORD*)pTmp;
        for (unsigned sector = 0; sector < numSectors; sector++) 
        {
            WORD sum = 0;
            for (unsigned secOff = 0; secOff < sSectorSize/2-1; secOff++)
                sum += *pData++;
            WORD cksum = *pData++;
            if (cksum != sum)
            {
                int e=0;
            }
        }
#endif

		dwTotRead += dwBytes;

        // Optionally filter MFT payload.
        if (haveFilter && dwBytes != 0)
        {
            // Read chunk is assumed to be in units of MFT records.
            DWORD mftCnt = dwBytes / m_dwMFTRecSize;
            assert(mftCnt * m_dwMFTRecSize == dwBytes);

            MFTRecord mftRecord;
            mftRecord.SetRecordInfo(0LL, m_dwMFTRecSize, m_dwBytesPerCluster);

            BYTE* pInTmp = pTmp;
            BYTE* pOutTmp = pTmp;
            dwBytes = 0;

            while (mftCnt-- != 0)
            {
                Block mftBlock(pInTmp, m_dwMFTRecSize);
                if (0 == mftRecord.ExtractFile(mftBlock, false, 0))
                {
                    if (pMFTFilter->IsMatch(mftRecord.m_attrStandard, mftRecord.m_attrFilename, &mftRecord))
                    {
                        if (pInTmp != pOutTmp)
                            memcpy(pOutTmp, pInTmp, m_dwMFTRecSize);
                        dwBytes += m_dwMFTRecSize;
                        pOutTmp += m_dwMFTRecSize;
                    }
                    pInTmp += m_dwMFTRecSize;
                }
            }

            for (unsigned mftRecIdx = 1; mftRecIdx < 16; mftRecIdx++)
                m_typeCnt[mftRecIdx] += mftRecord.GetTypeCnts()[mftRecIdx];
        }

        buffer.resize(begSize + dwBytes);
        dwTotSaved += dwBytes;
	}

	dwLen = dwTotSaved;

	return ERROR_SUCCESS;
}



