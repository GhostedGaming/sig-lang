# Compiler settings
CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++17 $(shell llvm-config --cxxflags)
LDFLAGS = $(shell llvm-config --ldflags --system-libs --libs core executionengine mcjit interpreter native)

# Directory structure
INCLUDES = -Isrc/lexer/public
SRCDIR = src
BUILDDIR = build

# Source files - update these patterns as needed
SOURCES = $(SRCDIR)/main.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Target
TARGET = compiler

# Default target
all: $(TARGET)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Link target
$(TARGET): $(BUILDDIR) $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

# Compile source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(TARGET)

# Install dependencies (Arch Linux specific)
install-deps:
	sudo pacman -S --needed llvm clang cmake

# Debug build
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Release build
release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)

# Show LLVM configuration
llvm-info:
	@echo "LLVM Version: $(shell llvm-config --version)"
	@echo "LLVM CXXFLAGS: $(shell llvm-config --cxxflags)"
	@echo "LLVM LDFLAGS: $(shell llvm-config --ldflags --system-libs --libs core)"

.PHONY: all clean install-deps debug release llvm-info