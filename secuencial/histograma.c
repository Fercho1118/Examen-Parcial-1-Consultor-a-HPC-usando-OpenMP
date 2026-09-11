#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX 1000000
#define CUBETAS 100

float A[MAX];
float temp[MAX];
float tempAux[MAX];

int histograma[CUBETAS];

int N;

void Merge(float arreglo[], int izq, int centro, int der) {

    int i = izq;
    int j = centro + 1;
    int k = izq;

    while (i <= centro && j <= der) {

        if (arreglo[i] <= arreglo[j]) {
            tempAux[k] = arreglo[i];
            i++;
        } else {
            tempAux[k] = arreglo[j];
            j++;
        }

        k++;
    }

    while (i <= centro) {
        tempAux[k] = arreglo[i];
        i++;
        k++;
    }

    while (j <= der) {
        tempAux[k] = arreglo[j];
        j++;
        k++;
    }

    for (i = izq; i <= der; i++) {
        arreglo[i] = tempAux[i];
    }
}

void MergeSort(float arreglo[], int izq, int der) {

    if (izq < der) {

        int centro = (izq + der) / 2;

        MergeSort(arreglo, izq, centro);
        MergeSort(arreglo, centro + 1, der);

        Merge(arreglo, izq, centro, der);
    }
}

void Histograma() {

    int i;
    int cubeta;

    for (i = 0; i < CUBETAS; i++) {
        histograma[i] = 0;
    }

    i = 0;
    cubeta = 0;

    float minimo = temp[0];
    float maximo = temp[N - 1];

    float ancho = (maximo - minimo) / CUBETAS;

    if (ancho == 0) {
        ancho = 1;
    }

    float limInf = minimo;
    float limSup = minimo + ancho;

    while (cubeta < CUBETAS && i < N) {

        if (cubeta == CUBETAS - 1) {

            /*
             * La última cubeta incluye el límite superior
             * para asegurarnos de contar el valor máximo.
             */

            if (temp[i] >= limInf && temp[i] <= maximo) {

                histograma[cubeta]++;
                i++;

            } else {

                cubeta++;
            }

        } else {

            if (temp[i] >= limInf && temp[i] < limSup) {

                histograma[cubeta]++;
                i++;

            } else {

                cubeta++;

                limInf = limSup;
                limSup = minimo + (cubeta + 1) * ancho;
            }
        }
    }
}

int main() {

    /* Semilla fija para comparar exactamente los mismos datos. */
    srand(12345);

    printf("Cantidad de temperaturas: ");
    scanf("%d", &N);

    if (N <= 0 || N > MAX) {
        printf("\nError: N debe estar entre 1 y %d\n", MAX);
        return 1;
    }

    for (int i = 0; i < N; i++) {
        A[i] = -100.0 + (rand() % 20001) / 100.0;
    }


    printf("\nPrimeras 20 temperaturas generadas:\n\n");

    for (int i = 0; i < 20 && i < N; i++) {
        printf("%.2f ", A[i]);
    }

    for (int i = 0; i < N; i++) {
        temp[i] = A[i];
    }

    double inicioTotal = omp_get_wtime();

    double inicioSort = omp_get_wtime();

    MergeSort(temp, 0, N - 1);

    double finSort = omp_get_wtime();

    double tiempoSort = finSort - inicioSort;

    double inicioHist = omp_get_wtime();

    Histograma();

    double finHist = omp_get_wtime();

    double tiempoHist = finHist - inicioHist;

    double finTotal = omp_get_wtime();

    double tiempoTotal = finTotal - inicioTotal;

    printf("\n\n=================================\n");
    printf("        RESULTADOS SERIAL\n");
    printf("=================================\n");

    printf("N                  : %d\n", N);
    printf("Merge Sort         : %.6f segundos\n", tiempoSort);
    printf("Histograma         : %.6f segundos\n", tiempoHist);
    printf("Tiempo total       : %.6f segundos\n", tiempoTotal);

    printf("=================================\n");

    printf("\nPrimeras 20 temperaturas ordenadas:\n\n");

    for (int i = 0; i < 20 && i < N; i++) {
        printf("%.2f ", temp[i]);
    }

    printf("\n\nHistograma\n\n");

    for (int i = 0; i < CUBETAS; i++) {
        printf("Cubeta %2d : %d\n", i, histograma[i]);
    }

    FILE *csv = fopen("temperaturas_ordenadas_serial.csv", "w");

    if (csv == NULL) {
        printf("\nNo se pudo crear el archivo CSV.\n");
        return 1;
    }

    fprintf(csv, "Indice,Temperatura\n");

    for (int i = 0; i < N; i++) {
        fprintf(csv, "%d,%.2f\n", i, temp[i]);
    }

    fclose(csv);

    FILE *archivo = fopen("histograma_serial.dat", "w");

    if (archivo == NULL) {
        printf("\nNo se pudo crear histograma_serial.dat\n");
        return 1;
    }

    float minimo = temp[0];
    float maximo = temp[N - 1];

    float ancho = (maximo - minimo) / CUBETAS;

    if (ancho == 0) {
        ancho = 1;
    }

    for (int i = 0; i < CUBETAS; i++) {

        float temperatura = minimo + (i * ancho);

        fprintf(
            archivo,
            "%.2f %d\n",
            temperatura,
            histograma[i]
        );
    }

    fclose(archivo);


    FILE *gp = popen("gnuplot", "w");

    if (gp == NULL) {
        printf("\nNo se pudo ejecutar gnuplot.\n");
        return 1;
    }

    fprintf(
        gp,
        "set terminal pngcairo size 1400,700 "
        "enhanced font 'Arial,12'\n"
    );

    fprintf(gp, "set output 'histograma_serial.png'\n");

    fprintf(gp, "set title 'Histograma Serial de Temperaturas'\n");

    fprintf(gp, "set xlabel 'Temperatura (°C)'\n");

    fprintf(gp, "set ylabel 'Frecuencia'\n");

    fprintf(gp, "set xrange [-100:100]\n");

    fprintf(gp, "set xtics -100,20,100\n");

    fprintf(gp, "set grid ytics\n");

    fprintf(gp, "set style fill solid 1.0\n");

    fprintf(gp, "set boxwidth 1.8\n");

    fprintf(
        gp,
        "plot 'histograma_serial.dat' using 1:2 "
        "with boxes lc rgb '#2E86DE' "
        "title 'Frecuencia'\n"
    );

    fprintf(gp, "exit\n");

    pclose(gp);


    printf("\nArchivos generados:\n");
    printf("temperaturas_ordenadas_serial.csv\n");
    printf("histograma_serial.dat\n");
    printf("histograma_serial.png\n");

    return 0;
}
