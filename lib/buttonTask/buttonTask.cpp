#include "buttonTask.h"

namespace buttonTask
{
    ButtonTask::ButtonTask(QueueArray<ButtonState>* queue, const uint8_t pin, const uint8_t scanRate)
    : Runnable(scanRate) 
    , _queue(queue)
    , _pin(pin)
    , _lastPinState(HIGH)
    {
        pinMode(_pin, INPUT);
    }   

    void ButtonTask::run()
    {
        const uint8_t currentPinState = digitalRead(_pin);
        if(currentPinState == HIGH && _lastPinState == LOW)
        {
            Serial.println("ePushed");
            _buttonState = ButtonState::ePushed;
            _queue->push(_buttonState);
        }
        else if(currentPinState == LOW && _lastPinState == HIGH)
        {
            Serial.println("eReleased");          
            _buttonState = ButtonState::eReleased;
            _counter = 0; 
            _queue->push(_buttonState);            
        }
        _lastPinState = currentPinState;

        if(_buttonState == ButtonState::ePushed)
        {
            ++_counter;
            if(_counter > _longPushCount)
            {
                Serial.println("eLongPush");                           
                _buttonState = ButtonState::eLongPush;
                _queue->push(_buttonState);
            }
        }


        
    } 
}