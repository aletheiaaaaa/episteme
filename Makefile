# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -O3 -flto -mavx2

# Directories
SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build

EVAL_DIR = ./episteme_dev_net.bin

ifndef EVALFILE
	EVALFILE = $(EVAL_DIR)
	DOWNLOAD_NET = true
endif

# Executable (can be overridden: make EXE=custom_name)
EXE ?= episteme
TARGET = $(BIN_DIR)/$(EXE)

# Source and object files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link object files
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Add the macro definition directly to CXXFLAGS
CXXFLAGS += -DEVALFILE=\"$(EVALFILE)\"

# Compile sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rebuild everything
rebuild: clean all
