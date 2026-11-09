CXX      ?= c++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -pedantic -O2
CPPFLAGS += -Iinclude

LIB_SRC  := src/Ride.cpp src/RideTypes.cpp src/Driver.cpp src/Rider.cpp src/RideSharingSystem.cpp
BUILD    := build

.PHONY: all demo test clean

all: $(BUILD)/rideshare $(BUILD)/tests

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/rideshare: $(LIB_SRC) src/main.cpp include/*.h | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIB_SRC) src/main.cpp -o $@

$(BUILD)/tests: $(LIB_SRC) tests/test_rideshare.cpp include/*.h | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIB_SRC) tests/test_rideshare.cpp -o $@

demo: $(BUILD)/rideshare
	./$(BUILD)/rideshare

test: $(BUILD)/tests
	./$(BUILD)/tests

clean:
	rm -rf $(BUILD)
