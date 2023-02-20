#pragma once
#include "../Configuration/Configurable.h"
#include <esp32-hal-timer.h>  // hw_timer_t


enum DisplayType {
    UnknownType = 0,  // Must be zero.
    OledSPISH1106,
    OledSPISSD1306,
    OledI2CSH1106,
    OledI2CSSD1306
};

class OLEDDisplay;

class Display : public Configuration::Configurable {
public:
    Display();
    ~Display();
    static void showInfo();
    void init();
    // Configuration handlers.
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;

private:
    int          _type    = DisplayType::UnknownType;
    static OLEDDisplay* _display;
    const char*  typeName();
    Pin          _rstPin;
    Pin          _csPin;
    Pin          _dcPin;

private:
    static void        onDisplayUpdate();
    static const int   _dsTimerNumber = 1;
    static hw_timer_t* _dsTimer;
};
