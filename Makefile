# ======================== Crystall ========================

CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -mbmi2 -DNDEBUG 

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

TARGET = crystall_new.exe

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f src/*.o $(TARGET)