CXX := g++

CXXFLAGS := -std=c++23 \
	    -Wall -Wextra -Wpedantic -Werror \
	    -g \
	    -Iinclude

SANITIZERS := -fsanitize=address,undefined

SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj

PROGRAM := $(BIN_DIR)/rv32i_interpreter

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: clean

.DELETE_ON_ERROR:

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(SANITIZERS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZERS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BIN_DIR)

