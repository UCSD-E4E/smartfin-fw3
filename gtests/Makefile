CXX = g++
CXXFLAGS = -std=c++17 -g  -I$(HOME)/local/include -DTEST_VERSION=1 -DGTESTING=1 
LDFLAGS = -L$(HOME)/local/lib -lgtest -lgtest_main -pthread

SRC_DIR = ../src
TEST_DIR = ./

SOURCES =  ../tests/scheduler_test_system.cpp $(SRC_DIR)/scheduler.cpp $(TEST_DIR)/scheduler_functions.cpp $(TEST_DIR)/gtest.cpp
OBJECTS = $(SOURCES:.cpp=.o)

OUTPUT = $(TEST_DIR)/test_scheduler
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(OBJECTS)

.PHONY: all clean