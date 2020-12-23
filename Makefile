# binary file
EXEC=bin

# folders
BIN_DIR=bin
SRC_DIR=source
INCLUDE_DIR=include

# compiler
CC=gcc

# compiler flags
CFLAGS=-Wall -g -I $(INCLUDE_DIR)/

# compiler command
COMPILE=$(CC) $(CFLAGS) -c $^ -o $@

# get .c files in source
SRC_FILES=$(wildcard $(SRC_DIR)/*.c)

# get name of .o files in source
SRC_OBJS=$(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

# Create src object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILE)

.PHONY: all build clean clean-src

all: build

build: $(SRC_OBJS)
	$(CC) $^ -o $(BIN_DIR)/$(EXEC)

clean-src:
	$(RM) $(SRC_OBJS)

clean:
	rm -rf $(BIN_DIR)/*