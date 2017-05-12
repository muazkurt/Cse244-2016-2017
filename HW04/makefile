all: grepTH

grepTH: grepTH.o
	gcc -o grepTH grepTH.o -lpthread

grepTH.o:
	gcc -c grepTH.c

clean:
	rm *.o grepTH log.log
