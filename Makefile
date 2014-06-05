CC=gcc
CFLAGS=-Wall -Wextra
BUILDDIR=bin/
SOURCEDIR=src/

all: server

server:
	mkdir -p bin/
	$(CC) $(CFLAGS) -pthread $(SOURCEDIR)server.c $(SOURCEDIR)protocol.c -o $(BUILDDIR)$@

client:
	$(CC) -pthread $(SOURCEDIR)client.c -o $(BUILDDIR)$@

clean:
	rm -rf $(BUILDDIR)*
