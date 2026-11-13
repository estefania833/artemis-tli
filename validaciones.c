/* validaciones.c - Lectura con fgets + strtol/strtod. Nunca usa scanf directo,
 * por lo que letras, vacios, desbordes y EOF no rompen el programa. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "validaciones.h"

#define TAM_LINEA 128

/* Lee una linea completa; descarta lo que exceda el buffer. 0 = EOF. */
int leer_linea(char *buffer, size_t tam)
{
    size_t len;
    int c;

    if (fgets(buffer, (int)tam, stdin) == NULL) {
        return 0;
    }
    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((c = getchar()) != '\n' && c != EOF) {
            /* descartar resto de linea demasiado larga */
        }
    }
    return 1;
}

static int solo_espacios(const char *s)
{
    while (*s != '\0') {
        if (!isspace((unsigned char)*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}

int leer_entero(const char *msg, int min, int max, int *destino)
{
    char linea[TAM_LINEA];
    char *fin;
    long valor;

    for (;;) {
        printf("%s", msg);
        if (!leer_linea(linea, sizeof linea)) {
            return 0;
        }
        if (solo_espacios(linea)) {
            printf("  [!] No ingreso nada. Intente de nuevo.\n");
            continue;
        }
        errno = 0;
        valor = strtol(linea, &fin, 10);
        if (fin == linea || !solo_espacios(fin) || errno == ERANGE) {
            printf("  [!] Entrada invalida: ingrese un numero entero.\n");
            continue;
        }
        if (valor < min || valor > max) {
            printf("  [!] Fuera de rango: debe estar entre %d y %d.\n", min, max);
            continue;
        }
        *destino = (int)valor;
        return 1;
    }
}

int leer_double(const char *msg, double min, double max, double *destino)
{
    char linea[TAM_LINEA];
    char *fin;
    char *coma;
    double valor;

    for (;;) {
        printf("%s", msg);
        if (!leer_linea(linea, sizeof linea)) {
            return 0;
        }
        if (solo_espacios(linea)) {
            printf("  [!] No ingreso nada. Intente de nuevo.\n");
            continue;
        }
        coma = strchr(linea, ',');          /* aceptar coma decimal */
        if (coma != NULL) {
            *coma = '.';
        }
        errno = 0;
        valor = strtod(linea, &fin);
        if (fin == linea || !solo_espacios(fin) || errno == ERANGE || !isfinite(valor)) {
            printf("  [!] Entrada invalida: ingrese un numero (ej: 10900 o 1.09e4).\n");
            continue;
        }
        if (valor < min || valor > max) {
            printf("  [!] Fuera de rango: debe estar entre %.6g y %.6g.\n", min, max);
            continue;
        }
        *destino = valor;
        return 1;
    }
}

int leer_si_no(const char *msg, int *destino)
{
    char linea[TAM_LINEA];

    for (;;) {
        printf("%s", msg);
        if (!leer_linea(linea, sizeof linea)) {
            return 0;
        }
        if (strlen(linea) == 1 && (tolower((unsigned char)linea[0]) == 's')) {
            *destino = 1;
            return 1;
        }
        if (strlen(linea) == 1 && (tolower((unsigned char)linea[0]) == 'n')) {
            *destino = 0;
            return 1;
        }
        printf("  [!] Responda con 's' o 'n'.\n");
    }
}

void esperar_enter(void)
{
    char linea[TAM_LINEA];

    printf("\nPresione ENTER para continuar...");
    leer_linea(linea, sizeof linea);
}

/* 1 si 's' termina en 'ext' (sin distinguir mayusculas: Windows no las distingue). */
static int termina_en(const char *s, const char *ext)
{
    size_t ls = strlen(s);
    size_t le = strlen(ext);
    size_t i;

    if (ls < le) {
        return 0;
    }
    for (i = 0; i < le; i++) {
        if (tolower((unsigned char)s[ls - le + i]) != tolower((unsigned char)ext[i])) {
            return 0;
        }
    }
    return 1;
}

static int caracteres_validos(const char *s)
{
    while (*s != '\0') {
        if (!isalnum((unsigned char)*s) && *s != '_' && *s != '-' && *s != '.') {
            return 0;
        }
        s++;
    }
    return 1;
}

int leer_nombre_archivo(const char *msg, const char *por_defecto, const char *extension,
                        char *destino, size_t tam)
{
    char linea[TAM_LINEA];
    size_t max_base = tam - strlen(extension) - 1;   /* lugar para agregar la extension */

    for (;;) {
        printf("%s", msg);
        if (!leer_linea(linea, sizeof linea)) {
            return 0;
        }
        if (solo_espacios(linea)) {
            snprintf(destino, tam, "%s", por_defecto);
            return 1;
        }
        if (!caracteres_validos(linea)) {
            printf("  [!] Use solo letras, numeros, '_', '-' o '.' (sin espacios ni carpetas).\n");
            continue;
        }
        if (strlen(linea) > max_base) {
            printf("  [!] Nombre demasiado largo (maximo %d caracteres).\n", (int)max_base);
            continue;
        }
        if (termina_en(linea, extension)) {
            snprintf(destino, tam, "%s", linea);
        } else {
            snprintf(destino, tam, "%s%s", linea, extension);
        }
        return 1;
    }
}
