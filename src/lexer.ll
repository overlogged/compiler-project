%option c++
%option yyclass="parser"
%option batch

%{
#include "parser.tab.hpp"
#include <string>
%}

digit [0-9]
addop [+-]
mulop [*/]
expop \^
number {digit}+

%%

{blank}+   loc.step ();
\n+        loc.lines (yyleng); loc.step ();

"-"        return yy::parser::make_MINUS  (loc);
"+"        return yy::parser::make_PLUS   (loc);
"*"        return yy::parser::make_STAR   (loc);
"/"        return yy::parser::make_SLASH  (loc);
"("        return yy::parser::make_LPAREN (loc);
")"        return yy::parser::make_RPAREN (loc);
":="       return yy::parser::make_ASSIGN (loc);

{int}      return make_NUMBER (yytext, loc);
{id}       return yy::parser::make_IDENTIFIER (yytext, loc);
.          {
             throw yy::parser::syntax_error
               (loc, "invalid character: " + std::string(yytext));
            }
            
<<EOF>>    return yy::parser::make_END (loc);

%%