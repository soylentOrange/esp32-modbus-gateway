#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Logging.h>
#include <ModbusBridgeWiFi.h>
#include <ModbusClientRTU.h>
#include "config.h"
#include "pages.h"
#include "restapi.h"
#include "localmodbus.h"

AsyncWebServer webServer(80);
Config config;
Preferences prefs;
ModbusClientRTU *MBclient;
ModbusBridgeWiFi MBbridge;
WiFiManager wm;
boolean isConnected(false);

// Start mDNS for webserver and modbus
void startMDNS(uint16_t webPort = 80, uint16_t modbusPort = 502) {
  // start mDNS
  MDNS.begin(WiFi.getHostname());
  MDNS.addService("http", "tcp", webPort);
  MDNS.addService("modbus", "tcp", modbusPort);
};

// Connect to Wifi
boolean WiFiConnect() {
  // Set Hostname
  if(config.getHostname().length() > 2) {
    WiFi.setHostname(config.getHostname().c_str());
  }
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  // Set (reduced) WiFi TX Power
  dbgln(String("[WiFi] TxPower: ") + ((float)config.getWiFiTXPower()) / 4 + "dBm")
  WiFi.setTxPower((wifi_power_t) config.getWiFiTXPower()); 
  wm.setClass("invert");
  auto reboot = false;
  wm.setAPCallback([&reboot](WiFiManager *wifiManager){reboot = true;});
  isConnected = wm.autoConnect();
  if (reboot){
    ESP.restart();
  }
  if(WiFi.getMode() == WIFI_MODE_STA) {
    dbgln("[WiFi] connected in WIFI_MODE_STA");
    wm.setWiFiAutoReconnect(false);
    return true;
  } else {
    dbgln("[WiFi] NOT connected in WIFI_MODE_STA");
    return false;
  }
};

// Callback for reconnecting
void WiFiLostIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  dbgln("[WiFi] (possibly) disconnected");
  
  if(WiFi.status() != WL_CONNECTED) {
    dbgln("[WiFi] trying to reconnect");
    MDNS.end();
    
    isConnected = false;
    isConnected = WiFiConnect();

    // Start mDNS
    if(isConnected) {
      startMDNS(80, config.getTcpPort());
    }
  } else {
    dbgln("[WiFi] actually is connected");
  }
}

void setup() {
  // Set up debug serial port
  debugSerial.begin(115200);
  dbgln();
  dbgln("[config] load");
  prefs.begin("modbusRtuGw");
  config.begin(&prefs);
  debugSerial.end();
  debugSerial.begin(config.getSerialBaudRate(), config.getSerialConfig());
  dbgln();
  dbgln("[wifi] start");
  isConnected = WiFiConnect();
  dbgln("[modbus] start");

  MBUlogLvl = LOG_LEVEL_WARNING;
  RTUutils::prepareHardwareSerial(modbusSerial);
#if defined(RX_PIN) && defined(TX_PIN)
  // use rx and tx-pins if defined in platformio.ini
  modbusSerial.begin(config.getModbusBaudRate(), config.getModbusConfig(), RX_PIN, TX_PIN );
  dbgln("Use user defined RX/TX pins");
#else
  // otherwise use default pins for hardware-serial2
  modbusSerial.begin(config.getModbusBaudRate(), config.getModbusConfig());
#endif

  MBclient = new ModbusClientRTU(config.getModbusRtsPin());
  MBclient->setTimeout(config.getModbusTimeout());
  MBclient->begin(modbusSerial, 1);
  uint8_t skipAddress = (config.getLocalModbusEnable() && config.getLocalModbusAddress() > 0 && config.getLocalModbusAddress() < 248) ? config.getLocalModbusAddress() : 0;
  for (uint8_t i = 1; i < 248; i++) {
    if(i != skipAddress) {
      MBbridge.attachServer(i, i, ANY_FUNCTION_CODE, MBclient);
    }
  }  

  // register worker for local Modbus function
  if(skipAddress) {
    setupLocalModbus(config.getLocalModbusAddress(), &MBbridge, &config, &wm);
  }

  // Start Modbus Bridge
  MBbridge.start(config.getTcpPort(), 10, config.getTcpTimeout());
  dbgln("[server] start");

  // Setup the pages for Webserver and Rest api (v1)
  setupPages(&webServer, MBclient, &MBbridge, &config, &wm);
  setupRestApi(&webServer, MBclient, &MBbridge, &config, &wm);
  webServer.begin();

  // Start mDNS
  startMDNS(80, config.getTcpPort());

  // Register Callbacks for re-starting mDNs after disconnect
  WiFi.onEvent(WiFiLostIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED); 
  dbgln("[setup] finished");
}

void loop() {
  // put your main code here, to run repeatedly:
}