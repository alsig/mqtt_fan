#include <fanTask.h>

namespace fanTask
{
    FanTask::FanTask(FanTaskConfig* config)
    : Runnable(config->_scanRate)
    , _config(config)
    , _state(States::eAuto)
    , _humidityDeadBand(5.0f)
    , _temperatureDeadBand(2.0f)
    {
        pinMode(*_config->_relayPin, OUTPUT);

        _dht = new DHT(*_config->_dhtPin, DHT11);
        _dht->begin();
        _timeOut = _timeOutReload =  5000 / _config->_scanRate; // every 5 second
    
        using namespace std::placeholders; 
        _mqttClient = new PubSubClient(_espClient);
        Serial.printf("mqtt Info: %s, %d\n", config->_mqttBrokerAddr->toString().c_str(), *config->_mqttBrokerPort);
        _mqttClient->setServer(*(config->_mqttBrokerAddr), *config->_mqttBrokerPort);
        auto cb = std::bind(&FanTask::callback, this, _1, _2, _3);
        _mqttClient->setCallback(cb);
        
        if(!_mqttClient->connect(config->_deviceName))
            Serial.printf("failed to connect: %d\n", _mqttClient->state());
        _mqttClient->subscribe(config->_modeTopic);
        ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eFlash, 10);
        _config->_ledTaskQueue->push(ledInfo);
    }
    
    void FanTask::callback(char* topic, uint8_t* payload, unsigned int length)
    {
        if(!strcmp(topic, _config->_modeTopic))
        {
            if(!strncmp((char*)payload, "on", length))
            {
                // update LED
                ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eOn);
                _config->_ledTaskQueue->push(ledInfo);
                _state = States::eOn;
            }
            else if(!strncmp((char*)payload, "off", length))
            {
                // update LED
                ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eOff);
                _config->_ledTaskQueue->push(ledInfo);
                _state = States::eOff;
            }
            else if(!strncmp((char*)payload, "auto", length))
            {
                // update LED
                ledTask::LEDInfo ledInfo(ledTask::LEDInfo::LEDState::eFlash, 10);
                _config->_ledTaskQueue->push(ledInfo);
                _state = States::eAuto;
            }
        }
    }
    void FanTask::run(void)
    {
        _mqttClient->loop();
        switch(_state)
        {
            case States::eOn:
                setRelay(RelayState::eOn);
                break;
            case States::eOff:
                setRelay(RelayState::eOff);
                break;
            case States::eAuto:
                autoFanCtrl();
                break;
        }
    }

    void FanTask::setRelay(const RelayState state)
    {
        digitalWrite(*_config->_relayPin, state == RelayState::eOn);
    }

    void FanTask::autoFanCtrl(void)
    {
        --_timeOut;
        if(_timeOut == 0)
        {
            _timeOut = _timeOutReload;
            float humidity = _dht->readHumidity();
            float temperature = _dht->readTemperature(false); // in celcius 
            #if 1 // insert this code when dht sensor is ready
            if(isnan(humidity))
            {
                Serial.println("Error reading humidity from DHT sensor");
                return;
            }
            if(isnan(temperature))
            {
                Serial.println("Error reading temperature from DHT sensor");
                return;
            }
            #else
                humidity = 50.0f;
                temperature = 23.0f;
            #endif
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.print(" %\t Temperature: ");
            Serial.print(temperature);
            Serial.print(" *C ");
            Serial.println("");
            
            if(temperature - _temperatureDeadBand > _config->_temperatureThreshold)
            {
                _relayState = RelayState::eOn;
            }
            else if(temperature < _config->_temperatureThreshold)
            {
                _relayState = RelayState::eOff;
            }

            if(humidity - _humidityDeadBand > _config->_humidityThreshold)
            {
                _relayState = RelayState::eOn;
            }
            else if(humidity < _config->_humidityThreshold)
            {
                _relayState = RelayState::eOff;
            }
            setRelay(_relayState);
        }
    }
}