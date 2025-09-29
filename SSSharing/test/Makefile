# The compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20

# Linker flags
LDFLAGS = -lntl -lgmp -lm  -lboost_program_options

# Name of the output file
OUTPUT = SSSharing.o

# All the input files
INPUT = SSSharing.cpp

# The main rule
all: 
	$(CXX) $(INPUT) -o $(OUTPUT) $(LDFLAGS)

# A rule to clean up generated files
clean:
		rm -f $(OUTPUT) *.o