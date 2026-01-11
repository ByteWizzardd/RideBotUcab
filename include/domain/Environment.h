#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "Global.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace OSBot {

/**
 * @class Environment
 * @brief Representa el entorno del robot: cuadrícula, obstáculos y
 * visualización
 *
 * Esta clase gestiona el mapa 2D donde opera el robot.
 * Es thread-safe mediante el uso de mutex para acceso concurrente.
 */
class Environment {
public:
  /**
   * @brief Constructor - Inicializa el entorno con dimensiones especificadas
   */
  Environment(int width, int height);

  /**
   * @brief Destructor
   */
  ~Environment();

  struct Node {
    int id;
    int x;
    int y;
    CellType type;
    Node *north = nullptr;
    Node *south = nullptr;
    Node *east = nullptr;
    Node *west = nullptr;
  };

  /**
   * @brief Inicializa el entorno con obstáculos aleatorios
   */
  void initialize();

  /**
   * @brief Renderiza el entorno en la consola (ASCII art)
   * NOTA: Este método adquiere el lock del mapa (SECCIÓN CRÍTICA)
   */
  void render();

  /**
   * @brief Verifica si una posición está libre de obstáculos
   * @param pos Posición a verificar
   * @return true si está libre, false si hay obstáculo o fuera de límites
   * NOTA: Thread-safe - usa lock interno
   */
  bool isPositionFree(const Point &pos) const;

  /**
   * @brief Actualiza la posición del robot en el mapa
   * @param pos Nueva posición del robot
   * NOTA: SECCIÓN CRÍTICA - Modifica el estado compartido del mapa
   */
  void updateRobotPosition(const Point &pos);

  /**
   * @brief Establece la posición objetivo
   * @param pos Posición objetivo
   */
  void setGoal(const Point &pos);

  /**
   * @brief Obtiene la posición objetivo
   * @return Posición objetivo actual
   */
  Point getGoal() const;

  /**
   * @brief Limpia la consola (multiplataforma)
   */
  void clearScreen();

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  /**
   * @brief Inicia el hilo de actualización del entorno
   */
  void start();

  /**
   * @brief Detiene el hilo de actualización del entorno
   */
  void stop();

  // ========== Métodos de edición interactiva ==========
  
  /**
   * @brief Alterna un obstáculo en una posición (agregar/eliminar)
   * @param pos Posición del obstáculo
   * @return true si se agregó, false si se eliminó
   */
  bool toggleObstacle(const Point &pos);
  
  /**
   * @brief Limpia todos los obstáculos (excepto bordes)
   */
  void clearAllObstacles();
  
  /**
   * @brief Genera obstáculos aleatorios
   * @param percentage Porcentaje de celdas con obstáculos (0-100)
   */
  void generateRandomObstacles(int percentage = 25);

private:
  int width_;
  int height_;
  std::vector<Node> graph_; // Grafo de nodos
  Point robotPosition_;
  Point goalPosition_;

  // *** SINCRONIZACIÓN ***
  // Mutex para proteger el acceso concurrente al mapa compartido
  mutable std::mutex mapMutex_;

  // Threading
  std::thread updateThread_;
  std::atomic<bool> running_;

  // Obstacle management
  static constexpr int MAX_OBSTACLES = 50; // Límite máximo de obstáculos
  int currentObstacleCount_;

  /**
   * @brief Coloca obstáculos aleatorios en el mapa
   */
  void placeObstacles();

  /**
   * @brief Obtiene el nodo en una posición específica
   */
  Node *getNode(int x, int y);

  /**
   * @brief Bucle de actualización periódica del entorno
   */
  void updateLoop();

  /**
   * @brief Cuenta el número actual de obstáculos en el mapa
   */
  int countObstacles() const;
};

} // namespace OSBot

#endif // ENVIRONMENT_H
