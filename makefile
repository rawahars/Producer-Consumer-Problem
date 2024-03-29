# Harsh Rawat, harsh-rawat, hrawat2
# Sidharth Gurbani, gurbani, gurbani
#
# This is the makefile for Assignment 2 of CS537 in Fall 2020 offering
# Run "make all" to build the source code
# The executable file is named prodcom
#
#
PROGNAME = prodcom
CC      = gcc
CFLAGS = -Wall -pedantic -Wextra
LDFLAGS = -pthread
OBJECTS = main.o Queue.o Threads.o statistics.o Error.o
SCAN_BUILD_DIR = scan-build-out

all: clean $(PROGNAME)

$(PROGNAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGNAME) $(OBJECTS)

main.o: main.c Queue.h Threads.h Error.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c main.c

statistics.o: statistics.c statistics.h Error.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c statistics.c

Queue.o: Queue.c Queue.h statistics.h Error.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c Queue.c

Threads.o: Threads.c Threads.h Queue.h Error.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c Threads.c

Error.o: Error.c Error.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c Error.c

clean:
	rm -f $(OBJECTS) $(PROGNAME)
	rm -rf $(SCAN_BUILD_DIR)

#
# Run the Clang Static Analyzer
#
scan-build: clean
	scan-build -o $(SCAN_BUILD_DIR) make $(PROGNAME)

#
# View the one scan available using firefox
#
scan-view: scan-build
	firefox -new-window $(SCAN_BUILD_DIR)/*/index.html