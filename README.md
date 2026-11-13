# Mision Artemis-tli
simulador por consola en C - Mision Artemis, Problema 3: Inyeccion Translunar (TLI)

## INTEGRANTES
* Gimenez, Maria Sasha Fiona
* Leguiza, Julia Estefania
* Perez Piñero, Tomas
* Spipp, Matias Hernan

### Reglas principales:
* El jugador/usuario ingresa o ajusta los parámetros iniciales y de empuje de la misión.
* La simulación avanza por intervalos de tiempo actualizando la física del movimiento.
* La misión se considera exitosa si se alcanza la ventana orbital objetivo dentro de los márgenes de velocidad y combustible permitidos; caso contrario, se reporta el fallo correspondiente.

#### Estructura del proyecto
* `main.c`: Punto de entrada y menú principal.
* `artemis.c` / `artemis.h`: Lógica central de la simulación.
* `fisica.c` / `fisica.h`: Cálculos de trayectoria, velocidad y aceleración.
* `escenarios.c` / `escenarios.h`: Configuración de condiciones iniciales.
* `archivos.c` / `archivos.h`: Lectura y guardado de datos.
* `reportes.c` / `reportes.h`: Impresión de resultados y resúmenes de misión.
* `Makefile`: Automatización de compilación.

##### Compilación y ejecución
Para compilar el proyecto desde la terminal:
* gcc -Wall -Wextra -std=c99 -pedantic -O2 -o artemis main.c artemis.c fisica.c validaciones.c reportes.c escenarios.c archivos.c -lm
./artemis

###### Registro de Uso de Inteligencia Artificial
En cumplimiento con las pautas de la cátedra, se detalla el uso de herramientas de IA durante el desarrollo del proyecto:

* **Herramienta utilizada:** Gemini (Google).
* **Tareas específicas:** 
  * Asistencia en la organización de la arquitectura modular del proyecto y definición de estructuras de datos.
  * Apoyo en la resolución de errores de compilación y enlazado entre múltiples módulos en lenguaje C.
* **Prompts / Consultas principales realizadas:**
  * *"¿Cómo organizar correctamente las estructuras y prototipos de funciones entre los archivos `.h` y `.c` para que no haya conflictos al incluirlos?"*
  * *"Revisar si la sintaxis y los tipos de datos del módulo de reportes compilan correctamente con el resto del proyecto en GCC."*
  * *"¿Cómo estructurar las tablas de salida por consola en C para mostrar de forma clara los resultados de la simulación?"*
* **Validación y adaptación del código generado:** 
  * Todas las sugerencias de estructuras y prototipos fueron revisadas y adaptadas a las variables y requerimientos específicos del problema de Inyección Translunar (TLI).
  * El código fue compilado y probado localmente en Visual Studio Code para verificar que no existieran errores ni fallos de ejecución antes de integrarlo al repositorio.
