/*
Winlog driver for M232
*/

#include "WinlogDriver.h"
#include "Windows.h"
#include "resource.h"
#include <string>
#include <iostream>
#include <math.h>
#include <strsafe.h>
using namespace std;

M232_CONFIG config = {
	"COM1",
	6.9558,
	82.6105
};
HANDLE hInst = NULL;
int lastUpdate = GetTickCount();
int counter = 0;

// The display names for each of the fields that will be sent back and displayed to the user.
// Change this to describe your fields

char *fieldnames[]=
{
	"N75",
	"N75_REQ",
	"MAP",
	"MAP_REQ",
	"KNOCK1",
	"KNOCK2",
	"KNOCK3",
	"KNOCK4",
	"KNOCK5",
	"KNOCKGLOB",
	"KNOCKDYN",
	"RPM",
	"LOAD",
	"UBATT",
	"IAT",
	"ECT",
	"TPS",
	"IGN",
	"IPW_EFF",
	"TVUB",
	"IPW_RAW",
	"IDC",
	"VSS",
	"MAF_T",
	"MAF_C",
	"AFR_REQ",
	"KNOCKOUT1",
	"KNOCKOUT2",
	"KNOCKOUT3",
	"KNOCKOUT4",
	"KNOCKOUT5",
	"KNOCKOUTAVG",
	"IGNOUT1",
	"IGNOUT2",
	"IGNOUT3",
	"IGNOUT4",
	"IGNOUT5",
	"RATE"
};

// The number of fields that we'll be sending back to WinLog, this is normally a count
// of the variables defined in the "FIELDS" structure above. Change this to match
// the number of fields defined above.
#define NUM_FIELDS 38

float fielddata[NUM_FIELDS];

// prototype for the callback to Winlog, initialized below in the Initialize function, you shouldn't
// need to change this!
int (__cdecl *notifycallback)(unsigned char notification, void *hint, void *hint2) = NULL;
void *notifyhint = NULL;


// Some object handles to notify our driver thread of events
HANDLE m_hThreadTerm = NULL;
HANDLE m_hThreadStarted = NULL;
HANDLE m_hThreadStoppedEvent = NULL;


// The main entry point to our driver
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	hInst = hModule;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

// SetConfig
// This API is called by Winlog to notify the driver of it's configuration, no actual 
// reconfiguration of the device should be done here, instead, just copy the configuration
// to your local structure. Usually, this code won't need to be changed.
WINLOG_API void SetConfig(void *pConfig)
{
	memcpy(&config, pConfig, sizeof(M232_CONFIG));
}

// GetConfig
// This API is called by Winlog to get the current configuration of the driver.
// Return a pointer to the configuration data.
// Usually, this code won't need to be changed.
WINLOG_API void *GetConfig(void)
{
	return (void*)&config;
}

// GetConfigSize
// This API is called by winlog to determine the size of the configuration data that the
// driver needs to be stored.
// Return the size of the configuration data in bytes.
// Usually, this code won't need to be changed.
WINLOG_API int GetConfigSize(void)
{
	return sizeof(M232_CONFIG);
}

// Calculates and sets data
void setData(unsigned char* buf, int len) {

	// Boost chip
	fielddata[0] = buf[0]/194.0*100; //N75
	fielddata[1] = buf[1]/194.0*100; //N75 base
	fielddata[2] = buf[2]/255.0*5/1.03515625*config.mapMult + config.mapOffset; //MAP
	fielddata[3] = buf[3]/255.0*5/1.03515625*config.mapMult + config.mapOffset; //MAP REQ
	fielddata[4] = buf[4]*0.5; // KNOCK1
	fielddata[5] = buf[5]*0.5; // KNOCK2
	fielddata[6] = buf[6]*0.5; // KNOCK3
	fielddata[7] = buf[7]*0.5; // KNOCK4
	fielddata[8] = buf[8]*0.5; // KNOCK5
	fielddata[9] = buf[9]*0.5; // KNOCKGLOB
	fielddata[10] = buf[10]*0.5; // KNOCKDYN

	// Fuel chip

	fielddata[11] = (buf[31])*40; //RPM
	fielddata[12] = (buf[30]*256+buf[29])/25.0;  //LOAD
	fielddata[13] = (buf[28])*0.068;  //Battery voltage
	fielddata[14] = ((buf[27])-70.0)*0.7;  //IAT
	fielddata[15] = ((buf[26])-70.0)*0.7;  //ECT
	fielddata[16] = (buf[25])*0.416;  //TPS
	fielddata[17] = 0.75*((buf[24])-(buf[23]));  //Timing request
	double ipw = 0.52*((buf[22]) + (buf[21])/256.0);
	fielddata[18] = ipw;  //IPW (effective)
	fielddata[19] = (buf[20])*0.010667; // Injector dead time
	fielddata[20] = ipw + fielddata[19]; // IPW (actual)
	fielddata[21] = fielddata[20]*fielddata[11]/1200.0; // IDC
	fielddata[22] = buf[19]*2;  //VSS
	fielddata[23] = buf[18];  //MAF table number
	fielddata[24] = buf[17];  //MAF table cell

	// Target AFR
	double calcvalue = (buf[15]*256+buf[14])*pow(2.0, buf[13]);
	unsigned char injconstant  = buf[16] == 0 ? 1 : buf[16];
	calcvalue = (128.0/injconstant)*calcvalue;
	calcvalue = calcvalue == 0 ? 1 : calcvalue;
	calcvalue = 1/(calcvalue/65536.0)*14.7;
	fielddata[25] = calcvalue > 99.0 ? 99.00 : calcvalue;

	// Knock including DYN and GLOB
	fielddata[26] = fielddata[4] + fielddata[9] + fielddata[10]; // KNOCKOUT1
	fielddata[27] = fielddata[5] + fielddata[9] + fielddata[10]; // KNOCKOUT2
	fielddata[28] = fielddata[6] + fielddata[9] + fielddata[10]; // KNOCKOUT3
	fielddata[29] = fielddata[7] + fielddata[9] + fielddata[10]; // KNOCKOUT4
	fielddata[30] = fielddata[8] + fielddata[9] + fielddata[10]; // KNOCKOUT5
	fielddata[31] = (fielddata[26] + fielddata[27] + fielddata[28] + fielddata[29] + fielddata[30])/5; // KNOCKOUTAVG

	// Ignition angle output
	fielddata[32] = fielddata[17] - fielddata[26]; //IGNOUT1
	fielddata[33] = fielddata[17] - fielddata[27]; //IGNOUT2
	fielddata[34] = fielddata[17] - fielddata[28]; //IGNOUT3
	fielddata[35] = fielddata[17] - fielddata[29]; //IGNOUT4
	fielddata[36] = fielddata[17] - fielddata[30]; //IGNOUT5

	// Last update
	int elapsed = GetTickCount() - lastUpdate;
	counter++;
	if (elapsed >= 250) {
		fielddata[37] = (1000.0 / elapsed) * counter;
		counter = 0;
		lastUpdate = GetTickCount();
	}

	if (notifycallback) {
		notifycallback(0, notifyhint, 0);
	}
}

// This is the main thread procedure for our driver, you'll want to do the specifics
// of communicating with your device in here!
// When fields are available to be logged, call the notifycallback function as below!
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    SetEvent(m_hThreadStarted);
	
	connectLogger(config.port, m_hThreadTerm, &setData);

	if (m_hThreadStoppedEvent)
		SetEvent(m_hThreadStoppedEvent);

	return 0;
}

/*
// Initialize
// This function is called by winlog to initialize the driver, communication with the device
// should be attempted here. Also, the device can be configured here, since it is guaranteed
// that SetConfig will be called prior to this, so your configuration data will be valid! 
// The function should return 1 if the device was sucessfully initialized, otherwise, return
// 0.

// The following parameters are passed to initialize:
HWND parent - The parent window that is initializing the driver
void *ptr - A pointer to the callback function that you can call to notify winlog of driver
            events. (See below for notification values)
void *hint - A value that should always be passed back as the NotifyCallback hint parameter.


NotifyCallback spec:
The notifycallback function informs WinLog of driver events, The parameters to the function
are:

NotifyCallback (unsigned char notification, void *hint, void *hint2);

unsigned char notification - This is the type of notification that you are informing WinLog
of, it can be one of the following:

// A notify callback value of 0 represents a sample is ready to be retrieved...
	notifycallback(0, notifyhint, 0);


// A notify callback value of 1 requests the front end to open a log file for us to dump data to, without
// updating gauges during the download. This is usually done to download stored log data to a file. If there
// is a failure to open the logfile, the function will return 0, otherwise, non-zero
if (!notifycallback(1, notifyhint, 0))
{
	return 0;
}

// A notify callback value of 2 requests the front end to close the currently open log file.
notifycallback(2, notifyhint, 0);

// a notify callback value of 3 will send a debug message to winlog.
if (notifycallback)
	notifycallback(3, notifyhint, msg);

void *hint - This value should always be the "hint" parameter that was passed in to the
             Initialize function.

void *hint2 - Notification specific. For example, the "Debug Message" notification uses
              this parameter to pass a Debug String to winlog.

*/

WINLOG_API int Initialize(HWND parent, void *ptr, void *hint)
{
	DWORD threadid;
	HANDLE hThread;
	notifyhint = hint;
	notifycallback = (int (__cdecl *)(unsigned char,void *,void *))ptr;
	m_hThreadTerm = CreateEvent( 0,TRUE,0,0);
	m_hThreadStarted = CreateEvent( 0,TRUE,0,0);
	m_hThreadStoppedEvent = CreateEvent( NULL, FALSE, FALSE, "ThreadStoppedEvent");
	ResetEvent(m_hThreadStoppedEvent);
	
	// make a thread for our driver to run in...
	threadid = 0;
	hThread = CreateThread(NULL, NULL, ThreadProc, NULL, NULL, &threadid);
	if (!hThread)
	{
		CloseHandle(m_hThreadTerm);
		CloseHandle(m_hThreadStarted);
		CloseHandle(m_hThreadStoppedEvent);
		m_hThreadTerm = NULL;
		m_hThreadStarted = NULL;
		m_hThreadStoppedEvent = NULL;
		return 0;
	}

	return 1;
}

// Shutdown
// This API is called by WinLog to shut the driver down, you should disconnect
// from the device and clean up any allocated resources here. 
// If the shutdown is successfull, return a 1, otherwise, return 0.
WINLOG_API int Shutdown(void)
{

	if (m_hThreadTerm)
		SetEvent(m_hThreadTerm);

	if (m_hThreadStoppedEvent)
	{
		// wait here for up to 5 seconds for the object thread to stop...
		if (WaitForSingleObject(m_hThreadStoppedEvent, 5000) != WAIT_OBJECT_0)
			return FALSE;
	}

	if (m_hThreadTerm)
	{
		CloseHandle(m_hThreadTerm);
		m_hThreadTerm = NULL;
	}

	if (m_hThreadStarted)
	{
		CloseHandle(m_hThreadStarted);
		m_hThreadStarted = NULL;
	}

	if (m_hThreadStoppedEvent)
	{
		CloseHandle(m_hThreadStoppedEvent);
		m_hThreadStoppedEvent = NULL;
	}

	return 1;
}

// GetLastDriverError
// WinLog calls this function to get a detailed explaination of the last failure, simply
// return a string detailing it here!
WINLOG_API char *GetLastDriverError(void)
{
	return "";
}

// GetFieldName
// WinLog Calls this api to get the name of the field specified in "FieldNum", if you're
// leaving the structures defined above intact, you probably won't need to change this field.
WINLOG_API char *GetFieldName(int fieldnum) {
	if (fieldnum >= NUM_FIELDS)
		return NULL;
	
	return fieldnames[fieldnum];
}

// GetFieldFormat
// WinLog Calls this api to get the name of the field format specified in "FieldNum", if you're
// leaving the structures defined above intact, you probably won't need to change this field.
WINLOG_API char GetFieldFormat(int fieldnum)
{

	if (fieldnum >= NUM_FIELDS)
		return 0;
	
	return 'f';
}

// GetFieldData
// This API is called by Winlog to get the data for a specific field after the driver notifies
// WinLog that data is ready (by calling the notifycallback function with a 0). Simply
// return a pointer to the field value. if you're
// leaving the structures defined above intact, you probably won't need to change this field.
WINLOG_API void *GetFieldData(int fieldnum)
{
	if (fieldnum >= NUM_FIELDS)
		return NULL;

	return &fielddata[fieldnum];
}


// GetDeviceName
// This API is called by WinLog to determine the name of your device, simply return the
// name of the device.
static char namebuffer[128];
WINLOG_API char *GetDeviceName(void)
{
	strcpy_s(namebuffer, "M2.3.2 Fast Logger");
	return namebuffer;
}

// GetDriverVersion
// This API is called by WinLog to determine the version of your driver, return this
// as an integer value.
WINLOG_API int GetDriverVersion(void)
{
	return 1;
}

// Configure
// This API is called by Winlog to have the driver display its configuration dialog, you
// should not reconfigure the device here, just display the configuration, and update the
// configuration structure, WinLog will reconfigure the device by calling initialize again
// later
WINLOG_API int Configure(HWND hParent)
{
	// Show your configuration dialog here!!!

	// ...

	// don't actually reconfigure the device here,
	// just configure the structures, the front end will re-initialize the device with the new settings.
	
	// return 0 if they cancelled your configuration dialog, or 1 if all went ok!

	return DialogBox((HINSTANCE)hInst, MAKEINTRESOURCE(IDD_DIALOG1), hParent, reinterpret_cast<DLGPROC>(DlgProc));
}

LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	char tmpbuf[100];
	HWND combobox = GetDlgItem(hWndDlg, IDC_COMBO1);
	char* headings[5] = {"", "Bosch 250kpa", "Bosch 300kpa", "MPXH6300A", "MPXH6400A"};
	double factors[5] = {0, 50, 65.88235, 62.89308, 82.61049};
	double offsets[5] = {0, 10, -6.35764, 0.222013, 6.9558};
	int sel = 0;

	switch(Msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hWndDlg, IDC_EDIT1, config.port);
		StringCbPrintf(tmpbuf, sizeof(tmpbuf), "%01.4f", config.mapMult);
		SetDlgItemText(hWndDlg, IDC_EDIT2, tmpbuf);
		StringCbPrintf(tmpbuf, sizeof(tmpbuf), "%01.4f", config.mapOffset);
		SetDlgItemText(hWndDlg, IDC_EDIT3, tmpbuf);

		for each (char* heading in headings) {
			SendMessage(combobox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(heading));
		}
		return TRUE;
		break;

		case WM_COMMAND:
			switch(wParam)
			{
			
			case MAKEWPARAM(IDC_COMBO1, CBN_SELCHANGE):
				sel = SendMessage(combobox, CB_GETCURSEL, 0, 0);

				if (sel != 0) {
					StringCbPrintf(tmpbuf, sizeof(tmpbuf), "%01.4f", factors[sel]);
					SetDlgItemText(hWndDlg, IDC_EDIT2, tmpbuf);
					StringCbPrintf(tmpbuf, sizeof(tmpbuf), "%01.4f", offsets[sel]);
					SetDlgItemText(hWndDlg, IDC_EDIT3, tmpbuf);
				}

				return TRUE;
			break;

			case IDOK:
				GetDlgItemText(hWndDlg, IDC_EDIT1, config.port, sizeof(config.port));
				GetDlgItemText(hWndDlg, IDC_EDIT2, tmpbuf, sizeof(tmpbuf));
				config.mapMult = atof(tmpbuf);
				GetDlgItemText(hWndDlg, IDC_EDIT3, tmpbuf, sizeof(tmpbuf));
				config.mapOffset = atof(tmpbuf);
				EndDialog(hWndDlg, 1);
				return TRUE;
			break;

			case IDCANCEL: 
				EndDialog(hWndDlg, 0);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

// GetDeviceFunction
// WinLog Calls this api to get the name of the device function specified in "functionnum".
// You should return the user readable name of the function requested, or NULL if the
// function is not a valid function number.
WINLOG_API char *GetDeviceFunction(int functionnum)
{
	switch (functionnum)
	{
	case 0:
		{
			return NULL;
		}
		break;
	}

	return NULL;
}

// ExecuteDeviceFunction
// WinLog calls this APT to executes a device specific functions.
// A return value of -1 should be returned if the function is
// not supported. The function numbers should match the numbers returned
// in the description
// of the functions returned in the GetDeviceFunction API.
WINLOG_API int ExecuteDeviceFunction(LPSTR functionname)
{
	return -1;
}