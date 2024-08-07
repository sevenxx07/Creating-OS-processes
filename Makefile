CC = gcc
CFLAGS = -Wall
EXTRA_CFLAGS =

NSDSRC = nd.c nsd.c nsd_main.c
NSDOBJ = $(patsubst %.c,%.o,$(NSDSRC))
HEADERS = nd.h nsd.h

#creating all .o files
%.o: %.c
	$(CC) -c -o $@ $< $(EXTRA_CFLAGS) $(CFLAGS)

app: nsd forkpipe
#compiling nsd and forkpipe
nsd: $(NSDOBJ) $(HEADERS)
	$(CC) -o nsd $(NSDOBJ) $(EXTRA_CFLAGS) $(CFLAGS)

forkpipe: forkpipe.o
	$(CC) -o forkpipe forkpipe.c $(EXTRA_CFLAGS) $(CFLAGS)

clean: #$(RM) == rm -f
	$(RM) nsd $(NSDOBJ) osy04.tgz forkpipe.o forkpipe

tar:
	make clean
	tar czf osy04.tgz Makefile *.[ch]



