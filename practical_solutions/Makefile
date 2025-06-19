CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
BIN_DIR = bin

# Find all directories named 'ex*' at any depth. These are our exercises.
EXERCISE_DIRS = $(shell find . -type d -name "ex*")

# Create a list of target executables in the bin directory.
# e.g., a source dir of ./model_exam_2425/ex1 becomes a target of bin/ex1
BINS = $(patsubst %,$(BIN_DIR)/%,$(notdir $(EXERCISE_DIRS)))

.PHONY: all clean run

all: $(BINS)

# Create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# This is the build rule. For each target binary (e.g., bin/ex1),
# it finds the corresponding source directory (e.g., ./model_exam_2425/ex1)
# and compiles all .c files found within it.
$(BINS): $(BIN_DIR)/%: | $(BIN_DIR)
	$(eval SOURCE_DIR := $(filter %$(notdir $@),$(EXERCISE_DIRS)))
	$(eval SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.c))
	@if [ -z "$(SOURCE_FILES)" ]; then \
		echo "Warning: No .c files found in $(SOURCE_DIR), skipping build for '$@'."; \
	else \
		echo "==> Compiling $@ from $(SOURCE_DIR)"; \
		$(CC) $(CFLAGS) -o $@ $(SOURCE_FILES); \
	fi

clean:
	rm -rf $(BIN_DIR)

# The 'run' target now shows available exercises if you don't specify one.
run:
	@if [ -z "$(EXERCISE)" ]; then \
		echo "Error: Please specify which exercise to run."; \
		echo "Usage: make run EXERCISE=<exercise_name>"; \
		echo "Available exercises: $(notdir $(EXERCISE_DIRS))"; \
		exit 1; \
	fi
	@if [ ! -f "$(BIN_DIR)/$(EXERCISE)" ]; then \
		echo "==> Executable not found. Building '$(EXERCISE)' first..."; \
		$(MAKE) $(BIN_DIR)/$(EXERCISE); \
	fi
	@echo "==> Running '$(EXERCISE)'..."
	@./$(BIN_DIR)/$(EXERCISE)
