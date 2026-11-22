CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

TARGET_MAIN = gitlite
TARGET_TESTS = run_tests

ifeq ($(OS),Windows_NT)
    TARGET_MAIN := $(TARGET_MAIN).exe
    TARGET_TESTS := $(TARGET_TESTS).exe
    RM = del /Q /F
else
    RM = rm -f
endif

all: $(TARGET_MAIN) $(TARGET_TESTS)

$(TARGET_MAIN): src/main.cpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(TARGET_MAIN)

$(TARGET_TESTS): tests/test_runner.cpp
	$(CXX) $(CXXFLAGS) tests/test_runner.cpp -o $(TARGET_TESTS)

test: $(TARGET_TESTS)
	./$(TARGET_TESTS)

clean:
	$(RM) $(TARGET_MAIN) $(TARGET_TESTS)

.PHONY: all test clean
