CXX       := g++
CXXFLAGS  := -std=c++23 -O3 -flto=auto -mavx2

SRC_DIR   := src
OBJ_DIR   := ./obj
BIN_DIR   := .

DEFAULT_NET := ./128_v2.bin
EVALFILE    ?= $(DEFAULT_NET)

CXXFLAGS  += -DEVALFILE=\"$(EVALFILE)\"

EXE     ?= episteme
TARGET  := $(BIN_DIR)/$(EXE)

SRCS    := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

rebuild: clean all
