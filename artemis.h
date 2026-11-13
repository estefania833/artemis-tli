/* artemis.h - Tipos compartidos, constantes y prototipos de la logica de mision.
 * Escenario: Problema 3 - Inyeccion Translunar (TLI) de Orion. */
#ifndef ARTEMIS_H
#define ARTEMIS_H

/* ===== Constantes fisicas (SI) ===== */
#define G_CONST       6.674e-11      /* N m^2 / kg^2   (consigna) */
#define MASA_TIERRA   5.972e24       /* kg             (consigna) */
#define RADIO_TIERRA  6.371e6        /* m              (consigna) */
/* OJO: la consigna NO da datos de la Luna. Valores estandar -> confirmar con la catedra */
#define MASA_LUNA     7.342e22       /* kg */
#define RADIO_LUNA    1.7374e6       /* m  */
#define G0_ESTANDAR   9.80665        /* m/s^2: gravedad estandar (Isp -> velocidad de escape de gases) */

/* ===== Valores por defecto de la consigna (Problema 3) ===== */
#define MASA_DEF      26000.0        /* kg */
#define V_TLI_DEF     10900.0        /* m/s */
#define DIST_TL_DEF   384000000.0    /* m  */
#define ALT_LEO_DEF   192000.0       /* m  */
/* OJO: la consigna NO da el Isp. 462 s ~ motor RL10 de la ICPS -> confirmar con la catedra */
#define ISP_DEF       462.0          /* s  */

/* ===== Rangos validos de entrada ===== */
#define MASA_MIN      1000.0
#define MASA_MAX      100000.0
#define V_TLI_MIN     7000.0
#define V_TLI_MAX     15000.0
#define DIST_TL_MIN   350000000.0
#define DIST_TL_MAX   420000000.0
#define ALT_LEO_MIN   160000.0
#define ALT_LEO_MAX   2000000.0
#define ISP_MIN       200.0
#define ISP_MAX       500.0

/* ===== Simulacion ===== */
#define DT_SIM        10.0           /* paso de integracion [s] */
#define HORAS_MIN     1
#define HORAS_MAX     48
#define MAX_EVENTOS   64
#define TAM_DESC      96
#define MAX_FILAS_A   10             /* filas de la tabla de desaceleracion (A) */
#define HORAS_TOPE_SIM 720.0         /* tope de la simulacion automatica (30 dias) */

/* ===== Escenarios y archivos ===== */
#define MAX_ESCENARIOS   50          /* tambien es el maximo de puntos de un barrido */
#define INTERVALO_CSV_S  600.0       /* una fila de trayectoria cada 10 min [s] */
#define TAM_NOMBRE_ARCH  64

typedef struct {
    double masa;        /* kg */
    double v_tli;       /* m/s */
    double dist_tl;     /* m: distancia Tierra-Luna */
    double altitud_leo; /* m */
    double isp;         /* s: impulso especifico del motor (para Tsiolkovsky, D) */
} ParametrosMision;

typedef enum {
    ETAPA_LEO = 0,
    ETAPA_TLI,
    ETAPA_DESACOPLE,
    ETAPA_NAVEGACION,
    ETAPA_FINALIZADA
} EtapaMision;

typedef enum {
    MISION_EN_CURSO = 0,
    MISION_EXITOSA,     /* alcanza el punto de equilibrio Tierra-Luna */
    MISION_RETORNO,     /* no alcanza el equilibrio: vuelve a la Tierra */
    MISION_ABORTADA
} EstadoMision;

typedef struct {
    double masa;                /* kg */
    double r;                   /* distancia al centro de la Tierra [m] */
    double v;                   /* velocidad radial [m/s] */
    double t;                   /* tiempo de mision desde la TLI [s] */
    double r_max;               /* maxima distancia alcanzada [m] */
    int    icps_acoplada;       /* 1 = acoplada */
    int    motores_encendidos;  /* 1 = encendidos */
    int    ultimo_hito;         /* ultimo multiplo de 100 000 km informado */
    EtapaMision etapa;
    EstadoMision estado;
} Orion;

typedef struct {
    double tiempo;              /* s */
    EtapaMision etapa;
    char descripcion[TAM_DESC];
} Evento;

typedef struct {
    Evento eventos[MAX_EVENTOS];
    int cantidad;
} Historial;

/* Una fila de la tabla de desaceleracion progresiva (A) */
typedef struct {
    double r;               /* m: distancia al centro de la Tierra */
    double g_tierra;        /* m/s^2: atraccion terrestre */
    double g_luna;          /* m/s^2: atraccion lunar */
    double a_neta;          /* m/s^2: aceleracion neta (- = hacia la Tierra) */
    double v;               /* m/s: velocidad por conservacion de energia */
    int    alcanza;         /* 1 = llega a esa distancia, 0 = el apogeo queda antes */
} FilaInercia;

/* Resultados de los analisis A-D */
typedef struct {
    /* (A) desaceleracion progresiva */
    FilaInercia tabla[MAX_FILAS_A];
    int    filas;
    /* (B) */
    double r_leo;           /* m */
    double k_inicial;       /* J */
    double k_esp;           /* J/kg */
    /* (C) */
    double r_eq;            /* m: punto de equilibrio medido desde la Tierra */
    double w_tierra;        /* J/kg: LEO -> equilibrio, solo potencial terrestre */
    double w_total;         /* J/kg: dU = U(r_eq) - U(r_LEO), Tierra + Luna */
    double u_leo;           /* J/kg: U(r_LEO) */
    double u_eq;            /* J/kg: U(r_eq) */
    double em_inicial;      /* J/kg: 1/2 v_TLI^2 + U(r_LEO) (justo despues de la TLI) */
    double em_final;        /* J/kg: U(r_eq) (llega con v = 0) */
    double w_ext;           /* J/kg: em_final - em_inicial (>0: falta trabajo, <=0: sobra) */
    double v_min_tli;       /* m/s: velocidad minima para llegar al equilibrio */
    double v_orb_leo;       /* m/s: velocidad circular en LEO (para el dv de la TLI, D) */
    /* (D) */
    double w_tierra_eq;     /* J/kg: superficie terrestre -> equilibrio */
    double w_luna_eq;       /* J/kg: superficie lunar -> equilibrio */
    double cociente_tl_lt;  /* w_tierra_eq / w_luna_eq */
    double e_escape_tierra; /* J/kg: desde superficie terrestre */
    double e_escape_luna;   /* J/kg: desde superficie lunar */
    double v_gases;         /* m/s: velocidad de escape de gases = Isp * g0 */
    double dv_ida;          /* m/s: delta-v ideal superficie terrestre -> equilibrio */
    double dv_vuelta;       /* m/s: delta-v ideal superficie lunar -> equilibrio */
    double comb_ida;        /* kg: combustible para la ida (Tsiolkovsky) */
    double comb_vuelta;     /* kg: combustible para la vuelta */
    double cociente_comb;   /* comb_ida / comb_vuelta */
    double dv_tli;          /* m/s: delta-v de la maniobra TLI (v_tli - v_orb_leo) */
    double comb_tli;        /* kg: combustible que gasta la ICPS en la TLI */
} Analisis;

/* Resultado de una mision simulada de forma automatica (barrido / escenarios) */
typedef struct {
    ParametrosMision params;
    EstadoMision estado;    /* EXITOSA, RETORNO o EN_CURSO si se llego al tope de tiempo */
    double r_max;           /* m: maxima distancia a la Tierra */
    double t_fin;           /* s: tiempo hasta el equilibrio o hasta el apogeo */
    double w_ext;           /* J/kg: trabajo adicional minimo (C) */
} Resultado;

typedef struct {
    Resultado items[MAX_ESCENARIOS];
    int cantidad;
} ListaEscenarios;

typedef enum {
    ORDEN_VELOCIDAD = 1,
    ORDEN_DISTANCIA,
    ORDEN_TIEMPO,
    ORDEN_TRABAJO
} CriterioOrden;

/* ===== Logica de mision (artemis.c) ===== */
void        params_por_defecto(ParametrosMision *p);
int         parametros_validos(const ParametrosMision *p);
void        configurar_parametros(ParametrosMision *p);
void        calcular_analisis(const ParametrosMision *p, Analisis *a);
int         registrar_evento(Historial *h, double t, EtapaMision etapa, const char *desc);
void        iniciar_orion(Orion *o, const ParametrosMision *p);
void        ejecutar_tli(Orion *o, const ParametrosMision *p, Historial *h);
void        desacoplar_icps(Orion *o, Historial *h);
int         navegar(Orion *o, const ParametrosMision *p, double horas, Historial *h);
void        abortar_mision(Orion *o, Historial *h);
int         simular_mision(Orion *o, const ParametrosMision *p, Historial *h);
void        simular_automatico(const ParametrosMision *p, Resultado *res);
const char *nombre_etapa(EtapaMision e);
const char *nombre_estado(EstadoMision s);
const char *nombre_estado_corto(EstadoMision s);

#endif
