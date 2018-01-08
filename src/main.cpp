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
#include <functional>
#include "config.h"





namespace app
{
    struct FanAppConfig
    {
        uint8_t _scanRate;
        QueueArray<ledTask::LEDInfo>*           _ledTaskQueue;
        QueueArray<buttonTask::ButtonState>*    _buttonTaskQueue;
    };

    class FanApp : scheduler::Runnable
    {
        public:
            FanApp(FanAppConfig* config)
            : Runnable(config->_scanRate)
            , _config(config)
            {
                using namespace std::placeholders; 
                
                _mqttClient = new PubSubClient(_espClient);
                Serial.printf("mqtt Info: %s, %d\n", config::mqttBrokerAddr.toString().c_str(), config::mqttBrokerPort);
                _mqttClient->setServer(config::mqttBrokerAddr, config::mqttBrokerPort);
                auto cb = std::bind(&FanApp::callback, this, _1, _2, _3);
                _mqttClient->setCallback(cb);
                
                if(!_mqttClient->connect(config::deviceName))
                    Serial.printf("failed to connect: %d\n", _mqttClient->state());
                _mqttClient->subscribe(config::ledTopic);
                ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eFlash, 2);
                _config->_ledTaskQueue->push(ledInfo);
            }
        private:
            void callback(char* topic, uint8_t* payload, unsigned int length)
            {
                if(!strcmp(topic, config::ledTopic))
                {
                    if(!strncmp((char*)payload, "on", length))
                    {
                        ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eOn);
                        _config->_ledTaskQueue->push(ledInfo);  
                    }
                    else if(!strncmp((char*)payload, "off", length))
                    {
                        ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eOff);
                        _config->_ledTaskQueue->push(ledInfo);
                    }
                    else if(!strncmp((char*)payload, "auto", length))
                    {
                        ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eFlash, 10);
                        _config->_ledTaskQueue->push(ledInfo);
                    }
                }
            }
            void run(void) override
            {
                _mqttClient->loop();
                if(!_config->_buttonTaskQueue->isEmpty())
                {
                    buttonTask::ButtonState buttonState = _config->_buttonTaskQueue->pop();
                    if(buttonState == buttonTask::ButtonState::ePushed)
                    {
                        _mqttClient->publish(config::buttonTopic, "pushed");
                    }   
                    else if(buttonState == buttonTask::ButtonState::eReleased)
                    {
                        _mqttClient->publish(config::buttonTopic, "released");
                    } 
                    else if(buttonState == buttonTask::ButtonState::eLongPush)
                    {
                        Serial.println("app::longPush");
                        _mqttClient->publish(config::buttonTopic, "longPush");                        
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

    //initialSetup::InitialSetup initialSetup;
    //initialSetup.run();
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
    app::FanAppConfig* appConfig = new app::FanAppConfig;
    appConfig->_scanRate = 20;
    appConfig->_buttonTaskQueue = buttonQueue;
    appConfig->_ledTaskQueue = ledQueue;
    
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