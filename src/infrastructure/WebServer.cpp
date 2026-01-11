#include "infrastructure/WebServer.h"
#include "application/Kernel.h"
#include "domain/Environment.h"
#include "domain/Global.h"
#include "infrastructure/httplib.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace OSBot {

WebServer::WebServer(Kernel &kernel, int port)
    : kernel_(kernel), port_(port), running_(false) {}

WebServer::~WebServer() { stop(); }

void WebServer::start() {
  if (running_)
    return;

  running_ = true;
  serverThread_ = std::make_unique<std::thread>(&WebServer::serverLoop, this);
  std::cout << "[WebServer] Iniciado en http://localhost:" << port_
            << std::endl;
}

void WebServer::stop() {
  if (!running_)
    return;

  running_ = false;
  if (serverThread_ && serverThread_->joinable()) {
    serverThread_->join();
  }
  std::cout << "[WebServer] Detenido" << std::endl;
}

void WebServer::serverLoop() {
  httplib::Server server;

  // Endpoint: Página principal
  server.Get("/", [this](const httplib::Request &, httplib::Response &res) {
    res.set_content(serveStaticFile("index.html"), "text/html");
  });

  // Endpoint: CSS
  server.Get("/styles.css",
             [this](const httplib::Request &, httplib::Response &res) {
               res.set_content(serveStaticFile("styles.css"), "text/css");
             });

  // Endpoint: JavaScript
  server.Get("/app.js",
             [this](const httplib::Request &, httplib::Response &res) {
               res.set_content(serveStaticFile("app.js"),
                               "application/javascript");
             });

  // API: Obtener estado completo
  server.Get("/api/state",
             [this](const httplib::Request &, httplib::Response &res) {
               res.set_content(getStateJSON(), "application/json");
               res.set_header("Access-Control-Allow-Origin", "*");
             });

  // API: Cambiar objetivo
  server.Post("/api/goal",
              [this](const httplib::Request &req, httplib::Response &res) {
                // Parse manual de JSON simple: {"x":10,"y":20}
                std::string body = req.body;
                size_t xPos = body.find("\"x\":");
                size_t yPos = body.find("\"y\":");

                if (xPos != std::string::npos && yPos != std::string::npos) {
                  int x = std::stoi(
                      body.substr(xPos + 4, body.find(",", xPos) - xPos - 4));
                  int y = std::stoi(
                      body.substr(yPos + 4, body.find("}", yPos) - yPos - 4));

                  // Limpiar todos los objetivos personales primero
                  kernel_.getRobotManager().clearAllPersonalGoals();
                  
                  kernel_.getEnvironment().setGoal(Point(x, y));
                  res.set_content("{\"success\":true}", "application/json");
                } else {
                  res.set_content("{\"success\":false,\"error\":\"Invalid JSON\"}",
                                  "application/json");
                }
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Pausar/reanudar
  server.Post("/api/pause",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                bool paused = body.find("true") != std::string::npos;
                kernel_.setPaused(paused);
                res.set_content("{\"success\":true}", "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Cambiar velocidad
  server.Post("/api/speed",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                size_t pos = body.find("\"speed\":");
                if (pos != std::string::npos) {
                  int speed = std::stoi(
                      body.substr(pos + 8, body.find("}", pos) - pos - 8));
                  kernel_.setSimulationSpeed(speed);
                  res.set_content("{\"success\":true}", "application/json");
                } else {
                  res.set_content("{\"success\":false}", "application/json");
                }
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Toggle obstáculo (agregar/eliminar)
  server.Post("/api/obstacle",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                size_t xPos = body.find("\"x\":");
                size_t yPos = body.find("\"y\":");

                if (xPos != std::string::npos && yPos != std::string::npos) {
                  int x = std::stoi(
                      body.substr(xPos + 4, body.find(",", xPos) - xPos - 4));
                  int y = std::stoi(
                      body.substr(yPos + 4, body.find("}", yPos) - yPos - 4));

                  bool added = kernel_.getEnvironment().toggleObstacle(Point(x, y));
                  std::string result = added ? "\"added\"" : "\"removed\"";
                  res.set_content("{\"success\":true,\"action\":" + result + "}", 
                                  "application/json");
                } else {
                  res.set_content("{\"success\":false}", "application/json");
                }
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Limpiar todos los obstáculos
  server.Post("/api/clear-obstacles",
              [this](const httplib::Request &, httplib::Response &res) {
                kernel_.getEnvironment().clearAllObstacles();
                res.set_content("{\"success\":true}", "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Generar obstáculos aleatorios
  server.Post("/api/random-obstacles",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                int percentage = 25; // default
                
                size_t pos = body.find("\"percentage\":");
                if (pos != std::string::npos) {
                  percentage = std::stoi(
                      body.substr(pos + 13, body.find("}", pos) - pos - 13));
                }
                
                kernel_.getEnvironment().generateRandomObstacles(percentage);
                res.set_content("{\"success\":true}", "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Reiniciar - reposicionar robot aleatoriamente
  server.Post("/api/reset",
              [this](const httplib::Request &, httplib::Response &res) {
                kernel_.getRobotManager().resetRobotPosition();
                res.set_content("{\"success\":true}", "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Agregar nuevo robot
  server.Post("/api/robot",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                int x = -1, y = -1;
                
                size_t xPos = body.find("\"x\":");
                size_t yPos = body.find("\"y\":");

                if (xPos != std::string::npos && yPos != std::string::npos) {
                  x = std::stoi(body.substr(xPos + 4, body.find(",", xPos) - xPos - 4));
                  y = std::stoi(body.substr(yPos + 4, body.find("}", yPos) - yPos - 4));
                }

                // Si no se especifican coordenadas o son inválidas, usar posición aleatoria
                if (x < 0 || y < 0) {
                   // Usar lógica simple para posición aleatoria segura
                   auto& env = kernel_.getEnvironment();
                   int width = env.getWidth();
                   int height = env.getHeight();
                   
                   // Intentar encontrar una posición libre (máx 50 intentos)
                   bool found = false;
                   for(int i=0; i<50; ++i) {
                       int tx = 5 + (rand() % (width - 10));
                       int ty = 5 + (rand() % (height - 10));
                       if(env.isPositionFree(Point(tx, ty))) {
                           x = tx;
                           y = ty;
                           found = true;
                           break;
                       }
                   }
                   // Fallback si no encuentra (aunque improbable con 50 intentos en mapa vacío)
                   if(!found) { x = 5; y = 5; }
                }

                int id = kernel_.getRobotManager().addRobot(Point(x, y));
                
                // Si el robot se creó exitosamente, inciarlo si el sistema no está pausado
                if (id > 0) {
                    auto info = kernel_.getRobotManager().getRobotInfo(id);
                    if (info && info->robot) {
                         info->robot->start();
                    }
                }

                std::string response = "{\"success\":true,\"id\":" + std::to_string(id) + "}";
                res.set_content(response, "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Eliminar robot
  server.Post("/api/robot/delete",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                int id = -1;
                
                size_t idPos = body.find("\"id\":");
                if (idPos != std::string::npos) {
                   id = std::stoi(body.substr(idPos + 5, body.find("}", idPos) - idPos - 5));
                }

                bool success = false;
                if (id > 0) {
                    success = kernel_.getRobotManager().removeRobot(id);
                } else {
                    // Si no se pasa ID, borrar el último (lógica simplificada para botón "Eliminar")
                    auto ids = kernel_.getRobotManager().getRobotIds();
                    if (!ids.empty()) {
                        // Borrar el último (mayor ID usualmente si es secuencial)
                        int lastId = ids.back();
                        // No borrar el robot 1 para evitar quedar sin robots (opcional)
                        if (lastId > 1) {
                             success = kernel_.getRobotManager().removeRobot(lastId);
                        }
                    }
                }

                std::string response = "{\"success\":" + std::string(success ? "true" : "false") + "}";
                res.set_content(response, "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Establecer objetivo específico para un robot
  server.Post("/api/robot/goal",
              [this](const httplib::Request &req, httplib::Response &res) {
                std::string body = req.body;
                int id = -1, x = -1, y = -1;
                
                size_t idPos = body.find("\"id\":");
                size_t xPos = body.find("\"x\":");
                size_t yPos = body.find("\"y\":");

                if (idPos != std::string::npos && xPos != std::string::npos && yPos != std::string::npos) {
                  id = std::stoi(body.substr(idPos + 5, body.find(",", idPos) - idPos - 5));
                  x = std::stoi(body.substr(xPos + 4, body.find(",", xPos) - xPos - 4));
                  y = std::stoi(body.substr(yPos + 4, body.find("}", yPos) - yPos - 4));
                }

                bool success = false;
                if (id > 0 && x >= 0 && y >= 0) {
                     success = kernel_.getRobotManager().setRobotGoal(id, Point(x, y));
                }

                std::string response = "{\"success\":" + std::string(success ? "true" : "false") + "}";
                res.set_content(response, "application/json");
                res.set_header("Access-Control-Allow-Origin", "*");
              });

  // API: Obtener estadísticas del sistema
  server.Get("/api/stats",
             [this](const httplib::Request &, httplib::Response &res) {
               res.set_content(getStatsJSON(), "application/json");
               res.set_header("Access-Control-Allow-Origin", "*");
             });

  std::cout << "[WebServer] Escuchando en puerto " << port_ << "..."
            << std::endl;
  server.listen("0.0.0.0", port_);
}

std::string WebServer::getStateJSON() {
  auto &env = kernel_.getEnvironment();
  auto &robotMgr = kernel_.getRobotManager();

  int width = env.getWidth();
  int height = env.getHeight();
  Point goal = env.getGoal();

  std::ostringstream json;
  json << "{";
  json << "\"grid\":{";
  json << "\"width\":" << width << ",";
  json << "\"height\":" << height << ",";
  json << "\"cells\":[";

  // Generar grid simplificado
  for (int y = 0; y < height; y++) {
    json << "[";
    for (int x = 0; x < width; x++) {
      Point pos(x, y);
      int cellType = 0; // EMPTY

      if (!env.isPositionFree(pos)) {
        cellType = 1; // OBSTACLE
      }

      json << cellType;
      if (x < width - 1)
        json << ",";
    }
    json << "]";
    if (y < height - 1)
      json << ",";
  }

  json << "]},";

  // Robots
  json << "\"robots\":[";
  auto robots = robotMgr.getAllRobots();
  for (size_t i = 0; i < robots.size(); i++) {
    if (robots[i] && robots[i]->robot) {
      auto info = robots[i]; // Use a more convenient name
      json << "{";
      json << "\"id\":" << info->id << ",";
      json << "\"x\":" << info->robot->getPosition().x << ",";
      json << "\"y\":" << info->robot->getPosition().y << ",";
      json << "\"state\":\"" << static_cast<int>(info->currentState) << "\",";
      json << "\"obstaclesAvoided\":" << info->obstaclesAvoided << ",";
      json << "\"active\":" << (info->isActive ? "true" : "false") << ",";
      // Incluir info del objetivo
      json << "\"goalX\":" << info->currentGoal.x << ",";
      json << "\"goalY\":" << info->currentGoal.y << ",";
      json << "\"hasPersonalGoal\":" << (info->hasPersonalGoal ? "true" : "false");
      json << "}";
      if (i < robots.size() - 1)
        json << ",";
    }
  }
  json << "],";

  // Goal
  json << "\"goal\":{\"x\":" << goal.x << ",\"y\":" << goal.y << "},";

  // Estado de pausa y velocidad
  json << "\"paused\":" << (kernel_.isPaused() ? "true" : "false") << ",";
  json << "\"speed\":" << kernel_.getSimulationSpeed();

  json << "}";
  return json.str();
}

std::string WebServer::getStatsJSON() {
  auto &robotMgr = kernel_.getRobotManager();
  
  // Obtener todos los robots
  auto robots = robotMgr.getAllRobots();
  
  // Calcular métricas
  int totalRobots = robots.size();
  int activeRobots = 0;
  int idleRobots = 0;
  double totalDistance = 0.0;
  int completedTasks = 0;
  int failedTasks = 0;
  int totalCellsTraveled = 0;
  
  for (const auto* robotInfo : robots) {
    if (robotInfo && robotInfo->robot) {
      // Contar robots por estado
      State state = robotInfo->robot->getState();
      if (state == State::NAVIGATING) {
        activeRobots++;
      } else if (state == State::IDLE || state == State::REACHED_GOAL) {
        idleRobots++;
      }
      
      // Acumular estadísticas
      totalDistance += robotInfo->totalDistanceTraveled;
      completedTasks += robotInfo->tasksCompleted;
      failedTasks += robotInfo->tasksFailed;
      totalCellsTraveled += robotInfo->cellsTraveled;
    }
  }
  
  // Calcular eficiencia
  int totalTasks = completedTasks + failedTasks;
  double efficiency = (totalTasks > 0) ? 
    (static_cast<double>(completedTasks) / totalTasks * 100.0) : 0.0;
  
  // Uptime (simulado - podría ser tiempo desde el inicio)
  static auto startTime = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
  
  // Construir JSON
  std::ostringstream json;
  json << "{";
  json << "\"totalTasks\":" << totalTasks << ",";
  json << "\"completedTasks\":" << completedTasks << ",";
  json << "\"failedTasks\":" << failedTasks << ",";
  json << "\"cellsTraveled\":" << totalCellsTraveled << ",";
  json << "\"totalDistance\":" << std::fixed << std::setprecision(2) << totalDistance << ",";
  json << "\"robotsActive\":" << activeRobots << ",";
  json << "\"robotsIdle\":" << idleRobots << ",";
  json << "\"totalRobots\":" << totalRobots << ",";
  json << "\"efficiency\":" << std::fixed << std::setprecision(1) << efficiency << ",";
  json << "\"uptime\":" << uptime;
  json << "}";
  
  return json.str();
}

std::string WebServer::serveStaticFile(const std::string &filename) {
  std::string path = "web/" + filename;
  std::ifstream file(path);

  if (!file.is_open()) {
    return "<!DOCTYPE html><html><body><h1>404 - File Not Found</h1><p>File: "
           + filename + "</p></body></html>";
  }

  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

} // namespace OSBot
