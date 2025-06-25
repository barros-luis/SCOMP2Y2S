CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -pthread
TARGET = solution
SRC = solution_correct_roles.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) 