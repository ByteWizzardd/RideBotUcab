#include "application/Kernel.h"
#include "domain/Robot.h"
#include <csignal>
#include <iostream>
#include <memory>
#include <atomic> // Added for std::atomic
#include <thread> // Added for std::thread
#include <chrono> // Added for std::chrono

// Global para manejo// Kernel global
std::unique_ptr<OSBot::Kernel> g_kernel;
std::atomic<bool> g_shuttingDown{false};

/**
 * @brief Manejador de señales para shutdown limpio
 */
void signalHandler(int signal) {
  if (g_shuttingDown.load()) {
    std::cout << "\n[Main] Ya está cerrando, por favor espera..." << std::endl;
    return;
  }
  
  g_shuttingDown.store(true);
  std::cout << "\n[Main] Señal de interrupción recibida (" << signal << ")" << std::endl;
  
  if (g_kernel) {
    g_kernel->shutdown();
  }
  
  exit(0);
}

bool getGoalCoordinates(int &x, int &y, int maxX, int maxY) {
  std::cout << "\n╔══════════════════════════════════════════════════╗\n";
  std::cout << "║      Ingrese las coordenadas del objetivo       ║\n";
  std::cout << "╚══════════════════════════════════════════════════╝\n\n";
  std::cout << "  Rango válido: X[1-" << maxX - 2 << "] Y[1-" << maxY - 2
            << "]\n\n";

  std::cout << "Coordenada X: ";
  if (!(std::cin >> x)) {
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    return false;
  }

  std::cout << "Coordenada Y: ";
  if (!(std::cin >> y)) {
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    return false;
  }

  // Validar rango
  if (x < 1 || x >= maxX - 1 || y < 1 || y >= maxY - 1) {
    std::cout << "\n❌ Coordenadas fuera de rango!\n";
    return false;
  }

  std::cout << "\n✅ Objetivo establecido en (" << x << "," << y << ")\n";
  return true;
}

int main() {
  // Configurar señales para shutdown limpio
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  std::cout << "╔══════════════════════════════════════════════════╗\n";
  std::cout << "║         OS-BOT - Simulación de Navegación       ║\n";
  std::cout << "║            Modo Interfaz Web Activo             ║\n";
  std::cout << "╚══════════════════════════════════════════════════╝\n";

  // Inicializar kernel
  g_kernel = std::make_unique<OSBot::Kernel>();

  if (!g_kernel->initialize()) {
    std::cerr << "Error al inicializar el kernel\n";
    return 1;
  }

  std::cout << "\n[Main] 🌐 Interfaz web disponible en: http://localhost:8080\n";
  std::cout << "[Main] 🤖 Robot creado en posición (5, 5)\n";
  std::cout << "[Main] 🎯 Objetivo inicial en (50, 30)\n";
  std::cout << "[Main] ℹ️  Puedes cambiar el objetivo desde la interfaz web\n";
  std::cout << "[Main] 🛑 Presiona Ctrl+C para detener el sistema\n\n";

  // Establecer objetivo inicial por defecto
  g_kernel->getEnvironment().setGoal(OSBot::Point(50, 30));

  // Crear un robot en posición inicial
  auto &robotManager = g_kernel->getRobotManager();
  int robotId = robotManager.addRobot(OSBot::Point(5, 5));

  // Iniciar sistema
  g_kernel->start();

  // Iniciar el kernel en un hilo separado
  std::thread kernelThread([&]() {
    g_kernel->run(0); // 0 = ejecutar indefinidamente
  });

  // Modo web: solo esperar señal de interrupción
  // El robot puede llegar al objetivo múltiples veces sin detener
  std::cout << "[Main] Sistema en ejecución...\n";
  
  // Loop simple - solo esperar Ctrl+C
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  kernelThread.join();
  return 0;
}
