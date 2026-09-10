# Resultados y Métricas

Esta sección presenta las decisiones de paralelización, las mediciones
de **speedup** y **eficiencia**, y el análisis experimental de los dos
algoritmos desarrollados: **histograma de temperaturas** y **Suma de
Riemann**.

## 1. Histograma de temperaturas

### Decisiones de paralelización

Para la versión paralela se utilizó OpenMP. Se paralelizaron las dos
operaciones principales del programa: el ordenamiento mediante **Merge
Sort** y la construcción del histograma.

En Merge Sort se aprovecharon las dos llamadas recursivas independientes
correspondientes a las mitades izquierda y derecha del arreglo. Estas se
implementaron mediante tareas de OpenMP. Para evitar que la creación de
tareas introdujera un costo excesivo, se estableció un umbral de
**10,000 elementos**; por debajo de este tamaño, el segmento se procesa
secuencialmente.

Para el histograma, el arreglo se distribuye entre los hilos y cada hilo
utiliza un histograma local de **100 cubetas**. Al finalizar, los
histogramas locales se combinan para producir el histograma global. Esta
decisión evita que varios hilos incrementen simultáneamente una misma
posición durante el recorrido principal y reduce el riesgo de
condiciones de carrera.

### Metodología de medición

Para evaluar el rendimiento se utilizó `omp_get_wtime()`. Se midieron
por separado:

-   Tiempo de Merge Sort.
-   Tiempo de construcción del histograma.
-   Tiempo total de ambos algoritmos.

Las operaciones de entrada/salida, generación de archivos CSV y `.dat`,
impresión de resultados y generación de la gráfica con gnuplot no se
incluyeron en las mediciones.

El speedup se calculó mediante:

$$
S_p = \frac{T_s}{T_p}
$$

donde $T_s$ es el tiempo de la implementación secuencial y $T_p$ es el
tiempo de la implementación paralela con $p$ hilos.

La eficiencia se calculó como:

$$
E_p(\%) = \frac{S_p}{p}\times 100
$$

### Resultados experimentales

Para evaluar el rendimiento de la implementación paralela del histograma
se realizaron pruebas variando la cantidad de hilos entre **1, 2, 4, 8 y 12**.

Los resultados obtenidos fueron:

| Hilos | Tiempo total (s) | Speedup | Eficiencia (%) |
|---:|---:|---:|---:|
| 1 | 0.072480 | 1.00 | 100.0 |
| 2 | 0.039711 | 1.83 | 91.3 |
| 4 | 0.022990 | 3.15 | 78.8 |
| 8 | **0.016915** | **4.28** | 53.6 |
| 12 | 0.017356 | 4.18 | 34.8 |

### Análisis de resultados

Para estas pruebas se tomó como referencia el tiempo de ejecución con
**1 hilo**:

$$
T_1 = 0.072480\text{ s}
$$

El speedup se calculó mediante:

$$
S_p = \frac{T_1}{T_p}
$$

y la eficiencia mediante:

$$
E_p(\%) = \frac{S_p}{p}\times100
$$

Los resultados muestran que la paralelización produjo una reducción
considerable en el tiempo de ejecución conforme aumentó el número de
hilos.

Con **2 hilos**, el tiempo disminuyó de **0.072480 s** a **0.039711 s**,
obteniendo un speedup de **1.83** y una eficiencia de **91.3%**. Esto
indica que los dos hilos fueron aprovechados de manera eficiente.

Con **4 hilos**, el tiempo se redujo hasta **0.022990 s**, alcanzando un
speedup de **3.15** y una eficiencia de **78.8%**. Aunque la eficiencia
disminuyó respecto a la configuración de 2 hilos, el tiempo de ejecución
continuó reduciéndose considerablemente.

La mejor configuración en términos de tiempo de ejecución fue la de
**8 hilos**, con un tiempo total de **0.016915 s** y un speedup de
**4.28**.

Para esta configuración:

$$
S_8 = \frac{0.072480}{0.016915} \approx 4.28
$$

y su eficiencia fue:

$$
E_8 = \frac{4.28}{8}\times100 \approx 53.6\%
$$

Esto significa que la implementación con 8 hilos fue aproximadamente
**4.28 veces más rápida** que la ejecución con un hilo.

Al aumentar a **12 hilos**, el tiempo aumentó ligeramente hasta
**0.017356 s**. Como consecuencia, el speedup disminuyó a **4.18** y la
eficiencia cayó hasta **34.8%**.

Este resultado permite identificar un punto de **rendimientos
decrecientes**. Agregar cuatro hilos adicionales después de los 8 hilos
no produjo una mejora adicional en el tiempo de ejecución. El overhead
de administración de hilos, creación de tareas, sincronización y
combinación de resultados comienza a limitar la escalabilidad del
algoritmo.

### Impacto de las decisiones de paralelización

Los resultados obtenidos respaldan las decisiones tomadas durante la
paralelización.

En **Merge Sort**, las mitades izquierda y derecha del arreglo pueden
ordenarse independientemente. El uso de tareas de OpenMP permite ejecutar
estas operaciones simultáneamente. Además, el uso de un umbral evita
crear tareas para segmentos demasiado pequeños, donde el costo de
administrarlas podría superar el beneficio del paralelismo.

Para la construcción del histograma, cada hilo utiliza un histograma
local de 100 cubetas. De esta manera, los hilos pueden procesar sus
elementos independientemente sin modificar continuamente un mismo
histograma global. Al finalizar, los resultados locales se combinan para
obtener el histograma final.

El impacto de estas decisiones puede observarse al comparar las
ejecuciones con 1 y 8 hilos:

$$
0.072480\text{ s} \rightarrow 0.016915\text{ s}
$$

La reducción porcentual del tiempo fue aproximadamente:

$$
\frac{0.072480-0.016915}{0.072480}\times100
\approx 76.7\%
$$

Por lo tanto, la configuración con 8 hilos logró una reducción aproximada
del **76.7% en el tiempo de ejecución**, además de alcanzar un speedup
máximo de **4.28**.

Las configuraciones con 2 y 4 hilos presentaron eficiencias más altas,
de **91.3%** y **78.8%**, respectivamente. Sin embargo, la configuración
con 8 hilos obtuvo el menor tiempo absoluto y el mayor speedup.

En conclusión, las decisiones de paralelización permitieron mejorar
significativamente el rendimiento del algoritmo. Los resultados también
demuestran que utilizar una mayor cantidad de hilos no garantiza una
mejora continua, ya que después de 8 hilos el overhead comienza a superar
el beneficio adicional del paralelismo.

### Evidencia de ejecución

> **PENDIENTE:** insertar las capturas o evidencia de las corridas
> utilizadas en la tabla anterior.

Sugerencia de evidencias:

-   Ejecución secuencial con $N=20,000$.
-   Ejecución paralela con 1 hilo.
-   Ejecución paralela con 2 hilos.
-   Ejecución paralela con 4 hilos.
-   Ejecución paralela con 8 hilos.
-   Ejecución paralela con 10 hilos.

Ejemplo de sintaxis:

``` markdown
![Evidencia](imagenes/histograma_camila.png)
```

------------------------------------------------------------------------

## 2. Suma de Riemann

### Decisiones de paralelización

El segundo algoritmo corresponde a una integración numérica mediante la
**Suma de Riemann**. El programa aproxima el área bajo la función:

$$
f(x) = x^2 + \sin(x)
$$

utilizando $10^9$ rectángulos.

En la implementación secuencial, cada rectángulo se procesa uno después
del otro. La principal oportunidad de paralelización está en que el
cálculo del área de cada rectángulo es independiente de los demás.

La implementación paralela utiliza:

``` c
#pragma omp parallel for reduction(+:areaTotal) schedule(static)
```

`parallel for` distribuye las iteraciones entre los hilos. La cláusula
`reduction(+:areaTotal)` permite que cada hilo acumule una suma parcial
y que OpenMP combine los resultados al final, evitando una condición de
carrera sobre `areaTotal`.

Se utilizó `schedule(static)` porque cada iteración realiza
aproximadamente la misma cantidad de trabajo, por lo que una
distribución estática permite repartir la carga de manera uniforme con
un bajo costo de planificación.

### Metodología de medición

Las ejecuciones se realizaron variando el número de hilos y manteniendo
constante el trabajo de $10^9$ rectángulos. El tiempo de referencia
corresponde a la ejecución con **1 hilo**.

Para este experimento:

$$
S_p = \frac{T_1}{T_p}
$$

y:

$$
E_p(\%) = \frac{S_p}{p}\times100
$$

### Resultados experimentales

    Hilos   Tiempo (s)   Speedup   Eficiencia (%)
  ------- ------------ --------- ----------------
        1     2.981788      1.00            100.0
        2     1.686604      1.77             88.4
        4     0.865526      3.45             86.1
        8     0.509791      5.85             73.1
       12     0.478131      6.24             52.0

### Análisis de resultados

La paralelización de la Suma de Riemann produjo una mejora
significativa. La ejecución con un hilo requirió **2.981788 s**,
mientras que con 12 hilos el tiempo disminuyó a **0.478131 s**.

Por ejemplo, para 4 hilos:

$$
S_4 = \frac{2.981788}{0.865526} \approx 3.45
$$

$$
E_4 = \frac{3.45}{4}\times100 \approx 86.1\%
$$

Con 2 hilos se obtuvo un speedup de **1.77** y una eficiencia de
**88.4%**. Con 4 hilos se alcanzó un speedup de **3.45**, manteniendo
una eficiencia alta de **86.1%**.

Al aumentar a 8 hilos, el tiempo disminuyó hasta **0.509791 s** y el
speedup aumentó a **5.85**, aunque la eficiencia descendió a **73.1%**.
Con 12 hilos se obtuvo el menor tiempo, **0.478131 s**, y el mayor
speedup, **6.24**, pero la eficiencia disminuyó hasta **52.0%**.

De 1 a 12 hilos, el tiempo se redujo aproximadamente un **84%**. Sin
embargo, entre 8 y 12 hilos el tiempo solo pasó de **0.509791 s** a
**0.478131 s**, mostrando rendimientos decrecientes al continuar
aumentando el número de hilos.

Las decisiones de paralelización contribuyeron directamente a esta
mejora. `parallel for` permitió repartir las iteraciones independientes;
`reduction` evitó una condición de carrera sin serializar cada
actualización de la suma; y `schedule(static)` fue apropiado porque las
iteraciones presentan una carga de trabajo uniforme.

En términos de rendimiento, **12 hilos obtuvieron el menor tiempo
absoluto y el mayor speedup**, mientras que **4 hilos mostraron un mejor
equilibrio entre aceleración y eficiencia**, con un speedup de 3.45 y
una eficiencia de 86.1%.

### Evidencia de ejecución

> **PENDIENTE:** insertar screenshot o video de las corridas que
> respaldan las mediciones anteriores.

Sugerencia de evidencias:

-   Suma de Riemann con 1 hilo.
-   Suma de Riemann con 2 hilos.
-   Suma de Riemann con 4 hilos.
-   Suma de Riemann con 8 hilos.
-   Suma de Riemann con 12 hilos.

Ejemplo:

``` markdown
![Evidencia](imagenes/suma_camila.png)
```

------------------------------------------------------------------------

## 3. Conclusión de las métricas

Los dos algoritmos muestran comportamientos distintos frente al
paralelismo.

En el histograma con 20,000 elementos, la mejor configuración fue de **2
hilos**, con un speedup de **1.82**, una eficiencia de **90.99%** y una
reducción aproximada del **45%** en el tiempo respecto a la versión
secuencial. A partir de ese punto, el overhead de OpenMP y la
sincronización limitaron la escalabilidad.

La Suma de Riemann presentó una escalabilidad mayor debido a que posee
una carga computacional considerable y sus iteraciones son
independientes. Con 12 hilos alcanzó un speedup de **6.24** y redujo el
tiempo aproximadamente un **84%** respecto a la ejecución con un hilo.

Estos resultados muestran que la efectividad de la paralelización
depende tanto del número de hilos como del tamaño y la estructura del
problema. Las decisiones tomadas permitieron mejorar el rendimiento,
pero las mediciones también muestran que agregar hilos indefinidamente
no garantiza una mejora proporcional.

## 4. Evidencia individual requerida

> **PENDIENTE DE COMPLETAR ANTES DE ENTREGAR.**

