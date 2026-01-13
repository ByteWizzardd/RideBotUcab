#include "application/TaskScheduler.h"
#include <algorithm>

namespace OSBot {

TaskScheduler::TaskScheduler() = default;

void TaskScheduler::add_task(const Task& task) {
    tasks_.push_back(task);
    std::push_heap(tasks_.begin(), tasks_.end(), TaskComparator());
}

Task TaskScheduler::get_next_task() {
    std::pop_heap(tasks_.begin(), tasks_.end(), TaskComparator());
    Task next_task = tasks_.back();
    tasks_.pop_back();
    return next_task;
}

bool TaskScheduler::has_tasks() const {
    return !tasks_.empty();
}

std::vector<Task> TaskScheduler::getAllTasks() const {
    return tasks_;
}

void TaskScheduler::clear() {
    tasks_.clear();
}

} // namespace OSBot
