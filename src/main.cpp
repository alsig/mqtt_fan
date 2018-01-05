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


namespace app
{
    struct FanAppConfig
    {
        uint8_t _scanRate;
        QueueArray<ledTask::LEDInfo>*           _ledTaskQueue;
        QueueArray<buttonTask::ButtonState>*    _buttonTaskQueue;
        initialSetup::MQTTBrokerInfo*           _mqttBrokerInfo;
    };

    class FanApp : scheduler::Runnable
    {
        public:
            FanApp(FanAppConfig* config)
            : Runnable(config->_scanRate)
            , _config(config)
            {
                _mqttClient = new PubSubClient(_espClient);
                Serial.printf("mqtt Info: %s, %d\n", _config->_mqttBrokerInfo->_addr, _config->_mqttBrokerInfo->_port);
                _mqttClient->setServer(_config->_mqttBrokerInfo->_addr,_config->_mqttBrokerInfo->_port);
                ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eFlash, 2);
                _config->_ledTaskQueue->push(ledInfo);
            }
        private:
            void run(void) override
            {
                if(!_config->_buttonTaskQueue->isEmpty())
                {
                    buttonTask::ButtonState buttonState = _config->_buttonTaskQueue->pop();
                    if(buttonState == buttonTask::ButtonState::ePushed)
                    {
                        Serial.println("app::pushed");                    
                        ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eOn);
                        _config->_ledTaskQueue->push(ledInfo);
                    }   
                    else if(buttonState == buttonTask::ButtonState::eReleased)
                    {
                        Serial.println("app::released");
                        ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eOff);
                        _config->_ledTaskQueue->push(ledInfo);
                    } 
                }
            }
            FanAppConfig*   _config;
            PubSubClient*   _mqttClient;
            WiFiClient      _espClient;
    };
}




void setup() 
{   
    Serial.begin(115200);
    Serial.println();

    initialSetup::InitialSetup initialSetup;
    initialSetup.run();

    QueueArray<ledTask::LEDInfo>* ledQueue = new QueueArray<ledTask::LEDInfo>(5);
    QueueArray<buttonTask::ButtonState>* buttonQueue = new QueueArray<buttonTask::ButtonState>(5);


    ledTask::LEDTask* ledTask = new ledTask::LEDTask(ledQueue, LED);
    buttonTask::ButtonTask* buttonTask = new buttonTask::ButtonTask(buttonQueue, BUTTON);
    
    // App config
    app::FanAppConfig* appConfig = new app::FanAppConfig;
    appConfig->_scanRate = 20;
    appConfig->_buttonTaskQueue = buttonQueue;
    appConfig->_ledTaskQueue = ledQueue;
    appConfig->_mqttBrokerInfo = initialSetup.getMQTTBrokerInfo();
    app::FanApp* fanApp = new app::FanApp(appConfig);
    
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