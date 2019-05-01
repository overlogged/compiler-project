CPPDEBUG = -g -Wall -D DEBUG
CPPFLAGS = -O2 -I include/ -std=c++17 ${CPPDEBUG}

all: compiler

bin/parser.o: src/parser.yy
	bison -d --output=src/parser.tab.cpp src/parser.yy
	g++ $(CPPFLAGS) -c -o bin/parser.o src/parser.tab.cpp

bin/lexer.o: src/lexer.ll
	flex -+ --outfile=src/lexer.yy.cpp $<
	g++ $(CPPFLAGS) -c src/lexer.yy.cpp -o bin/lexer.o

.PHONY: clean
clean:
	rm -rf $(CLEANLIST)