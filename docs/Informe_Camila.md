# Resultados y Métricas

## Decisiones de paralelización

Para la versión paralela se utilizó OpenMP. Se paralelizaron las dos
operaciones principales del programa: el ordenamiento mediante Merge Sort
y la construcción del histograma.

En Merge Sort se aprovecharon las dos llamadas recursivas independientes
correspondientes a las mitades izquierda y derecha del arreglo. Estas se
implementaron utilizando tareas de OpenMP.

Para evitar que la creación de tareas introdujera un costo excesivo, se
estableció un umbral de 10,000 elementos. Cuando un segmento contiene
menos elementos que este umbral, se procesa de manera secuencial.

Para el histograma se dividió el recorrido del arreglo entre los hilos.
Cada hilo utiliza un histograma local de 100 cubetas. Al finalizar, los
histogramas locales se combinan para producir el histograma global.

Esta decisión evita que múltiples hilos incrementen simultáneamente una
misma posición del histograma, reduciendo las condiciones de carrera y
la necesidad de sincronización durante el procesamiento principal.

---

El segundo algoritmo desarrollado corresponde a una integración numérica
mediante la **Suma de Riemann**. El programa aproxima el área bajo la función:

$$
f(x) = x^2 + \sin(x)
$$

utilizando $10^9$ rectángulos.

En la implementación secuencial, cada rectángulo se procesa uno después
del otro. Para cada posición $x_i$ se calcula la altura de la función,
posteriormente el área del rectángulo y finalmente este resultado se
acumula en `areaTotal`.

La principal oportunidad de paralelización se encuentra en este recorrido,
ya que el cálculo del área de un rectángulo no depende del cálculo de los
demás rectángulos.

Por esta razón, en la implementación paralela se utilizó:

```c
#pragma omp parallel for reduction(+:areaTotal) schedule(static)

## Metodología de medición

Para evaluar el rendimiento se utilizó `omp_get_wtime()`.

Se midieron independientemente:

- Tiempo de Merge Sort.
- Tiempo de construcción del histograma.
- Tiempo total de ambos algoritmos.

Las operaciones de entrada/salida, generación de archivos CSV, creación
del archivo `.dat`, impresión de resultados y generación de la gráfica
con gnuplot no fueron incluidas en las mediciones.

El speedup se calculó mediante:

$$
S_p = \frac{T_s}{T_p}
$$

donde:

- $T_s$ representa el tiempo de ejecución secuencial.
- $T_p$ representa el tiempo de ejecución paralelo utilizando $p$ hilos.

La eficiencia se calculó mediante:

$$
E_p = \frac{S_p}{p}
$$

y expresada como porcentaje:

$$
E_p(\%) = \frac{S_p}{p} \times 100
$$

---

## Prueba inicial: N = 20,000

Como primera prueba se utilizaron 20,000 temperaturas.

## Resultados experimentales - Histograma

Las pruebas se realizaron utilizando **20,000 temperaturas** y variando
la cantidad de hilos de la implementación paralela.

| Versión | Hilos | Merge Sort (s) | Histograma (s) | Tiempo total (s) | Speedup | Eficiencia |
|---|---:|---:|---:|---:|---:|---:|
| Secuencial | 1 | 0.004617 | 0.000089 | **0.004706** | **1.000** | **100.00%** |
| Paralela | 1 | 0.005219 | 0.000117 | **0.005336** | **0.882** | **88.19%** |
| Paralela | 2 | 0.002494 | 0.000092 | **0.002586** | **1.820** | **90.99%** |
| Paralela | 4 | 0.002607 | 0.000244 | **0.002852** | **1.650** | **41.25%** |
| Paralela | 8 | 0.003184 | 0.000306 | **0.003491** | **1.348** | **16.85%** |
| Paralela | 10 | 0.002806 | 0.000149 | **0.002955** | **1.593** | **15.93%** |

## Resultados experimentales — Suma de Riemann

| Hilos | Tiempo (s) | Speedup | Eficiencia (%) |
|---:|---:|---:|---:|
| 1  | 2.981788 | 1.00 | 100.0 |
| 2  | 1.686604 | 1.77 | 88.4 |
| 4  | 0.865526 | 3.45 | 86.1 |
| 8  | 0.509791 | 5.85 | 73.1 |
| 12 | 0.478131 | 6.24 | 52.0 |

### Análisis de resultados

Los resultados muestran que la paralelización sí logró reducir el tiempo
de ejecución respecto a la implementación secuencial, aunque el rendimiento
no aumentó de forma proporcional al número de hilos.

La mejor configuración observada fue la ejecución con **2 hilos**, con un
tiempo total de **0.002586 segundos**, frente a los **0.004706 segundos**
de la versión secuencial. Esto produjo un speedup de:

$$
S_2 = \frac{0.004706}{0.002586} \approx 1.820
$$

y una eficiencia de:

$$
E_2 = \frac{1.820}{2}\times100 \approx 90.99\%
$$

Por lo tanto, con 2 hilos el programa fue aproximadamente **1.82 veces
más rápido que la versión secuencial**, aprovechando cerca del **91% de
la capacidad teórica de los dos hilos**.

Al aumentar a 4 hilos, el speedup disminuyó a **1.650** y la eficiencia
a **41.25%**. Con 8 hilos se obtuvo un speedup de **1.348**, mientras que
con 10 hilos el speedup aumentó nuevamente a **1.593**, pero con una
eficiencia de solamente **15.93%**.

Este comportamiento indica que agregar más hilos no necesariamente
produce una mejora proporcional. Para una entrada de 20,000 elementos,
la cantidad de trabajo disponible es relativamente pequeña y el costo
de creación y coordinación de tareas, sincronización y reducción comienza
a representar una parte importante del tiempo total.

También puede observarse que la ejecución paralela con un solo hilo fue
más lenta que la implementación secuencial. El programa secuencial tardó
0.004706 segundos, mientras que la versión OpenMP con un hilo tardó
0.005336 segundos. Esto evidencia el overhead introducido por la
infraestructura de paralelización incluso cuando no existe paralelismo
real.

En conclusión, para el tamaño de entrada evaluado, **2 hilos presentaron
la mejor configuración**, logrando el menor tiempo de ejecución, el mayor
speedup y la mayor eficiencia entre las configuraciones paralelas
evaluadas.

--------------------------------------------------------------------------

Los resultados muestran que la paralelización de la Suma de Riemann
produjo una mejora significativa en el tiempo de ejecución. La versión
con un hilo requirió **2.981788 segundos**, mientras que la configuración
con 12 hilos redujo el tiempo a **0.478131 segundos**.

El speedup se calculó mediante:

$$
S_p = \frac{T_1}{T_p}
$$

Por ejemplo, para 4 hilos:

$$
S_4 = \frac{2.981788}{0.865526} \approx 3.45
$$

Esto significa que utilizando 4 hilos el algoritmo fue aproximadamente
**3.45 veces más rápido** que con un solo hilo.

La eficiencia se calculó mediante:

$$
E_p = \frac{S_p}{p}\times100
$$

Para 4 hilos:

$$
E_4 = \frac{3.45}{4}\times100 \approx 86.1\%
$$

Los resultados muestran una buena escalabilidad durante las primeras
configuraciones. Con 2 hilos se obtuvo un speedup de **1.77** y una
eficiencia de **88.4%**, mientras que con 4 hilos se alcanzó un speedup
de **3.45** manteniendo una eficiencia de **86.1%**.

Al aumentar a 8 hilos, el tiempo disminuyó hasta **0.509791 segundos** y
el speedup aumentó a **5.85**. Sin embargo, la eficiencia descendió a
**73.1%**. Con 12 hilos se obtuvo el menor tiempo de ejecución,
**0.478131 segundos**, y el mayor speedup, **6.24**, pero la eficiencia
disminuyó hasta **52.0%**.

Esta disminución de eficiencia indica que, aunque agregar más hilos
continúa reduciendo el tiempo de ejecución, la mejora deja de ser
proporcional al número de hilos. Entre 8 y 12 hilos, por ejemplo, se
agregaron cuatro hilos adicionales, pero el tiempo solamente disminuyó
de 0.509791 a 0.478131 segundos.

### Impacto de las decisiones de paralelización

La mejora obtenida se relaciona directamente con la estructura de la
Suma de Riemann. El cálculo de cada rectángulo es independiente de los
demás, por lo que las iteraciones pueden distribuirse entre distintos
hilos sin dependencias entre ellas.

Se utilizó `parallel for` para repartir las iteraciones y
`reduction(+:areaTotal)` para que cada hilo pudiera acumular
independientemente una suma parcial. Al finalizar, OpenMP combina estas
sumas para obtener el área total. Esta estrategia evita una condición
de carrera sobre `areaTotal` sin introducir una sección crítica en cada
iteración.

También se utilizó `schedule(static)`, debido a que todas las iteraciones
realizan aproximadamente la misma cantidad de trabajo. Esto permite
distribuir la carga de manera uniforme entre los hilos con un bajo costo
de planificación.

En términos de rendimiento, la configuración con **12 hilos obtuvo el
menor tiempo absoluto y el mayor speedup**, siendo aproximadamente
**6.24 veces más rápida** que la ejecución con un hilo. Sin embargo,
la configuración de **4 hilos presentó un mejor equilibrio entre
aceleración y aprovechamiento de recursos**, alcanzando un speedup de
3.45 con una eficiencia todavía alta de 86.1%.

Por lo tanto, las mediciones demuestran que las decisiones de
paralelización sí mejoraron el rendimiento del
algoritmo, aunque también muestran que existe un punto de rendimientos
decrecientes al continuar aumentando el número de hilos.

### Evidencia de ejecución

> Insertar aquí screenshot de la ejecución secuencial con N = 1,000.

![Ejecución secuencial](imagenes/secuencial_1000.png)

> Insertar aquí screenshot de la ejecución paralela con 10 hilos.

![Ejecución paralela - 10 hilos](imagenes/paralelo_1000_10h.png)

---

## Evaluación con diferentes cantidades de hilos

Para determinar el comportamiento de la solución paralela se realizaron
pruebas utilizando diferentes cantidades de hilos.

Las ejecuciones se realizaron mediante:

```bash
OMP_NUM_THREADS=1 ./histograma
OMP_NUM_THREADS=2 ./histograma
OMP_NUM_THREADS=4 ./histograma
OMP_NUM_THREADS=8 ./histograma
OMP_NUM_THREADS=10 ./histograma