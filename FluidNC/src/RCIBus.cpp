#include "Planner.h"
#include "Protocol.h"
#include "System.h"
#include "Stepper.h"
#include "MotionControl.h"
#include "Motors/MotorDriver.h"
#include "Machine/MachineConfig.h"

#include "RCIBus.h"
#include "RCChannel.h"

#include <esp32-hal.h>  // millis()
#include <string.h>
#include <cstring>
#include <iostream>

#include "JogSpeed.h"


Uart* RCIBus::_uart = nullptr;



#ifdef FLUIDNC_CONSOLE // testing stuff
uint8_t test_buffer[] = { 0x20, 0x40, 
0xDB, 0x5, //0
0xDC, 0x5, //1
0x54, 0x5, //2
0xDC, 0x5, //3
0xE8, 0x3, //4
0xD0, 0x7, //5
0xD2, 0x5,//6
0xE8, 0x3,//7
0xDC, 0x5,//8
0xDC, 0x5,//9
0xDC, 0x5,//10
0xDC, 0x5,//11
0xDC, 0x5,//12
0xDC, 0x5,//13
0xDA, 0xF3 };
#endif

RCIBus::RCIBus() : Channel("rc_ibus") {
    for (int i = 0; i < PROTOCOL_CHANNELS; ++i) {
        _channels[i] = nullptr;
    }
}

RCIBus::~RCIBus() {
    for (int i = 0; i < PROTOCOL_CHANNELS; ++i) {
        if (_channels[i])
            delete _channels[i];
    }
}


void RCIBus::init() {
    if (!_uart) {
        log_info("Uart not defined");
        return;
    }

    _uart->_rxd_pin.report("RX");
    _uart->_txd_pin.report("TX");
    _state   = DISCARD;
    _last    = millis();
    _ptr     = 0;
    _len     = 0;
    _chksum  = 0;
    _lchksum = 0;

    _uart->begin();
#ifdef FLUIDNC_CONSOLE
    _uart->write(test_buffer, 32);
#endif
    allChannels.registration(this);

    for (int i = 0; i < PROTOCOL_CHANNELS; ++i) {
        if (_channels[i]) {
            _channels[i]->doLog();
        }
    }
    
    for (size_t idx = 0; idx < 2; idx++) {
    auto axisSetting = config->_axes->_axis[idx];
        if (axisSetting != nullptr) {
            _max_speed = std::min(_max_speed, axisSetting->_maxRate);
        }
    }

    log_info("RemoteControl started! Max speed rate:" << _max_speed );
}

void RCIBus::group(Configuration::HandlerBase& handler) {
    handler.section("uart", _uart);
    char tmp[10];
    tmp[0] = 0;
    strcat(tmp, "channel");

    for (size_t i = 0; i < PROTOCOL_CHANNELS; ++i) {
        if (i < 10) {
            tmp[7] = char(i + '0');
            tmp[8] = '\0';
        } else if (i < 20) {
            tmp[7] = '1';
            tmp[8] = char(i - 10 + '0');
            tmp[9] = '\0';
        } else {
            //to many channels
            break;
        }      
        handler.section(tmp, _channels[i], i+1);
    }

}

void RCIBus::afterParse() {
}
/*
 *  supports max 14 channels in this lib (with messagelength of 0x20 there is room for 14 channels)
  Example set of bytes coming over the iBUS line for setting servos: 
    20 40 DB 5 DC 5 54 5 DC 5 E8 3 D0 7 D2 5 E8 3 DC 5 DC 5 DC 5 DC 5 DC 5 DC 5 DA F3
  Explanation
    Protocol length: 20
    Command code: 40 
    Channel 0: DB 5  -> value 0x5DB
    Channel 1: DC 5  -> value 0x5Dc
    Channel 2: 54 5  -> value 0x554
    Channel 3: DC 5  -> value 0x5DC
    Channel 4: E8 3  -> value 0x3E8
    Channel 5: D0 7  -> value 0x7D0
    Channel 6: D2 5  -> value 0x5D2
    Channel 7: E8 3  -> value 0x3E8
    Channel 8: DC 5  -> value 0x5DC
    Channel 9: DC 5  -> value 0x5DC
    Channel 10: DC 5 -> value 0x5DC
    Channel 11: DC 5 -> value 0x5DC
    Channel 12: DC 5 -> value 0x5DC
    Channel 13: DC 5 -> value 0x5DC
    Checksum: DA F3 -> calculated by adding up all previous bytes, total must be FFFF
 */

bool RCIBus::readBuffer(void) {
    bool ret = false; 
#ifdef FLUIDNC_CONSOLE
    _state   = GET_LENGTH;
#endif
    while (_uart->available() > 0) {
#ifndef FLUIDNC_CONSOLE
        uint32_t now = millis();
        if (now - _last >= PROTOCOL_TIMEGAP) {
            _state = GET_LENGTH;
        }
        _last = now;
#endif
        uint8_t v = _uart->read();
         switch (_state) {
            case GET_LENGTH:
                if (v <= PROTOCOL_LENGTH) {
                    _ptr    = 0;
                    _len    = v - PROTOCOL_OVERHEAD;
                    _chksum = 0xFFFF - v;
                    _state  = GET_DATA;
                } else {
                    _state = DISCARD;
                }
                break;

            case GET_DATA:
                _buffer[_ptr++] = v;
                _chksum -= v;
                if (_ptr == _len) {
                    _state = GET_CHKSUML;
                }
                break;

            case GET_CHKSUML:
                _lchksum = v;
                _state   = GET_CHKSUMH;
                break;

            case GET_CHKSUMH:
                // Validate checksum
                if (_chksum == (v << 8) + _lchksum) {
                    // Execute command - we only know command 0x40
                    switch (_buffer[0]) {
                        case PROTOCOL_COMMAND40:
                            // Valid - extract channel data
                            for (uint8_t i = 1; i < PROTOCOL_CHANNELS * 2 + 1; i += 2) {
                                RCChannel* p = _channels[i / 2];
                                if ( p ) {
                                    p->setValue(_buffer[i] | (_buffer[i + 1] << 8));
                                }
                            }
                            ret = true; 
                            break;

                        default:
                            break;
                    }
                }
                _state = DISCARD;
                break;

            case DISCARD:
            default:
                break;
        }
    }
    return ret;
}

bool RCIBus::do_log() {

    bool changed = false;
    for (int i = 0; i < PROTOCOL_CHANNELS; i++) {
        if (_channels[i] && _channels[i]->isChanged() ){
            changed = true;
            _channels[i]->doLog();
        }
    }
    //needPrint = true;
    /*
    if (needPrint) {
        for (uint8_t i = 0; i < PROTOCOL_CHANNELS; i++) {
            if (_channels[i]) {
                _channels[i]->doLog();
            }
        }
    }
    */
    return changed;
}

    Channel* RCIBus::pollLine(char* line) {
    
    Channel*   ret           = nullptr;
    if( !readBuffer() ) {
        return ret;
     }
      
    bool bnewval = do_log();

    int signal_speed = 0;
    int signal_rotate = 0;
    int signal_left = -1;
    int signal_right   = -1;
    float   speedleft     = 0.f;
    float   speedright    = 0.f;
    for (size_t i = 0; i < PROTOCOL_CHANNELS; ++i) {
        if (_channels[i]) {
            switch (_channels[i]->type()) {
                case RCCommand::NoCmd:
                    break; 
                case RCCommand::Move:
                    signal_speed = _channels[i]->normolizedValue();
                    break;
                case RCCommand::Rotate:
                    signal_rotate = _channels[i]->normolizedValue();
                    break;
                case RCCommand::Left:
                    signal_left = _channels[i]->normolizedValue();
                    break;
                case RCCommand::Right:
                    signal_right = _channels[i]->normolizedValue();
                    break;
                    
                case RCCommand::GrblCmd:
                    if (line) {
                        if (_channels[i]->isChanged() && _channels[i]->isOn()) {
                            strncpy(line, _channels[i]->getGrblCmd(), Channel::maxLine);
                            _channels[i]->cmdDone();
                            return this;
                        }
                    }
                    break;
                    /*
                case RCCommand::Reset:
                    if (_channels[i]->isOn() ) {
                        ;  //mc_reset();
                        return ret;
                    }
                    */
                    break;
                    
            }
        }
    }
    bool do_move = true;

    if (signal_left != -1 && signal_right != -1) { // left\right type driving
        speedleft  = _max_speed * float(signal_left) / 100.0f;
        speedright = _max_speed * float(signal_right) / 100.0f;

        if (signal_left == 0 && signal_right == 0)
            do_move = false;

    } else {
        float   cur_speed  = _max_speed * float(signal_speed) / 100.0f;
        Percent overrotate = abs(signal_rotate);
        speedleft          = cur_speed;
        speedright         = cur_speed;

        if (signal_speed != 0) {      // move
            if (signal_rotate > 0) {  // rignt side is stopped
                speedright *= (float(100 - signal_rotate) / 100.0f);
            } else if (signal_rotate < 0) {  // rignt side is stopped
                speedleft *= (float(100 + signal_rotate) / 100.0f);
            }
        } else if (signal_rotate != 0) {  //rotate on point
            speedleft  = _max_speed * float(signal_rotate) / 200.0f;  // rotation speed  twice less then max speed
            speedright = -1 * speedleft;
        } else {
            do_move = false;
        }
    }

    if ( !(sys.state == State::JogSpeed || (sys.state == State::Idle && do_move)) )
        return ret;


    //if (bnewval) {
    log_info((do_move ? "MOVE" : "STOP") << "signal_left :" << signal_left << " signal_right " << signal_right);
    log_info(" speedleft :" << speedleft << " speedright " << speedright );
   // log_info("signal_speed :" << signal_speed << " signal_rotate " << signal_rotate);
        
    //}

    if (!do_move) {  // stop
        jog_speed_stop();
    } else if (line && sys.state == State::Idle) {  //start moving
        jog_speed_execute(speedleft, speedright);
        ret = this;
    } else { // change speed
        jog_speed_execute(speedleft, speedright);
    }

    protocol_execute_realtime();
    return ret;
}
