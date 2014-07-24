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

Module Name:


    cesync.h

Abstract:

    Include file for synchronization modules for Windows CE

--*/
#ifndef _INC_CESYNC_H
#define _INC_CESYNC_H

// max size of the object type name
#define MAX_OBJTYPE_NAME    100

// max. size of a packet in IReplObjHandler::GetPacket & IReplObjHandler::SetPacket (about 254K)
#define MAX_PACKET_SIZE     260000

#define MAX_ACTIVE_VOL      16                  // up to 16 active volumes (including the default system volume) can be synchronized during each connection

typedef struct _tagReplSetup *PREPLSETUP;

typedef TCHAR   OBJTYPENAME[ MAX_OBJTYPE_NAME ];
typedef char    OBJTYPENAMEA[ MAX_OBJTYPE_NAME ];
typedef WCHAR   OBJTYPENAMEW[ MAX_OBJTYPE_NAME ];

#define FACILITY_CESYNC     0x14
#define MAKE_RERR(code)     ((HRESULT)(MAKE_SCODE( SEVERITY_ERROR, FACILITY_CESYNC, code )))
#define MAKE_RWRN(code)     ((HRESULT)(MAKE_SCODE( SEVERITY_SUCCESS, FACILITY_CESYNC, code )))

#ifndef UNDER_CE
typedef struct _REPLOBJ FAR *HREPLOBJ;
typedef struct _REPLITEM FAR *HREPLITEM;
typedef struct _REPLFLD FAR *HREPLFLD;

#endif

// Error/Return code used
#define RERR_SHUT_DOWN      MAKE_RERR( 0x0001 ) // serious error, asking implementation to shut down immediately
#define RERR_STORE_REPLACED MAKE_RERR( 0x0002 ) // the store was replaced.
#define RERR_CANCEL         MAKE_RERR( 0x0003 ) // user cancel the operation
#define RERR_RESTART        MAKE_RERR( 0x0004 ) // restart the operation, applicable in RSC_END_SYNC & RSC_END_CHECK
#define RERR_IGNORE         MAKE_RERR( 0x0005 ) // used by IReplStore::GetConflictInfo.
#define RERR_UNLOAD         MAKE_RERR( 0x0006 ) // used by IReplStore::ActivateDialog or IReplStore::IsFolderChanged to request unloading of replication modules
#define RERR_OBJECT_DELETED MAKE_RERR( 0x0007 ) // used by IReplStore::IsValidObject, indicates the object identified by the hObject is deleted
#define RERR_CORRUPT        MAKE_RERR( 0x0008 ) // used by IReplStore::IsValidObject, indicates the object identified by the hObject is corrupted
#define RERR_NO_DEVICE      MAKE_RERR( 0x0009 ) // returned by IReplNotify::QueryDevice. indicates no selected or connected device exists
#define RERR_NO_ERR_PROMPT  MAKE_RERR( 0x0010 ) // returned by IReplStore::Initialize. indicates error initializing. No UI is needed to show this error.
#define RERR_DISCARD        MAKE_RERR( 0x0011 ) // returned by IReplObjHandler::SetPacket. indicates this object should be discarded from the device immediately.
#define RERR_DISCARD_LOCAL  MAKE_RERR( 0x0012 ) // returned by IReplObjHandler::SetPaket. indicates this object should be discarded from the desktop only.
#define RERR_VOL_INACTIVE   MAKE_RERR( 0x0013 ) // returned by IReplObjHandler::GetPacket && IReplObjHandler::SetPacket, the volume has become inactive.
#define RERR_BIG_OBJ_TYPE   MAKE_RERR( 0x0014 ) // returned by IReplNotify::QueryDevice on QDC_SYNC_DATA
#define RERR_BIG_CODE       MAKE_RERR( 0x0015 ) // returned by IReplNotify::QueryDevice on QDC_SYNC_DATA
#define RERR_UNMATCHED      MAKE_RERR( 0x0016 ) // returned by IReplNotify::QueryDevice on QDC_SYNC_DATA
#define RERR_DEVICE_WIN     MAKE_RERR( 0x0017 ) // returned by IReplStore::GetConflictInfo, resolve the conflict so device object wins
#define RERR_DESKTOP_WIN    MAKE_RERR( 0x0018 ) // returned by IReplStore::GetConflictInfo, resolve the conflict so desktop object wins
#define RERR_SKIP_ALL_OBJ   MAKE_RERR( 0x0019 ) // returned by IReplStore::ReportStatus on RSC_WRITE_OBJ_FAILED, skip sync of all remaining objects
#define RERR_SETUP_NOT_COMPLETE MAKE_RERR(0x001A) // Returned by IReplStore::Initialize. Indicates the object cannot sync because further setup is needed

// use by IReplObjHandler
#define RERR_SKIP_ALL       MAKE_RERR( 0x0100 )  // skip all incoming packets because of write errors
#define RERR_BAD_OBJECT     MAKE_RERR( 0x0101 )  // this is a bad object because of read error, server should not try to replicate it again
#define RERR_TRY_AGAIN      MAKE_RERR( 0x0102 )  // this is a bad object because of read error, server should can try to replicate it again later
#define RERR_USER_SKIP      MAKE_RERR( 0x0103 )  // object skipped by the user

// Used by WCESMgr
#define ERROR_NOT_INITIALIZED        MAKE_RERR( 0x0500 ) // Object is used without being initialized #define ERROR_INVALID_RESOURCE      MAKE_RERR( 0x0501 ) // Attempeted to load invalid resource
#define ERROR_INVALID_ID             MAKE_RERR( 0x0502 ) // ID/GUID is invalid
#define ERROR_ADD_FAILED             MAKE_RERR( 0x0503 ) // Add failed for example of DataSource, Engine or SSP
#define ERROR_DELETE_FAILED          MAKE_RERR( 0x0504 ) // Delete Failed 
#define ERROR_ARRAY_OPERATION_FAILED MAKE_RERR( 0x0505 ) // When enumerate, sort or filter fails 
#define ERROR_LOCK_ALREADY_HELD      MAKE_RERR( 0x0506 ) // When a thread synching object attempts to acquire a locka nd it is aleady held
#define ERROR_NODE_HAS_NO_CHILD      MAKE_RERR( 0x0507 ) // The XML Node has no children
#define ERROR_INVALID_GUID           MAKE_RERR( 0x0508 ) // Invalid GUID
#define ERROR_SAVE_FAILED            MAKE_RERR( 0x0509 ) // Save failed for example of DataSource, Engine or SSP
#define ERROR_CREATION_FAILED        MAKE_RERR( 0x0510 ) // Creation failed

// these are warning codes
#define RWRN_LAST_PACKET             MAKE_RWRN( 0x0001 )
#define RWRN_DIRECTORY_EXISTS        MAKE_RWRN( 0x0002 ) // Directory already exists

// flags used in RSC_BEGIN_SYNC
#define BSF_AUTO_SYNC           ((UINT)0x00000001)  // This flag is being obsoleted in ActiveSync 3.0
#define BSF_REMOTE_SYNC         ((UINT)0x00000002)  // consistent with RSC_REMOTE_SYNC, set if we are sync'ing remotely
#define BSF_SHOW_FATAL_ERRORS   ((UINT)0x00000004)  // If an error occurs which prevents the SSP from synchronizing at all, it can show the error to the user when this flag is set
#define BSF_SHOW_RESOLVE_ERRORS ((UINT)0x00000008)  // The SSP can show any error messages that it comes across as during the resolve items phase
#define BSF_RESERVED            ((UINT)0x80000000)  // Reserved by ActiveSync server.

// Code for ReportStatus
#define RSC_BEGIN_SYNC          ((UINT)1)   // Synchronization is about to start, uReserved is combination of bit flags, see BSF_* above
#define RSC_END_SYNC            ((UINT)2)   // Synchronization is about to end
#define RSC_BEGIN_CHECK         ((UINT)3)   // FindFirstItem is about to be called, followed by FindNextItem
#define RSC_END_CHECK           ((UINT)4)   // FindItemClose has been called
#define RSC_DATE_CHANGED        ((UINT)5)   // System Date has changed, this is called for each known desktop object, unless when both hFolder & hItem are NULL
#define RSC_RELEASE             ((UINT)6)   // Replication is about to release the store
#define RSC_REMOTE_SYNC         ((UINT)7)   // Indicates if remote sync is enabled. uParam will TRUE if all sync
                                            // will be remote until this status is reported again with uParam set to FALSE
#define RSC_INTERRUPT           ((UINT)8)   // interrupt current operation
#define RSC_BEGIN_SYNC_OBJ      ((UINT)9)   // Synchronization is about to start on an object type. uReserved points to
#define RSC_END_SYNC_OBJ        ((UINT)10)  // Synchronization is about to end on an object type.
#define RSC_OBJ_TYPE_ENABLED    ((UINT)11)  // Synchronization of the given object is enabled, hFolder is indeed a pointer to a string (object type name)
#define RSC_OBJ_TYPE_DISABLED   ((UINT)12)  // Synchronization of the given object is disabled, hFolder is indeed a pointer to a string (object type name)
#define RSC_BEGIN_BATCH_WRITE   ((UINT)13)  // A series of SetPacket will be called on a number of objects, this is the right time for some service providers to start a transaction
#define RSC_END_BATCH_WRITE     ((UINT)14)  // above write ends, this is the right time for some service providers to commit the transaction
#define RSC_CONNECTION_CHG      ((UINT)15)  // connection status has changed. uParam is TRUE if connection established. FALSE otherwise.
#define RSC_WRITE_OBJ_FAILED    ((UINT)16)  // failed writing an object on the device. uParam is the HRESULT code.
#define RSC_DELETE_OBJ_FAILED   ((UINT)17)  // failed deleting an object on the device. uParam is the HRESULT code.
#define RSC_WRITE_OBJ_SUCCESS   ((UINT)18)  // writing of an object succeeded on the device. uParam is a pointer to SDREQUEST (with (lpbData, cbData) representing the volume ID)
#define RSC_DELETE_OBJ_SUCCESS  ((UINT)19)  // deletion of an object succeeded on the device. uParam is a pointer to SDREQUEST (with (lpbData, cbData) representing the volume ID)
#define RSC_READ_OBJ_FAILED     ((UINT)20)  // failed to read an object from the device. uParam is the HRESULT code
#define RSC_TIME_CHANGED        ((UINT)21)  // System time has changed, this is called only once.

#define RSC_BEGIN_BACKUP        ((UINT)22)  // Backup is about to start.
#define RSC_END_BACKUP          ((UINT)23)  // Backup has ended.
#define RSC_BEGIN_RESTORE       ((UINT)24)  // Restore is about to start.

#define RSC_PREPARE_SYNC_FLD    ((UINT)26)  // Prepare to sync one specific folder whether or not any objects of the type are dirty. hFolder is a pointer to the object name that will be synced. 

//
//========================= IReplNotify ==============================
//

typedef struct tagDevInfo
{
    DWORD   pid;                // device ID
    char    szName[ MAX_PATH ]; // device name
    char    szType[ 80 ];       // device type
    char    szPath[ MAX_PATH ]; // device path
} DEVINFO, *PDEVINFO;

// a structure used to get/set custom sync. data from/to the device
typedef struct SDREQUEST
{
#ifdef UNDER_CE
    OBJTYPENAME szObjType;  // the object type where this data is coming from
#else
    OBJTYPENAMEA szObjType;  // the object type where this data is coming from
#endif
    BOOL        fSet;       // TRUE if sending data down and FALSE if getting data up
    UINT        uCode;      // for getting data from the device, this code must be less than 8
    LPBYTE      lpbData;
    UINT        cbData;
} SDREQUEST, *PSDREQUEST;

// code for QueryDevice
#define QDC_SEL_DEVICE      1   // Selected device info, *ppvData points to DEVINFO
#define QDC_CON_DEVICE      2   // Connected device info, *ppvData points to DEVINFO
#define QDC_SEL_DEVICE_KEY  3   // get a registry key that can be used to store selected device specific settings.
                                // *ppvData points to HKEY, caller must close reg key when its usage is over
#define QDC_CON_DEVICE_KEY  4   // get a registry key that can be used to store connnected device specific settings.
                                // *ppvData points to HKEY, caller must close reg key when its usage is over
#define QDC_SYNC_DATA   5       // get or set custom sync data from the device, *ppvData points to SDREQUEST

#define INF_OVERRIDE        ((UINT)0x0001000)   // used for OnItemNotify, override the default action of "delete wins over change"

#undef  INTERFACE
#define INTERFACE   IReplNotify
DECLARE_INTERFACE_( IReplNotify, IUnknown )
{
#ifndef UNDER_CE
    STDMETHOD(       SetStatusText)     ( THIS_ LPCSTR lpszText ) PURE;  // lpszText can have special syntax, see programmer's guide
    STDMETHOD_(HWND, GetWindow)         ( THIS_ UINT uFlags ) PURE;
    STDMETHOD(       OnItemNotify )     ( THIS_ UINT uCode, LPCSTR lpszProgId, LPCSTR lpszName, HREPLITEM hItem, ULONG ulFlags ) PURE;
    STDMETHOD(       QueryDevice )      ( THIS_ UINT uCode, LPVOID *ppvData ) PURE;
#endif

    // Internal use only
    STDMETHOD(       OnItemCompleted )  ( THIS_ PREPLSETUP pSetup ) PURE;
};

#define RNC_CREATED     1
#define RNC_MODIFIED    2
#define RNC_DELETED     3
#define RNC_SHUTDOWN    4
#define RNC_IDLE        5

#ifndef UNDER_CE

#define SCF_SINGLE_THREAD   ((UINT)0x00000001)  // set if the implementation only supports single thread operation.
#define SCF_SIMULATE_RTS    ((UINT)0x00000002)  // set if the implementation wants to simulate detection of real-time change/deletes

typedef struct tagStoreInfo
{
    UINT    cbStruct;               // Size of this structure
    UINT    uFlags;                 // Miscelleanous flags on the store, see SCF_xxx above
#ifdef UNDER_CE
    TCHAR   szProgId[ 256 ];        // Output, ProgID name of the store object
    TCHAR   szStoreDesc[ 200 ];     // Output, description of the store, will be displayed to the user
#else
    CHAR   szProgId[ 256 ];        // Output, ProgID name of the store object
    CHAR   szStoreDesc[ 200 ];     // Output, description of the store, will be displayed to the user
#endif
    UINT    uTimerRes;              // Input/Output, resolution of timer in micro-seconds. 5000 by default.

    UINT    cbMaxStoreId;           // Input, max. size of the store ID that can be stored in buffer pointed by lpbStoreId.
    UINT    cbStoreId;              // Output, actual size of the store ID stored in buffer pointed by lpbStoreId
    LPBYTE  lpbStoreId;             // Output pointer to a buffer of anything that uniquely
                                    // identifies the current store instance (Eg. a schedule file)
} STOREINFO, *PSTOREINFO;

typedef struct tagObjUIData
{
    UINT        cbStruct;               // size of this structure
    HICON       hIconLarge;             // Handle of a large icon used in the list view display in Synchronization Status
    HICON       hIconSmall;             // Handle of a small icon used in the list view display in Synchronization Status
    char        szName[ MAX_PATH ];     // Text displayed in the "Name" column in Synchronization Status
    char        szSyncText[ MAX_PATH ]; // Text displayed in the  "Sync Copy In" column in Synchronization Status
    char        szTypeText[ 80 ];       // Text displayed in the  "Type" column in Synchronization Status
    char        szPlTypeText[ 80 ];     // Plural form of text displayed in the  "Type" column in Synchronization Status
} OBJUIDATA, *POBJUIDATA;

enum ReplDialogs
{
    OPTIONS_DIALOG
};

//
//========================= IEnumReplItem ==============================
//
DEFINE_GUID( IID_IEnumReplItem,              /* a417bc0e-7be1-11ce-ad82-00aa006ec559 */
    0xa417bc0e,
    0x7be1,
    0x11ce,
    0xad, 0x82, 0x00, 0xaa, 0x00, 0x6e, 0xc5, 0x59
);

#undef  INTERFACE
#define INTERFACE   IEnumReplItem
DECLARE_INTERFACE_( IEnumReplItem, IUnknown )
{
    STDMETHOD(Next)                         ( THIS_ ULONG celt, HREPLITEM *phItem, ULONG *pceltFetched ) PURE;
    STDMETHOD(Skip)                         ( THIS_ ULONG celt ) PURE;
    STDMETHOD(Reset)                        ( THIS ) PURE;
    STDMETHOD(Clone)                        ( THIS_ IEnumReplItem **ppenum ) PURE;
    STDMETHOD_( HREPLFLD, GetFolderHandle)  ( THIS ) PURE;
};

typedef struct tagConfInfo
{
    UINT        cbStruct;
    HREPLFLD    hFolder;
    HREPLITEM   hLocalItem;
    HREPLITEM   hRemoteItem;

#ifdef UNDER_CE
    OBJTYPENAME szLocalName;
    TCHAR       szLocalDesc[ 512 ];

    OBJTYPENAME szRemoteName;
    TCHAR       szRemoteDesc[ 512 ];
#else
    OBJTYPENAMEA szLocalName;
    CHAR         szLocalDesc[ 512 ];

    OBJTYPENAMEA szRemoteName;
    CHAR         szRemoteDesc[ 512 ];
#endif
} CONFINFO, *PCONFINFO;

// flags for uParam of IReplStore::ReportStatus
#define PSA_RESET_INTERRUPT ((UINT)0x00000001)  // this flag is set if we're clearing the interrupt state (ie. we go back to normal operation)
#define PSA_SYS_SHUTDOWN    ((UINT)0x00000002)  // Windows is shutting down

// Actions for Setup
#define RSTP_SETUP          ((WORD)0x0001)  // New setup
#define RSTP_CREATE         ((WORD)0x0002)  // New profile
#define RSTP_RENAME         ((WORD)0x0003)  // Rename profile
#define RSTP_DELETE         ((WORD)0x0004)  // Delete profile

//========================= IReplSetup ==============================
//

DEFINE_GUID( IID_IReplSetup, /* 60178ec0-c670-11d0-837a-0000f80220b9 */
    0x60178ec0,
    0xc670,
    0x11d0,
    0x83, 0x7a, 0x00, 0x00, 0xf8, 0x02, 0x20, 0xb9
);

#undef  INTERFACE
#define INTERFACE   IReplSetup

//
//  IReplSetup is included but is obsolete
//
DECLARE_INTERFACE_( IReplSetup, IUnknown )
{
    // *** IReplSetup methods ***
    STDMETHOD(          Setup )             ( THIS_ HWND hwndParent, DWORD dwDeviceId, WORD wAction ) PURE;
};

//
//========================= IReplStore ==============================
//
DEFINE_GUID (IID_IReplStore,            // a417bc0f-7be1-11ce-ad82-00aa006ec559
    0xa417bc0f,
    0x7be1,
    0x11ce,
    0xad, 0x82, 0x00, 0xaa, 0x00, 0x6e, 0xc5, 0x59
);

// Flags for Initialize
#define ISF_SELECTED_DEVICE     ((UINT)0x00000001)  // set if the store is initialized for selected device
                                                    // otherwise it's initialized for connected device
#define ISF_REMOTE_CONNECTED    ((UINT)0x00000002)  // set if the store is initialized during remote connection, all UI should be suppressed.

#undef  INTERFACE
#define INTERFACE   IReplStore
DECLARE_INTERFACE_( IReplStore, IUnknown )
{
    // *** IReplStore methods ***
    STDMETHOD(          Initialize )        ( THIS_ IReplNotify *pNotify, UINT uFlags ) PURE;
    STDMETHOD(          GetStoreInfo )      ( THIS_ PSTOREINFO pStoreInfo ) PURE;
    STDMETHOD(          ReportStatus )      ( THIS_ HREPLFLD hFld, HREPLITEM hItem, UINT uStatus, UINT uParam ) PURE;
    STDMETHOD_( int,    CompareStoreIDs)    ( THIS_ LPBYTE, UINT, LPBYTE, UINT ) PURE;

    // Item related routines
    STDMETHOD_( int,    CompareItem )       ( THIS_ HREPLITEM hItem1, HREPLITEM hItem2 ) PURE;
    STDMETHOD_( BOOL,   IsItemChanged)      ( THIS_ HREPLFLD hFld, HREPLITEM hItem, HREPLITEM hItemComp ) PURE;
    STDMETHOD_( BOOL,   IsItemReplicated )  ( THIS_ HREPLFLD hFld, HREPLITEM hItem ) PURE;
    STDMETHOD_( void,   UpdateItem )        ( THIS_ HREPLFLD hFld, HREPLITEM hItemDst, HREPLITEM hItemSrc ) PURE;

    // Folder related routines
    STDMETHOD(          GetFolderInfo )     ( THIS_ LPCSTR lpszObjType, HREPLFLD *phFld, IUnknown ** ) PURE;
    STDMETHOD(          IsFolderChanged )   ( THIS_ HREPLFLD hFld, BOOL *pfChanged ) PURE;

    // Enumeration of folders
    STDMETHOD(          FindFirstItem )     ( THIS_ HREPLFLD hFld,  HREPLITEM *phItem, BOOL *pfExist ) PURE;   // get first object the folder
    STDMETHOD(          FindNextItem )      ( THIS_ HREPLFLD hFld,  HREPLITEM *phItem, BOOL *pfExist ) PURE;   // get next object the folder
    STDMETHOD(          FindItemClose )     ( THIS_ HREPLFLD hFld ) PURE;                   // done enumerating

    // Object management routines
    STDMETHOD_(UINT,    ObjectToBytes )     ( THIS_ HREPLOBJ hObject, LPBYTE lpb ) PURE;
    STDMETHOD_(HREPLOBJ,BytesToObject )     ( THIS_ LPBYTE lpb, UINT cb ) PURE;
    STDMETHOD_(void,    FreeObject )        ( THIS_ HREPLOBJ hObject ) PURE;
    STDMETHOD_(BOOL,    CopyObject )        ( THIS_ HREPLOBJ hObjSrc, HREPLOBJ hObjDest ) PURE;
    STDMETHOD(          IsValidObject )     ( THIS_ HREPLFLD hFld, HREPLITEM hObject, UINT uFlags ) PURE;

    // UI related routines
    STDMETHOD(          ActivateDialog)     ( THIS_ UINT uidDialog, HWND hwndParent, HREPLFLD hFld, IEnumReplItem *penumItem ) PURE;
    STDMETHOD(          GetObjTypeUIData)   ( THIS_ HREPLFLD hFld, POBJUIDATA pData ) PURE;
    STDMETHOD(          GetConflictInfo )   ( THIS_ PCONFINFO pConfInfo ) PURE;
    STDMETHOD(          RemoveDuplicates )  ( THIS_ LPCSTR lpszObjType, UINT uFlags ) PURE;
};

//
//=========== Function prototypes for Enable/Disable Callbacks  ==================
//
typedef HRESULT (WINAPI * PFNONSSPENABLE)(HWND);    //For exported function "OnSSPEnable"
typedef HRESULT (WINAPI * PFNONSSPDISABLE)(HWND);   //For exported function "OnSSPDisable"


//
//========================= IReplStore2 ==========================================
//
DEFINE_GUID (IID_IReplStore2,            // 4a696d00-f75f-11d2-b549-00c04f5b9a11
    0x4a696d00,
    0xf75f,
    0x11d2,
    0xb5, 0x49, 0x00, 0xc0, 0x4f, 0x5b, 0x9a, 0x11
);

#undef  INTERFACE
#define INTERFACE   IReplStore2
DECLARE_INTERFACE_( IReplStore2, IReplStore )
{
    // *** IReplStore2 methods ***

    // UI related routines
    STDMETHOD(DoOptionsDialog)     ( THIS_ HWND hwndParent, LPCSTR lpszObjType, IReplNotify * pntfy) PURE;
};


#endif

//
//=========== Section for object serializing & deserializing interfaces ==========
//
#define RSF_CONFLICT_OBJECT             0x00000001  // this is about getting/writting a conflicting object
#define RSF_NEW_OBJECT                  0x00000002  // this is a new object to be written
#define RSF_DUPLICATED_OBJECT           0x00000004  // the object is an exact duplicate of an existing object
#define RSF_COMBINE                     0x00000008  // the object is being writen to desktop during a combine operation
#define RSF_SYNC_DEVICE_ONLY            0x00000010  // the object should be sync'ed from device to desktop only
#define RSF_SYNC_DESKTOP_ONLY           0x00000020  // the object should be sync'ed from desktop to device only
#define RSF_UPDATED_HANDLE              0x00000040  // this is a new object, but the oid already exists (eg, file rename)
#define RSF_DISCARDED_OBJ               0x00000080  // used in DeleteObj. indicates the object is deleted as a result of RERR_DISCARD being returned by SetPacket
#define RSF_NEW_VOLUME                  0x00000100  // used by ActiveSync manager only.
#define RSF_AUTO_COMBINE                0x00000200  // the object is being written to the desktop, similar to RSF_COMBINE except there were no items on the desktop to combine with

#define RSF_RESERVED1                   0x00100000  // reserved by ActiveSync manager: DO NOT USE THESE
#define RSF_RESERVED2                   0x00200000
#define RSF_RESERVED3                   0x00400000
#define RSF_RESERVED4                   0x00800000  

typedef struct _tagReplSetup
{
    UINT        cbStruct;
    BOOL        fRead;
    DWORD       dwFlags;                // see RSF_xxx above.
    HRESULT     hr;
#ifdef UNDER_CE
    OBJTYPENAME szObjType;
#else
    OBJTYPENAMEA szObjType;
#endif
    IReplNotify *pNotify;

    DWORD       oid;
    DWORD       oidNew;

#ifndef UNDER_CE
    IReplStore  *pStore;

    HREPLFLD    hFolder;
    HREPLITEM   hItem;
#endif

    LPBYTE  lpbVolumeID;    // ID of the volume for the object. NULL if the object is in the default volume
    UINT    cbVolumeID;     // size of above ID in bytes
} REPLSETUP, *PREPLSETUP;


//========================= IReplObjHandler ==============================
//
// Specifies the interface for replication object handler
// (object serializer/deserializer)
#undef  INTERFACE
#define INTERFACE   IReplObjHandler
DECLARE_INTERFACE_( IReplObjHandler, IUnknown )
{
    //  Called everytime when an object is about to be serialized/deserialized
    STDMETHOD( Setup )  ( THIS_ PREPLSETUP pSetup ) PURE;

    //  Called everytime when it's the time to clean up the serializer/deserializer for the object
    STDMETHOD( Reset ) ( THIS_ PREPLSETUP pSetup ) PURE;

    /* A request to get a data packet (serialize the object)
    handler should pass back the buffer along with the size bytes */
    STDMETHOD( GetPacket )(  THIS_ LPBYTE *lppbData,  DWORD *pcbData, DWORD cbRecommend ) PURE;

    /* A request to set a data packet (deserialize the byte stream) */
    STDMETHOD( SetPacket )( THIS_ LPBYTE lpbData, DWORD cbData ) PURE;

    /* A request to delete the given object */
    STDMETHOD( DeleteObj )( THIS_ PREPLSETUP pSetup ) PURE;
};

typedef struct tagObjTypeInfo
{
    UINT            cbStruct;       // Input. Size of the structure in bytes.
    OBJTYPENAMEW    szObjType;      // Input, the object type name
    UINT            uFlags;         // Reserved. Not in use yet.
    WCHAR           szName[ 80 ];   // Output, the name of a file system object storing all these object
    UINT            cObjects;       // Output, number of existing objects
    UINT            cbAllObj;       // Output, total number of bytes used to store existing objects
    FILETIME        ftLastModified; // Output, last time any object is modified
} OBJTYPEINFO, *POBJTYPEINFO;

#ifdef UNDER_CE

#define ONF_FILE            ((UINT)0x00000001)
#define ONF_DIRECTORY       ((UINT)0x00000002)
#define ONF_DATABASE        ((UINT)0x00000004)
#define ONF_RECORD          ((UINT)0x00000008)

#define ONF_CHANGED         ((UINT)0x00000010)  // set if the file system object is changed
#define ONF_DELETED         ((UINT)0x00000020)  // set if the file system object is deleted

#define ONF_CLEAR_CHANGE    ((UINT)0x00000040)  // client should clear the change bit for the object whose object id is pointed at by poid
#define ONF_CALL_BACK       ((UINT)0x00000080)  // Output, client asks server to call ObjectNotify 2 sec. later. (ObjectNotify is callback 
                                                // function, see definition of POBJNOTIFYPROC)
#define ONF_CALLING_BACK    ((UINT)0x00000100)  // set if this call is a result of ONF_CALL_BACK being set earlier

/*  Definitions of cOidChg, cOidDel and poid
    in all cases, poid points to a list of object id's

1) when ONF_CHANGED is set, cOidChg is the number of object id's in the list that should be synchronized. cOidDel is not used
2) when ONF_DELETED is set, cOidChg is not used, cOidDel is the number of deleted object id's in the list that should be synchronized
3) when both ONF_CHANGED & ONF_DELETED is not set,
    cOidChg is count of object id's in the first part of the list for objects that are changed
    cOidDel is count of object id's in the later part of the list for objects that are not changed

*/

typedef struct tagObjNotify
{
    UINT        cbStruct;       // Input. Size of the structure in bytes.
#ifdef UNDER_CE
    OBJTYPENAME szObjType;      // Input, the object type name
#else
    OBJTYPENAMEA szObjType;      // Input, the object type name
#endif
    UINT        uFlags;         // Input, Flags, see ONF_xxx above
    UINT        uPartnerBit;    // Input, which partner this
    CEOID       oidObject;      // Input. CEOID of the file system object changed/deleted
    CEOIDINFO   oidInfo;        // Input. Information about the file system object

    UINT        cOidChg;        // Output, see above comment for definition.
    UINT        cOidDel;        // Output, see above comment for definition.
    UINT        *poid;          // Output, see above comment for definition.
                                // Note that, memory pointed to by this pointer is owned by the clients.
                                // It will not be freed by replication.
    LPBYTE      lpbVolumeID;    // ID of the volume where all above objects lives. NULL if the objects are in RAM
    UINT        cbVolumeID;     // size of above ID in bytes
} OBJNOTIFY, *POBJNOTIFY;

#define FO_MORE_VOLUME      ((UINT)0x00000001)  // set by ActiveSync module. there are more volumes of objects
#define FO_DONE_ONE_VOL     ((UINT)0x00000002)  // set by ActiveSync manager, let ActiveSync module to free up the memory allocated in FINDOBJINFO

typedef struct tagFindObjInfo
{
    UINT        uFlags;         // See FO_* above
#ifdef UNDER_CE
    OBJTYPENAME szObjType;      // what object type we need to enumerate
#else
    OBJTYPENAMEA szObjType;      // what object type we need to enumerate
#endif

    UINT        *poid;          // points to list of object ID's,
                                // first part is for unchanged objects, last part is for changed objects

    UINT        cUnChg;         // # of unchanged object ID's in above list
    UINT        cChg;           // # of changed object ID's in above list

    LPBYTE      lpbVolumeID;    // ID of the volume where all above objects lives. NULL if the objects are in RAM
    UINT        cbVolumeID;     // size of above ID in bytes

    LPVOID      lpvUser;        // an ActiveSync module can save anything it wants in this variable
} FINDOBJINFO, *PFINDOBJINFO;

#ifdef __cplusplus
extern "C"{
#endif

// Functions exported by client's device module

// for Function: InitObjType
typedef BOOL (*PINITOBJPROC)( LPWSTR lpszObjType, IReplObjHandler **ppObjHandler, UINT uPartnerBit );

// for Function: ObjectNofity
typedef BOOL (*POBJNOTIFYPROC)( POBJNOTIFY );

// for Function: GetObjTypeInfo
typedef BOOL (*PGETOBJTYPEINFO)( POBJTYPEINFO );

// for Function: ReportStatus
typedef BOOL (*PREPORTSTATUS)( LPWSTR lpszObjType, UINT uCode, UINT uParam );

// for Function: FindObjects
typedef HRESULT (*PFINDOBJECTS)( PFINDOBJINFO );

// for Function: SyncData
typedef HRESULT (*PSYNCDATA )( PSDREQUEST psd );

#ifdef __cplusplus
}
#endif

#endif  // UNDER_CE

#define SZ_OUTSTORE_PROG_ID     TEXT( "MS.WinCE.OutLook" )
#define SZ_SCDSTORE_PROG_ID     TEXT( "MS.WinCE.SchedulePlus" )

#define SZ_APPT                 TEXT( "Appointment" )
#define SZ_CONTACT              TEXT( "Contact" )
#define SZ_TASK                 TEXT( "Task" )
#define SZ_FILE                 TEXT( "File" )
#define SZ_INBOX                TEXT( "Inbox" )
#define SZ_CHANNELS             TEXT( "Channel" )
#endif  // _INC_CESYNC_H
