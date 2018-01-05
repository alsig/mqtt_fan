#include "ledTask.h"

namespace ledTask
{
    static const uint8_t scanRate = 50;
    LEDTask::LEDTask(QueueArray<LEDInfo>* queue, const uint8_t pin)
    : Runnable(scanRate)
    , _queue(queue)
    , _pin(pin)
    , _flashState(HIGH)
    , _count(0)
    {
        pinMode(pin, OUTPUT);
    }

    void LEDTask::run(void)
    {
        if(!_queue->isEmpty())
        {
            _info = _queue->pop();
            _count = 0;
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
            
                if(_count >= _info._period)
                {
                    digitalWrite(_pin, _flashState);
                    _flashState = !_flashState;
                    _count = 0;
                }
                ++_count;
                break;
        }
    }
}