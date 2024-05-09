
CXX = g++
CXXFLAGS = -std=c++17 -g -DTEST_VERSION=1 


SRC_DIR = src
TEST_DIR = tests


SOURCES =  $(TEST_DIR)/scheduler_test_system.cpp $(SRC_DIR)/scheduler.cpp $(TEST_DIR)/scheduler_test.cpp


OUTPUT = $(TEST_DIR)/test

all: $(OUTPUT)


$(OUTPUT): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@


clean:
	rm -f $(OUTPUT)

.PHONY: all clean