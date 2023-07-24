# Compiler and compiler flags
CXX := g++
CXXFLAGS := -std=c++11 -Wall

# Executable name
EXECUTABLE := chip8_emulator

# Default target
all: $(EXECUTABLE)

# Rule to compile the source file and generate the executable
$(EXECUTABLE): main.cpp
	$(CXX) $(CXXFLAGS) $< -lSDL2 -o $@

# Clean build files
clean:
	rm -f $(EXECUTABLE)
