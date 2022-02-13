all: Listdir

Listdir: main.o Listdir.o List.o
	gcc .links/main.o .links/Listdir.o .links/List.o -o .exe/Listdir

main.o: links main.c
	gcc -Iheader -c main.c -o .links/main.o

Listdir.o: links src/Listdir.c List.o
	gcc -Iheader -c src/Listdir.c -o .links/Listdir.o

List.o: links src/List.c
	gcc -Iheader -c src/List.c -o .links/List.o

links: clean
	mkdir -p .links .exe

clean:
	rm -r .links .exe log.log | true
