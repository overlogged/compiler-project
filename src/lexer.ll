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

id                     [a-zA-Z][a-zA-Z_0-9]*
blank                  [ \t\r]
digit                  [0-9]
non_zero_digit         [1-9]
hex_digit              [0-9a-fA-F]
oct_digit              [0-7]
digit_seq              {digit}+
hex_digit_seq          {hex_digit}+
float_suffix           [fFlL]
int_suffix             ([uU]?([lL]|"ll"|"LL"))|(([lL]|"ll"|"LL")?[uU])
hex_prefix             [0][xX]
dec_float_const        ((({digit_seq}?\.{digit_seq})|({digit_seq}\.)|{digit_seq})(([eE][+-]?{digit_seq})?{float_suffix}?))
hex_float_const        ({hex_prefix}(({hex_digit_seq}?\.{hex_digit_seq})|({hex_digit_seq}\.)|{hex_digit_seq})(([p][+-]?{digit_seq})?{float_suffix}?))
dec_int_const          {non_zero_digit}{digit}*
hex_int_const          {hex_prefix}{hex_digit}+
oct_int_const          [0]oct_digit*
bin_int_const          [0][bB][0-1]+
float_const            {dec_float_const}|{hex_float_const}
int_const              (({dec_int_const}|{hex_int_const}|{oct_int_const}){int_suffix}?)|{bin_int_const}
const_var              ({float_const}|{int_const})
unary_bin_op           ("+"|"-")
unary_op               ("~"|"!")
binary_op              ("*"|"/"|"%"|"&"|"|"|"^"|">>"|"<<"|"||"|"&&"|"~"|"!"|">"|"<"|"<="|">="|"!="|"==")
assign_op              ("*"|"/"|"%"|"&"|"|"|"^"|">>"|"<<"|"||"|"&&"|"+"|"-")?"="
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

"{"             return yy::parser::make_LANGLE (loc);
"}"             return yy::parser::make_RANGLE (loc);
"["             return yy::parser::make_LSQUARE (loc);  
"]"             return yy::parser::make_RSQUARE (loc);  
"("             return yy::parser::make_LPAREN (loc);
")"             return yy::parser::make_RPAREN (loc);
":"             return yy::parser::make_COLON (loc);
";"             return yy::parser::make_SEMICOLON (loc);
","             return yy::parser::make_COMMA (loc);
"fn"            return yy::parser::make_KW_FN (loc);
{id}            return yy::parser::make_IDENTIFIER (yytext, loc);
{const_var}     return yy::parser::make_CONST_VAR (yytext, loc);
{assign_op}     return yy::parser::make_ASSIGN_OP (yytext, loc);
{unary_bin_op}  return yy::parser::make_UNARY_BINARY_OP (yytext, loc);
{unary_op}      return yy::parser::make_UNARY_OP (yytext, loc);
{binary_op}     return yy::parser::make_BINARY_OP (yytext, loc);
.               {
                    throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
}
<<EOF>>      return yy::parser::make_END (loc);
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