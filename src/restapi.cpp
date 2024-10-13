#include "restapi.h"
#define ETAG "\"" __DATE__ "" __TIME__ "\""
#include "AsyncJson.h"
#include <ArduinoJson.h>
AsyncCallbackJsonWebHandler* restHandler_v1_post_config;

void setupRestApi(AsyncWebServer *server, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm){

    // Handler for setting config (via HTTP_POST)
    restHandler_v1_post_config = new AsyncCallbackJsonWebHandler("/rest/v1/config", [config](AsyncWebServerRequest *request, JsonVariant &json) {
        // Get existing config
        uint16_t _tcpPort = config->getTcpPort();
        uint32_t _tcpTimeout = config->getTcpTimeout();
        uint32_t _modbusTimeout = config->getModbusTimeout();
        unsigned long _modbusBaudRate = config->getModbusBaudRate();
        uint8_t _modbusDataBits = config->getModbusDataBits();        
        uint8_t _modbusParity = config->getModbusParity();            
        uint8_t _modbusStopBits = config->getModbusStopBits();            
        int8_t _modbusRtsPin = config->getModbusRtsPin();
        unsigned long _serialBaudRate = config->getSerialBaudRate();
        uint8_t _serialDataBits = config->getSerialDataBits();
        uint8_t _serialParity = config->getSerialParity();
        uint8_t _serialStopBits = config->getSerialStopBits();
        int8_t _WiFiTXPower = config->getWiFiTXPower();
        uint8_t _localMbEnable = config->getLocalModbusEnable();
        uint8_t _localMbAddress = config->getLocalModbusAddress();
        String _hostname = config->getHostname();

        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonVariant& root = response->getRoot(); 
        uint8_t numAcceptableParameters = 0;
        uint8_t numErraneousParameters = 0;

        if(!json["tcpPort"].isNull()) {
            auto check = json["tcpPort"].as<uint16_t>();
            if(check != 80) {
                _tcpPort = check;
                root["acceptableConfig"]["tcpPort"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["tcpPort"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }    
            json.remove("tcpPort");       
            dbg("[webserver] got tcpPort: "); dbgln(check);
        }

        if(!json["tcpTimeout"].isNull()) {
            auto check = json["tcpTimeout"].as<uint32_t>();
            if(check > 0) {
                _tcpTimeout = check;
                root["acceptableConfig"]["tcpTimeout"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["tcpTimeout"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }   
            json.remove("tcpTimeout");     
            dbg("[webserver] got tcpTimeout: "); dbgln(check);
        }

        if(!json["modbusTimeout"].isNull()) {
            auto check = json["modbusTimeout"].as<uint32_t>();
            if(check > 0) {
                _modbusTimeout = check;
                root["acceptableConfig"]["modbusTimeout"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["modbusTimeout"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }   
            json.remove("modbusTimeout");     
            dbg("[webserver] got modbusTimeout: "); dbgln(check);
        }

        if(!json["modbusBaudRate"].isNull()) {
            _modbusBaudRate = json["modbusBaudRate"].as<unsigned long>();
            root["acceptableConfig"]["modbusBaudRate"] = _modbusBaudRate;
            numAcceptableParameters = numAcceptableParameters + 1;  
            json.remove("modbusBaudRate");  
            dbg("[webserver] got modbusBaudRate: "); dbgln(_modbusBaudRate);
        }

        if(!json["modbusDataBits"].isNull()) {
            auto check = json["modbusDataBits"].as<uint8_t>();
            if(check >= 5 && check <= 8) {
                _modbusDataBits = check;
                root["acceptableConfig"]["modbusDataBits"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["modbusDataBits"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }   
            json.remove("modbusDataBits");  
            dbg("[webserver] got modbusDataBits: "); dbgln(check);
        }

        if(!json["modbusParity"].isNull()) {
            auto check = json["modbusParity"].as<uint8_t>();
            if(check == 0 || check == 2 || check ==3) {
                _modbusParity = check;
                root["acceptableConfig"]["modbusParity"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["modbusParity"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }     
            json.remove("modbusParity");            
            dbg("[webserver] got modbusParity: "); dbgln(check);
        }

        if(!json["modbusStopBits"].isNull()) {
            auto check = json["modbusStopBits"].as<uint8_t>();
            if(check >= 1 && check <= 3) {
                _modbusStopBits = check;
                root["acceptableConfig"]["modbusStopBits"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["modbusStopBits"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }    
            json.remove("modbusStopBits");             
            dbg("[webserver] got modbusStopBits: "); dbgln(check);
        }

        if(!json["modbusRtsPin"].isNull()) {
            auto check = json["modbusRtsPin"].as<int8_t>();
            if(check == -1 ||   // Auto
                check == 4 ||   // GPIO4
                check == 13 ||  // GPIO13
                check == 14 ||  // GPIO14
                check == 18 ||  // GPIO18
                check == 19 ||  // GPIO19
                check == 21 ||  // GPIO21
                check == 22 ||  // GPIO22
                check == 23 ||  // GPIO23
                check == 25 ||  // GPIO25
                check == 26 ||  // GPIO26
                check == 27 ||  // GPIO27
                check == 32 ||  // GPIO32
                check == 33     // GPIO33
                ) {
                _modbusStopBits = check;
                root["acceptableConfig"]["modbusRtsPin"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["modbusRtsPin"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }     
            json.remove("modbusRtsPin");            
            dbg("[webserver] got modbusRtsPin: "); dbgln(check);
        }

        if(!json["serialBaudRate"].isNull()) {
            _serialBaudRate = json["serialBaudRate"].as<unsigned long>();
            root["acceptableConfig"]["serialBaudRate"] = _serialBaudRate;
            numAcceptableParameters = numAcceptableParameters + 1;
            json.remove("serialBaudRate");  
            dbg("[webserver] got serialBaudRate: "); dbgln(_serialBaudRate);
        }

        if(!json["serialDataBits"].isNull()) {
            auto check = json["serialDataBits"].as<uint8_t>();
            if(check >= 5 && check <= 8) {
                _serialDataBits = check;
                root["acceptableConfig"]["serialDataBits"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["serialDataBits"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }   
            json.remove("serialDataBits");  
            dbg("[webserver] got serialDataBits: "); dbgln(check);
        }

        if(!json["serialParity"].isNull()) {
            auto check = json["serialParity"].as<uint8_t>();
            if(check == 0 || check == 2 || check ==3) {
                _serialParity = check;
                root["acceptableConfig"]["serialParity"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["serialParity"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }     
            json.remove("serialParity");            
            dbg("[webserver] got serialParity: "); dbgln(check);
        }

        if(!json["serialStopBits"].isNull()) {
            auto check = json["serialStopBits"].as<uint8_t>();
            if(check >= 1 && check <= 3) {
                _serialStopBits = check;
                root["acceptableConfig"]["serialStopBits"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["serialStopBits"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }              
            json.remove("serialStopBits");   
            dbg("[webserver] got serialStopBits: "); dbgln(check);
        }

        if(!json["hostname"].isNull()) {
            auto check = json["hostname"].as<String>();
            if(check.length() > 2 && check.length() <= 63) {
                _hostname = check;
                root["acceptableConfig"]["hostname"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["hostname"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }   
            json.remove("hostname");  
            dbg("[webserver] got hostname: "); dbgln(check);
        }

        if(!json["localModbusEnable"].isNull()) {
            if(json["localModbusEnable"].as<boolean>()) {
                _localMbEnable = 1;
                config->setLocalModbusEnable(1);
            } else {
                _localMbEnable = 0;
                config->setLocalModbusEnable(0);
            }
            root["acceptableConfig"]["localModbusEnable"] = (boolean) _localMbEnable;
            numAcceptableParameters = numAcceptableParameters + 1;   
            json.remove("localModbusEnable");  
            dbg("[webserver] got localModbusEnable: "); dbgln((boolean) _localMbEnable);
        }

        if(!json["localModbusAddress"].isNull()) {
            auto check = json["localModbusAddress"].as<uint8_t>();
            if(check > 0 && check < 248) {
                _localMbAddress = check;
                root["acceptableConfig"]["localModbusAddress"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["localModbusAddress"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }      
            json.remove("localModbusAddress");  
            dbg("[webserver] got localModbusAddress: "); dbgln(check);         
        }

        if(!json["wifiTXPower"].isNull()) {
            auto check = json["wifiTXPower"].as<int8_t>();

            if(check == 8 ||    // 2dBm
                check == 20 ||  // 5dBm
                check == 28 ||  // 7dBm
                check == 34 ||  // 8.5dBm
                check == 44 ||  // 11dBm
                check == 52 ||  // 13dBm
                check == 60 ||  // 15dBm
                check == 68 ||  // 17dBm
                check == 74 ||  // 18.5dBm
                check == 76 ||  // 19dBm
                check == 78     // 19.5dBm
                ) {
                _WiFiTXPower = check;
                root["acceptableConfig"]["wifiTXPower"] = check;
                numAcceptableParameters = numAcceptableParameters + 1;
            } else {
                root["erraneousConfig"]["wifiTXPower"] = check;
                numErraneousParameters = numErraneousParameters + 1;
            }      
            json.remove("wifiTXPower");  
            dbg("[webserver] got wifiTXPower: "); dbgln(check);         
        }
        
        // check for errors
        if(numErraneousParameters == 0) {
            // no errors have occurred...
            // additionally, check for unknown parameters
            if(json.size() != 0) {
                dbgln("[webserver] unknown parameters...");
                root["unknownConfig"] = json;
                response->setLength();
                response->setCode(500);
                request->send(response);
            } else {
                // check for new parameters
                if(!root["acceptableConfig"].isNull()) {
                    root["newConfig"] = root["acceptableConfig"];
                    root.remove("acceptableConfig");  
                    dbgln("[webserver] new config saved...");
                    
                    // save config
                    config->setTcpPort(_tcpPort);
                    config->setTcpTimeout(_tcpTimeout);
                    config->setModbusTimeout(_modbusTimeout);
                    config->setModbusBaudRate(_modbusBaudRate);
                    config->setModbusDataBits(_modbusDataBits);        
                    config->setModbusParity(_modbusParity);            
                    config->setModbusStopBits(_modbusStopBits);            
                    config->setModbusRtsPin(_modbusRtsPin);
                    config->setSerialBaudRate(_serialBaudRate);
                    config->setSerialDataBits(_serialDataBits);
                    config->setSerialParity(_serialParity);
                    config->setSerialStopBits(_serialStopBits);
                    config->setWiFiTXPower(_WiFiTXPower);
                    config->setLocalModbusEnable(_localMbEnable);
                    config->setLocalModbusAddress(_localMbAddress);
                    config->setHostname(_hostname);

                    // send response
                    response->setLength();
                    response->setCode(200);
                    request->send(response);
                } else {
                    dbgln("[webserver] new config is empty...");
                    response->setLength();
                    response->setCode(400);
                    request->send(response);
                } 
            }  
            
        } else {
            // Errors have occurred while processing...
            // additionally, check for unknown parameters
            if(json.size() != 0) {
                root["unknownConfig"] = json;
            }
            dbgln("[webserver] problem with config...");
            response->setLength();
            response->setCode(500);
            request->send(response);      
        }

        // We shouldn't be here
        dbgln("[webserver] unknown problem with config...");
        response->setLength();
        response->setCode(500);
        request->send(response);
    });

    // Add config Rest-Handler to server for HTTP_POST only
    restHandler_v1_post_config->setMethod(HTTP_POST);
    server->addHandler(restHandler_v1_post_config);

    // Handler for getting config (via HTTP_GET)
  server->on("/rest/v1/config", HTTP_GET, [config](AsyncWebServerRequest *request) {
        dbgln("[Rest-API] GET /rest/v1/config");
        // Prepare resonpse
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonVariant& root = response->getRoot();
        root["tcpPort"] = config->getTcpPort();
        root["tcpTimeout"] = config->getTcpTimeout();
        root["modbusTimeout"] = config->getModbusTimeout();
        root["modbusBaudRate"] = config->getModbusBaudRate();
        root["modbusDataBits"] = config->getModbusDataBits();
        root["modbusParity"] = config->getModbusParity();
        root["modbusStopBits"] = config->getModbusStopBits();
        root["modbusRtsPin"] = config->getModbusRtsPin();
        root["serialBaudRate"] = config->getSerialBaudRate();
        root["serialDataBits"] = config->getSerialDataBits();
        root["serialParity"] = config->getSerialParity();
        root["serialStopBits"] = config->getSerialStopBits();
        root["hostname"] = config->getHostname();
        root["localModbusEnable"] = config->getLocalModbusEnable();
        root["localModbusAddress"] = config->getLocalModbusAddress();
        root["wifiTXPower"] = config->getWiFiTXPower();
        root["info"]["tcpTimeout"] = "in ms";
        root["info"]["modbusTimeout"] = "in ms";
        root["info"]["modbusDataBits"] = "5..8";
        root["info"]["modbusParity"] = "0: None, 2: Even, 3: Odd";
        root["info"]["modbusStopBits"] = "1: 1 bit, 2: 1.5 bits, 3: 2 bits";
        root["info"]["modbusRtsPin"] = "-1: Auto, X: Pin GPIOX";
        root["info"]["serialDataBits"] = "5..8";
        root["info"]["serialParity"] = "0: None, 2: Even, 3: Odd";
        root["info"]["serialStopBits"] = "1: 1 bit, 2: 1.5 bits, 3: 2 bits";
        root["info"]["hostname"] = "3-63 characters";
        root["info"]["localModbusEnable"] = "0: off, 1: on";
        root["info"]["localModbusAddress"] = "1..247";
        root["info"]["wifiTXPower"] = "8: 2dBm, 20: 5dBm, 28: 7dBm, 34: 8.5dBm, 44: 11dBm, 52: 13dBm, 60: 15dBm, 68: 17dBm, 74: 18.5dBm, 76: 19dBm, 78: 19.5dBm";
        response->setLength();
        request->send(response);
  });

    // Reboot handler (via HTTP_POST)
    server->on("/rest/v1/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        dbgln("[Rest-API] POST /rest/v1/reboot");
        request->send(200);
        ESP.restart();
    });

  // Handler for getting status (via HTTP_GET)
  server->on("/rest/v1/status", HTTP_GET, [rtu, bridge](AsyncWebServerRequest *request) {
        dbgln("[Rest-API] GET /rest/v1/status");
        // Prepare resonpse
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonVariant& root = response->getRoot();
        root["ESP"]["TxPower"]["value"] = ((float)WiFi.getTxPower())/4;
        root["ESP"]["TxPower"]["unit"] = "dBm";
        root["ESP"]["Temperature"]["value"] = temperatureRead();
        root["ESP"]["Temperature"]["unit"] = "°C";
        root["ESP"]["Uptime"]["value"] = esp_timer_get_time() / 1000000;
        root["ESP"]["Uptime"]["unit"] = "s";
        root["ESP"]["SSID"] = WiFi.SSID();
        root["ESP"]["RSSI"]["value"] = WiFi.RSSI();
        root["ESP"]["RSSI"]["unit"] = "dBm";
        root["ESP"]["MAC"] = WiFi.macAddress();
        root["ESP"]["IP"] = WiFi.localIP().toString();
        root["RTU"]["Messages"]["value"] = rtu->getMessageCount();
        root["RTU"]["Messages"]["unit"] = "";
        root["RTU"]["PendingMessages"]["value"] = rtu->pendingRequests();
        root["RTU"]["PendingMessages"]["unit"] = "";
        root["RTU"]["Errors"]["value"] = rtu->getErrorCount();
        root["RTU"]["Errors"]["unit"] = "";
        root["Bridge"]["Messages"]["value"] = bridge->getMessageCount();
        root["Bridge"]["Messages"]["unit"] = "";
        root["Bridge"]["Clients"]["value"] = bridge->activeClients();
        root["Bridge"]["Clients"]["unit"] = "";
        root["Bridge"]["Errors"]["value"] = bridge->getErrorCount();
        root["Bridge"]["Errors"]["unit"] = "";
        root["Info"]["BuildTime"] = __DATE__ " " __TIME__;
        response->setLength();
        request->send(response);
  });
}