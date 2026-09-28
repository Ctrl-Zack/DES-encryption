ifeq ($(shell uname -o 2>/dev/null), Android)
    CXX ?= clang++
else
    CXX ?= g++-13
endif

CXXFLAGS = -std=c++23 -Wall -Wextra -O3

ifdef file
    SRC = $(file)
    TARGET = $(basename $(file))
else
    SRC = main.cpp
    TARGET = main
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean run

# make file=ENCRYPT.cpp CXX=g++-13