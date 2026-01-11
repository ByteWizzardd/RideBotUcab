#ifndef RIDEBOT_STORAGE_H
#define RIDEBOT_STORAGE_H

#include "application/TaskScheduler.h"
#include "domain/Environment.h"
#include "domain/Robot.h"
#include <fstream>
#include <string>
#include <vector>

namespace OSBot {

/**
 * @brief Sistema de almacenamiento binario persistente
 * Formato: .osbot (binario personalizado con magic number)
 */
class Storage {
public:
  // Magic number para validar archivos
  static constexpr uint32_t MAGIC_NUMBER = 0x4F534254; // "OSBT" en ASCII
  static constexpr uint16_t VERSION = 1;

  /**
   * @brief Guarda el estado completo del sistema
   */
  static bool save_state(const std::string &filename,
                         const Environment &environment,
                         const std::vector<const Robot *> &robots,
                         const TaskScheduler &task_scheduler);

  /**
   * @brief Carga el estado del sistema
   */
  static bool load_state(const std::string &filename, Environment &environment,
                         std::vector<Robot *> &robots,
                         TaskScheduler &task_scheduler);

private:
  // Métodos auxiliares de escritura
  static void writeHeader(std::ofstream &ofs, uint16_t num_robots,
                          uint16_t num_tasks, uint16_t num_obstacles);
  static void writePoint(std::ofstream &ofs, const Point &p);
  static void writeEnvironment(std::ofstream &ofs, const Environment &env);
  static void writeRobots(std::ofstream &ofs,
                          const std::vector<const Robot *> &robots);
  static void writeTasks(std::ofstream &ofs, const TaskScheduler &scheduler);

  // Métodos auxiliares de lectura
  static bool readHeader(std::ifstream &ifs, uint16_t &num_robots,
                         uint16_t &num_tasks, uint16_t &num_obstacles);
  static Point readPoint(std::ifstream &ifs);
  static bool readEnvironment(std::ifstream &ifs, Environment &env);
  static bool readRobots(std::ifstream &ifs, std::vector<Robot *> &robots,
                         uint16_t count);
  static bool readTasks(std::ifstream &ifs, TaskScheduler &scheduler,
                        uint16_t count);
};

} // namespace OSBot

#endif // RIDEBOT_STORAGE_H
