#include "application/TaskScheduler.h"

namespace OSBot {

TaskScheduler::TaskScheduler() = default;

void TaskScheduler::add_task(const Task& task) {
    task_queue.push(task);
}

Task TaskScheduler::get_next_task() {
    Task next_task = task_queue.top();
    task_queue.pop();
    return next_task;
}

bool TaskScheduler::has_tasks() const {
    return !task_queue.empty();
}

} // namespace OSBot
