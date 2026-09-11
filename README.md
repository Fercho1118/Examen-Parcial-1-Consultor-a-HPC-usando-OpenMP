# Los Paralelos

Consultora HPC para el Examen Parcial 1 de Computación Paralela y Distribuida (CC3069, UVG).
Integrantes:

- Camila Richter (23183)
- Fernando Rueda (23748)
- Sebastián García (22291)

## De qué se trata

De los tres problemas que nos tocaron elegimos paralelizar dos con OpenMP: la Suma de Riemann
(integración numérica) y el Histograma de temperaturas (con un Merge Sort previo). Dejamos fuera el
de Ruta Mínima en Grafos porque era el de paralelización más enredada, con carga irregular y reparto
dinámico.

La idea de la consultora fue tomar cada algoritmo secuencial, pasarlo a paralelo y medir de verdad si
valía la pena, comparando tiempos, speedup y eficiencia.

## Cómo está organizado el repo

- `secuencial/` tiene los programas base en C, sin paralelizar.
- `paralelo/` tiene las versiones optimizadas con OpenMP.
- `docs/` tiene toda la documentación:
  - `Informe General.md`, donde juntamos los tres puntos que pide el examen: contexto y datos,
    estrategia de paralelización, y los resultados de los tres.
  - Un reporte individual por cada quien (`FernandoR_...`, `Informe_Camila`, `SebastianG_...`) con
    sus propias métricas y capturas.
  - `corridas/` con los logs de las ejecuciones, y `bench_suma.sh` / `bench_histograma.sh`, los
    scripts para reproducir las mediciones.
- `images/` junta todas las gráficas y capturas de evidencia.

## Cómo trabajamos

Cada quien clonó el repo, corrió los dos programas en su propia máquina y armó su archivo de
resultados con su speedup, su eficiencia y sus capturas de las corridas. Como cada uno midió en un
equipo distinto, los números no son idénticos, pero apuntan a lo mismo. Al final juntamos todo en el
Informe General, que es el que cuenta la historia completa.

## Cómo reproducir las mediciones

```bash
bash docs/bench_suma.sh          # Suma de Riemann: secuencial vs OpenMP
bash docs/bench_histograma.sh    # Histograma: secuencial vs OpenMP
```
