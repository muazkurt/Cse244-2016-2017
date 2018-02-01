all: exe

exe: main.o List.o
	gcc main.o List.o -std=c99 -o List

main.o: main.c
	gcc -std=c99 -c main.c

List.o: List.c
	gcc -std=c99 -c List.c

clean:
	rm *.o List
