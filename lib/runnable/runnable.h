#pragma once
#include <stdint.h>

class Task;

namespace scheduler
{
    class SchedulerWrapper
    {
        public:
            static void run();

    };

    class Runnable
    {
        public:
            Runnable(uint32_t scanRate);
            virtual ~Runnable();
        protected:
            virtual void run(void) = 0;
        private:
            Task* _task;
    };
}