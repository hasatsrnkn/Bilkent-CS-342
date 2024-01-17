all: memsim

memsim: file.c globals.c main.c utils.c algorithms.c pagetable.c
	 gcc -Wall -o memsim file.c globals.c main.c utils.c algorithms.c pagetable.c -lrt -lm

clean:
	rm -fr *~ memsim
