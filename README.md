# OS-Bot

## Compilacion y Ejecucion

Este proyecto incluye un script `build.sh` para facilitar la compilacion y ejecucion.

### Permisos

Asegurate de que el script tenga permisos de ejecucion:

```bash
chmod +x build.sh
```

### Comandos Disponibles

**Compilar y Ejecutar (Opcion recomendada)**

Compila el proyecto y, si es exitoso, lo ejecuta inmediatamente.

```bash
./build.sh
```

O explicitamente:

```bash
./build.sh run
```

**Solo Compilar**

Compila el proyecto sin ejecutarlo.

```bash
./build.sh build
```

**Limpiar**

Elimina los archivos de compilacion (directorio `build`).

```bash
./build.sh clean
```

**Reconstruir**

Limpia todo y vuelve a compilar desde cero.

```bash
./build.sh rebuild
```
