#include "pages.h"
#include "localmodbus.h"
#define ETAG "\"" __DATE__ "" __TIME__ "\""

// Remember last slave-ID
static uint8_t _lastSlaveID;

void setupPages(AsyncWebServer *server, ModbusClientRTU *rtu, ModbusBridgeWiFi *bridge, Config *config, WiFiManager *wm){
  
  _lastSlaveID = 1;

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Main");
    sendButton(response, "Status", "status");
    sendButton(response, "Config", "config");
    sendButton(response, "Debug", "debug");
    sendButton(response, "Firmware update", "update");
    sendButton(response, "WiFi reset", "wifi", "r");
    sendButton(response, "Reboot", "reboot", "r");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/status", HTTP_GET, [rtu, bridge](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /status");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Status");
    response->print("<table>");

    // show ESP infos...
    sendTableRow(response, "WiFi TX Power (dBm)", String(((float)WiFi.getTxPower())/4, 2));
    sendTableRow(response, "ESP Temperature (°C)", String(temperatureRead(), 2));
    sendTableRow(response, "ESP Uptime (sec)", esp_timer_get_time() / 1000000);
    sendTableRow(response, "ESP SSID", WiFi.SSID());
    sendTableRow(response, "ESP RSSI", WiFi.RSSI());
    sendTableRow(response, "ESP WiFi Quality", WiFiQuality(WiFi.RSSI()));
    sendTableRow(response, "ESP MAC", WiFi.macAddress());
    sendTableRow(response, "ESP IP",  WiFi.localIP().toString() );
    sendTableRow(response, "RTU Messages", rtu->getMessageCount());
    sendTableRow(response, "RTU Pending Messages", rtu->pendingRequests());
    sendTableRow(response, "RTU Errors", rtu->getErrorCount());
    sendTableRow(response, "Bridge Message", bridge->getMessageCount());
    sendTableRow(response, "Bridge Clients", bridge->activeClients());
    sendTableRow(response, "Bridge Errors", bridge->getErrorCount());
    response->print("<tr><td>&nbsp;</td><td></td></tr>");
    sendTableRow(response, "Build time", __DATE__ " " __TIME__);
    response->print("</table><p></p>");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /reboot");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Really?");
    sendButton(response, "Back", "/");
    response->print("<form method=\"post\">"
        "<button class=\"r\">Yes, do it!</button>"
      "</form>");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /reboot");
    request->redirect("/");
    dbgln("[webserver] rebooting...");
    ESP.restart();
    dbgln("[webserver] rebooted...");
  });

  server->on("/config", HTTP_GET, [config](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /config");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Wifi Settings");
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"hn\">Hostname</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"text\" minlength=\"2\" maxlength=\"63\" id=\"hn\" name=\"hn\" value=\"%s\">", config->getHostname().length() > 2 ? config->getHostname().c_str() : WiFi.getHostname());
    response->print("</tr>"
    "</tr>"
      "<tr>"
        "<td>"
          "<label for=\"tx\">TX Power</label>"
        "</td>"
        "<td>");
    response->printf("<select id=\"tx\" name=\"tx\" data-value=\"%d\">", config->getWiFiTXPower());
    response->print("<option value=\"78\">19.5dBm</option>"
                    "<option value=\"76\">19dBm</option>"
                    "<option value=\"74\">18.5dBm</option>"
                    "<option value=\"68\">17dBm</option>"
                    "<option value=\"60\">15dBm</option>"
                    "<option value=\"52\">13dBm</option>"
                    "<option value=\"44\">11dBm</option>"
                    "<option value=\"34\">8.5dBm</option>"
                    "<option value=\"28\">7dBm</option>"
                    "<option value=\"20\">5dBm</option>"
                    "<option value=\"8\">2dBm</option>"
        "</select>"
      "</td>"
    "</tr>"
    "</table>"
    "<h3>Modbus TCP</h3>"
    "<table>"
      "<tr>"
        "<td>"
          "<label for=\"tp\">TCP Port</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" max=\"65535\" id=\"tp\" name=\"tp\" value=\"%d\">", config->getTcpPort());
    response->print("</td>"
      "</tr>"
      "<tr>"
        "<td>"
          "<label for=\"tt\">TCP Timeout (ms)</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" id=\"tt\" name=\"tt\" value=\"%d\">", config->getTcpTimeout());
    response->print("</td>"
        "</tr>"
        "</table>"
        "<h3>Local Modbus Server</h3>"
        "<table>"
          "<tr>"
            "<td>"
              "<label for=\"re\">Enable local Modbus Server</label>"
            "</td>"
            "<td>");
    response->printf("<select id=\"re\" name=\"re\" data-value=\"%d\">", config->getLocalModbusEnable());  
    response->print("<option value=\"0\">disabled</option>"
              "<option value=\"1\">enabled</option>"
              "</select>"
            "</td>"
          "</tr>"
          "<tr>"
            "<td>"
              "<label for=\"ra\">Local Modbus Address</label>"
            "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"1\" max=\"247\" id=\"ra\" name=\"ra\" value=\"%d\">", config->getLocalModbusAddress());
    response->print("</td>"
          "</tr>"
        "</table>"
        "<h3>Modbus RTU</h3>"
        "<table>"
        "<tr>"
          "<td>"
            "<label for=\"mb\">Baud rate</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"0\" id=\"mb\" name=\"mb\" value=\"%lu\">", config->getModbusBaudRate());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"md\">Data bits</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"5\" max=\"8\" id=\"md\" name=\"md\" value=\"%d\">", config->getModbusDataBits());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"mp\">Parity</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"mp\" name=\"mp\" data-value=\"%d\">", config->getModbusParity());
    response->print("<option value=\"0\">None</option>"
              "<option value=\"2\">Even</option>"
              "<option value=\"3\">Odd</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"ms\">Stop bits</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"ms\" name=\"ms\" data-value=\"%d\">", config->getModbusStopBits());
    response->print("<option value=\"1\">1 bit</option>"
              "<option value=\"2\">1.5 bits</option>"
              "<option value=\"3\">2 bits</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"mr\">RTS Pin</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"mr\" name=\"mr\" data-value=\"%d\">", config->getModbusRtsPin());
    response->print("<option value=\"-1\">Auto</option>"
              "<option value=\"4\">D4</option>"
              "<option value=\"13\">D13</option>"
              "<option value=\"14\">D14</option>"
              "<option value=\"18\">D18</option>"
              "<option value=\"19\">D19</option>"
              "<option value=\"21\">D21</option>"
              "<option value=\"22\">D22</option>"
              "<option value=\"23\">D23</option>"
              "<option value=\"25\">D25</option>"
              "<option value=\"26\">D26</option>"
              "<option value=\"27\">D27</option>"
              "<option value=\"32\">D32</option>"
              "<option value=\"33\">D33</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
        "<td>"
          "<label for=\"mt\">Modbus Timeout (ms)</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" id=\"mt\" name=\"mt\" value=\"%d\">", config->getModbusTimeout());
    response->print("</td>"
        "</tr>"
        "</table>"
        "<h3>Serial (Debug)</h3>"
        "<table>"
        "<tr>"
          "<td>"
            "<label for=\"sb\">Baud rate</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"0\" id=\"sb\" name=\"sb\" value=\"%lu\">", config->getSerialBaudRate());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"sd\">Data bits</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"5\" max=\"8\" id=\"sd\" name=\"sd\" value=\"%d\">", config->getSerialDataBits());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"sp\">Parity</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"sp\" name=\"sp\" data-value=\"%d\">", config->getSerialParity());
    response->print("<option value=\"0\">None</option>"
              "<option value=\"2\">Even</option>"
              "<option value=\"3\">Odd</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"ss\">Stop bits</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"ss\" name=\"ss\" data-value=\"%d\">", config->getSerialStopBits());
    response->print("<option value=\"1\">1 bit</option>"
              "<option value=\"2\">1.5 bits</option>"
              "<option value=\"3\">2 bits</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr><td>&nbsp;</td><td></td></tr>"
        "</table>");   
    response->print("<button class=\"r\">Save</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
    response->print("<script>"
      "(function(){"
        "var s = document.querySelectorAll('select[data-value]');"
        "for(d of s){"
          "d.querySelector(`option[value='${d.dataset.value}']`).selected=true"
      "}})();"
      "</script>");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/config", HTTP_POST, [config](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /config");
    if (request->hasParam("tp", true)){
      auto port = request->getParam("tp", true)->value().toInt();
      config->setTcpPort(port);
      dbg("[webserver] saved port: "); dbgln(port);
    }
    if (request->hasParam("tt", true)){
      auto timeout = request->getParam("tt", true)->value().toInt();
      config->setTcpTimeout(timeout);
      dbg("[webserver] saved tcp timeout: "); dbgln(timeout);
    }
    if (request->hasParam("mb", true)){
      auto baud = request->getParam("mb", true)->value().toInt();
      config->setModbusBaudRate(baud);
      dbg("[webserver] saved modbus baud rate: "); dbgln(baud);
    }
    if (request->hasParam("md", true)){
      auto data = request->getParam("md", true)->value().toInt();
      config->setModbusDataBits(data);
      dbg("[webserver] saved modbus data bits: "); dbgln(data);
    }
    if (request->hasParam("mp", true)){
      auto parity = request->getParam("mp", true)->value().toInt();
      config->setModbusParity(parity);
      dbg("[webserver] saved modbus parity: "); dbgln(parity);
    }
    if (request->hasParam("ms", true)){
      auto stop = request->getParam("ms", true)->value().toInt();
      config->setModbusStopBits(stop);
      dbg("[webserver] saved modbus stop bits: "); dbgln(stop);
    }
    if (request->hasParam("mr", true)){
      auto rts = request->getParam("mr", true)->value().toInt();
      config->setModbusRtsPin(rts);
      dbg("[webserver] saved modbus rts pin: "); dbgln(rts);
    }
    if (request->hasParam("mt", true)){
      auto timeout = request->getParam("mt", true)->value().toInt();
      config->setModbusTimeout(timeout);
      dbg("[webserver] saved Modbus timeout: "); dbgln(timeout);
    }
    if (request->hasParam("sb", true)){
      auto baud = request->getParam("sb", true)->value().toInt();
      config->setSerialBaudRate(baud);
      dbg("[webserver] saved serial baud rate: "); dbgln(baud);
    }
    if (request->hasParam("sd", true)){
      auto data = request->getParam("sd", true)->value().toInt();
      config->setSerialDataBits(data);
      dbg("[webserver] saved serial data bits: "); dbgln(data);
    }
    if (request->hasParam("sp", true)){
      auto parity = request->getParam("sp", true)->value().toInt();
      config->setSerialParity(parity);
      dbg("[webserver] saved serial parity: "); dbgln(parity);
    }
    if (request->hasParam("ss", true)){
      auto stop = request->getParam("ss", true)->value().toInt();
      config->setSerialStopBits(stop);
      dbg("[webserver] saved serial stop bits: "); dbgln(stop);
    }
    if (request->hasParam("hn", true)){
      auto hostname = request->getParam("hn", true)->value();
      config->setHostname(hostname);
      dbg("[webserver] saved hostname: ");
      dbgln(config->getHostname());
    }
    if (request->hasParam("tx", true)){
      auto txPower = request->getParam("tx", true)->value().toInt();
      config->setWiFiTXPower((int8_t)txPower);
      dbg("[webserver] saved TX Power: ");
      dbg(((float)config->getWiFiTXPower())/4);
      dbgln("dBm");
    }        
    if (request->hasParam("re", true)){
      auto enabled = request->getParam("re", true)->value().toInt();
      config->setLocalModbusEnable((uint8_t) enabled);
      dbg("[webserver] saved local Modbus server enable: ");
      dbgln((uint8_t) enabled);
    }
    if (request->hasParam("ra", true)){
      auto address = request->getParam("ra", true)->value().toInt();
      config->setLocalModbusAddress((uint8_t) address);
      dbg("[webserver] saved local Modbus server address: ");
      dbgln((uint8_t) address);
    }
    request->redirect("/");    
  });

  server->on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /debug");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Modbus Debug");
    sendButton(response, "Read (FC01 - FC04)", "debug_read");
    sendButton(response, "Diagnostic Serial (FC08)", "debug_diagnosticSerial");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/debug_read", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /debug_read");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Modbus Debug - Read from Device");
    sendDebugForm_read(response, String(_lastSlaveID), "1", "3", "1");
    sendButton(response, "Back", "debug");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/debug_diagnosticSerial", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /debug_diagnosticSerial");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Modbus Debug - Diagnostic Serial");
    sendDebugForm_diagnosticSerial(response, String(_lastSlaveID), "0", "0000");
    sendButton(response, "Back", "debug");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/debug_read", HTTP_POST, [config, rtu, bridge](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /debug_read");
    String slaveId = "1";
    if (request->hasParam("slave", true)){
      slaveId = request->getParam("slave", true)->value();
      _lastSlaveID = (uint8_t) slaveId.toInt();
    }
    String reg = "1";
    if (request->hasParam("reg", true)){
      reg = request->getParam("reg", true)->value();
    }
    String func = "3";
    if (request->hasParam("func", true)){
      func = request->getParam("func", true)->value();
    }
    String count = "1";
    if (request->hasParam("count", true)){
      count = request->getParam("count", true)->value();
    }
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Modbus Debug - Read from Device");
    response->print("<pre>");
    auto previous = LOGDEVICE;
    auto previousLevel = MBUlogLvl;
    auto debug = WebPrint(previous, response);
    LOGDEVICE = &debug;
    MBUlogLvl = LOG_LEVEL_DEBUG;
    ModbusMessage answer;
    // Call local worker (when enabled)
    if(slaveId.toInt() == config->getLocalModbusAddress() && config->getLocalModbusEnable()) {
      MBSworker lclWrkr = bridge->getWorker(slaveId.toInt(), func.toInt());
      if(lclWrkr != NULL) {
        LOG_D("found local worker... calling for response\n");
        answer = lclWrkr(ModbusMessage(slaveId.toInt(), func.toInt(), reg.toInt(), count.toInt()));
      } else {
        LOG_D("no local worker found... answering with ILLEGAL_FUNCTION\n");
        answer.setError(slaveId.toInt(), func.toInt(), ILLEGAL_FUNCTION);
      }
    } else {
      // Call via RTU
      answer = rtu->syncRequest(0xdeadbeef, slaveId.toInt(), func.toInt(), reg.toInt(), count.toInt());
    }    
    MBUlogLvl = previousLevel;
    LOGDEVICE = previous;
    response->print("</pre>");
    auto error = answer.getError();
    if (error == SUCCESS){
      auto count = answer[2];
      response->print("<span >Answer: 0x");
      for (size_t i = 0; i < count; i++)
      {
        response->printf("%02x", answer[i + 3]);
      }      
      response->print("</span>");
    }
    else{
      response->printf("<span class=\"e\">Error: %#02x (%s)</span>", error, ErrorName(error).c_str());
    }
    sendDebugForm_read(response, slaveId, reg, func, count);
    sendButton(response, "Back", "debug");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/debug_diagnosticSerial", HTTP_POST, [config, rtu, bridge](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /debug_diagnosticSerial");
    String slaveId = "1";
    if (request->hasParam("slave", true)){
      slaveId = request->getParam("slave", true)->value();
      _lastSlaveID = (uint8_t) slaveId.toInt();
    }
    String subFunction = "0";
    if (request->hasParam("sf", true)){
      subFunction = request->getParam("sf", true)->value();
    }
    String data = "0000";
    if (request->hasParam("dt", true)){
      data = request->getParam("dt", true)->value();
    }

    // convert data string to WORD
    uint16_t dataWord = 0;
    if(data.length() != 0) {
      dataWord = (uint16_t) strtol(data.c_str(), NULL, 16);
    } 
    
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Modbus Debug - Diagnostic Serial");
    response->print("<pre>");
    auto previous = LOGDEVICE;
    auto previousLevel = MBUlogLvl;
    auto debug = WebPrint(previous, response);
    LOGDEVICE = &debug;
    MBUlogLvl = LOG_LEVEL_DEBUG;
    ModbusMessage answer;
    // Call local worker (when enabled)
    if(slaveId.toInt() == config->getLocalModbusAddress() && config->getLocalModbusEnable()) {
      MBSworker lclWrkr = bridge->getWorker(slaveId.toInt(), DIAGNOSTICS_SERIAL);
      if(lclWrkr != NULL) {
        LOG_D("found local worker... calling for response\n");
        answer = lclWrkr(ModbusMessage(slaveId.toInt(), DIAGNOSTICS_SERIAL, subFunction.toInt(), dataWord));        
      } else {
        LOG_D("no local worker found... answering with ILLEGAL_FUNCTION\n");
        answer.setError(slaveId.toInt(), DIAGNOSTICS_SERIAL, ILLEGAL_FUNCTION);
      }
    } else {
      // Call via RTU
      answer = rtu->syncRequest(0xdeadbeef, slaveId.toInt(), DIAGNOSTICS_SERIAL, subFunction.toInt(), dataWord);   
    }    
    MBUlogLvl = previousLevel;
    LOGDEVICE = previous;
    response->print("</pre>");
    auto error = answer.getError();
    if (error == SUCCESS){
      auto count = answer.size() - 4;
      if(count < 0) {
        count = 0;
      }
      response->print("<span >Answer: 0x");
      for (size_t i = 0; i < count; i++) {
        response->printf("%02x", answer[i + 4]);
      }      
      response->print("</span>");
    }
    else {
      response->printf("<span class=\"e\">Error: %#02x (%s)</span>", error, ErrorName(error).c_str());
    }
    sendDebugForm_diagnosticSerial(response, slaveId, subFunction, data);
    sendButton(response, "Back", "debug");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /update");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Firmware Update");
    response->print("<form method=\"post\" enctype=\"multipart/form-data\">"
      "<input type=\"file\" name=\"file\" id=\"file\" required/>"
      "<p></p>"
      "<button class=\"r\">Upload</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    request->onDisconnect([](){
      ESP.restart();
    });
    dbgln("[webserver] OTA finished");
    if (Update.hasError()){
      auto *response = request->beginResponse(500, "text/plain", "Ota failed");
      response->addHeader("Connection", "close");
      request->send(response);
    }
    else{
      auto *response = request->beginResponseStream("text/html");
      response->addHeader("Connection", "close");
      sendResponseHeader(response, "Firmware Update", true);
      response->print("<p>Update successful.</p>");
      sendButton(response, "Back", "/");
      sendResponseTrailer(response);
      request->send(response);
    }
  }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    dbg("[webserver] OTA progress: "); dbgln(index);
    if (!index) {
      //TODO add MD5 Checksum and Update.setMD5
      int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
        Update.printError(Serial);
        return request->send(400, "text/plain", "OTA could not begin");
      }
    }
    // Write chunked data to the free sketch space
    if(len){
      if (Update.write(data, len) != len) {
        return request->send(400, "text/plain", "OTA could not write data");
      }
    }
    if (final) { // if the final flag is set then this is the last frame of data
      if (!Update.end(true)) { //true to set the size to the current progress
        Update.printError(Serial);
        return request->send(400, "text/plain", "Could not end OTA");
      }
    }else{
      return;
    }
  });

  server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /wifi");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "WiFi reset");
    response->print("<p class=\"e\">"
        "This will delete the stored WiFi config<br/>"
        "and restart the ESP in AP mode.<br/> Are you sure?"
      "</p>");
    sendButton(response, "Back", "/");
    response->print("<p></p>"
      "<form method=\"post\">"
        "<button class=\"r\">Yes, do it!</button>"
      "</form>");    
    sendResponseTrailer(response);
    request->send(response);
  });

  server->on("/wifi", HTTP_POST, [wm](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /wifi");
    request->redirect("/");
    wm->erase();
    dbgln("[webserver] erased wifi config");
    dbgln("[webserver] rebooting...");
    ESP.restart();
    dbgln("[webserver] rebooted...");
  });

  server->on("/favicon.ico", [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /favicon.ico");
    request->send(204);//TODO add favicon
  });

  server->on("/style.css", [](AsyncWebServerRequest *request){
    if (request->hasHeader("If-None-Match")){
      auto header = request->getHeader("If-None-Match");
      if (header->value() == String(ETAG)){
        request->send(304);
        return;
      }
    }
    dbgln("[webserver] GET /style.css");
    auto *response = request->beginResponseStream("text/css");
    sendMinCss(response);
    response->print(
    "button.r{"
	    "background: #d43535;"
    "}"
    "button.r:hover{"
	    "background: #931f1f;"
    "}"
    "table{"
      "text-align:left;"
      "width:100%;"
    "}"
    "input{"
      "width:100%;"
    "}"
    ".e{"
      "color:red;"
    "}"
    "pre{"
      "text-align:left;"
    "}"
    );
    response->addHeader("ETag", ETAG);
    request->send(response);
  });

  server->onNotFound([](AsyncWebServerRequest *request){
    dbg("[webserver] request to "); dbg(request->url()); dbgln("not found");
    request->send(404, "text/plain", "404");
  });
}

void sendMinCss(AsyncResponseStream *response){
  response->print("body{"    
      "font-family:sans-serif;"
	    "text-align: center;"
      "background: #252525;"
	    "color: #faffff;"
    "}"
    "#content{"
	    "display: inline-block;"
	    "min-width: 340px;"
    "}"
    "button{"
	    "width: 100%;"
	    "line-height: 2.4rem;"
	    "background: #1fa3ec;"
	    "border: 0;"
	    "border-radius: 0.3rem;"
	    "font-size: 1.2rem;"
      "-webkit-transition-duration: 0.4s;"
      "transition-duration: 0.4s;"
	    "color: #faffff;"
    "}"
    "button:hover{"
	    "background: #0e70a4;"
    "}");
}

void sendResponseHeader(AsyncResponseStream *response, const char *title, bool inlineStyle){
    response->print("<!DOCTYPE html>"
      "<html lang=\"en\" class=\"\">"
      "<head>"
      "<meta charset='utf-8'>"
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/>");
    response->printf("<title>ESP32 Modbus Gateway - %s</title>", title);
    if (inlineStyle){
      response->print("<style>");
      sendMinCss(response);
      response->print("</style>");
    }
    else{
      response->print("<link rel=\"stylesheet\" href=\"style.css\">");
    }
    response->print(
      "</head>"
      "<body>"
      "<h2>ESP32 Modbus Gateway</h2>");
    response->printf("<h3>%s</h3>", title);
    response->print("<div id=\"content\">");
}

void sendResponseTrailer(AsyncResponseStream *response){
    response->print("</div></body></html>");
}

void sendButton(AsyncResponseStream *response, const char *title, const char *action, const char *css){
    response->printf(
      "<form method=\"get\" action=\"%s\">"
        "<button class=\"%s\">%s</button>"
      "</form>"
      "<p></p>", action, css, title);
}

void sendTableRow(AsyncResponseStream *response, const char *name, String value){
    response->printf(
      "<tr>"
        "<td>%s:</td>"
        "<td>%s</td>"
      "</tr>", name, value.c_str());
}

void sendTableRow(AsyncResponseStream *response, const char *name, uint32_t value){
    response->printf(
      "<tr>"
        "<td>%s:</td>"
        "<td>%d</td>"
      "</tr>", name, value);
}

void sendDebugForm_diagnosticSerial(AsyncResponseStream *response, String slaveId, String subFunction, String data) {
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"slave\">Slave ID</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"247\" id=\"slave\" name=\"slave\" value=\"%s\">", slaveId.c_str());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"func\">Function</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"sf\" name=\"sf\" data-value=\"%s\">", subFunction.c_str());
    response->print("<option value=\"0\">00 Return Query Data</option>"
              "<option value=\"1\">01 Restart Communications</option>"
              "<option value=\"2\">02 Return Diagnostic Register</option>"
              "<option value=\"10\">10 Clear Counters and Diagnostic Register</option>"
              "<option value=\"11\">11 Return Bus Message Count</option>"
              "<option value=\"12\">12 Return Bus Communication Error Count</option>" 
              "<option value=\"14\">14 Return Slave Message Count</option>"             
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"reg\">Data 0x:</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"text\" minlength=\"0\" maxlength=\"4\" id=\"dt\" name=\"dt\" value=\"%s\">", data.c_str());
    response->print("</td>"
        "</tr>"
        "<tr><td>&nbsp;</td><td></td></tr>"
      "</table>");
    response->print("<button class=\"r\">Make it so</button>"
      "</form>"
      "<p></p>");
    response->print("<script>"
      "(function(){"
        "var s = document.querySelectorAll('select[data-value]');"
        "for(d of s){"
          "d.querySelector(`option[value='${d.dataset.value}']`).selected=true"
      "}})();"
      "</script>");
}

void sendDebugForm_read(AsyncResponseStream *response, String slaveId, String reg, String function, String count) {
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"slave\">Slave ID</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"247\" id=\"slave\" name=\"slave\" value=\"%s\">", slaveId.c_str());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"func\">Function</label>"
          "</td>"
          "<td>");
    response->printf("<select id=\"func\" name=\"func\" data-value=\"%s\">", function.c_str());
    response->print("<option value=\"1\">01 Read Coils</option>"
              "<option value=\"2\">02 Read Discrete Inputs</option>"
              "<option value=\"3\">03 Read Holding Register</option>"
              "<option value=\"4\">04 Read Input Register</option>"
            "</select>"
          "</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"reg\">Register</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"65535\" id=\"reg\" name=\"reg\" value=\"%s\">", reg.c_str());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"count\">Count</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"65535\" id=\"count\" name=\"count\" value=\"%s\">", count.c_str());
    response->print("</td>"
        "</tr>"
        "<tr><td>&nbsp;</td><td></td></tr>"
      "</table>");
    response->print("<button class=\"r\">Make it so</button>"
      "</form>"
      "<p></p>");
    response->print("<script>"
      "(function(){"
        "var s = document.querySelectorAll('select[data-value]');"
        "for(d of s){"
          "d.querySelector(`option[value='${d.dataset.value}']`).selected=true"
      "}})();"
      "</script>");
}

const String ErrorName(Modbus::Error code)
{
    switch (code)
    {
        case Modbus::Error::SUCCESS: return "Success";
        case Modbus::Error::ILLEGAL_FUNCTION: return "Illegal function";
        case Modbus::Error::ILLEGAL_DATA_ADDRESS: return "Illegal data address";
        case Modbus::Error::ILLEGAL_DATA_VALUE: return "Illegal data value";
        case Modbus::Error::SERVER_DEVICE_FAILURE: return "Server device failure";
        case Modbus::Error::ACKNOWLEDGE: return "Acknowledge";
        case Modbus::Error::SERVER_DEVICE_BUSY: return "Server device busy";
        case Modbus::Error::NEGATIVE_ACKNOWLEDGE: return "Negative acknowledge";
        case Modbus::Error::MEMORY_PARITY_ERROR: return "Memory parity error";
        case Modbus::Error::GATEWAY_PATH_UNAVAIL: return "Gateway path unavailable";
        case Modbus::Error::GATEWAY_TARGET_NO_RESP: return "Gateway target no response";
        case Modbus::Error::TIMEOUT: return "Timeout";
        case Modbus::Error::INVALID_SERVER: return "Invalid server";
        case Modbus::Error::CRC_ERROR: return "CRC error";
        case Modbus::Error::FC_MISMATCH: return "Function code mismatch";
        case Modbus::Error::SERVER_ID_MISMATCH: return "Server id mismatch";
        case Modbus::Error::PACKET_LENGTH_ERROR: return "Packet length error";
        case Modbus::Error::PARAMETER_COUNT_ERROR: return "Parameter count error";
        case Modbus::Error::PARAMETER_LIMIT_ERROR: return "Parameter limit error";
        case Modbus::Error::REQUEST_QUEUE_FULL: return "Request queue full";
        case Modbus::Error::ILLEGAL_IP_OR_PORT: return "Illegal ip or port";
        case Modbus::Error::IP_CONNECTION_FAILED: return "IP connection failed";
        case Modbus::Error::TCP_HEAD_MISMATCH: return "TCP header mismatch";
        case Modbus::Error::EMPTY_MESSAGE: return "Empty message";
        case Modbus::Error::ASCII_FRAME_ERR: return "ASCII frame error";
        case Modbus::Error::ASCII_CRC_ERR: return "ASCII crc error";
        case Modbus::Error::ASCII_INVALID_CHAR: return "ASCII invalid character";
        default: return "undefined error";
    }
}

// translate RSSI to quality string
const String WiFiQuality(int rssiValue)
{
    switch (rssiValue)
    {
        case -30 ... 0: return "Amazing"; 
        case -67 ... -31: return "Very Good"; 
        case -70 ... -68: return "Okay"; 
        case -80 ... -71: return "Not Good"; 
        default: return "Unusable";
    }
}
