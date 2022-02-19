all: Listdir

test: Listdir
	./.exe/Listdir as tests

Listdir: main.o Listdir.o List.o restart.o
	gcc .links/main.o .links/Listdir.o .links/List.o .links/restart.o -o .exe/Listdir

main.o: links main.c
	gcc -Iheader -c main.c -o .links/main.o

Listdir.o: links src/Listdir.c List.o restart.o
	gcc -Iheader -c src/Listdir.c -o .links/Listdir.o

List.o: links src/List.c restart.o
	gcc -Iheader -c src/List.c -o .links/List.o

restart.o: links src/restart.c
	gcc -Iheader -c src/restart.c -o .links/restart.o

links: clean
	mkdir -p .links .exe

clean:
	rm -r .links .exe log.log | true
