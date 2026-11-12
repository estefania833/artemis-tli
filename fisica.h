/* fisica.h - Funciones fisicas puras (sin entrada/salida por pantalla). */
#ifndef FISICA_H
#define FISICA_H

double energia_cinetica(double masa, double v);                 /* K = 1/2 m v^2  [J]   */
double radio_desde_altitud(double altitud);                     /* r = RT + h     [m]   */
double velocidad_orbital(double r);                             /* circular       [m/s] */
double punto_equilibrio(double dist_tl);                        /* desde la Tierra [m]  */
double potencial_esp(double r, double dist_tl, int con_luna);   /* U/m            [J/kg]*/
double aceleracion_radial(double r, double dist_tl);            /* a(r), + = alejarse   */
double trabajo_min_esp(double r_ini, double r_fin, double dist_tl, int con_luna); /* J/kg */
double velocidad_minima_tli(double r_leo, double dist_tl);      /* llegar al equilibrio */
double energia_escape_esp(double gm, double radio);             /* GM/R           [J/kg]*/
void   avanzar_verlet(double *r, double *v, double dt, double dist_tl); /* modifica r, v */
double gravedad_tierra(double r);                               /* |g_T| = GMT/r^2 [m/s^2] */
double gravedad_luna(double r, double dist_tl);                 /* |g_L| = GML/(D-r)^2     */
int    velocidad_en(double r, double r0, double v0, double dist_tl, double *v); /* 1 = alcanza */
double velocidad_gases(double isp);                             /* ve = Isp g0    [m/s] */
double masa_combustible(double masa_final, double dv, double isp); /* Tsiolkovsky [kg]  */
double energia_mecanica_esp(double r, double v, double dist_tl);   /* Em/m = 1/2 v^2 + U/m */
double trabajo_adicional_esp(double r_leo, double v_tli, double dist_tl); /* Wext/m (C) */

#endif
