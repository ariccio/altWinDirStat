// dirstatdoc.cpp: Implementation of CDirstatDoc
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"
#include "windirstat.h"
#include "item.h"
#include "mainframe.h"
#include "osspecific.h"
#include "deletewarningdlg.h"
#include "modalshellapi.h"
#include ".\dirstatdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	const COLORREF _cushionColors[] = {
		RGB(0, 0, 255),
		RGB(255, 0, 0),
		RGB(0, 255, 0),
		RGB(0, 255, 255),
		RGB(255, 0, 255),
		RGB(255, 255, 0),
		RGB(150, 150, 255),
		RGB(255, 150, 150),
		RGB(150, 255, 150),
		RGB(150, 255, 255),
		RGB(255, 150, 255),
		RGB(255, 255, 150),
		RGB(255, 255, 255)
	};
}

CDirstatDoc *_theDocument;

CDirstatDoc *GetDocument()
{
	return _theDocument;
}


IMPLEMENT_DYNCREATE(CDirstatDoc, CDocument)


CDirstatDoc::CDirstatDoc()
{
	ASSERT(_theDocument == NULL);
	_theDocument    = this;
	m_rootItem      = NULL;
	m_workingItem   = NULL;
	m_zoomItem      = NULL;
	m_selectedItem  = NULL;

	m_showFreeSpace = CPersistence::GetShowFreeSpace();
	m_showUnknown   = CPersistence::GetShowUnknown();
	m_extensionDataValid = false;

	//TRACE(_T("sizeof(CItem) = %d\r\n"), sizeof(CItem));
}

CDirstatDoc::~CDirstatDoc()
{
	if ( ( m_showFreeSpace != NULL ) && ( m_showUnknown != NULL ) ) {
		CPersistence::SetShowFreeSpace( m_showFreeSpace );
		CPersistence::SetShowUnknown( m_showUnknown );
		}
	if ( m_rootItem != NULL ) {
		delete m_rootItem;
		}
	if ( _theDocument != NULL ) {
		_theDocument   = NULL;
		}
}

// Encodes a selection from the CSelectDrivesDlg into a string which can be routed as a pseudo document "path" through MFC and finally arrives in OnOpenDocument().
CString CDirstatDoc::EncodeSelection(_In_ const RADIO radio, _In_ const CString folder, _In_ const CStringArray& drives)
{
	CString ret;
	TRACE( _T( "Encoding selection %s\r\n" ), folder );
	switch (radio)
	{
		case RADIO_ALLLOCALDRIVES:
		case RADIO_SOMEDRIVES:
			{
				for (int i = 0; i < drives.GetSize(); i++) {
					if ( i > 0 ) {
						ret += CString( GetEncodingSeparator( ) );
						}
					ret     += drives[i];
					}
			}
			break;
		
		case RADIO_AFOLDER:
			ret.Format( _T( "%s" ), folder.GetString( ) );
			break;
	}
	TRACE( _T( "Selection encoded as '%s'\r\n" ), ret );
	return ret;
}

// The inverse of EncodeSelection
void CDirstatDoc::DecodeSelection(_In_ const CString s, _Inout_ CString& folder, _Inout_ CStringArray& drives)
{
	folder.Empty();
	drives.RemoveAll();

	// s is either something like "C:\programme"
	// or something like "C:|D:|E:".

	CStringArray sa;
	int i = 0;

	while ( i < s.GetLength( ) ) {
		CString token;
		while ( i < s.GetLength( ) && s[ i ] != GetEncodingSeparator( ) ) {
			token += s[ i++ ];
			}
		
		token.TrimLeft( );
		token.TrimRight( );
		ASSERT( !token.IsEmpty( ) );
		sa.Add( token );

		if ( i < s.GetLength( ) ) {
			i++;
			}
		}

	ASSERT( sa.GetSize( ) > 0 );
	if (sa.GetSize() > 1) {
		for (int j = 0; j < sa.GetSize(); j++) {
			CString d = sa[ j ];
			ASSERT( d.GetLength() == 2);
			ASSERT( d[1] == _T(':'));

			drives.Add(d + _T("\\"));
			}
		}
	else {
		CString f = sa[0];
		if (f.GetLength() == 2 && f[1] == _T(':')) {
			drives.Add(f + _T("\\"));
			}
		else {
			// Remove trailing backslash, if any and not drive-root.
			if (f.GetLength() > 0 && f.Right(1) == _T("\\") && (f.GetLength() != 3 || f[1] != _T(':'))) {
				f = f.Left(f.GetLength() - 1);
				}

			folder = f;
			}
		}
}

TCHAR CDirstatDoc::GetEncodingSeparator()
{
	return _T('|'); // This character must be one, which is not allowed in file names.
}

void CDirstatDoc::DeleteContents()
{
	//TRACE( _T("Deleting contents of %s\r\n"), m_selectedItem->GetPath() );
	if ( m_rootItem != NULL ) {
		delete m_rootItem;
		m_rootItem   = NULL;
		}
#pragma warning(suppress: 6387)
	SetWorkingItem( NULL );
	if ( m_zoomItem != NULL ) {
		m_zoomItem   = NULL;
		}
	if ( m_selectedItem != NULL ) {
		m_selectedItem   = NULL;
		}
	GetApp()->ReReadMountPoints();
}

BOOL CDirstatDoc::OnNewDocument()
{
	
	if ( !CDocument::OnNewDocument( ) ) {
		return FALSE;
		}
	UpdateAllViews(NULL, HINT_NEWROOT);
	return TRUE;
}

BOOL CDirstatDoc::OnOpenDocument(_In_ const LPCTSTR lpszPathName)
{
	CDocument::OnNewDocument(); // --> DeleteContents()
	TRACE( _T( "Opening new document, path: %s\r\n" ), lpszPathName );
	CString spec = lpszPathName;
	CString folder;
	CStringArray drives;
	DecodeSelection(spec, folder, drives);
	
	CStringArray rootFolders;
	if (drives.GetSize() > 0) {
		m_showMyComputer = (drives.GetSize() > 1);
		for ( int i = 0; i < drives.GetSize( ); i++ ) {
			rootFolders.Add( drives[ i ] );
			}
		}
	else {
		ASSERT(!folder.IsEmpty());
		m_showMyComputer = false;
		rootFolders.Add(folder);
		}

	CArray<CItem *, CItem *> driveItems;

	if (m_showMyComputer) {
		m_rootItem = new CItem((ITEMTYPE)(IT_MYCOMPUTER|ITF_ROOTITEM), LoadString(IDS_MYCOMPUTER));
		for (int i = 0; i < rootFolders.GetSize(); i++) {
			CItem *drive = new CItem(IT_DRIVE, rootFolders[i]);
			driveItems.Add(drive);
			m_rootItem->AddChild(drive);
			}
		}
	else {
		ITEMTYPE type= IsDrive(rootFolders[0]) ? IT_DRIVE : IT_DIRECTORY;
		m_rootItem= new CItem((ITEMTYPE)(type|ITF_ROOTITEM), rootFolders[0], false);
		if ( m_rootItem->GetType( ) == IT_DRIVE ) {
			driveItems.Add( m_rootItem );
			}
		m_rootItem->UpdateLastChange();
		}
	m_zoomItem= m_rootItem;

	for (int i=0; i < driveItems.GetSize(); i++) {
		if ( OptionShowFreeSpace( ) ) {
			driveItems[ i ]->CreateFreeSpaceItem( );
			}
		if ( OptionShowUnknown( ) ) {
			driveItems[ i ]->CreateUnknownItem( );
			}
		}

	TRACE( _T( "**BANG** ---AAAAND THEY'RE OFF! THE RACE HAS BEGUN!\r\n" ) );
	BOOL behavedWell = QueryPerformanceCounter( &m_searchStartTime );
	if ( !behavedWell ) {
		exit( 666 );
		}
	behavedWell = QueryPerformanceFrequency( &m_timerFrequency );
	if ( !behavedWell ) {
		exit( 666 );
		}

	SetWorkingItem(m_rootItem);
	GetMainFrame()->FirstUpdateProgress( );
	GetMainFrame()->MinimizeGraphView();
	GetMainFrame()->MinimizeTypeView();

	UpdateAllViews(NULL, HINT_NEWROOT);
	GetMainFrame( )->FirstUpdateProgress( );
	return true;
}

void CDirstatDoc::SetPathName( _In_ const LPCTSTR lpszPathName, BOOL /*bAddToMRU*/)
{
	/*
	  We don't want MFCs AfxFullPath()-Logic, because lpszPathName is not a path. So we have overridden this.
	  MRU would be fine but is not implemented yet.
	*/
	 

	m_strPathName = lpszPathName;
	ASSERT(!m_strPathName.IsEmpty());       // must be set to something
	m_bEmbedded = FALSE;
	SetTitle(lpszPathName);

	ASSERT_VALID(this);
}

void CDirstatDoc::Serialize(_In_ const CArchive& /*ar*/)
{
}

// Prefix the window title (with percentage or "Scanning")
//
void CDirstatDoc::SetTitlePrefix(const CString prefix)
{
	CString docName = prefix + GetTitle( );
	TRACE( _T( "Setting window title to '%s'\r\n" ), docName );
	GetMainFrame( )->UpdateFrameTitleForDocument( docName );
}

COLORREF CDirstatDoc::GetCushionColor(_In_ LPCTSTR ext)
{
	SExtensionRecord r;
	r.bytes = 0;
	r.color = COLORREF(0);
	r.files = 0;
	VERIFY(GetExtensionData()->Lookup(ext, r));//Hotpath?
	return r.color;
}

COLORREF CDirstatDoc::GetZoomColor() const
{
	return RGB(0,0,255);
}

bool CDirstatDoc::OptionShowFreeSpace() const
{
	return m_showFreeSpace;
}

bool CDirstatDoc::OptionShowUnknown() const
{
	return m_showUnknown;
}

//const CExtensionData *CDirstatDoc::GetExtensionData()
CExtensionData *CDirstatDoc::GetExtensionData( )
{
	if ( !m_extensionDataValid ) {
		RebuildExtensionData( );
		}
	return &m_extensionData;
}

LONGLONG CDirstatDoc::GetRootSize() const
{
	ASSERT(m_rootItem != NULL);
	ASSERT(IsRootDone());
	return m_rootItem->GetSize();
}

void CDirstatDoc::ForgetItemTree()
{
	// The program is closing.
	// As "delete m_rootItem" can last a long time (many minutes), if we have been paged out, we simply forget our item tree here and hope that the system will free all our memory anyway.
	//delete m_rootItem;//seems fine to me!
	m_rootItem     = NULL;
	TRACE(_T("Not deleting m_rootItem!\r\n") );//FIXME
	m_zoomItem     = NULL;
	m_selectedItem = NULL;
	
}

// This method does some work for ticks ms. 
// return: true if done or suspended.
//
bool CDirstatDoc::Work( _In_ DWORD ticks ) {
	//TRACE( _T( "Doing work for %lu\r\n"), ticks );//noisy as shit
	if ( m_rootItem == NULL ) {
		return true;
		}

	if ( GetMainFrame( )->IsProgressSuspended( ) ) {
		return true;
		}

	if ( !m_rootItem->IsDone( ) ) {
		m_rootItem->DoSomeWork( ticks );
		if ( m_rootItem->IsDone( ) ) {
			m_extensionDataValid = false;

			GetMainFrame( )->SetProgressPos100( );
			GetMainFrame( )->RestoreTypeView( );

			LARGE_INTEGER doneTime;
			BOOL behavedWell = QueryPerformanceCounter( &doneTime );
			if ( !behavedWell ) {
				exit( 666 );
				}
			const double AdjustedTimerFrequency = ( ( double ) 1 ) / m_timerFrequency.QuadPart;
			m_searchTime = ( doneTime.QuadPart - m_searchStartTime.QuadPart) * AdjustedTimerFrequency;


			GetMainFrame( )->RestoreGraphView( );
			
			UpdateAllViews(NULL);//nothing has been done?
			//Complete?
			}
		else {
			ASSERT(m_workingItem != NULL);
			if ( m_workingItem != NULL ) { // to be honest, "defensive programming" is stupid, but c'est la vie: it's safer. //<== Whoever wrote this is wrong about "defensive programming" == stupid
				GetMainFrame( )->SetProgressPos( m_workingItem->GetProgressPos( ) );
				}
			UpdateAllViews(NULL, HINT_SOMEWORKDONE);
			}

		}
	if (m_rootItem->IsDone()) {
		SetWorkingItem(NULL);
		return true;
		}
	else {
		return false;
		}
}

bool CDirstatDoc::IsDrive(_In_ CString spec)
{
	return (spec.GetLength() == 3 && spec[1] == _T(':') && spec[2] == _T('\\'));
}

// Starts a refresh of all mount points in our tree.
// Called when the user changes the follow mount points option.
//
void CDirstatDoc::RefreshMountPointItems()
{
	CWaitCursor wc;

	CItem *root = GetRootItem();
	if ( root == NULL ) {
		return;
		}
	RecurseRefreshMountPointItems(root);
}

// Starts a refresh of all junction points in our tree.
// Called when the user changes the ignore junction points option.
//
void CDirstatDoc::RefreshJunctionItems()
{
	CWaitCursor wc;

	CItem *root =  GetRootItem();
	if (   root == NULL ) {
		return;
		}
	RecurseRefreshJunctionItems(root);
}

bool CDirstatDoc::IsRootDone()    const
{
	return m_rootItem != NULL && m_rootItem->IsDone();
}

CItem *CDirstatDoc::GetRootItem() const
{
	return m_rootItem;
}

CItem *CDirstatDoc::GetZoomItem() const
{
	return m_zoomItem;
}

bool CDirstatDoc::IsZoomed()      const
{
	return GetZoomItem() != GetRootItem();
}

void CDirstatDoc::SetSelection(_In_ const CItem *item, _In_ const bool keepReselectChildStack)
{
	CItem *newzoom = CItem::FindCommonAncestor( m_zoomItem, item );
	TRACE( _T( "Setting new selection\r\n" ) );
	if ( newzoom  != m_zoomItem ) {
		SetZoomItem( newzoom );
		}

	bool keep = keepReselectChildStack || m_selectedItem == item;

	m_selectedItem = const_cast< CItem * >( item );
	GetMainFrame( )->SetSelectionMessageText( );

	if ( !keep ) {
		ClearReselectChildStack( );
		}
}

CItem *CDirstatDoc::GetSelection() const
{
	return m_selectedItem;
}

void CDirstatDoc::SetHighlightExtension(_In_ const LPCTSTR ext)
{
	m_highlightExtension = ext;
	TRACE( _T( "Highlighting extension %s\r\n" ), m_highlightExtension );
	GetMainFrame()->SetSelectionMessageText();
}

CString CDirstatDoc::GetHighlightExtension() const
{
	return m_highlightExtension;
}

void CDirstatDoc::UnlinkRoot()
{
	/*
	  The very root has been deleted.
	*/
	TRACE( _T("The very root has been deleted!\r\n"));
	DeleteContents();
	UpdateAllViews(NULL, HINT_NEWROOT);
}


LONGLONG CDirstatDoc::GetWorkingItemReadJobs() const
{
	if ( m_workingItem != NULL ) {
		return m_workingItem->GetReadJobs( );
		}
	else {
		return 0;
		}
}

void CDirstatDoc::OpenItem(_In_ const CItem *item)
{
	ASSERT( item != NULL );
	CWaitCursor wc;

	try
	{
		CString path;
		switch (item->GetType())
		{
		case IT_MYCOMPUTER:
			{
				SHELLEXECUTEINFO sei;
				SecureZeroMemory( &sei, sizeof( sei ) );
				sei.cbSize = sizeof(sei);
				sei.hwnd   = *AfxGetMainWnd();
				sei.lpVerb = _T("open");
				//sei.fMask= SEE_MASK_INVOKEIDLIST;
				sei.nShow  = SW_SHOWNORMAL;
				CCoTaskMem<LPITEMIDLIST> pidl;
			
				GetPidlOfMyComputer( &pidl );
				sei.lpIDList = pidl;
				sei.fMask   |= SEE_MASK_IDLIST;
				ShellExecuteEx( &sei );
				// ShellExecuteEx seems to display its own Messagebox, if failed.
				return;
			}
			break;
		case IT_DRIVE:
		case IT_DIRECTORY:
			path = item->GetFolderPath();
			break;
		case IT_FILE:
			path = item->GetPath();
			break;
		default:
			ASSERT(false);
		}
		ShellExecuteWithAssocDialog(*AfxGetMainWnd(), path);
	}
	catch (CException *pe)
	{
		pe->ReportError();
		pe->Delete();
	}
}

void CDirstatDoc::RecurseRefreshMountPointItems(_In_ CItem *item)
{
	if ( item->GetType( ) == IT_DIRECTORY && item != GetRootItem( ) && GetApp( )->IsMountPoint( item->GetPath( ) ) ) {
		RefreshItem(item);
		}
	for ( int i = 0; i < item->GetChildrenCount( ); i++ ) {
		RecurseRefreshMountPointItems(item->GetChild(i));//ranged for?
		}
}

void CDirstatDoc::RecurseRefreshJunctionItems(_In_ CItem *item)
{
	if (item->GetType() == IT_DIRECTORY && item != GetRootItem() && GetApp()->IsJunctionPoint(item->GetPath())) {
		RefreshItem(item);
		}
	for (int i = 0; i < item->GetChildrenCount(); i++) {
		RecurseRefreshJunctionItems(item->GetChild(i));
		}
}

void CDirstatDoc::GetDriveItems(_Inout_ CArray<CItem *, CItem *>& drives)
{
	/*
	  Gets all items of type IT_DRIVE.
	*/
	drives.RemoveAll();

	CItem *root = GetRootItem( );
	
	if ( root == NULL ) {
		return;
		}
	if ( root->GetType( ) == IT_MYCOMPUTER ) {
		for (int i = 0; i < root->GetChildrenCount(); i++) {
			CItem *drive = root->GetChild( i );
			ASSERT( drive->GetType( ) == IT_DRIVE );
			drives.Add( drive );
			}
		}
	else if (root->GetType() == IT_DRIVE) {
		drives.Add( root );
		}
}


void CDirstatDoc::RebuildExtensionData()
{
	/*
	  The MAJORITY of draw time is spent in SortExtensionData!
	*/
	CWaitCursor wc;
	CStringArray sortedExtensions;

	m_extensionData.RemoveAll( );
	stdExtensionData.clear( );

	m_rootItem->RecurseCollectExtensionData( &m_extensionData );


	//m_rootItem->stdRecurseCollectExtensionData( stdExtensionData );

	SortExtensionData( sortedExtensions );
	SetExtensionColors( sortedExtensions );


	//at the moment, this is slightly, but consistently, faster ///but colors get fucked up
	//std::vector<CString> vector_sortedExtensions = stdSortExtData( sortedExtensions );
	//stdSetExtensionColors( vector_sortedExtensions );

	m_extensionDataValid = true;
}

std::vector<CString> CDirstatDoc::stdSortExtData( _In_ CStringArray& extensionsToSort) {
	std::vector<CString> sortedExtensions;

	POSITION pos = m_extensionData.GetStartPosition( );
	while ( pos != NULL ) {
		CString ext;
		SExtensionRecord r;
		m_extensionData.GetNextAssoc( pos, ext, r );
		sortedExtensions.push_back( ext );
		}
	
	//std::sort(sortedExtensions.begin(), sortedExtensions.end(), stdCompareExtensions );
	std::sort(sortedExtensions.begin(), sortedExtensions.end() );
#ifdef DEBUG
	for ( auto extension : sortedExtensions ) {
		TRACE( _T( "Extension: %s\r\n" ), extension );
		}
#endif
	return std::move( sortedExtensions );
	}

CExtensionData* CDirstatDoc::GetExtensionDataPtr( ) {
	return &m_extensionData;
	}

void CDirstatDoc::SortExtensionData( _Inout_ CStringArray& sortedExtensions)
{
	sortedExtensions.SetSize( m_extensionData.GetCount( ) );

	int i = 0;
	POSITION pos = m_extensionData.GetStartPosition( );
	while ( pos != NULL ) {
		CString ext;
		SExtensionRecord r;
		m_extensionData.GetNextAssoc( pos, ext, r );
		sortedExtensions[ i++ ] = ext;
		}

	_pqsortExtensionData = &m_extensionData;
	qsort( sortedExtensions.GetData( ), sortedExtensions.GetSize( ), sizeof( CString ), &_compareExtensions );
	_pqsortExtensionData = NULL;
}

void CDirstatDoc::SetExtensionColors(_In_ const CStringArray& sortedExtensions)
{
	static CArray<COLORREF, COLORREF&> colors;
	
	if (colors.GetSize() == 0) {
		CTreemap::GetDefaultPalette(colors);
		}

	for ( int i = 0; i < sortedExtensions.GetSize( ); i++ ) {
		COLORREF c = colors[ colors.GetSize( ) - 1 ];	
		
		if ( i < colors.GetSize( ) ) {
			c = colors[ i ];
			TRACE( _T( "Selected color %lu at position %i - i < colors.GetSize\tcolors.GetSize: %i\r\n\r\n" ), c, i, colors.GetSize( ) );
			}
#ifdef DEBUG
		else {
			TRACE( _T( "Selected color %lu at position %i\r\n" ), c, ( colors.GetSize( ) - 1 ) );
			}

		debuggingLogger aLog;
		aLog.iLessThan_Colors_GetSize = (i < colors.GetSize() ? true : false );
		aLog.iterator = i;
		aLog.color = c;
		aLog.extension = sortedExtensions[ i ];
		ColorExtensionSetDebugLog.push_back( aLog );
		TRACE( _T( "Setting extension %s (at %i) to color %lu\r\n" ), sortedExtensions[ i ], i, c );
#endif

		m_extensionData[ sortedExtensions[ i ] ].color = c;//typedef CMap<CString, LPCTSTR, SExtensionRecord, SExtensionRecord&> CExtensionData;
		//stdExtensionData[ sortedExtensions[ i ] ].color = c;
		}
#ifdef DEBUG
	traceOut_ColorExtensionSetDebugLog( );
#endif
}

#ifdef DEBUG
void CDirstatDoc::traceOut_ColorExtensionSetDebugLog( ) {
	DWORD averageColorSum = 0;
	std::vector<DWORD> uniqColors;
	for ( auto aSingleLog : ColorExtensionSetDebugLog ) {
		averageColorSum += aSingleLog.color;
		if ( isColorInVector( aSingleLog.color, uniqColors ) ) {
			
			}
		else {
			uniqColors.push_back( aSingleLog.color );
			}
		}
	DWORD averageColor = averageColorSum / ColorExtensionSetDebugLog.size( );
	TRACE( _T( "Average of all colors %lu\r\n" ), averageColor );
	TRACE( _T( "Known colors: \r\n" ) );
	for ( auto aColorValue : uniqColors ) {
		TRACE( _T( "\t%lu,\r\n" ), aColorValue );
		}
	TRACE( _T( "\r\n" ) );
	}

bool CDirstatDoc::isColorInVector( DWORD aColor, std::vector<DWORD>& colorVector ) {
	for ( auto aSingleColor : colorVector ) {
		if ( aSingleColor == aColor ) {
			return true;
			}
		}
	return false;
	}

#endif


void CDirstatDoc::stdSetExtensionColors( _In_ const std::vector<CString>& extensionsToSet ) {
	static CArray<COLORREF, COLORREF&> colors;
	if ( colors.GetSize() == 0 ) {
		CTreemap::GetDefaultPalette( colors );
		}
	//for ( auto extensionIterator = extensionsToSet.begin( ); extensionIterator != extensionsToSet.end( ); ++extensionIterator ) {
	//	}
	//TRACE( _T( "Setting color of %lu extensions....\r\n" ), extensionsToSet.size( ) );
	//auto sizeExts = extensionsToSet.size( );
	for ( auto i = 0; i < extensionsToSet.size(); ++i ) {
		COLORREF c = colors[ colors.GetSize( ) - 1 ];
		if ( i < colors.GetSize( ) ) {
			c = colors[ i ];
			}
		m_extensionData[ extensionsToSet[ i ] ].color = c;
		}
	}

CExtensionData *CDirstatDoc::_pqsortExtensionData;

int __cdecl CDirstatDoc::_compareExtensions(_In_ const void *item1, _In_ const void *item2) 
{
	CString *ext1 = (CString *)item1;
	CString *ext2 = (CString *)item2;
	SExtensionRecord r1;
	SExtensionRecord r2;
	VERIFY( _pqsortExtensionData->Lookup( *ext1, r1 ) );
	VERIFY( _pqsortExtensionData->Lookup( *ext2, r2 ) );
	return signum( r2.bytes - r1.bytes );
}

bool CDirstatDoc::stdCompareExtensions(_In_ const CString *stringOne, _In_ const CString *stringTwo ) {
	return stringOne > stringTwo;
	}

void CDirstatDoc::SetWorkingItemAncestor(_In_ CItem *item)
{
	if ( m_workingItem != NULL ) {
		SetWorkingItem( CItem::FindCommonAncestor( m_workingItem, item ) );
		}
	else {
		SetWorkingItem( item );
		}
}

void CDirstatDoc::SetWorkingItem(_In_ CItem *item)
{
	if ( GetMainFrame( ) != NULL ) {
		if ( item != NULL ) {
			GetMainFrame( )->ShowProgress( item->GetProgressRange( ) );
			}
		else {
			GetMainFrame( )->HideProgress( );
			}
		}
	m_workingItem = item;
}

bool CDirstatDoc::DeletePhysicalItem( _In_ CItem *item, _In_ const bool toTrashBin)
{
	/*
	Deletes a file or directory via SHFileOperation.
	Return: false, if canceled
	*/
	if ( CPersistence::GetShowDeleteWarning( ) ) {
		CDeleteWarningDlg warning;
		warning.m_fileName = item->GetPath( );
		if ( IDYES != warning.DoModal( ) ) {
			return false;
			}
		CPersistence::SetShowDeleteWarning( !warning.m_dontShowAgain );
		}

	ASSERT( item->GetParent( ) != NULL );

	CModalShellApi msa;
	msa.DeleteFile( item->GetPath( ), toTrashBin );

	RefreshItem( item );
	//CMainFrame::GetTheFrame()->UpdateRB( );
	return true;
}

void CDirstatDoc::SetZoomItem(_In_ CItem *item)
{
	if ( item == NULL ) {
		return;
		}
	m_zoomItem = item;
	UpdateAllViews( NULL, HINT_ZOOMCHANGED );
}

void CDirstatDoc::RefreshItem(_In_ CItem *item)
{
	/*
	Starts a refresh of an item.
	If the physical item has been deleted,
	updates selection, zoom and working item accordingly.	
	*/
	ASSERT( item != NULL );

	CWaitCursor wc;
	ClearReselectChildStack( );

	if ( item->IsAncestorOf( GetZoomItem( ) ) ) {
		SetZoomItem( item );
		}
	if ( item->IsAncestorOf( GetSelection( ) ) ) {
		SetSelection( item );
		UpdateAllViews( NULL, HINT_SELECTIONCHANGED );
		}

	SetWorkingItemAncestor( item );
	CItem *parent = item->GetParent( );

	if ( !item->StartRefresh( ) ) {
		if ( GetZoomItem( ) == item ) {
			SetZoomItem( parent );
			}
		if ( GetSelection( ) == item ) {
			SetSelection( parent );
			UpdateAllViews( NULL, HINT_SELECTIONCHANGED );
			}
		if ( m_workingItem == item ) {
			SetWorkingItem( parent );
			}
		}
	UpdateAllViews( NULL );
}

void CDirstatDoc::PushReselectChild(CItem *item)
{
	m_reselectChildStack.AddHead(item);
}

CItem *CDirstatDoc::PopReselectChild()
{
	return m_reselectChildStack.RemoveHead();
}

void CDirstatDoc::ClearReselectChildStack()
{
	m_reselectChildStack.RemoveAll();
}

bool CDirstatDoc::IsReselectChildAvailable() const
{
	return !m_reselectChildStack.IsEmpty();
}

bool CDirstatDoc::DirectoryListHasFocus() const
{
	return (GetMainFrame()->GetLogicalFocus() == LF_DIRECTORYLIST);
}

BEGIN_MESSAGE_MAP(CDirstatDoc, CDocument)
	ON_COMMAND(ID_REFRESHSELECTED, OnRefreshselected)
	ON_UPDATE_COMMAND_UI(ID_REFRESHSELECTED, OnUpdateRefreshselected)
	ON_COMMAND(ID_REFRESHALL, OnRefreshall)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//ON_COMMAND(ID_CLEANUP_EMPTYRECYCLEBIN, OnCleanupEmptyrecyclebin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWFREESPACE, OnUpdateViewShowfreespace)
	ON_COMMAND(ID_VIEW_SHOWFREESPACE, OnViewShowfreespace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWUNKNOWN, OnUpdateViewShowunknown)
	ON_COMMAND(ID_VIEW_SHOWUNKNOWN, OnViewShowunknown)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_SELECTPARENT, OnUpdateTreemapSelectparent)
	ON_COMMAND(ID_TREEMAP_SELECTPARENT, OnTreemapSelectparent)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMIN, OnUpdateTreemapZoomin)
	ON_COMMAND(ID_TREEMAP_ZOOMIN, OnTreemapZoomin)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_ZOOMOUT, OnUpdateTreemapZoomout)
	ON_COMMAND(ID_TREEMAP_ZOOMOUT, OnTreemapZoomout)
	ON_UPDATE_COMMAND_UI(ID_REFRESHALL, OnUpdateRefreshall)
	ON_UPDATE_COMMAND_UI(ID_TREEMAP_RESELECTCHILD, OnUpdateTreemapReselectchild)
	ON_COMMAND(ID_TREEMAP_RESELECTCHILD, OnTreemapReselectchild)
END_MESSAGE_MAP()


void CDirstatDoc::OnUpdateRefreshselected(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( DirectoryListHasFocus( ) && GetSelection( ) != NULL && GetSelection( )->GetType( ) != IT_FREESPACE && GetSelection( )->GetType( ) != IT_UNKNOWN );
}

void CDirstatDoc::OnRefreshselected()
{
	RefreshItem( GetSelection( ) );
	//CMainFrame::GetTheFrame( )->UpdateRB( );
}

void CDirstatDoc::OnUpdateRefreshall(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetRootItem() != NULL);
}

void CDirstatDoc::OnRefreshall()
{
	RefreshItem( GetRootItem( ) );
	//CMainFrame::GetTheFrame( )->UpdateRB( );
}

void CDirstatDoc::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	const CItem *item = GetSelection( );
	auto thisItemType = item->GetType( );
	pCmdUI->Enable( DirectoryListHasFocus( ) && item != NULL && thisItemType != IT_MYCOMPUTER && thisItemType != IT_FILESFOLDER && thisItemType != IT_FREESPACE && thisItemType != IT_UNKNOWN );
}

void CDirstatDoc::OnEditCopy()
{
	TRACE( _T( "User chose 'Edit'->'Copy'!\r\n") );
	const CItem *item = GetSelection( );
	ASSERT( item != NULL );
	ASSERT( item->GetType( ) == IT_DRIVE || item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE );

	GetMainFrame( )->CopyToClipboard( item->GetPath( ) );
}


void CDirstatDoc::OnUpdateViewShowfreespace(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_showFreeSpace);
}

void CDirstatDoc::OnViewShowfreespace()
{
	CArray<CItem *, CItem *> drives;
	GetDriveItems(drives);

	if ( m_showFreeSpace ) {
		for (int i = 0; i < drives.GetSize(); i++) { //ranged for?
			CItem *free = drives[i]->FindFreeSpaceItem();
			ASSERT(free != NULL);
		
			if ( GetSelection( ) == free ) {
				SetSelection( free->GetParent( ) );
				}
			if ( GetZoomItem( ) == free ) {
				m_zoomItem = free->GetParent( );
				}
			drives[ i ]->RemoveFreeSpaceItem( );
			}
		m_showFreeSpace = false;
		}
	else {
		for ( int i = 0; i < drives.GetSize( ); i++ ) { //ranged for?
			drives[ i ]->CreateFreeSpaceItem( );
			}
		m_showFreeSpace = true;
		}

	if ( drives.GetSize( ) > 0 ) {
		SetWorkingItem( GetRootItem( ) );
		}
	UpdateAllViews(NULL);
}

void CDirstatDoc::OnUpdateViewShowunknown(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_showUnknown);
}

void CDirstatDoc::OnViewShowunknown()
{
	CArray<CItem *, CItem *> drives;
	GetDriveItems(drives);

	if (m_showUnknown) {
		for (int i = 0; i < drives.GetSize(); i++) {
			CItem *unknown = drives[i]->FindUnknownItem();
			ASSERT(unknown != NULL);
		
			if ( GetSelection( ) == unknown ) {
				SetSelection( unknown->GetParent( ) );
				}
			if ( GetZoomItem( ) == unknown ) {
				m_zoomItem = unknown->GetParent( );
				}
			drives[ i ]->RemoveUnknownItem( );
			}
		m_showUnknown = false;
		}
	else {
		for ( int i = 0; i < drives.GetSize( ); i++ ) {
			drives[ i ]->CreateUnknownItem( );
			}
		m_showUnknown = true;
		}

	if ( drives.GetSize( ) > 0 ) {
		SetWorkingItem( GetRootItem( ) );
		}
	//CMainFrame::GetTheFrame( )->UpdateRB( );
	UpdateAllViews(NULL);
}

void CDirstatDoc::OnUpdateTreemapZoomin(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( ( m_rootItem != NULL ) && ( m_rootItem->IsDone( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( ) != GetZoomItem( ) ) );
}

void CDirstatDoc::OnTreemapZoomin()
{
	CItem *p = GetSelection( );
	CItem *z = NULL;
	auto zoomItem = GetZoomItem( );
	while (p != zoomItem) {
		z = p;
		p = p->GetParent( );
		}
	if ( z == NULL ) {
		return;
		}
	else {
		ASSERT( z != NULL );
		SetZoomItem( z );
		}
}

void CDirstatDoc::OnUpdateTreemapZoomout(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( ( m_rootItem != NULL ) && ( m_rootItem->IsDone( ) ) && ( GetZoomItem( ) != m_rootItem ) );
}

void CDirstatDoc::OnTreemapZoomout()
{
	SetZoomItem( GetZoomItem( )->GetParent( ) );
}

void CDirstatDoc::OnUpdateExplorerHere(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( )->GetType( ) != IT_FREESPACE ) && ( GetSelection( )->GetType( ) != IT_UNKNOWN ) );
}

void CDirstatDoc::OnExplorerHere()
{
	try
	{
		
		const CItem *item = GetSelection( );
		ASSERT( item != NULL );
		TRACE( _T( "User wants to open Explorer in %s!\r\n"), item->GetFolderPath( ) );
		if (item->GetType() == IT_MYCOMPUTER) {
			SHELLEXECUTEINFO sei;
			sei.cbSize = NULL;
			sei.dwHotKey = NULL;
			sei.fMask = NULL;
			sei.hIcon = NULL;
			sei.hInstApp = NULL;
			sei.hkeyClass = NULL;
			sei.hMonitor = NULL;
			sei.hProcess = NULL;
			sei.hwnd = NULL;
			sei.lpClass = NULL;
			sei.lpDirectory = NULL;
			sei.lpFile = NULL;
			sei.lpIDList = NULL;
			sei.lpParameters = NULL;
			sei.lpVerb = NULL;
			sei.nShow = NULL;
			
			sei.cbSize = sizeof( sei );
			sei.hwnd = *AfxGetMainWnd( );
			sei.lpVerb = _T( "explore" );
			sei.nShow  = SW_SHOWNORMAL;
			
			CCoTaskMem<LPITEMIDLIST> pidl;
			GetPidlOfMyComputer( &pidl );
		
			sei.lpIDList = pidl;
			sei.fMask   |= SEE_MASK_IDLIST;

			ShellExecuteEx( &sei );
			// ShellExecuteEx seems to display its own Messagebox on error.
			}
		else {
			MyShellExecute( *AfxGetMainWnd( ), _T( "explore" ), item->GetFolderPath( ), NULL, NULL, SW_SHOWNORMAL );
			}
	}
	catch (CException *pe)
	{
		pe->ReportError( );
		pe->Delete( );
	}
}

void CDirstatDoc::OnUpdateCommandPromptHere(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( GetSelection( ) != NULL ) && ( GetSelection( )->GetType( ) != IT_MYCOMPUTER ) && ( GetSelection( )->GetType( ) != IT_FREESPACE ) && ( GetSelection( )->GetType( ) != IT_UNKNOWN ) && ( !( GetSelection( )->HasUncPath( ) ) ) );
}

void CDirstatDoc::OnCommandPromptHere()
{
	try
	{
		CItem *item = GetSelection( );
		ASSERT( item != NULL );
		TRACE( _T( "User wants to open a command prompt in %s!\r\n" ), item->GetFolderPath( ) );
		CString cmd = GetCOMSPEC( );

		MyShellExecute( *AfxGetMainWnd( ), _T( "open" ), cmd, NULL, item->GetFolderPath( ), SW_SHOWNORMAL );
	}
	catch (CException *pe)
	{
		pe->ReportError( );
		pe->Delete( );
	}
}

void CDirstatDoc::OnUpdateCleanupDeletetotrashbin(CCmdUI *pCmdUI)
{
	CItem *item = GetSelection( );
	
	pCmdUI->Enable( ( DirectoryListHasFocus( ) ) && ( item != NULL ) && ( item->GetType( ) == IT_DIRECTORY || item->GetType( ) == IT_FILE ) && ( !( item->IsRootItem( ) ) ) );

}

void CDirstatDoc::OnCleanupDeletetotrashbin()
{
	CItem *item = GetSelection();
	
	if ( item == NULL || item->GetType( ) != IT_DIRECTORY && item->GetType( ) != IT_FILE || item->IsRootItem( ) ) {
		return;
		}

	if ( DeletePhysicalItem( item, true ) ) {
		UpdateAllViews( NULL );
		}
}

void CDirstatDoc::OnUpdateCleanupDelete(CCmdUI *pCmdUI)
{
	CItem *item = GetSelection();
	
	pCmdUI->Enable( ( DirectoryListHasFocus() ) && ( item != NULL ) && ( item->GetType() == IT_DIRECTORY || item->GetType() == IT_FILE ) && ( !(item->IsRootItem()) ) );
}

void CDirstatDoc::OnCleanupDelete()
{
	CItem *item = GetSelection( );
	
	if ( item == NULL || item->GetType( ) != IT_DIRECTORY && item->GetType( ) != IT_FILE || item->IsRootItem( ) ) {
		return;
		}

	if ( DeletePhysicalItem( item, false ) ) {
		SetWorkingItem( GetRootItem( ) );
		UpdateAllViews( NULL );
		}
}
void CDirstatDoc::OnUpdateTreemapSelectparent(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( ( GetSelection() != NULL ) && ( GetSelection()->GetParent() != NULL ) );
}

void CDirstatDoc::OnTreemapSelectparent()
{
	PushReselectChild( GetSelection( ) );
	CItem *p = GetSelection( )->GetParent( );
	SetSelection( p, true );
	UpdateAllViews( NULL, HINT_SHOWNEWSELECTION );
}

void CDirstatDoc::OnUpdateTreemapReselectchild(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsReselectChildAvailable( ) );
}

void CDirstatDoc::OnTreemapReselectchild()
{
	CItem *item = PopReselectChild( );
	SetSelection( item, true );
	UpdateAllViews( NULL, HINT_SHOWNEWSELECTION );
}


// CDirstatDoc Diagnostics
#ifdef _DEBUG
void CDirstatDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDirstatDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG



// $Log$
// Revision 1.14  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.13  2004/11/07 10:17:37  bseifert
// Bugfix: Recursive UDCs must not follow junction points.
//
// Revision 1.12  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
