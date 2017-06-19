all: grephSh

grephSh: grephSh.o
	gcc -o grephSh grephSh.o -lpthread

grephSh.o:
	gcc -c grephSh.c

clean:
	rm *.o grephSh log.log
