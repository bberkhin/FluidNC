#include "Display.h"
#include "../EnumItem.h"
#include "../Machine/MachineConfig.h"
#include "../Protocol.h"
#include "SH1106Spi.h"


EnumItem dsTypes[] = { 
      { DisplayType::UnknownType, "unknown" }, 
      { DisplayType::OledSPISH1106, "OLEDSPISH1106" },
      { DisplayType::OledSPISSD1306, "OLEDSPISSD1306" },
      { DisplayType::OledI2CSH1106, "OLEDI2CSH1106" },
      { DisplayType::OledI2CSSD1306, "OLEDI2CSSD1306" },
      { DisplayType::UnknownType } };

hw_timer_t* Display::_dsTimer = nullptr;  // Handle
OLEDDisplay* Display::_display = nullptr;

Display::Display() {
}

Display::~Display() { 
}

void Display::init() {

    int csPin = 5;
    int dcPin    = 16;
    if (_type == DisplayType::OledSPISH1106 || _type == DisplayType::OledSPISSD1306) {
        auto spiConfig = config->_spi;
        Assert(spiConfig && spiConfig->defined(), "SPI bus is not configured. Cannot initialize SPI display.");
        auto rstPin  = _rstPin.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
        auto csPin   = _csPin.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
        auto dcPin   = _dcPin.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);

          if (_type == DisplayType::OledSPISH1106) {
            Assert(spiConfig && spiConfig->defined(), "SPI bus is not configured. Cannot initialize SPI display.");
              Display::_display = new SH1106Spi(rstPin, dcPin, csPin);
            //  SH1106Spi(uint8_t _rst, uint8_t _dc, uint8_t _cs,
        }
//        _display->connect();   //?
        if (Display::_display == nullptr) {
            log_info(" Display Init: FAILD");
            return;
        }
        log_info(" Display Init: Step1");
        
       // pinMode(rstPin, OUTPUT);
        //pinMode(csPin, OUTPUT);

        log_info(" Display Init: Step11");
        SPI.setClockDivider(SPI_CLOCK_DIV2);

        delay(1);
        log_info(" Display Init: Step111");
        digitalWrite(rstPin, LOW);
        delay(10);
        log_info(" Display Init: Step1111");
        digitalWrite(rstPin, HIGH);
        log_info(" Display Init: Step2");
        _display->init();
        log_info(" Display Init: Step22");

        Display::_display->flipScreenVertically();
        log_info(" Display Init: Step3");
        Display::_display->setFont(ArialMT_Plain_10);
        log_info(" Display Init: Step4");
       
        _display->clear();
        _display->setTextAlignment(TEXT_ALIGN_LEFT);
        _display->drawString(5, 20, "Hello world");
        _display->display();

     
        // run timer
        _dsTimer = timerBegin(_dsTimerNumber, 80, true);
        timerAttachInterrupt(_dsTimer, onDisplayUpdate, true);
        timerAlarmWrite(_dsTimer, 1000000, true);  //each 1 sec
        timerAlarmEnable(_dsTimer);

    } else {
        Assert(false, "Display type is not supported.");
    }
}

void Display::group(Configuration::HandlerBase& handler) {
    handler.item("type", _type, dsTypes);
    handler.item("rst_pin", _rstPin);
    handler.item("cs_pin", _csPin);
    handler.item("dc_pin", _dcPin);
}

void Display::afterParse() {
    log_info("Display type: " << typeName());
    if (!_rstPin.defined()) {  // validation ensures the rest is also defined.
        log_info("Display: Reset Pin (rst_pin) undefined");
    }
    if (!_csPin.defined()) {  // validation ensures the rest is also defined.
        log_info("Display: Reset _csPin (cs_pin) undefined");
    }
    if (!_dcPin.defined()) {  // validation ensures the rest is also defined.
        log_info("Display: Reset Pin (dc_pin) undefined");
    }
}

const char* Display::typeName() {
    for (size_t i = 0; dsTypes[i].name != nullptr; ++i) {
        if (dsTypes[i].value == _type)
            return dsTypes[_type].name;
    }
    return "unknown type";
}


static const char* display_state_name() {
    switch (sys.state) {
        case State::Idle:
            return "Idle";
        case State::Cycle:
            return "Run";
        case State::Hold:
            if (!(sys.suspend.bit.jogCancel)) {
                return sys.suspend.bit.holdComplete ? "Hold:0" : "Hold:1";
            }  // Continues to print jog state during jog cancel.
        case State::Jog:
            return "Jog";
        case State::JogSpeed:
            return "JogSpeed";
        case State::Homing:
            return "Home";
        case State::ConfigAlarm:
        case State::Alarm:
            return "Alarm";
        case State::CheckMode:
            return "Check";
        case State::SafetyDoor:
            if (sys.suspend.bit.initiateRestore) {
                return "Door:3";  // Restoring
            }
            if (sys.suspend.bit.retractComplete) {
                return sys.suspend.bit.safetyDoorAjar ? "Door:1" : "Door:0";
                // Door:0 means door closed and ready to resume
            }
            return "Door:2";  // Retracting
        case State::Sleep:
            return "Sleep";
    }
    return "";
}

void Display::showInfo() {
  
    static State oldInfo = State::ConfigAlarm;
    static int   oldRate = 0;
      // Report realtime feed speed
    int rate = int(Stepper::get_realtime_rate() *10);


    if (oldInfo == sys.state && oldRate == rate)
        return;

    _display->clear();
    _display->setTextAlignment(TEXT_ALIGN_LEFT);
    _display->drawString(5, 5, display_state_name());

    char buf[256];
    sprintf(buf, "Speed: %.1f", rate/10.f);
    _display->drawString(5, 20, buf);
    _display->display();
    oldInfo = sys.state;
    oldRate = rate;

}

void IRAM_ATTR Display::onDisplayUpdate() {

    if (_display == nullptr)
        return;

    rtUpdateDisplay = true;

   
   
}
