/***********************************************************************
 MATRIZSUM.C
 ----------------------------------------------------------------------
 Programa que crea una matriz cuadrada de tamaño m x m y calcula la
 suma de cada fila utilizando múltiples procesos.

 Cada proceso hijo se encarga de calcular la suma de un conjunto de
 filas de la matriz.

 PARÁMETROS DE ENTRADA
 ----------------------------------------------------------------------
 m : tamaño de la matriz (m x m)
 n : número de procesos hijos

 CONDICIONES
 ----------------------------------------------------------------------
 - m debe ser un entero positivo
 - n debe ser un entero positivo
 - n debe dividir exactamente a m (m % n == 0)

 EJEMPLO DE EJECUCIÓN
 ----------------------------------------------------------------------
 gcc -Wall -o matrizsum matrizsum.c
 ./matrizsum 4 2

 RESULTADO
 ----------------------------------------------------------------------
 Cada proceso imprime:
 - su PID
 - la fila que está procesando
 - los valores de la fila
 - la suma total de la fila

 CONCEPTOS DE SISTEMAS OPERATIVOS UTILIZADOS
 ----------------------------------------------------------------------
 - Creación de procesos con fork()
 - Identificación de procesos con getpid()
 - Sincronización con wait()
***********************************************************************/

#include <stdio.h>      // Funciones de entrada/salida (printf, etc.)
#include <stdlib.h>     // Funciones de utilidad (malloc, rand, exit)
#include <unistd.h>     // Funciones del sistema POSIX (fork, getpid)
#include <sys/types.h>  // Definiciones de tipos usados por procesos
#include <sys/wait.h>   // Función wait() para esperar procesos hijos
#include <time.h>       // Función time() para semilla aleatoria
#include <errno.h>      // Manejo de errores del sistema
#include <string.h>     // Funciones de manejo de strings

int main(int argc, char *argv[])
{
    /**************************************************************
     1. VALIDACIÓN DE ARGUMENTOS
    **************************************************************/

    // El programa debe recibir exactamente 2 argumentos
    // ./matrizsum m n
    if (argc != 3)
    {
        fprintf(stderr, "Uso correcto: %s <m> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;

    // Convertir argumento m de texto a entero
    long m = strtol(argv[1], &endptr, 10);

    // Validar que m sea un número entero positivo
    if (*endptr != '\0' || m <= 0)
    {
        fprintf(stderr, "Error: m debe ser un entero positivo\n");
        return EXIT_FAILURE;
    }

    // Convertir argumento n de texto a entero
    long n = strtol(argv[2], &endptr, 10);

    // Validar que n sea un número entero positivo
    if (*endptr != '\0' || n <= 0)
    {
        fprintf(stderr, "Error: n debe ser un entero positivo\n");
        return EXIT_FAILURE;
    }

    // Verificar que n divida exactamente a m
    if (m % n != 0)
    {
        fprintf(stderr, "Error: n debe ser divisor de m\n");
        return EXIT_FAILURE;
    }

    // Evitar que existan más procesos que filas
    if (n > m)
    {
        fprintf(stderr, "Error: n no puede ser mayor que m\n");
        return EXIT_FAILURE;
    }

    /**************************************************************
     2. CREACIÓN DINÁMICA DE LA MATRIZ
    **************************************************************/

    // Se crea un arreglo de punteros (filas)
    int **mat = malloc(sizeof(int*) * m);

    if (!mat)
    {
        perror("Error al reservar memoria");
        return EXIT_FAILURE;
    }

    // Reservar memoria para cada fila
    for (long i = 0; i < m; i++)
    {
        mat[i] = malloc(sizeof(int) * m);

        if (!mat[i])
        {
            perror("Error al reservar memoria");

            // Liberar memoria previamente reservada
            for (long k = 0; k < i; k++)
                free(mat[k]);

            free(mat);
            return EXIT_FAILURE;
        }
    }

    /**************************************************************
     3. LLENAR MATRIZ CON VALORES ALEATORIOS
    **************************************************************/

    // Inicializar generador de números aleatorios
    srand(time(NULL));

    for (long i = 0; i < m; i++)
    {
        for (long j = 0; j < m; j++)
        {
            // Números entre 0 y 9
            mat[i][j] = rand() % 10;
        }
    }

    /**************************************************************
     4. CÁLCULO DE FILAS POR PROCESO
    **************************************************************/

    // Cada proceso trabajará con m/n filas
    long filas_por_proceso = m / n;

    /**************************************************************
     5. CREACIÓN DE PROCESOS HIJOS
    **************************************************************/

    for (long p = 0; p < n; p++)
    {
        // fork() crea un nuevo proceso hijo
        pid_t pid = fork();

        // Si fork falla
        if (pid < 0)
        {
            fprintf(stderr, "Error al crear proceso\n");
            return EXIT_FAILURE;
        }

        // Código que ejecuta el proceso hijo
        else if (pid == 0)
        {
            // Obtener PID del proceso hijo
            pid_t mypid = getpid();

            // Determinar filas asignadas al proceso
            long inicio = p * filas_por_proceso;
            long fin = inicio + filas_por_proceso;

            /******************************************************
             6. PROCESAMIENTO DE FILAS
            ******************************************************/

            for (long fila = inicio; fila < fin; fila++)
            {
                int suma = 0;

                printf("PID %d - fila %ld: ", mypid, fila);

                // Recorrer columnas de la fila
                for (long col = 0; col < m; col++)
                {
                    printf("%d ", mat[fila][col]);

                    // Acumular suma
                    suma += mat[fila][col];
                }

                // Mostrar suma total de la fila
                printf(" -> suma = %d\n", suma);

                fflush(stdout);
            }

            // El hijo termina su ejecución
            exit(EXIT_SUCCESS);
        }

        // El padre continúa creando más procesos
    }

    /**************************************************************
     7. ESPERAR A QUE TERMINEN LOS PROCESOS HIJOS
    **************************************************************/

    for (long i = 0; i < n; i++)
    {
        wait(NULL);
    }

    /**************************************************************
     8. LIBERAR MEMORIA
    **************************************************************/

    for (long i = 0; i < m; i++)
        free(mat[i]);

    free(mat);

    /**************************************************************
     9. FIN DEL PROGRAMA
    **************************************************************/

    return EXIT_SUCCESS;
}