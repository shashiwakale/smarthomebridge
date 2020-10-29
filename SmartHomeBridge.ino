#include "Arduino.h"
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "fauxmoESP.h"

// -------Global Defines----------------------------------------------------
#define CONFIG_LED 2
#define CONFIG_BUTTON D1  //14 on V2, 15 on V3
#define PRODUCT_NAME "Smart Home Bridge v1.0"

#define SERIAL_BAUDRATE     115200

#define RELAY1          D5
#define RELAY2          D6
#define RELAY3          D7
#define RELAY4          D8

#define ID_SW1            "switch 1"
#define ID_SW2            "switch 2"
#define ID_SW3            "switch 3"
#define ID_SW4            "switch 4"

// -------Global Variables----------------------------------------------------
char g_cTokenKey[100];
char g_cDeviceName[40];
char g_cMessage[100];
boolean g_bResetFlag = false;
bool g_bSaveConfig = false;
int g_nResetButtonSec = 0;
boolean l_bWiFiConnection; 

// Fauxmo ESP Object
fauxmoESP g_oFauxmo;


/*
 * Function ID    : 01
 * Function Name  : setup
 * Description    : This function will responsible for initilizing digital IO
 *                  and WiFi Connection.
 * Arguments      : -
 * Return Value   : -
*/
void setup() 
{
    // put your setup code here, to run once:
    Serial.begin(SERIAL_BAUDRATE);
    pinMode(CONFIG_LED, OUTPUT);
    pinMode(CONFIG_BUTTON, INPUT);
    digitalWrite(CONFIG_LED, HIGH);
  
    // LEDs
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY4, LOW);
  
    ReadFile();
    l_bWiFiConnection = WiFiConnection();

    if(l_bWiFiConnection)
    {
        // By default, fauxmoESP creates it's own webserver on the defined port
        // The TCP port must be 80 for gen3 devices (default is 1901)
        // This has to be done before the call to enable()
        g_oFauxmo.createServer(true); // not needed, this is the default value
        g_oFauxmo.setPort(80); // This is required for gen3 devices

        // You have to call enable(true) once you have a WiFi connection
        // You can enable or disable the library at any moment
        // Disabling it will prevent the devices from being discovered and switched
        g_oFauxmo.enable(true);

        // You can use different ways to invoke alexa to modify the devices state:
        // "Alexa, turn yellow lamp on"
        // "Alexa, turn on yellow lamp
        // "Alexa, set yellow lamp to fifty" (50 means 50% of brightness, note, this example does not use this functionality)
        
        // Add virtual devices
        g_oFauxmo.addDevice(ID_SW1);
        g_oFauxmo.addDevice(ID_SW2);
        g_oFauxmo.addDevice(ID_SW3);
        g_oFauxmo.addDevice(ID_SW4);

        g_oFauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) 
        {
            // Callback when a command from Alexa is received. 
            // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
            // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
            // Just remember not to delay too much here, this is a callback, exit as soon as possible.
            // If you have to do something more involved here set a flag and process it in your main loop.
            
            Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    
            // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
            // Otherwise comparing the device_name is safer.
    
            if (strcmp(device_name, ID_SW1)==0) {
                digitalWrite(RELAY1, state ? HIGH : LOW);
            } else if (strcmp(device_name, ID_SW2)==0) {
                digitalWrite(RELAY2, state ? HIGH : LOW);
            } else if (strcmp(device_name, ID_SW3)==0) {
                digitalWrite(RELAY3, state ? HIGH : LOW);
            } else if (strcmp(device_name, ID_SW4)==0) {
                digitalWrite(RELAY4, state ? HIGH : LOW);
            }
         });
    }
}

/*
 * Function ID    : 02
 * Function Name  : loop
 * Description    : main function. Itterative code
 * Arguments      : -
 * Return Value   : -
*/
void loop() 
{
    boolean l_bRetValue;
    // Check WiFi connection
    if(l_bWiFiConnection)
    {
        // Check for Config button
        l_bRetValue = ConfigButtonCheck(); 

        if(l_bRetValue)
        {
            // fauxmoESP uses an async TCP server but a sync UDP server
            // Therefore, we have to manually poll for UDP packets
            g_oFauxmo.handle();

            // This is a sample code to output free heap every 5 seconds
            // This is a cheap way to detect memory leaks
            static unsigned long last = millis();
            if (millis() - last > 5000) 
            {
                last = millis();
                Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
            }
            
            // If your device state is changed by any other means (MQTT, physical button,...)
            // you can instruct the library to report the new state to Alexa on next request:
            // g_oFauxmo.setState(ID_YELLOW, true, 255);
        }
    }
}
