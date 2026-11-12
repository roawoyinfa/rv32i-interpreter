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
LIB_OBJS := $(filter-out $(OBJ_DIR)/main.o,$(OBJS))
DEPS := $(OBJS:.o=.d)

TEST_DIR = tests
TEST_SRC_DIR = $(TEST_DIR)/integration
TEST_BIN_DIR = $(BIN_DIR)/tests

TEST_SRCS := $(wildcard $(TEST_SRC_DIR)/*.cpp)
TEST_BINS := $(patsubst $(TEST_SRC_DIR)/%.cpp,$(TEST_BIN_DIR)/%,$(TEST_SRCS))

TEST_CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)

.PHONY: clean

.DELETE_ON_ERROR:

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(SANITIZERS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZERS) -MMD -MP -c $< -o $@

test: $(TEST_BINS)

$(TEST_BIN_DIR)/%: $(TEST_SRC_DIR)/%.cpp $(LIB_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CXX) $(TEST_CXXFLAGS) $(SANITIZERS) $< $(LIB_OBJS) -o $@

-include $(DEPS)

clean:
	rm -rf $(BIN_DIR)

