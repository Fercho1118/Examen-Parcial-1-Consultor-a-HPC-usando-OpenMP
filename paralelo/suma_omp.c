#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// misma f(x) que el secuencial para que el area de igual
double f(double x)
{
    return x * x + sin(x);
}

int main(int argc, char *argv[])
{
    double a = 0.0;
    double b = 3.141592653589793;   // por defecto uso [0, pi]
    long long n = 1000000000LL;     // 10^9 rectangulos
    int num_threads = omp_get_max_threads();

    // se puede correr como: ./suma_omp a b hilos
    // asi cambio los hilos sin recompilar
    if (argc >= 3) {
        a = atof(argv[1]);
        b = atof(argv[2]);
    } else {
        printf("Ingrese el limite inferior (a): ");
        scanf("%lf", &a);
        printf("Ingrese el limite superior (b): ");
        scanf("%lf", &b);
    }
    if (argc >= 4) {
        num_threads = atoi(argv[3]);
    }
    omp_set_num_threads(num_threads);

    double dx = (b - a) / n;
    double areaTotal = 0.0;

    // uso omp_get_wtime (tiempo real). clock() no sirve aca porque suma el
    // tiempo de todos los cores y el speedup saldria mal
    double inicio = omp_get_wtime();

    // reparto el for entre los hilos.
    // reduction(+:areaTotal): cada hilo suma en su copia y al final se juntan,
    // asi no chocan al escribir en areaTotal (evita la race condition).
    // schedule(static): todos los rectangulos cuestan igual, reparto parejo.
    #pragma omp parallel for reduction(+:areaTotal) schedule(static)
    for (long long i = 0; i < n; i++)
    {
        double xi = a + i * dx;
        areaTotal += f(xi) * dx;
    }

    double fin = omp_get_wtime();
    double tiempo = fin - inicio;

    printf("Integracion Numerica (Suma de Riemann) - PARALELO OpenMP\n");
    printf("Hilos utilizados      = %d\n", num_threads);
    printf("Area total aproximada = %.15lf\n", areaTotal);
    printf("Rectangulos           = %lld\n", n);
    printf("Tiempo de ejecucion   = %.6lf segundos (wall time)\n", tiempo);
    return 0;
}
