all: exe

exe: s.o c.o sh.o
	gcc -o timeServer timeServer.o -lm

s.o: s.c
	gcc -c *.c

c.o: c.c
	gcc -o seeWhat seeWhat.o -lm

sh.o: sh.c
	gcc -o showResult showResult.o -lm

clean:
	rm *.o seeWhat timeServer showResult
