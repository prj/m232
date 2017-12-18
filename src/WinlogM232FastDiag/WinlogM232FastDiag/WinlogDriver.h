#include "stdio.h"
#include "stdlib.h"
#include "Windows.h"

#define WINLOG_API extern "C" __declspec(dllexport)

typedef struct
{
	char port[6];
	double mapOffset;
	double mapMult;
} M232_CONFIG;

LRESULT CALLBACK DlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void connectLogger(char* port, HANDLE exitHandle, void (*callbackFunc)(unsigned char*, int));