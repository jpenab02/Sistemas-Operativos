# Makefile para aplicacion "empleados"
# Uso: ver comentarios en "empleados.c"


empleados: entrada.o empleados.o
	gcc -o empleados entrada.o empleados.o
empleados.o: empleados.c empleados.h
	gcc -c empleados.c
entrada.o: entrada.c
	gcc -c entrada.c

clean:
	rm *.o empleados
