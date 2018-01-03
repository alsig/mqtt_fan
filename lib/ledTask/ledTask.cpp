#include "ledTask.h"

namespace ledTask
{
    static const uint8_t scanRate = 50;
    LEDTask::LEDTask(QueueArray<LEDInfo>& queue, const uint8_t pin)
    : Runnable(scanRate)
    , _queue(queue)
    , _pin(pin)
    , _flashState(HIGH)
    {
        pinMode(pin, OUTPUT);
    }

    void LEDTask::run(void)
    {
        if(!_queue.isEmpty())
        {
            _info = _queue.pop();
        }
        switch(_info._state)
        {
            case LEDInfo::LEDState::eOn:
                digitalWrite(_pin, HIGH);
                break;
            case LEDInfo::LEDState::eOff:
                digitalWrite(_pin, LOW);
                break;
            case LEDInfo::LEDState::eFlash:
                digitalWrite(_pin, _flashState);
                _flashState = !_flashState;
                break;
        }
    }
}