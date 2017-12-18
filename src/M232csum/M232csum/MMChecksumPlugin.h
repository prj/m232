//--------------------------------------------------------------------------------------
// File:    TPChecksumPlugin.h
// Desc:    Defines TunerPro RT checksum plug-in functions and structures.
//
// Hist:    12/8/10 - Initial version
//
// Copyright (c) Mark Mansur. All rights reserved. This source code is *not*
// redistributable, and may be obtained only by permission from its author, Mark Mansur.
// For information, please see www.tunerpro.net, or contact mark@tunerpro.net.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef _ITPCHECKSUMPLUGIN_H
#define _ITPCHECKSUMPLUGIN_H

//---------------------------------------------------------------------------------------
// The version of the client contract defined by this header. If breaking changes are
//  made in the future, this version will be incremented, and the application can choose
//  not to load the plug-in module after inspecting this version number.
//---------------------------------------------------------------------------------------
#define MMCS_CONTRACT_VERSION       1

//---------------------------------------------------------------------------------------
// Max string lengths
//---------------------------------------------------------------------------------------
#define MMCS_MAX_NAME               50
#define MMCS_MAX_DESC               512
#define MMCS_MAX_VERSION            15
#define MMCS_MAX_AUTHOR             100

//---------------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------------
struct MMCHECKSUMPLUGININFO
{
    DWORD cbStructSize;                     // Size of this structure, set by the caller
    DWORD dwContractVersion;                // Plug-in should set to MMCS_CONTRACT_VERSION
    GUID  ID;                               // A unique identifier for the plug-in
    DWORD dwChecksumCount;                  // The number of checksum types that this plug-in implements
    CHAR  strName[MMCS_MAX_NAME];           // User friendly name for the plug-in module
    CHAR  strDesc[MMCS_MAX_DESC];           // User friendly description for the plug-in module
    CHAR  strAuthor[MMCS_MAX_AUTHOR];       // User friendly info about the author of the plug-in module
    CHAR  strVersion[MMCS_MAX_VERSION];     // User friendly version string
};

//---------------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------------
struct MMCHECKSUMINFO
{
    DWORD cbStructSize;                     // Size of this structure, set by the caller
    CHAR  strName[MMCS_MAX_NAME];           // Friendly name of the checksum (e.g. "2001 Ferrari 360 Checksum")
    CHAR  strDesc[MMCS_MAX_DESC];           // Details of the checksum
    CHAR  strVersion[MMCS_MAX_VERSION];     // User-friendly version string
    DWORD cbExpectedDataSize;               // Size of the data expected to be passed, or 0 no expectation, -1 for various.
};

//---------------------------------------------------------------------------------------
// Data Type flags
//---------------------------------------------------------------------------------------
#define MMCS_TYPE_SIGNED        0x00000001
#define MMCS_TYPE_LSBFIRST      0x00000002
#define MMCS_TYPE_FLOATINGPOINT 0x00000004

struct MMCHECKSUMCALC
{
    DWORD cbStructSize;                     // Size of the structure, set by caller
    DWORD dwChecksumIndex;                  // The 0-based index of the checksum to use in the plug-in module
    BYTE* pBaseData;                        // Pointer to beginning of buffer
    DWORD cbBaseData;                       // Size of the buffer pointed to by pBaseData.
    DWORD dwRegionStartAddress;             // The start address within pBaseData of the region to calculate
    DWORD dwRegionEndAddress;               // The end address within pBaseData of the region to calculate
    DWORD dwChecksumFlags;                  // Flags (if needed)
    DWORD dwStoreAddress;                   // The store address for the checksum (if needed)
    DWORD dwStoreSizeBits;                  // The size of the checksum to be stored (if needed)
    DWORD dwStoreTypeFlags;                 // LSB First, signed, floating point, etc (if needed)
    DWORD dwElementSizeBits;                // The size of each data block (if needed)
    DWORD dwElementTypeFlags;               // LSB first, signed, floating point, etc (if needed)
    VOID* pUserData;                        // Pointer to additional data to be utilized by module
    DWORD cbUserdata;                       // Size of additional data, in bytes, to be utilized by module
};

//---------------------------------------------------------------------------------------
// Extern "C" functions that the checksum DLL must implement in order to be recognized by
// TunerPro. These are provided for example purposes, and are only defined in your plug-in
// DLL, hence the commented declarations.
//---------------------------------------------------------------------------------------
//extern "C" HRESULT MMCSGetChecksumPluginInfo(MMCHECKSUMPLUGININFO* pPluginInfoStruct);
//extern "C" HRESULT MMCSGetChecksumInfo(DWORD dwIndex, MMCHECKSUMINFO* pCSInfoStruct);
//extern "C" HRESULT MMCSCalculateChecksum(MMCHECKSUMCALC* pCalcInfo);
//extern "C" HRESULT MMCSGetLastErrorMessage(CHAR* strTextOut, DWORD cbTextOut);

#endif // _ITPCHECKSUMPLUGIN_H