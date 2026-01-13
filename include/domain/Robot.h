#ifndef ROBOT_H
#define ROBOT_H

#include "Environment.h"
#include "Global.h"
#include <atomic>
#include <deque>
#include <memory>
#include <thread>
#include <vector>

namespace OSBot {

/**
 * @class Robot
 * @brief Representa el robot autónomo que navega en el entorno
 *
 * El robot ejecuta su lógica en un hilo separado y usa sensores
 * simulados para leer el entorno y navegar hacia el objetivo.
 */
class Robot {
public:
  /**
   * @brief Constructor
   * @param env Referencia al entorno donde opera el robot
   */
  explicit Robot(Environment &env);

  /**
   * @brief Destructor - Asegura que el hilo se detenga correctamente
   */
  ~Robot();

  /**
   * @brief Inicia el hilo de ejecución del robot
   */
  void start();

  /**
   * @brief Detiene el hilo de ejecución del robot
   */
  void stop();

  /**
   * @brief Establece la posición del robot (usado para inicialización)
   */
  void setPosition(const Point &pos);

  /**
   * @brief Obtiene el estado actual del robot
   */
  State getState() const;

  /**
   * @brief Obtiene la posición actual del robot
   */
  Point getPosition() const;
  
  /**
   * @brief Retorna el número de celdas que el robot ha recorrido
   */
  size_t getCellsTraveled() const { return cellsTraveled_; }
  
  /**
   * @brief Retorna el número de obstáculos esquivados (recalculaciones de ruta)
   */
  int getObstaclesAvoided() const { return obstaclesAvoided_; }

  /**
   * @brief Obtiene el ID del robot
   */
  int getId() const;

  /**
   * @brief Establece el ID del robot
   */
  void setId(int id);

  /**
   * @brief Obtiene el nivel de batería
   */
  float getBatteryLevel() const;

  /**
   * @brief Establece el nivel de batería
   */
  void setBatteryLevel(float level);

  /**
   * @brief Establece un objetivo personal para este robot
   * @param p Coordenada del objetivo
   */
  void setPersonalGoal(OSBot::Point p) {
      personalGoal_ = p;
      hasPersonalGoal_ = true;
      // Forzar reevaluación inmediata
      if (currentState_ == State::REACHED_GOAL || currentState_ == State::IDLE) {
          currentState_ = State::NAVIGATING;
      }
  }

  /**
   * @brief Limpia el objetivo personal (vuelve a usar el global)
   */
  void clearPersonalGoal() {
      hasPersonalGoal_ = false;
  }
  
  /**
   * @brief Obtiene el objetivo actual (personal o global)
   */
  OSBot::Point getGoal() const {
      if (hasPersonalGoal_) {
          return personalGoal_;
      }
      return environment_.getGoal();
  }
  
  /**
   * @brief Verifica si el robot tiene un objetivo personal asignado
   */
  bool hasPersonalGoal() const { return hasPersonalGoal_; }

private:
  Environment &environment_;
  Point currentPosition_;
  State currentState_;

  // Objetivo personal
  Point personalGoal_;
  bool hasPersonalGoal_;

  // Control de hilos
  std::unique_ptr<std::thread> robotThread_;
  std::atomic<bool> running_;

  // Datos para serialización
  int id_;
  float batteryLevel_;

  /**
   * @brief Función principal del hilo del robot
   * Contiene el bucle principal de navegación
   */
  void run();

  /**
   * @brief Lógica de navegación simple hacia el objetivo
   * Usa algoritmo greedy (moverse hacia la coordenada más cercana)
   */
  void navigate();

  /**
   * @brief Simula la lectura de sensores del entorno
   * @param targetPos Posición a verificar
   * @return true si la posición es navegable
   */
  bool sensePosition(const Point &targetPos);

  /**
   * @brief Calcula la distancia Manhattan entre dos puntos
   */
  int manhattanDistance(const Point &a, const Point &b) const;

  /**
   * @brief Intenta moverse a una nueva posición
   * @param newPos Nueva posición deseada
   * @return true si el movimiento fue exitoso
   */
  bool moveTo(const Point &newPos);

  // ========== NUEVOS: Detección de Stuck y Pathfinding ==========

  // Historial de posiciones y ruta planificada
  std::deque<Point> positionHistory_; // Últimas N posiciones
  std::vector<Point> plannedPath_;    // Ruta calculada por A*
  size_t pathIndex_;                  // Índice actual en plannedPath_
  int obstaclesAvoided_;              // Contador de obstáculos esquivados
  size_t cellsTraveled_;              // Contador total de celdas recorridas

  static constexpr size_t MAX_HISTORY = 10;
  static constexpr size_t STUCK_THRESHOLD = 3; // Repetir 3 veces = stuck

  /**
   * @brief Detecta si el robot está oscilando entre posiciones
   * @return true si stuck (posición se repite >= STUCK_THRESHOLD veces)
   */
  bool isStuck() const;

  /**
   * @brief Agrega posición al historial (mantiene tamaño MAX_HISTORY)
   */
  void addToHistory(const Point &pos);

  /**
   * @brief Recalcula ruta completa usando A*
   */
  void recalculatePath();

  /**
   * @brief Sigue la ruta planificada paso a paso
   * @return true si se movió exitosamente
   */
  bool followPlannedPath();

  /**
   * @brief Navegación greedy original (fallback)
   */
  void navigateGreedy();
};

} // namespace OSBot

#endif // ROBOT_H
