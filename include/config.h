#ifndef CONFIG_H
    #define CONFIG_H
    #include <Preferences.h>                   
    #include <vector>
    #define debugSerial Serial
    #define modbusSerial Serial2
    #define MAX_INPUT_PINS 8
    #define MAX_COIL_PINS 8
    #define DEBUG

    class Config{
        private:
            Preferences *_prefs;
            uint16_t _tcpPort;
            uint32_t _tcpTimeout;
            uint32_t _modbusTimeout;
            unsigned long _modbusBaudRate;
            uint32_t _modbusConfig;
            int8_t _modbusRtsPin;
            unsigned long _serialBaudRate;
            uint32_t _serialConfig;
            int8_t _WiFiTXPower;
            uint8_t _localMbEnable;
            uint8_t _localMbAddress;
            String _hostname;
            uint8_t _coilPinCount;
            uint8_t _inputPinCount;
            bool _coilPinsTouched;
            bool _inputPinsTouched;
            std::vector<uint8_t> _coilPins; 
            std::vector<uint8_t> _coilPinModes; 
            std::vector<uint8_t> _inputPins; 
            std::vector<uint8_t> _inputPinModes; 
        public:
            Config();
            void begin(Preferences *prefs);
            uint16_t getTcpPort();
            void setTcpPort(uint16_t value);
            uint32_t getTcpTimeout();
            void setTcpTimeout(uint32_t value);
            uint32_t getModbusTimeout();
            void setModbusTimeout(uint32_t value);
            uint32_t getModbusConfig();
            unsigned long getModbusBaudRate();
            void setModbusBaudRate(unsigned long value);
            uint8_t getModbusDataBits();
            void setModbusDataBits(uint8_t value);
            uint8_t getModbusParity();
            void setModbusParity(uint8_t value);
            uint8_t getModbusStopBits();
            void setModbusStopBits(uint8_t value);
            int8_t getModbusRtsPin();
            void setModbusRtsPin(int8_t value);
            uint32_t getSerialConfig();
            unsigned long getSerialBaudRate();
            void setSerialBaudRate(unsigned long value);
            uint8_t getSerialDataBits();
            void setSerialDataBits(uint8_t value);
            uint8_t getSerialParity();
            void setSerialParity(uint8_t value);
            uint8_t getSerialStopBits();
            void setSerialStopBits(uint8_t value);
            int8_t getWiFiTXPower();
            void setWiFiTXPower(int8_t value);
            uint8_t getLocalModbusEnable();
            void setLocalModbusEnable(uint8_t value);
            uint8_t getLocalModbusAddress();
            void setLocalModbusAddress(uint8_t value);
            String getHostname();
            void setHostname(String value);
            uint8_t getCoilPinMode(uint8_t coil);
            uint8_t getInputPinMode(uint8_t input);
            uint8_t getCoilPin(uint8_t coil);
            uint8_t getInputPin(uint8_t input);
            uint8_t getCoilPinCount();
            uint8_t getInputPinCount();
            void setCoilPinCount(uint8_t count);
            void setInputPinCount(uint8_t count);
            void setCoilPinMode(uint8_t coil, uint8_t mode);
            void setInputPinMode(uint8_t input, uint8_t mode);
            void setCoilPin(uint8_t coil, uint8_t pin);
            void setInputPin(uint8_t input, uint8_t pin);
            void saveInputs();
            void saveCoils();
    };
    #ifdef DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else /* DEBUG */
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif /* DEBUG */
#endif /* CONFIG_H */