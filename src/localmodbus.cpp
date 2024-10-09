#include "localmodbus.h"
#define ETAG "\"" __DATE__ "" __TIME__ "\""

static ModbusClientRTU* _rtu = NULL;
static ModbusBridgeWiFi* _bridge = NULL;


// FC03: worker do serve Modbus function code 0x03 (READ_HOLD_REGISTER)
ModbusMessage FC03(ModbusMessage request) {
    uint16_t address;           // requested register address
    uint16_t words;             // requested number of registers
    ModbusMessage response;     // response message to be sent back

    LOG_D("WORKER CALLED FC03");

    // get request values
    request.get(2, address);
    request.get(4, words);

    // Address and words valid? We assume 10 registers here for demo
    if (address && words && (address + words) <= 10) {
        // Looks okay. Set up message with serverID, FC and length of data
        response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
        // Fill response with requested data
        for (uint16_t i = address; i < address + words; ++i) {
        response.add(i);
        }
    } else {
        // No, either address or words are outside the limits. Set up error response.
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    }
    return response;
}

// FC08: worker do serve Modbus function code 0x08 (DIAGNOSTICS_SERIAL)
ModbusMessage FC08(ModbusMessage request) {
    uint16_t subFunctionCode;   // Sub-function code
    ModbusMessage response;     // response message to be sent back
    uint16_t resultWord = 0;

    LOG_D("WORKER CALLED FC08\n");

    // get request values
    request.get(2, subFunctionCode);

    switch(subFunctionCode) {
        case RETURN_QUERY_DATA:
            LOG_D("Return Query Data\n");
            response = request; 
            break;
        case RESTART_COMMUNICATION_OPTION:
            LOG_D("Restart Communications Option\n");
            response = request; 
            break;
        case CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER:
            LOG_D("Restart Communications Option\n");
            if(_rtu != NULL) {
                _rtu->resetCounts();
            }
            if(_bridge != NULL) {
                _bridge->resetCounts();
            }
            response = request; 
            break;
        case RETURN_BUS_MESSAGE_COUNT:
            LOG_D("Return Bus Message Count\n");
            if(_bridge != NULL) {
                resultWord = _bridge->getMessageCount();
            }
            response = ModbusMessage(request.getServerID(), DIAGNOSTICS_SERIAL, RETURN_BUS_MESSAGE_COUNT, resultWord);
            break;
        case RETURN_BUS_COMMUNICATION_ERROR_COUNT:
            LOG_D("Return Bus Communication Error Count\n");
            if(_bridge != NULL) {
                resultWord = _bridge->getErrorCount();
            }
            response = ModbusMessage(request.getServerID(), DIAGNOSTICS_SERIAL, RETURN_BUS_COMMUNICATION_ERROR_COUNT, resultWord);
            break;
        case RETURN_SLAVE_MESSAGE_COUNT:
            LOG_D("Return Slave Message Count\n");
            if(_rtu != NULL) {
                resultWord = _rtu->getMessageCount();
            }
            response = ModbusMessage(request.getServerID(), DIAGNOSTICS_SERIAL, RETURN_SLAVE_MESSAGE_COUNT, resultWord);
            break;
        default: 
            LOG_D("default: ILLEGAL_FUNCTION\n");
            response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_FUNCTION);
    }

    return response;
}

void setupLocalModbus(uint8_t serverID, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm) {
    String EfuseMac = String(ESP.getEfuseMac(), 16);
    dbgln(EfuseMac);
    _rtu = rtu;
    _bridge = bridge;
    bridge->registerWorker(serverID, READ_HOLD_REGISTER, &FC03);
    bridge->registerWorker(serverID, DIAGNOSTICS_SERIAL, &FC08);
    //bridge->registerWorker(serverID, REPORT_SERVER_ID_SERIAL, &FC08);
}