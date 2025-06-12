CXX       := g++
CXXFLAGS  := -std=c++23 -O3 -flto -mavx2

SRC_DIR   := src
OBJ_DIR   := ./obj
BIN_DIR   := .

<<<<<<< HEAD
DEFAULT_NET := ./episteme_dev_net.bin
EVALFILE    ?= $(DEFAULT_NET)
=======
EVAL_DIR = ./episteme_dev_net.bin
>>>>>>> 61ac67e (hopefully final net fix)

CXXFLAGS  += -DEVALFILE=\"$(EVALFILE)\"

EXE     ?= episteme
TARGET  := $(BIN_DIR)/$(EXE)

SRCS    := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: check_net $(TARGET)

check_net:
	@if [ ! -f $(EVALFILE) ]; then \
		echo >&2 "Error: Neural net file '$(EVALFILE)' not found."; \
		echo >&2 "Hint: Did you run 'git lfs pull'?"; \
		exit 1; \
	fi

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

<<<<<<< HEAD
=======
# Add the macro definition directly to CXXFLAGS
CXXFLAGS += -DEVALFILE=\"$(EVALFILE)\"

# Compile sources
>>>>>>> 61ac67e (hopefully final net fix)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< with EVALFILE=$(EVALFILE)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

rebuild: clean all
