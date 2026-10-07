CXX = g++
CXXFLAGS = -std=c++17 -Wall -I core/include -I database/include
LDFLAGS = -lsqlite3

SRC = $(wildcard src/*.cpp) database/src/db.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = hoot

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
