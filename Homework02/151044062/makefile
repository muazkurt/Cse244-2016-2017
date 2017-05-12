all: exe

exe: main.o Listdir.o
	gcc main.o Listdir.o -o Listdir

main.o: main.c
	gcc -c main.c

List.o: Listdir.c
	gcc -c Listdir.c

clean:
	rm *.o Listdir
