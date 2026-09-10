# Resultados y Métricas — Fernando Rueda (23748)

**Consultora HPC:** Los Paralelos
**Problema:** Integración Numérica (Suma de Riemann)
**Máquina de pruebas:** Apple M4 Pro — 12 núcleos (8 de rendimiento + 4 de eficiencia), 24 GB RAM
**Compilador:** `clang` + `libomp` (OpenMP), optimización `-O2`
**Tamaño del problema:** n = 10⁹ rectángulos, intervalo [0, π], f(x) = x² + sin(x)

---

## Qué paralelicé y por qué mejora al secuencial

El secuencial recorre los 10⁹ rectángulos en un solo ciclo y va acumulando el área en una
variable (`areaTotal += f(xi) * dx`). Ese es el trabajo que reparto entre varios hilos con
una sola directiva:

```c
#pragma omp parallel for reduction(+:areaTotal) schedule(static)
```

Tomé tres decisiones y cada una tiene una razón concreta:

- **`reduction(+:areaTotal)`** — es el corazón del asunto. Si dejara que todos los hilos
  escribieran directamente sobre `areaTotal` tendría una *race condition* y el resultado
  saldría mal. La reducción le da a cada hilo su propia copia privada, cada uno suma su
  pedazo por separado, y al final OpenMP junta todas las copias en una sola suma. Descarté
  usar `#pragma omp critical` porque serializaría la suma (un hilo a la vez) y mataría el
  speedup.

- **`schedule(static)`** — todos los rectángulos cuestan exactamente lo mismo, así que la
  carga es uniforme. El reparto estático en bloques iguales es lo óptimo aquí; un `dynamic`
  solo agregaría overhead de coordinación sin ningún beneficio.

- **`omp_get_wtime()` en vez de `clock()`** — el secuencial medía con `clock()`, que suma el
  tiempo de CPU de *todos* los núcleos. En paralelo eso daría un "tiempo" que crece con los
  hilos y un speedup falso. Cambié a `omp_get_wtime()`, que mide tiempo real de pared, que es
  lo que de verdad le importa al usuario.

**La correctitud se mantiene:** el área da 12.33542554459… en todas las corridas. Solo cambian
los últimos dígitos según el número de hilos, porque el orden en que se suman los términos
varía en la reducción. Eso es normal en punto flotante y confirma que estoy resolviendo el
mismo problema, no uno distinto.

---

## Metodología de medición

- Cada configuración se corrió **3 veces** y reporto el **mejor tiempo** (reduce el ruido del
  sistema operativo).
- El tiempo base **T₁** es el propio binario paralelo corrido con **1 hilo**, para que T₁ y Tₚ
  se midan exactamente igual (mismo código, mismo `omp_get_wtime()`).
- **Speedup:** S(p) = T₁ / Tₚ  **Eficiencia:** E(p) = S(p) / p × 100 %

---

## Resultados

| Hilos | Tiempo (s) | Speedup (T₁/Tₚ) | Eficiencia |
|:-----:|:----------:|:---------------:|:----------:|
| 1     | 2.0067     | 1.00×           | 100.0 %    |
| 2     | 1.1493     | 1.75×           | 87.3 %     |
| 4     | 0.5912     | 3.39×           | 84.9 %     |
| 8     | 0.3075     | 6.53×           | 81.6 %     |
| 12    | 0.2672     | 7.51×           | 62.6 %     |

![Speedup y Eficiencia](fernandoR_speedup_eficiencia.png)

---

## Análisis

- **De 1 a 8 hilos el escalamiento es casi lineal.** El speedup sube de forma sostenida y la
  eficiencia se queda arriba del 80 %, lo cual es muy bueno: significa que la paralelización
  aprovecha casi todo el hardware disponible. Pasé de ~2.0 s a ~0.31 s (**6.5× más rápido**).

- **La caída de eficiencia a 12 hilos (62.6 %) tiene una explicación de hardware.** El M4 Pro
  tiene 8 núcleos de rendimiento y 4 de eficiencia (más lentos). Con 8 hilos uso solo los
  rápidos; al pasar a 12, los últimos 4 hilos caen en núcleos lentos y no aportan lo mismo,
  así que el speedup sigue subiendo (7.51×) pero la eficiencia baja. Es un límite físico de la
  máquina, no de mi código.

- **Por qué la eficiencia nunca llega al 100 %:** siempre hay un costo de crear y sincronizar
  los hilos y de juntar las copias de la reducción al final. En un problema como este, además,
  la operación por rectángulo es barata, así que el acceso a memoria empieza a pesar. Aun así,
  el resultado es sólido: **la versión paralela es hasta 7.5× más rápida que la secuencial**.

**Conclusión:** las tres decisiones (reducción, scheduling estático y medición con tiempo de
pared) sí se tradujeron en una mejora real y medible sobre el algoritmo secuencial.

---

## Evidencia de corridas

Salida real del programa (log completo en [`fernandoR_corridas.txt`](fernandoR_corridas.txt)):

```
$ ./suma_omp 0 3.141592653589793 1
Hilos utilizados      = 1
Area total aproximada = 12.335425544595667
Tiempo de ejecucion   = 2.047350 segundos (wall time)

$ ./suma_omp 0 3.141592653589793 8
Hilos utilizados      = 8
Area total aproximada = 12.335425544596465
Tiempo de ejecucion   = 0.311146 segundos (wall time)

$ ./suma_omp 0 3.141592653589793 12
Hilos utilizados      = 12
Area total aproximada = 12.335425544596564
Tiempo de ejecucion   = 0.276347 segundos (wall time)
```

> **Pendiente mío:** adjuntar el screenshot/video de mis corridas (requisito del examen).

---

## Cómo reproducir

```bash
# Compilar y correr el barrido completo (speedup + eficiencia)
bash docs/bench_suma.sh
```

O manualmente:

```bash
# macOS (clang + libomp)
clang -Xpreprocessor -fopenmp -I"$(brew --prefix libomp)/include" \
      -L"$(brew --prefix libomp)/lib" -lomp -O2 paralelo/suma_omp.c -o suma_omp

# Linux (gcc)
gcc -fopenmp -O2 paralelo/suma_omp.c -o suma_omp

./suma_omp 0 3.141592653589793 8   # a  b  num_hilos
```
