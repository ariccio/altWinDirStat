// NTFSearch.cpp : Defines the entry point for the application.
//

/* BugFixes
	- limit Results // more or less done 0xfff0
	- extend searchstrings 
	- include sort // nearly done
	- include typeinformation // done
	- progressbar completion / fixed by cheating
*/



#include "stdafx.h"
#include "commdlg.h"
#include "NTFS-Search.h"
#include "commctrl.h"
#include "shellapi.h"
#include "NTFS_STRUCT.h"
#include "objidl.h"
#include "shlobj.h"
#include "shlwapi.h"
#include "SimplePattern.h"
#include "process.h"

#define MAX_LOADSTRING 100

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define PACKVERSION(major,minor) MAKELONG(minor,major)

#define FILES           0
#define FILENAMES       1
#define DELETENOW       1
#define DELETEONREBOOT 42

PHEAPBLOCK FileStrings;
PHEAPBLOCK PathStrings;

typedef struct SearchResult
{
	int    icon;
	LPTSTR extra;
	LPTSTR filename;
	LPTSTR path;

}*PSearchResult;

SearchResult results[0xffff];

BOOL         glHelp      = FALSE;

int          results_cnt = 0    ;
int          glSensitive = FALSE;


struct ThreadInfo
{
	HWND        hWnd;
	PDISKHANDLE disk;
};

// Global Variables:
HINSTANCE hInst;									// current instance
TCHAR     szTitle         [ MAX_LOADSTRING ];		// The title bar text
TCHAR     szWindowClass   [ MAX_LOADSTRING ];		// the main window class name
TCHAR     szLS            [ MAX_LOADSTRING ];
TCHAR     szNoS           [ MAX_LOADSTRING ];
TCHAR     szTotal         [ MAX_LOADSTRING ];
TCHAR     szFound         [ MAX_LOADSTRING ];
TCHAR     szSearch        [ MAX_LOADSTRING ];
TCHAR     szLoading       [ MAX_LOADSTRING ];
TCHAR     szResults       [ MAX_LOADSTRING ];
TCHAR     szSaveRes       [ MAX_LOADSTRING ];
TCHAR     szSaveResErr    [ MAX_LOADSTRING ];
TCHAR     szTooMany       [ MAX_LOADSTRING ];
TCHAR     szStrange       [ MAX_LOADSTRING ];
TCHAR     szAccessDenied  [ MAX_LOADSTRING ];
TCHAR     szDiskError     [ MAX_LOADSTRING ];
TCHAR     szWarning       [ MAX_LOADSTRING ];

TCHAR     szFNF           [ MAX_PATH       ];
TCHAR     szDelete        [ MAX_PATH       ];
TCHAR     szDeleteOnReboot[ MAX_PATH       ];
TCHAR     szDeletedFile   [ MAX_PATH       ];

TCHAR     szFiles[ ] = TEXT( "Textfile\0*.txt\0All files\0*.*\0\0" );

HIMAGELIST list ;
HIMAGELIST list2;
HIMAGELIST list3;

HMENU popup;
HMENU hm   ;

PDISKHANDLE disks[32];

VOID ReleaseAllDisks();

//Controls
HWND hEdit    ;
HWND hListView;
HWND hCheck1  ;
HWND hCheck2  ;
HWND hGroup   ;
HWND hStatus  ;
HWND hCombo   ;

HWND glDlg = NULL;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass ( HINSTANCE  hInstance                                                                                   );
BOOL				InitInstance    ( HINSTANCE,           int                                                                               );

LRESULT CALLBACK	WndProc         ( HWND,                UINT,               WPARAM,             LPARAM                                    );
INT_PTR CALLBACK	About           ( HWND,                UINT,               WPARAM,             LPARAM                                    );
INT_PTR CALLBACK	Help            ( HWND,                UINT,               WPARAM,             LPARAM                                    );
INT_PTR CALLBACK	Waiting         ( HWND,                UINT,               WPARAM,             LPARAM                                    );
LRESULT CALLBACK	SearchDlg       ( HWND hDlg,           UINT message,       WPARAM wParam,      LPARAM lParam                             );

void StartLoading                   ( PDISKHANDLE disk,    HWND hWnd                                                                         );
int ProcessLoading                  ( HWND hWnd,           HWND hCombo,        int reload                                                    );

DWORD WINAPI LoadSearchInfo         ( LPVOID lParam                                                                                          );

int SearchFiles                     ( HWND hWnd,           PDISKHANDLE disk,   TCHAR *filename,    BOOL deleted,   SEARCHP* pat              );
int Search                          ( HWND hWnd,           int disk,           TCHAR *filename,    BOOL deleted                              );

UINT ExecuteFile                    ( HWND hWnd,           LPWSTR str,         USHORT flags                                                  );
UINT ExecuteFileEx                  ( HWND hWnd,           LPTSTR command,     LPWSTR str,         LPCTSTR dir,    UINT show,   USHORT flags );

BOOL UnloadDisk                     ( HWND hWnd,           int index                                                                         );
BOOL SearchString                   ( LPWSTR pattern,      int length,         LPWSTR string,      int len                                   );

int filecompare                     ( const void *arg1,    const void *arg2                                                                  );
int pathcompare                     ( const void *arg1,    const void *arg2                                                                  );
int extcompare                      ( const void *arg1,    const void *arg2                                                                  );

BOOL ProcessPopupMenu               ( HWND hWnd,           int index,          DWORD item                                                    );
BOOL SaveResults                    ( LPWSTR filename                                                                                        );

void PrepareCopy                    ( HWND hWnd,           UINT flags                                                                        );
void DeleteFiles                    ( HWND hWnd,           UINT flags                                                                        );

DWORD GetDllVersion                 ( LPCTSTR lpszDllName                                                                                    );
DWORD ShowError                     (                                                                                                        );


int APIENTRY _tWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER ( hPrevInstance );
	UNREFERENCED_PARAMETER ( lpCmdLine     );

	// TODO: Place code here.
	MSG    msg;
	HACCEL hAccelTable;


	INITCOMMONCONTROLSEX   init  ;
	init.dwSize = sizeof(  init );
	init.dwICC  = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;//ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS |ICC_LISTVIEW_CLASSES;
	
	InitCommonControlsEx( &init );


	// Initialize global strings
	LoadString ( hInstance, IDS_ACCESSDENIED,   szAccessDenied,   MAX_LOADSTRING );
	LoadString ( hInstance, IDS_DELETE,         szDelete,         MAX_PATH       );
	LoadString ( hInstance, IDS_DELETEDFILE,    szDeletedFile,    MAX_PATH       );
	LoadString ( hInstance, IDS_DELETEONREBOOT, szDeleteOnReboot, MAX_PATH       );
	LoadString ( hInstance, IDS_DISKERROR,      szDiskError,      MAX_LOADSTRING );
	LoadString ( hInstance, IDS_FILENOTFOUND,   szFNF,            MAX_PATH       );
	LoadString ( hInstance, IDS_FOUND,          szFound,          MAX_LOADSTRING );
	LoadString ( hInstance, IDS_LOADING,        szLoading,        MAX_LOADSTRING );
	LoadString ( hInstance, IDS_LOADED,         szLS,             MAX_LOADSTRING );
	LoadString ( hInstance, IDS_UNSUPPORTED,    szNoS,            MAX_LOADSTRING );
	LoadString ( hInstance, IDS_RESULTS,        szResults,        MAX_LOADSTRING );
	LoadString ( hInstance, IDS_SAVERES,        szSaveRes,        MAX_LOADSTRING );
	LoadString ( hInstance, IDS_SAVERESERR,     szSaveResErr,     MAX_LOADSTRING );
	LoadString ( hInstance, IDS_SEARCHING,      szSearch,         MAX_LOADSTRING );
	LoadString ( hInstance, IDS_LOADSTRANGE,    szStrange,        MAX_LOADSTRING );
	LoadString ( hInstance, IDS_APP_TITLE,      szTitle,          MAX_LOADSTRING );
	LoadString ( hInstance, IDS_TOOMANY,        szTooMany,        MAX_LOADSTRING );
	LoadString ( hInstance, IDS_TOTAL,          szTotal,          MAX_LOADSTRING );
	LoadString ( hInstance, IDS_WARNING,        szWarning,        MAX_LOADSTRING );
	LoadString ( hInstance, IDC_NTFSEARCH,      szWindowClass,    MAX_LOADSTRING );



	//LoadString( hInstance, IDS_FILES,          szSaveRes,        MAX_LOADSTRING );


	MyRegisterClass ( hInstance );

	//SearchString(TEXT("*a*W*"),5, TEXT("Hallo Welt"),10);

	FileStrings = CreateHeap ( 0xffff * sizeof( SearchResult ) );
	PathStrings = CreateHeap ( 0xfff  * MAX_PATH               );

	// Perform application initialization:
	if ( !InitInstance ( hInstance, nCmdShow ) ) {
		return FALSE;
		}

	hAccelTable = LoadAccelerators ( hInstance, MAKEINTRESOURCE( IDC_NTFSEARCH ) );

	// Main message loop:
	while ( GetMessage( &msg, NULL, 0, 0 ) ) {
		//IsDialogMessage(msg.hwnd, &msg);
		if ( !TranslateAccelerator ( glDlg, hAccelTable, &msg ) ) {
			TranslateMessage ( &msg );
			DispatchMessage  ( &msg );
			}
		}
	FreeHeap ( PathStrings );
	FreeHeap ( FileStrings );

#ifdef TRACING
	TRACE( _T( "tWinMain done!\r\n" ) );
#endif
	//Beep(2500,150);
	return ( int ) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hbrBackground	= ( HBRUSH )      ( COLOR_3DFACE + 1                                 );
	wcex.hCursor		= LoadCursor      ( NULL,           IDC_ARROW                        );
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon        ( hInstance,      MAKEINTRESOURCE( IDI_NTFSEARCH ) );
	wcex.hIconSm		= LoadIcon        ( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL     ) );
	wcex.lpfnWndProc	= WndProc;
	wcex.lpszMenuName	= MAKEINTRESOURCE ( IDC_NTFSEARCH                                    );
	wcex.lpszClassName	= szWindowClass;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance ( HINSTANCE hInstance, int nCmdShow )
{
	HWND      hWnd;
	HICON     icon;
	HINSTANCE shell;

	hInst = hInstance; // Store instance handle in our global variable
	hWnd  = CreateWindow     ( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 0, 0, NULL, NULL, hInstance, NULL );

	if ( !hWnd ) {
		return FALSE;
		}

	shell = LoadLibrary      ( TEXT ( "shell32.dll" ) );

	list  = ImageList_Create (  GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), ILC_MASK | ILC_COLOR32, 8, 8 );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 8  ) );
	ImageList_AddIcon        (  list,  icon                  );
	DestroyIcon              (         icon                  );

	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 9  ) );
	ImageList_AddIcon        (  list,  icon                  );
	DestroyIcon              (         icon                  );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 10 ) );
	ImageList_AddIcon        (  list,  icon                  );
	DestroyIcon              (         icon                  );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 5  ) );
	ImageList_AddIcon        (  list,  icon                  );
	DestroyIcon              (         icon                  );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 11 ) );
	ImageList_AddIcon        (  list,  icon                  );
	DestroyIcon              (         icon                  );

	// List2
	list2 = ImageList_Create ( GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), ILC_MASK | ILC_COLOR32, 8, 8 );

	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 1  ) );
	ImageList_AddIcon        (  list2, icon                  );
	DestroyIcon              (         icon                  );

	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 2  ) );
	ImageList_AddIcon        (  list2, icon                  );
	DestroyIcon              (         icon                  );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 42 ) );
	ImageList_AddIcon        (  list2, icon                  );
	DestroyIcon              (         icon                  );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 5  ) );
	ImageList_AddIcon        (  list2, icon                  );
	DestroyIcon              (         icon                  );
	
	icon  = LoadIcon         (  shell, MAKEINTRESOURCE( 10 ) );
	ImageList_AddIcon        (  list2, icon                  );
	DestroyIcon              (         icon                  );

	FreeLibrary              ( shell );
	popup = LoadMenu         ( hInst, ( LPWSTR ) IDR_MENU1 );

	hm = GetSubMenu          ( popup, 0 );
	SetMenuDefaultItem       ( hm, 0, TRUE );

	//CreateDialogParam(hInst, (LPCTSTR)IDD_SEARCH, NULL, (DLGPROC)SearchDlg,0);
	DialogBox                ( hInst, MAKEINTRESOURCE( IDD_SEARCH ), NULL, ( DLGPROC ) SearchDlg );
	DestroyWindow            ( hWnd );

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int         wmId;
	int         wmEvent;
	PAINTSTRUCT ps;
	HDC         hdc;

	switch (message)
	{
		case WM_COMMAND:
			wmId    = LOWORD( wParam );
			wmEvent = HIWORD( wParam );
			// Parse the menu selections:
			switch ( wmId )
			{
				case IDM_ABOUT:
					DialogBox           ( hInst, MAKEINTRESOURCE( IDD_ABOUTBOX ), hWnd,   About  );
					break;
				case IDM_EXIT:
					DestroyWindow       ( hWnd                                                   );
					break;
				case EN_CHANGE:
					break;
				default:
					return DefWindowProc( hWnd,  message,                         wParam, lParam );
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint( hWnd, &ps );
			EndPaint        ( hWnd, &ps );
			break;
		case WM_DESTROY:
			ImageList_Destroy( list  );
			ImageList_Destroy( list2 );
			DestroyMenu      ( popup );
			ReleaseAllDisks  (       );
			PostQuitMessage  (   0   );
			break;
		default:
			return DefWindowProc        ( hWnd,  message,                         wParam, lParam );
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER( lParam );
	switch ( message )
	{
		case WM_INITDIALOG:
			return     ( INT_PTR ) TRUE;
		case WM_COMMAND:
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
				EndDialog( hDlg, LOWORD( wParam ) );
				return ( INT_PTR ) TRUE;
				}
			break;
	}
	return             ( INT_PTR ) FALSE;
}

// Help handler
INT_PTR CALLBACK Help(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER( lParam );
	switch ( message )
	{
		case WM_INITDIALOG:
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( "Help - HELP - Help"         ) );
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( "You can use * as a wildcard") );
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( "*AnyText* - *.*"            ) );
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( "*AnyText - *.html"          ) );
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( "AnyText* - Data.*"          ) );
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( "Any*Text - Use*Data"        ) );
			SendDlgItemMessage ( hDlg, IDC_LIST1, LB_ADDSTRING, 0, ( LPARAM ) TEXT( ""                           ) );
			glHelp = TRUE;
			return     ( INT_PTR ) TRUE;
		case WM_MEASUREITEM:
			{
				LPMEASUREITEMSTRUCT item;
				item             = ( LPMEASUREITEMSTRUCT ) lParam;
				item->itemHeight = 50;
			}
			break;
		case WM_DRAWITEM:
			{
				LPDRAWITEMSTRUCT item;
				item = ( LPDRAWITEMSTRUCT ) lParam;
				if ( item->itemAction & ODA_DRAWENTIRE ) {
					DrawText( item->hDC, ( TCHAR* ) item->itemData, wcslen( ( TCHAR* ) item->itemData ), &item->rcItem, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
					}
			}	
			break;
		case WM_COMMAND:
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
				EndDialog( hDlg, LOWORD( wParam ) );
				return ( INT_PTR ) TRUE;
				}
			break;
		case WM_DESTROY:
			glHelp = FALSE;
			break;
	}
	return             ( INT_PTR ) FALSE;
}


LRESULT CALLBACK SearchDlg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	/*
	  THIS FUNCTION IS GODDAMNED HUGE! BUGBUG
	*/
	int      id;
	int      msg;
	int      index;
	int      b;
	TCHAR    data;
	DWORD    loaded;
	LONG_PTR pt;
			
	static TCHAR tmp[MAX_PATH];
	static TCHAR path[0xffff];
	
	COMBOBOXEXITEM item;

	switch( message )
	{
		case WM_INITDIALOG:
		{
#ifdef TRACING
			TRACE( _T( "Initializing search dialog...\r\n" ) );
#endif
			TCHAR buf[ MAX_PATH ];
			hListView = GetDlgItem ( hWnd, IDC_RESULT );
			hCombo    = GetDlgItem ( hWnd, IDC_DRIVES );
			if ( GetDllVersion( TEXT( "comctl32.dll" ) ) >= PACKVERSION( 6, 0 ) ) {
				//Proceed.
				ListView_SetExtendedListViewStyle( hListView, LVS_EX_DOUBLEBUFFER );
				hStatus = CreateWindowEx( WS_EX_COMPOSITED, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 20, 20, 100, 100, hWnd, NULL, hInst, NULL );
				}
			else {
				// Use an alternate approach for older DLL versions.
				hStatus = CreateWindowEx( 0,                STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 20, 20, 100, 100, hWnd, NULL, hInst, NULL );
				}

			LVCOLUMN col;
			col.mask     = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;// | LVCF_IMAGE | LVCF_FMT;
			col.fmt      = HDF_BITMAP_ON_RIGHT;
			LoadString ( hInst, IDS_FILENAME, &buf[ 0 ], MAX_PATH );
			col.pszText  = buf;
			//col.iImage = 5;
			col.cx       = 220;
			col.iSubItem = 0;

			ListView_InsertColumn( hListView, 0, &col );
			LoadString ( hInst, IDS_PATH,     &buf[ 0 ], MAX_PATH );
			
			col.pszText  = buf;
			col.cx       = 320;
			col.iSubItem = 2;

			ListView_InsertColumn( hListView, 2, &col );
			LoadString ( hInst, IDS_EXT,      &buf[ 0 ], MAX_PATH );
			
			col.pszText  = buf;
			col.cx       = 60;
			col.iSubItem = 1;

			ListView_InsertColumn( hListView, 1, &col );
			ListView_SetImageList( hListView, list2, LVSIL_SMALL );
			
			DWORD drives;
			drives = GetLogicalDrives();

			memset( &item, 0, sizeof( item ) );
			
			item.iItem          = -1;
			item.mask           = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_LPARAM |CBEIF_SELECTEDIMAGE;
			item.iImage         = 0;
			item.iSelectedImage = 1;
			SendMessage             ( hCombo, CBEM_SETIMAGELIST, 0, ( LPARAM ) list  );
	
			LoadString ( hInst, IDS_USELOADED,  &buf[0], MAX_PATH);
			
			item.pszText = buf;
			item.lParam  = 0xff;
			SendMessage             ( hCombo, CBEM_INSERTITEM,   0, ( LPARAM ) &item );
			for ( int i = 0; i < 32; i++ ) {
				if ( ( drives >> ( i ) ) & 0x1 ) {
					TCHAR str[ 5 ];
					UINT type;
					wsprintf( str, TEXT( "%C:\\" ), 0x41 + i );
					item.pszText = str;
					item.lParam  = i;
					type         = GetDriveType(str);
					if ( type == DRIVE_FIXED ) {
						SendMessage ( hCombo, CBEM_INSERTITEM,   0, ( LPARAM ) &item );
						}
					}
				}
			LoadString ( hInst, IDS_LOADALL, &buf[ 0 ],  MAX_PATH );
			
			item.pszText = buf;
			item.lParam  = 0xfe;
			
			SendMessage (             hCombo,  CBEM_INSERTITEM, 0, ( LPARAM ) &item );
			SendMessage (             hCombo,  CB_SETCURSEL,    0, 0                );
			
			int parts[ 2 ] = { 250, -1 };
			
			SendMessage (             hStatus, SB_SETPARTS,     2, ( LPARAM ) &parts );
			return TRUE;
		}
		case WM_SIZE:
		{
			int width ;
			int height;
			
			RECT rt ;
			RECT rt2;
			
			width  = LOWORD( lParam );
			height = HIWORD( lParam );
			
#ifdef TRACING
			TRACE( _T( "Resizing window...\r\n" ) );
#endif

			MoveWindow( hStatus,   0, height,              width, 20,                                                    TRUE );
			GetWindowRect( hStatus, &rt );
			GetWindowRect( hCombo, &rt2 );
			
			rt2.bottom += 12;
			MapWindowPoints( NULL, hWnd, ( LPPOINT ) &rt2, 2 );
			MoveWindow( hListView, 0, ( rt2.bottom ) + 20, width, height - ( rt.bottom - rt.top ) - ( rt2.bottom + 20 ), TRUE );
			break;
		}
		case WM_NOTIFY:
			LPNMHDR hdr;
			LPNMLISTVIEW listitem;
			hdr = ( LPNMHDR ) lParam;
			if ( hdr->hwndFrom == hListView ) {
				//int len;
				switch( hdr->code )
				{
					case NM_DBLCLK:
						index = SendMessage( hdr->hwndFrom, LVM_GETSELECTIONMARK, 0, 0 );
						if ( index >= 0 ) {
							ProcessPopupMenu( hWnd, index, IDM_OPEN );
							}
						break;
					case NM_RCLICK:
						break;
					case LVN_DELETEALLITEMS:
						pt = TRUE;
						SetLastError( 0 );
						SetWindowLong( hWnd, DWL_MSGRESULT, pt );
						return TRUE;
					case LVN_DELETEITEM:
						Beep( 2500, 5 );
						break;
					case NM_RETURN:
						Beep( 2500, 50 );
						break;
					case LVN_ODFINDITEM:
						NMLVFINDITEM *finditem;
						TCHAR stmp [ 2 ];
						TCHAR stmp2[ 2 ];
						finditem = (NMLVFINDITEM*) lParam;
						if ( finditem->lvfi.flags & LVFI_STRING ) {
							stmp   [ 0 ]  = finditem->lvfi.psz[ 0 ];
							stmp   [ 1 ]  = 0;
							stmp2  [ 1 ]  = 0;
							CharLower( stmp );
							int j = 0;
							for ( int i = finditem->iStart; i != finditem->iStart - 1; i++ ) {
								if ( j >= results_cnt ) {
									break;
									}
								if ( i >= results_cnt ) {
									i = 0;
									}
								stmp2[ 0 ] = results[ i ].filename[ 0 ];
								CharLower( stmp2 );
								if ( stmp[ 0 ] == stmp2[ 0 ] ) {
									pt = i;
									SetWindowLong ( hWnd, DWL_MSGRESULT, pt );
									return TRUE;
									}
								j++;
								}
							}
						pt = -1;
						SetWindowLong             ( hWnd, DWL_MSGRESULT, pt );
						return TRUE;		
						break;
					case LVN_GETDISPINFO:
						NMLVDISPINFO* info;
						info = ( NMLVDISPINFO* ) lParam;
						SearchResult* res;
						if ( info->item.iItem > -1 && info->item.iItem < results_cnt ) {
							res = &results[ info->item.iItem ];//(SearchResult*) &SearchResults->data[info->item.iItem*sizeof(SearchResult)];
							if ( info->item.mask & LVIF_TEXT ) {
								if (      info->item.iSubItem == 0 ) {
									info->item.pszText = ( LPTSTR ) res->filename;
									}
								else if ( info->item.iSubItem == 1 ) {
									info->item.pszText = ( LPTSTR ) res->extra;
									}
								else {
									info->item.pszText = ( LPTSTR ) res->path;
									}
								}
							if ( info->item.mask & LVIF_IMAGE ) {
								    info->item.iImage = res->icon;
								}
							}
						break;
					case LVN_BEGINDRAG:
						listitem = ( LPNMLISTVIEW ) lParam;
						break;
					case LVN_COLUMNCLICK:
						listitem = (LPNMLISTVIEW) lParam;
						if ( listitem->iSubItem == 0 ) {
							qsort ( ( void* ) &results[ 0 ], results_cnt, sizeof( SearchResult ), filecompare );//AHH WHY VOID* BUGBUG
							}
						else if ( listitem->iSubItem == 1 ) {
							qsort ( ( void* ) &results[ 0 ], results_cnt, sizeof( SearchResult ), extcompare );//AHH WHY VOID* BUGBUG
							}
						else if ( listitem->iSubItem == 2 ) {
							qsort ( ( void* ) &results[ 0 ], results_cnt, sizeof( SearchResult ), pathcompare );//AHH WHY VOID* BUGBUG
							}
						InvalidateRect( listitem->hdr.hwndFrom, NULL, TRUE );
						break;
				}	
			}
			break;
		case WM_USER+2:
#ifdef TRACING
			TRACE( _T( "SearchDlg handling custom event...\r\n" ) );
#endif
			int count;
			count = SendMessage         ( hCombo, CB_GETCOUNT,    0, 0 );
			memset( &item, 0, sizeof( item ) );
			item.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
			item.iImage                         = 2;
			item.iSelectedImage                 = 2;
			loaded              = 0;
			for ( int i = 0; i<count; i++ ) {//CANNOT be vectorized - assignments are of different sizes
				data = SendMessage      ( hCombo,    CB_GETITEMDATA, i, 0 );
				if (data > 0 && data < 32) {
					if ( disks[ data ] != NULL ) {
						if ( disks[ data ]->filesSize != 0 ) {
							wsprintf( tmp, &szLS[ 0 ]/*TEXT("%C:\\ {loaded with %d entries}")*/, disks[ data ]->DosDevice,                  disks[ data ]->filesSize );
							item.iItem   = i;
							item.pszText = tmp;
							SendMessage ( hCombo,    CBEM_SETITEM,         0, ( LPARAM ) &item );
							}
						else {
							wsprintf( tmp, &szNoS[ 0 ]/*TEXT("%C:\\ {UNSUPPORTED}")*/,           disks[ data ]->DosDevice,                  disks[ data ]->filesSize );
							item.iItem          = i;
							item.iImage         = 4;
							item.iSelectedImage = 4;
							item.pszText        = tmp;
							SendMessage ( hCombo,    CBEM_SETITEM,         0, ( LPARAM ) &item );
							item.iImage         = 2;
							item.iSelectedImage = 2;
							}
						loaded += disks[ data ]->realFiles;
						}
					else {
						wsprintf(     tmp,                                                       TEXT( "%C:\\" ),                           0x41 + data              );
						item.iItem              = i;
						item.pszText            = tmp;
						item.iImage             = 0;
						item.iSelectedImage     = 1;
						SendMessage(      hCombo,    CBEM_SETITEM,         0, ( LPARAM ) &item );
						item.iImage             = 2;
						item.iSelectedImage     = 2;
					}
				}
				}
			InvalidateRect( hCombo, NULL, TRUE );
			wsprintf(                 tmp,                                                       &szTotal[ 0 ]/*TEXT("%d entries total")*/, loaded                   );
			SendMessage(                  hStatus,   SB_SETTEXT,           1, ( LPARAM ) tmp   );
			break;
		case WM_CONTEXTMENU:
			if ( hListView == ( HWND ) wParam ) {
//#ifdef TRACING
//			TRACE( _T( "SearchDlg opening a context menu\r\n" ) );
//#endif
				index = SendMessage(      hListView, LVM_GETSELECTIONMARK, 0, 0                );
				if ( index >= 0 ) {
					POINTS pt;
					//POINT  pt2;
					RECT   rt;
					RECT   rt2;
					DWORD  dw;
					pt = MAKEPOINTS(lParam);
					if (pt.x == -1 && pt.y == -1) {
						ListView_GetItemRect      ( hListView, index, &rt, LVIR_SELECTBOUNDS );
						GetWindowRect             ( hListView,        &rt2                   );
						dw = ListView_GetTopIndex ( hListView                                );
						dw = index - dw;
						dw = TrackPopupMenu ( hm, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, rt2.left + rt.right, rt2.top + ( rt.bottom - rt.top )*( dw + 1 ), 0, hWnd, NULL );
						}
					else {
						dw = TrackPopupMenu ( hm, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x,                pt.y,                                        0, hWnd, NULL );
						}
					ProcessPopupMenu( hWnd, index, dw );
					}
				}
			break;
		case WM_COMMAND:
			id  = LOWORD( wParam );
			msg = HIWORD( wParam );
			if ( msg == EN_SETFOCUS ) {
				EnableWindow( GetDlgItem( hWnd, IDOK ), TRUE );
				break;
				}
			else if ( id == IDM_CLEAR ) {
				ListView_DeleteAllItems( hListView );
				EnableWindow( GetDlgItem( hWnd, IDOK ), TRUE );
				}
			else if ( id == IDM_ABOUT ) {
				DialogBox( hInst, MAKEINTRESOURCE( IDD_ABOUTBOX ), hWnd, About );
				}
			else if ( id == IDM_SAVE ) {
				OPENFILENAME of;
				wsprintf     ( tmp, &szResults[ 0 ]/*TEXT("results_%d.txt")*/, results_cnt );
				memset( &of, 0, sizeof( OPENFILENAME ) );
				of.lStructSize    = sizeof( OPENFILENAME );
				of.lpstrFile      = tmp;
				of.nMaxFile       = MAX_PATH;
				of.lpstrFilter    = szFiles;//TEXT("Textfile\0*.txt\0All files\0*.*\0\0");
				of.nFileExtension = 1;
				of.hwndOwner      = hWnd;
				of.Flags          = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
				of.lpstrTitle     = szSaveRes;//TEXT("Save results");
				if ( GetSaveFileName( &of ) != 0 ) {
					if ( SaveResults( of.lpstrFile ) == FALSE ) {
						MessageBox( hWnd, szSaveResErr /*TEXT("The results couldn't be saved.")*/, 0, MB_ICONERROR );
						}
					}
				}
			else if ( id == ID_HELP ) {
				DialogBox( hInst, MAKEINTRESOURCE( IDD_HELP ), hWnd, Help );
				}
			if ( id == IDOK || id == IDOK2 ) {
				if ( hListView == GetFocus( ) ) {
					index = SendMessage ( hListView, LVM_GETSELECTIONMARK, 0,     0 );
					if ( index >= 0 ) {
						ProcessPopupMenu( hWnd, index, IDM_OPEN );
						}
					}
				else {
					wsprintf ( tmp, &szSearch[ 0 ]/*TEXT("Searching ...")*/              );
					SendMessage         ( hStatus,   SB_SETTEXT,           0,     ( LPARAM ) tmp );
					
					index = SendMessage ( hCombo,    CB_GETCURSEL,         0,     0              );
					data  = SendMessage ( hCombo,    CB_GETITEMDATA,       index, 0              );
					GetDlgItemText( hWnd, IDC_EDIT, tmp, MAX_PATH );
					DWORD res;

					b = SendDlgItemMessage( hWnd,    IDC_DELETED, BM_GETCHECK,    0, 0 );
					if ( b == BST_CHECKED ) {
						res = Search ( hWnd, data, tmp, FALSE );
						}
					else {
						res = Search ( hWnd, data, tmp, TRUE  );
						}
					wsprintf ( tmp, &szFound[ 0 ]/* TEXT("%d files found.")*/, res      );
					SendMessage(          hStatus,   SB_SETTEXT,           0,     ( LPARAM ) tmp );
					b = SendDlgItemMessage( hWnd, IDC_LIVE,       BM_GETCHECK,               0, 0 );
					if (res>0 && b!=BST_CHECKED) {
						SetFocus( hListView );
						ListView_SetSelectionMark( hListView, 0 );
						}
					}
				break;
				}
			else if ( id == IDC_UNLOAD && msg == BN_CLICKED ) {
				index = SendMessage (     hCombo,    CB_GETCURSEL,         0,     0              );
				data  = SendMessage (     hCombo,    CB_GETITEMDATA,       index, 0              );
				if (data > 0 && data < 32) {
					ListView_DeleteAllItems( hListView );
					ReUseBlocks( PathStrings, FALSE );
					UnloadDisk( hWnd, data );
					}
				else {
					ListView_DeleteAllItems( hListView );
					ReUseBlocks( PathStrings, FALSE );
					for ( int i = 0; i < 32; i++ ) {//CANNOT be vectorized - "not enough type information"
						UnloadDisk( hWnd, i );
						}
					}
				}
			else if ( id == IDC_REFRESH ) {
				ProcessLoading( hWnd, hCombo, TRUE );
				}
			else if ( id == IDC_CASE ) {
				b = SendDlgItemMessage(     hWnd,    IDC_CASE,             BM_GETCHECK,               0, 0  );
				if ( b == BST_CHECKED ) {
					glSensitive = TRUE;
					}
				else {
					glSensitive = FALSE;
					}
				}
			else if ( msg == EN_CHANGE ) {
				int len;
				len = SendDlgItemMessage(   hWnd,    id,                   WM_GETTEXTLENGTH,          0, 0  );
				b   = SendDlgItemMessage(   hWnd,    IDC_LIVE,             BM_GETCHECK,               0, 0  );
				
				EnableWindow( GetDlgItem( hWnd, IDOK ), TRUE );
			
				if (len > 2 && b==BST_CHECKED) {
					SendMessage(          hWnd,      WM_COMMAND,           MAKEWPARAM( IDOK, BN_CLICKED ), 0 );
					}
				}
			else if ( msg == CBN_SELCHANGE ) {
				ProcessLoading( hWnd, ( HWND ) lParam, FALSE );
				}
			if ( LOWORD( wParam ) == IDCANCEL )  {
				EndDialog( hWnd, LOWORD( wParam ) );
				return TRUE;
				}
			break;
		case WM_HELP:
			if ( !glHelp ) {
				DialogBox( hInst, MAKEINTRESOURCE( IDD_HELP ), hWnd, Help );
				}
			break;
	}
	return FALSE;
}

DWORD WINAPI LoadSearchInfo(LPVOID lParam)
{
	//DWORD res;
	ULONGLONG   res;
	STATUSINFO  status;
	ThreadInfo* info;
	
	info = ( ThreadInfo* ) ( lParam );
	
	status.Value = PBM_SETPOS;
	status.hWnd  = GetDlgItem(info->hWnd, IDC_PROGRESS);

#ifdef TRACING
	TRACE( _T( "LoadSearchInfo\r\n" ) );
#endif

	if ( info->disk->filesSize == 0 ) {
		res = LoadMFT( info->disk, FALSE );
		if (res !=0) { //typo?: "if (res = LoadMFT(info->disk, FALSE)!=0)" 
#ifdef TRACING
			TRACE( _T( "LoadMFT returned %llu\r\n" ), res );
#endif
			SendDlgItemMessage( info->hWnd, IDC_PROGRESS, PBM_SETRANGE32, 0, info->disk->NTFS.entryCount );
			ParseMFT          ( info->disk, SEARCHINFO,   &status );
			}
		else if (res==3) {
			MessageBox(         info->hWnd, szStrange,    NULL, MB_ICONERROR );
			}
		}
	else {
		SendDlgItemMessage(     info->hWnd, IDC_PROGRESS, PBM_SETRANGE32, 0, info->disk->NTFS.entryCount );
		ReparseDisk(            info->disk, SEARCHINFO,   &status );
		}
	Sleep( 800 );//???BUGBUG
	SendMessage( info->hWnd, WM_USER + 1, 0, 0 );
	return 0;
}

void StartLoading(PDISKHANDLE disk, HWND hWnd)
{
	DialogBoxParam( hInst, ( LPCTSTR ) IDD_WAIT, hWnd, ( DLGPROC ) Waiting, ( LPARAM ) disk );
}

INT_PTR CALLBACK Waiting( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			if ( lParam != NULL ) {
				DWORD  threadId;
				HANDLE handle;
				TCHAR  tmp[ 256 ];
				DWORD  range;
	#ifdef TRACING
				TRACE( _T( "Loading dialog...\r\n" ) );
	#endif
				static ThreadInfo info;
				info.disk = ( PDISKHANDLE ) lParam;
				info.hWnd = hDlg;
				wsprintf      (       tmp, &szLoading[ 0 ]/*TEXT("Loading %C:\\ ... - Please wait")*/, info.disk->DosDevice );
				SetWindowText ( hDlg, tmp );
				range = SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_GETRANGE, TRUE, NULL );
				//handle = CreateThread(NULL, 0, LoadSearchInfo, (LPVOID) &info, 0, &threadId); 
				handle = ( HANDLE ) _beginthreadex( NULL, 0, ( unsigned int( __stdcall* )( void* ) )LoadSearchInfo, ( LPVOID ) &info, 0, ( unsigned int* ) &threadId );//void*?? BUGBUG
				CloseHandle( handle );
				}
			return ( INT_PTR ) TRUE;

		case WM_COMMAND:
#ifdef TRACING
			TRACE( _T( "'Waiting' handling WM_COMMAND...\r\n" ) );
#endif
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
				EndDialog( hDlg, LOWORD( wParam ) );
				return ( INT_PTR ) TRUE;
				}
			break;
		case WM_USER+1:
#ifdef TRACING
			TRACE( _T( "'Waiting' handling custom event!...\r\n" ) );
#endif
			HWND hWnd;
			hWnd = GetParent(hDlg);
			if ( hWnd != NULL ) {
				PostMessage( hWnd, WM_USER + 2, 0, 0 );
				}
			EndDialog( hDlg, 0 );
			return TRUE;
			break;
	}
	return (INT_PTR)FALSE;
}

VOID ReleaseAllDisks()
{
	for ( int i = 0; i < 32; i++ ) {
		if ( disks[ i ] != NULL ) {
			CloseDisk( disks[ i ] );
			}
		}
}

int Search(HWND hWnd, int disk, TCHAR *filename, BOOL deleted)
{
	DWORD ret = 0;
	SendMessage             ( hListView, WM_SETREDRAW, FALSE, 0 );
	ListView_DeleteAllItems ( hListView                         );
	ReUseBlocks( PathStrings, FALSE );
	ReUseBlocks( FileStrings, FALSE );
	results_cnt = 0;

	SEARCHP* pat;

	pat = StartSearch( filename, wcslen( filename ) );
	if ( pat == NULL ) {
		return 0;
		}

	if ( disk > 0 && disk < 32 ) {
		if ( disks[ disk ] != NULL ) {
			ret = SearchFiles( hWnd, disks[ disk ], filename, deleted, pat );
			}
		}
	else {
		for ( int i = 0; i < 32; i++ ) {//CANNOT be vectorized.
			if ( disks[ i ] != NULL && results_cnt < 0xfff0 ) {
				ret += SearchFiles( hWnd, disks[ i ], filename, deleted, pat );
				}
			}
		}
	if ( ret != results_cnt ) {
		DebugBreak( );
		}
	results_cnt = ret;
	ListView_SetItemCountEx ( hListView, results_cnt,  0       );
	SendMessage             ( hListView, WM_SETREDRAW, TRUE, 0 );
	EndSearch( pat );
	return ret;
}

int SearchFiles(HWND hWnd, PDISKHANDLE disk, TCHAR *filename, BOOL deleted, SEARCHP* pat)
{
	int    last;
	int    hit = 0;
	int    len = 0;
	int    res = 0;

	LVITEM item;

	//PUCHAR data;
	WCHAR  tmp[0xffff];
	SEARCHFILEINFO *info;

	if (!glSensitive) {
		  _wcslwr(filename);
		}

	len  = wcslen(filename);
	info = disk->fFiles;

	memset( &item, 0, sizeof( item ) );
	item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	
	for ( int i = 0; i < disk->filesSize; i++ ) { //cannot be vectorized (function calls)
		if ( deleted == TRUE || ( info[ i ].Flags & 0x1 ) ) {
			if ( info[ i ].FileName != NULL ) {
				if ( !glSensitive ) {
					//MessageBox(0,PSEARCHFILEINFO(data)->FileName,0,0);
					//wcscpy_s(tmp,info->FileName, info->FileNameLength);
					memcpy( tmp, info[ i ].FileName, info[ i ].FileNameLength*sizeof( TCHAR ) +2 );
					_wcslwr(tmp);
					res = SearchStr( pat, ( wchar_t* ) tmp, info[ i ].FileNameLength );
					}
				else {
					res = SearchStr( pat, ( wchar_t* ) info[ i ].FileName, info[ i ].FileNameLength );
					}
				if ( res == TRUE ) {
					/*
				//if (wcsstr(tmp, filename)!=NULL)
				//if (SearchString(tmp, PSEARCHFILEINFO(data)->FileNameLength, filename, len)==TRUE)
				//if (StringRegCompare(tmp, PSEARCHFILEINFO(data)->FileNameLength, filename, len)==TRUE)
					*/
					SearchResult*   res;
					res           = &results[ results_cnt++ ];//(SearchResult*) AllocData(SearchResults, sizeof(SearchResult));
					LPTSTR t      = GetPath ( disk, i );
					int    s      = wcslen  ( t );
					
					res->filename = ( LPTSTR ) info[ i ].FileName;
					res->path     = AllocAndCopyString( PathStrings, t, s );
					res->icon     = info[ i ].Flags;
					if ( !( info[ i ].Flags & 0x002 ) ) {
						LPTSTR ret = wcsrchr( res->filename, L'.' );
						if ( ret != NULL ) {
							res->extra = ret + 1;
							}
						else {
							res->extra = TEXT( " " );
							}
						}
					else {
						res->extra = TEXT( " " );
						}
					/*item.pszText = LPSTR_TEXTCALLBACK;//(LPWSTR)PSEARCHFILEINFO(data)->FileName;
					item.iItem	 = i;
					item.iImage = PSEARCHFILEINFO(data)->Flags;
					item.lParam = (LPARAM)res;
					*/
					last = ListView_InsertItem( hListView, &item );
					/*
					//swprintf(tmp,L"%u",i);
				//	ListView_SetItemText(hListView,last,1, LPSTR_TEXTCALLBACK);
					*/
					hit++;
					if ( results_cnt > 0xfff0 ) {//0xfff0 = 65520
						//int res;
						//res = MessageBox(0, TEXT("Your search produces too many results!\nContinue your search?"), 0, MB_ICONINFORMATION | MB_TASKMODAL | MB_YESNO);
						MessageBox( hWnd, &szTooMany[ 0 ]/*TEXT("Your search produces too many results!")*/, 0, MB_ICONWARNING | MB_OK );
						/*
						//if (res!=IDYES)
						//{
							//SendMessage(hListView, WM_SETREDRAW,TRUE,0);
						//}*/
						break;
						}
					}
				}
			}
		//data+=disk->IsLong;
		}
	//qsort(result, hit, sizeof(SearchResult),
	//SendMessage(hListView, WM_SETREDRAW,TRUE,0);
	return hit;
}
UINT ExecuteFile(HWND hWnd, LPWSTR str, USHORT flags)
{
	UINT res;
	SHELLEXECUTEINFO shell;
	
	memset( &shell,  0, sizeof( SHELLEXECUTEINFO ) );
	shell.cbSize     =  sizeof( SHELLEXECUTEINFO );
	shell.lpFile     =  str;
	shell.fMask      =  SEE_MASK_INVOKEIDLIST;
	shell.lpVerb     =  NULL;
	shell.nShow      =  SW_SHOWDEFAULT;
	

	if ( !( flags & 0x001 ) ) {
		MessageBox( hWnd, szDeletedFile /*TEXT("The file is deleted and cannot be accessed throught the filesystem driver.\nUse a recover program to get access to the stored data.")*/, 0, MB_ICONWARNING );
		return 0;
		}

	//res = (UINT) ShellExecute(0,NULL, str, NULL, NULL, SW_SHOWDEFAULT);
	ShellExecuteEx( &shell );
	res = ( UINT ) shell.hInstApp;
	switch ( res )
	{
		case SE_ERR_NOASSOC:
			ShellExecute( 0, TEXT( "openas" ), str, NULL, NULL, SW_SHOWDEFAULT );
			break;
		case SE_ERR_ASSOCINCOMPLETE:
			break;
		case SE_ERR_ACCESSDENIED:
			MessageBox( hWnd, szAccessDenied, 0, MB_ICONERROR );
			break;
		case ERROR_PATH_NOT_FOUND:
			//MessageBox(hWnd, TEXT("The path couldn't be found.\nProbably the path is hidden."), 0, MB_ICONWARNING);
			break;
		case ERROR_FILE_NOT_FOUND:
			//MessageBox(hWnd, szFNF/*TEXT("The file couldn't be found.\nThe file is probably hidden or a metafile.")*/, 0, MB_ICONERROR);
			break;
		case ERROR_BAD_FORMAT:
			//MessageBox(hWnd, TEXT("This is not a valid Win32 Executable File."), 0, MB_ICONINFORMATION);
			break;
		case SE_ERR_DLLNOTFOUND:
			break;
		default:
			//if (res>32)
			break;
	}
	return res;
}

BOOL UnloadDisk(HWND hWnd, int index)
{
	//PUCHAR data;
	if ( disks[ index ] != NULL ) {
		/*data = (PUCHAR) disks[index]->sFiles;
		for (int i=0;i<disks[index]->filesSize;i++)
		{
			if (PSEARCHFILEINFO(data)->FileName !=NULL)
				delete PSEARCHFILEINFO(data)->FileName;
			data +=disks[index]->IsLong;
		}*/
		CloseDisk( disks[ index ] );
		disks[ index ] = NULL;
		SendMessage( hWnd, WM_USER + 2, 0, 0 );
		return TRUE;
		}
	return FALSE;
}

UINT ExecuteFileEx(HWND hWnd, LPTSTR command, LPWSTR str, LPCTSTR dir, UINT show, USHORT flags)
{
	UINT             res;
	SHELLEXECUTEINFO shell;
	
	memset( &shell,   0, sizeof( SHELLEXECUTEINFO ) );
	shell.cbSize      =  sizeof( SHELLEXECUTEINFO );
	shell.lpFile      =  str;
	shell.fMask       =  SEE_MASK_INVOKEIDLIST;
	shell.lpVerb      =  command;
	shell.nShow       =  show;
	shell.lpDirectory =  dir;

	if ( !( flags & 0x001 ) ) {
		MessageBox( hWnd, szDeletedFile/*TEXT("The file is deleted and cannot be accessed throught the filesystem driver.\nUse a recover program to get access to the stored data.")*/, 0, MB_ICONWARNING );
		return 0;
		}

	//res = (UINT) ShellExecute(0,NULL, str, NULL, NULL, SW_SHOWDEFAULT);
	ShellExecuteEx( &shell );
	res = ( UINT ) shell.hInstApp;
	switch ( res )
	{
		case SE_ERR_NOASSOC:
			ShellExecute( 0, TEXT( "openas" ), str, NULL, NULL, SW_SHOWDEFAULT );
			break;
		case SE_ERR_ASSOCINCOMPLETE:
			break;
		case SE_ERR_ACCESSDENIED:
			MessageBox( hWnd, szAccessDenied, 0, MB_ICONERROR );
			break;
		case ERROR_PATH_NOT_FOUND:
			//MessageBox(hWnd, TEXT("The path coulnd't be found.\nPropably the path is hidden."), 0, MB_ICONWARNING);
			break;
		case ERROR_FILE_NOT_FOUND:
			//MessageBox(hWnd, TEXT("The file coulnd't be found.\nThe file is propably hidden or a metafile."), 0, MB_ICONERROR);
			break;
		case ERROR_BAD_FORMAT:
			//MessageBox(hWnd, TEXT("This is not a valid Win32 Executable File."), 0, MB_ICONINFORMATION);
			break;
		case SE_ERR_DLLNOTFOUND:
			break;
		default:
			//if (res>32)
			break;
	}
	return res;
}

/*BOOL SearchString(LPWSTR string, int length, LPWSTR pattern, int len)
{
	int p=0, s=0;
	if (length < len) 
		return FALSE;

	return TRUE;
}
*/
int filecompare( const void *arg1, const void *arg2 )
{
	/*
	  Compare all of both strings:
	*/
	return _wcsicmp( PSearchResult( arg1 )->filename, PSearchResult( arg2 )->filename );
}

int pathcompare( const void *arg1, const void *arg2 )
{
	/*
	  Compare all of both strings:
	*/
	return _wcsicmp( PSearchResult( arg1 )->path, PSearchResult( arg2 )->path );
}

int extcompare( const void *arg1, const void *arg2 )
{
	/*
	  Compare all of both strings:
	*/
	return _wcsicmp( PSearchResult( arg1 )->extra, PSearchResult( arg2 )->extra );
}

BOOL SaveResults(LPWSTR filename)
{
	bool   error = false;
	DWORD  written;
	TCHAR  buff[0xffff];
	HANDLE file = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL );

	if ( file == INVALID_HANDLE_VALUE ) {
		return FALSE;
		}
	
	for ( int i = 0; i < results_cnt; i++ ) {//CANNOT be vectorized (function call)
		wcscpy_s( buff, results[ i ].path );
		wcscat_s( buff, results[ i ].filename );
		wcscat_s( buff, TEXT( "\r\n" ) );
		if ( WriteFile( file, buff, wcslen( buff )*sizeof( TCHAR ), &written, NULL ) != TRUE ) {
			error = true;
			}
		}
	
	CloseHandle(file);
	
	if ( error == true ) {
		return FALSE;
		}
	
	return TRUE;
}

BOOL ProcessPopupMenu(HWND hWnd, int index, DWORD item)
{
	TCHAR   path[0xffff];
	int     len;
	
	wcscpy( path, results[ index ].path );
	
	len = wcslen( path );
	path[ len ] = 0;

	switch(item)
	{
		case IDM_OPEN:
			wcscpy( &path[ len ],                                         results[ index ].filename                                         );
			len += wcslen( path );
			path[ len ] = 0;
			ExecuteFile(   hWnd,                       path,              results[ index ].icon                                             );
			break;
		case IDM_OPENMIN:
			wcscpy( &path[ len ],                                         results[ index ].filename                                         );
			len += wcslen( path );
			path[ len ] = 0;
			ExecuteFileEx( hWnd, NULL,                 path,              results[ index ].path,     SW_MINIMIZE,     results[ index ].icon );
			break;
		case IDM_OPENWITH:
			wcscpy( &path[ len ],                                         results[ index ].filename                                         );
			len += wcslen( path );
			path[ len ] = 0;
			ExecuteFileEx( hWnd, TEXT( "openas" ),     path,              results[ index ].path,     SW_SHOWDEFAULT,  results[ index ].icon );
			break;
		case IDM_OPENDIR:
			ExecuteFileEx( hWnd, TEXT( "explore" ),    path,              results[ index ].path,     SW_SHOWDEFAULT,  results[ index ].icon );
			break;
		case ID_CMDPROMPT:
			ExecuteFileEx( hWnd, TEXT( "open" ),       TEXT( "cmd.exe" ), results[ index ].path,     SW_SHOWDEFAULT,  results[ index ].icon );
			break;
		case IDM_PROPERTIES:
			wcscpy( &path[ len ],                                         results[ index ].filename                                         );
			len += wcslen( path );
			path[ len ] = 0;
			ExecuteFileEx( hWnd, TEXT( "properties" ), path,              results[ index ].path,     SW_SHOWDEFAULT,  results[ index ].icon );
			break;
		case ID_COPY:
			PrepareCopy( hWnd, FILES );
			break;
		case ID_COPY_NAMES:
			PrepareCopy( hWnd, FILENAMES );
			break;
		case ID_DELETE:
			DeleteFiles( hWnd, DELETENOW );
			break;
		case ID_DELETE_ON_REBOOT:
			DeleteFiles( hWnd, DELETEONREBOOT );
			break;
		default:
			break;
	}

	return TRUE;
}

void PrepareCopy(HWND hWnd, UINT flags)
{
	DWORD     datasize = 0;

	UINT      mask;

	//STGMEDIUM stg;
	
	DROPFILES files;
	
	int       newline;
	int       structsize;
	int       len;

	TCHAR     *buff = new TCHAR[ 0x100000 ];

	PVOID     ptr;
	
	HGLOBAL   hdrop = GlobalAlloc( 0, /*sizeof(stg) + */structsize + datasize*sizeof( TCHAR ) );

	switch ( flags )
	{
		case FILES:
			newline    = 0;
			structsize = sizeof( files );
			break;
		case FILENAMES:
		default:
			newline    = L'\n';
			structsize = 0;
			break;
	}

	/* stg.tymed = TYMED_HGLOBAL;
	   stg.hGlobal = NULL;
	*/
	files.fNC    = TRUE;
	files.fWide  = TRUE;
	files.pt.x   = 0;
	files.pt.y   = 0;
	files.pFiles = sizeof( files );
	
	for ( int i = 0; i < results_cnt; i++ ) {//CANNOT be vectorized
		mask = ListView_GetItemState( hListView, i, LVIS_SELECTED );
		if ( mask & LVIS_SELECTED && results[ i ].icon & 0x001 ) {
			wcscpy( &buff[ datasize ], results[ i ].path );
			len = wcslen( &buff[ datasize ] );
			wcscpy( &buff[ datasize + len ], results[ i ].filename );
			len += wcslen( &buff[ datasize + len ] );
			
			if ( flags == FILES ) {
				buff[ datasize + len ] = 0;
				}
			else {
				buff[ datasize + len ] = L'\r';
				buff[ datasize + len + 1 ] = L'\n';
				datasize += 1;
				}
			datasize += len + 1;
			if ( datasize > 0x7fff0 ) {
				break;
				}
			}
		}
	buff[ datasize   ] = 0;
	buff[ datasize++ ] = 0;
	/*
	//stg.hGlobal = GlobalAlloc(0, sizeof(files) + datasize*sizeof(TCHAR));
	//HANDLE hdrop = GlobalAlloc(0, /\*sizeof(stg) + *\/structsize + datasize*sizeof(TCHAR));
	//ptr  = GlobalLock(hdrop);
	//CopyMemory(ptr, &stg, sizeof(STGMEDIUM));
	//GlobalUnlock(hdrop);
	*/
	ptr = GlobalLock( hdrop );
	CopyMemory( ptr, &files, structsize );
	//ASSERT( sizeof( buff ) == datasize*sizeof( TCHAR ) );

	CopyMemory( PUCHAR( ptr ) + structsize, buff, datasize*sizeof( TCHAR ) );
	//CopyMemory: Dest, source, length
	
	GlobalUnlock( hdrop );
	
	if ( OpenClipboard( hWnd ) ) {
		if ( EmptyClipboard( ) ) {
			if ( flags == FILES ) {
				if ( !SetClipboardData( CF_HDROP, hdrop ) ) {
					GlobalFree( hdrop );
					}
				}
			else {
				if ( !SetClipboardData( CF_UNICODETEXT, hdrop ) ) {
					GlobalFree( hdrop );
					}
				}
			}
		else {
			GlobalFree( hdrop );
			}
		CloseClipboard( );
		}
	else {
		GlobalFree( hdrop );
		}
	delete[] buff;
}

void DeleteFiles(HWND hWnd, UINT flags)
{
	DWORD res;
	UINT  mask;
	int   len;
	TCHAR buff[0x8000];
	TCHAR path[0x8000];

	if ( flags == DELETENOW ) {
		for ( int i = 0; i < results_cnt; i++ ) {//CANNOT be vectorized
			mask = ListView_GetItemState( hListView, i, LVIS_SELECTED );
			if ( mask & LVIS_SELECTED && results[ i ].icon & 0x001 ) {
				wcscpy( path, results[ i ].path );
				len = wcslen( path );
				wcscpy( &path[ len ], results[ i ].filename );
				len += wcslen( &path[ len ] );
				path[ len ] = 0;
					
				wsprintf( buff, szDelete/*TEXT("Are you sure you want to delete\n\n%.1024s\n\nfrom disk?\nYou can't restore this file!")*/, path );
				res = MessageBox( hWnd, buff, szWarning/*TEXT("WARNING")*/, MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON2 );
				if ( res == IDYES ) {
					if ( !DeleteFile( path ) ) {
						ShowError( );
						}
					}
				else if ( res == IDCANCEL ) {
					break;
					}
				}	
			}
		}
	else if ( flags == DELETEONREBOOT ) {
		for ( int i = 0; i < results_cnt; i++ ) {//CANNOT be vectorized
			mask = ListView_GetItemState( hListView, i, LVIS_SELECTED );
			if ( mask & LVIS_SELECTED && results[ i ].icon & 0x001 ) {
				wcscpy( path, results[ i ].path );
				len = wcslen( path );
				wcscpy( &path[ len ], results[ i ].filename );
				len += wcslen( &path[ len ] );
				path[ len ] = 0;
					
				wsprintf( buff, szDeleteOnReboot/*TEXT("Warning!!!\nThis is very dangerous - you try to delete a file on reboot - this may cause damage to the system.\nYour system might be unaccessible afterwards.\n\nAre you sure you want to delete\n\n%.1024s\n\nfrom disk?\nYou can't restore this file!")*/, path );
				res = MessageBox( hWnd, buff, szWarning/*TEXT("WARNING")*/, MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON2 );
				if ( res == IDYES ) {
					if ( !MoveFileEx( path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ) {
						ShowError( );
						}
					}
				else if ( res == IDCANCEL ) {
					break;
					}
				}
			}
		}
	else {
		}
}

DWORD GetDllVersion(LPCTSTR lpszDllName)
{
	HINSTANCE hinstDll;
	DWORD     dwVersion = 0;

	/* 
	  For security purposes, LoadLibrary should be provided with a fully-qualified path to the DLL.
	  The lpszDllName variable should be tested to ensure that it is a fully qualified path before it is used. 
	*/
	hinstDll = LoadLibrary( lpszDllName );
	
	if ( hinstDll ) {
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = ( DLLGETVERSIONPROC ) GetProcAddress( hinstDll, "DllGetVersion" );

		/*
		  Because some DLLs might not implement this function, you must test for it explicitly.
		  Depending on the particular  DLL, the lack of a DllGetVersion function can be a useful indicator of the version.
		*/

		if ( pDllGetVersion ) {
			DLLVERSIONINFO dvi;
			HRESULT        hr;
			SecureZeroMemory( &dvi, sizeof( dvi ) );
			dvi.cbSize = sizeof( dvi );
			hr = ( *pDllGetVersion )( &dvi );
			if ( SUCCEEDED( hr ) ) {
				dwVersion = PACKVERSION( dvi.dwMajorVersion, dvi.dwMinorVersion );
				}
			}
		FreeLibrary( hinstDll );
		}
	return dwVersion;
}

DWORD ShowError() 
{ 
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	//LPVOID lpDisplayBuf;
	DWORD  dw = GetLastError(); 

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
	MessageBox( NULL, ( LPCTSTR ) lpMsgBuf, 0, MB_OK | MB_ICONINFORMATION );

	LocalFree( lpMsgBuf );
	return dw;
}

int ProcessLoading(HWND hWnd, HWND hCombo, int reload)
{
	int index;
	int data;
	int b;
	EnableWindow( GetDlgItem( hWnd, IDOK ), TRUE );
	index = SendMessage( ( HWND ) hCombo, CB_GETCURSEL, 0, 0 );
	data  = SendMessage( ( HWND ) hCombo, CB_GETITEMDATA, index, 0 );
#ifdef TRACING
	TRACE( _T( "ProcessLoading\r\n" ) );
#endif
	if ( data > 0 && data < 32 ) {
		b = SendDlgItemMessage( hWnd, IDC_LOADALWAYS, BM_GETCHECK, 0, 0 );
		if ( disks[ data ] == NULL ) {
			disks[ data ] = OpenDisk( 0x41 + data );
			if ( disks[ data ] != NULL ) {
				//LoadMFT(disks[data], FALSE);
				StartLoading(disks[data], hWnd);
				}
			else {
				MessageBox( hWnd, szDiskError/*TEXT("The disk couldn't be opened.\nYOU HAVE TO BE ADMINISTRATOR TO USE THIS TOOL.")*/, 0, MB_ICONINFORMATION );
				// set different icon
				}
			}
		else if ( b == BST_CHECKED || reload > 0 ) {
			StartLoading( disks[ data ], hWnd );
			}
		}
	else if ( data == 0xfe ) {
		DWORD drives;
		drives = GetLogicalDrives( );
		for ( int i = 0; i < 32; i++ ) {
			if ( ( drives >> ( i ) ) & 0x1 ) {
				TCHAR str[5];
				UINT  type;
				wsprintf( str, TEXT( "%C:\\" ), 0x41 + i );
				type = GetDriveType( str );
				if ( type == DRIVE_FIXED ) {
					if ( disks[ i ] == NULL ) {
						disks[ i ] = OpenDisk( 0x41 + i );
						if ( disks[ i ] != NULL ) {
							StartLoading( disks[ i ], hWnd );
							}
						else {
							// set different icon
							}
						}
					}
				}
			}
		SendMessage( hCombo, CB_SETCURSEL, 0, 0 );
		}
	else if ( reload > 0 ) {
		for ( int i = 0; i<32; i++ ) {
			if ( disks[ i ] != NULL ) {
				StartLoading( disks[ i ], hWnd );
				}
			}
		}
	SetFocus( GetDlgItem( hWnd, IDC_EDIT ) );
	return 0;
}