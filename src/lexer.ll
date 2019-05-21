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

id                     [a-zA-Z_][_a-zA-Z_0-9]*
blank                  [ \t\r]
digit                  [0-9]
non_zero_digit         [1-9]
hex_digit              [0-9a-fA-F]
hex_quad               {hex_digit}{4}
oct_digit              [0-7]
digit_seq              {digit}+
hex_digit_seq          {hex_digit}+
float_suffix           [fFlL]
int_suffix             ([uU]?([lL]|"ll"|"LL"))|(([lL]|"ll"|"LL")?[uU])
hex_prefix             [0][xX]
dec_float_const        ((({digit_seq}?\.{digit_seq})|({digit_seq}\.)|{digit_seq})(([eE][+-]?{digit_seq})?{float_suffix}?))
hex_float_const        ({hex_prefix}(({hex_digit_seq}?\.{hex_digit_seq})|({hex_digit_seq}\.)|{hex_digit_seq})(([p][+-]?{digit_seq})?{float_suffix}?))
dec_int_const          {non_zero_digit}{digit}*{int_suffix}?
hex_int_const          {hex_prefix}{hex_digit}+{int_suffix}?
oct_int_const          [0]oct_digit+{int_suffix}?
bin_int_const          [0][bB][0-1]+
simple_escape_seq      [\\]['"abfnrtv\\]
octal_escape_seq       [\\]{oct_digit}|[\\]{oct_digit}{2}|[\\]{oct_digit}{3}
hex_escape_seq         [\\]"x"{hex_digit}{2}
universal_char         [\\]"u"{hex_quad}|[\\]"U"{hex_quad}{2}
escape_seq             {simple_escape_seq}|{octal_escape_seq}|{hex_escape_seq}|{universal_char}
c_char_seq             ([^'\\\r\n])|{escape_seq}
char_const             (\'{c_char_seq}\')|("L"\'{c_char_seq}\')|("u"\'{c_char_seq}\')|("U"\'{c_char_seq}\')
string_const           \"{c_char_seq}+\"
unary_bin_op           ("+"|"-"|"*")
unary_op               ("~"|"!")
binary_op              ("*"|"/"|"%"|"^"|">>"|"<<"|"||"|"&&"|"~"|"!"|">"|"<"|"<="|">="|"!="|"==")
assign_op              ("*"|"/"|"%"|"&"|"|"|"^"|">>"|"<<"|"||"|"&&"|"+"|"-")=
equal                  "="
referrence_op          "&"
union_op               "|"
question_mark          "?"
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

"."             return yy::parser::make_DOT (loc);
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
"return"        return yy::parser::make_KW_RETURN (loc);
"for"           return yy::parser::make_KW_FOR (loc);
"in"            return yy::parser::make_KW_IN (loc);
"while"         return yy::parser::make_KW_WHILE (loc);
"loop"          return yy::parser::make_KW_LOOP (loc);
"if"            return yy::parser::make_KW_IF (loc);
"else"          return yy::parser::make_KW_ELSE (loc);
"var"           return yy::parser::make_KW_VAR (loc);
"val"           return yy::parser::make_KW_VAL (loc);
{equal}         return yy::parser::make_EQUAL (loc);
{question_mark} return yy::parser::make_QUESTION_MARK (loc);
{referrence_op} return yy::parser::make_REFERRENCE_OP (loc);
{union_op}      return yy::parser::make_UNION_OP (loc);
{id}            return yy::parser::make_IDENTIFIER (yytext, loc);
{oct_int_const} return yy::parser::make_OCT_INT_CONST(yytext, loc);
{bin_int_const} return yy::parser::make_BIN_INT_CONST(yytext, loc);
{hex_int_const} return yy::parser::make_HEX_INT_CONST(yytext, loc);
{dec_int_const} return yy::parser::make_DEC_INT_CONST(yytext, loc);
{dec_float_const}   return yy::parser::make_DEC_FLOAT_CONST(yytext, loc);
{hex_float_const}   return yy::parser::make_HEX_FLOAT_CONST(yytext, loc);
{char_const}    return yy::parser::make_CHAR_CONST(yytext, loc);
{string_const}  return yy::parser::make_STRING_CONST(yytext, loc);
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