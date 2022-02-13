all: Listdir

Listdir: main.o Listdir.o
	gcc .links/main.o .links/Listdir.o -o .exe/Listdir

main.o: links main.c
	gcc -Iheader -c main.c -o .links/main.o

Listdir.o: links src/Listdir.c
	gcc -Iheader -c src/Listdir.c -o .links/Listdir.o

links: clean
	mkdir -p .links .exe

clean:
	rm -r .links .exe | true
