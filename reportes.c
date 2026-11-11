/* reportes.c - Salida de texto. Cada funcion escribir_* recibe el destino (FILE *):
 * stdout para la pantalla o un archivo abierto para guardar el reporte (.txt).
 * Las funciones mostrar_* son atajos que escriben en pantalla. */
#include <stdio.h>
#include "artemis.h"
#include "fisica.h"
#include "reportes.h"

/* ===================== Escritura en cualquier destino ===================== */

void escribir_encabezado(FILE *out, const char *titulo)
{
    fprintf(out, "\n==============================================================\n");
    fprintf(out, "  %s\n", titulo);
    fprintf(out, "==============================================================\n");
}

void escribir_parametros(FILE *out, const ParametrosMision *p)
{
    fprintf(out, "  Masa de Orion .............. %.1f kg\n", p->masa);
    fprintf(out, "  Velocidad de inyeccion ..... %.1f m/s (%.3f km/s)\n", p->v_tli, p->v_tli / 1000.0);
    fprintf(out, "  Distancia Tierra-Luna ...... %.1f km\n", p->dist_tl / 1000.0);
    fprintf(out, "  Altitud LEO ................ %.1f km\n", p->altitud_leo / 1000.0);
    fprintf(out, "  Impulso especifico (Isp) ... %.1f s\n", p->isp);
}

void escribir_estado(FILE *out, const Orion *o, const ParametrosMision *p)
{
    double r_eq = punto_equilibrio(p->dist_tl);
    double avance = o->r / r_eq;
    int i, llenos;

    if (avance < 0.0) avance = 0.0;
    if (avance > 1.0) avance = 1.0;
    llenos = (int)(avance * 30.0);

    fprintf(out, "\n--- ESTADO DE ORION ---------------------------------------\n");
    fprintf(out, "  Etapa ............ %s\n", nombre_etapa(o->etapa));
    fprintf(out, "  Estado ........... %s\n", nombre_estado(o->estado));
    fprintf(out, "  ICPS ............. %s | Motores: %s\n",
            o->icps_acoplada ? "acoplada" : "desacoplada",
            o->motores_encendidos ? "encendidos" : "apagados");
    fprintf(out, "  Tiempo de mision . %.2f h\n", o->t / 3600.0);
    fprintf(out, "  Altitud .......... %.1f km\n", (o->r - RADIO_TIERRA) / 1000.0);
    fprintf(out, "  Dist. a la Tierra  %.1f km | a la Luna: %.1f km\n",
            o->r / 1000.0, (p->dist_tl - o->r) / 1000.0);
    fprintf(out, "  Velocidad ........ %.1f m/s (%.3f km/s)\n", o->v, o->v / 1000.0);
    fprintf(out, "  Hacia el equilibrio: [");
    for (i = 0; i < 30; i++) fputc(i < llenos ? '#' : '-', out);
    fprintf(out, "] %.1f%%\n", avance * 100.0);
    fprintf(out, "-----------------------------------------------------------\n");
}

/* (A) Tabla: como la gravedad va frenando a Orion al alejarse. */
static void escribir_tabla_inercia(FILE *out, const Analisis *a)
{
    int i;
    const FilaInercia *f;

    fprintf(out, "\n  Dist. Tierra |  g Tierra  |  g Luna    | Desacel. neta | Velocidad\n");
    fprintf(out, "      [km]     |  [m/s^2]   |  [m/s^2]   |    [m/s^2]    |   [m/s]\n");
    fprintf(out, "  -------------+------------+------------+---------------+-----------\n");
    for (i = 0; i < a->filas; i++) {
        f = &a->tabla[i];
        fprintf(out, "  %12.0f | %10.3e | %10.3e | %13.3e | ",
                f->r / 1000.0, f->g_tierra, f->g_luna, -f->a_neta);
        if (f->alcanza) {
            fprintf(out, "%9.1f\n", f->v);
        } else {
            fprintf(out, " no llega\n");
        }
    }
    fprintf(out, "  (ultima fila = punto de equilibrio: ahi la desaceleracion neta es 0)\n");
}

void escribir_analisis(FILE *out, const ParametrosMision *p, const Analisis *a)
{
    escribir_encabezado(out, "ANALISIS DEL PROBLEMA 3 (A - D)");
    escribir_parametros(out, p);

    fprintf(out, "\n[A] Primera ley de Newton (inercia)\n");
    fprintf(out, "  Con los motores apagados no actua fuerza propulsora: Orion no\n");
    fprintf(out, "  'frena' por falta de empuje; la unica fuerza neta es la gravedad.\n");
    fprintf(out, "  Si solo existiera inercia, mantendria v constante (MRU). Pero la\n");
    fprintf(out, "  atraccion terrestre (F = G M m / r^2) apunta hacia la Tierra, asi\n");
    fprintf(out, "  que su velocidad disminuye de forma progresiva al alejarse, y la\n");
    fprintf(out, "  desaceleracion cae con 1/r^2. La inercia explica que siga\n");
    fprintf(out, "  moviendose; la gravedad explica que se desacelere.\n");
    fprintf(out, "  Velocidad por conservacion de la energia: v = sqrt(v0^2 + 2 (U0 - U(r)))\n");
    escribir_tabla_inercia(out, a);
    if (a->filas >= 2) {
        fprintf(out, "  La desaceleracion pasa de %.2f m/s^2 en LEO a %.2e m/s^2 a %.0f km:\n",
                -a->tabla[0].a_neta, -a->tabla[a->filas - 2].a_neta,
                a->tabla[a->filas - 2].r / 1000.0);
        fprintf(out, "  la gravedad sigue frenando, pero cada vez con menos intensidad.\n");
    }

    fprintf(out, "\n[B] Energia cinetica inicial\n");
    fprintf(out, "  K = 1/2 m v^2 = %.5e J   (referencia consigna: 1.54453e12 J con los\n", a->k_inicial);
    fprintf(out, "  datos por defecto). Por unidad de masa: %.5e J/kg\n", a->k_esp);

    fprintf(out, "\n[C] Trabajo adicional minimo por unidad de masa LEO -> punto de equilibrio\n");
    fprintf(out, "  r_LEO = %.1f km | punto de equilibrio a %.1f km de la Tierra\n",
            a->r_leo / 1000.0, a->r_eq / 1000.0);
    fprintf(out, "  Sin motores Em = K + U se conserva, asi que el trabajo externo es la\n");
    fprintf(out, "  variacion de energia mecanica: W_ext/m = Em_final/m - Em_inicial/m\n");
    fprintf(out, "  U/m = -G MT / r - G ML / (D - r)\n\n");
    fprintf(out, "  Em_inicial/m = 1/2 v_TLI^2 + U(r_LEO)\n");
    fprintf(out, "               = %.5e + (%.5e) = %.5e J/kg\n", a->k_esp, a->u_leo, a->em_inicial);
    fprintf(out, "  Em_final/m   = 0 + U(r_eq)   (minimo: llega con v = 0)\n");
    fprintf(out, "               = %.5e J/kg\n", a->em_final);
    fprintf(out, "  W_ext/m      = %.5e - (%.5e) = %.5e J/kg\n",
            a->em_final, a->em_inicial, a->w_ext);
    if (a->w_ext > 0.0) {
        fprintf(out, "\n  => Wext/m = %.4e J/kg: la energia de la TLI NO alcanza por poco;\n", a->w_ext);
        fprintf(out, "     Orion necesita ese trabajo adicional para llegar al equilibrio.\n");
    } else {
        fprintf(out, "\n  => Wext/m = 0: le sobran %.4e J/kg, Orion llega al equilibrio\n", -a->w_ext);
        fprintf(out, "     sin trabajo adicional.\n");
    }
    fprintf(out, "     Velocidad minima de TLI para llegar: %.1f m/s (ingresada: %.1f m/s)\n",
            a->v_min_tli, p->v_tli);

    fprintf(out, "\n[D] Tierra->Luna vs Luna->Tierra\n");
    fprintf(out, "  Energia por unidad de masa:\n");
    fprintf(out, "  Escapar de la superficie terrestre (GM/R) .. %.4e J/kg\n", a->e_escape_tierra);
    fprintf(out, "  Escapar de la superficie lunar (GM/R) ...... %.4e J/kg\n", a->e_escape_luna);
    fprintf(out, "  Superficie terrestre -> equilibrio ......... %.4e J/kg\n", a->w_tierra_eq);
    fprintf(out, "  Superficie lunar -> equilibrio ............. %.4e J/kg\n", a->w_luna_eq);
    fprintf(out, "  Cociente (Tierra->Luna)/(Luna->Tierra) ..... %.1f veces\n", a->cociente_tl_lt);

    fprintf(out, "\n  Combustible (Tsiolkovsky: m_comb = m (e^(dv/ve) - 1), ve = Isp g0 = %.1f m/s)\n",
            a->v_gases);
    fprintf(out, "  Delta-v ideal ida (sup. Tierra -> equilibrio) .. %8.1f m/s\n", a->dv_ida);
    fprintf(out, "  Delta-v ideal vuelta (sup. Luna -> equilibrio) . %8.1f m/s\n", a->dv_vuelta);
    fprintf(out, "  Combustible ida ................................ %10.1f kg\n", a->comb_ida);
    fprintf(out, "  Combustible vuelta ............................. %10.1f kg\n", a->comb_vuelta);
    fprintf(out, "  Cociente de combustible (ida / vuelta) ......... %10.1f veces\n", a->cociente_comb);
    fprintf(out, "  Solo la maniobra TLI (dv = %.1f m/s) ......... %10.1f kg\n", a->dv_tli, a->comb_tli);
    fprintf(out, "  (masa final = %.0f kg de Orion; delta-v ideal, sin perdidas por\n", p->masa);
    fprintf(out, "   atmosfera ni gravedad durante el encendido)\n");

    fprintf(out, "\n  La Tierra tiene mucha mas masa y gravedad superficial (pozo de\n");
    fprintf(out, "  potencial mucho mas profundo): salir de ella exige mucha mas\n");
    fprintf(out, "  energia. Ademas, el combustible crece EXPONENCIALMENTE con el delta-v:\n");
    fprintf(out, "  la energia de ida es %.1f veces la de vuelta, pero el combustible\n",
            a->cociente_tl_lt);
    fprintf(out, "  es %.1f veces mayor. El regreso cae casi \"cuesta abajo\".\n", a->cociente_comb);
}

void escribir_historial(FILE *out, const Historial *h)
{
    int i;

    fprintf(out, "\n--- HISTORIAL DE EVENTOS (%d) ---------------------------\n", h->cantidad);
    if (h->cantidad == 0) {
        fprintf(out, "  (sin eventos registrados)\n");
    }
    for (i = 0; i < h->cantidad; i++) {
        fprintf(out, "  %2d) T+%7.2f h | %-12s | %s\n", i + 1, h->eventos[i].tiempo / 3600.0,
                nombre_etapa(h->eventos[i].etapa), h->eventos[i].descripcion);
    }
}

void escribir_reporte_final(FILE *out, const Orion *o, const ParametrosMision *p, const Historial *h)
{
    escribir_encabezado(out, "REPORTE FINAL DE MISION - ARTEMIS TLI");
    escribir_parametros(out, p);
    escribir_estado(out, o, p);
    fprintf(out, "  Maxima distancia alcanzada: %.1f km (equilibrio en %.1f km)\n",
            o->r_max / 1000.0, punto_equilibrio(p->dist_tl) / 1000.0);
    fprintf(out, "  Energia cinetica inicial (TLI): %.5e J\n",
            energia_cinetica(p->masa, p->v_tli));
    fprintf(out, "  Resultado: %s\n", nombre_estado(o->estado));
    escribir_historial(out, h);
}

/* ===================== Escenarios (barrido y comparacion) ===================== */

static void escribir_cabecera_escenarios(FILE *out)
{
    fprintf(out, "\n   # | vTLI m/s | masa kg | LEO km | D mil km | Resultado | Dist.max km | Tiempo h | Wext/m J/kg\n");
    fprintf(out, "  ---+----------+---------+--------+----------+-----------+-------------+----------+------------\n");
}

static void escribir_fila_escenario(FILE *out, const Resultado *r, int numero)
{
    fprintf(out, "  %2d | %8.1f | %7.0f | %6.0f | %8.1f | %-9s | %11.0f | %8.2f | %11.3e\n",
            numero, r->params.v_tli, r->params.masa, r->params.altitud_leo / 1000.0,
            r->params.dist_tl / 1.0e6, nombre_estado_corto(r->estado),
            r->r_max / 1000.0, r->t_fin / 3600.0, r->w_ext);
}

void escribir_tabla_escenarios(FILE *out, const ListaEscenarios *l)
{
    int i;

    if (l->cantidad == 0) {
        fprintf(out, "\n  (no hay escenarios cargados)\n");
        return;
    }
    escribir_cabecera_escenarios(out);
    for (i = 0; i < l->cantidad; i++) {
        escribir_fila_escenario(out, &l->items[i], i + 1);
    }
    fprintf(out, "  Tiempo h = hasta el equilibrio (EXITOSA) o hasta el apogeo (RETORNO).\n");
    fprintf(out, "  Wext/m > 0: trabajo adicional que falta; <= 0: la TLI alcanza sola.\n");
}

/* ===================== Atajos a pantalla ===================== */

void mostrar_encabezado(const char *titulo)
{
    escribir_encabezado(stdout, titulo);
}

void mostrar_parametros(const ParametrosMision *p)
{
    escribir_parametros(stdout, p);
}

void mostrar_estado(const Orion *o, const ParametrosMision *p)
{
    escribir_estado(stdout, o, p);
}

void mostrar_analisis(const ParametrosMision *p, const Analisis *a)
{
    escribir_analisis(stdout, p, a);
}

void mostrar_historial(const Historial *h)
{
    escribir_historial(stdout, h);
}

void mostrar_reporte_final(const Orion *o, const ParametrosMision *p, const Historial *h)
{
    escribir_reporte_final(stdout, o, p, h);
}

void mostrar_tabla_escenarios(const ListaEscenarios *l)
{
    escribir_tabla_escenarios(stdout, l);
}

void mostrar_escenario(const Resultado *r, int numero)
{
    escribir_cabecera_escenarios(stdout);
    escribir_fila_escenario(stdout, r, numero);
}
