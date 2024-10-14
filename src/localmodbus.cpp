#include "localmodbus.h"
#define ETAG "\"" __DATE__ "" __TIME__ "\""

static const uint16_t COIL_ON = 0xFF00;
static const uint16_t COIL_OFF = 0;

static ModbusClientRTU* _rtu = NULL;
static ModbusBridgeWiFi* _bridge = NULL;
static Config* _config = NULL;
static uint8_t _serverID;

// FreeRTOS-task to pause some time and then reboot
void rebootAfterDelay(void * parameter){
    vTaskDelay((uint32_t) parameter / portTICK_PERIOD_MS);
    ESP.restart();
    vTaskDelete(NULL);  // will probably not be reached...
}

// FC01: worker do serve Modbus function code 0x01 (READ_COIL)
ModbusMessage FC01(ModbusMessage request) {
    uint16_t address;           // requested starting address
    uint16_t coils;             // requested number of coils
    ModbusMessage response;     // response message to be sent back
    uint8_t coilPinCount;       // number of available Coil-pins
    std::vector<uint8_t> responseData;

    LOG_D("Worker for FC01\n");

    // are we ready to serve
    if(_config == NULL) {
        response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_FAILURE);
        return response;
    }

    // get request values
    request.get(2, address);
    request.get(4, coils);    

    // check request
    coilPinCount = _config->getCoilPinCount();
    if((address + coils) > coilPinCount) {
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
        return response;
    }

    // read coils pins starting from address
    // Coils are addressed starting at zero. Therefore coil numbered 1 is addressed as 0
    for(size_t i = 0; i < (coils/8 + coils%8 ? 1 : 0); i++) {
        uint8_t responseDataByte = 0;
        uint8_t coilsToRead = coils % 8;
        if(coils >= 8) coilsToRead = 8;

        for(size_t i = 0; i < coilsToRead; i++) {
            if(digitalRead(_config->getCoilPin(i + address))) {
                responseDataByte |= 1 << i;
            }
        }

        coils -= 8;
        responseData.push_back(responseDataByte);
    }

    // fill and return response message
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(coils/8 + coils%8 ? 1 : 0));
    response.add(responseData);
    return response;
}

// FC02: worker do serve Modbus function code 0x02 (READ_DISCR_INPUT)
ModbusMessage FC02(ModbusMessage request) {
    uint16_t address;           // requested starting address
    uint16_t inputs;            // requested number of inputs
    ModbusMessage response;     // response message to be sent back
    uint8_t inputPinCount;      // number of available Input-pins
    std::vector<uint8_t> responseData;

    LOG_D("Worker for FC02\n");

    // are we ready to serve
    if(_config == NULL) {
        response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_FAILURE);
        return response;
    }

    // get request values
    request.get(2, address);
    request.get(4, inputs);    

    // check request
    inputPinCount = _config->getInputPinCount();
    if((address + inputs) > inputPinCount) {
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
        return response;
    }

    // read discrete input pins starting from address
    // inputs are addressed starting at zero. Therefore input numbered 1 is addressed as 0
    for(size_t i = 0; i < (inputs/8 + inputs%8 ? 1 : 0); i++) {
        uint8_t responseDataByte = 0;
        uint8_t inputsToRead = inputs % 8;
        if(inputs >= 8) inputsToRead = 8;

        for(size_t i = 0; i < inputsToRead; i++) {
            if(digitalRead(_config->getInputPin(i + address))) {
                responseDataByte |= 1 << i;
            }
        }

        inputs -= 8;
        responseData.push_back(responseDataByte);
    }

    // fill and return response message
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(inputs/8 + inputs%8 ? 1 : 0));
    response.add(responseData);
    return response;
}

// FC03: worker do serve Modbus function code 0x03 (READ_HOLD_REGISTER)
// Gets the current temperature (from address 0x00 with a length of 4 Bytes)
ModbusMessage FC03(ModbusMessage request) {
    uint16_t address;           // requested register address
    uint16_t words;             // requested number of registers
    ModbusMessage response;     // response message to be sent back

    LOG_D("Worker for FC03\n");

    // get request values
    request.get(2, address);
    request.get(4, words);

    // return current temperature
    if (address == 0 && words == 2) {
        // Looks okay. Set up message with serverID, FC and length of data
        response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
        // add current temperature to response
        response.add(temperatureRead());
    } else {
        // Set up error response.
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    }
    return response;
}

// FC04: worker do serve Modbus function code 0x04 (READ_INPUT_REGISTER)
// Gets the current temperature (from address 0x00 with a length of 4 Bytes)
ModbusMessage FC04(ModbusMessage request) {
    uint16_t address;           // requested register address
    uint16_t words;             // requested number of registers
    ModbusMessage response;     // response message to be sent back

    LOG_D("Worker for FC04\n");

    // get request values
    request.get(2, address);
    request.get(4, words);

    // return current temperature
    if (address == 0 && words == 2) {
        // Looks okay. Set up message with serverID, FC and length of data
        response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
        // add current temperature to response
        response.add(temperatureRead());
    } else {
        // Set up error response.
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    }
    return response;
}

// FC05: worker do serve Modbus function code 0x05 (WRITE_COIL)
ModbusMessage FC05(ModbusMessage request) {
    uint16_t address;           // requested coil
    uint16_t coilState;         // requested state of coil
    ModbusMessage response;     // response message to be sent back
    uint8_t coilPinCount;       // number of available Coil-pins

    LOG_D("Worker for FC05\n");

    // are we ready to serve?
    if(_config == NULL) {
        response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_FAILURE);
        return response;
    }

    // get request values
    request.get(2, address);
    request.get(4, coilState);

    // is the requested coil-state valid?
    if(coilState != COIL_ON && coilState != COIL_OFF) {
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
        return response;
    }    

    // check request
    coilPinCount = _config->getCoilPinCount();
    if((address) >= coilPinCount) {
        response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
        return response;
    }

    // write the coil
    // Coils are addressed starting at zero. Therefore coil numbered 1 is addressed as 0
    if(coilState == COIL_ON) {
        if(_config->getCoilPinMode((uint8_t) address) == OUTPUT || 
        _config->getCoilPinMode((uint8_t) address) == OUTPUT_OPEN_DRAIN) {
            digitalWrite(_config->getCoilPin((uint8_t) address), HIGH);
        }
    } else {
        if(_config->getCoilPinMode((uint8_t) address) == OUTPUT || 
        _config->getCoilPinMode((uint8_t) address) == OUTPUT_OPEN_DRAIN) {
            digitalWrite(_config->getCoilPin((uint8_t) address), LOW);
        }
    }

    // echo the request message
    return request;
}

// FC08: worker do serve Modbus function code 0x08 (DIAGNOSTICS_SERIAL)
ModbusMessage FC08(ModbusMessage request) {
    uint16_t subFunctionCode;   // Sub-function code
    ModbusMessage response;     // response message to be sent back
    uint16_t resultWord = 0;    // response data to be sent back

    LOG_D("Worker for FC08\n");

    // are we ready to serve
    if(_rtu == NULL || _bridge == NULL) {
        response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_FAILURE);
        return response;
    }

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
            // create a task to reboot
            xTaskCreate(rebootAfterDelay, "rebootAfterDelay", configMINIMAL_STACK_SIZE, 
                (void*) 1000, // delay for 1000 ms
                1, NULL);
            break;
        case CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER:
            LOG_D("Clear Counters and Diagnostic Register\n");
            _rtu->resetCounts();
            _bridge->resetCounts();
            response = request; 
            break;
        case RETURN_BUS_MESSAGE_COUNT:
            LOG_D("Return Bus Message Count\n");
            resultWord = (uint16_t) _bridge->getMessageCount();
            response = ModbusMessage(request.getServerID(), DIAGNOSTICS_SERIAL, RETURN_BUS_MESSAGE_COUNT, resultWord);
            break;
        case RETURN_BUS_COMMUNICATION_ERROR_COUNT:
            LOG_D("Return Bus Communication Error Count\n");
            resultWord = (uint16_t) _bridge->getErrorCount();
            response = ModbusMessage(request.getServerID(), DIAGNOSTICS_SERIAL, RETURN_BUS_COMMUNICATION_ERROR_COUNT, resultWord);
            break;
        case RETURN_SLAVE_MESSAGE_COUNT:
            LOG_D("Return Slave Message Count\n");
            resultWord = (uint16_t) _rtu->getMessageCount();
            response = ModbusMessage(request.getServerID(), DIAGNOSTICS_SERIAL, RETURN_SLAVE_MESSAGE_COUNT, resultWord);
            break;
        default: 
            LOG_D("default: ILLEGAL_FUNCTION\n");
            response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_FUNCTION);
    }

    return response;
}

void setupLocalModbus(uint8_t serverID, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm) {
    _rtu = rtu;
    _bridge = bridge;
    _config = config;
    _serverID = serverID;
    //bridge->registerWorker(serverID, READ_HOLD_REGISTER, &FC03);
    bridge->registerWorker(serverID, READ_INPUT_REGISTER, &FC04);
    bridge->registerWorker(serverID, DIAGNOSTICS_SERIAL, &FC08);
}

void enableCoilWorkers() {
    _bridge->registerWorker(_serverID, READ_COIL, &FC01);
    _bridge->registerWorker(_serverID, WRITE_COIL, &FC05);
}

void enableInputWorkers() {
    _bridge->registerWorker(_serverID, READ_DISCR_INPUT, &FC02);
}