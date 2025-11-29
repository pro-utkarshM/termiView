CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -std=c99 -Iinclude
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS) -O2
FFTW_LIBS = $(shell pkg-config --libs fftw3)
LDFLAGS = -lm $(FFTW_LIBS)

SRCDIR = src
INCDIR = include
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
TARGET = termiView
TARGET_DEBUG = termiView-debug

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

# Default target: release build
all: $(TARGET)

# Release build
$(TARGET): CFLAGS := $(CFLAGS_RELEASE)
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Debug build
debug: CFLAGS := $(CFLAGS_DEBUG)
debug: clean $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET_DEBUG) $(LDFLAGS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Install target
install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)..."
	@mkdir -p $(BINDIR)
	@install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	@echo "Installation complete!"
	@echo "Run 'termiView --help' to get started"

# Uninstall target
uninstall:
	@echo "Uninstalling $(TARGET) from $(BINDIR)..."
	@rm -f $(BINDIR)/$(TARGET)
	@echo "Uninstall complete!"

# Clean build artifacts
clean:
	rm -f $(SRCDIR)/*.o $(TARGET) $(TARGET_DEBUG) tests/image_processing_test tests/frequency_test tests/filters_test

# Clean everything including output files
distclean: clean
	rm -f *.txt

# Run tests
test: test_image_processing test_frequency test_filters
	@echo "Running basic integration tests..."
	@./$(TARGET) --version
	@./$(TARGET) --help > /dev/null
	@./$(TARGET) assets/kitty.jpeg -w 40 -h 20 -o test_output.txt
	@echo "Integration tests passed!"

test_image_processing: src/image_processing.o
	$(CC) $(CFLAGS) -Itests tests/image_processing_test.c src/image_processing.o -o tests/image_processing_test $(LDFLAGS)
	@./tests/image_processing_test

test_frequency: src/frequency.o src/image_processing.o
	$(CC) $(CFLAGS) -Itests tests/frequency_test.c src/frequency.o src/image_processing.o -o tests/frequency_test $(LDFLAGS)
	@./tests/frequency_test

test_filters: src/filters.o src/image_processing.o
	$(CC) $(CFLAGS) -Itests tests/filters_test.c src/filters.o src/image_processing.o -o tests/filters_test $(LDFLAGS)
	@./tests/filters_test

.PHONY: all debug install uninstall clean distclean test test_image_processing test_frequency test_filters
