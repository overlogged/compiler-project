%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring> // strerror
#include <string>
#include "parser.hpp"
#include "driver.h"
%}


%{
// Pacify warnings in yy_init_buffer (observed with Flex 2.6.4)
// and GCC 6.4.0, 7.3.0.
#if defined __GNUC__ && !defined __clang__ && 6 <= __GNUC__
# pragma GCC diagnostic ignored "-Wnull-dereference"
#endif

// Of course, when compiling C as C++, expect warnings about NULL.
#if defined __clang__
# pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
# pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#define FLEX_VERSION (YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION)

// Old versions of Flex (2.5.35) generate an incomplete documentation comment.
//
//  In file included from src/scan-code-c.c:3:
//  src/scan-code.c:2198:21: error: empty paragraph passed to '@param' command
//        [-Werror,-Wdocumentation]
//   * @param line_number
//     ~~~~~~~~~~~~~~~~~^
//  1 error generated.
#if FLEX_VERSION < 206 && defined __clang__
# pragma clang diagnostic ignored "-Wdocumentation"
#endif

// Old versions of Flex (2.5.35) use 'register'.  Warnings introduced in
// GCC 7 and Clang 6.
#if FLEX_VERSION < 206
# if defined __clang__ && 6 <= __clang_major__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
# elif defined __GNUC__ && 7 <= __GNUC__
#  pragma GCC diagnostic ignored "-Wregister"
# endif
#endif

#if FLEX_VERSION < 206
  # if defined __clang__
  #  pragma clang diagnostic ignored "-Wconversion"
  #  pragma clang diagnostic ignored "-Wdocumentation"
  #  pragma clang diagnostic ignored "-Wshorten-64-to-32"
  #  pragma clang diagnostic ignored "-Wsign-conversion"
  # elif defined __GNUC__
  #  pragma GCC diagnostic ignored "-Wconversion"
  #  pragma GCC diagnostic ignored "-Wsign-conversion"
  # endif
#endif
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