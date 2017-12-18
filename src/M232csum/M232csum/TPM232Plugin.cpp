//--------------------------------------------------------------------------------------
// File:    TPChecksumPluginSample.cpp
// Desc:    This file implements a simple checksum plugin-in DLL. It can be used as a
//          base-line for your own plug-in, or as an educational example of the plug-in
//          architecture.
//
//          Documentation should have been distributed with the TunerPro Developer SDK.
//          Also make sure to study ITPPlugin.h for more information on the plug-in
//          architecture.
//
// Copyright (c) Mark Mansur. All rights reserved. This source code is *not*
// redistributable, and may be obtained only by permission from its author, Mark Mansur.
// For information, please see www.tunerpro.net, or contact mark@tunerpro.net.
//--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "rpc.h" // for GUID used for unique identifier
#include "MMChecksumPlugin.h"

// We'll use this to return a meaningful error message to the application upon request
CHAR g_strLastError[1024];

//---------------------------------------------------------------------------------------
// Every checksum plug-in needs to have a unique identifier. This identifier allows
// TunerPro to determine if the plug-in has already been loaded. Every time you create
// a new checksum plug-in DLL, you should create a new GUID to uniquely identify it.
//---------------------------------------------------------------------------------------
#define PLUGIN_GUID     "E9D30EED-CDA3-40DD-BDC2-1C5A8CBD51B4"

//---------------------------------------------------------------------------------------
// A single checksum plug-in can implement many different types of checksums and checksum
// calculations. This sample will implement 2 different kinds. The application will 
// reference the checksum by index, so we'll define those indexes here.
//---------------------------------------------------------------------------------------
enum 
{
    CHECKSUM_M232,
    CHECKSUM_COUNT
};

//
//---------------------------------------------------------------------------------------
//  There are four C functions that need to be exported from the DLL in order for 
//  TunerPro to successfully load and interact with the plug-in DLL. They are:
// 
//  HRESULT MMCSGetChecksumPluginInfo(MMCHECKSUMPLUGININFO* pPluginInfoStruct);
//  HRESULT MMCSGetChecksumInfo(DWORD dwIndex, MMCHECKSUMINFO* pCSInfoStruct);
//  HRESULT MMCSCalculateChecksum(MMCHECKSUMCALC* pCalcInfo);
//  HRESULT MMCSGetLastErrorMessage(CHAR* strTextOut, DWORD cbTextOut);
//
// We'll implement them here. To tell the compiler that we want to export these
// functions from the DLL, they are prefaced with __declspec(dllexport). To tell the
// compiler to treat them as plain C (rather than C++), they're also prefaced with
// extern "C".
//---------------------------------------------------------------------------------------
//

//---------------------------------------------------------------------------------------
// Func: MMCSgetChecksumPluginInfo (Exported)
// Desc: The application will call this function to gather information about the 
//       plug-in module. The application will use the returned info to make use of the
//       plug-in. Some of the info returned by the plug-in may be displayed to the user.
//---------------------------------------------------------------------------------------
extern "C" __declspec(dllexport)
HRESULT MMCSGetChecksumPluginInfo(MMCHECKSUMPLUGININFO* pPluginInfoStruct)
{
    HRESULT hr = S_OK;

    // Clear the last error text
    g_strLastError[0] = '\0';

    // The application will pass in a structure for us to fill in. Let's check the size of
    // the structure the app passed, just to make sure we're on the same version
    if ( !pPluginInfoStruct )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if ( pPluginInfoStruct->cbStructSize != sizeof(MMCHECKSUMPLUGININFO) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_VERSION);
        goto Exit;
    }

    // Uniquely identify this plug-in DLL
    UuidFromStringA((RPC_CSTR)PLUGIN_GUID, &pPluginInfoStruct->ID);
    // Tell the application what contract version we're implementing
    pPluginInfoStruct->dwContractVersion = MMCS_CONTRACT_VERSION;
    // Tell the application how many checksum types we're implementing
    pPluginInfoStruct->dwChecksumCount = CHECKSUM_COUNT;
    // User friendly version string
    StringCbCopyA(pPluginInfoStruct->strVersion, ARRAYSIZE(pPluginInfoStruct->strVersion),
        "1.0.0");
    // Info about the author
    StringCbCopyA(pPluginInfoStruct->strAuthor, ARRAYSIZE(pPluginInfoStruct->strAuthor),
        "prj (www.prj-tuning.com)");
    // User-friendly name of the module
    StringCbCopyA(pPluginInfoStruct->strName, ARRAYSIZE(pPluginInfoStruct->strName),
        "M2.3.2 TunerPro Checksum Plug-in");
    // User-friendly description of the module
    StringCbCopyA(pPluginInfoStruct->strDesc, ARRAYSIZE(pPluginInfoStruct->strDesc),
        "AAN/ABY/ADU");

Exit:
    return hr;
}



//---------------------------------------------------------------------------------------
// Func: MMCSGetChecksumInfo (Exported)
// Desc: 
//---------------------------------------------------------------------------------------
extern "C" __declspec(dllexport)
HRESULT MMCSGetChecksumInfo(DWORD dwIndex, MMCHECKSUMINFO* pCSInfoStruct)
{
    HRESULT hr = S_OK;

    // Clear the last error text
    g_strLastError[0] = '\0';

    if ( !pCSInfoStruct )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if ( pCSInfoStruct->cbStructSize != sizeof(MMCHECKSUMINFO) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_VERSION);
        goto Exit;
    }

    //
    // Return info about the checksum
    //
    switch ( dwIndex )
    {
    case CHECKSUM_M232:
        StringCbCopyA(pCSInfoStruct->strName, ARRAYSIZE(pCSInfoStruct->strName),
            "M2.3.2 Checksum");
        StringCbCopyA(pCSInfoStruct->strDesc, ARRAYSIZE(pCSInfoStruct->strDesc),
            "Auto detect boost/fuel chip.");
        StringCbCopyA(pCSInfoStruct->strVersion, ARRAYSIZE(pCSInfoStruct->strVersion),
            "SUM1.0");
        // No specific data size expected
        pCSInfoStruct->cbExpectedDataSize = 0;
        break;
    default:
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        break;
    }

Exit:
    return hr;
}



//---------------------------------------------------------------------------------------
// Func: MMCSCalculateChecksum (Exported)
// Desc: The application will call this function to calculate a checksum. The structure
//       passed in gives the plug-in all of the info necessary to do the work, including 
//       the index of the checksum calculation to use. Note that some of the info can be
//       ignored by the plug-in if it isn't needed. More complex checksum calculations,
//       for instance, may not use some of the information passed in.
//---------------------------------------------------------------------------------------
extern "C" __declspec(dllexport)
HRESULT MMCSCalculateChecksum(MMCHECKSUMCALC* pCalcInfo)
{
    HRESULT hr = S_OK;

    // Clear the last error text
    g_strLastError[0] = '\0';

    if ( !pCalcInfo )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if ( pCalcInfo->cbStructSize != sizeof(MMCHECKSUMCALC) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_VERSION);
        goto Exit;
    }

    // Verify that we have data to work on
    if ( !pCalcInfo->pBaseData || !pCalcInfo->cbBaseData )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    //
    // Calculate and store the checksum. The caller tells us which 
    // checksum calculator to use.
    //
    switch ( pCalcInfo->dwChecksumIndex )
    {

	case CHECKSUM_M232:
		{
			if (pCalcInfo->cbBaseData == (1024*64)) {
				// Fuel chip, checksum range 0x0000-0xFEFF, stored at 0xFF00/0xFF01. Sum No carry.
				WORD csum = 0;
				for (UINT ui = 0; ui <= 0xFEFF; ui++) {
					csum += pCalcInfo->pBaseData[ui];
				}
				pCalcInfo->pBaseData[0xFF00] = csum / 256;
				pCalcInfo->pBaseData[0xFF01] = csum % 256;
			} else if (pCalcInfo->cbBaseData == (1024*32)) {
				// Boost chip, checksum range 0x0000-0x3FFF, stored at 0x3FFA, 0x3FFB. Also double stack automatically.
				WORD csum = 0;
				for (UINT ui = 0; ui < 0x3FFA; ui++) {
					csum += pCalcInfo->pBaseData[ui];
				}
				csum += pCalcInfo->pBaseData[0x3FFE];
				csum += pCalcInfo->pBaseData[0x3FFF];
				csum += 0xFF;
				csum += 0xFF;

				pCalcInfo->pBaseData[0x3FFA] = csum / 256;
				pCalcInfo->pBaseData[0x3FFB] = csum % 256;
				pCalcInfo->pBaseData[0x3FFC] = 0xFF - (csum / 256);
				pCalcInfo->pBaseData[0x3FFD] = 0xFF - (csum % 256);

				for (UINT ui = 0x4000; ui < 0x8000; ui++) {
					pCalcInfo->pBaseData[ui] = pCalcInfo->pBaseData[ui-0x4000];
				}
			} else {
                hr = HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
                StringCbCopyA(g_strLastError, ARRAYSIZE(g_strLastError),
                    "Expected data size is 32KB (boost) or 64KB (fuel)");
            }
		}
        break;

    default:
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        StringCbCopyA(g_strLastError, ARRAYSIZE(g_strLastError),
            "Invalid Checksum Index");
        break;
    }

Exit:
    return hr;
}

//---------------------------------------------------------------------------------------
// Func: MMCSGetLastErrorMessage (Exported)
// Desc: When an error occurs in the plug-in, the plug-in can format a user friendly 
//       error string. The application may request this string to give the user more
//       information on what went wrong. It generally only makes sense to format an 
//       error string for failures that are a result of the definition author or
//       end user.
//---------------------------------------------------------------------------------------
extern "C" __declspec(dllexport)
HRESULT MMCSGetLastErrorMessage(CHAR* strTextOut, DWORD cbTextOut)
{
    HRESULT hr = S_OK;
    StringCbCopyA(strTextOut, cbTextOut, g_strLastError);
    return hr;
}