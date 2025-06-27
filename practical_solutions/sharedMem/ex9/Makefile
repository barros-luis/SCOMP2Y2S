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

INTERFACE = interface
ENCRYPT = encrypt
DISPLAY = display

.PHONY: all clean run_interface run_encrypt run_display

all: $(INTERFACE) $(ENCRYPT) $(DISPLAY)

$(INTERFACE): interface.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(INTERFACE) interface.c

$(ENCRYPT): encrypt.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(ENCRYPT) encrypt.c

$(DISPLAY): display.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(DISPLAY) display.c

run_interface: $(INTERFACE)
	./$(INTERFACE)

run_encrypt: $(ENCRYPT)
	./$(ENCRYPT)

run_display: $(DISPLAY)
	./$(DISPLAY)

clean:
	rm -f $(INTERFACE) $(ENCRYPT) $(DISPLAY) 