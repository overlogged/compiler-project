#pragma once

#include "parse_tree.h"
#include "parser.hpp"
#include <map>
#include <string>

// Give Flex the prototype of yylex we want and
#define YY_DECL yy::parser::symbol_type yylex(driver &drv)

// declare it for the parser's sake.
YY_DECL;

#define register

// Conducting the whole scanning and parsing of Calc++.
class driver
{
  public:
    driver() : trace_parsing(false), trace_scanning(false) {}

    // Run the parser on file F.  Return 0 on success.
    int parse(const std::string &f);
    // The name of the file being parsed.
    std::string file;
    // Whether to generate parser debug traces.
    bool trace_parsing;

    // Handling the scanner.
    void scan_begin();
    void scan_end();
    // Whether to generate scanner debug traces.
    bool trace_scanning;
    // The token's location used by the scanner.
    yy::location location;

    // parse_tree
    node_module parse_tree;
};