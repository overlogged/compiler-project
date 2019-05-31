# This Makefile is designed to be simple and readable.  It does not
# aim at portability.  It requires GNU Make.

BASE = bin/main
CXX = g++
FLEX = flex
CXXFLAGS = -g -I$(shell llvm-config --includedir) -std=c++17
LLVMLIBS = support core irreader executionengine interpreter mc mcjit bitwriter x86codegen target
LDFLAGS = $(shell llvm-config --ldflags --libs $(LLVMLIBS)) $(shell llvm-config --system-libs) -lffi

all: bin $(BASE)

.PHONY: bin
bin:
	mkdir -p bin

src/parser.cpp src/parser.hpp: src/parser.yy
	bison -d --output=src/parser.cpp $<

src/lexer.cpp: src/lexer.ll
	flex -o $@ $<

bin/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BASE): bin/main.o bin/parser.o bin/lexer.o bin/driver.o bin/utils.o bin/syntax_tree.o bin/function.o bin/codegen_llvm.o bin/type.o bin/syntax_expr.o bin/codegen_llvm_type.o bin/codegen_llvm_fun.o 
	$(CXX) -o $@ $^ $(LDFLAGS)

%.json: %.rs
	bash -c "bin/main $^ > $@"

.PHONY: test clean

test: all bin/main samples/function.json

$(BASE).o: src/parser.hpp
parser.o: src/parser.hpp
lexer.o: src/parser.hpp

clean:
	rm -rf bin src/*.hh src/parser.hpp src/parser.cpp src/lexer.cpp samples/*.json