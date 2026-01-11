#ifndef RIDEBOT_GPSSENSOR_H
#define RIDEBOT_GPSSENSOR_H

#include "domain/Global.h"

namespace OSBot {

struct GPSData {
  double latitude;
  double longitude;
  double accuracy; // Precisión estimada en metros
};

class GPSSensor {
public:
  /**
   * @brief Constructor del sensor GPS
   * @param base_lat Latitud base (coordenada de referencia)
   * @param base_lon Longitud base (coordenada de referencia)
   * @param scale_factor Factor de escala para conversión grid→GPS
   */
  GPSSensor(double base_lat = 10.4806, double base_lon = -66.9036,
            double scale_factor = 0.001);

  /**
   * @brief Obtiene datos de posición GPS basados en la posición del robot
   * @param position Posición actual del robot en el grid
   * @return Datos GPS (latitud, longitud, precisión)
   */
  GPSData get_data(const Point &position);

private:
  double base_latitude_;
  double base_longitude_;
  double scale_factor_;
  bool add_noise_;
};

} // namespace OSBot

#endif // RIDEBOT_GPSSENSOR_H
