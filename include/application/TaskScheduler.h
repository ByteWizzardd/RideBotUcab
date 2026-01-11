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

private:
    struct TaskComparator {
        bool operator()(const Task& a, const Task& b) {
            return a.getPriority() < b.getPriority();
        }
    };

    std::priority_queue<Task, std::vector<Task>, TaskComparator> task_queue;
};

} // namespace OSBot

#endif //RIDEBOT_TASKSCHEDULER_H
