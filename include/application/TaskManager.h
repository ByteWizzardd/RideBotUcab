#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "domain/Task.h"
#include "RobotManager.h"
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <map>
#include <functional>

namespace OSBot {

/**
 * @brief Comparador para la cola de prioridad de tareas
 */
struct TaskPriorityCompare {
    bool operator()(const std::shared_ptr<Task>& a, const std::shared_ptr<Task>& b) const {
        return static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
    }
};

/**
 * @brief Gestor de tareas y planificación
 * Implementa algoritmos de planificación de tareas para múltiples robots
 */
class TaskManager {
public:
    explicit TaskManager(RobotManager& robotManager);
    ~TaskManager();
    
    // Gestión de tareas
    int createTask(const std::vector<Point>& waypoints, TaskPriority priority = TaskPriority::NORMAL);
    bool cancelTask(int taskId);
    std::shared_ptr<Task> getTask(int taskId) const;
    
    // Planificación
    void scheduleNextTasks();
    void update();
    
    // Consultas
    size_t getPendingTaskCount() const;
    size_t getActiveTaskCount() const;
    size_t getCompletedTaskCount() const;
    std::vector<std::shared_ptr<Task>> getAllTasks() const;
    std::vector<std::shared_ptr<Task>> getTasksByStatus(TaskStatus status) const;
    
    // Estadísticas
    double getAverageCompletionTime() const;
    double getSuccessRate() const;
    
private:
    RobotManager& robotManager_;
    std::map<int, std::shared_ptr<Task>> allTasks_;
    std::priority_queue<std::shared_ptr<Task>, 
                        std::vector<std::shared_ptr<Task>>, 
                        TaskPriorityCompare> pendingTasks_;
    
    int nextTaskId_;
    mutable std::mutex tasksMutex_;
    
    // Algoritmos de planificación
    bool assignTaskToRobot(std::shared_ptr<Task> task);
    int findBestRobotForTask(const Task& task) const;
    double calculateTaskCost(int robotId, const Task& task) const;
};

} // namespace OSBot

#endif // TASK_MANAGER_H

