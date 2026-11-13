/* escenarios.c - Barrido de velocidades de inyeccion y lista de escenarios:
 * agregar, eliminar, ordenar (seleccion) y buscar (lineal). */
#include <stdio.h>
#include "artemis.h"
#include "fisica.h"
#include "validaciones.h"
#include "reportes.h"
#include "escenarios.h"

/* ===================== Operaciones sobre la lista ===================== */

int agregar_escenario(ListaEscenarios *l, const Resultado *r)
{
    if (l->cantidad >= MAX_ESCENARIOS) {
        return 0;
    }
    l->items[l->cantidad] = *r;
    l->cantidad++;
    return 1;
}

/* Elimina el elemento 'indice' (0 .. cantidad-1) corriendo los siguientes un lugar. */
int eliminar_escenario(ListaEscenarios *l, int indice)
{
    int i;

    if (indice < 0 || indice >= l->cantidad) {
        return 0;
    }
    for (i = indice; i < l->cantidad - 1; i++) {
        l->items[i] = l->items[i + 1];
    }
    l->cantidad--;
    return 1;
}

static double valor_criterio(const Resultado *r, CriterioOrden criterio)
{
    switch (criterio) {
        case ORDEN_VELOCIDAD: return r->params.v_tli;
        case ORDEN_DISTANCIA: return r->r_max;
        case ORDEN_TIEMPO:    return r->t_fin;
        case ORDEN_TRABAJO:   return r->w_ext;
    }
    return 0.0;
}

/* Ordenamiento por seleccion: en cada pasada busca el menor (o mayor) del resto
 * y lo intercambia con la posicion i. */
void ordenar_escenarios(ListaEscenarios *l, CriterioOrden criterio, int ascendente)
{
    int i, j, elegido;
    double vj, ve;
    Resultado aux;

    for (i = 0; i < l->cantidad - 1; i++) {
        elegido = i;
        for (j = i + 1; j < l->cantidad; j++) {
            vj = valor_criterio(&l->items[j], criterio);
            ve = valor_criterio(&l->items[elegido], criterio);
            if (ascendente ? (vj < ve) : (vj > ve)) {
                elegido = j;
            }
        }
        if (elegido != i) {
            aux = l->items[i];
            l->items[i] = l->items[elegido];
            l->items[elegido] = aux;
        }
    }
}

/* Busqueda lineal: entre los escenarios que llegan al equilibrio, el de menor v_TLI. */
int buscar_menor_v_exitosa(const ListaEscenarios *l)
{
    int i;
    int mejor = -1;

    for (i = 0; i < l->cantidad; i++) {
        if (l->items[i].estado == MISION_EXITOSA) {
            if (mejor == -1 || l->items[i].params.v_tli < l->items[mejor].params.v_tli) {
                mejor = i;
            }
        }
    }
    return mejor;
}

/* ===================== Barrido de velocidades ===================== */

void barrido_velocidades(const ParametrosMision *p, ListaEscenarios *l)
{
    ListaEscenarios barrido;
    ParametrosMision q = *p;
    double desde, hasta, paso = 1.0, paso_min, paso_max;
    int n, i, idx, agregar, agregados;
    char msg[160];

    mostrar_encabezado("BARRIDO DE VELOCIDADES DE INYECCION");
    printf("  Se simula una mision completa por cada velocidad, con el resto de los\n");
    printf("  parametros actuales. Maximo %d simulaciones.\n\n", MAX_ESCENARIOS);

    snprintf(msg, sizeof msg, "Velocidad inicial [m/s] (%.0f - %.0f): ", V_TLI_MIN, V_TLI_MAX);
    if (!leer_double(msg, V_TLI_MIN, V_TLI_MAX, &desde)) return;

    snprintf(msg, sizeof msg, "Velocidad final [m/s] (%.1f - %.0f): ", desde, V_TLI_MAX);
    if (!leer_double(msg, desde, V_TLI_MAX, &hasta)) return;

    if (hasta > desde) {
        /* el paso minimo asegura no superar MAX_ESCENARIOS simulaciones */
        paso_max = hasta - desde;
        paso_min = paso_max / (MAX_ESCENARIOS - 1);
        snprintf(msg, sizeof msg, "Paso [m/s] (%.2f - %.2f): ", paso_min, paso_max);
        if (!leer_double(msg, paso_min, paso_max, &paso)) return;
        n = (int)((hasta - desde) / paso + 1e-9) + 1;
        if (n > MAX_ESCENARIOS) {
            n = MAX_ESCENARIOS;
        }
    } else {
        n = 1;
    }

    printf("\n  Simulando %d mision(es)...\n", n);
    barrido.cantidad = 0;
    for (i = 0; i < n; i++) {
        q.v_tli = desde + i * paso;
        simular_automatico(&q, &barrido.items[i]);
        barrido.cantidad++;
    }

    mostrar_tabla_escenarios(&barrido);

    idx = buscar_menor_v_exitosa(&barrido);
    if (idx >= 0) {
        printf("\n  Menor velocidad del barrido que llega al equilibrio: %.1f m/s\n",
               barrido.items[idx].params.v_tli);
    } else {
        printf("\n  Ninguna velocidad del barrido llega al equilibrio.\n");
    }
    printf("  Velocidad minima analitica (Em = K + U) .......... %.1f m/s\n",
           velocidad_minima_tli(radio_desde_altitud(p->altitud_leo), p->dist_tl));

    snprintf(msg, sizeof msg, "\nAgregar estos %d resultados a los escenarios? (s/n): ", n);
    if (!leer_si_no(msg, &agregar) || !agregar) {
        return;
    }
    agregados = 0;
    for (i = 0; i < barrido.cantidad; i++) {
        if (!agregar_escenario(l, &barrido.items[i])) {
            printf("  [!] Lista de escenarios llena (maximo %d).\n", MAX_ESCENARIOS);
            break;
        }
        agregados++;
    }
    printf("  Se agregaron %d escenario(s). Total: %d.\n", agregados, l->cantidad);
}

/* ===================== Menu de escenarios ===================== */

static void opcion_ordenar(ListaEscenarios *l)
{
    int criterio, sentido;

    if (l->cantidad < 2) {
        printf("\n  [!] Se necesitan al menos 2 escenarios para ordenar.\n");
        return;
    }
    printf("\n  Ordenar por: 1) Velocidad TLI  2) Distancia maxima  3) Tiempo  4) Wext/m\n");
    if (!leer_entero("  Criterio: ", 1, 4, &criterio)) return;
    printf("  Sentido: 1) Ascendente  2) Descendente\n");
    if (!leer_entero("  Sentido: ", 1, 2, &sentido)) return;

    ordenar_escenarios(l, (CriterioOrden)criterio, sentido == 1);
    mostrar_tabla_escenarios(l);
}

static void opcion_eliminar(ListaEscenarios *l)
{
    int numero;
    char msg[64];

    if (l->cantidad == 0) {
        printf("\n  [!] No hay escenarios cargados.\n");
        return;
    }
    mostrar_tabla_escenarios(l);
    snprintf(msg, sizeof msg, "\n  Numero a eliminar (1 - %d): ", l->cantidad);
    if (!leer_entero(msg, 1, l->cantidad, &numero)) return;
    eliminar_escenario(l, numero - 1);
    printf("  Escenario %d eliminado. Quedan %d.\n", numero, l->cantidad);
}

void menu_escenarios(ListaEscenarios *l, const ParametrosMision *p)
{
    int opcion = -1;
    int idx, confirmar;
    Resultado r;

    while (opcion != 0) {
        mostrar_encabezado("ESCENARIOS - COMPARAR MISIONES");
        printf("  Escenarios cargados: %d / %d\n\n", l->cantidad, MAX_ESCENARIOS);
        printf("  1) Agregar escenario con los parametros actuales\n");
        printf("  2) Ver escenarios\n");
        printf("  3) Ordenar escenarios\n");
        printf("  4) Buscar la menor velocidad que llega al equilibrio\n");
        printf("  5) Eliminar un escenario\n");
        printf("  6) Vaciar la lista\n");
        printf("  0) Volver\n\n");

        if (!leer_entero("Opcion: ", 0, 6, &opcion)) {
            return;
        }
        switch (opcion) {
            case 1:
                simular_automatico(p, &r);
                if (agregar_escenario(l, &r)) {
                    printf("\n  Escenario agregado:\n");
                    mostrar_escenario(&r, l->cantidad);
                } else {
                    printf("\n  [!] Lista llena (maximo %d). Elimine alguno primero.\n", MAX_ESCENARIOS);
                }
                break;
            case 2:
                mostrar_tabla_escenarios(l);
                break;
            case 3:
                opcion_ordenar(l);
                break;
            case 4:
                idx = buscar_menor_v_exitosa(l);
                if (idx >= 0) {
                    printf("\n  Menor velocidad que llega al equilibrio:\n");
                    mostrar_escenario(&l->items[idx], idx + 1);
                } else {
                    printf("\n  Ningun escenario cargado llega al equilibrio.\n");
                }
                break;
            case 5:
                opcion_eliminar(l);
                break;
            case 6:
                if (!leer_si_no("  Seguro que quiere borrar todos los escenarios? (s/n): ", &confirmar)) {
                    return;
                }
                if (confirmar) {
                    l->cantidad = 0;
                    printf("  Lista vaciada.\n");
                }
                break;
            case 0:
                break;
        }
    }
}
