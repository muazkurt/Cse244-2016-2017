all: exe

exe: main.o hello.o
	gcc main.o hello.o -o exe

main.o: main.c
	gcc -c main.c

hello.o: hello.c
	gcc -c hello.c

clean:
	rm *.o exe
