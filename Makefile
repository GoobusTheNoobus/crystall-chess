# ======================== Crystall ========================

CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -mbmi2 -g -Isrc

TARGET = crystall.exe

SRCS := $(wildcard src/*.cpp) \
        $(wildcard src/*/*.cpp) \
        $(wildcard src/*/*/*.cpp)

OBJS := $(patsubst src/%.cpp, obj/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj $(TARGET)

.PHONY: all clean