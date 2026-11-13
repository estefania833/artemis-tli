/* fisica.c - Modelo fisico 1D (movimiento radial sobre la linea Tierra-Luna).
 * Supuestos (documentar en docs/modelo_fisico.md):
 *  - Orion se mueve sobre la recta Tierra-Luna, sin rotacion ni perturbaciones.
 *  - Tierra y Luna fijas, motores apagados tras la TLI (solo gravedad).
 *  - Punto de equilibrio: donde |g_Tierra| = |g_Luna|. */
#include <math.h>
#include "artemis.h"
#include "fisica.h"

double energia_cinetica(double masa, double v)
{
    return 0.5 * masa * v * v;
}

double radio_desde_altitud(double altitud)
{
    return RADIO_TIERRA + altitud;
}

double velocidad_orbital(double r)
{
    return sqrt(G_CONST * MASA_TIERRA / r);
}

/* MT/r^2 = ML/(D-r)^2  =>  r = D / (1 + sqrt(ML/MT)) */
double punto_equilibrio(double dist_tl)
{
    return dist_tl / (1.0 + sqrt(MASA_LUNA / MASA_TIERRA));
}

/* Energia potencial por unidad de masa. con_luna = 0: solo Tierra. */
double potencial_esp(double r, double dist_tl, int con_luna)
{
    double u = -G_CONST * MASA_TIERRA / r;
    if (con_luna) {
        u -= G_CONST * MASA_LUNA / (dist_tl - r);
    }
    return u;
}

double aceleracion_radial(double r, double dist_tl)
{
    double dl = dist_tl - r;
    return -G_CONST * MASA_TIERRA / (r * r) + G_CONST * MASA_LUNA / (dl * dl);
}

/* Trabajo externo minimo por unidad de masa = variacion de energia potencial
 * (la nave llega con velocidad nula): W/m = U(r_fin) - U(r_ini). */
double trabajo_min_esp(double r_ini, double r_fin, double dist_tl, int con_luna)
{
    return potencial_esp(r_fin, dist_tl, con_luna) - potencial_esp(r_ini, dist_tl, con_luna);
}

/* 1/2 v^2 = W/m  =>  v = sqrt(2 W/m) */
double velocidad_minima_tli(double r_leo, double dist_tl)
{
    double w = trabajo_min_esp(r_leo, punto_equilibrio(dist_tl), dist_tl, 1);
    return sqrt(2.0 * w);
}

double energia_escape_esp(double gm, double radio)
{
    return gm / radio;
}

/* Un paso de velocity-Verlet (orden 2, conserva bien la energia). */
void avanzar_verlet(double *r, double *v, double dt, double dist_tl)
{
    double a0 = aceleracion_radial(*r, dist_tl);
    double a1;

    *r += (*v) * dt + 0.5 * a0 * dt * dt;
    a1 = aceleracion_radial(*r, dist_tl);
    *v += 0.5 * (a0 + a1) * dt;
}

double gravedad_tierra(double r)
{
    return G_CONST * MASA_TIERRA / (r * r);
}

double gravedad_luna(double r, double dist_tl)
{
    double dl = dist_tl - r;
    return G_CONST * MASA_LUNA / (dl * dl);
}

/* Conservacion de la energia mecanica entre r0 y r (motores apagados):
 * 1/2 v^2 + U(r) = 1/2 v0^2 + U(r0)  =>  v = sqrt(v0^2 + 2 (U(r0) - U(r))).
 * Devuelve 0 (y *v = 0) si v^2 < 0: la nave no llega a esa distancia. */
int velocidad_en(double r, double r0, double v0, double dist_tl, double *v)
{
    double v2 = v0 * v0 + 2.0 * (potencial_esp(r0, dist_tl, 1) - potencial_esp(r, dist_tl, 1));

    if (v2 < 0.0) {
        *v = 0.0;
        return 0;
    }
    *v = sqrt(v2);
    return 1;
}

double velocidad_gases(double isp)
{
    return isp * G0_ESTANDAR;
}

/* Ecuacion de Tsiolkovsky: dv = ve ln(m0/mf)  =>  m_comb = m0 - mf = mf (e^(dv/ve) - 1) */
double masa_combustible(double masa_final, double dv, double isp)
{
    return masa_final * (exp(dv / velocidad_gases(isp)) - 1.0);
}

double energia_mecanica_esp(double r, double v, double dist_tl)
{
    return 0.5 * v * v + potencial_esp(r, dist_tl, 1);
}

/* Wext/m = Em_final/m - Em_inicial/m = U(r_eq) - [1/2 v_TLI^2 + U(r_LEO)] */
double trabajo_adicional_esp(double r_leo, double v_tli, double dist_tl)
{
    return potencial_esp(punto_equilibrio(dist_tl), dist_tl, 1)
         - energia_mecanica_esp(r_leo, v_tli, dist_tl);
}
