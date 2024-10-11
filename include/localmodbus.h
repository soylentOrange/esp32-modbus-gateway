#ifndef LOCALMODBUS_H
#define LOCALMODBUS_H

#include <WiFiManager.h>
#include <ModbusBridgeWiFi.h>
#include <ModbusClientRTU.h>
#include <Update.h>
#include "config.h"

// sub-function codes for FC08 (DIAGNOSTICS_SERIAL)
enum SubFunctionCode : uint16_t {
  RETURN_QUERY_DATA                         = 0x00,
  RESTART_COMMUNICATION_OPTION              = 0x01,
  RETURN_DIAGNOSTIC_REGISTER                = 0x02,
  CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER    = 0x0a,
  RETURN_BUS_MESSAGE_COUNT                  = 0x0b,
  RETURN_BUS_COMMUNICATION_ERROR_COUNT      = 0x0c,
  RETURN_SLAVE_MESSAGE_COUNT                = 0x0e,
};

void setupLocalModbus(uint8_t serverID, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm);
#endif /* LOCALMODBUS_H */