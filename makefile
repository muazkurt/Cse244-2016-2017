all: grepTH

grepTH: main.o grepTH.o
	gcc .links/main.o .links/grepTH.o -o .exe/grepTH -lpthread

main.o: links main.c
	gcc -Iheader -c main.c -o .links/main.o

grepTH.o: links src/grepTH.c
	gcc -Iheader -c src/grepTH.c -o .links/grepTH.o

links: clean
	mkdir -p .links .exe

clean:
	rm -r .links .exe | true
