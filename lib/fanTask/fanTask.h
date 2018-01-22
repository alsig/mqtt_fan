#pragma once
#include <QueueArray.h>
#include <ledTask.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

namespace fanTask
{
    struct FanTaskConfig
    {
        uint8_t                         _scanRate;
        QueueArray<ledTask::LEDInfo>*   _ledTaskQueue;
        const IPAddress*                _mqttBrokerAddr;
        const uint16_t*                 _mqttBrokerPort;
        const char*                     _deviceName;
        const char*                     _modeTopic;
        const uint8_t*                  _relayPin;
        const uint8_t*                  _dhtPin;
        float                           _humidityThreshold;
        float                           _temperatureThreshold;
    };

    class FanTask : scheduler::Runnable
    {
        public:
            FanTask(FanTaskConfig* config);
        private:
            enum class States
            {
                eOn,
                eOff,
                eAuto
            };

            enum class RelayState
            {
                eOn,
                eOff,
            };
            
            void callback(char* topic, uint8_t* payload, unsigned int length);
            void run(void) override;
            void setRelay(const RelayState state);
            void autoFanCtrl(void);
            States          _state;
            FanTaskConfig*  _config;
            PubSubClient*   _mqttClient;
            WiFiClient      _espClient;
            DHT*            _dht;
            uint32_t        _timeOut;
            uint32_t        _timeOutReload;
            RelayState      _relayState;
            const float     _humidityDeadBand;
            const float     _temperatureDeadBand;

    };
}