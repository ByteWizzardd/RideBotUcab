#include <gtest/gtest.h>
#include "../include/TaskScheduler.h"

TEST(TaskSchedulerTest, PriorityQueue) {
    TaskScheduler scheduler;
    scheduler.add_task({1, TaskPriority::LOW, {0, 0}});
    scheduler.add_task({2, TaskPriority::HIGH, {0, 0}});
    scheduler.add_task({3, TaskPriority::MEDIUM, {0, 0}});

    ASSERT_EQ(scheduler.get_next_task().id, 2);
    ASSERT_EQ(scheduler.get_next_task().id, 3);
    ASSERT_EQ(scheduler.get_next_task().id, 1);
}
