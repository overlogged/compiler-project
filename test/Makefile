# This Makefile is designed to be simple and readable.  It does not
# aim at portability.  It requires GNU Make.

BASE = main
BISON = bison
CXX = g++
FLEX = flex
XSLTPROC = xsltproc
CXXFLAGS = -std=c++17 -g
BISONFLAGS = -d
FLEXFLAGS = -+

all: $(BASE)

parser.cpp parser.hpp: parser.yy
	$(BISON) $(BISONFLAGS) --output=parser.cpp $<

lexer.cpp: lexer.ll
	$(FLEX) $(FLEXFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BASE): main.o parser.o lexer.o driver.o utils.o
	$(CXX) -o $@ $^

$(BASE).o: parser.hpp
parser.o: parser.hpp
lexer.o: parser.hpp

clean:
	rm -f *.hh *.out *.o parser.hpp parser.cpp lexer.cpp