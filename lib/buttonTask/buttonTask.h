#pragma once
#include "runnable.h"
#include "queueArray.h"

namespace buttonTask
{
    enum class ButtonState
    {
        ePushed,
        eReleased,
        eLongPush
    };

    class ButtonTask : public scheduler::Runnable
    {
        public:
            ButtonTask(QueueArray<ButtonState>& queue,  const uint8_t pin, const uint8_t scanRate = 1);
        private:
            void run() override;

            QueueArray<ButtonState>& _queue;
            const uint8_t           _pin;
            uint8_t                 _lastPinState;
            ButtonState             _buttonState;
            uint16_t                 _counter;
            static const uint16_t   _longPushCount = 1000;


    };
} 