#pragma once
#include "Configuration/Configurable.h"


enum RCCommand {
    NoCmd = 0,  // Must be zero.
    Move,
    Rotate,
    Left,
    Right,
    GrblCmd,
    Reset
};

class RCChannel : public Configuration::Configurable {
public:
    RCChannel(int num);
    bool      isBinary() { return _isBinary; }
    bool      isOn();
    bool      isChanged() { return _isChanged;  }
    int       value() { return _value; }
    void      setValue(int val);
    int       normolizedValue(int *pdir = nullptr);
    int      type() { return _type; }
    const char* typeName();
    const char* getGrblCmd() { return _cmdLine.isEmpty() ? "" : _cmdLine.c_str(); }

    // Configuration system helpers:
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;
    void doLog();
    void cmdDone() {} // TODO

private:
    int       _num      = 0;
    int       _minValue = 1000;
    int       _maxValue = 2000;
    int       _value    = 0;
    bool      _isChanged = false;
    bool      _isBinary = false;
    int _type     = RCCommand::NoCmd;
    String      _cmdLine;
};
