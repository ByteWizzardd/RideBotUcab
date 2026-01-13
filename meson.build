# ============================================
# OS-Bot v2.0 - Sistema Operativo Multi-Robot
# Build System: Meson
# ============================================

project('os-bot', 'cpp',
  version: '2.0.0',
  default_options: [
    'cpp_std=c++17',           # Estándar C++17
    'warning_level=3',          # Nivel de advertencias alto
    'optimization=2'            # Optimización moderada
  ]
)

# ============================================
# Dependencias
# ============================================

# Dependencia obligatoria de hilos para concurrencia
threads_dep = dependency('threads')

# ============================================
# Archivos fuente
# ============================================

sources = [
  'src/main.cpp',
  'src/domain/Environment.cpp',
  'src/domain/Robot.cpp',
  'src/domain/Task.cpp',
  'src/application/Kernel.cpp',
  'src/application/RobotManager.cpp',
  'src/application/TaskManager.cpp',
  'src/application/ThreadManager.cpp',
  'src/application/TaskScheduler.cpp',
  'src/application/NavigationModule.cpp',
  'src/application/AStar.cpp',
  'src/infrastructure/GPSSensor.cpp',
  'src/infrastructure/LIDARSensor.cpp',
  'src/infrastructure/Storage.cpp',
  'src/infrastructure/WebServer.cpp'
]

# ============================================
# Configuración de includes
# ============================================

inc_dirs = include_directories('include')

# ============================================
# Definición del ejecutable
# ============================================

executable('os-bot',
  sources,
  include_directories: inc_dirs,
  dependencies: [threads_dep],
  install: true
)

# ============================================
# Test Executable
# ============================================

test_sources = [
  'tests/test_storage.cpp',
  'src/domain/Environment.cpp',
  'src/domain/Robot.cpp',
  'src/domain/Task.cpp',
  'src/application/Kernel.cpp',
  'src/application/RobotManager.cpp',
  'src/application/TaskManager.cpp',
  'src/application/ThreadManager.cpp',
  'src/application/TaskScheduler.cpp',
  'src/application/NavigationModule.cpp',
  'src/application/AStar.cpp',
  'src/infrastructure/GPSSensor.cpp',
  'src/infrastructure/LIDARSensor.cpp',
  'src/infrastructure/Storage.cpp',
  'src/infrastructure/WebServer.cpp'
]

executable('os-bot-test',
  test_sources,
  include_directories: inc_dirs,
  dependencies: [threads_dep],
  install: false
)

# ============================================
# Mensaje informativo
# ============================================

message('Configurando OS-Bot v' + meson.project_version())
message('Estándar C++: ' + get_option('cpp_std'))
message('Características:')
message('  - Sistema Multi-Robot')
message('  - Planificación de Tareas')
message('  - Navegación A* con detección de stuck')
