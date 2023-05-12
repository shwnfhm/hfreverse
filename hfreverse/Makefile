###############
#
# Makefile for OS-hfreverse
#
###############
CCOPTS = -Wall -Werror

hfreverse: hfreverse.o
	gcc -o hfreverse hfreverse.o

hfreverse.o: hfreverse.c
	gcc $(CCOPTS) -c hfreverse.c

clean:
	/bin/rm -fr hfreverse hfreverse.o tests-out

test:
	./test-reverse.sh

test-all:
	./test-reverse.sh -c
