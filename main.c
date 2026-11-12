/* main.c - Menu principal y flujo general. */
#include <stdio.h>
#include "artemis.h"
#include "validaciones.h"
#include "reportes.h"
#include "escenarios.h"
#include "archivos.h"

/* Pide el nombre de un archivo de salida y, si ya existe, confirma si se sobrescribe.
 * Devuelve 1 si se puede continuar. */
static int pedir_archivo_salida(const char *por_defecto, const char *extension, char *nombre)
{
    char msg[96];
    int sobrescribir;

    snprintf(msg, sizeof msg, "  Nombre del archivo (ENTER = %s): ", por_defecto);
    if (!leer_nombre_archivo(msg, por_defecto, extension, nombre, TAM_NOMBRE_ARCH)) {
        return 0;
    }
    if (archivo_existe(nombre)) {
        printf("  [!] '%s' ya existe.\n", nombre);
        if (!leer_si_no("  Sobrescribir? (s/n): ", &sobrescribir) || !sobrescribir) {
            printf("  Operacion cancelada.\n");
            return 0;
        }
    }
    return 1;
}

static void informar(EstadoArchivo e, const char *nombre)
{
    if (e == ARCH_OK) {
        printf("\n  Listo: '%s'.\n", nombre);
    } else {
        printf("\n  [!] '%s': %s\n", nombre, mensaje_archivo(e));
    }
}

static void menu_archivos(ParametrosMision *params, const Orion *orion,
                          const ParametrosMision *params_mision, const Historial *historial,
                          int hay_mision, const ListaEscenarios *escenarios)
{
    int opcion, filas;
    char nombre[TAM_NOMBRE_ARCH];
    Analisis analisis;
    EstadoArchivo e;

    mostrar_encabezado("ARCHIVOS - EXPORTAR / GUARDAR / CARGAR");
    printf("  1) Exportar trayectoria a CSV (simula con los parametros actuales)\n");
    printf("  2) Guardar reporte en TXT (analisis A-D + ultima mision + escenarios)\n");
    printf("  3) Exportar escenarios a CSV\n");
    printf("  4) Guardar parametros actuales\n");
    printf("  5) Cargar parametros desde archivo\n");
    printf("  0) Volver\n\n");

    if (!leer_entero("Opcion: ", 0, 5, &opcion)) {
        return;
    }
    switch (opcion) {
        case 1:
            if (!pedir_archivo_salida("trayectoria.csv", ".csv", nombre)) break;
            e = exportar_trayectoria_csv(nombre, params, &filas);
            informar(e, nombre);
            if (e == ARCH_OK) {
                printf("  %d filas (una cada %.0f min). Columnas: t, r, altitud, distancia\n",
                       filas, INTERVALO_CSV_S / 60.0);
                printf("  a la Luna, v, a, Em. Em casi constante = energia conservada.\n");
            }
            break;
        case 2:
            if (!pedir_archivo_salida("reporte.txt", ".txt", nombre)) break;
            calcular_analisis(params, &analisis);
            e = guardar_reporte_txt(nombre, params, &analisis,
                                    hay_mision ? orion : NULL, params_mision,
                                    historial, escenarios);
            informar(e, nombre);
            break;
        case 3:
            if (escenarios->cantidad == 0) {
                printf("\n  [!] No hay escenarios cargados (use Barrido o Escenarios).\n");
                break;
            }
            if (!pedir_archivo_salida("escenarios.csv", ".csv", nombre)) break;
            e = exportar_escenarios_csv(nombre, escenarios);
            informar(e, nombre);
            break;
        case 4:
            if (!pedir_archivo_salida("parametros.txt", ".txt", nombre)) break;
            e = guardar_parametros(nombre, params);
            informar(e, nombre);
            break;
        case 5:
            if (!leer_nombre_archivo("  Archivo a cargar (ENTER = parametros.txt): ",
                                     "parametros.txt", ".txt", nombre, sizeof nombre)) break;
            e = cargar_parametros(nombre, params);
            informar(e, nombre);
            if (e == ARCH_OK) {
                printf("\n");
                mostrar_parametros(params);
            } else {
                printf("  Los parametros actuales no se modificaron.\n");
            }
            break;
        case 0:
            break;
    }
}

int main(void)
{
    ParametrosMision params;
    ParametrosMision params_mision;     /* parametros con los que se corrio la ultima mision */
    Orion orion;
    Historial historial;
    Analisis analisis;
    ListaEscenarios escenarios;
    int opcion = -1;
    int hay_mision = 0;

    params_por_defecto(&params);
    params_mision = params;
    historial.cantidad = 0;
    escenarios.cantidad = 0;

    while (opcion != 0) {
        mostrar_encabezado("ARTEMIS TLI - Simulador de Inyeccion Translunar");
        printf("  1) Configurar parametros de mision\n");
        printf("  2) Iniciar mision\n");
        printf("  3) Analisis fisico A-D\n");
        printf("  4) Ver ultimo reporte / historial\n");
        printf("  5) Barrido de velocidades de inyeccion\n");
        printf("  6) Escenarios (comparar / ordenar / buscar)\n");
        printf("  7) Archivos (CSV / TXT / parametros)\n");
        printf("  0) Salir\n\n");

        if (!leer_entero("Opcion: ", 0, 7, &opcion)) {
            printf("\nFin de entrada. Saliendo.\n");
            break;
        }

        switch (opcion) {
            case 1:
                configurar_parametros(&params);
                break;
            case 2:
                if (simular_mision(&orion, &params, &historial)) {
                    hay_mision = 1;
                    params_mision = params;
                }
                break;
            case 3:
                calcular_analisis(&params, &analisis);
                mostrar_analisis(&params, &analisis);
                break;
            case 4:
                if (hay_mision) {
                    mostrar_reporte_final(&orion, &params_mision, &historial);
                } else {
                    printf("\n  [!] Todavia no se ejecuto ninguna mision.\n");
                }
                break;
            case 5:
                barrido_velocidades(&params, &escenarios);
                break;
            case 6:
                menu_escenarios(&escenarios, &params);
                break;
            case 7:
                menu_archivos(&params, &orion, &params_mision, &historial, hay_mision, &escenarios);
                break;
            case 0:
                printf("\nMision finalizada. Hasta la proxima.\n");
                break;
        }
        if (opcion != 0) {
            esperar_enter();
        }
    }
    return 0;
}
