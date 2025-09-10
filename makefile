# Define the compiler
CC = gcc

# Define the compiler flags
CFLAGS = -Wall -Iinclude -MMD -MP

# Define the source files
SRCS = $(wildcard src/*.c)

# Define the object files directory
OBJ_DIR = builds/object_files

# Define the object files
OBJS = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Define the dependency files
DEPS = $(patsubst $(OBJ_DIR)/%.o, $(OBJ_DIR)/%.d, $(OBJS))

# Define the output directory and executable name
BUILD_DIR = builds
EXECUTABLE = $(BUILD_DIR)/framework_2

# Define the default target
all: $(EXECUTABLE)
# Include dependency files
-include $(DEPS)

# Rule to create the output directories if they don't exist
$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@

# Rule to link the object files into the executable
$(EXECUTABLE): $(OBJS) | $(BUILD_DIR) $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Rule to compile the source files into object files
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up the build artifacts
clean:
	rm -f $(OBJS) $(EXECUTABLE)

# Rule to remove the output directories and all build artifacts
distclean: clean
	rmdir $(OBJ_DIR) $(BUILD_DIR)

# Define the debug target
debug: CFLAGS += -g
debug: $(EXECUTABLE)

# Phony targets to avoid conflicts with files of the same name
.PHONY: all clean distclean debug