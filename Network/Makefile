# The compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17

# Linker flags (like -lpthread)
LDFLAGS = -lpthread -lboost_program_options

# Name of the output file
OUTPUT = network.o

# All the input files
INPUT = Client.h Client.cpp Server.h Server.cpp main.cpp

TEST_INPUT = Client.h Client.cpp Server.h Server.cpp test.cpp

TEST_OUTPUT = test.o

# The main rule
all: 
	$(CXX) $(CXXFLAGS) $(INPUT) -o $(OUTPUT) $(LDFLAGS)

debug:
	$(CXX) $(CXXFLAGS) $(INPUT) -o $(OUTPUT) $(LDFLAGS) -g

test:
	$(CXX) $(CXXFLAGS) $(TEST_INPUT) -o $(TEST_OUTPUT) $(LDFLAGS)
  
# A rule to clean up generated files
clean:
		rm -f *.o