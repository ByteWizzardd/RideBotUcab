#!/bin/bash

# ============================================
# Script de Compilación y Ejecución - OS-Bot
# ============================================

set -e  # Salir si hay algún error

echo "╔════════════════════════════════════════════════════╗"
echo "║      OS-BOT - Script de Compilación Rápida        ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Función de ayuda
show_help() {
    echo "Uso: ./build.sh [OPCIÓN]"
    echo ""
    echo "Opciones:"
    echo "  build     - Solo compilar el proyecto"
    echo "  run       - Compilar y ejecutar"
    echo "  clean     - Limpiar archivos de compilación"
    echo "  rebuild   - Limpiar y recompilar desde cero"
    echo "  help      - Mostrar esta ayuda"
    echo ""
    echo "Sin argumentos: Compila y ejecuta (equivalente a 'run')"
    exit 0
}

# Función para compilar
do_build() {
    echo -e "${YELLOW}[1/2] Configurando proyecto con Meson...${NC}"
    if [ ! -d "build" ]; then
        meson setup build
    else
        echo "  → Directorio build ya existe, usando configuración existente"
    fi
    
    echo ""
    echo -e "${YELLOW}[2/2] Compilando con Ninja...${NC}"
    meson compile -C build
    
    if [ $? -eq 0 ]; then
        echo ""
        echo -e "${GREEN}✅ Compilación exitosa!${NC}"
        echo "  → Ejecutable: build/os-bot"
        return 0
    else
        echo ""
        echo -e "${RED}❌ Error en la compilación${NC}"
        return 1
    fi
}

# Función para ejecutar
do_run() {
    if [ ! -f "build/os-bot" ]; then
        echo -e "${RED}❌ Ejecutable no encontrado. Compilando primero...${NC}"
        do_build || exit 1
    fi
    
    echo ""
    echo -e "${GREEN}🚀 Ejecutando OS-Bot...${NC}"
    echo "  → Presiona Ctrl+C para detener la simulación"
    echo ""
    sleep 1
    ./build/os-bot
}

# Función para limpiar
do_clean() {
    echo -e "${YELLOW}🧹 Limpiando archivos de compilación...${NC}"
    if [ -d "build" ]; then
        rm -rf build
        echo -e "${GREEN}✅ Directorio build eliminado${NC}"
    else
        echo "  → No hay nada que limpiar"
    fi
}

# Función para rebuild
do_rebuild() {
    do_clean
    echo ""
    do_build
}

# Procesamiento de argumentos
case "${1:-run}" in
    build)
        do_build
        ;;
    run)
        do_build && do_run
        ;;
    clean)
        do_clean
        ;;
    rebuild)
        do_rebuild
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo -e "${RED}❌ Opción no reconocida: $1${NC}"
        echo ""
        show_help
        ;;
esac

