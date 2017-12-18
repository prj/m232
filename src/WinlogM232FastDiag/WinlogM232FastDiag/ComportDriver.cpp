#include "Windows.h"
#include <vector>
#define BOOST_PACKET_SIZE 13
#define FUEL_PACKET_SIZE 19
#define PACKET_SIZE BOOST_PACKET_SIZE + FUEL_PACKET_SIZE
using namespace std;

boolean validatePacket(vector<unsigned char> *buffer) {
	if ((*buffer)[BOOST_PACKET_SIZE - 2] == 0xAA && (*buffer)[BOOST_PACKET_SIZE - 1] == 0xBB) {
		unsigned char prev = 0x00;
		for (int i = BOOST_PACKET_SIZE; i < PACKET_SIZE; i++) {
			if (prev == 0xAA && (*buffer)[i] == 0xBB) {
				return false;
			}
			prev = (*buffer)[i];
		}
		return true;
	}
	return false;
}

void connectLogger(char* port, HANDLE exitHandle, void (*callbackFunc)(unsigned char*, int)) {
	HANDLE comport;
	DCB comsettings;
	ULONG len;
	unsigned char INBUFFER[1];
	vector<unsigned char> buffer;

	while(WaitForSingleObject(exitHandle, 100) != WAIT_OBJECT_0) {
		if ((comport = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != NULL) {

			COMMTIMEOUTS timeouts;
			timeouts.ReadIntervalTimeout = MAXWORD;
			timeouts.ReadTotalTimeoutConstant = 500;
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.WriteTotalTimeoutConstant = 0;
			timeouts.WriteTotalTimeoutMultiplier = 0;

			SetCommTimeouts(comport, &timeouts);

			GetCommState(comport, &comsettings);
			comsettings.BaudRate = 187500;
			comsettings.Parity = FALSE;
			comsettings.fParity = FALSE;
			comsettings.StopBits = 0;
			comsettings.ByteSize = 8;
			SetCommState(comport, &comsettings);

			while(WaitForSingleObject(exitHandle, 0) != WAIT_OBJECT_0) {
				ReadFile(comport, &INBUFFER, sizeof(INBUFFER), &len, NULL);
				if (len == 0) {
					break;
				}
				buffer.push_back(INBUFFER[0]);
				if (buffer.size() > PACKET_SIZE) {
					buffer.erase(buffer.begin());
					if (validatePacket(&buffer)) {
						callbackFunc(&buffer[0], buffer.size());
					}
				}
			}
			CloseHandle(comport);
		}
	}
}