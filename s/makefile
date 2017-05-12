all: timerServer seeWhat showResult

timerServer: timerServer.o
	gcc -o timerServer timerServer.o -lm

timeServer.o: timerServer.c
	gcc -c timerServer.c

seeWhat: seeWhat.o
	gcc -o seeWhat seeWhat.o -lm

seeWhat.o: seeWhat.c
	gcc -c seeWhat.c

showResult: showResult.o
	gcc -o showResult showResult.o -lm

showResult.o: showResult.c
	gcc -c showResult.c
clean:
	rm *.o seeWhat timerServer showResult
