// Copyright (c) 2018 -	Bart Dring
// Copyright (c) 2021 -	Stefan de Bruijn
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Encoder.h"
#include <esp32-hal-gpio.h>  // CHANGE

#include "Uart.h" //testing

namespace Machine {

    Encoder::Encoder(int currentaxis) : _axis(currentaxis) {}

    Encoder::~Encoder() {
        _outA.detachInterrupt();
        _outB.detachInterrupt();
    }

    void Encoder::init() {
        
        if (_outA.undefined() || _outB.undefined() ) {
            return;
        }

        _outA.report("_outA");
        _outB.report("_outB");
        if (_impulsesPerMm < 1) {
            log_info("incorrect impulsesPerMm value: " << _impulsesPerMm);
        } else {
            log_info("impulsesPerMm: " << _impulsesPerMm);
        }

        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (_outA.capabilities().has(Pins::PinCapabilities::PullDown)) {
            attr = attr | Pin::Attr::PullDown;
        }
        _outA.setAttr(attr);
        attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (_outB.capabilities().has(Pins::PinCapabilities::PullDown)) {
            attr = attr | Pin::Attr::PullDown;
        }
        _outB.setAttr(attr);
        _outA.attachInterrupt(ISRHandlerA, RISING, this);
        _outB.attachInterrupt(ISRHandlerB, RISING, this);
    }

    void IRAM_ATTR Encoder::handleISRA() {

        bool bit = _outB.read();
        if (!_outB.read()) {
            _encoder0Pos++;
        } else {
            _encoder0Pos--;
        }
       
        int pos = _encoder0Pos;
       // Uart0 << "encA: " << pos << " B: " << bit << "\n";
        Uart0 << pos << "\n";
    }

    void IRAM_ATTR Encoder::handleISRB() {
        if (!_outA.read()) {
            _encoder0Pos--;
        } else {
            _encoder0Pos++;
        }
    }

    
    /*  void IRAM_ATTR Encoder::handleISR() {
        // code from https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
      
        _old_AB <<= 2;  //remember previous state
        
//        _outA.report("handleISR");
        if (!_outA.defined() || !_outB.defined())
            return;
        
            auto pinState = _outA.read();

        int8_t ENC_PORT = ((_outB.read()) ? (1 << 1) : 0) | ((_outA.read()) ? (1 << 0) : 0);

        _old_AB |= (ENC_PORT & 0x03);  //add current state

        int8_t currentDirection = (enc_states[(_old_AB & 0x0f)]);  //-1,0 or 1

        if (currentDirection != 0) {
            _encoder0Pos += currentDirection;
                    
            //respect limits
            if (_encoder0Pos > (_maxEncoderValue))
                _encoder0Pos = _circleValues ? _minEncoderValue : _maxEncoderValue;
            if (_encoder0Pos < (_minEncoderValue))
                _encoder0Pos = _circleValues ? _maxEncoderValue : _minEncoderValue;
        }
      }
      */


    void Encoder::reset(float newValue_) {       
          long encoder0Pos     = lround(newValue_ * _impulsesPerMm);
          _encoder0Pos         = encoder0Pos;
          _lastReadEncoder0Pos = _encoder0Pos;
          if (_encoder0Pos > _maxEncoderValue)
              _encoder0Pos = _circleValues ? _minEncoderValue : _maxEncoderValue;
          if (_encoder0Pos < _minEncoderValue)
              _encoder0Pos = _circleValues ? _maxEncoderValue : _minEncoderValue;
      }

    
    float Encoder::readEncoder() {
        return (_encoder0Pos / _impulsesPerMm);  // to mm
    }

     void Encoder::setEncoderValue(float newValue) { 
        reset(newValue); 
     }

     float Encoder::encoderChanged() {
         long encoder0Pos     = _encoder0Pos;
         long encoder0Diff = encoder0Pos - _lastReadEncoder0Pos;
         _lastReadEncoder0Pos = encoder0Pos;
         return (encoder0Diff / _impulsesPerMm);
      }

    void Encoder::group(Configuration::HandlerBase& handler) {
        handler.item("a_pin", _outA);
        handler.item("b_pin", _outB);
        handler.item("impulses_per_mm", _impulsesPerMm);
    }
};
