#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>
    #define debugSerial Serial
    #define modbusSerial Serial2
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
    };
    #ifdef DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else /* DEBUG */
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif /* DEBUG */
#endif /* CONFIG_H */