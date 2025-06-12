# === Compiler and Flags ===
CXX       := g++
CXXFLAGS  := -std=c++23 -O3 -flto -mavx2

# === Directories ===
SRC_DIR   := src
OBJ_DIR   := build/obj
BIN_DIR   := build

# === Neural Net File ===
DEFAULT_NET := ./episteme_dev_net.bin

ifndef EVALFILE
    EVALFILE := $(DEFAULT_NET)
endif

# Quote the file path for preprocessor definition
CXXFLAGS += -DEVALFILE=\"$(EVALFILE)\"

# === Executable Name (can be overridden) ===
EXE       ?= episteme
TARGET    := $(BIN_DIR)/$(EXE)

# === Source and Object Files ===
SRCS      := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS      := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# === Default Target ===
all: check_net $(TARGET)

# === Check that the net file exists (Git LFS sanity) ===
check_net:
	@if [ ! -f $(EVALFILE) ]; then \
		echo >&2 "Error: NNUE file '$(EVALFILE)' not found."; \
		echo >&2 "Hint: Did you forget to run 'git lfs pull'?"; \
		exit 1; \
	fi

# === Build Target ===
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# === Object File Compilation ===
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< with EVALFILE=$(EVALFILE)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# === Cleanup ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# === Rebuild Everything ===
rebuild: clean all
