//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*++

    Copyright (c) Microsoft Corporation. All rights reserved.
    
    File: ceapimap.h
    
    Abstract:
    
    Contents:  This file contains mappings for all the WinCE API names, 
               structures and constants to the new names.

               Eg. PEGOID -> CEOID
                   PegCreateDatabase -> CeCreateDatabase

--*/

#ifndef _CEAPIMAP_H
#define _CEAPIMAP_H

//
// Prop Ids for WinCE Properties:
//
typedef CEPROPID                    PEGPROPID;
typedef PEGPROPID                   *PPEGPROPID;

//
// Unique identifier for all WINCE objects
//
typedef CEOID                       PEGOID;
typedef PEGOID                      *PPEGOID;

//
// Structures:
//
#ifdef RAPI_H
typedef CE_FIND_DATA        PEG_FIND_DATA;
typedef LPCE_FIND_DATA      LPPEG_FIND_DATA;
typedef LPLPCE_FIND_DATA    LPLPPEG_FIND_DATA;

#ifndef UNDER_CE
typedef CEDB_FIND_DATA      PEGDB_FIND_DATA;
typedef LPCEDB_FIND_DATA    LPPEGDB_FIND_DATA;
typedef LPLPCEDB_FIND_DATA  LPLPPEGDB_FIND_DATA;
#endif
#endif

typedef CEFILEINFO                  PEGFILEINFO;
typedef PEGFILEINFO                 *PPEGFILEINFO;

typedef CEDIRINFO                   PEGDIRINFO;                  
typedef PEGDIRINFO                  *PPEGDIRINFO;                

typedef CERECORDINFO                PEGRECORDINFO;              
typedef PEGRECORDINFO               *PPEGRECORDINFO;             

#define PEGDB_SORT_DESCENDING       CEDB_SORT_DESCENDING
#define PEGDB_SORT_CASEINSENSITIVE  CEDB_SORT_CASEINSENSITIVE
#define PEGDB_SORT_UNKNOWNFIRST     CEDB_SORT_UNKNOWNFIRST
#define PEGDB_SORT_GENERICORDER     CEDB_SORT_GENERICORDER

#define PEGDB_MAXDBASENAMELEN       CEDB_MAXDBASENAMELEN
#define PEGDB_MAXSORTORDER          CEDB_MAXSORTORDER

//
// Values for flag:
//
#define PEGDB_VALIDNAME             CEDB_VALIDNAME
#define PEGDB_VALIDTYPE             CEDB_VALIDTYPE
#define PEGDB_VALIDSORTSPEC         CEDB_VALIDSORTSPEC
#define PEGDB_VALIDMODTIME          CEDB_VALIDMODTIME

typedef CEDBASEINFO                 PEGDBASEINFO; 
typedef PEGDBASEINFO                *PPEGDBASEINFO;               

typedef CEOIDINFO                   PEGOIDINFO;   
typedef PEGOIDINFO                  *PPEGOIDINFO;                

//
// Flags for open database - use low word:
//
#define PEGDB_AUTOINCREMENT         CEDB_AUTOINCREMENT
 
#define PEGDB_SEEK_PEGOID           CEDB_SEEK_CEOID   
#define PEGDB_SEEK_BEGINNING        CEDB_SEEK_BEGINNING
#define PEGDB_SEEK_END              CEDB_SEEK_END
#define PEGDB_SEEK_CURRENT          CEDB_SEEK_CURRENT
#define PEGDB_SEEK_VALUESMALLER     CEDB_SEEK_VALUESMALLER   
#define PEGDB_SEEK_VALUEFIRSTEQUAL  CEDB_SEEK_VALUEFIRSTEQUAL
#define PEGDB_SEEK_VALUEGREATER     CEDB_SEEK_VALUEGREATER   
#define PEGDB_SEEK_VALUENEXTEQUAL   CEDB_SEEK_VALUENEXTEQUAL 

typedef CEBLOB                      PEGBLOB;
typedef PEGBLOB                     *PPEGBLOB;

#define PEGVT_I2                    CEVT_I2
#define PEGVT_UI2                   CEVT_UI2                    
#define PEGVT_I4                    CEVT_I4                     
#define PEGVT_UI4                   CEVT_UI4                            
#define PEGVT_FILETIME              CEVT_FILETIME               
#define PEGVT_LPWSTR                CEVT_LPWSTR                 
#define PEGVT_BLOB                  CEVT_BLOB                   

typedef CEVALUNION                  PEGVALUNION;
typedef PEGVALUNION                 *PPEGVALUNION;

#define PEGDB_PROPNOTFOUND          CEDB_PROPNOTFOUND            
#define PEGDB_PROPDELETE            CEDB_PROPDELETE              

typedef CEPROPVAL                   PEGPROPVAL; 
typedef PEGPROPVAL                  *PPEGPROPVAL; 

#define PEGDB_MAXDATABLOCKSIZE      CEDB_MAXDATABLOCKSIZE 
#define PEGDB_MAXPROPDATASIZE       CEDB_MAXPROPDATASIZE  
#define PEGDB_MAXRECORDSIZE         CEDB_MAXRECORDSIZE

#define PEGDB_ALLOWREALLOC          CEDB_ALLOWREALLOC  

#ifndef UNDER_CE
typedef CEOSVERSIONINFO             PEGOSVERSIONINFO;
typedef LPCEOSVERSIONINFO           LPPEGOSVERSIONINFO;
#endif

#define PegCreateDatabase         CeCreateDatabase
#define PegDeleteDatabase         CeDeleteDatabase
#define PegDeleteRecord           CeDeleteRecord
#define PegFindFirstDatabase      CeFindFirstDatabase
#define PegFindNextDatabase       CeFindNextDatabase
#define PegOidGetInfo             CeOidGetInfo
#define PegOpenDatabase           CeOpenDatabase
#define PegReadRecordProps        CeReadRecordProps
#define PegSeekDatabase           CeSeekDatabase
#define PegSetDatabaseInfo        CeSetDatabaseInfo
#define PegWriteRecordProps       CeWriteRecordProps

#ifndef UNDER_CE
#define PegFindFirstFile          CeFindFirstFile
#define PegFindNextFile           CeFindNextFile
#define PegFindClose              CeFindClose
#define PegGetFileAttributes      CeGetFileAttributes
#define PegSetFileAttributes      CeSetFileAttributes
#define PegCreateFile             CeCreateFile
#define PegReadFile               CeReadFile
#define PegWriteFile              CeWriteFile
#define PegCloseHandle            CeCloseHandle
#define PegFindAllFiles           CeFindAllFiles
#define PegFindAllDatabases       CeFindAllDatabases
#define PegSetFilePointer         CeSetFilePointer
#define PegSetEndOfFile           CeSetEndOfFile
#define PegCreateDirectory        CeCreateDirectory
#define PegRemoveDirectory        CeRemoveDirectory
#define PegCreateProcess          CeCreateProcess
#define PegMoveFile               CeMoveFile
#define PegCopyFile               CeCopyFile
#define PegDeleteFile             CeDeleteFile
#define PegGetFileSize            CeGetFileSize
#define PegRegOpenKeyEx           CeRegOpenKeyEx
#define PegRegEnumKeyEx           CeRegEnumKeyEx
#define PegRegCreateKeyEx         CeRegCreateKeyEx
#define PegRegCloseKey            CeRegCloseKey
#define PegRegDeleteKey           CeRegDeleteKey
#define PegRegEnumValue           CeRegEnumValue
#define PegRegDeleteValue         CeRegDeleteValue
#define PegRegQueryInfoKey        CeRegQueryInfoKey
#define PegRegQueryValueEx        CeRegQueryValueEx
#define PegRegSetValueEx          CeRegSetValueEx
#define PegGetStoreInformation    CeGetStoreInformation
#define PegGetSystemMetrics       CeGetSystemMetrics
#define PegGetDesktopDeviceCaps   CeGetDesktopDeviceCaps
#define PegGetSystemInfo          CeGetSystemInfo
#define PegSHCreateShortcut       CeSHCreateShortcut
#define PegSHGetShortcutTarget    CeSHGetShortcutTarget
#define PegCheckPassword          CeCheckPassword
#define PegGetFileTime            CeGetFileTime
#define PegSetFileTime            CeSetFileTime
#define PegGetVersionEx           CeGetVersionEx
#define PegGetWindow              CeGetWindow
#define PegGetWindowLong          CeGetWindowLong
#define PegGetWindowText          CeGetWindowText
#define PegGetClassName           CeGetClassName
#define PegGlobalMemoryStatus     CeGlobalMemoryStatus
#define PegGetSystemPowerStatusEx CeGetSystemPowerStatusEx
#define PegGetTempPath            CeGetTempPath
#define PegGetSpecialFolderPath   CeGetSpecialFolderPath

#define PegRapiInitEx             CeRapiInitEx
#define PegRapiInit               CeRapiInit
#define PegRapiUninit             CeRapiUninit
#define PegGetLastError           CeGetLastError
#define RapiFreeBuffer            CeRapiFreeBuffer
#define GetRapiError              CeRapiGetError
#endif

#ifdef CONN_INTERNAL
#include <pceapimp.h>  // internal defines
#endif

#endif // _CEAPIMAP_H

