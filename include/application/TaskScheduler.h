#ifndef RIDEBOT_TASKSCHEDULER_H
#define RIDEBOT_TASKSCHEDULER_H

#include "domain/Task.h"
#include <vector>
#include <queue>

namespace OSBot {

class TaskScheduler {
public:
    TaskScheduler();

    void add_task(const Task& task);
    Task get_next_task();
    bool has_tasks() const;
    std::vector<Task> getAllTasks() const;
    void clear();

private:
    struct TaskComparator {
        bool operator()(const Task& a, const Task& b) {
            return a.getPriority() < b.getPriority();
        }
    };

    std::vector<Task> tasks_;
};

} // namespace OSBot

#endif //RIDEBOT_TASKSCHEDULER_H
