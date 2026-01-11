#include "domain/Robot.h"
#include "application/AStar.h"
#include "infrastructure/LIDARSensor.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

namespace OSBot {

Robot::Robot(Environment &env)
    : environment_(env), currentPosition_(1, 1), currentState_(State::IDLE),
      running_(false), pathIndex_(0) {}

Robot::~Robot() { stop(); }

void Robot::start() {
  if (running_.load()) {
    std::cerr << "[Robot] Ya está en ejecución\n";
    return;
  }

  running_.store(true);
  currentState_ = State::NAVIGATING;

  // Crear y lanzar el hilo del robot
  robotThread_ = std::make_unique<std::thread>(&Robot::run, this);

  std::cout << "[Robot] Hilo iniciado\n";
}

void Robot::stop() {
  if (!running_.load()) {
    return;
  }

  running_.store(false);
  currentState_ = State::SHUTDOWN;

  // Esperar a que el hilo termine
  if (robotThread_ && robotThread_->joinable()) {
    robotThread_->join();
  }

  std::cout << "[Robot] Hilo detenido\n";
}

State Robot::getState() const { return currentState_; }

Point Robot::getPosition() const { return currentPosition_; }

void Robot::run() {
  std::cout << "[Robot] Bucle principal iniciado\n";
  
  Point lastGoal = environment_.getGoal(); // Guardar objetivo inicial

  while (running_.load()) {
    Point currentGoal = environment_.getGoal();
    
    // Detectar si el objetivo cambió
    if (currentState_ == State::REACHED_GOAL && currentGoal != lastGoal) {
      std::cout << "[Robot] 🎯 Nuevo objetivo detectado! Reiniciando navegación...\n";
      currentState_ = State::NAVIGATING;
      plannedPath_.clear();
      pathIndex_ = 0;
      lastGoal = currentGoal;
    }
    
    if (currentState_ == State::NAVIGATING) {
      navigate();
      lastGoal = currentGoal; // Actualizar objetivo conocido
    } else if (currentState_ == State::REACHED_GOAL) {
      // Robot en espera, esperando nuevo objetivo
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Simular velocidad de procesamiento del robot
    std::this_thread::sleep_for(
        std::chrono::milliseconds(Constants::SIMULATION_SPEED_MS));
  }

  std::cout << "[Robot] Bucle principal finalizado\n";
}

void Robot::navigate() {
  Point goal = environment_.getGoal();

  // Verificar si ya alcanzó el objetivo
  if (currentPosition_ == goal) {
    currentState_ = State::REACHED_GOAL;
    plannedPath_.clear();
    return;
  }

  // 1. Actualizar historial
  addToHistory(currentPosition_);

  // 2. Detectar stuck
  if (isStuck()) {
    std::cout << "🔴 STUCK detectado! Recalculando ruta...\n";
    recalculatePath();
  }

  // 3. Si hay ruta planificada, seguirla
  if (!plannedPath_.empty()) {
    if (followPlannedPath()) {
      currentState_ = State::NAVIGATING;
    } else {
      // Fallo al seguir ruta, recalcular
      plannedPath_.clear();
      pathIndex_ = 0;
    }
  } else {
    // 4. Fallback a greedy
    navigateGreedy();
  }
}

void Robot::navigateGreedy() {
  Point goal = environment_.getGoal();

  // Verificar si ya alcanzó el objetivo
  if (currentPosition_ == goal) {
    currentState_ = State::REACHED_GOAL;
    return;
  }

  // Algoritmo de navegación greedy simple
  // Intenta moverse en la dirección que reduce la distancia al objetivo

  std::vector<Point> possibleMoves;

  // Generar movimientos posibles (4 direcciones)
  possibleMoves.push_back(
      Point(currentPosition_.x + 1, currentPosition_.y)); // Derecha
  possibleMoves.push_back(
      Point(currentPosition_.x - 1, currentPosition_.y)); // Izquierda
  possibleMoves.push_back(
      Point(currentPosition_.x, currentPosition_.y + 1)); // Abajo
  possibleMoves.push_back(
      Point(currentPosition_.x, currentPosition_.y - 1)); // Arriba

  // Filtrar movimientos bloqueados usando sensores
  std::vector<Point> validMoves;
  for (const auto &move : possibleMoves) {
    if (sensePosition(move)) {
      validMoves.push_back(move);
    }
  }

  if (validMoves.empty()) {
    std::cout << "🚫 Robot bloqueado! No hay movimientos válidos\n";
    std::cout << "🔄 Recalculando ruta con A*...\n";
    recalculatePath();
    return;
  }

  // Elegir el movimiento que minimiza la distancia al objetivo
  Point bestMove = validMoves[0];
  int bestDistance = manhattanDistance(bestMove, goal);

  for (const auto &move : validMoves) {
    int distance = manhattanDistance(move, goal);
    if (distance < bestDistance) {
      bestDistance = distance;
      bestMove = move;
    }
  }

  // Ejecutar movimiento
  if (moveTo(bestMove)) {
    currentState_ = State::NAVIGATING;
  } else {
    currentState_ = State::BLOCKED;
  }
}

bool Robot::sensePosition(const Point &targetPos) {
  // Simular lectura de sensores: consultar el entorno
  // Esta llamada está protegida internamente por el mutex del Environment
  return environment_.isPositionFree(targetPos);
}

int Robot::manhattanDistance(const Point &a, const Point &b) const {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool Robot::moveTo(const Point &newPos) {
  // Validar que la posición esté dentro de los límites del mapa
  if (!sensePosition(newPos)) {
    // Posición inválida o bloqueada
    return false;
  }
  
  // *** INTERACCIÓN CON SECCIÓN CRÍTICA ***
  // Esta llamada modifica el mapa compartido (protegido por mutex interno)
  environment_.updateRobotPosition(newPos);
  currentPosition_ = newPos;
  return true;
}

// ========== NUEVAS FUNCIONES: Detección de Stuck y Pathfinding ==========

void Robot::addToHistory(const Point &pos) {
  positionHistory_.push_back(pos);

  // Mantener tamaño máximo
  if (positionHistory_.size() > MAX_HISTORY) {
    positionHistory_.pop_front();
  }
}

bool Robot::isStuck() const {
  if (positionHistory_.size() < STUCK_THRESHOLD) {
    return false;
  }

  // Contar cuántas veces aparece la posición actual en el historial
  int count = std::count(positionHistory_.begin(), positionHistory_.end(),
                         currentPosition_);

  return count >= static_cast<int>(STUCK_THRESHOLD);
}

void Robot::recalculatePath() {
  // Calcular ruta con A* usando el mapa real del Environment
  Route route =
      AStar::find_path(currentPosition_, environment_.getGoal(), environment_);

  // Convertir Route a vector de Points
  plannedPath_.clear();
  for (const auto &waypoint : route) {
    plannedPath_.push_back(
        Point(static_cast<int>(waypoint.x), static_cast<int>(waypoint.y)));
  }

  pathIndex_ = 0;
}

bool Robot::followPlannedPath() {
  if (pathIndex_ >= plannedPath_.size()) {
    // Ruta completada
    plannedPath_.clear();
    pathIndex_ = 0;
    return false;
  }

  Point nextPos = plannedPath_[pathIndex_];

  // Verificar si el siguiente paso es válido
  if (!sensePosition(nextPos)) {
    // El camino está bloqueado, recalcular
    return false;
  }

  // Moverse
  if (moveTo(nextPos)) {
    pathIndex_++;
    return true;
  }

  return false;
}

} // namespace OSBot
