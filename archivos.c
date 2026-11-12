/* archivos.c - Manejo de archivos con fopen/fprintf/fgets.
 * Siempre se verifica la apertura, la escritura (ferror) y el cierre (fclose). */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "artemis.h"
#include "fisica.h"
#include "reportes.h"
#include "archivos.h"

#define TAM_LINEA_ARCH 128
#define TAM_CLAVE      32

/* Bits para controlar que el archivo de parametros traiga todas las claves */
#define BIT_MASA   1
#define BIT_VTLI   2
#define BIT_DIST   4
#define BIT_ALT    8
#define BIT_ISP    16
#define BITS_TODOS (BIT_MASA | BIT_VTLI | BIT_DIST | BIT_ALT | BIT_ISP)

int archivo_existe(const char *nombre)
{
    FILE *f = fopen(nombre, "r");

    if (f == NULL) {
        return 0;
    }
    fclose(f);
    return 1;
}

const char *mensaje_archivo(EstadoArchivo e)
{
    switch (e) {
        case ARCH_OK:             return "Operacion realizada correctamente.";
        case ARCH_ERROR_ABRIR:    return "No se pudo abrir o crear el archivo.";
        case ARCH_ERROR_ESCRIBIR: return "Error al escribir el archivo (disco lleno o sin permiso?).";
        case ARCH_ERROR_FORMATO:  return "Formato invalido o faltan datos en el archivo.";
        case ARCH_ERROR_RANGO:    return "Hay valores fuera de los rangos validos.";
    }
    return "Error desconocido.";
}

/* Cierra el archivo y devuelve si hubo error de escritura en algun momento. */
static EstadoArchivo cerrar(FILE *f)
{
    int hubo_error = ferror(f);

    if (fclose(f) != 0 || hubo_error) {
        return ARCH_ERROR_ESCRIBIR;
    }
    return ARCH_OK;
}

/* Escribe un numero con coma decimal, seguido de ';' o de fin de linea. */
static void campo_csv(FILE *f, double x, int ultimo)
{
    char buf[40];
    char *c;

    snprintf(buf, sizeof buf, "%.10g", x);
    for (c = buf; *c != '\0'; c++) {
        if (*c == '.') {
            *c = ',';
        }
    }
    fprintf(f, "%s%c", buf, ultimo ? '\n' : ';');
}

static void fila_trayectoria(FILE *f, const Orion *o, const ParametrosMision *p)
{
    campo_csv(f, o->t / 3600.0, 0);
    campo_csv(f, o->r / 1000.0, 0);
    campo_csv(f, (o->r - RADIO_TIERRA) / 1000.0, 0);
    campo_csv(f, (p->dist_tl - o->r) / 1000.0, 0);
    campo_csv(f, o->v, 0);
    campo_csv(f, aceleracion_radial(o->r, p->dist_tl), 0);
    campo_csv(f, energia_mecanica_esp(o->r, o->v, p->dist_tl), 1);
}

/* Simula la mision completa con los parametros dados y guarda una fila cada
 * INTERVALO_CSV_S segundos. La columna Em permite ver que la energia se conserva. */
EstadoArchivo exportar_trayectoria_csv(const char *nombre, const ParametrosMision *p, int *filas)
{
    FILE *f;
    Orion o;
    Historial h;

    *filas = 0;
    f = fopen(nombre, "w");
    if (f == NULL) {
        return ARCH_ERROR_ABRIR;
    }

    h.cantidad = 0;
    iniciar_orion(&o, p);
    ejecutar_tli(&o, p, &h);
    desacoplar_icps(&o, &h);

    fprintf(f, "t_h;r_km;altitud_km;dist_luna_km;v_m_s;a_m_s2;Em_J_kg\n");
    fila_trayectoria(f, &o, p);
    (*filas)++;
    while (o.estado == MISION_EN_CURSO && o.t < HORAS_TOPE_SIM * 3600.0) {
        navegar(&o, p, INTERVALO_CSV_S / 3600.0, &h);
        fila_trayectoria(f, &o, p);
        (*filas)++;
    }
    return cerrar(f);
}

EstadoArchivo exportar_escenarios_csv(const char *nombre, const ListaEscenarios *l)
{
    FILE *f;
    int i;
    const Resultado *r;

    f = fopen(nombre, "w");
    if (f == NULL) {
        return ARCH_ERROR_ABRIR;
    }
    fprintf(f, "n;v_tli_m_s;masa_kg;altitud_leo_km;dist_tl_km;isp_s;resultado;"
               "dist_max_km;tiempo_h;wext_J_kg\n");
    for (i = 0; i < l->cantidad; i++) {
        r = &l->items[i];
        fprintf(f, "%d;", i + 1);
        campo_csv(f, r->params.v_tli, 0);
        campo_csv(f, r->params.masa, 0);
        campo_csv(f, r->params.altitud_leo / 1000.0, 0);
        campo_csv(f, r->params.dist_tl / 1000.0, 0);
        campo_csv(f, r->params.isp, 0);
        fprintf(f, "%s;", nombre_estado_corto(r->estado));
        campo_csv(f, r->r_max / 1000.0, 0);
        campo_csv(f, r->t_fin / 3600.0, 0);
        campo_csv(f, r->w_ext, 1);
    }
    return cerrar(f);
}

EstadoArchivo guardar_reporte_txt(const char *nombre, const ParametrosMision *p, const Analisis *a,
                                  const Orion *o, const ParametrosMision *p_mision,
                                  const Historial *h, const ListaEscenarios *l)
{
    FILE *f;
    time_t ahora = time(NULL);
    struct tm *local = NULL;
    char fecha[32] = "(fecha no disponible)";

    f = fopen(nombre, "w");
    if (f == NULL) {
        return ARCH_ERROR_ABRIR;
    }
    if (ahora != (time_t)-1) {
        local = localtime(&ahora);
    }
    if (local != NULL) {
        strftime(fecha, sizeof fecha, "%Y-%m-%d %H:%M", local);
    }

    escribir_encabezado(f, "ARTEMIS TLI - REPORTE GUARDADO");
    fprintf(f, "  Generado: %s\n", fecha);

    escribir_analisis(f, p, a);

    if (o != NULL) {
        escribir_reporte_final(f, o, p_mision, h);
    } else {
        fprintf(f, "\n  (todavia no se ejecuto ninguna mision interactiva)\n");
    }

    if (l->cantidad > 0) {
        escribir_encabezado(f, "ESCENARIOS");
        escribir_tabla_escenarios(f, l);
    }
    return cerrar(f);
}

EstadoArchivo guardar_parametros(const char *nombre, const ParametrosMision *p)
{
    FILE *f = fopen(nombre, "w");

    if (f == NULL) {
        return ARCH_ERROR_ABRIR;
    }
    fprintf(f, "# ARTEMIS TLI - parametros de mision (unidades SI)\n");
    fprintf(f, "masa=%.10g\n", p->masa);
    fprintf(f, "v_tli=%.10g\n", p->v_tli);
    fprintf(f, "dist_tl=%.10g\n", p->dist_tl);
    fprintf(f, "altitud_leo=%.10g\n", p->altitud_leo);
    fprintf(f, "isp=%.10g\n", p->isp);
    return cerrar(f);
}

static int linea_vacia_o_comentario(const char *s)
{
    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }
    return *s == '\0' || *s == '#';
}

/* Lee un archivo clave=valor. Solo modifica *p si el archivo es completo y valido. */
EstadoArchivo cargar_parametros(const char *nombre, ParametrosMision *p)
{
    FILE *f;
    ParametrosMision nuevo = *p;
    char linea[TAM_LINEA_ARCH];
    char clave[TAM_CLAVE];
    double valor;
    int usados, leidos = 0;
    EstadoArchivo estado = ARCH_OK;

    f = fopen(nombre, "r");
    if (f == NULL) {
        return ARCH_ERROR_ABRIR;
    }

    while (estado == ARCH_OK && fgets(linea, sizeof linea, f) != NULL) {
        if (strchr(linea, '\n') == NULL && !feof(f)) {
            estado = ARCH_ERROR_FORMATO;            /* linea demasiado larga */
            break;
        }
        if (linea_vacia_o_comentario(linea)) {
            continue;
        }
        usados = 0;
        if (sscanf(linea, " %31[^= \t] = %lf %n", clave, &valor, &usados) != 2
            || linea[usados] != '\0') {
            estado = ARCH_ERROR_FORMATO;            /* no es clave=numero */
        } else if (strcmp(clave, "masa") == 0) {
            nuevo.masa = valor;        leidos |= BIT_MASA;
        } else if (strcmp(clave, "v_tli") == 0) {
            nuevo.v_tli = valor;       leidos |= BIT_VTLI;
        } else if (strcmp(clave, "dist_tl") == 0) {
            nuevo.dist_tl = valor;     leidos |= BIT_DIST;
        } else if (strcmp(clave, "altitud_leo") == 0) {
            nuevo.altitud_leo = valor; leidos |= BIT_ALT;
        } else if (strcmp(clave, "isp") == 0) {
            nuevo.isp = valor;         leidos |= BIT_ISP;
        } else {
            estado = ARCH_ERROR_FORMATO;            /* clave desconocida */
        }
    }
    fclose(f);

    if (estado != ARCH_OK) {
        return estado;
    }
    if (leidos != BITS_TODOS) {
        return ARCH_ERROR_FORMATO;                  /* falta algun parametro */
    }
    if (!parametros_validos(&nuevo)) {
        return ARCH_ERROR_RANGO;
    }
    *p = nuevo;
    return ARCH_OK;
}
