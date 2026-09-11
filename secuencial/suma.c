#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

double f(double x)
{
    return x * x + sin(x);
}

int main(int argc, char *argv[])
{
    /* Mismos datos por defecto que la version OpenMP. */
    double a = 0.0;
    double b = 3.141592653589793;
    long long n = 1000000000LL;

    /* Uso: ./suma_serial a b */
    if (argc >= 3) {
        a = atof(argv[1]);
        b = atof(argv[2]);
    } else {
        printf("Ingrese el limite inferior (a): ");
        if (scanf("%lf", &a) != 1) {
            return 1;
        }

        printf("Ingrese el limite superior (b): ");
        if (scanf("%lf", &b) != 1) {
            return 1;
        }
    }

    double dx = (b - a) / n;
    double areaTotal = 0.0;
    long long i = 0;

    /* Mismo temporizador de pared que la version OpenMP. */
    double inicio = omp_get_wtime();

    /* Algoritmo completamente secuencial. */
    while (i < n) {
        double xi = a + i * dx;
        areaTotal += f(xi) * dx;
        i++;
    }

    double fin = omp_get_wtime();
    double tiempo = fin - inicio;

    printf("Integracion Numerica (Suma de Riemann) - SECUENCIAL\n");
    printf("Area total aproximada = %.15lf\n", areaTotal);
    printf("Rectangulos           = %lld\n", n);
    printf("Tiempo de ejecucion   = %.6lf segundos (wall time)\n", tiempo);

    return 0;
}
