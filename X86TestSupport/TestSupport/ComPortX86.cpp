#include "ComPortX86.h"
#include <iostream>
#include <conio.h>
#include "StdTimer.h"
#include "src/Serial.h"  // execute_realtime_command


#define MAX_DEVPATH_LENGTH 1024
extern StdTimer g_timer;

ComPortX86::ComPortX86(const char* pPort) : hSerial(INVALID_HANDLE_VALUE), Channel("com_win32") {
    DCB          dcb;
    BOOL         fSuccess;
    TCHAR        devicePath[MAX_DEVPATH_LENGTH];
    COMMTIMEOUTS commTimeout;

    if (pPort != NULL) {
        mbstowcs_s(NULL, devicePath, MAX_DEVPATH_LENGTH, pPort, strlen(pPort));
        hSerial = CreateFile(devicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    }
    if (hSerial != INVALID_HANDLE_VALUE) {
        //  Initialize the DCB structure.
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        fSuccess      = GetCommState(hSerial, &dcb);
        if (!fSuccess) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return;
        }

        GetCommState(hSerial, &dcb);
        dcb.BaudRate = CBR_115200;  //  baud rate
        dcb.ByteSize = 8;           //  data size, xmit and rcv
        dcb.Parity   = NOPARITY;    //  parity bit
        dcb.StopBits = ONESTOPBIT;  //  stop bit
        dcb.fBinary  = TRUE;
        dcb.fParity  = TRUE;

        fSuccess = SetCommState(hSerial, &dcb);
        if (!fSuccess) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return;
        }

        GetCommTimeouts(hSerial, &commTimeout);
        commTimeout.ReadIntervalTimeout         = 1;
        commTimeout.ReadTotalTimeoutConstant    = 1;
        commTimeout.ReadTotalTimeoutMultiplier  = 1;
        commTimeout.WriteTotalTimeoutConstant   = 1;
        commTimeout.WriteTotalTimeoutMultiplier = 1;
        SetCommTimeouts(hSerial, &commTimeout);
    }
}

ComPortX86::ComPortX86() : Channel("com_win32"), hSerial(INVALID_HANDLE_VALUE) {}

ComPortX86::~ComPortX86() {}

int ComPortX86::read() {
    DWORD   dwBytesRead;
    uint8_t data;
    int     ret = -1;

    if (hSerial != INVALID_HANDLE_VALUE) {
        if (ReadFile(hSerial, &data, 1, &dwBytesRead, NULL) && dwBytesRead == 1) {
            ret = static_cast<int>(data);
        }
    } else {
        if (_kbhit()) {
            ret    = _getch();
           // char c = static_cast<char>(ret);
            //std::cout << c;
            //if (c == 10 || c == 13)
              //  std::cout << "\n";
        }
    }
    return ret;
}

size_t ComPortX86::write(uint8_t c) {
    DWORD dwBytesWritten = 1;
    if (hSerial != INVALID_HANDLE_VALUE) {
        WriteFile(hSerial, &c, 1, &dwBytesWritten, NULL);
    } else {
        ;// std::cout << c;
    }
    return dwBytesWritten;
}

Channel* ComPortX86::pullLineFromUart(char* line) 
{ 
 while (1) {
    int ch;
    if (line && _queue.size()) {
        ch = _queue.front();
        _queue.pop();
    } else {
        ch = read();
    }

    // ch will only be negative if read() was called and returned -1
    // The _queue path will return only nonnegative character values
    if (ch < 0) {
        break;
    }
    if ( is_realtime_command(ch)) {
        //std::cout <<  ch << "; ";
        execute_realtime_command(static_cast<Cmd>(ch), *this);
        continue;
    }
    if (line) {
        if (ch == '\r' || ch == '\n') {
            _line[_linelen] = '\0';
            if (_linelen == 0)
                continue;
            strcpy(line, _line);
            _linelen = 0;
           // std::cout << line << "\n";
            return this;
        }
        if (_linelen < (Channel::maxLine - 1)) {
            _line[_linelen++] = ch;
        }
        else 
        {
            std::cout << "Buffer overflow\n";
        }
    } else {
        // If we are not able to handle a line we save the character
        // until later
        _queue.push(uint8_t(ch));
    }
}
return nullptr;
}

Channel* ComPortX86::pollLine(char* line) 
{
    if (hSerial != INVALID_HANDLE_VALUE)
        return pullLineFromUart(line);
    return Channel::pollLine(line);
}


void ComPortX86::ack(Error status) {
    
    Channel::ack(status);
    
    if (hSerial == INVALID_HANDLE_VALUE)
        return;

    switch (status) {
        case Error::Ok:  // Error::Ok
            ;//std::cout << "ok\n";
            break;
        default:
            // With verbose errors, the message text is displayed instead of the number.
            // Grbl 0.9 used to display the text, while Grbl 1.1 switched to the number.
            // Many senders support both formats.
            std::cout << "error:";
            std::cout << errorString(status);
            std::cout << "\n";
            break;
    }
}
