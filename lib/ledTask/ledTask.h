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
                eOff,
                eOn,
                eFlash
            };
            LEDInfo(const LEDState state = LEDState::eOff, const uint8_t period = 0)
            : _state(state)
            , _period(period)
            {    
            }
            LEDState    _state;
            uint8_t     _period;
    };

    class LEDTask : public scheduler::Runnable
    {
        public:
            LEDTask(QueueArray<LEDInfo>* queue, const uint8_t pin, const bool invert=true);
        private:
            void run() override;
            QueueArray<LEDInfo>*    _queue;
            const uint8_t           _pin;
            uint8_t                 _flashState;
            LEDInfo                 _info;
            uint8_t                 _count;
            const bool              _invert;
    };
}