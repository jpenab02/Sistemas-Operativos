#include <stdio.h>

int main()
{
    int entero1;
    int entero2;

    scanf("%d", &entero1);
    scanf("%d", &entero2);

    if (entero1 > entero2)
    {
        printf("%d es mayor que %d", entero1, entero2);
    }
    else if (entero1 < entero2)
    {
        printf("%d es menor que %d", entero1, entero2);
    }
    else
    {
        printf("%d es igual que %d", entero1, entero2);
    }

    return 0;
}
