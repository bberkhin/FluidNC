#include "Encoders.h"
#include "Encoder.h"
#include "Machine/MachineConfig.h"  // config


namespace Machine {

    Encoders::Encoders() : _encoder() {
        for (int i = 0; i < MAX_N_AXIS; ++i) {
            _encoder[i] = nullptr;
        }
    }

    void Encoders::init() {
        for (int axis = X_AXIS; axis < _numberAxis; axis++) {
            auto a = _encoder[axis];
            if (a) {
                log_info("Encoder " << axisName(axis));
                a->init();
            }
        }
    }

    bool Encoders::need_report_status() {
        return _status;
    }

    void Encoders::set_report_status(bool status) {
        _status = status;
    }

    void Encoders::reset_encoders(float* position) {
        auto   a      = config->_axes;
        auto n_axis = a ? a->_numberAxis : 0;
        n_axis = std::min( _numberAxis, n_axis);

        for (int i = 0; i < n_axis; ++i) {
            if ( _encoder[i] )
                _encoder[i]->reset(position[i]);
        }
    }


    float* Encoders::get_pos() { 
        static float position[MAX_N_AXIS];
        size_t       n_axis = _numberAxis ? _numberAxis : MAX_N_AXIS;
        for (size_t i = 0; i < n_axis; ++i) {
            auto a = _encoder[i];
            if (a) 
                position[i] = a->readEncoder();
            else
                position[i] = 0;
        }
        return position;
    }


    // Configuration helpers:

    void Encoders::group(Configuration::HandlerBase& handler) {
        // Handle axis names xyzabc.  handler.section is inferred
        // from a template.
        char tmp[2];
        tmp[1] = '\0';

        // During the initial configuration parsing phase, _numberAxis is 0 so
        // we try for all the axes.  Subsequently we use the number of axes
        // that are actually present.
        size_t n_axis = _numberAxis ? _numberAxis : MAX_N_AXIS;
        for (size_t i = 0; i < n_axis; ++i) {
            tmp[0] = tolower(_names[i]);
            handler.section(tmp, _encoder[i], i);
        }
    }

    void Encoders::afterParse() {
        // Find the last axis that was declared and set _numberAxis accordingly
        for (size_t i = MAX_N_AXIS; i > 0; --i) {
            if (_encoder[i - 1] != nullptr) {
                _numberAxis = i;
                break;
            }
        }
        // Senders might assume 3 axes in reports
        if (_numberAxis < 3) {
            _numberAxis = 3;
        }

        for (int i = 0; i < _numberAxis; ++i) {
            if (_encoder[i] == nullptr) {
                _encoder[i] = new Encoder(i);
            }
        }
    }

    Encoders::~Encoders() {
        for (int i = 0; i < MAX_N_AXIS; ++i) {
            if (_encoder[i] != nullptr) {
                delete _encoder[i];
            }
        }
    }
}

