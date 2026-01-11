#include "application/RobotManager.h"
#include <algorithm>
#include <random>
#include <iostream>

namespace OSBot {

RobotManager::RobotManager(Environment& env)
    : environment_(env)
    , nextRobotId_(1)
{
}

RobotManager::~RobotManager() {
    stopAllRobots();
}

int RobotManager::addRobot(const Point& homePosition) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    int robotId = nextRobotId_++;
    auto robot = std::make_unique<Robot>(environment_);
    auto robotInfo = std::make_unique<RobotInfo>(robotId, std::move(robot), homePosition);
    
    robots_[robotId] = std::move(robotInfo);
    return robotId;
}

bool RobotManager::removeRobot(int robotId) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    auto it = robots_.find(robotId);
    if (it != robots_.end()) {
        if (it->second->robot) {
            it->second->robot->stop();
        }
        robots_.erase(it);
        return true;
    }
    return false;
}

void RobotManager::startAllRobots() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    for (auto& [id, info] : robots_) {
        if (info->robot && info->isActive) {
            info->robot->start();
        }
    }
}

void RobotManager::stopAllRobots() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    for (auto& [id, info] : robots_) {
        if (info->robot) {
            info->robot->stop();
        }
    }
}

bool RobotManager::assignTask(int robotId, std::shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    auto it = robots_.find(robotId);
    if (it != robots_.end() && it->second->currentTaskId == -1) {
        it->second->currentTaskId = task->getId();
        task->setAssignedRobot(robotId);
        return true;
    }
    return false;
}

void RobotManager::unassignTask(int robotId) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    auto it = robots_.find(robotId);
    if (it != robots_.end()) {
        it->second->currentTaskId = -1;
    }
}

size_t RobotManager::getRobotCount() const {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    return robots_.size();
}

std::vector<int> RobotManager::getRobotIds() const {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    std::vector<int> ids;
    ids.reserve(robots_.size());
    for (const auto& [id, _] : robots_) {
        ids.push_back(id);
    }
    return ids;
}

RobotInfo* RobotManager::getRobotInfo(int robotId) {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    auto it = robots_.find(robotId);
    return (it != robots_.end()) ? it->second.get() : nullptr;
}

const RobotInfo* RobotManager::getRobotInfo(int robotId) const {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    auto it = robots_.find(robotId);
    return (it != robots_.end()) ? it->second.get() : nullptr;
}

std::vector<const RobotInfo*> RobotManager::getAllRobots() const {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    std::vector<const RobotInfo*> result;
    result.reserve(robots_.size());
    for (const auto& [id, info] : robots_) {
        result.push_back(info.get());
    }
    return result;
}

void RobotManager::update() {
  std::lock_guard<std::mutex> lock(robotsMutex_);

  for (auto &pair : robots_) {
    auto &info = pair.second;
    if (info && info->robot) {
      // Update state
      State previousState = info->currentState;
      info->currentState = info->robot->getState();
      info->lastUpdateTime = std::chrono::system_clock::now();
      
      // Update cells traveled from robot's position history
      info->cellsTraveled = info->robot->getCellsTraveled();
      
      // Update obstacles avoided
      info->obstaclesAvoided = info->robot->getObstaclesAvoided();
      
      // Distance is approximately cells traveled (each cell ~= 1 meter)
      info->totalDistanceTraveled = static_cast<double>(info->cellsTraveled);
      
      // If robot just reached goal, increment completed tasks
      if (previousState == State::NAVIGATING && 
          info->currentState == State::REACHED_GOAL) {
        info->tasksCompleted++;
      }
      
      // Update activity
      updateRobotStats(*info);
    }
  }
}

void RobotManager::updateRobotStats(RobotInfo& info) {
    // Aquí se pueden actualizar estadísticas como distancia recorrida, etc.
    info.lastUpdateTime = std::chrono::system_clock::now();
}

bool RobotManager::isRobotAvailable(int robotId) const {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    auto it = robots_.find(robotId);
    if (it != robots_.end()) {
        return it->second->isActive && 
               it->second->currentTaskId == -1 && 
               it->second->currentState == State::IDLE;
    }
    return false;
}

int RobotManager::findAvailableRobot() const {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    for (const auto& [id, info] : robots_) {
        if (info->isActive && 
            info->currentTaskId == -1 && 
            info->currentState == State::IDLE) {
            return id;
        }
    }
    return -1; // No hay robots disponibles
}

void RobotManager::resetRobotPosition() {
    std::lock_guard<std::mutex> lock(robotsMutex_);
    
    // Regenerar obstáculos aleatorios (25% del área)
    environment_.generateRandomObstacles(25);
    std::cout << "[RobotManager] Obstáculos regenerados" << std::endl;
    
    // Obtener dimensiones del entorno
    int width = environment_.getWidth();
    int height = environment_.getHeight();
    
    // Generar posición aleatoria válida (evitando bordes)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(2, width - 3);
    std::uniform_int_distribution<> distY(2, height - 3);
    
    for (auto& [id, info] : robots_) {
        if (info && info->robot) {
            // Detener el robot actual
            info->robot->stop();
            
            // Generar nueva posición aleatoria
            Point newPos;
            bool validPosition = false;
            int attempts = 0;
            
            while (!validPosition && attempts < 100) {
                newPos.x = distX(gen);
                newPos.y = distY(gen);
                
                // Verificar que la posición esté libre
                if (environment_.isPositionFree(newPos)) {
                    validPosition = true;
                }
                attempts++;
            }
            
            if (!validPosition) {
                // Fallback a posición por defecto
                newPos = Point(5, 5);
            }
            
            // Crear nuevo robot en la nueva posición
            auto newRobot = std::make_unique<Robot>(environment_);
            info->robot = std::move(newRobot);
            info->homePosition = newPos;
            
            // Resetear estadísticas
            info->tasksCompleted = 0;
            info->tasksFailed = 0;
            info->totalDistanceTraveled = 0.0;
            info->cellsTraveled = 0;
            info->obstaclesAvoided = 0;
            info->currentTaskId = -1;
            info->currentState = State::IDLE;
            
            // Actualizar posición del robot en el entorno
            environment_.updateRobotPosition(newPos);
            
            // Reiniciar el robot
            info->robot->start();
            
            std::cout << "[RobotManager] Robot " << id << " reposicionado a (" 
                      << newPos.x << ", " << newPos.y << ")" << std::endl;
        }
    }
}

} // namespace OSBot

