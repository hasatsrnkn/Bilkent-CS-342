all: primeP primeT

primeP: main.c util.c process.c globals.c
	gcc -Wall -o primeP main.c util.c process.c globals.c -lrt -lm

primeT: primeT.c util.c globals.c
	gcc -Wall -o primeT primeT.c util.c globals.c -lm -pthread	

clean: 
	rm -fr *~ primeP primeT
