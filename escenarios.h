/* escenarios.h - Barrido de velocidades y comparacion de escenarios (ordenamiento y busqueda). */
#ifndef ESCENARIOS_H
#define ESCENARIOS_H

#include "artemis.h"

/* Operaciones sobre la lista (sin entrada/salida por pantalla) */
int  agregar_escenario(ListaEscenarios *l, const Resultado *r);    /* 1 = ok, 0 = lista llena */
int  eliminar_escenario(ListaEscenarios *l, int indice);           /* 1 = ok, 0 = indice invalido */
void ordenar_escenarios(ListaEscenarios *l, CriterioOrden criterio, int ascendente);
int  buscar_menor_v_exitosa(const ListaEscenarios *l);             /* indice, o -1 si ninguno llega */

/* Interactivos */
void barrido_velocidades(const ParametrosMision *p, ListaEscenarios *l);
void menu_escenarios(ListaEscenarios *l, const ParametrosMision *p);

#endif
