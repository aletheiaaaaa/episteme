# === Compiler and Flags ===
CXX       := g++
CXXFLAGS  := -std=c++23 -O3 -flto -mavx2

# === Project Structure ===
SRC_DIR   := src
OBJ_DIR   := build/obj
BIN_DIR   := build

# === Neural Net File ===
NET_FILE      := episteme_dev_net.bin
EVALFILE      := ./$(NET_FILE)

# === Executable ===
EXE       ?= episteme
TARGET    := $(BIN_DIR)/$(EXE)

# === Source and Object Files ===
SRCS      := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS      := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# === Preprocessor definition ===
CXXFLAGS  += -DEVALFILE=\"$(EVALFILE)\"

# === Default Target ===
all: check_net $(TARGET)

# === Check NNUE file exists ===
check_net:
	@if [ ! -f $(EVALFILE) ]; then \
		echo >&2 "Error: Neural net file '$(EVALFILE)' not found."; \
		echo >&2 "Did you forget to run 'git lfs pull'?"; \
		exit 1; \
	fi

# === Link object files into executable ===
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# === Compile each source file ===
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# === Clean build artifacts ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# === Force full rebuild ===
rebuild: clean all
