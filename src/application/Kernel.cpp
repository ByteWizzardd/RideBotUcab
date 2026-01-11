#include "application/Kernel.h"
#include "infrastructure/WebServer.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace OSBot {

Kernel::Kernel() : running_(false), paused_(false), 
                   simulationSpeed_(Constants::SIMULATION_SPEED_MS) {}

Kernel::~Kernel() { shutdown(); }

bool Kernel::initialize() {
  std::cout << "[Kernel] Inicializando sistema operativo multi-robot..."
            << std::endl;

  // Inicializar entorno
  environment_ = std::make_unique<Environment>(Constants::GRID_WIDTH,
                                               Constants::GRID_HEIGHT);
  environment_->initialize();
  environment_->start();
  std::cout << "[Kernel] ✓ Entorno inicializado" << std::endl;

  // Inicializar gestor de robots
  robotManager_ = std::make_unique<RobotManager>(*environment_);
  std::cout << "[Kernel] ✓ Gestor de robots inicializado" << std::endl;

  // Inicializar gestor de tareas
  taskManager_ = std::make_unique<TaskManager>(*robotManager_);
  std::cout << "[Kernel] ✓ Gestor de tareas inicializado" << std::endl;

  // Inicializar servidor web
  webServer_ = std::make_unique<WebServer>(*this, 8080);
  webServer_->start();
  std::cout << "[Kernel] ✓ Servidor web iniciado en http://localhost:8080" 
            << std::endl;

  return true;
}

void Kernel::start() {
  if (running_)
    return;

  std::cout << "[Kernel] Iniciando sistema..." << std::endl;

  running_ = true;

  // Iniciar robots
  robotManager_->startAllRobots();
  std::cout << "[Kernel] ✓ Robots iniciados" << std::endl;

  // Iniciar hilo de actualización
  updateThread_ = std::make_unique<std::thread>(&Kernel::updateLoop, this);
  std::cout << "[Kernel] ✓ Hilo de actualización iniciado" << std::endl;
}

void Kernel::run(int durationSeconds) {
  printSystemInfo();

  std::cout << "\n[Kernel] Sistema operativo en ejecución" << std::endl;
  std::cout << "[Kernel] Presiona Ctrl+C para detener el sistema\n"
            << std::endl;

  auto startTime = std::chrono::steady_clock::now();

  while (running_) {
    // Renderizar entorno
    environment_->render();

    // Verificar duración
    if (durationSeconds > 0) {
      auto currentTime = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                         currentTime - startTime)
                         .count();
      if (elapsed >= durationSeconds) {
        shutdown();
        break;
      }
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(Constants::SIMULATION_SPEED_MS));
  }
}

void Kernel::shutdown() {
  if (!running_)
    return;

  std::cout << "\n[Kernel] Iniciando apagado del sistema..." << std::endl;
  running_ = false;

  // Detener servidor web
  if (webServer_) {
    webServer_->stop();
  }
  std::cout << "[Kernel] ✓ Servidor web detenido" << std::endl;

  // Detener hilo de actualización
  if (updateThread_ && updateThread_->joinable()) {
    updateThread_->join();
  }
  std::cout << "[Kernel] ✓ Hilo de actualización detenido" << std::endl;

  // Detener robots
  robotManager_->stopAllRobots();
  std::cout << "[Kernel] ✓ Robots detenidos" << std::endl;

  // Detener entorno
  environment_->stop();
  std::cout << "[Kernel] ✓ Entorno detenido" << std::endl;

  std::cout << "[Kernel] Sistema apagado correctamente" << std::endl;
}

void Kernel::updateLoop() {
  std::cout << "[Update Thread] Bucle de actualización iniciado" << std::endl;

  while (running_) {
    // Solo actualizar si no está pausado
    if (!paused_) {
      // Actualizar robots
      robotManager_->update();

      // Actualizar tareas
      taskManager_->update();

      // Planificar nuevas tareas
      taskManager_->scheduleNextTasks();
    }

    // Esperar antes de la siguiente actualización (usando velocidad configurable)
    std::this_thread::sleep_for(
        std::chrono::milliseconds(simulationSpeed_.load()));
  }

  std::cout << "[Update Thread] Bucle de actualización finalizado" << std::endl;
}

void Kernel::printSystemInfo() {
  std::cout << "\n╔════════════════════════════════════════════════════╗"
            << std::endl;
  std::cout << "║          OS-BOT KERNEL v2.0.0 (Multi-Robot)       ║"
            << std::endl;
  std::cout << "║      Sistema Operativo para Robots Autónomos      ║"
            << std::endl;
  std::cout << "╠════════════════════════════════════════════════════╣"
            << std::endl;
  std::cout << "║ Arquitectura: Monolítica con Microservicios       ║"
            << std::endl;
  std::cout << "║ Gestión de Hilos: std::thread (C++17)             ║"
            << std::endl;
  std::cout << "║ Sincronización: std::mutex + std::lock_guard      ║"
            << std::endl;
  std::cout << "╠════════════════════════════════════════════════════╣"
            << std::endl;
  std::cout << "║ Subsistemas:                                       ║"
            << std::endl;
  std::cout << "║   • Environment (Entorno/Mapa)                     ║"
            << std::endl;
  std::cout << "║   • RobotManager (Gestión Multi-Robot)             ║"
            << std::endl;
  std::cout << "║   • TaskManager (Planificación de Tareas)          ║"
            << std::endl;
  std::cout << "║   • Statistics (Métricas y Monitoreo)              ║"
            << std::endl;
  std::cout << "╠════════════════════════════════════════════════════╣"
            << std::endl;
  std::cout << "║ Grid: " << Constants::GRID_WIDTH << "x"
            << Constants::GRID_HEIGHT;
  std::cout << "                                          ║" << std::endl;
  std::cout << "║ Velocidad: " << Constants::SIMULATION_SPEED_MS << "ms/tick";
  std::cout << "                                     ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝"
            << std::endl;
}

} // namespace OSBot
