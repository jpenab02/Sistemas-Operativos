/* matrizproc.c
   Compilar:
     gcc -Wall -pthread -o matrizproc matrizproc.c

   Uso:
     ./matrizproc m u

   Donde:
     m = tamaño de la matriz (m x m), entero > 0
     u = "sumar" o "max"

   Nota:
     Cambia GROUP_NUM por el número de tu grupo.
     p = GROUP_NUM + 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#ifndef GROUP_NUM
#define GROUP_NUM 3
#endif

typedef enum {
    OP_SUMAR,
    OP_MAX
} operation_t;

typedef struct {
    long start_row;
    long end_row;
    long m;
    int **mat;
    int *results;
    operation_t op;
} thread_arg_t;

static void free_matrix(int **mat, long m)
{
    if (mat == NULL) return;

    for (long i = 0; i < m; i++) {
        free(mat[i]);
    }
    free(mat);
}

static void *process_rows(void *arg)
{
    thread_arg_t *t = (thread_arg_t *)arg;

    for (long fila = t->start_row; fila < t->end_row; fila++) {
        int resultado;

        if (t->op == OP_SUMAR) {
            resultado = 0;
        } else {
            resultado = t->mat[fila][0];
        }

        for (long col = 0; col < t->m; col++) {
            int val = t->mat[fila][col];

            if (t->op == OP_SUMAR) {
                resultado += val;
            } else if (val > resultado) {
                resultado = val;
            }
        }

        t->results[fila] = resultado;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Uso correcto: %s <m> <sumar|max>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    long m = strtol(argv[1], &endptr, 10);

    if (*endptr != '\0' || m <= 0) {
        fprintf(stderr, "Error: m debe ser un entero positivo\n");
        return EXIT_FAILURE;
    }

    operation_t op;
    if (strcmp(argv[2], "sumar") == 0) {
        op = OP_SUMAR;
    } else if (strcmp(argv[2], "max") == 0) {
        op = OP_MAX;
    } else {
        fprintf(stderr, "Error: u debe ser \"sumar\" o \"max\"\n");
        fprintf(stderr, "Uso correcto: %s <m> <sumar|max>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int p = GROUP_NUM + 2;
    if (p <= 0) {
        p = 1;
    }

    int **mat = malloc(sizeof(int *) * m);
    if (mat == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (long i = 0; i < m; i++) {
        mat[i] = malloc(sizeof(int) * m);
        if (mat[i] == NULL) {
            perror("malloc");
            free_matrix(mat, i);
            return EXIT_FAILURE;
        }
    }

    int *results = calloc(m, sizeof(int));
    if (results == NULL) {
        perror("calloc");
        free_matrix(mat, m);
        return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));

    for (long i = 0; i < m; i++) {
        for (long j = 0; j < m; j++) {
            mat[i][j] = rand() % 10;
        }
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * p);
    thread_arg_t *targs = malloc(sizeof(thread_arg_t) * p);

    if (threads == NULL || targs == NULL) {
        perror("malloc");
        free(threads);
        free(targs);
        free(results);
        free_matrix(mat, m);
        return EXIT_FAILURE;
    }

    long base = m / p;
    long resto = m % p;
    long current_row = 0;

    for (int i = 0; i < p; i++) {
        long rows_for_this = base + (i < resto ? 1 : 0);

        targs[i].start_row = current_row;
        targs[i].end_row = current_row + rows_for_this;
        targs[i].m = m;
        targs[i].mat = mat;
        targs[i].results = results;
        targs[i].op = op;

        if (pthread_create(&threads[i], NULL, process_rows, &targs[i]) != 0) {
            fprintf(stderr, "Error: no se pudo crear el hilo %d\n", i);

            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            free(threads);
            free(targs);
            free(results);
            free_matrix(mat, m);
            return EXIT_FAILURE;
        }

        current_row += rows_for_this;
    }

    for (int i = 0; i < p; i++) {
        pthread_join(threads[i], NULL);
    }

    for (long i = 0; i < m; i++) {
        for (long j = 0; j < m; j++) {
            printf("%d ", mat[i][j]);
        }
        printf("%d\n", results[i]);
    }

    free(threads);
    free(targs);
    free(results);
    free_matrix(mat, m);

    return EXIT_SUCCESS;
}
