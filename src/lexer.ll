%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring> // strerror
#include <string>
#include "parser.hpp"
#include "driver.h"
%}

%option noyywrap nounput noinput batch debug

id    [a-zA-Z][a-zA-Z_0-9]*
blank [ \t\r]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}
%%

%{
    // A handy shortcut to the location held by the driver.
    yy::location& loc = drv.location;
    // Code run each time yylex is called.
    loc.step ();
%}


{blank}+    loc.step ();
\n+         loc.lines (yyleng); loc.step ();

"{"         return yy::parser::make_LANGLE (loc);
"}"         return yy::parser::make_RANGLE (loc);
"["         return yy::parser::make_LSQUARE (loc);  
"]"         return yy::parser::make_RSQUARE (loc);  
"("         return yy::parser::make_LPAREN (loc);
")"         return yy::parser::make_RPAREN (loc);
":"         return yy::parser::make_COLON (loc);
","         return yy::parser::make_COMMA (loc);
"fn"        return yy::parser::make_KW_FN (loc);
{id}        return yy::parser::make_IDENTIFIER (yytext, loc);
.           {
                throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
}
<<EOF>>    return yy::parser::make_END (loc);
%%

void driver::scan_begin()
{
    yy_flex_debug = trace_scanning;
    if (file.empty() || file == "-")
        yyin = stdin;
    else if (!(yyin = fopen(file.c_str(), "r")))
    {
        std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
}

void driver::scan_end()
{
    fclose(yyin);
}