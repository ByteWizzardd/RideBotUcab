#include "infrastructure/GPSSensor.h"
#include <cmath>
#include <random>

namespace OSBot {

GPSSensor::GPSSensor(double base_lat, double base_lon, double scale_factor)
    : base_latitude_(base_lat), base_longitude_(base_lon),
      scale_factor_(scale_factor), add_noise_(true) {}

GPSData GPSSensor::get_data(const Point &position) {
  GPSData data;

  // Conversión de coordenadas del grid a GPS
  // El grid crece hacia abajo y a la derecha
  // GPS: latitud crece hacia el norte, longitud crece hacia el este
  data.latitude =
      base_latitude_ - (position.y * scale_factor_); // Y crece hacia abajo
  data.longitude = base_longitude_ +
                   (position.x * scale_factor_); // X crece hacia la derecha

  // Simular ruido gaussiano (precisión típica de GPS: 5-10 metros)
  if (add_noise_) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> noise(0.0, 0.00005); // ~5 metros de desviación

    data.latitude += noise(gen);
    data.longitude += noise(gen);
    data.accuracy =
        std::abs(noise(gen)) * 111000.0; // Convertir a metros aproximados
  } else {
    data.accuracy = 0.0; // GPS perfecto
  }

  return data;
}

} // namespace OSBot
