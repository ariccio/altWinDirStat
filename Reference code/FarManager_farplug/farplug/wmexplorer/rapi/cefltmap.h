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
/****************************************************************************
*                                                                           *
* Cefilt.h -- Windows CE Services filter procedure declarations,            *
*             structures, constant definitions and macros mapping.          *
*                                                                           *
* Copyright (c) Microsoft Corporation. All rights reserved.		            *
*                                                                           *
****************************************************************************/

#ifndef _CEFLTMAP_
#define _CEFLTMAP_

// {6C5C05E0-97A2-11cf-8011-00A0C90A8F78}
DEFINE_GUID(IID_IPegasusFileFilterSite, 
0x6c5c05e0, 0x97a2, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);
// {6C5C05E1-97A2-11cf-8011-00A0C90A8F78}
DEFINE_GUID(IID_IPegasusFileFilter, 
0x6c5c05e1, 0x97a2, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);
// {6C5C05E2-97A2-11cf-8011-00A0C90A8F78}
DEFINE_GUID(IID_IPegasusFileFilterOptions, 
0x6c5c05e2, 0x97a2, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);


#define HRESULT_TO_PFERROR    HRESULT_TO_CFERROR

typedef CF_ERROR              PF_ERROR;
typedef CFF_DESTINATIONFILE   PFF_DESTINATIONFILE; 
typedef CFF_CONVERTINFO       PFF_CONVERTINFO; 
typedef CFF_SOURCEFILE        PFF_SOURCEFILE;

#define PF_OPENFLAT           CF_OPENFLAT     
#define PF_OPENCOMPOUND       CF_OPENCOMPOUND 
#define PF_OPENDONT           CF_OPENDONT     
#define PF_OPENASKMEHOW       CF_OPENASKMEHOW 

typedef CFF_CONVERTOPTIONS    PFF_CONVERTOPTIONS;

#define IPegasusFileFilterSite      ICeFileFilterSite
#define IPegasusFileFilter          ICeFileFilter
#define IPegasusFileFilterOptions   ICeFileFilterOptions

#endif /* !_CEFLTMAP_ */

