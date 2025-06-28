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

TARGET = diff
SRC = different_solution.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) 