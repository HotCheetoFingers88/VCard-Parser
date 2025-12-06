# Directories
SRC_DIR = src
BIN_DIR = bin
INC_DIR = include

# Files
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared

# Source files
SRCS = $(SRC_DIR)/VCParser.c $(SRC_DIR)/LinkedListAPI.c $(SRC_DIR)/VCHelpers.c
OBJS = $(SRCS:.c=.o)

# Header files
INCLUDES = -I$(INC_DIR)

# Output shared library
LIBRARY = $(BIN_DIR)/libvcparser.so

# Targets

# Default target to create shared library
all: $(LIBRARY)

# Create the shared library
$(LIBRARY): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

# Compile .c files to .o object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Custom target to build parser and the shared library
parser: $(LIBRARY)

# Clean up generated files
clean:
	rm -f $(SRC_DIR)/*.o $(BIN_DIR)/libvcparser.so
