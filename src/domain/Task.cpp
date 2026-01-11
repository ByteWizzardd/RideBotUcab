#include "domain/Task.h"
#include <algorithm>

namespace OSBot {

Task::Task(int id, const std::vector<Point>& waypoints, TaskPriority priority)
    : id_(id)
    , waypoints_(waypoints)
    , currentWaypointIndex_(0)
    , priority_(priority)
    , status_(TaskStatus::PENDING)
    , assignedRobotId_(-1)
    , createdTime_(std::chrono::system_clock::now())
    , estimatedDuration_(0.0)
{
}

Point Task::getCurrentWaypoint() const {
    if (currentWaypointIndex_ < static_cast<int>(waypoints_.size())) {
        return waypoints_[currentWaypointIndex_];
    }
    return waypoints_.empty() ? Point(0, 0) : waypoints_.back();
}

void Task::setStatus(TaskStatus status) {
    status_ = status;
    
    if (status == TaskStatus::IN_PROGRESS && 
        startTime_ == std::chrono::system_clock::time_point{}) {
        startTime_ = std::chrono::system_clock::now();
    }
    
    if (status == TaskStatus::COMPLETED || status == TaskStatus::FAILED || status == TaskStatus::CANCELLED) {
        completionTime_ = std::chrono::system_clock::now();
    }
}

void Task::setAssignedRobot(int robotId) {
    assignedRobotId_ = robotId;
    if (status_ == TaskStatus::PENDING) {
        setStatus(TaskStatus::ASSIGNED);
    }
}

void Task::advanceToNextWaypoint() {
    if (currentWaypointIndex_ < static_cast<int>(waypoints_.size())) {
        currentWaypointIndex_++;
    }
}

double Task::getProgress() const {
    if (waypoints_.empty()) return 0.0;
    return static_cast<double>(currentWaypointIndex_) / waypoints_.size() * 100.0;
}

} // namespace OSBot

