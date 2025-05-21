# Makefile for Mini Ada Compiler

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g

SRCS = main.cpp LexicalAnalyzer.cpp Parser.cpp SymbolTable.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# In case some .cpp files do not include their corresponding .h files explicitly:
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

LexicalAnalyzer.o: LexicalAnalyzer.cpp LexicalAnalyzer.h
	$(CXX) $(CXXFLAGS) -c LexicalAnalyzer.cpp -o LexicalAnalyzer.o

Parser.o: Parser.cpp Parser.h
	$(CXX) $(CXXFLAGS) -c Parser.cpp -o Parser.o

SymbolTable.o: SymbolTable.cpp SymbolTable.h
	$(CXX) $(CXXFLAGS) -c SymbolTable.cpp -o SymbolTable.o

clean:
	rm -f $(OBJS) $(TARGET)
