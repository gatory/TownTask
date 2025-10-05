# Cross-platform Raylib Makefile
# Supports Windows (MinGW), macOS, and Linux
# Cross-platform Raylib setup credited to Claude.ai

# Detect platform
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Iinclude

# Platform-specific settings
ifeq ($(UNAME_S), Darwin)    # macOS
    LIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXT = 
else ifeq ($(UNAME_S), Linux)    # Linux
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXT = 
else    # Windows (MinGW)
    LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    EXT = .exe
    # Add raylib paths for common Windows installations
    INCLUDES += -I/mingw64/include -I/usr/local/include
    LDFLAGS += -L/mingw64/lib -L/usr/local/lib
endif

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/game$(EXT)

# Find all .cpp files in src directory
# SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
SOURCES = $(shell find $(SRC_DIR) -name "*.cpp")
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link the executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

# Compile source files to object files
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the game
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
