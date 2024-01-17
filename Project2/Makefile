all: serverk clientk

serverk: disk.c globals.c hash_table.c main.c mq.c server.c
	gcc -Wall -o serverk disk.c globals.c hash_table.c main.c mq.c server.c -lrt

clientk: clientk.c global_clientk.c util_clientk.c
	gcc -Wall -o clientk clientk.c global_clientk.c util_clientk.c -lm	

clean:
	rm -fr *~ serverk clientk
