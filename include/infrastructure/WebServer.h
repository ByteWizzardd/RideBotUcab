#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace OSBot {

// Forward declarations
class Kernel;

/**
 * @class WebServer
 * @brief Servidor HTTP para interfaz web de visualización
 * 
 * Proporciona API REST para controlar y visualizar la simulación
 * desde un navegador web.
 */
class WebServer {
public:
  /**
   * @brief Constructor
   * @param kernel Referencia al kernel para acceder al estado
   * @param port Puerto del servidor (default: 8080)
   */
  WebServer(Kernel &kernel, int port = 8080);

  /**
   * @brief Destructor
   */
  ~WebServer();

  /**
   * @brief Inicia el servidor HTTP en un hilo separado
   */
  void start();

  /**
   * @brief Detiene el servidor HTTP
   */
  void stop();

  /**
   * @brief Verifica si el servidor está corriendo
   */
  bool isRunning() const { return running_; }

  /**
   * @brief Obtiene el puerto del servidor
   */
  int getPort() const { return port_; }

private:
  Kernel &kernel_;
  int port_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> serverThread_;

  /**
   * @brief Loop principal del servidor HTTP
   */
  void serverLoop();

  /**
   * @brief Genera JSON con el estado actual del sistema
   */
  std::string getStateJSON();

  /**
   * @brief Genera JSON con las estadísticas del sistema
   */
  std::string getStatsJSON();

  /**
   * @brief Sirve archivos estáticos desde el directorio web/
   */
  std::string serveStaticFile(const std::string &filename);
};

} // namespace OSBot

#endif // WEBSERVER_H
