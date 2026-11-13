/* validaciones.h - Ingreso seguro de datos por consola.
 * Todas devuelven 1 si se obtuvo un valor valido, 0 si hubo fin de entrada (EOF). */
#ifndef VALIDACIONES_H
#define VALIDACIONES_H

#include <stddef.h>

int  leer_linea(char *buffer, size_t tam);
int  leer_entero(const char *msg, int min, int max, int *destino);
int  leer_double(const char *msg, double min, double max, double *destino);
int  leer_si_no(const char *msg, int *destino);   /* destino: 1 = si, 0 = no */
void esperar_enter(void);
/* Nombre de archivo: solo letras, numeros, '_', '-' y '.'. ENTER = por_defecto.
 * Si no termina en 'extension' (ej: ".csv") se la agrega. */
int  leer_nombre_archivo(const char *msg, const char *por_defecto, const char *extension,
                         char *destino, size_t tam);

#endif
