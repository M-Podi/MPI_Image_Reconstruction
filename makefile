# Compiler
MPICC = mpicc

# Flags
CFLAGS = -lm

# Source files
SRCS = pgm_IO.c main.c

# Executable name
TARGET = out

# Default target: compile all sources into the executable
all: $(TARGET)

$(TARGET): $(SRCS)
	@if [ -f $(TARGET) ]; then \
		rm -f $(TARGET); \
	fi
	$(MPICC) $(SRCS) -o $(TARGET) $(CFLAGS)

# Run the program
run: $(TARGET)
	mpirun --oversubscribe -np 4 ./$(TARGET) image_640x480.pgm 10

# Clean up
clean:
	rm -f $(TARGET) *.o  # Also remove object files

