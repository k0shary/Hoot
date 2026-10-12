CXX = g++
CXXFLAGS = -std=c++17 -Wall -I core/include -I database/include -I network/include
LDFLAGS = -lsqlite3 -lcurl


CORE_SRCS = core/src/Evaluator.cpp core/src/GeoEngine.cpp core/src/PlacesFetcher.cpp
DB_SRCS = database/src/db.cpp
NETWORK_SRCS = network/src/server.cpp network/src/main.cpp


OBJS = $(CORE_SRCS:.cpp=.o) $(DB_SRCS:.cpp=.o) $(NETWORK_SRCS:.cpp=.o)
TARGET = hoot

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


test_evaluator: tests/test_evaluator.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) tests/test_evaluator.cpp $(OBJS) -o build/$@ $(LDFLAGS)

test_db: tests/test_db.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) tests/test_db.cpp $(OBJS) -o build/$@ $(LDFLAGS)

clean:
	rm -f core/src/*.o database/src/*.o $(TARGET) build/*
