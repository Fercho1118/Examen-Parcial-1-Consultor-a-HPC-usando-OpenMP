#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX 1000000
#define CUBETAS 100

/*
 * Si el segmento es menor que este valor,
 * dejamos de crear tareas y utilizamos
 * Merge Sort serial.
 */
#define UMBRAL 10000

float A[MAX];
float temp[MAX];
float tempAux[MAX];

int histograma[CUBETAS];

int N;


/* =========================
   MERGE
   ========================= */

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


/* 
   Se utiliza para segmentos pequeños
   para evitar crear demasiadas tareas.
*/

void MergeSortSerial(float arreglo[], int izq, int der) {

    if (izq < der) {

        int centro = (izq + der) / 2;

        MergeSortSerial(arreglo, izq, centro);
        MergeSortSerial(arreglo, centro + 1, der);

        Merge(arreglo, izq, centro, der);
    }
}

void MergeSortParalelo(float arreglo[], int izq, int der) {

    if (izq < der) {

        int centro = (izq + der) / 2;

        /*
         * Solo creamos nuevas tareas cuando
         * el segmento es suficientemente grande.
         */

        if ((der - izq + 1) > UMBRAL) {

            /*
             * Tarea izquierda
             */

            #pragma omp task shared(arreglo) firstprivate(izq, centro)
            {
                MergeSortParalelo(
                    arreglo,
                    izq,
                    centro
                );
            }


            /*
             * Tarea derecha
             */

            #pragma omp task shared(arreglo) firstprivate(centro, der)
            {
                MergeSortParalelo(
                    arreglo,
                    centro + 1,
                    der
                );
            }


            /*
             * No podemos hacer Merge hasta que
             * las dos mitades estén ordenadas.
             */

            #pragma omp taskwait

        } else {

            /*
             * Para segmentos pequeños dejamos
             * de crear tareas.
             */

            MergeSortSerial(
                arreglo,
                izq,
                centro
            );

            MergeSortSerial(
                arreglo,
                centro + 1,
                der
            );
        }


        /*
         * Ambas mitades ya están ordenadas.
         */

        Merge(
            arreglo,
            izq,
            centro,
            der
        );
    }
}

void HistogramaParalelo() {

    /*
     * Inicializar histograma global.
     */

    for (int c = 0; c < CUBETAS; c++) {
        histograma[c] = 0;
    }


    float minimo = temp[0];
    float maximo = temp[N - 1];

    float ancho = (maximo - minimo) / CUBETAS;

    if (ancho == 0) {
        ancho = 1;
    }


    /*
     * Crear equipo de hilos.
     */

    #pragma omp parallel
    {

        /*
         * Cada hilo tiene su propio histograma.
         *
         * De esta manera evitamos que varios
         * hilos modifiquen simultáneamente
         * histograma[].
         */

        int histLocal[CUBETAS] = {0};


        /*
         * OpenMP divide automáticamente
         * las N temperaturas entre los hilos.
         */

        #pragma omp for
        for (int i = 0; i < N; i++) {

            /*
             * Determinar directamente a qué
             * cubeta pertenece la temperatura.
             */

            int cubeta =
                (int)((temp[i] - minimo) / ancho);


            /*
             * El valor máximo puede producir
             * exactamente CUBETAS.
             *
             * En ese caso pertenece a la última.
             */

            if (cubeta >= CUBETAS) {
                cubeta = CUBETAS - 1;
            }

            if (cubeta < 0) {
                cubeta = 0;
            }


            /*
             * Solo modificamos el histograma
             * privado del hilo.
             */

            histLocal[cubeta]++;
        }


        /*
         * REDUCCIÓN
         *
         * Cada hilo suma su histograma local
         * al histograma global.
         *
         * critical evita condiciones de carrera.
         */

        #pragma omp critical
        {
            for (int c = 0; c < CUBETAS; c++) {

                histograma[c] += histLocal[c];
            }
        }
    }
}

int main() {

    srand(time(NULL));

    printf("Cantidad de temperaturas: ");
    scanf("%d", &N);

    if (N <= 0 || N > MAX) {
        printf("\nError: N debe estar entre 1 y %d\n", MAX);
        return 1;
    }

    for (int i = 0; i < N; i++) {

        A[i] =
            -100.0 +
            (rand() % 20001) / 100.0;
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


    /*
     * parallel crea el equipo de hilos.
     *
     * single hace que solo un hilo comience
     * la recursión.
     *
     * Las tareas creadas dentro pueden ser
     * ejecutadas por los demás hilos.
     */

    #pragma omp parallel
    {
        #pragma omp single
        {
            MergeSortParalelo(
                temp,
                0,
                N - 1
            );
        }
    }


    double finSort = omp_get_wtime();

    double tiempoSort =
        finSort - inicioSort;

    double inicioHist =
        omp_get_wtime();


    HistogramaParalelo();


    double finHist =
        omp_get_wtime();

    double tiempoHist =
        finHist - inicioHist;

    double finTotal =
        omp_get_wtime();

    double tiempoTotal =
        finTotal - inicioTotal;

    int numHilos = omp_get_max_threads();

    printf("\n\n=================================\n");
    printf("       RESULTADOS PARALELO\n");
    printf("=================================\n");

    printf("N                  : %d\n", N);
    printf("Hilos              : %d\n", numHilos);

    printf(
        "Merge Sort         : %.6f segundos\n",
        tiempoSort
    );

    printf(
        "Histograma         : %.6f segundos\n",
        tiempoHist
    );

    printf(
        "Tiempo total       : %.6f segundos\n",
        tiempoTotal
    );

    printf("=================================\n");

    printf(
        "\nPrimeras 20 temperaturas ordenadas:\n\n"
    );

    for (int i = 0; i < 20 && i < N; i++) {

        printf("%.2f ", temp[i]);
    }

    printf("\n\nHistograma\n\n");

    for (int i = 0; i < CUBETAS; i++) {

        printf(
            "Cubeta %2d : %d\n",
            i,
            histograma[i]
        );
    }

    FILE *csv =
        fopen(
            "temperaturas_ordenadas_paralelo.csv",
            "w"
        );

    if (csv == NULL) {

        printf(
            "\nNo se pudo crear el archivo CSV.\n"
        );

        return 1;
    }


    fprintf(
        csv,
        "Indice,Temperatura\n"
    );


    for (int i = 0; i < N; i++) {

        fprintf(
            csv,
            "%d,%.2f\n",
            i,
            temp[i]
        );
    }

    fclose(csv);

    FILE *archivo =
        fopen(
            "histograma_paralelo.dat",
            "w"
        );

    if (archivo == NULL) {

        printf(
            "\nNo se pudo crear histograma_paralelo.dat\n"
        );

        return 1;
    }


    float minimo = temp[0];
    float maximo = temp[N - 1];

    float ancho =
        (maximo - minimo) / CUBETAS;

    if (ancho == 0) {
        ancho = 1;
    }


    for (int i = 0; i < CUBETAS; i++) {

        float temperatura =
            minimo + (i * ancho);

        fprintf(
            archivo,
            "%.2f %d\n",
            temperatura,
            histograma[i]
        );
    }

    fclose(archivo);

    FILE *gp =
        popen("gnuplot", "w");

    if (gp == NULL) {

        printf(
            "\nNo se pudo ejecutar gnuplot.\n"
        );

        return 1;
    }


    fprintf(
        gp,
        "set terminal pngcairo size 1400,700 "
        "enhanced font 'Arial,12'\n"
    );

    fprintf(
        gp,
        "set output 'histograma_paralelo.png'\n"
    );

    fprintf(
        gp,
        "set title 'Histograma Paralelo de Temperaturas'\n"
    );

    fprintf(
        gp,
        "set xlabel 'Temperatura (°C)'\n"
    );

    fprintf(
        gp,
        "set ylabel 'Frecuencia'\n"
    );

    fprintf(
        gp,
        "set xrange [-100:100]\n"
    );

    fprintf(
        gp,
        "set xtics -100,20,100\n"
    );

    fprintf(
        gp,
        "set grid ytics\n"
    );

    fprintf(
        gp,
        "set style fill solid 1.0\n"
    );

    fprintf(
        gp,
        "set boxwidth 1.8\n"
    );

    fprintf(
        gp,
        "plot 'histograma_paralelo.dat' using 1:2 "
        "with boxes lc rgb '#2E86DE' "
        "title 'Frecuencia'\n"
    );

    fprintf(
        gp,
        "exit\n"
    );

    pclose(gp);


    printf("\nArchivos generados:\n");
    printf("temperaturas_ordenadas_paralelo.csv\n");
    printf("histograma_paralelo.dat\n");
    printf("histograma_paralelo.png\n");


    return 0;
}