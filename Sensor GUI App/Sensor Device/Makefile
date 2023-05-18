# CXX = clang++-8
# CXXFLAGS += -std=c++17 -stdlib=libc++
CC = g++
CXXFLAGS += -Wall
# LDFLAGS += -lpthread

TARGET_FILE = device

DEVICE_IP ?= "127.00.00.01"
DEVICE_PORT ?= "8080"

all:
	$(CC) $(CXXFLAGS) $(TARGET_FILE).cpp -o $(TARGET_FILE).o
	./$(TARGET_FILE).o $(DEVICE_IP) $(DEVICE_PORT)

run:
	./$(TARGET_FILE).o -D$(DEVICE_IP) -D$(DEVICE_PORT)

clean:
	rm -f $(TARGET_FILE).o
