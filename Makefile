CC = clang
CXX = clang++

CFLAGS = -O3 -march=native -Wall -Wextra -std=c11
CXXFLAGS = -O3 -march=native -Wall -Wextra -std=c++17

TARGET = bench
BUILD_DIR = build

U64_SRC = flat_hash_map_u64.c
STR_SRC = flat_hash_map_str.c
CPP_SRC = benchmark.cpp

U64_OBJ = $(BUILD_DIR)/flat_hash_map_u64.o
STR_OBJ = $(BUILD_DIR)/flat_hash_map_str.o
CPP_OBJ = $(BUILD_DIR)/benchmark.o

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(U64_OBJ) $(STR_OBJ) $(CPP_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)


$(U64_OBJ): $(U64_SRC) flat_hash_map_u64.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


$(STR_OBJ): $(STR_SRC) flat_hash_map_str.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


$(CPP_OBJ): $(CPP_SRC) flat_hash_map_u64.h flat_hash_map_str.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

clean-all: clean


rebuild: clean all




