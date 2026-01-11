#include "infrastructure/Storage.h"
#include "domain/Environment.h"
#include <cstring>
#include <ctime>
#include <iostream>

namespace OSBot {

// ============================================================================
// ESCRITURA (SAVE)
// ============================================================================

bool Storage::save_state(const std::string &filename,
                         const Environment &environment,
                         const std::vector<const Robot *> &robots,
                         const TaskScheduler &task_scheduler) {

  std::ofstream ofs(filename, std::ios::binary);
  if (!ofs.is_open()) {
    std::cerr << "[Storage] Error: No se pudo crear el archivo: " << filename
              << std::endl;
    return false;
  }

  try {
    // Obtener contadores
    uint16_t num_robots = static_cast<uint16_t>(robots.size());
    uint16_t num_tasks = 0;     // TODO: Obtener de task_scheduler
    uint16_t num_obstacles = 0; // TODO: Calcular del environment

    // Escribir secciones
    writeHeader(ofs, num_robots, num_tasks, num_obstacles);
    writeEnvironment(ofs, environment);
    writeRobots(ofs, robots);
    writeTasks(ofs, task_scheduler);

    std::cout << "[Storage] Estado guardado exitosamente en: " << filename
              << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "[Storage] Error al guardar: " << e.what() << std::endl;
    return false;
  }
}

void Storage::writeHeader(std::ofstream &ofs, uint16_t num_robots,
                          uint16_t num_tasks, uint16_t num_obstacles) {
  // Magic number
  ofs.write(reinterpret_cast<const char *>(&MAGIC_NUMBER),
            sizeof(MAGIC_NUMBER));

  // Versión
  ofs.write(reinterpret_cast<const char *>(&VERSION), sizeof(VERSION));

  // Timestamp (Unix time)
  uint64_t timestamp = static_cast<uint64_t>(std::time(nullptr));
  ofs.write(reinterpret_cast<const char *>(&timestamp), sizeof(timestamp));

  // Contadores
  ofs.write(reinterpret_cast<const char *>(&num_robots), sizeof(num_robots));
  ofs.write(reinterpret_cast<const char *>(&num_tasks), sizeof(num_tasks));
  ofs.write(reinterpret_cast<const char *>(&num_obstacles),
            sizeof(num_obstacles));
}

void Storage::writePoint(std::ofstream &ofs, const Point &p) {
  ofs.write(reinterpret_cast<const char *>(&p.x), sizeof(p.x));
  ofs.write(reinterpret_cast<const char *>(&p.y), sizeof(p.y));
}

void Storage::writeEnvironment(std::ofstream &ofs, const Environment &env) {
  // Dimensiones
  int32_t width = env.getWidth();
  int32_t height = env.getHeight();
  ofs.write(reinterpret_cast<const char *>(&width), sizeof(width));
  ofs.write(reinterpret_cast<const char *>(&height), sizeof(height));

  // TODO: Escribir obstáculos si Environment expone la lista
}

void Storage::writeRobots(std::ofstream &ofs,
                          const std::vector<const Robot *> &robots) {
  for (const auto *robot : robots) {
    if (!robot)
      continue;

    // TODO: Escribir datos del robot según RobotInfo
    // Por ahora placeholder
    int32_t id = 0;
    ofs.write(reinterpret_cast<const char *>(&id), sizeof(id));
  }
}

void Storage::writeTasks(std::ofstream &ofs, const TaskScheduler &scheduler) {
  // TODO: Serializar tareas del scheduler
  // Placeholder por ahora
}

// ============================================================================
// LECTURA (LOAD)
// ============================================================================

bool Storage::load_state(const std::string &filename, Environment &environment,
                         std::vector<Robot *> &robots,
                         TaskScheduler &task_scheduler) {

  std::ifstream ifs(filename, std::ios::binary);
  if (!ifs.is_open()) {
    std::cerr << "[Storage] Error: No se pudo abrir el archivo: " << filename
              << std::endl;
    return false;
  }

  try {
    uint16_t num_robots, num_tasks, num_obstacles;

    // Leer y validar header
    if (!readHeader(ifs, num_robots, num_tasks, num_obstacles)) {
      std::cerr << "[Storage] Error: Archivo corrupto o incompatible"
                << std::endl;
      return false;
    }

    // Leer secciones
    if (!readEnvironment(ifs, environment))
      return false;
    if (!readRobots(ifs, robots, num_robots))
      return false;
    if (!readTasks(ifs, task_scheduler, num_tasks))
      return false;

    std::cout << "[Storage] Estado cargado exitosamente desde: " << filename
              << std::endl;
    std::cout << "[Storage]   - Robots: " << num_robots << std::endl;
    std::cout << "[Storage]   - Tareas: " << num_tasks << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "[Storage] Error al cargar: " << e.what() << std::endl;
    return false;
  }
}

bool Storage::readHeader(std::ifstream &ifs, uint16_t &num_robots,
                         uint16_t &num_tasks, uint16_t &num_obstacles) {
  // Leer magic number
  uint32_t magic;
  ifs.read(reinterpret_cast<char *>(&magic), sizeof(magic));
  if (magic != MAGIC_NUMBER) {
    std::cerr << "[Storage] Magic number inválido: 0x" << std::hex << magic
              << std::endl;
    return false;
  }

  // Leer versión
  uint16_t version;
  ifs.read(reinterpret_cast<char *>(&version), sizeof(version));
  if (version != VERSION) {
    std::cerr << "[Storage] Versión incompatible: " << version << std::endl;
    return false;
  }

  // Leer timestamp
  uint64_t timestamp;
  ifs.read(reinterpret_cast<char *>(&timestamp), sizeof(timestamp));

  // Leer contadores
  ifs.read(reinterpret_cast<char *>(&num_robots), sizeof(num_robots));
  ifs.read(reinterpret_cast<char *>(&num_tasks), sizeof(num_tasks));
  ifs.read(reinterpret_cast<char *>(&num_obstacles), sizeof(num_obstacles));

  return true;
}

Point Storage::readPoint(std::ifstream &ifs) {
  Point p;
  ifs.read(reinterpret_cast<char *>(&p.x), sizeof(p.x));
  ifs.read(reinterpret_cast<char *>(&p.y), sizeof(p.y));
  return p;
}

bool Storage::readEnvironment(std::ifstream &ifs, Environment &env) {
  // Leer dimensiones
  int32_t width, height;
  ifs.read(reinterpret_cast<char *>(&width), sizeof(width));
  ifs.read(reinterpret_cast<char *>(&height), sizeof(height));

  // TODO: Validar que coincidan con el environment actual
  // TODO: Leer obstáculos

  return true;
}

bool Storage::readRobots(std::ifstream &ifs, std::vector<Robot *> &robots,
                         uint16_t count) {
  // TODO: Recrear robots desde datos binarios
  for (uint16_t i = 0; i < count; ++i) {
    int32_t id;
    ifs.read(reinterpret_cast<char *>(&id), sizeof(id));
  }
  return true;
}

bool Storage::readTasks(std::ifstream &ifs, TaskScheduler &scheduler,
                        uint16_t count) {
  // TODO: Recrear tareas desde datos binarios
  return true;
}

} // namespace OSBot
