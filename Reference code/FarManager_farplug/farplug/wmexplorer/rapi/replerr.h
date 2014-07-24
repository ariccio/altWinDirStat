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
* replerr.h -- Pegasus filter error codes                                   *
*                                                                           *
* Copyright (c) Microsoft Corporation. All rights reserved.		            *
*                                                                           *
****************************************************************************/

#ifndef _REPLERR_
#define _REPLERR_

/*
 * Define how errors are declared
 */
#define CF_DECLARE_ERROR(e)  (0x80040000 | e)
#define PF_DECLARE_ERROR(e)  CF_DECLARE_ERROR(e)

/*
 * Predefined error messages
 */
#define ERROR_ALREADYCONVERTING  CF_DECLARE_ERROR(0x5000)  // conversion is not reentrant
#define ERROR_UNKNOWNCONVERSION  CF_DECLARE_ERROR(0x5001)  // conversion is not recognized by converter dll
#define ERROR_BADFILE            CF_DECLARE_ERROR(0x5002)  // generic error that indicates that the format of a file was not understood

#endif /* !_REPLERR_ */
