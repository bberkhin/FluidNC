#pragma once
// Copyright (c) 2021 -  Stefan de Bruijn
// Copyright (c) 2021 -  Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Configuration/Configurable.h"
#include "Encoder.h"

namespace Machine {

    class Encoder;

    class Encoders : public Configuration::Configurable {
        bool _switchedStepper = false;
        bool _status          = false;
    public:
        static constexpr const char* _names = "XYZABC";

        Encoders();
        inline char axisName(int index) { return index < MAX_N_AXIS ? _names[index] : '?'; }  // returns axis letter
        int      _numberAxis = 0;
        Encoder* _encoder[MAX_N_AXIS];

        bool   need_report_status();
        void   set_report_status(bool status);
        void   reset_encoders(float* pos);
        float* get_pos();

        void init();

        // Configuration helpers:
        void group(Configuration::HandlerBase& handler) override;
        void afterParse() override;

        ~Encoders();
    };
};
