#include "domain/Environment.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <thread>

namespace OSBot {

Environment::Environment(int width, int height)
    : width_(width), height_(height), robotPosition_(1, 1), running_(false),
      currentObstacleCount_(0) {

  // Generar posición aleatoria para la meta
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distX(2, width - 3);
  std::uniform_int_distribution<> distY(2, height - 3);

  // Asegurar que la meta no esté en la posición inicial del robot
  do {
    goalPosition_.x = distX(gen);
    goalPosition_.y = distY(gen);
  } while (goalPosition_.x == robotPosition_.x &&
           goalPosition_.y == robotPosition_.y);

  // Inicializar grafo
  initialize();
}

Environment::~Environment() { stop(); }

void Environment::initialize() {
  // SECCIÓN CRÍTICA: Modificación del mapa compartido
  std::lock_guard<std::mutex> lock(mapMutex_);

  graph_.clear();
  graph_.reserve(width_ * height_);

  // Crear nodos
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      Node node;
      node.id = y * width_ + x;
      node.x = x;
      node.y = y;
      node.type = CellType::EMPTY;
      graph_.push_back(node);
    }
  }

  // Establecer vecinos (4-conectividad)
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      Node *node = getNode(x, y);
      if (node) {
        node->north = getNode(x, y - 1);
        node->south = getNode(x, y + 1);
        node->west = getNode(x - 1, y);
        node->east = getNode(x + 1, y);
      }
    }
  }

  // Colocar bordes (obstáculos)
  for (int x = 0; x < width_; ++x) {
    getNode(x, 0)->type = CellType::OBSTACLE;
    getNode(x, height_ - 1)->type = CellType::OBSTACLE;
  }
  for (int y = 0; y < height_; ++y) {
    getNode(0, y)->type = CellType::OBSTACLE;
    getNode(width_ - 1, y)->type = CellType::OBSTACLE;
  }

  // Colocar obstáculos aleatorios al inicio
  placeObstacles();

  // Colocar objetivo en el grid
  Node *goalNode = getNode(goalPosition_.x, goalPosition_.y);
  if (goalNode)
    goalNode->type = CellType::GOAL;
    
  // NOTA: Los robots YA NO se marcan en el grid (multi-robot fix)
  // Las posiciones de robots se manejan en RobotManager
}

Environment::Node *Environment::getNode(int x, int y) {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    return nullptr;
  }
  return &graph_[y * width_ + x];
}

void Environment::placeObstacles() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distX(2, width_ - 3);
  std::uniform_int_distribution<> distY(2, height_ - 3);

  // Colocar bastantes obstáculos aleatorios para evaluar navegación
  int numObstacles =
      (width_ * height_) / 4; // ~25% del área (sin contar bordes)

  for (int i = 0; i < numObstacles; ++i) {
    int x = distX(gen);
    int y = distY(gen);

    // No colocar obstáculos en posición inicial del robot o objetivo
    if ((x != robotPosition_.x || y != robotPosition_.y) &&
        (x != goalPosition_.x || y != goalPosition_.y)) {
      Node *node = getNode(x, y);
      if (node)
        node->type = CellType::OBSTACLE;
    }
  }
}

void Environment::clearScreen() { (void)std::system("clear"); }

void Environment::render() {
  // *** SECCIÓN CRÍTICA ***
  // Lock para lectura del mapa compartido durante el renderizado
  std::lock_guard<std::mutex> lock(mapMutex_);

  clearScreen();

  std::cout << "╔══════════════════════════════════════════════════╗\n";
  std::cout << "║         OS-BOT - Simulación de Navegación       ║\n";
  std::cout << "╚══════════════════════════════════════════════════╝\n\n";

  // Renderizar grid
  for (int y = 0; y < height_; ++y) {
    std::cout << "  ";
    for (int x = 0; x < width_; ++x) {
      Node *node = getNode(x, y);
      if (!node)
        continue;

      switch (node->type) {
      case CellType::EMPTY:
        std::cout << " · ";
        break;
      case CellType::OBSTACLE:
        std::cout << " █ ";
        break;
      case CellType::ROBOT:
        std::cout << " R ";
        break;
      case CellType::GOAL:
        std::cout << " G ";
        break;
      }
    }
    std::cout << "\n";
  }

  std::cout << "\n  Leyenda: R=Robot  G=Goal  █=Obstáculo  ·=Vacío\n";
  std::cout << "  Robot: (" << robotPosition_.x << "," << robotPosition_.y
            << ")";
  std::cout << "  →  Goal: (" << goalPosition_.x << "," << goalPosition_.y
            << ")\n";
}

bool Environment::isPositionFree(const Point &pos) const {
  // *** SECCIÓN CRÍTICA ***
  // Lock para lectura segura del mapa
  std::lock_guard<std::mutex> lock(mapMutex_);

  // Verificar límites
  if (pos.x < 0 || pos.x >= width_ || pos.y < 0 || pos.y >= height_) {
    return false;
  }

  // Verificar si hay obstáculo
  Node *node = const_cast<Environment *>(this)->getNode(pos.x, pos.y);
  return node && node->type != CellType::OBSTACLE;
}

void Environment::updateRobotPosition(const Point &pos) {
  // *** MULTI-ROBOT FIX ***
  // Los robots YA NO escriben en el grid (CellType::ROBOT)
  // Solo validan que la posición sea válida.
  // El WebServer obtiene las posiciones reales desde RobotManager.
  
  // Validación básica de límites
  if (pos.x < 0 || pos.x >= width_ || pos.y < 0 || pos.y >= height_) {
    return; // Posición inválida
  }
  
  // NOTA: NO modificamos el grid aquí para evitar race conditions
  // con múltiples robots. Cada robot mantiene su propia posición
  // y el rendering se hace en base a los datos del RobotManager.
}

void Environment::setGoal(const Point &pos) {
  std::lock_guard<std::mutex> lock(mapMutex_);
  
  // Limpiar objetivo anterior
  Node *oldNode = getNode(goalPosition_.x, goalPosition_.y);
  if (oldNode && oldNode->type == CellType::GOAL) {
    oldNode->type = CellType::EMPTY;
  }

  goalPosition_ = pos;
  Node *newGoalNode = getNode(goalPosition_.x, goalPosition_.y);
  if (newGoalNode) {
    newGoalNode->type = CellType::GOAL;
  }
}

Point Environment::getGoal() const {
  std::lock_guard<std::mutex> lock(mapMutex_);
  return goalPosition_;
}

void Environment::start() {
  if (running_)
    return;
  running_ = true;
  updateThread_ = std::thread(&Environment::updateLoop, this);
}

void Environment::stop() {
  if (!running_)
    return;
  running_ = false;
  if (updateThread_.joinable()) {
    updateThread_.join();
  }
}

void Environment::updateLoop() {
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    if (!running_)
      break;

    // DESHABILITADO: Generación automática de obstáculos
    // El usuario quiere control manual completo sobre los obstáculos
    // Los obstáculos solo se deben agregar/eliminar mediante la interfaz web
    
    /* COMENTADO - Generación automática de obstáculos
    std::lock_guard<std::mutex> lock(mapMutex_);

    // Contar obstáculos actuales
    currentObstacleCount_ = countObstacles();

    // Randomly toggle some cells (excluding borders)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(1, width_ - 2);
    std::uniform_int_distribution<> distY(1, height_ - 2);

    for (int i = 0; i < 5; ++i) { // Change 5 cells per update
      int x = distX(gen);
      int y = distY(gen);

      Node *node = getNode(x, y);
      if (node) {
        // Don't modify robot or goal
        if (node->type != CellType::ROBOT && node->type != CellType::GOAL) {
          // Toggle entre EMPTY/OBSTACLE respetando límite
          if (node->type == CellType::EMPTY &&
              currentObstacleCount_ < MAX_OBSTACLES) {
            node->type = CellType::OBSTACLE;
            currentObstacleCount_++;
          } else if (node->type == CellType::OBSTACLE) {
            node->type = CellType::EMPTY;
            currentObstacleCount_--;
          }
        }
      }
    }
    */
  }
}

int Environment::countObstacles() const {
  int count = 0;
  for (const auto &node : graph_) {
    if (node.type == CellType::OBSTACLE) {
      count++;
    }
  }
  return count;
}

// ========== Implementación de métodos de edición interactiva ==========

bool Environment::toggleObstacle(const Point &pos) {
  std::lock_guard<std::mutex> lock(mapMutex_);
  
  // Validar posición
  if (pos.x <= 0 || pos.x >= width_ - 1 || pos.y <= 0 || pos.y >= height_ - 1) {
    // No permitir modificar bordes
    return false;
  }
  
  Node *node = getNode(pos.x, pos.y);
  if (!node) return false;
  
  // No modificar si es objetivo
  if (node->type == CellType::GOAL) {
    return false;
  }
  
  // Alternar
  if (node->type == CellType::OBSTACLE) {
    node->type = CellType::EMPTY;
    currentObstacleCount_--;
    return false; // Se eliminó
  } else {
    node->type = CellType::OBSTACLE;
    currentObstacleCount_++;
    return true; // Se agregó
  }
}

void Environment::clearAllObstacles() {
  std::lock_guard<std::mutex> lock(mapMutex_);
  
  // Limpiar todos los obstáculos excepto bordes
  for (int y = 1; y < height_ - 1; y++) {
    for (int x = 1; x < width_ - 1; x++) {
      Node *node = getNode(x, y);
      if (node && node->type == CellType::OBSTACLE) {
        node->type = CellType::EMPTY;
      }
    }
  }
  
  currentObstacleCount_ = countObstacles();
}

void Environment::generateRandomObstacles(int percentage) {
  std::lock_guard<std::mutex> lock(mapMutex_);
  
  // Primero limpiar obstáculos existentes (excepto bordes)
  for (int y = 1; y < height_ - 1; y++) {
    for (int x = 1; x < width_ - 1; x++) {
      Node *node = getNode(x, y);
      if (node && node->type == CellType::OBSTACLE) {
        node->type = CellType::EMPTY;
      }
    }
  }
  
  // Generar nuevos obstáculos aleatorios
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distX(1, width_ - 2);
  std::uniform_int_distribution<> distY(1, height_ - 2);
  std::uniform_int_distribution<> distChance(0, 100);
  
  // Calcular número de obstáculos basado en porcentaje
  int innerArea = (width_ - 2) * (height_ - 2);
  int targetObstacles = (innerArea * percentage) / 100;
  
  int placed = 0;
  while (placed < targetObstacles) {
    int x = distX(gen);
    int y = distY(gen);
    
    // No colocar en robot o objetivo
    if ((x == robotPosition_.x && y == robotPosition_.y) ||
        (x == goalPosition_.x && y == goalPosition_.y)) {
      continue;
    }
    
    Node *node = getNode(x, y);
    if (node && node->type == CellType::EMPTY) {
      node->type = CellType::OBSTACLE;
      placed++;
    }
  }
  
  currentObstacleCount_ = countObstacles();
}

} // namespace OSBot
