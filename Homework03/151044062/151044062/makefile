all: exe

exe: main.o Listdir.o
	gcc main.o Listdir.o -o exe

main.o: main.c
	gcc -c -g main.c

Listdir.o: Listdir.c
	gcc -c -g Listdir.c

clean:
	rm *.o exe log.log
