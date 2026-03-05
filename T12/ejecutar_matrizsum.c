#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Verificar que el usuario pase los argumentos
    if (argc != 3)
    {
        printf("Uso: %s m n\n", argv[0]);
        return 1;
    }

    printf("Ejecutando programa matrizsum con exec...\n");

    // exec reemplaza este proceso por el programa matrizsum
    execl("./matrizsum", "matrizsum", argv[1], argv[2], NULL);

    // Si exec falla, se ejecuta esta línea
    perror("Error al ejecutar matrizsum");

    return 1;
}
