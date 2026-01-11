# Configuración de RideBot

## Tamaño del Grid

Para cambiar el tamaño del grid, edita el archivo:
`include/domain/Global.h`

```cpp
namespace Constants {
    constexpr int GRID_WIDTH = 60;   // Cambia este valor
    constexpr int GRID_HEIGHT = 40;  // Cambia este valor
    constexpr int SIMULATION_SPEED_MS = 100;
}
```

Después de hacer cambios, recompila:
```bash
./build.sh rebuild
```

## Ejemplos de Configuración

### Grid pequeño (más rápido):
```cpp
GRID_WIDTH = 30
GRID_HEIGHT = 30
```

### Grid mediano (recomendado):
```cpp
GRID_WIDTH = 60
GRID_HEIGHT = 40
```

### Grid grande (más desafiante):
```cpp
GRID_WIDTH = 100
GRID_HEIGHT = 80
```

**NOTA**: Grids más grandes requieren más tiempo de cálculo para A*.
