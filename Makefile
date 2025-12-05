CC              = gcc
SRCDIR          = src
INCDIR          = include
PREFIX         ?= /usr/local
BINDIR          = $(PREFIX)/bin
MANDIR          = $(PREFIX)/share/man/man1

# Base C flags
CFLAGS_BASE     = -Wall -Wextra -Wno-unused-parameter -std=c99 -I$(INCDIR)
CFLAGS_DEBUG    = $(CFLAGS_BASE) -g -DDEBUG
CFLAGS_RELEASE  = $(CFLAGS_BASE) -O2

# Libraries
FFTW_LIBS       = $(shell pkg-config --libs fftw3)
LDFLAGS         = -lm $(FFTW_LIBS) -lavformat -lavcodec -lswscale -lavutil

# Sources / objects
SOURCES         = $(wildcard $(SRCDIR)/*.c)
OBJECTS         = $(SOURCES:.c=.o)

TARGET          = termiView
TARGET_DEBUG    = termiView-debug

# Default target: release build
all: $(TARGET)

# Release build
$(TARGET): CFLAGS = $(CFLAGS_RELEASE)
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: clean $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET_DEBUG) $(LDFLAGS)

# Generic compile rule
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
	rm -f $(SRCDIR)/*.o $(TARGET) $(TARGET_DEBUG) \
	      tests/image_processing_test tests/frequency_test \
	      tests/filters_test tests/compression_test tests/video_processing_test

# Clean everything including output files
distclean: clean
	rm -f *.txt

# ---- Tests ----

test: test_image_processing test_frequency test_filters test_compression test_video_processing
	@echo "Running basic integration tests..."
	@./$(TARGET) --version
	@./$(TARGET) --help > /dev/null
	@./$(TARGET) assets/kitty.jpeg -w 40 -h 20 -o test_output.txt
	@echo "Integration tests passed!"

test_image_processing: $(SRCDIR)/image_processing.o
	$(CC) $(CFLAGS_BASE) -Itests tests/image_processing_test.c \
	      $(SRCDIR)/image_processing.o -o tests/image_processing_test $(LDFLAGS)
	@./tests/image_processing_test

test_frequency: $(SRCDIR)/frequency.o $(SRCDIR)/image_processing.o
	$(CC) $(CFLAGS_BASE) -Itests tests/frequency_test.c \
	      $(SRCDIR)/frequency.o $(SRCDIR)/image_processing.o \
	      -o tests/frequency_test $(LDFLAGS)
	@./tests/frequency_test

test_filters: $(SRCDIR)/filters.o $(SRCDIR)/image_processing.o
	$(CC) $(CFLAGS_BASE) -Itests tests/filters_test.c \
	      $(SRCDIR)/filters.o $(SRCDIR)/image_processing.o \
	      -o tests/filters_test $(LDFLAGS)
	@./tests/filters_test

test_compression: $(SRCDIR)/compression.o $(SRCDIR)/image_processing.o
	$(CC) $(CFLAGS_BASE) -Itests tests/compression_test.c \
	      $(SRCDIR)/compression.o $(SRCDIR)/image_processing.o \
	      -o tests/compression_test $(LDFLAGS)
	@./tests/compression_test

test_video_processing: $(SRCDIR)/video_processing.o $(SRCDIR)/image_processing.o
	$(CC) $(CFLAGS_BASE) -Itests tests/video_processing_test.c \
	      $(SRCDIR)/video_processing.o $(SRCDIR)/image_processing.o \
	      -o tests/video_processing_test $(LDFLAGS)
	@./tests/video_processing_test

.PHONY: all debug install uninstall clean distclean test \
        test_image_processing test_frequency test_filters \
        test_compression test_video_processing
