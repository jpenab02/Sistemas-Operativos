
/* archivo empleados.c
 * Ejemplo sencillo de listas con arreglos
 * Uso: empleados archivo 
 * Lee un archivo de empleados con su edad y tiempo de trabajo
 * Imprime los empleados y sus datos leidos del archivo pasado en la
 * linea de comandos, y luego imprime aquellos empleados
 * seleccionados que cumplen con los requisitos minimos de edad y
 * tiempo de trabajo (ver empleados.h)
 * Ejemplo: empleados datos.in
 * Ver archivo de entrada "datos.in" como ejemplo del formato
 *
 * Adaptado por Carlos Figueira, mayo 2013
 */

#include <stdio.h>
#include "empleados.h"

extern int mgetline(char *, int, FILE *);

/* asignar
   inserta un nuevo empleado en la lista en la posicion pos
   e inicializa su edad, tiempo de trabajo y nombre
*/
void asignar(emple *emp, int edad, int tiempo, char *n, int pos)
{
     
  char *p;
   
  emp[pos].edad = edad;
  emp[pos].ttrabajo = tiempo;
  p = emp[pos].nombre;
  while((*p++ = *n++) != '\0');
}

/* imprimir
   imprime los primeros cuantos empleados del arreglo
*/
void  imprimir(emple *emp, int cuantos)
{
  int i;

    printf("Empleados:\n");

  for(i=0; i < cuantos; i++)
    printf("%d %d %s\n",  emp[i].edad, emp[i].ttrabajo, emp[i].nombre);
}

/* seleccionar
 Imprime los empleados que tengan mas de SELMINEDAD años de edad y 
 SELMINTT de trabajo  
 */
void  seleccionar(emple *emp, int cuantos)
{  
  int i;
  printf("\n\nEmpleados mayores de %d an~os de edad y %d o mas an~os de trabajo:\n",SELMINEDAD,SELMINTT);

  for(i=0; i < cuantos; i++){
    if ( emp[i].edad > SELMINEDAD &&  emp[i].ttrabajo > SELMINTT)
      printf("%d %d %s\n",  emp[i].edad, emp[i].ttrabajo, emp[i].nombre);
  }
}

int main(int argc, char *argv[]) 
{
  emple miemp[MAXEMP];
  char line[MAXLIN], nom[MAXNOMBRE];
  FILE *fp;
  int i, edad=0, tiempo=0;
  
  fp = fopen(argv[1], "r");
  i = 0;
  while (mgetline(line, sizeof(line), fp) > 0)  {
    sscanf(line, "%d %d %s", &edad, &tiempo, nom);
    /* printf("valid: %s\n", line); */
    /* printf("%d, %d, %s\n", edad, tiempo, nom); */
    asignar(miemp, edad, tiempo, nom, i++);
  }
  
  imprimir(miemp, i);
  seleccionar(miemp, i);
     
  fclose(fp); 
}
