#include "runnable.h"
#include <functional>
#include <TaskScheduler.h>

namespace scheduler
{
    Scheduler runner;

    void SchedulerWrapper::run()
    {
        runner.execute();
    }

    Runnable::Runnable(uint32_t scanRate)
    {
        _task = new Task;
        TaskCallback callback = std::bind(&Runnable::run, this);
        _task->set(scanRate * TASK_MILLISECOND, TASK_FOREVER, callback);
        runner.addTask(*_task);
        _task->enable();
    }

    Runnable::~Runnable()
    {
        delete _task;
    }
}
