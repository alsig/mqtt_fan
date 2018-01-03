#pragma once
#include "runnable.h"
#include "queueArray.h"

namespace ledTask
{
    class LEDInfo
    {
        public:
        
            enum class LEDState
            {
                eFlash,
                eOn,
                eOff
            };
            LEDInfo(LEDState state = LEDState::eOff, uint8_t freq = 0)
            : _state(state)
            , _freq(freq)
            {    
            }
            LEDState    _state;
            uint8_t     _freq;
    };

    class LEDTask : public scheduler::Runnable
    {
        public:
            LEDTask(QueueArray<LEDInfo>& queue, const uint8_t pin) ;
        private:
            void run() override;
            QueueArray<LEDInfo> _queue;
            const uint8_t       _pin;
            uint8_t             _flashState;
            LEDInfo             _info;
    };
}