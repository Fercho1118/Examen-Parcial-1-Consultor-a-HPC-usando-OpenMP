# Informe General — Consultora HPC "Los Paralelos"

**Universidad del Valle de Guatemala — CC3069 Computación Paralela y Distribuida**
**Integrantes:** Camila Richter (23183), Fernando Rueda (23748), Sebastián García (22291)
**Repositorio:** <https://github.com/Fercho1118/Examen-Parcial-1-Consultor-a-HPC-usando-OpenMP>

De los tres problemas asignados a nuestro grupo elegimos implementar en paralelo dos: la
**Suma de Riemann** (integración numérica) y el **Histograma de temperaturas** (precedido de un
Merge Sort). Descartamos el de Ruta Mínima en Grafos por ser el de paralelización más compleja
(carga irregular y reparto dinámico). Este informe reúne los tres puntos requeridos. Las
mediciones individuales de cada integrante están en sus reportes respectivos:

- [FernandoR_Resultados_y_Métricas.md](FernandoR_Resultados_y_Métricas.md)
- [Informe_Camila.md](Informe_Camila.md)
- [SebastianG_Resultados_y_Metricas.md](SebastianG_Resultados_y_Metricas.md)

---

# 1. Contexto y Datos

## 1.1 Suma de Riemann

**Contexto y planteamiento.** El problema consiste en realizar la **integración numérica** de una
función continua en un intervalo cerrado $[a, b]$ para aproximar el área bajo la curva. Muchas
funciones no poseen una antiderivada analítica directa, por lo que se recurre a aproximaciones
numéricas. Implementamos el método de la **Suma de Riemann (extremo izquierdo)** sobre la función
no lineal $f(x) = x^2 + \sin(x)$. El desafío no está en la fórmula, sino en la **carga
computacional**: para reducir el error hay que procesar una cantidad masiva de subdivisiones, lo
que lo convierte en un problema limitado por CPU (*CPU-bound*).

**Propuesta secuencial original.** El algoritmo es estrictamente lineal en C: recibe los límites
$a$ y $b$, calcula el ancho uniforme `dx = (b - a) / n`, y en un bucle controlado por un contador
`long long` acumula el área de cada rectángulo (`altura * dx`) sobre la variable `areaTotal`.
Finalmente imprime el área aproximada, el número de iteraciones y el tiempo de ejecución.

**Justificación de los datos de prueba.**
- **Tamaño de la muestra:** $n = 10^9$ rectángulos. Una muestra masiva fuerza al procesador a
  ejecutar miles de millones de operaciones de punto flotante en un solo hilo, dando un "tiempo
  base" medible (de varios segundos) indispensable para evaluar el speedup.
- **Origen de los datos:** se generan de forma matemática y determinista. Los puntos $x_i$ y sus
  alturas se calculan en tiempo de ejecución (`x*x + sin(x)`), sin depender de lecturas de disco.
- **Estructuras en memoria:** **no se usan arreglos**. Almacenar $10^9$ valores gastaría gigabytes
  innecesarios. En su lugar se usan escalares (`double` y `long long`) que se reutilizan en cada
  iteración, logrando memoria constante $O(1)$.

## 1.2 Histograma de temperaturas

**Contexto y planteamiento.** El problema consiste en clasificar un conjunto masivo de mediciones
de temperatura en **100 cubetas (rangos)** y contar cuántas caen en cada una. Nuestra
implementación añade una etapa previa de **ordenamiento (Merge Sort)** para obtener el mínimo y el
máximo de forma directa y facilitar el particionado en cubetas de ancho uniforme.

**Propuesta secuencial original.** En C, el programa genera las temperaturas, las ordena con un
Merge Sort recursivo y luego recorre el arreglo asignando cada valor a su cubeta
`(int)((temp[i] - minimo) / ancho)`, incrementando el contador correspondiente. Al final escribe
los resultados y una gráfica del histograma.

**Justificación de los datos de prueba.**
- **Tamaño de la muestra:** $N = 10^6$ temperaturas (máximo que admite la implementación, por sus
  arreglos estáticos `#define MAX 1000000`), clasificadas en 100 cubetas.
- **Origen de los datos:** se generan aleatoriamente en el rango $[-100, 100]$ °C. Se usa una
  **semilla fija** para que las versiones secuencial y paralela procesen exactamente la misma
  secuencia y la comparación sea justa.
- **Estructuras en memoria:** arreglos globales de `float` para las temperaturas y sus copias de
  ordenamiento, más un arreglo de 100 enteros para el histograma. El uso de memoria es $O(N)$.

---

# 2. Estrategia de Paralelización

Ambos programas se paralelizaron con **OpenMP** y se midieron con `omp_get_wtime()` (tiempo de
pared), no con `clock()`, porque `clock()` suma el tiempo de CPU de todos los núcleos y daría un
speedup falso en paralelo.

## 2.1 Suma de Riemann

El cálculo del área de cada rectángulo es **independiente**, así que el bucle principal se
paraleliza con una sola directiva:

```c
#pragma omp parallel for reduction(+:areaTotal) schedule(static)
```

- **`parallel for`** reparte las $10^9$ iteraciones entre los hilos.
- **`reduction(+:areaTotal)`** le da a cada hilo un acumulador privado y combina las sumas
  parciales al final. Así se evita la **condición de carrera** sobre `areaTotal` sin serializar la
  suma. Se descartó `#pragma omp critical`, que obligaría a sumar un hilo a la vez y anularía el
  speedup.
- **`schedule(static)`** es el reparto óptimo porque todas las iteraciones cuestan lo mismo
  (carga uniforme). Un `dynamic` solo agregaría overhead de planificación sin beneficio.

## 2.2 Histograma de temperaturas

Se optimizaron las dos etapas del programa:

1. **Merge Sort con tareas.** Una región `parallel` crea el equipo y `single` inicia una única
   recursión. Cada mitad independiente se lanza como `#pragma omp task` y `taskwait` garantiza que
   ambas terminen antes de combinarlas. Un **umbral de 10,000 elementos** evita crear tareas para
   segmentos pequeños, donde el costo de administrarlas superaría el beneficio.
2. **Histograma privado por hilo.** `#pragma omp for` reparte las temperaturas y cada hilo cuenta
   en su propio arreglo local de 100 cubetas, evitando que varios hilos incrementen la misma
   posición a la vez (**condición de carrera**). Al final, una sección `#pragma omp critical`
   combina los histogramas locales con el global. Aquí `critical` sí es adecuado porque se ejecuta
   **una vez por hilo** (100 sumas), no una vez por temperatura, por lo que la serialización es
   mínima.

**Sobre el balance de carga:** en Riemann el reparto estático basta por la uniformidad del
trabajo. En el histograma, el Merge Sort tiene partes inherentemente secuenciales (las
combinaciones cerca de la raíz del árbol) y barreras entre dependencias, lo que limita su
escalabilidad según la Ley de Amdahl.

---

# 3. Resultados y Métricas

Como cada quien tenía que medir por su cuenta, corrimos los dos programas en nuestras propias
máquinas y aquí juntamos lo que obtuvimos los tres. El detalle completo, con todas las corridas, las
gráficas y las capturas, está en nuestros reportes individuales que enlazamos al inicio.

Vale la pena aclarar que no medimos exactamente igual. Fernando corrió en una Apple M4 Pro (12
núcleos) y Camila en su equipo, los dos hasta 12 hilos y calculando el speedup contra la corrida
de un solo hilo. Sebastián lo hizo en una Intel Core i5-10300H (4 núcleos, 8 hilos lógicos), hasta
8 hilos y comparando contra el secuencial. Por eso en su caso el de 1 hilo sale en 0.94×, porque el
overhead de OpenMP lo deja apenas más lento que el secuencial puro.

## 3.1 Suma de Riemann — speedup por número de hilos

| Hilos | Fernando (M4 Pro) | Camila | Sebastián (i5) |
|:-----:|:-----------------:|:------:|:--------------:|
| 1  | 1.00× | 1.00× | 1.00× |
| 2  | 1.78× | 1.77× | 1.66× |
| 4  | 3.44× | 3.45× | 2.47× |
| 8  | 6.87× | 5.85× | 3.08× |
| 12 | 8.21× | 6.24× | — |

## 3.2 Histograma — speedup por número de hilos (tiempo total)

| Hilos | Fernando (M4 Pro) | Camila | Sebastián (i5) |
|:-----:|:-----------------:|:------:|:--------------:|
| 1  | 1.00× | 1.00× | 0.94× |
| 2  | 1.86× | 1.83× | 1.70× |
| 4  | 2.90× | 3.15× | 2.36× |
| 8  | 4.03× | 4.28× | 2.93× |
| 12 | 4.78× | 4.18× | — |

## 3.3 La mejor configuración de cada uno

| Integrante | Problema | Mejor config | Speedup | Eficiencia |
|:-----------|:-----------|:------------:|:-------:|:----------:|
| Fernando  | Riemann    | 12 hilos | 8.21× | 68.4% |
| Camila    | Riemann    | 12 hilos | 6.24× | 52.0% |
| Sebastián | Riemann    | 8 hilos  | 3.08× | 38.5% |
| Fernando  | Histograma | 12 hilos | 4.78× | 39.8% |
| Camila    | Histograma | 8 hilos  | 4.28× | 53.6% |
| Sebastián | Histograma | 8 hilos  | 2.93× | 36.6% |

## 3.4 Qué observamos

Aunque las máquinas eran distintas, los tres llegamos a la misma conclusión.

La Suma de Riemann fue la que mejor escaló. Como es un problema grande, parejo y de pura suma, se
acerca bastante al speedup ideal mientras haya núcleos libres: la M4 Pro de 12 núcleos llegó a
8.21×, Camila a 6.24×, y el i5 de Sebastián se quedó en 3.08× simplemente porque tiene menos
núcleos.

El histograma, en cambio, se estancó mucho antes en los tres casos. Su trabajo total es chiquito, apenas unas decenas de milisegundos, y el Merge Sort arrastra partes que van en serie, más las
barreras y el `critical` de la combinación final. Todo eso frena la aceleración y la eficiencia se
cae rápido (los tres terminamos por debajo del ~54 %).

Al final la lección nos quedó clara: paralelizar no es solo meter más hilos. Rinde de verdad cuando
el problema es lo bastante grande y regular, y además depende del hardware que uno tenga, que es
justo el tipo de criterio que debería manejar una consultora de HPC.
