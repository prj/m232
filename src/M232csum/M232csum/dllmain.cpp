// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        // This will be hit when the application loads your checksum DLL. Handle
        // any necessary initialization here.
        break;

	case DLL_PROCESS_DETACH:
        // This will be hit when the application unloads your checksum DLL. Do
        // any necessary uninitialization here.
		break;
	}
	return TRUE;
}

