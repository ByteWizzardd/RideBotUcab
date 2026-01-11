#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdint>

/**
 * @file Global.h
 * @brief Estructuras y tipos compartidos globalmente en el sistema OS-Bot
 */

namespace OSBot {

/**
 * @brief Estructura para representar coordenadas en el grid 2D
 */
struct Point {
    int x;
    int y;
    
    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Estados posibles del robot y del sistema
 */
enum class State {
    IDLE,           // Inactivo
    NAVIGATING,     // Navegando hacia objetivo
    REACHED_GOAL,   // Objetivo alcanzado
    BLOCKED,        // Bloqueado por obstáculo
    ERROR,          // Estado de error
    SHUTDOWN        // Apagando sistema
};

/**
 * @brief Tipos de celdas en el entorno
 */
enum class CellType {
    EMPTY,          // Celda vacía
    OBSTACLE,       // Obstáculo
    ROBOT,          // Posición del robot
    GOAL            // Objetivo
};

/**
 * @brief Constantes del sistema
 * NOTA: Puedes cambiar estos valores para ajustar el tamaño del grid
 */
namespace Constants {
    // Tamaño del grid (puedes cambiar estos valores)
    constexpr int GRID_WIDTH = 60;   // Ancho del grid (celdas)
    constexpr int GRID_HEIGHT = 60;  // Alto del grid (celdas)
    constexpr int SIMULATION_SPEED_MS = 100;  // Velocidad de actualización en ms
}

} // namespace OSBot

#endif // GLOBAL_H

