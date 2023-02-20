#pragma once
#include "src/Channel.h"
#include <Windows.h>
#include <queue>

class ComPortX86 : public Channel {
public:
    ComPortX86(const char* pPort);
    ComPortX86();
    ~ComPortX86();
    virtual size_t write(uint8_t c) override;
    virtual int    read() override;
    virtual void   ack(Error status) override;
    virtual int    available() { return true; }
    virtual int    peek() { return 0; }
    virtual void   flush() { return; }
    
    virtual int    rx_buffer_available() { return 0; }
    virtual Channel* pollLine(char* line);
    Channel* pullLineFromUart(char* line);

private:
    HANDLE hSerial;
    std::queue<uint8_t> _queue;
};
