#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

/*
Author: who am i? what does it even mean to 'be'
Date (Due): 12/17/21
Description: Create serial connection between uC and PC
*/

// Define buffers
#define READ_TIMEOUT    500
#define BUFFERSIZE      200


HANDLE hComm;
DWORD dwBytesRead = 0;
DWORD dwEventMask;
BOOL temp = FALSE;
char ReadBuffer[BUFFERSIZE] = { 0 };
char ReadData;
char SerialBuffer[64] = { 0 };
COMMTIMEOUTS timeouts = { 0 };
unsigned char loop = 0;

int main()
{
    hComm = CreateFile(
        L"COM1",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // Test for connection
    if (hComm == INVALID_HANDLE_VALUE) {
        printf("hComm error\n\r");
    }
    else {
        printf("hComm initialized\n\r");
    }

    // Verify current settings
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    temp = GetCommState(hComm, &dcbSerialParams);
    if (temp == FALSE)
    {
        printf("\nCommState error\n\r");
    }
    
    // Establish parameters
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;    

    temp = SetCommState(hComm, &dcbSerialParams);
    if (temp == FALSE) {
        printf("DCB_Structure error\n\r");
    }

    //Setting Timeouts
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (SetCommTimeouts(hComm, &timeouts) == FALSE) {
        printf("Timeout error\n\r");
    }

    // Setting Receive Mask
    temp = SetCommMask(hComm, EV_RXCHAR);
    if (temp == FALSE) {
        printf("CommMask error\n\r");
        return -1;
    }

    // Setting WaitComm() Event
    temp = WaitCommEvent(hComm, &dwEventMask, NULL); // Wait for the character to be received
    if (temp == FALSE) {
        printf("Error! in Setting WaitCommEvent()\n\r");
        return -1;
    }

    // Read data
    do {
        temp = ReadFile(hComm, &ReadData, sizeof(ReadData), &dwBytesRead, NULL);
        SerialBuffer[loop] = ReadData;
        ++loop;
    } while (dwBytesRead > 0);
    --loop;

    // Print data
    for (int i = 0; i < loop; ++i) {
        printf("%c", SerialBuffer[i]);
    }

    CloseHandle(hComm);
    return 0;
}