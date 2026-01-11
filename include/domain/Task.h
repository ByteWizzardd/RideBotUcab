#ifndef TASK_H
#define TASK_H

#include "Global.h"
#include <string>
#include <vector>
#include <chrono>
#include <memory>

namespace OSBot {

/**
 * @brief Prioridad de una tarea
 */
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    URGENT = 3
};

/**
 * @brief Estado de una tarea
 */
enum class TaskStatus {
    PENDING,       // En cola esperando asignación
    ASSIGNED,      // Asignada a un robot
    IN_PROGRESS,   // En ejecución
    COMPLETED,     // Completada exitosamente
    FAILED,        // Falló
    CANCELLED      // Cancelada
};

/**
 * @brief Representa una tarea que debe realizar un robot
 */
class Task {
public:
    using TimePoint = std::chrono::system_clock::time_point;
    
    Task(int id, const std::vector<Point>& waypoints, TaskPriority priority = TaskPriority::NORMAL);
    
    // Getters
    int getId() const { return id_; }
    const std::vector<Point>& getWaypoints() const { return waypoints_; }
    Point getCurrentWaypoint() const;
    TaskPriority getPriority() const { return priority_; }
    TaskStatus getStatus() const { return status_; }
    int getAssignedRobotId() const { return assignedRobotId_; }
    TimePoint getCreatedTime() const { return createdTime_; }
    TimePoint getStartTime() const { return startTime_; }
    TimePoint getCompletionTime() const { return completionTime_; }
    double getEstimatedDuration() const { return estimatedDuration_; }
    int getCurrentWaypointIndex() const { return currentWaypointIndex_; }
    
    // Setters
    void setStatus(TaskStatus status);
    void setAssignedRobot(int robotId);
    void advanceToNextWaypoint();
    void setEstimatedDuration(double seconds) { estimatedDuration_ = seconds; }
    
    // Utilidades
    bool isCompleted() const { return status_ == TaskStatus::COMPLETED; }
    bool isFailed() const { return status_ == TaskStatus::FAILED; }
    bool isActive() const { 
        return status_ == TaskStatus::ASSIGNED || status_ == TaskStatus::IN_PROGRESS; 
    }
    bool hasMoreWaypoints() const { return currentWaypointIndex_ < static_cast<int>(waypoints_.size()); }
    double getProgress() const;
    
private:
    int id_;
    std::vector<Point> waypoints_;
    int currentWaypointIndex_;
    TaskPriority priority_;
    TaskStatus status_;
    int assignedRobotId_;
    
    TimePoint createdTime_;
    TimePoint startTime_;
    TimePoint completionTime_;
    double estimatedDuration_;  // En segundos
};

} // namespace OSBot

#endif // TASK_H
