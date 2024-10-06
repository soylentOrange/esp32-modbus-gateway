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

AsyncWebServer webServer(80);
Config config;
Preferences prefs;
ModbusClientRTU *MBclient;
ModbusBridgeWiFi MBbridge;
WiFiManager wm;
static volatile uint32_t ip_addr(INADDR_NONE);

// Callback for setting up mdns
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint32_t new_ip_addr = IPAddress(info.got_ip.ip_info.ip.addr);
  dbg("new IP-Address: "); dbgln(IPAddress(new_ip_addr));
  if(new_ip_addr != ip_addr) {
    ip_addr = new_ip_addr;
    dbgln("re-start mDNS");
    MDNS.begin(WiFi.getHostname());
    MDNS.addService("http", "tcp", 80);
  }
}

// Callback for stopping mdns
void WiFiLostIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  dbgln("Lost IP-Address");
  ip_addr = INADDR_NONE;
  MDNS.end();
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
  dbgln("[wifi] start");
  // Set Hostname
  if(config.getHostname().length() > 2) {
    WiFi.setHostname(config.getHostname().c_str());
  }
  // Enable auto-reconnect
  wm.setWiFiAutoReconnect(true);
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  // Set (reduced) WiFi TX Power
  dbgln(String("[WiFi] TxPower: ") + ((float)config.getWiFiTXPower()) / 4 + "dBm")
  WiFi.setTxPower((wifi_power_t) config.getWiFiTXPower()); 
  wm.setClass("invert");
  auto reboot = false;
  wm.setAPCallback([&reboot](WiFiManager *wifiManager){reboot = true;});
  wm.autoConnect();
  if (reboot){
    ESP.restart();
  }
  dbgln("[wifi] finished");
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
  MBclient->setTimeout(1000);
  MBclient->begin(modbusSerial, 1);
  for (uint8_t i = 1; i < 248; i++)
  {
    MBbridge.attachServer(i, i, ANY_FUNCTION_CODE, MBclient);
  }  
  MBbridge.start(config.getTcpPort(), 10, config.getTcpTimeout());
  dbgln("[modbus] finished");
  dbgln("[server] start");
  setupPages(&webServer, MBclient, &MBbridge, &config, &wm);
  webServer.begin();
  ip_addr = WiFi.localIP();
  MDNS.begin(WiFi.getHostname());
  MDNS.addService("http", "tcp", 80);
  dbgln("[server] finished");
  // Register Callbacks for re-starting mDNs after reconnect
  WiFi.onEvent(WiFiLostIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_LOST_IP);  
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);  
  dbgln("[setup] finished");
}

void loop() {
  // put your main code here, to run repeatedly:
}