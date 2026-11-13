/* artemis.c - Estado de la mision, etapas y ciclo de simulacion. */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "artemis.h"
#include "fisica.h"
#include "validaciones.h"
#include "reportes.h"

const char *nombre_etapa(EtapaMision e)
{
    static const char *nombres[] = {
        "LEO", "TLI", "Desacople", "Navegacion", "Finalizada"
    };
    return nombres[e];
}

const char *nombre_estado(EstadoMision s)
{
    switch (s) {
        case MISION_EN_CURSO: return "En curso";
        case MISION_EXITOSA:  return "EXITOSA: alcanza el punto de equilibrio Tierra-Luna";
        case MISION_RETORNO:  return "Sin llegada: alcanza el apogeo y vuelve hacia la Tierra";
        case MISION_ABORTADA: return "Abortada por el operador";
    }
    return "Desconocido";
}

/* Version corta para tablas y archivos CSV. */
const char *nombre_estado_corto(EstadoMision s)
{
    switch (s) {
        case MISION_EN_CURSO: return "EN CURSO";
        case MISION_EXITOSA:  return "EXITOSA";
        case MISION_RETORNO:  return "RETORNO";
        case MISION_ABORTADA: return "ABORTADA";
    }
    return "?";
}

void params_por_defecto(ParametrosMision *p)
{
    p->masa        = MASA_DEF;
    p->v_tli       = V_TLI_DEF;
    p->dist_tl     = DIST_TL_DEF;
    p->altitud_leo = ALT_LEO_DEF;
    p->isp         = ISP_DEF;
}

/* 1 si todos los parametros estan dentro de los rangos validos (ej: al cargar un archivo). */
int parametros_validos(const ParametrosMision *p)
{
    return p->masa        >= MASA_MIN    && p->masa        <= MASA_MAX
        && p->v_tli       >= V_TLI_MIN   && p->v_tli       <= V_TLI_MAX
        && p->dist_tl     >= DIST_TL_MIN && p->dist_tl     <= DIST_TL_MAX
        && p->altitud_leo >= ALT_LEO_MIN && p->altitud_leo <= ALT_LEO_MAX
        && p->isp         >= ISP_MIN     && p->isp         <= ISP_MAX;
}

void configurar_parametros(ParametrosMision *p)
{
    ParametrosMision nuevo = *p;
    char msg[160];

    mostrar_encabezado("CONFIGURACION DE MISION");
    printf("  (Los rangos protegen al modelo fisico)\n\n");

    snprintf(msg, sizeof msg, "Masa de Orion [kg] (%.0f - %.0f): ", MASA_MIN, MASA_MAX);
    if (!leer_double(msg, MASA_MIN, MASA_MAX, &nuevo.masa)) return;

    snprintf(msg, sizeof msg, "Velocidad de inyeccion [m/s] (%.0f - %.0f): ", V_TLI_MIN, V_TLI_MAX);
    if (!leer_double(msg, V_TLI_MIN, V_TLI_MAX, &nuevo.v_tli)) return;

    snprintf(msg, sizeof msg, "Distancia Tierra-Luna [m] (%.3g - %.3g): ", DIST_TL_MIN, DIST_TL_MAX);
    if (!leer_double(msg, DIST_TL_MIN, DIST_TL_MAX, &nuevo.dist_tl)) return;

    snprintf(msg, sizeof msg, "Altitud LEO [m] (%.0f - %.0f): ", ALT_LEO_MIN, ALT_LEO_MAX);
    if (!leer_double(msg, ALT_LEO_MIN, ALT_LEO_MAX, &nuevo.altitud_leo)) return;

    snprintf(msg, sizeof msg, "Impulso especifico del motor [s] (%.0f - %.0f): ", ISP_MIN, ISP_MAX);
    if (!leer_double(msg, ISP_MIN, ISP_MAX, &nuevo.isp)) return;

    *p = nuevo;     /* solo se pisan los datos si se completo toda la carga */
    printf("\n  Parametros actualizados.\n");
}

/* Agrega una fila a la tabla de desaceleracion (A) si hay lugar. */
static void agregar_fila_inercia(Analisis *a, const ParametrosMision *p, double r)
{
    FilaInercia *f;

    if (a->filas >= MAX_FILAS_A) {
        return;
    }
    f = &a->tabla[a->filas];
    f->r        = r;
    f->g_tierra = gravedad_tierra(r);
    f->g_luna   = gravedad_luna(r, p->dist_tl);
    f->a_neta   = aceleracion_radial(r, p->dist_tl);
    f->alcanza  = velocidad_en(r, a->r_leo, p->v_tli, p->dist_tl, &f->v);
    a->filas++;
}

/* (A) Distancias de muestra [m desde el centro de la Tierra] entre LEO y el equilibrio. */
static void calcular_tabla_inercia(const ParametrosMision *p, Analisis *a)
{
    static const double muestras[] = {
        2.0e7, 5.0e7, 1.0e8, 1.5e8, 2.0e8, 2.5e8, 3.0e8
    };
    int n = (int)(sizeof muestras / sizeof muestras[0]);
    int i;

    a->filas = 0;
    agregar_fila_inercia(a, p, a->r_leo);
    for (i = 0; i < n; i++) {
        if (muestras[i] > a->r_leo && muestras[i] < a->r_eq) {
            agregar_fila_inercia(a, p, muestras[i]);
        }
    }
    agregar_fila_inercia(a, p, a->r_eq);
}

void calcular_analisis(const ParametrosMision *p, Analisis *a)
{
    double gm_t = G_CONST * MASA_TIERRA;
    double gm_l = G_CONST * MASA_LUNA;
    double r_sup_luna = p->dist_tl - RADIO_LUNA;

    /* (B) */
    a->r_leo     = radio_desde_altitud(p->altitud_leo);
    a->k_inicial = energia_cinetica(p->masa, p->v_tli);
    a->k_esp     = 0.5 * p->v_tli * p->v_tli;

    /* (C) */
    a->r_eq      = punto_equilibrio(p->dist_tl);
    a->w_tierra  = trabajo_min_esp(a->r_leo, a->r_eq, p->dist_tl, 0);
    a->w_total   = trabajo_min_esp(a->r_leo, a->r_eq, p->dist_tl, 1);
    a->u_leo      = potencial_esp(a->r_leo, p->dist_tl, 1);
    a->u_eq       = potencial_esp(a->r_eq, p->dist_tl, 1);
    a->em_inicial = a->k_esp + a->u_leo;
    a->em_final   = a->u_eq;
    a->w_ext      = a->em_final - a->em_inicial;
    a->v_min_tli  = velocidad_minima_tli(a->r_leo, p->dist_tl);
    a->v_orb_leo  = velocidad_orbital(a->r_leo);

    /* (A) usa r_leo y r_eq, por eso va despues */
    calcular_tabla_inercia(p, a);

    /* (D) energias */
    a->w_tierra_eq     = trabajo_min_esp(RADIO_TIERRA, a->r_eq, p->dist_tl, 1);
    a->w_luna_eq       = trabajo_min_esp(r_sup_luna, a->r_eq, p->dist_tl, 1);
    a->cociente_tl_lt  = a->w_tierra_eq / a->w_luna_eq;
    a->e_escape_tierra = energia_escape_esp(gm_t, RADIO_TIERRA);
    a->e_escape_luna   = energia_escape_esp(gm_l, RADIO_LUNA);

    /* (D) combustible: delta-v ideal (1/2 dv^2 = W/m) y Tsiolkovsky con la masa de Orion */
    a->v_gases     = velocidad_gases(p->isp);
    a->dv_ida      = sqrt(2.0 * a->w_tierra_eq);
    a->dv_vuelta   = sqrt(2.0 * a->w_luna_eq);
    a->comb_ida    = masa_combustible(p->masa, a->dv_ida, p->isp);
    a->comb_vuelta = masa_combustible(p->masa, a->dv_vuelta, p->isp);
    a->cociente_comb = a->comb_ida / a->comb_vuelta;
    a->dv_tli      = p->v_tli - a->v_orb_leo;
    a->comb_tli    = masa_combustible(p->masa, a->dv_tli, p->isp);
}

/* Registra un evento si hay lugar. Devuelve 1 si se guardo, 0 si el historial esta lleno. */
int registrar_evento(Historial *h, double t, EtapaMision etapa, const char *desc)
{
    Evento *e;

    if (h->cantidad >= MAX_EVENTOS) {
        return 0;
    }
    e = &h->eventos[h->cantidad];
    e->tiempo = t;
    e->etapa  = etapa;
    snprintf(e->descripcion, TAM_DESC, "%s", desc);
    h->cantidad++;
    return 1;
}

void iniciar_orion(Orion *o, const ParametrosMision *p)
{
    o->masa = p->masa;
    o->r    = radio_desde_altitud(p->altitud_leo);
    o->v    = velocidad_orbital(o->r);    /* velocidad circular en LEO */
    o->t    = 0.0;
    o->r_max = o->r;
    o->icps_acoplada = 1;
    o->motores_encendidos = 0;
    o->ultimo_hito = 0;
    o->etapa  = ETAPA_LEO;
    o->estado = MISION_EN_CURSO;
}

void ejecutar_tli(Orion *o, const ParametrosMision *p, Historial *h)
{
    char desc[TAM_DESC];

    o->etapa = ETAPA_TLI;
    o->motores_encendidos = 1;
    registrar_evento(h, o->t, ETAPA_TLI, "Encendido de la ICPS");

    o->v = p->v_tli;
    snprintf(desc, sizeof desc, "Inyeccion translunar completada: v = %.1f m/s", o->v);
    registrar_evento(h, o->t, ETAPA_TLI, desc);
}

void desacoplar_icps(Orion *o, Historial *h)
{
    o->etapa = ETAPA_DESACOPLE;
    o->icps_acoplada = 0;
    o->motores_encendidos = 0;
    registrar_evento(h, o->t, ETAPA_DESACOPLE, "ICPS desacoplada");
    registrar_evento(h, o->t, ETAPA_DESACOPLE, "Motores apagados: trayectoria libre");
    o->etapa = ETAPA_NAVEGACION;
}

/* Avanza 'horas' de mision (o menos si termina antes). Devuelve 1 si sigue en curso. */
int navegar(Orion *o, const ParametrosMision *p, double horas, Historial *h)
{
    double r_eq = punto_equilibrio(p->dist_tl);
    long pasos = (long)(horas * 3600.0 / DT_SIM + 0.5);   /* redondeo: evita perder un paso */
    long i;
    char desc[TAM_DESC];

    for (i = 0; i < pasos && o->estado == MISION_EN_CURSO; i++) {
        avanzar_verlet(&o->r, &o->v, DT_SIM, p->dist_tl);
        o->t += DT_SIM;
        if (o->r > o->r_max) {
            o->r_max = o->r;
        }
        if ((int)(o->r / 1.0e8) > o->ultimo_hito) {
            o->ultimo_hito = (int)(o->r / 1.0e8);
            snprintf(desc, sizeof desc, "Cruza los %d 000 km de la Tierra", o->ultimo_hito * 100);
            registrar_evento(h, o->t, ETAPA_NAVEGACION, desc);
        }
        if (o->r >= r_eq) {
            o->estado = MISION_EXITOSA;
        } else if (o->v <= 0.0) {
            o->v = 0.0;                 /* apogeo: velocidad radial nula */
            o->estado = MISION_RETORNO;
        }
    }

    if (o->estado == MISION_EXITOSA) {
        registrar_evento(h, o->t, ETAPA_NAVEGACION, "Alcanza el punto de equilibrio Tierra-Luna");
    } else if (o->estado == MISION_RETORNO) {
        snprintf(desc, sizeof desc, "Apogeo a %.0f km: la gravedad invierte el movimiento", o->r / 1000.0);
        registrar_evento(h, o->t, ETAPA_NAVEGACION, desc);
    }
    if (o->estado != MISION_EN_CURSO) {
        o->etapa = ETAPA_FINALIZADA;
    }
    return o->estado == MISION_EN_CURSO;
}

void abortar_mision(Orion *o, Historial *h)
{
    o->estado = MISION_ABORTADA;
    o->etapa  = ETAPA_FINALIZADA;
    registrar_evento(h, o->t, ETAPA_FINALIZADA, "Mision abortada por el operador");
}

/* Ciclo interactivo completo. Devuelve 1 si la mision se ejecuto (aunque fuera abortada). */
int simular_mision(Orion *o, const ParametrosMision *p, Historial *h)
{
    int opcion, horas, confirmar;

    h->cantidad = 0;
    iniciar_orion(o, p);
    registrar_evento(h, 0.0, ETAPA_LEO, "Orion en orbita baja terrestre (LEO)");

    mostrar_encabezado("INICIO DE MISION - ARTEMIS TLI");
    mostrar_parametros(p);
    mostrar_estado(o, p);

    if (!leer_si_no("\nEjecutar la Inyeccion Translunar (TLI)? (s/n): ", &confirmar)) {
        return 0;
    }
    if (!confirmar) {
        abortar_mision(o, h);
        mostrar_reporte_final(o, p, h);
        return 1;
    }

    ejecutar_tli(o, p, h);
    desacoplar_icps(o, h);
    printf("\n  >> TLI completada. ICPS desacoplada y motores apagados.\n");
    mostrar_estado(o, p);

    while (o->estado == MISION_EN_CURSO) {
        printf("\n[NAVEGACION]  1) Avanzar tiempo   2) Ver estado   3) Ver historial   4) Abortar\n");
        if (!leer_entero("Opcion: ", 1, 4, &opcion)) {
            abortar_mision(o, h);
            break;
        }
        switch (opcion) {
            case 1:
                if (!leer_entero("Horas a avanzar (1 - 48): ", HORAS_MIN, HORAS_MAX, &horas)) {
                    abortar_mision(o, h);
                    break;
                }
                navegar(o, p, (double)horas, h);
                mostrar_estado(o, p);
                break;
            case 2:
                mostrar_estado(o, p);
                break;
            case 3:
                mostrar_historial(h);
                break;
            case 4:
                abortar_mision(o, h);
                break;
        }
    }

    mostrar_reporte_final(o, p, h);
    return 1;
}

/* Mision completa sin preguntar nada: TLI, desacople y navegacion hasta que termine
 * (o hasta HORAS_TOPE_SIM). Se usa en el barrido y en los escenarios. */
void simular_automatico(const ParametrosMision *p, Resultado *res)
{
    Orion o;
    Historial h;

    h.cantidad = 0;
    iniciar_orion(&o, p);
    ejecutar_tli(&o, p, &h);
    desacoplar_icps(&o, &h);
    navegar(&o, p, HORAS_TOPE_SIM, &h);

    res->params = *p;
    res->estado = o.estado;
    res->r_max  = o.r_max;
    res->t_fin  = o.t;
    res->w_ext  = trabajo_adicional_esp(radio_desde_altitud(p->altitud_leo), p->v_tli, p->dist_tl);
}
