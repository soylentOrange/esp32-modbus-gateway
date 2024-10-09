#ifndef LOCALMODBUS_H
    #define LOCALMODBUS_H

    #include <WiFiManager.h>
    #include <ModbusBridgeWiFi.h>
    #include <Update.h>
    #include "config.h"

enum SubFunctionCode : uint16_t {
  RETURN_QUERY_DATA             = 0x00,
  RESTART_COMMUNICATION_OPTION  = 0x01,
};

    void setupLocalModbus(uint8_t serverID, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm);
#endif /* LOCALMODBUS_H */