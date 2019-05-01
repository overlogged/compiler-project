%require "3.2"
%language "c++"
%define api.value.type variant
%define api.token.constructor
%locations
%define parse.trace
%define parse.assert
%define parse.error verbose

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  ASSIGN  ":="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%type  <int> exp


%{
#include <cmath>
#include <iostream>

int yyparse(void);
int yylex(void);
int yywrap()
{
    return 1;
}
int yyerror(const char *msg);

double results[1000];
int count = 0;

%}

%define api.token.prefix {TOK_}

%token ADDOP MULOP EXPOP NUMBER LBRACE RBRACE

%type <double> final exp term factor atom primary NUMBER
%type <char> ADDOP MULOP EXPOP

%%
final:
    exp {
        results[count++] = $1;
    } |
    final exp {
        results[count++] = $2;
    };

exp:
    exp ADDOP term {
        if($2 == '+'){
            $$ = $1 + $3;
        }else{
            $$ = $1 - $3;
        }
    } |
    term {
        $$ = $1;
    };

atom: 
    NUMBER {
        $$ = $1;
    } |
    LBRACE exp RBRACE {
        $$ = $2;
    };

primary:
    atom {
        $$ = $1;
    } |
    ADDOP primary {
        if($1 == '-') {
            $$ = - $2;
        } else {
            $$ = $2;
        }
    };

factor:
    primary EXPOP factor {
        $$ = pow($1,$3);
    } |
    primary {
        $$ = $1;
    };

term:
    term MULOP factor {
        if($2 == '*'){
            $$ = $1 * $3;
        }else{
            $$ = $1 / $3;
        }
    } |
    factor {
        $$ = $1;
    };

%%

/*

final = exp \n
exp = exp + term | exp - term | term
term = term * factor | term / factor | factor
factor = primary ^ factor | primary
primary =  atom | - primary
atom = NUMBER | ( exp )

*/

int yyerror(const char *msg)
{
    printf("Error encountered: %s \n", msg);
    return 0;
}