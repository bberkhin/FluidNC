// Copyright (c) 2021 -	Stefan de Bruijn
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Configuration/Configurable.h"
#include "Uart.h"

class RCChannel;
const uint8_t PROTOCOL_CHANNELS = 10;

class RCIBus : public Channel, public Configuration::Configurable {
    public:
    RCIBus();
    ~RCIBus();
    // Initializes coolant control pins.
    void init();
    
    // Channal
    virtual size_t   write(uint8_t c) { return 0; }
    virtual int      read() { return 0; }
    virtual int      available() { return true; }
    virtual int      peek() { return 0; }
    virtual void     flush() { return; }

    virtual Channel* pollLine(char* line);
    const char*      name() { return "rc_ibus"; }
    virtual int rx_buffer_available() { return 0; }

    // Configuration helpers:
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;

private:
    bool do_log();
    bool readBuffer(void);
    

private:
    RCChannel*   _channels[PROTOCOL_CHANNELS];
    static Uart* _uart;

    enum { GET_LENGTH, GET_DATA, GET_CHKSUML, GET_CHKSUMH, DISCARD };

    static const uint8_t PROTOCOL_LENGTH    = 0x20;
    static const uint8_t PROTOCOL_OVERHEAD  = 3;  // <len><cmd><data....><chkl><chkh>
    static const uint8_t PROTOCOL_TIMEGAP   = 3;  // Packets are received very ~7ms so use ~half that for the gap    
    static const uint8_t PROTOCOL_COMMAND40 = 0x40;  // Command is always 0x40
    
    uint8_t         _state = DISCARD;             // state machine state for iBUS protocol
    uint32_t        _last = 0;                        // milis() of prior message
    uint8_t         _buffer[PROTOCOL_LENGTH];     // message buffer
    uint8_t         _ptr = 0;                         // pointer in buffer
    uint8_t         _len = 0;                         // message length
    uint16_t        _chksum = 0;                      // checksum calculation
    uint8_t         _lchksum = 0;                     // checksum lower byte received    
    float _max_speed    = 1000;                   //msxspeed
};
