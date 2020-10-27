void ConfigButtonCheck()
{
  if(!digitalRead(CONFIG_BUTTON))
  {
    for(int i = 0; i < 3; i++)
    {
      if(!digitalRead(CONFIG_BUTTON))
      {
        g_nResetButtonSec++;
      }
      delay(1000);
    }
    if(g_nResetButtonSec == 3)
    {
        Serial.println("External Reset Request Received");
        digitalWrite(CONFIG_LED, HIGH);
        g_bResetFlag = true;
        ReadFile();
        delay(100);
        WiFiConnection();
        digitalWrite(CONFIG_LED, HIGH);
        g_nResetButtonSec = 0;
    }
    else
    {
      g_nResetButtonSec = 0;
    }
  }
}

void SaveConfigCallbackFunction () 
{
  Serial.println("Should save config");
  g_bSaveConfig = true;
}

boolean WiFiConnection() 
{
  WiFiManager l_oWiFiManager;
  
  digitalWrite(CONFIG_LED, LOW);
  
  if (g_bResetFlag)
  {  
    l_oWiFiManager.resetSettings();// held the button long enough, so wipe out the wifi settings and launch the portal
    g_bResetFlag = false;
  }
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  // Form Input Token
  WiFiManagerParameter l_wmpTokenKey("Token", "Your Token", g_cTokenKey, 100);
  // Form Input Device Name
  WiFiManagerParameter l_wmpDeviceBoardName("DeviceName", "Device Name", g_cDeviceName, 40);
  // Form Input Message
  WiFiManagerParameter l_wmpMessage("Message", "Enter Message to be Sent", g_cMessage, 100);

  //set config save notify callback
  l_oWiFiManager.setSaveConfigCallback(SaveConfigCallbackFunction);

  //add all your parameters here
  l_oWiFiManager.addParameter(&l_wmpTokenKey);  
  l_oWiFiManager.addParameter(&l_wmpDeviceBoardName);
  l_oWiFiManager.addParameter(&l_wmpMessage);

  //reset settings - for testing
  //l_oWiFiManager.resetSettings();
  l_oWiFiManager.setTimeout(120);//2min, if can't get connected just go back to sleep
  
  if (!l_oWiFiManager.autoConnect(PRODUCT_NAME)) 
  {
    // SSID you connect to from browser
    Serial.println("failed to connect and hit timeout");
    digitalWrite(CONFIG_LED, HIGH);
    return 0;
  }

  Serial.println("Connected");

  //read updated parameters
  strcpy(g_cTokenKey, l_wmpTokenKey.getValue());
  strcpy(g_cDeviceName, l_wmpDeviceBoardName.getValue());
  strcpy(g_cMessage, l_wmpMessage.getValue());

  //save the custom parameters to FS
  if (g_bSaveConfig) 
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["TokenKey"] = g_cTokenKey;
    json["DeviceName"] = g_cDeviceName;
    json["Message"] = g_cMessage;

    File l_fConfigFile = SPIFFS.open("/config.json", "w");
    if (!l_fConfigFile) 
    {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(l_fConfigFile);
    l_fConfigFile.close();
    //end save
  }

  Serial.println("");// WiFi info
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(CONFIG_LED, HIGH);
}

void ReadFile() 
{
  //clean FS, for testing
  //SPIFFS.format();
  //read configuration from FS json
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) 
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) 
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File l_fConfigFile = SPIFFS.open("/config.json", "r");
      if (l_fConfigFile) 
      {
        Serial.println("opened config file");
        size_t size = l_fConfigFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        l_fConfigFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        //Print JSON Packet
        json.printTo(Serial);
        //If valid JSON 
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          if (json.containsKey("Message") && json.containsKey("DeviceName") && json.containsKey("TokenKey")) 
          {
            strcpy(g_cMessage, json["Message"]);
            strcpy(g_cDeviceName, json["DeviceName"]);
            strcpy(g_cTokenKey, json["TokenKey"]);
            Serial.println("\nkeys found - all good");
          }
          else 
          {
            SPIFFS.format();
          }

        } 
        else 
        {
          Serial.println("failed to load json config");
        }
      }
    }
  } 
  else 
  {
    Serial.println("failed to mount FS");
  }
  //end read
}
