# === Compiler & Optimization ===
CXX       := g++
CXXFLAGS  := -std=c++23 -O3 -flto -mavx2

# === Paths ===
SRC_DIR   := src
OBJ_DIR   := build/obj
BIN_DIR   := build

# === Neural Network Path (can be overridden by EVALFILE env var) ===
DEFAULT_NET := ./episteme_dev_net.bin
EVALFILE    ?= $(DEFAULT_NET)

# Define macro for compiler
CXXFLAGS  += -DEVALFILE=\"$(EVALFILE)\"

# === Executable Name (default or from OpenBench via EXE=...) ===
EXE     ?= episteme
TARGET  := $(BIN_DIR)/$(EXE)

# === Source & Object Lists ===
SRCS    := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# === Link ===
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# === Compile Objects ===
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< with EVALFILE=$(EVALFILE)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# === Clean ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# === Full Rebuild ===
rebuild: clean all
