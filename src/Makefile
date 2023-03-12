CC=gcc
CFLAGS=-g -ggdb3 -Wall -Wextra -std=gnu99
EXECUTABLES=run_server
SOURCE=run_server.c httpserver.c httplib.c

all: $(EXECUTABLES)

run_server:: $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $@

clean: 
	rm -f $(EXECUTABLES)
