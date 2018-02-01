all: exe

exe: main.o Listdir.o
	gcc main.o Listdir.o -std=c99 -o Listdir

main.o: main.c
	gcc -std=c99 -c main.c

Listdir.o: Listdir.c
	gcc -std=c99 -c Listdir.c

clean:
	rm *.o Listdir
