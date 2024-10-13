#include "config.h"

Config::Config()
    :_prefs(NULL)
    ,_tcpPort(502)
    ,_tcpTimeout(60000)
    ,_modbusTimeout(900)
    ,_modbusBaudRate(9600)
    ,_modbusConfig(SERIAL_8N1)
    ,_modbusRtsPin(-1)
    ,_serialBaudRate(115200)
    ,_serialConfig(SERIAL_8N1)
    ,_WiFiTXPower(60)
    ,_localMbEnable(0)
    ,_localMbAddress(247)
    ,_hostname("modbusRtuGw")
    ,_coilPinCount(0)
    ,_inputPinCount(0)
    ,_coilPinsTouched(false)
    ,_inputPinsTouched(false)
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _tcpPort = _prefs->getUShort("tcpPort", _tcpPort);
    _tcpTimeout = _prefs->getULong("tcpTimeout", _tcpTimeout);
    _modbusTimeout = _prefs->getULong("modbusTimeout", _modbusTimeout);
    _modbusBaudRate = _prefs->getULong("modbusBaudRate", _modbusBaudRate);
    _modbusConfig = _prefs->getULong("modbusConfig", _modbusConfig);
    _modbusRtsPin = _prefs->getChar("modbusRtsPin", _modbusRtsPin);
    _serialBaudRate = _prefs->getULong("serialBaudRate", _serialBaudRate);
    _serialConfig = _prefs->getULong("serialConfig", _serialConfig);
    _WiFiTXPower = _prefs->getChar("txPower", _WiFiTXPower); 
    _localMbEnable = _prefs->getUChar("localMbEn", _localMbEnable); 
    _localMbAddress = _prefs->getUChar("localMbAdd", _localMbAddress); 
    _hostname = _prefs->getString("hostname", _hostname);

    // read coil configs
    _coilPins.resize(MAX_COIL_PINS, 0);
    _coilPinModes.resize(MAX_COIL_PINS, 0);
    _coilPinCount = _prefs->getBytesLength("coilPins");
    if(_coilPinCount > 0 && _coilPinCount <= MAX_COIL_PINS) {
        uint8_t* coilPinsPos = _coilPins.data();
        _prefs->getBytes("coilPins", coilPinsPos, _coilPinCount);
    } 
    #ifdef LED_BUILTIN
    // when prefs are empty use LED_BUILTIN when available
    else if(_coilPinCount == 0) {
        _coilPinCount = 1;
        _coilPins[0] = LED_BUILTIN;
    }
    #endif

    auto coilPinModesLength = _prefs->getBytesLength("coilPinModes");
    if(coilPinModesLength == _coilPinCount) {
        uint8_t* coilPinModesPos = _coilPinModes.data();
        _prefs->getBytes("coilPinModes", coilPinModesPos, _coilPinCount);
    } 
    #ifdef LED_BUILTIN
    // when prefs are empty use LED_BUILTIN when available
    else if(coilPinModesLength == 0 && _coilPinCount == 1) {
        _coilPinModes[0] = OUTPUT;
    }
    #endif
    else {
        _coilPinCount = 0;
        for(uint8_t i = 0; i < MAX_COIL_PINS; i++) {
            _coilPins[i] = 0;
            _coilPinModes[i] = 0;
        }
    }

    // get number of valid coils
    _coilPinCount = 0;
    for(uint8_t i = 0; i < MAX_COIL_PINS; i++) {
        if(_coilPinModes[i] == OUTPUT || _coilPinModes[i] == OUTPUT_OPEN_DRAIN) {
            _coilPinCount = max(_coilPinCount,  (uint8_t) (i + 1));
        } else {
            _coilPinModes[i] = 0;
        }
    }

    // read input configs
    _inputPins.resize(MAX_INPUT_PINS, 0);
    _inputPinModes.resize(MAX_INPUT_PINS, 0);
    _inputPinCount = _prefs->getBytesLength("inputPins");
    if(_inputPinCount > 0 && _inputPinCount <= MAX_INPUT_PINS) {
        uint8_t* inputPinsPos = _inputPins.data();
        _prefs->getBytes("inputPins", inputPinsPos, _inputPinCount);
    }

    auto inputPinModesLength = _prefs->getBytesLength("inputPinModes");
    if(inputPinModesLength == _inputPinCount) {
        uint8_t* inputPinModesPos = _inputPinModes.data();
        _prefs->getBytes("inputPinModes", inputPinModesPos, _inputPinCount);
    } else {
        _inputPinCount = 0;
        for(uint8_t i = 0; i < MAX_INPUT_PINS; i++) {
            _inputPins[i] = 0;
            _inputPinModes[i] = 0;
        }
    }

    // get number of valid inputs
    _inputPinCount = 0;
    for(uint8_t i = 0; i < MAX_INPUT_PINS; i++) {
        if(_inputPinModes[i] == INPUT || 
            _inputPinModes[i] == INPUT_PULLUP ||
            _inputPinModes[i] == INPUT_PULLDOWN) {
            _inputPinCount = max(_inputPinCount, (uint8_t) (i + 1));
        } else {
            _inputPinModes[i] = 0;
        }
    }
}

uint16_t Config::getTcpPort(){
    return _tcpPort;
}

void Config::setTcpPort(uint16_t value){
    if (_tcpPort == value) return;
    _tcpPort = value;
    _prefs->putUShort("tcpPort", _tcpPort);
}

uint32_t Config::getTcpTimeout(){
    return _tcpTimeout;
}

void Config::setTcpTimeout(uint32_t value){
    if (_tcpTimeout == value) return;
    _tcpTimeout = value;
    _prefs->putULong("tcpTimeout", _tcpTimeout);
}

uint32_t Config::getModbusTimeout(){
    return _modbusTimeout;
}

void Config::setModbusTimeout(uint32_t value){
    if (_modbusTimeout == value) return;
    _modbusTimeout = value;
    _prefs->putULong("modbusTimeout", _modbusTimeout);
}

uint32_t Config::getModbusConfig(){
    return _modbusConfig;
}

unsigned long Config::getModbusBaudRate(){
    return _modbusBaudRate;
}

void Config::setModbusBaudRate(unsigned long value){
    if (_modbusBaudRate == value) return;
    _modbusBaudRate = value;
    _prefs->putULong("modbusBaudRate", _modbusBaudRate);
}

uint8_t Config::getModbusDataBits(){
    return ((_modbusConfig & 0xc) >> 2) + 5;
}

void Config::setModbusDataBits(uint8_t value){
    auto dataBits = getModbusDataBits();
    value -= 5;
    value = (value << 2) & 0xc;
    if (value == dataBits) return;
    _modbusConfig = (_modbusConfig & 0xfffffff3) | value;
    _prefs->putULong("modbusConfig", _modbusConfig);
}

uint8_t Config::getModbusParity(){
    return _modbusConfig & 0x3;
}

void Config::setModbusParity(uint8_t value){
    auto parity = getModbusParity();
    value = value & 0x3;
    if (parity == value) return;
    _modbusConfig = (_modbusConfig & 0xfffffffc) | value;
    _prefs->putULong("modbusConfig", _modbusConfig);
}

uint8_t Config::getModbusStopBits(){
    return (_modbusConfig & 0x30) >> 4;
}

void Config::setModbusStopBits(uint8_t value){
    auto stopbits = getModbusStopBits();
    value = (value << 4) & 0x30;
    if (stopbits == value) return;
    _modbusConfig = (_modbusConfig & 0xffffffcf) | value;
    _prefs->putULong("modbusConfig", _modbusConfig);
}

int8_t Config::getModbusRtsPin(){
    return _modbusRtsPin;
}

void Config::setModbusRtsPin(int8_t value){
    if (_modbusRtsPin == value) return;
    _modbusRtsPin = value;
    _prefs->putChar("modbusRtsPin", _modbusRtsPin);
}

uint32_t Config::getSerialConfig(){
    return _serialConfig;
}

unsigned long Config::getSerialBaudRate(){
    return _serialBaudRate;
}

void Config::setSerialBaudRate(unsigned long value){
    if (_serialBaudRate == value) return;
    _serialBaudRate = value;
    _prefs->putULong("serialBaudRate", _serialBaudRate);
}

uint8_t Config::getSerialDataBits(){
    return ((_serialConfig & 0xc) >> 2) + 5;
}

void Config::setSerialDataBits(uint8_t value){
    auto dataBits = getSerialDataBits();
    value -= 5;
    value = (value << 2) & 0xc;
    if (value == dataBits) return;
    _serialConfig = (_serialConfig & 0xfffffff3) | value;
    _prefs->putULong("serialConfig", _serialConfig);
}

uint8_t Config::getSerialParity(){
    return _serialConfig & 0x3;
}

void Config::setSerialParity(uint8_t value){
    auto parity = getSerialParity();
    value = value & 0x3;
    if (parity == value) return;
    _serialConfig = (_serialConfig & 0xfffffffc) | value;
    _prefs->putULong("serialConfig", _serialConfig);
}

uint8_t Config::getSerialStopBits(){
    return (_serialConfig & 0x30) >> 4;
}

void Config::setSerialStopBits(uint8_t value){
    auto stopbits = getSerialStopBits();
    value = (value << 4) & 0x30;
    if (stopbits == value) return;
    _serialConfig = (_serialConfig & 0xffffffcf) | value;
    _prefs->putULong("serialConfig", _serialConfig);
}
String Config::getHostname(){
    return _hostname;
}

void Config::setHostname(String value){
     if (_hostname == value) return;
    _hostname = value;
    _prefs->putString("hostname", _hostname);
}

int8_t Config::getWiFiTXPower(){
    return _WiFiTXPower;
}

void Config::setWiFiTXPower(int8_t value){
     if (_WiFiTXPower == value) return;
    _WiFiTXPower = value;
    _prefs->putChar("txPower", _WiFiTXPower);
}

uint8_t Config::getLocalModbusAddress(){
    return _localMbAddress;
}

void Config::setLocalModbusAddress(uint8_t value){
     if (_localMbAddress == value) return;
    _localMbAddress = value;
    _prefs->putUChar("localMbAdd", _localMbAddress);
}

uint8_t Config::getLocalModbusEnable(){
    return _localMbEnable;
}

void Config::setLocalModbusEnable(uint8_t value){
     if (_localMbEnable == value) return;
    _localMbEnable = value;
    _prefs->putUChar("localMbEn", _localMbEnable);
}

uint8_t Config::getCoilPinMode(uint8_t coil) {
    if(coil >= MAX_COIL_PINS ) return 0;
    return _coilPinModes[coil];
}

uint8_t Config::getInputPinMode(uint8_t input) {
    if(input >= MAX_INPUT_PINS ) return 0;
    return _inputPinModes[input];
}

uint8_t Config::getCoilPin(uint8_t coil) {
    if(coil >= MAX_COIL_PINS ) return 0;
    return _coilPins[coil];
}

uint8_t Config::getInputPin(uint8_t input) {
    if(input >= MAX_INPUT_PINS ) return 0;
    return _inputPins[input];
}

uint8_t Config::getCoilPinCount() {
    return _coilPinCount;
}

uint8_t Config::getInputPinCount() {
    return _inputPinCount;
}

void Config::setCoilPinCount(uint8_t count) {
    if(count >= 0 && count <= MAX_COIL_PINS) {
        _coilPinCount = count;
        _coilPinsTouched = true;
        if(count < MAX_COIL_PINS) {
            for(uint8_t i = _coilPinCount; i < MAX_INPUT_PINS; i++) {
                _coilPins[i] = 0;
                _coilPinModes[i] = 0;
            }
        }
    }
}

void Config::setInputPinCount(uint8_t count) {
    if(count >= 0 && count <= MAX_INPUT_PINS) {
        _inputPinCount = count;
        _inputPinsTouched = true;
        if(count < MAX_INPUT_PINS) {
            for(uint8_t i = _inputPinCount; i < MAX_INPUT_PINS; i++) {
                _inputPins[i] = 0;
                _inputPinModes[i] = 0;
            }
        }
    }
}

void Config::setCoilPinMode(uint8_t coil, uint8_t mode) {
    if(coil >= MAX_COIL_PINS ) return;
    if(_coilPinModes[coil] == mode) return;
    _coilPinModes[coil] = mode;
    _coilPinsTouched = true;
}

void Config::setInputPinMode(uint8_t input, uint8_t mode) {
    if(input >= MAX_INPUT_PINS ) return;
    if(_inputPinModes[input] == mode) return;
    _inputPinModes[input] = mode;
    _inputPinsTouched = true;
}

void Config::setCoilPin(uint8_t coil, uint8_t pin) {
    if(coil >= MAX_COIL_PINS ) return;
    if(_coilPins[coil] == pin) return;
    _coilPins[coil] = pin;
    _coilPinsTouched = true;
}

void Config::setInputPin(uint8_t input, uint8_t pin) {
    if(input >= MAX_INPUT_PINS ) return;
    if(_inputPins[input] == pin) return;
    _inputPins[input] = pin;
    _inputPinsTouched = true;
}

void Config::saveInputs() {
    if(_inputPinsTouched) {
        uint8_t* inputPinModesPos = _inputPinModes.data();
        _prefs->putBytes("inputPinModes", inputPinModesPos, MAX_INPUT_PINS);
        uint8_t* inputPinsPos = _inputPins.data();
        _prefs->putBytes("inputPins", inputPinsPos, MAX_INPUT_PINS);
        _inputPinsTouched = false;
    }
}

void Config::saveCoils() {
    if(_coilPinsTouched) {
        uint8_t* coilPinModesPos = _coilPinModes.data();
        _prefs->putBytes("coilPinModes", coilPinModesPos, MAX_COIL_PINS);
        uint8_t* coilPinsPos = _coilPins.data();
        _prefs->putBytes("coilPins", coilPinsPos, MAX_COIL_PINS);
        _coilPinsTouched = false;
    }
}