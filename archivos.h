/* archivos.h - Exportar e importar datos (CSV / TXT). No escribe en pantalla:
 * devuelve un EstadoArchivo y el que llama decide que mostrar. */
#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include "artemis.h"

typedef enum {
    ARCH_OK = 0,
    ARCH_ERROR_ABRIR,       /* no se pudo abrir / crear el archivo */
    ARCH_ERROR_ESCRIBIR,    /* fallo al escribir o al cerrar */
    ARCH_ERROR_FORMATO,     /* contenido con formato invalido o incompleto */
    ARCH_ERROR_RANGO        /* valores fuera de los rangos validos */
} EstadoArchivo;

int           archivo_existe(const char *nombre);
const char   *mensaje_archivo(EstadoArchivo e);

/* CSV con ';' como separador y coma decimal (abre directo en Excel en espanol) */
EstadoArchivo exportar_trayectoria_csv(const char *nombre, const ParametrosMision *p, int *filas);
EstadoArchivo exportar_escenarios_csv(const char *nombre, const ListaEscenarios *l);

/* Reporte de texto: analisis A-D + ultima mision (si o != NULL) + escenarios (si hay) */
EstadoArchivo guardar_reporte_txt(const char *nombre, const ParametrosMision *p, const Analisis *a,
                                  const Orion *o, const ParametrosMision *p_mision,
                                  const Historial *h, const ListaEscenarios *l);

/* Parametros en formato clave=valor */
EstadoArchivo guardar_parametros(const char *nombre, const ParametrosMision *p);
EstadoArchivo cargar_parametros(const char *nombre, ParametrosMision *p);

#endif
