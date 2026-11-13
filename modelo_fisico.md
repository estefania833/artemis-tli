# Modelo fisico - ARTEMIS TLI (borrador para acordar con el equipo / catedra)

## Supuestos
- Movimiento 1D radial sobre la recta Tierra-Luna. Tierra y Luna fijas, sin rotacion ni perturbaciones.
- Tras la TLI los motores quedan apagados: solo actua la gravedad (Tierra + Luna).
- Integracion numerica: velocity-Verlet, dt = 10 s.
- **Datos de la Luna (MASA_LUNA, RADIO_LUNA) NO estan en la consigna**: valores estandar. Confirmar con la catedra.

## Formulas
- r_LEO = RT + h
- K = 1/2 m v^2  (referencia: 1.54453e12 J)
- Punto de equilibrio (g_T = g_L): r_eq = D / (1 + sqrt(ML/MT))  ~ 345 672 km desde la Tierra
- U/m = -G MT / r - G ML / (D - r)
- Trabajo minimo por unidad de masa: W/m = U(r_eq) - U(r_LEO)   (llega con v ~ 0)
- v_min = sqrt(2 W/m)
- Aceleracion radial: a(r) = -G MT / r^2 + G ML / (D - r)^2
- Velocidad a distancia r (motores apagados): v(r) = sqrt(v0^2 + 2 (U(r0) - U(r)))  (tabla de la pregunta A)
- Velocidad orbital en LEO: v_orb = sqrt(G MT / r_LEO) ~ 7 793 m/s

## Pregunta C: trabajo adicional minimo (Em = K + U)
- Em_inicial/m = 1/2 v_TLI^2 + U(r_LEO)   (justo despues de la TLI)
- Em_final/m   = U(r_eq)                  (minimo: llega con v = 0)
- Wext/m = Em_final/m - Em_inicial/m = dU - K/m = 5.9462e7 - 5.9405e7 = 5.715e4 J/kg
- Si da <= 0, Wext = 0: la TLI alcanza por si sola.

## Pregunta D: combustible (Tsiolkovsky)
- **Isp NO esta en la consigna**: por defecto 462 s (motor RL10 de la ICPS). Configurable (200 - 500 s).
- ve = Isp g0;  m_comb = m_final (e^(dv/ve) - 1), con m_final = masa de Orion
- dv ideal = sqrt(2 W/m) desde la superficie hasta el equilibrio (sin perdidas por atmosfera/gravedad)
- Con los datos por defecto: ida dv ~ 11 072 m/s -> ~273 400 kg; vuelta dv ~ 2 272 m/s -> ~16 900 kg
  Energia ida/vuelta ~ 23.7 veces, combustible ida/vuelta ~ 16.1 veces.
- Combustible de la TLI sola (dv = v_TLI - v_orb ~ 3 107 m/s): ~25 600 kg.

## Resultado a discutir
Con los datos por defecto (v = 10 900 m/s) K/m = 5.9405e7 J/kg < W/m = 5.9462e7 J/kg:
en este modelo Orion NO llega al equilibrio (apogeo ~ 313 400 km). Hace falta v >= ~10 905 m/s.
Con v = 10 950 m/s si llega.

## Simulacion automatica (barrido, escenarios, CSV)
- Misma fisica que la mision interactiva (TLI -> desacople -> navegar con Verlet).
- Termina al llegar al equilibrio (EXITOSA), en el apogeo (RETORNO) o a las 720 h (tope de seguridad).
- La columna Em del CSV de trayectoria deberia quedar casi constante: comprueba la conservacion
  de la energia mecanica y la precision del integrador.

## Formatos de archivo
- CSV: separador ';' y coma decimal (se abre directo en Excel configurado en espanol).
- Parametros: texto clave=valor, lineas con '#' son comentarios. Ejemplo:
  ```
  # ARTEMIS TLI - parametros de mision (unidades SI)
  masa=26000
  v_tli=10900
  dist_tl=384000000
  altitud_leo=192000
  isp=462
  ```
  Al cargar se exigen las 5 claves y que esten dentro de los rangos validos; si no, no se modifica nada.
