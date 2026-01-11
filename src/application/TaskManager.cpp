#include "application/TaskManager.h"
#include <algorithm>
#include <cmath>

namespace OSBot {

TaskManager::TaskManager(RobotManager& robotManager)
    : robotManager_(robotManager)
    , nextTaskId_(1)
{
}

TaskManager::~TaskManager() {
}

int TaskManager::createTask(const std::vector<Point>& waypoints, TaskPriority priority) {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    int taskId = nextTaskId_++;
    auto task = std::make_shared<Task>(taskId, waypoints, priority);
    
    allTasks_[taskId] = task;
    pendingTasks_.push(task);
    
    return taskId;
}

bool TaskManager::cancelTask(int taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    auto it = allTasks_.find(taskId);
    if (it != allTasks_.end()) {
        auto task = it->second;
        
        if (task->getStatus() == TaskStatus::PENDING || 
            task->getStatus() == TaskStatus::ASSIGNED) {
            task->setStatus(TaskStatus::CANCELLED);
            
            // Desasignar del robot si estaba asignada
            if (task->getAssignedRobotId() != -1) {
                robotManager_.unassignTask(task->getAssignedRobotId());
            }
            return true;
        }
    }
    return false;
}

std::shared_ptr<Task> TaskManager::getTask(int taskId) const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    auto it = allTasks_.find(taskId);
    return (it != allTasks_.end()) ? it->second : nullptr;
}

void TaskManager::scheduleNextTasks() {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    // Intentar asignar tareas pendientes a robots disponibles
    while (!pendingTasks_.empty()) {
        auto task = pendingTasks_.top();
        
        // Verificar que la tarea siga pendiente
        if (task->getStatus() != TaskStatus::PENDING) {
            pendingTasks_.pop();
            continue;
        }
        
        // Intentar asignar a un robot
        if (assignTaskToRobot(task)) {
            pendingTasks_.pop();
        } else {
            // No hay robots disponibles, esperar
            break;
        }
    }
}

void TaskManager::update() {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    // Actualizar el estado de las tareas activas
    for (auto& [id, task] : allTasks_) {
        if (task->isActive()) {
            int robotId = task->getAssignedRobotId();
            if (robotId != -1) {
                const RobotInfo* robotInfo = robotManager_.getRobotInfo(robotId);
                if (robotInfo) {
                    // Verificar si el robot alcanzó el waypoint actual
                    Point robotPos = robotInfo->robot->getPosition();
                    Point targetWaypoint = task->getCurrentWaypoint();
                    
                    if (robotPos == targetWaypoint) {
                        task->advanceToNextWaypoint();
                        
                        if (!task->hasMoreWaypoints()) {
                            task->setStatus(TaskStatus::COMPLETED);
                            robotManager_.unassignTask(robotId);
                        }
                    }
                    
                    // Verificar si el robot está bloqueado
                    if (robotInfo->currentState == State::BLOCKED) {
                        task->setStatus(TaskStatus::FAILED);
                        robotManager_.unassignTask(robotId);
                    }
                }
            }
        }
    }
}

size_t TaskManager::getPendingTaskCount() const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    return pendingTasks_.size();
}

size_t TaskManager::getActiveTaskCount() const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    size_t count = 0;
    for (const auto& [id, task] : allTasks_) {
        if (task->isActive()) {
            count++;
        }
    }
    return count;
}

size_t TaskManager::getCompletedTaskCount() const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    size_t count = 0;
    for (const auto& [id, task] : allTasks_) {
        if (task->isCompleted()) {
            count++;
        }
    }
    return count;
}

std::vector<std::shared_ptr<Task>> TaskManager::getAllTasks() const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    std::vector<std::shared_ptr<Task>> tasks;
    tasks.reserve(allTasks_.size());
    for (const auto& [id, task] : allTasks_) {
        tasks.push_back(task);
    }
    return tasks;
}

std::vector<std::shared_ptr<Task>> TaskManager::getTasksByStatus(TaskStatus status) const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    std::vector<std::shared_ptr<Task>> tasks;
    for (const auto& [id, task] : allTasks_) {
        if (task->getStatus() == status) {
            tasks.push_back(task);
        }
    }
    return tasks;
}

double TaskManager::getAverageCompletionTime() const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    double totalTime = 0.0;
    int count = 0;
    
    for (const auto& [id, task] : allTasks_) {
        if (task->isCompleted()) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                task->getCompletionTime() - task->getStartTime()
            ).count();
            totalTime += duration;
            count++;
        }
    }
    
    return (count > 0) ? totalTime / count : 0.0;
}

double TaskManager::getSuccessRate() const {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    
    int completed = 0;
    int total = 0;
    
    for (const auto& [id, task] : allTasks_) {
        if (task->isCompleted() || task->isFailed()) {
            total++;
            if (task->isCompleted()) {
                completed++;
            }
        }
    }
    
    return (total > 0) ? (static_cast<double>(completed) / total * 100.0) : 100.0;
}

bool TaskManager::assignTaskToRobot(std::shared_ptr<Task> task) {
    int bestRobotId = findBestRobotForTask(*task);
    
    if (bestRobotId != -1) {
        if (robotManager_.assignTask(bestRobotId, task)) {
            task->setStatus(TaskStatus::ASSIGNED);
            return true;
        }
    }
    return false;
}

int TaskManager::findBestRobotForTask(const Task& task) const {
    auto robotIds = robotManager_.getRobotIds();
    
    int bestRobotId = -1;
    double bestCost = std::numeric_limits<double>::max();
    
    for (int robotId : robotIds) {
        if (robotManager_.isRobotAvailable(robotId)) {
            double cost = calculateTaskCost(robotId, task);
            if (cost < bestCost) {
                bestCost = cost;
                bestRobotId = robotId;
            }
        }
    }
    
    return bestRobotId;
}

double TaskManager::calculateTaskCost(int robotId, const Task& task) const {
    const RobotInfo* robotInfo = robotManager_.getRobotInfo(robotId);
    if (!robotInfo || !robotInfo->robot) {
        return std::numeric_limits<double>::max();
    }
    
    Point robotPos = robotInfo->robot->getPosition();
    Point taskStart = task.getWaypoints().front();
    
    // Costo basado en distancia Manhattan
    int dx = std::abs(robotPos.x - taskStart.x);
    int dy = std::abs(robotPos.y - taskStart.y);
    
    return static_cast<double>(dx + dy);
}

} // namespace OSBot

