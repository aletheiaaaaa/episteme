CXX       := g++
CXXFLAGS  := -std=c++23 -O3 -flto -mavx2

SRC_DIR   := src
OBJ_DIR   := build/obj
BIN_DIR   := build

DEFAULT_NET := ./episteme_dev_net.bin
EVALFILE    ?= $(DEFAULT_NET)

CXXFLAGS  += -DEVALFILE=\"$(EVALFILE)\"

EXE     ?= episteme
TARGET  := $(BIN_DIR)/$(EXE)

SRCS    := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< with EVALFILE=$(EVALFILE)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

rebuild: clean all
