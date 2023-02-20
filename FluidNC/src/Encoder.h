// Copyright (c) 2021 -	Stefan de Bruijn
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Configuration/Configurable.h"
#include "Pin.h"
#include <esp_attr.h>  // IRAM_ATTR

#define AIESP32ROTARYENCODER_DEFAULT_STEPS 2

namespace Machine {

class Encoder : public Configuration::Configurable {
    public:
        Encoder(int currentaxis);
        ~Encoder();
        void init();

        void reset(float newValue = 0);
        float readEncoder();
        void setEncoderValue(float pos);
        float encoderChanged();

        // Configuration handlers.
        void group(Configuration::HandlerBase& handler) override;

    private:
        int _axis;
        Pin _outA;
        Pin _outB;

        void IRAM_ATTR handleISRA();
        void IRAM_ATTR handleISRB();
        static void IRAM_ATTR ISRHandlerA(void* data) { static_cast<Encoder*>(data)->handleISRA(); }
        static void IRAM_ATTR ISRHandlerB(void* data) { static_cast<Encoder*>(data)->handleISRB(); }

        volatile long _encoder0Pos         = 0;
        bool          _circleValues        = false;
        long          _minEncoderValue     = -1 << 15;
        long          _maxEncoderValue     = 1 << 15;
        uint8_t       _old_AB              = 0;
        long          _lastReadEncoder0Pos = 0;
        float         _impulsesPerMm       = 80.0f;
        int8_t        enc_states[16]       = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };
};
};
