/* matrizproc.c
   Compilar:
     gcc -Wall -pthread -o matrizproc matrizproc.c

   Uso:
     ./matrizproc m u
       - m : tamaño de la matriz (m x m), entero > 0
       - u : "sumar"  -> calcular suma por fila
             "max"    -> calcular máximo por fila

     Opcional (para definir p = grupo + 2):
     1) pasar grupo como tercer argumento:
        ./matrizproc m u grupo_num
     2) o definir variable de entorno GROUP_NUM:
        export GROUP_NUM=3
        ./matrizproc m u

   Nota:
     Si no se pasa grupo por argumento ni existe GROUP_NUM, se usará grupo=3
     (por tanto p = 5) como valor por defecto.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

typedef enum { OP_SUMAR, OP_MAX } operation_t;

typedef struct {
    int thread_id;      // id lógico del hilo (0..p-1)
    long start_row;     // fila inicial (inclusive)
    long end_row;       // fila final (exclusive)
    long m;             // tamaño de la matriz (m x m)
    int **mat;          // puntero a la matriz
    operation_t op;     // operación a realizar
    pthread_mutex_t *print_mutex; // mutex para imprimir sin intercalado
} thread_arg_t;

/* Función que procesa las filas asignadas al hilo */
void *process_rows(void *arg) {
    thread_arg_t *t = (thread_arg_t*) arg;

    // Buffer temporal para formar cada línea antes de imprimir
    // Cada fila imprimirá m números + resultado -> estimamos 20*m bytes (suficiente)
    size_t bufsize = (size_t)(20 * t->m + 100);
    char *buffer = malloc(bufsize);
    if (!buffer) {
        perror("malloc buffer");
        pthread_exit(NULL);
    }

    for (long fila = t->start_row; fila < t->end_row; ++fila) {
        // Calcular suma o máximo
        int acc = (t->op == OP_MAX) ? t->mat[fila][0] : 0;

        // Construir la línea: "<fila>: v0 v1 ... vm-1 -> resultado\n"
        size_t pos = 0;
        pos += snprintf(buffer + pos, bufsize - pos, "Hilo %d - fila %ld: ", t->thread_id, fila);

        for (long col = 0; col < t->m; ++col) {
            int val = t->mat[fila][col];
            if (t->op == OP_SUMAR) acc += val;
            else if (val > acc) acc = val;

            pos += snprintf(buffer + pos, bufsize - pos, "%d ", val);

            // Prevención mínima si buffer se llena (no debería ocurrir con estimación)
            if (pos + 50 >= bufsize) break;
        }

        pos += snprintf(buffer + pos, bufsize - pos, "-> %d\n", acc);

        // Imprimir la línea de forma atómica (bloqueando mutex)
        pthread_mutex_lock(t->print_mutex);
        fputs(buffer, stdout);
        fflush(stdout);
        pthread_mutex_unlock(t->print_mutex);
    }

    free(buffer);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Uso: %s m u [grupo_num]\n", argv[0]);
        fprintf(stderr, "   u: \"sumar\" o \"max\"\n");
        return EXIT_FAILURE;
    }

    // Parsear m
    char *endptr;
    long m = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || m <= 0) {
        fprintf(stderr, "Error: m debe ser entero > 0\n");
        return EXIT_FAILURE;
    }

    // Parsear operación
    operation_t op;
    if (strcmp(argv[2], "sumar") == 0) op = OP_SUMAR;
    else if (strcmp(argv[2], "max") == 0) op = OP_MAX;
    else {
        fprintf(stderr, "Error: u debe ser \"sumar\" o \"max\"\n");
        return EXIT_FAILURE;
    }

    // Determinar grupo_num: preferir argumento opcional; sino variable de entorno; sino 3 por defecto
    long grupo_num = 3;
    if (argc == 4) {
        grupo_num = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0' || grupo_num < 0) {
            fprintf(stderr, "Error: grupo_num debe ser entero >= 0\n");
            return EXIT_FAILURE;
        }
    } else {
        char *env = getenv("GROUP_NUM");
        if (env) {
            grupo_num = strtol(env, &endptr, 10);
            if (*endptr != '\0' || grupo_num < 0) {
                fprintf(stderr, "Warning: GROUP_NUM no válido, usando 3 por defecto\n");
                grupo_num = 3;
            }
        }
    }

    int p = (int)(grupo_num + 2); // número de hilos
    if (p < 1) p = 1;

    // Crear matriz m x m dinámicamente
    int **mat = malloc(sizeof(int*) * m);
    if (!mat) { perror("malloc mat"); return EXIT_FAILURE; }
    for (long i = 0; i < m; ++i) {
        mat[i] = malloc(sizeof(int) * m);
        if (!mat[i]) {
            perror("malloc mat[i]");
            for (long k = 0; k < i; ++k) free(mat[k]);
            free(mat);
            return EXIT_FAILURE;
        }
    }

    // Llenar matriz con valores aleatorios 0..9
    srand((unsigned)(time(NULL) ^ getpid()));
    for (long i = 0; i < m; ++i)
        for (long j = 0; j < m; ++j)
            mat[i][j] = rand() % 10;

    // Calculo reparto de filas: q = m / p, r = m % p
    long q = m / p;
    long r = m % p; // los primeros r hilos reciben (q+1) filas

    // Crear p hilos
    pthread_t *threads = malloc(sizeof(pthread_t) * p);
    thread_arg_t *targs = malloc(sizeof(thread_arg_t) * p);
    pthread_mutex_t print_mutex;
    pthread_mutex_init(&print_mutex, NULL);

    long current_row = 0;
    for (int i = 0; i < p; ++i) {
        long rows_for_this = q + (i < r ? 1 : 0);
        long start = current_row;
        long end = start + rows_for_this;
        if (start >= m) { start = end = m; } // hilos sin filas si p>m

        targs[i].thread_id = i;
        targs[i].start_row = start;
        targs[i].end_row = end;
        targs[i].m = m;
        targs[i].mat = mat;
        targs[i].op = op;
        targs[i].print_mutex = &print_mutex;

        int rc = pthread_create(&threads[i], NULL, process_rows, &targs[i]);
        if (rc != 0) {
            fprintf(stderr, "Error: pthread_create %d\n", rc);
            // unir hilos ya creados y limpiar
            for (int j = 0; j < i; ++j) pthread_join(threads[j], NULL);
            for (long mm = 0; mm < m; ++mm) free(mat[mm]);
            free(mat); free(threads); free(targs);
            pthread_mutex_destroy(&print_mutex);
            return EXIT_FAILURE;
        }

        current_row = end;
    }

    // Esperar a los hilos
    for (int i = 0; i < p; ++i) pthread_join(threads[i], NULL);

    // Liberar memoria
    for (long i = 0; i < m; ++i) free(mat[i]);
    free(mat);
    free(threads);
    free(targs);
    pthread_mutex_destroy(&print_mutex);

    return EXIT_SUCCESS;
}
