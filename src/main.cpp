#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "initialSetup.h"
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include "runnable.h"
#include <QueueArray.h>
#include "ledtask.h"
#include "buttonTask.h"
#include "common.h"
static const uint8_t LED = 2;
static const uint8_t BUTTON = 4;

QueueArray<ledTask::LEDInfo> ledQueue(5);
QueueArray<buttonTask::ButtonState> buttonQueue(5);

void setup() 
{   
    Serial.begin(115200);
    Serial.println();

    initialSetup::InitialSetup initialSetup;
    initialSetup.run();

    
    ledTask::LEDTask* ledTask = new ledTask::LEDTask(ledQueue, LED);
    buttonTask::ButtonTask* buttonTask = new buttonTask::ButtonTask(buttonQueue, BUTTON);
    ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eFlash, 2);
    ledQueue.push(ledInfo);
    
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