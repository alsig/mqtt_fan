#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "initialSetup.h"
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include "runnable.h"
#include <QueueArray.h>
#include "ledtask.h"
#include "buttonTask.h"
#include <functional>
#include "config.h"
#include <fanTask.h>

void setup() 
{   
    Serial.begin(115200);
    Serial.println();

    WiFi.begin(config::ssid, config::passkey);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    QueueArray<ledTask::LEDInfo>* ledQueue = new QueueArray<ledTask::LEDInfo>(5);
    QueueArray<buttonTask::ButtonState>* buttonQueue = new QueueArray<buttonTask::ButtonState>(5);


    ledTask::LEDTask* ledTask = new ledTask::LEDTask(ledQueue, config::ledPin);
    buttonTask::ButtonTask* buttonTask = new buttonTask::ButtonTask(buttonQueue, config::buttonPin);
    
    // App config
    fanTask::FanTaskConfig* fanTaskConfig = new fanTask::FanTaskConfig;
    fanTaskConfig->_scanRate = 20;
    fanTaskConfig->_ledTaskQueue = ledQueue;
    fanTaskConfig->_mqttBrokerAddr = &config::mqttBrokerAddr;
    fanTaskConfig->_mqttBrokerPort = &config::mqttBrokerPort;
    fanTaskConfig->_deviceName = config::deviceName;
    fanTaskConfig->_modeTopic = config::modeTopic;
    fanTaskConfig->_relayPin = &config::relayPin;
    fanTaskConfig->_dhtPin = &config::dhtPin;
    fanTaskConfig->_temperatureThreshold = config::temperatureThreshold;
    fanTaskConfig->_humidityThreshold = config::humidityThreshold;
    
    fanTask::FanTask* fanApp = new fanTask::FanTask(fanTaskConfig);
    
    ArduinoOTA.onStart([]() 
    {
        Serial.println("OTA Start");
    });

    ArduinoOTA.onEnd([]() 
    {
        Serial.println("OTA End");
        Serial.println("Rebooting...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
    {
        Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) 
    {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
}

void loop() 
{
    scheduler::SchedulerWrapper::run();
    ArduinoOTA.handle();
}

