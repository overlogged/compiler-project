%language "c++"
%require "3.0"
%defines

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include "parse_tree.h"
  class driver;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
#include "driver.h"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  KW_FN   "fn"
  LANGLE  "{"
  RANGLE  "}"
  LSQUARE "["
  RSQUARE "]"
  LPAREN  "("
  RPAREN  ")"
  SEMICOLON ";"
  COLON   ":"
  COMMA   ","
;
%token <std::string> CONST_VAR
%token <std::string> ASSIGN_OP
%token <std::string> BINARY_OP
%token <std::string> FUNCTION_CALL
%token <std::string> IDENTIFIER "identifier"
%%
%start module_first;

module_first: module { drv.parse_tree = $1; }

%type <node_module> module;
module: %empty { $$.blocks = std::vector<node_block>();}
      | module block { $1.blocks.push_back($2); $$ = std::move($1); };

%type <node_block> block;
block: functionBlock { $$ = $1;}

%type <node_function_block> functionBlock;
functionBlock: KW_FN IDENTIFIER LPAREN parameters RPAREN type LANGLE exp1s RANGLE 
{
    $$ = node_function_block { 
        .fun_name = $2,
        .params = $4.params,
        .ret_type = $6,
        .exp1s = &$8
    };
}

%type <node_parameters> parameters;
parameters: 
    %empty { $$.params = std::vector<node_var_name_type>();}
    | varNameType { 
        $$.params = std::vector<node_var_name_type>();
        $$.params.push_back($1);
    }
    | parameters COMMA varNameType {
        $$ = std::move($1);
        $$.params.push_back($3);
    };


%type <node_var_name_type> varNameType;
varNameType: IDENTIFIER COLON type 
{
    $$ = node_var_name_type {
        .name = $1,
        .type = $3
    };
}

%type <std::string> type;
type: IDENTIFIER 
{ 
    // todo
    $$ = $1; 
}
%type <node_exp1s> exp1s;
exp1s: 
    %empty { $$.exp1s = std::vector<node_exp1>();}
    | exp1s exp1 SEMICOLON {
        std::cout<<"1tiao"<<std::endl;
        $$ = std::move($1);
        $$.exp1s.push_back($2);
        drv.test = &($$.exp1s[0]);
        std::cout<<"parse get_if<node_expalt>: "<<std::get_if<node_exp1alt*>(&($$.exp1s[0].val))<<std::endl;
        std::cout<<"parse get_if<node_exp2>: "<<std::get_if<node_exp2*>(&($$.exp1s[0].val))<<std::endl;
        std::cout<<"parse node_address: "<<&($$.exp1s[0])<<std::endl;
    };
%type <node_exp1> exp1;
exp1:
    exp1 ASSIGN_OP exp2 {
        $$.val = new(node_exp1alt);
        (*std::get_if<node_exp1alt*>(&($$.val)))->val0 = $1;
        (*std::get_if<node_exp1alt*>(&($$.val)))->val1 = $3;
    }
    | exp2{
        std::cout<<"1->2"<<std::endl;
        $$.val = &$1;
    }
%type <node_exp2> exp2;
exp2:
    exp2"?"exp2":"exp3{
        $$.val = new(node_exp2alt);
        (*std::get_if<node_exp2alt*>(&($$.val)))->val0 = $1;
        (*std::get_if<node_exp2alt*>(&($$.val)))->val1 = $3;
        (*std::get_if<node_exp2alt*>(&($$.val)))->val2 = $5;
    }
    | exp3{
        std::cout<<"2->3"<<std::endl;
        $$.val = &$1;
        std::cout<<*std::get_if<std::string>(&(((*std::get_if<node_exp3*>(&($$.val)))->vars)[0].val))<<std::endl;
    }
%type <node_exp3> exp3;
exp3:
    exp3 BINARY_OP exp4{
       $1.vars.push_back($3);
       $1.ops.push_back($2);
       $$ = std::move($1);
    }
    |exp4 {
        std::cout<<"3->4"<<std::endl;
        $$.vars = std::vector<node_exp4>();
        $$.vars.push_back($1);
        std::cout<<*std::get_if<std::string>(&(($$.vars)[0].val))<<std::endl;
    }
%type <node_exp4> exp4;
exp4:
    IDENTIFIER{
        $$.val = $1;
    }
    |FUNCTION_CALL{
        $$.val = $1;
    }
    |CONST_VAR{
        std::cout<<"const"<<std::endl;
        $$.val = $1;
        std::cout<<*std::get_if<std::string>(&($$.val))<<std::endl;
    }
    |LPAREN exp1 RPAREN{
        $$.val = &$2;
    }
%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
