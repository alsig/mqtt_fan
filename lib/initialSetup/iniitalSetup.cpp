#include "initialSetup.h"
#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>




namespace initialSetup
{
    bool shouldSaveConfig = false;

    void saveConfigCallback () 
    {
        Serial.println("Should save config");
        shouldSaveConfig = true;
    }
    InitialSetup::InitialSetup()
    {
        strncpy(_portAsString, "8123", _portLength);
        strncpy(_hostname, _stationSSID, _hostnameLength);
    }

    bool InitialSetup::run()
    {
        if (SPIFFS.begin()) 
        {
            Serial.println("mounted file system");
            if (SPIFFS.exists("/config.json")) 
            {
                //file exists, reading and loading
                Serial.println("reading config file");
                File configFile = SPIFFS.open("/config.json", "r");
                if (configFile) 
                {
                    Serial.println("opened config file");
                    size_t size = configFile.size();
                    // Allocate a buffer to store contents of the file.
                    std::unique_ptr<char[]> buf(new char[size]);

                    configFile.readBytes(buf.get(), size);
                    DynamicJsonBuffer jsonBuffer;
                    JsonObject& json = jsonBuffer.parseObject(buf.get());
                    json.printTo(Serial);
                    if (json.success()) 
                    {
                        Serial.println("\nparsed json");
                        
                        strncpy(_mqttBrokerInfo._addr, json["mqtt_server"], MQTTBrokerInfo::addrLength);
                        strncpy(_hostname, json["hostname"], _hostnameLength);
                        _mqttBrokerInfo._port = json["mqtt_port"];
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

        WiFiManagerParameter custom_mqtt_server("server", "mqtt server", _mqttBrokerInfo._addr, MQTTBrokerInfo::addrLength);
        WiFiManagerParameter custom_mqtt_port("port", "mqtt port", _portAsString, _portLength);
        WiFiManagerParameter custom_hostname("hostname", "hostname", _hostname, _hostnameLength);

        WiFiManager wifiManager;
        wifiManager.setSaveConfigCallback(saveConfigCallback);

  
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_hostname);
        

        wifiManager.setTimeout(120);
    
        if (!wifiManager.autoConnect(_stationSSID)) 
        {
            Serial.println("failed to connect and hit timeout");
            delay(3000);
            //reset and try again, or maybe put it to deep sleep
            ESP.reset();
            delay(5000);
        }

        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");

        //read updated parameters
        strncpy(_mqttBrokerInfo._addr, custom_mqtt_server.getValue(), MQTTBrokerInfo::addrLength);
        _mqttBrokerInfo._port = atoi(custom_mqtt_port.getValue());
    

        //save the custom parameters to FS
        if (shouldSaveConfig) 
        {
            Serial.println("saving config");
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.createObject();
            json["mqtt_server"] =  _mqttBrokerInfo._addr;
            itoa(_mqttBrokerInfo._port, _portAsString, 10);
            json["mqtt_port"] = _portAsString;
            json["hostname"] = _hostname;
            

            File configFile = SPIFFS.open("/config.json", "w");
            if (!configFile) 
            {
                Serial.println("failed to open config file for writing");
            }

            json.printTo(Serial);
            json.printTo(configFile);
            configFile.close();
            //end save
        }

        Serial.println("local ip");
        Serial.println(WiFi.localIP());
    }

    void InitialSetup::reset()
    {
        WiFiManager wifiManager;
        wifiManager.resetSettings();
        // Remove config.json
    }

    MQTTBrokerInfo* InitialSetup::getMQTTBrokerInfo()
    {
        return &_mqttBrokerInfo;
    }

    NetworkInfo* InitialSetup::getNetworkInfo()
    {
        return &_networkInfo;
    }
}