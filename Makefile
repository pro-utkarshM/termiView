CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -std=c99 -Iinclude
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS) -O2
LDFLAGS = -lm

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
	rm -f $(SRCDIR)/*.o $(TARGET) $(TARGET_DEBUG)

# Clean everything including output files
distclean: clean
	rm -f *.txt

# Run tests
test: $(TARGET)
	@echo "Running basic tests..."
	@./$(TARGET) --version
	@./$(TARGET) --help > /dev/null
	@./$(TARGET) assets/kitty.jpeg -w 40 -h 20 -o test_output.txt
	@echo "Tests passed!"

.PHONY: all debug install uninstall clean distclean test
