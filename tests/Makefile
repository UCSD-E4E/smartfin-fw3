
CXX = g++
CXXFLAGS = -std=c++17 -g -DTEST_VERSION=1


SRC_DIR = ../src
TEST_DIR = ./


SOURCES =  $(TEST_DIR)/scheduler_test_system.cpp $(SRC_DIR)/scheduler.cpp $(SRC_DIR)/ensembles.cpp $(TEST_DIR)/scheduler_test.cpp


OUTPUT = $(TEST_DIR)/test.o

all: $(OUTPUT)


$(OUTPUT): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@


clean:
	rm -f $(OUTPUT)

.PHONY: all clean