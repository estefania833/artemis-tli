/* reportes.h - Salida de texto: estado, analisis A-D, historial, reporte final y escenarios.
 * escribir_* -> a cualquier destino (stdout o archivo). mostrar_* -> a pantalla. */
#ifndef REPORTES_H
#define REPORTES_H

#include <stdio.h>
#include "artemis.h"

void escribir_encabezado(FILE *out, const char *titulo);
void escribir_parametros(FILE *out, const ParametrosMision *p);
void escribir_estado(FILE *out, const Orion *o, const ParametrosMision *p);
void escribir_analisis(FILE *out, const ParametrosMision *p, const Analisis *a);
void escribir_historial(FILE *out, const Historial *h);
void escribir_reporte_final(FILE *out, const Orion *o, const ParametrosMision *p, const Historial *h);
void escribir_tabla_escenarios(FILE *out, const ListaEscenarios *l);

void mostrar_encabezado(const char *titulo);
void mostrar_parametros(const ParametrosMision *p);
void mostrar_estado(const Orion *o, const ParametrosMision *p);
void mostrar_analisis(const ParametrosMision *p, const Analisis *a);
void mostrar_historial(const Historial *h);
void mostrar_reporte_final(const Orion *o, const ParametrosMision *p, const Historial *h);
void mostrar_tabla_escenarios(const ListaEscenarios *l);
void mostrar_escenario(const Resultado *r, int numero);

#endif
