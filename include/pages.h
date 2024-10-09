#ifndef PAGES_H
    #define PAGES_H

    #include <WiFiManager.h>
    #include <ESPAsyncWebServer.h>
    #include <ModbusBridgeWiFi.h>
    #include <ModbusClientRTU.h>
    #include <Update.h>
    #include "config.h"
    #include "debug.h"

    void setupPages(AsyncWebServer* server, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm);
    void sendResponseHeader(AsyncResponseStream *response, const char *title, bool inlineStyle = false);
    void sendResponseTrailer(AsyncResponseStream *response);
    void sendButton(AsyncResponseStream *response, const char *title, const char *action, const char *css = "");
    void sendTableRow(AsyncResponseStream *response, const char *name, uint32_t value);
    void sendTableRow(AsyncResponseStream *response, const char *name, String value);
    void sendDebugForm_read(AsyncResponseStream *response, String slaveId, String reg, String function, String count);
    void sendDebugForm_diagnosticSerial(AsyncResponseStream *response, String slaveId, String subFunction, String data);
    void sendMinCss(AsyncResponseStream *response);
    const String ErrorName(Modbus::Error code);
    const String WiFiQuality(int rssiValue);
#endif /* PAGES_H */