#ifndef KERNEL_H
#define KERNEL_H

#include "RobotManager.h"
#include "TaskManager.h"
#include "domain/Environment.h"
#include "domain/Global.h"
#include "infrastructure/Storage.h"
#include <atomic>
#include <memory>
#include <thread>

// Forward declaration to avoid circular dependency
namespace OSBot {
class WebServer;
}

namespace OSBot {

/**
 * @class Kernel
 * @brief Núcleo monolítico del sistema operativo multi-robot
 *
 * El Kernel orquesta todos los subsistemas:
 * - Gestión de múltiples robots
 * - Sistema de tareas y planificación
 * - Estadísticas y monitoreo
 * - Sincronización de hilos
 */
class Kernel {
public:
  /**
   * @brief Constructor
   */
  Kernel();

  /**
   * @brief Destructor - Asegura el apagado limpio de todos los hilos
   */
  ~Kernel();

  /**
   * @brief Inicializa todos los subsistemas del kernel
   * @return true si la inicialización fue exitosa
   */
  bool initialize();

  /**
   * @brief Inicia la ejecución del sistema (lanza hilos)
   */
  void start();

  /**
   * @brief Ejecuta el bucle principal del kernel
   * @param durationSeconds Duración en segundos (0 = infinito)
   */
  void run(int durationSeconds = 0);

  /**
   * @brief Detiene todos los subsistemas y hilos
   */
  void shutdown();

  // Acceso a subsistemas (para control externo si es necesario)
  Environment &getEnvironment() { return *environment_; }
  RobotManager &getRobotManager() { return *robotManager_; }
  TaskManager &getTaskManager() { return *taskManager_; }

  // Control de pausa y velocidad (para WebServer)
  void setPaused(bool paused) { paused_ = paused; }
  bool isPaused() const { return paused_; }
  void setSimulationSpeed(int speed) { simulationSpeed_ = speed; }
  int getSimulationSpeed() const { return simulationSpeed_; }

private:
  // Subsistemas principales
  std::unique_ptr<Environment> environment_;
  std::unique_ptr<RobotManager> robotManager_;
  std::unique_ptr<TaskManager> taskManager_;
  std::unique_ptr<WebServer> webServer_;

  // Control de hilos
  std::unique_ptr<std::thread> updateThread_;
  std::atomic<bool> running_;
  std::atomic<bool> paused_;
  std::atomic<int> simulationSpeed_;

  /**
   * @brief Bucle de actualización del sistema
   * Actualiza robots, tareas
   */
  void updateLoop();

  /**
   * @brief Imprime información del sistema al iniciar
   */
  void printSystemInfo();
};

} // namespace OSBot

#endif // KERNEL_H
