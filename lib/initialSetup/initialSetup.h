#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <memory>
#include <memory>
namespace initialSetup
{
    struct MQTTBrokerInfo
    {
            static const uint32_t addrLength = 64;
            char _addr[addrLength]; // Needs init
            uint32_t _port;
    };

    struct NetworkInfo
    {
            static const uint32_t ssidLength = 40;
            char _ssid[ssidLength];
    };

    class InitialSetup
    {
        public:
            InitialSetup();
            bool run();
            void reset();
            MQTTBrokerInfo* getMQTTBrokerInfo();
            NetworkInfo* getNetworkInfo();

        private:
            MQTTBrokerInfo _mqttBrokerInfo;
            NetworkInfo    _networkInfo;
            static const uint8_t _portLength = 6;
            char _portAsString[_portLength];

            static const uint8_t _hostnameLength = 40;
            char _hostname[_hostnameLength];

            static constexpr const char*const _stationSSID = "MQTT Device";
    };
}