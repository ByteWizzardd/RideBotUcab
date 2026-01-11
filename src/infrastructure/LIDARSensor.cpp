#include "infrastructure/LIDARSensor.h"
#include "domain/Environment.h"
#include <cmath>

namespace OSBot {

LIDARSensor::LIDARSensor(const Environment &env, double max_range)
    : environment_(env), max_range_(max_range) {}

LidarData LIDARSensor::scan(const Point &position) {
  LidarData data;
  data.ranges.reserve(360);

  // Escanear 360 grados (1 grado de resolución)
  for (int angle = 0; angle < 360; ++angle) {
    double distance = raycast(position, static_cast<double>(angle));
    data.ranges.push_back(distance);
  }

  return data;
}

double LIDARSensor::raycast(const Point &start, double angle) {
  // Convertir ángulo a radianes
  double rad = angle * M_PI / 180.0;

  // Dirección del rayo
  double dx = std::cos(rad);
  double dy = std::sin(rad);

  // Incremento para el raycast (resolución)
  const double step = 0.5;

  // Avanzar el rayo hasta encontrar un obstáculo o alcanzar max_range
  for (double dist = 0; dist < max_range_; dist += step) {
    // Calcular posición actual del rayo
    int x = static_cast<int>(start.x + dx * dist);
    int y = static_cast<int>(start.y + dy * dist);

    Point test_pos(x, y);

    // Verificar si está fuera del mapa o es un obstáculo
    if (!environment_.isPositionFree(test_pos)) {
      return dist;
    }
  }

  // No se encontró obstáculo dentro del rango
  return max_range_;
}

} // namespace OSBot
