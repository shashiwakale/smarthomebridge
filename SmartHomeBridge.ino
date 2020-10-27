#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson


#define CONFIG_LED 2
#define CONFIG_BUTTON D1  //14 on V2, 15 on V3
#define PRODUCT_NAME "Smart Home Bridge v1.0"

//globals for credentials
char g_cTokenKey[100];
char g_cDeviceName[40];
char g_cMessage[100];
boolean g_bResetFlag = false;

//flag for saving data
bool g_bSaveConfig = false;
int g_nResetButtonSec = 0;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(CONFIG_LED, OUTPUT);
  pinMode(CONFIG_BUTTON, INPUT);
  digitalWrite(CONFIG_LED, HIGH);
  ReadFile();
  WiFiConnection();
}

void loop() 
{
   ConfigButtonCheck();
   delay(1000);
}
