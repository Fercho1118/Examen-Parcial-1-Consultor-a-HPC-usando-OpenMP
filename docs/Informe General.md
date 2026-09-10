# Contexto y Datos

## 1. Descripción del Problema Elegido
* **Contexto y Planteamiento:** El problema consiste en realizar la **integración numérica** de una función matemática continua en un intervalo cerrado $[a, b]$ para determinar el área exacta bajo la curva. En escenarios de ingeniería, física o análisis de datos, muchas funciones complejas no poseen una antiderivada analítica directa, por lo que se requiere recurrir a aproximaciones numéricas de alta precisión.
* **Enfoque Computacional:** Se implementa el método de la **Suma de Riemann (extremo izquierdo)** sobre la función no lineal **$f(x) = x^2 + \sin(x)$**. El desafío principal del problema no radica en la complejidad de la fórmula, sino en la **carga computacional**: para maximizar la precisión y reducir el error de aproximación, el algoritmo debe procesar una cantidad masiva de subdivisiones de forma secuencial, convirtiéndose en un problema limitado por la capacidad de procesamiento de la CPU (*CPU-bound*).

## 2. Propuesta Secuencial Original
El flujo lógico del algoritmo está diseñado de forma estrictamente secuencial y lineal utilizando el lenguaje C. Los pasos que ejecuta el programa son los siguientes:

1. **Entrada de Parámetros:** El usuario define dinámicamente el dominio de integración ingresando los límites inferior ($a$) y superior ($b$) a través de la consola.
2. **Cálculo del Diferencial ($\Delta x$):** Se calcula el ancho uniforme de cada rectángulo mediante la fórmula matemática `dx = (b - a) / n`.
3. **Monitoreo de Rendimiento:** Se captura el estado inicial del reloj del sistema (`clock()`) justo antes de iniciar la carga pesada.
4. **Ciclo de Acumulación (Bucle Principal):** Se ejecuta un bucle iterativo `while` controlado por un contador de tipo `long long`. En cada iteración:
   * Se calcula la posición actual en el eje X: $x_i = a + i \cdot \Delta x$.
   * Se evalúa la altura de la curva invocando a la función matemática `f(xi)`.
   * Se calcula el área del rectángulo individual (`altura * dx`) y se acumula en la variable global `areaTotal`.
5. **Cálculo de Métricas y Salida:** Se detiene el temporizador, se calcula el tiempo exacto de procesamiento en segundos y se imprimen en pantalla el área aproximada con alta precisión decimal (`%.15lf`), el total de iteraciones y el tiempo de ejecución.

## 3. Justificación de los Datos de Prueba Utilizados
* **Tamaño de la Muestra ($n$):** Se definió un tamaño fijo y masivo de **$n = 1,000,000,000$ ($10^9$) rectángulos**. La elección de este tamaño es intencional y crítica para el proyecto, ya que una muestra tan grande fuerza al procesador a ejecutar un billón de operaciones de punto flotante en un solo hilo. Esto permite obtener un "tiempo base" medible (de varios segundos) indispensable para evaluar el rendimiento secuencial puro antes de aplicar cualquier técnica de optimización o paralelización.
* **Origen de los Datos:** Los datos se generan de forma matemática intrínseca y determinista. Los límites del intervalo son ingresados por el usuario, mientras que los puntos de evaluación ($x_i$) y sus alturas se calculan dinámicamente en tiempo de ejecución combinando operaciones algebraicas y trigonométricas (`x * x + sin(x)`). Esto simula el comportamiento de un entorno de análisis numérico real sin depender de lecturas lentas de archivos externos (I/O).
* **Estructuras de Datos en Memoria:** Con el fin de maximizar la velocidad y evitar el desbordamiento de memoria (*Out of Memory*), **no se utilizan arreglos, colecciones ni vectores**. Almacenar un billón de valores en un arreglo requeriría gigabytes de memoria RAM innecesariamente. En su lugar, el algoritmo utiliza **estructuras de datos primitivas escalares de alta precisión** (`double` para variables de punto flotante y `long long` para los contadores de 64 bits). Las variables se reutilizan y sobrescriben en cada iteración en los registros de la CPU, logrando una eficiencia de memoria de espacio constante $O(1)$.


## Estrategia de paralelización

## Resultados y metricas