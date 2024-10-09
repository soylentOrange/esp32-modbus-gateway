#ifndef RESTAPI_H
    #define RESTAPI_H

    #include <WiFiManager.h>
    #include <ESPAsyncWebServer.h>
    #include <ModbusBridgeWiFi.h>
    #include <ModbusClientRTU.h>
    #include <Update.h>
    #include "config.h"
    #include "debug.h"

    void setupRestApi(AsyncWebServer* server, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm);
#endif /* RESTAPI_H */