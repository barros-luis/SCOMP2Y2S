CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -pthread

# Detect OS and set appropriate flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS - no rt library needed
    LDFLAGS = 
else
    # Linux - need rt library
    LDFLAGS = -lrt
endif

WRITER = writer
READER = reader
WRITER_SRC = writer.c
READER_SRC = reader.c

.PHONY: all clean run_writer run_reader

all: $(WRITER) $(READER)

$(WRITER): $(WRITER_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(WRITER) $(WRITER_SRC)

$(READER): $(READER_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(READER) $(READER_SRC)

run_writer: $(WRITER)
	./$(WRITER)

run_reader: $(READER)
	./$(READER)

clean:
	rm -f $(WRITER) $(READER) 