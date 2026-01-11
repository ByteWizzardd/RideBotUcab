#ifndef ROBOT_MANAGER_H
#define ROBOT_MANAGER_H

#include "domain/Global.h"
#include "domain/Robot.h"
#include "domain/Task.h"
#include "domain/Environment.h"
#include <memory>
#include <vector>
#include <mutex>
#include <map>

namespace OSBot {

/**
 * @brief Información extendida de un robot
 */
struct RobotInfo {
    int id;
    std::unique_ptr<Robot> robot;
    Point homePosition;
    State currentState;
    int currentTaskId;
    int tasksCompleted;
    int tasksFailed;
    double totalDistanceTraveled;
    int cellsTraveled;
    std::chrono::system_clock::time_point lastUpdateTime;
    bool isActive;
    
    RobotInfo(int robotId, std::unique_ptr<Robot> robotPtr, Point home)
        : id(robotId)
        , robot(std::move(robotPtr))
        , homePosition(home)
        , currentState(State::IDLE)
        , currentTaskId(-1)
        , tasksCompleted(0)
        , tasksFailed(0)
        , totalDistanceTraveled(0.0)
        , cellsTraveled(0)
        , lastUpdateTime(std::chrono::system_clock::now())
        , isActive(true)
    {}
};

/**
 * @brief Gestor de múltiples robots
 * Coordina la operación de múltiples robots en el entorno
 */
class RobotManager {
public:
    explicit RobotManager(Environment& env);
    ~RobotManager();
    
    // Gestión de robots
    int addRobot(const Point& homePosition);
    bool removeRobot(int robotId);
    void startAllRobots();
    void stopAllRobots();
    
    // Asignación de tareas
    bool assignTask(int robotId, std::shared_ptr<Task> task);
    void unassignTask(int robotId);
    
    // Consultas
    size_t getRobotCount() const;
    std::vector<int> getRobotIds() const;
    RobotInfo* getRobotInfo(int robotId);
    const RobotInfo* getRobotInfo(int robotId) const;
    std::vector<const RobotInfo*> getAllRobots() const;
    
    // Actualización
    void update();
    
    // Estado
    bool isRobotAvailable(int robotId) const;
    int findAvailableRobot() const;
    
private:
    Environment& environment_;
    std::map<int, std::unique_ptr<RobotInfo>> robots_;
    int nextRobotId_;
    mutable std::mutex robotsMutex_;
    
    void updateRobotStats(RobotInfo& info);
};

} // namespace OSBot

#endif // ROBOT_MANAGER_H

