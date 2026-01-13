#include "infrastructure/Storage.h"
#include "domain/Environment.h"
#include "domain/Robot.h"
#include "application/TaskScheduler.h"
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
    std::vector<Task> tasks = task_scheduler.getAllTasks();
    uint16_t num_robots = static_cast<uint16_t>(robots.size());
    uint16_t num_tasks = static_cast<uint16_t>(tasks.size());

    // Contar obstáculos
    uint16_t num_obstacles = 0;
    for (int x = 0; x < environment.getWidth(); ++x) {
        for (int y = 0; y < environment.getHeight(); ++y) {
            if (!environment.isPositionFree(Point(x, y))) {
                num_obstacles++;
            }
        }
    }

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

  // Escribir obstáculos
  for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
          if (!env.isPositionFree(Point(x, y))) {
              writePoint(ofs, Point(x, y));
          }
      }
  }
}

void Storage::writeRobots(std::ofstream &ofs,
                          const std::vector<const Robot *> &robots) {
  for (const auto *robot : robots) {
    if (!robot)
      continue;

    int32_t id = robot->getId();
    Point pos = robot->getPosition();
    uint8_t state = static_cast<uint8_t>(robot->getState());
    float battery = robot->getBatteryLevel();

    ofs.write(reinterpret_cast<const char *>(&id), sizeof(id));
    writePoint(ofs, pos);
    ofs.write(reinterpret_cast<const char *>(&state), sizeof(state));
    ofs.write(reinterpret_cast<const char *>(&battery), sizeof(battery));
  }
}

void Storage::writeTasks(std::ofstream &ofs, const TaskScheduler &scheduler) {
  std::vector<Task> tasks = scheduler.getAllTasks();
  for (const auto& task : tasks) {
      int32_t id = task.getId();
      // Usamos el primer waypoint como target principal para simplificar
      // o el actual si tiene múltiples. El formato dice Target X/Y.
      Point target = task.getCurrentWaypoint(); 
      uint8_t priority = static_cast<uint8_t>(task.getPriority());
      uint8_t status = static_cast<uint8_t>(task.getStatus());

      ofs.write(reinterpret_cast<const char *>(&id), sizeof(id));
      writePoint(ofs, target);
      ofs.write(reinterpret_cast<const char *>(&priority), sizeof(priority));
      ofs.write(reinterpret_cast<const char *>(&status), sizeof(status));
  }
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
    if (!readEnvironment(ifs, environment, num_obstacles))
      return false;
    if (!readRobots(ifs, robots, num_robots, environment))
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

bool Storage::readEnvironment(std::ifstream &ifs, Environment &env, uint16_t num_obstacles) {
  // Leer dimensiones
  int32_t width, height;
  ifs.read(reinterpret_cast<char *>(&width), sizeof(width));
  ifs.read(reinterpret_cast<char *>(&height), sizeof(height));

  if (width != env.getWidth() || height != env.getHeight()) {
      std::cerr << "[Storage] Warning: Dimensiones del mapa difieren. Se esperaban " 
                << env.getWidth() << "x" << env.getHeight() << " pero se leyó " 
                << width << "x" << height << std::endl;
      // Continuamos pero podría haber problemas si los obstáculos están fuera de rango
  }

  // Limpiar obstáculos actuales
  env.clearAllObstacles();
  
  // Leer obstáculos y marcarlos
  for (uint16_t i = 0; i < num_obstacles; ++i) {
      Point p = readPoint(ifs);
      // toggleObstacle agrega si no existe, o quita si existe.
      // Como limpiamos todo antes, toggle agregará.
      // Verificamos rango por seguridad
      if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height) {
          env.toggleObstacle(p);
      }
  }

  return true;
}

bool Storage::readRobots(std::ifstream &ifs, std::vector<Robot *> &robots,
                         uint16_t count, Environment &env) {
  // Nota: No limpiamos el vector 'robots' aquí porque asumimos que el caller (RobotManager)
  // ya debe haber gestionado la limpieza de los robots anteriores si fuera necesario.
  // Sin embargo, load_state recibe un vector de punteros. 
  // Para ser seguros, deberíamos asumir que el vector que nos pasan es para llenar.
  
  // IMPORTANTE: Storage NO posee los robots. RobotManager sí.
  // Si RobotManager nos pasó un vector vacío para llenar, bien.
  // Si nos pasó el vector actual, hay que limpiarlo?
  // Normalmente load_state debería reconstruir todo.
  
  // Vamos a instanciar nuevos robots.
  for (uint16_t i = 0; i < count; ++i) {
    int32_t id;
    uint8_t stateVal;
    float battery;

    ifs.read(reinterpret_cast<char *>(&id), sizeof(id));
    Point pos = readPoint(ifs);  // Leer DESPUÉS del ID
    ifs.read(reinterpret_cast<char *>(&stateVal), sizeof(stateVal));
    ifs.read(reinterpret_cast<char *>(&battery), sizeof(battery));

    // Crear nuevo robot usando el environment pasado
    Robot* robot = new Robot(env);
    robot->setId(id);
    robot->setPosition(pos);
    robot->setBatteryLevel(battery);
    
    // Si queremos restaurar el estado exacto, necesitaríamos un setter de estado
    // pero por defecto inicia en IDLE, lo cual es seguro.
    
    robots.push_back(robot);
  }
  return true;
}


bool Storage::readTasks(std::ifstream &ifs, TaskScheduler &scheduler,
                        uint16_t count) {
  scheduler.clear();
  for (uint16_t i = 0; i < count; ++i) {
      int32_t id;
      ifs.read(reinterpret_cast<char *>(&id), sizeof(id));

      Point target = readPoint(ifs);
      uint8_t priority;
      uint8_t status;
      ifs.read(reinterpret_cast<char *>(&priority), sizeof(priority));
      ifs.read(reinterpret_cast<char *>(&status), sizeof(status));

      std::vector<Point> waypoints = {target};
      Task task(id, waypoints, static_cast<TaskPriority>(priority));
      
      // La tarea se crea con estado PENDING por defecto
      // Si el estado guardado era diferente, lo seteamos
      task.setStatus(static_cast<TaskStatus>(status));
      
      scheduler.add_task(task);
  }
  return true;
}

} // namespace OSBot

