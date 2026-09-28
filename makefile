ifeq ($(shell uname -o 2>/dev/null), Android)
    CXX ?= clang++
else
    CXX ?= g++-13
endif

CXXFLAGS = -std=c++23 -Wall -Wextra -O3 -Iinclude

ifdef file
    SRC = $(file)
    TARGET = bin/$(basename $(notdir $(file)))
else
    SRC = server.cpp
    TARGET = bin/server
endif

all: bin $(TARGET)

bin:
	@mkdir -p bin

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

server:
	$(CXX) $(CXXFLAGS) server.cpp -o bin/server

client:
	$(CXX) $(CXXFLAGS) client.cpp -o bin/client

run-server: server
	./bin/server

run-client: client
	./bin/client

clean:
	rm -rf bin/

.PHONY: all server client run-server run-client clean