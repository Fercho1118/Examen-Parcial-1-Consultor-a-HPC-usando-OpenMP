#include <stdio.h>
#include <math.h>
#include <time.h>  

// Definir la función f(x)
double f(double x)
{
    return x * x + sin(x);   // Ejemplo: f(x) = x²
}

int main()
{
    // Declaración de variables
    double a, b;
    double dx;
    double areaTotal = 0.0;
    double xi, altura, areaRectangulo;

    long long n = 1000000000LL; // 10^9 rectángulos
    long long i = 0;

    // Variables para medir el tiempo
    clock_t inicio, fin;
    double tiempo;

    printf("Integracion Numerica (Suma de Riemann)\n");

    printf("Ingrese el limite inferior (a): ");
    scanf("%lf", &a);

    printf("Ingrese el limite superior (b): ");
    scanf("%lf", &b);

    // Calcular ancho del rectángulo
    dx = (b - a) / n;

    // Inicializar área total
    areaTotal = 0.0;

    // Iniciar temporizador
    inicio = clock();

    // Ciclo principal
    while (i < n)
    {
        // Calcular xi
        xi = a + i * dx;

        // Evaluar altura
        altura = f(xi);

        // Calcular área del rectángulo
        areaRectangulo = altura * dx;

        // Sumar al área total
        areaTotal += areaRectangulo;

        // Incrementar contador
        i++;
    }

    // Detener temporizador
    fin = clock();

    // Calcular tiempo en segundos
    tiempo = (double)(fin - inicio) / CLOCKS_PER_SEC;

    // Mostrar resultados
    printf("Area total aproximada = %.15lf\n", areaTotal);
    printf("Rectangulos utilizados = %lld\n", n);
    printf("Tiempo de ejecucion = %.3lf segundos\n", tiempo);
    return 0;
}