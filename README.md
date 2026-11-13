# Mision Artemis-tli
simulador por consola en C - Mision Artemis, Problema 3: Inyeccion Translunar (TLI)

## INTEGRANTES
* Gimenez, Maria Sasha Fiona
* Leguiza, Julia Estefania
* Perez Piñero, Tomas
* Spipp, Matias Hernan

### Estructura del proyecto
* `main.c`: Punto de entrada y menú principal.
* `artemis.c` / `artemis.h`: Lógica central de la simulación.
* `fisica.c` / `fisica.h`: Cálculos de trayectoria, velocidad y aceleración.
* `escenarios.c` / `escenarios.h`: Configuración de condiciones iniciales.
* `archivos.c` / `archivos.h`: Lectura y guardado de datos.
* `reportes.c` / `reportes.h`: Impresión de resultados y resúmenes de misión.
* `Makefile`: Automatización de compilación.

## Compilación y ejecución
Para compilar el proyecto desde la terminal:
gcc -Wall -Wextra -std=c99 -pedantic -O2 -o artemis main.c artemis.c fisica.c validaciones.c reportes.c escenarios.c archivos.c -lm
./artemis
