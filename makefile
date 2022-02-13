all: List

List: main.o List.o
	gcc .links/main.o .links/List.o -o .exe/List

main.o: links main.c
	gcc -Iheader -c main.c -o .links/main.o

List.o: links src/List.c
	gcc -Iheader -c src/List.c -o .links/List.o

links: clean
	mkdir -p .links .exe

clean:
	rm -r .links .exe | true
