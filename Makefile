CC=gcc
CFLAGS=-Wall -Wextra -g
BUILDDIR=bin/
SOURCEDIR=src/

all: server client

server:
	mkdir -p bin/
	$(CC) $(CFLAGS) -pthread \
		$(SOURCEDIR)server.c $(SOURCEDIR)protocol.c $(SOURCEDIR)list.c \
		-o $(BUILDDIR)$@

client:
	$(CC) -pthread $(SOURCEDIR)client.c -o $(BUILDDIR)$@

clean:
	rm -rf $(BUILDDIR)*
