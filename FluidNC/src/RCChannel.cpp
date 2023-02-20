#include "RCChannel.h"
#include "EnumItem.h"

  EnumItem rcTypes[] = { 
      { RCCommand::NoCmd, "nocmd" },
      { RCCommand::Move, "move" },
      { RCCommand::Rotate, "rotate" },
      { RCCommand::Left, "left" },     
      { RCCommand::Right, "right" },
      { RCCommand::GrblCmd, "grblcmd" }, 
      { RCCommand::Reset, "reset" }, 
      { RCCommand::NoCmd } };

RCChannel::RCChannel(int num) : _num(num) {}

// from min to max to -100 100
 //(x - in_min) * (out_max - out_min) / (_maxValue - _minValue) + out_min;

bool RCChannel::isOn() {
    return (isBinary() && _value == _maxValue) ? true : false;
}

void RCChannel::setValue(int newValue) {
    if (newValue < _minValue || newValue > _maxValue)
        return;

    if (abs(newValue -_value) > 2 ) {
        _value = newValue;
        _isChanged = true;
    } else
        _isChanged = false;
}

int RCChannel::normolizedValue(int* pdir) {
    int val = float(_value - _minValue) * (200.0f / float(_maxValue - _minValue)) - 100;
    if (val > 100)
        val = 100;
    else if (val < -100)
        val = -100;
    else if (val < 10 && val > -10)
        val = 0;
    if ( pdir )
        *pdir = val > 0 ? 1 : (val < 0 ? -1 : 0);
    return val;
}
    
// Configuration system helpers:
/* example config
num : 2 
min : 1000 
max : 2000 
cmd : rotate
*/

void RCChannel::group(Configuration::HandlerBase& handler)  {
    handler.item("min", _minValue);
    handler.item("max", _maxValue);
    handler.item("binary", _isBinary );
    handler.item("cmd", _type, rcTypes);
    handler.item("txt", _cmdLine, 0, Channel::maxLine);
}

void RCChannel::afterParse()
{
    log_info("Channel " << _num << " Type: " << typeName() );
}

const char* RCChannel::typeName() {
   
    for (size_t i = 0; rcTypes[i].name != nullptr; ++i) {
        if (rcTypes[i].value == _type)
            return rcTypes[_type].name;
    }
    return "unknown type";
}

void RCChannel::doLog() {
    if (isBinary()) {
        log_info("Channel " << _num << (isOn() ? " ON " : " OFF ") << (!_cmdLine.isEmpty() ? _cmdLine.c_str()  : " ") << " Value: " << _value
                            << " Type: " << typeName()
                            << " Min: " << _minValue
                            << " Max: " << _maxValue);
    }
    else
        log_info("Channel " << _num << " Value: " << _value << " Type: " << typeName() << " Min: " << _minValue << " Max: " << _maxValue);
}
