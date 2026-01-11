#ifndef RIDEBOT_LIDARSENSOR_H
#define RIDEBOT_LIDARSENSOR_H

#include "domain/Global.h"
#include <vector>

namespace OSBot {

// Forward declaration
class Environment;

struct LidarData {
  std::vector<double> ranges; // Distancias en cada ángulo (360 elementos)
};

class LIDARSensor {
public:
  /**
   * @brief Constructor del sensor LIDAR
   * @param env Referencia al entorno para detectar obstáculos
   * @param max_range Rango máximo de detección
   */
  LIDARSensor(const Environment &env, double max_range = 500.0);

  /**
   * @brief Escanea 360° desde la posición dada
   * @param position Posición del robot en el grid
   * @return Datos LIDAR con 360 distancias
   */
  LidarData scan(const Point &position);

private:
  const Environment &environment_;
  double max_range_;

  /**
   * @brief Realiza raycast en una dirección específica
   * @param start Punto de inicio
   * @param angle Ángulo en grados (0-359)
   * @return Distancia al obstáculo más cercano
   */
  double raycast(const Point &start, double angle);
};

} // namespace OSBot

#endif // RIDEBOT_LIDARSENSOR_H
