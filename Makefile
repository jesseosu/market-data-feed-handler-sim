# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -pthread

TARGET = market_data_feed_handler_sim

all: $(TARGET)

$(TARGET): market_data_feed_handler_sim.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) market_data_feed_handler_sim.cpp

clean:
	rm -f $(TARGET)
