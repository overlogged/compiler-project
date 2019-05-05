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
functionBlock: KW_FN IDENTIFIER LPAREN parameters RPAREN type LANGLE exp_list RANGLE 
{
    $$ = node_function_block { 
        .fun_name = $2,
        .params = $4.params,
        .ret_type = $6,
        .exp_list = $8
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
%type <std::shared_ptr<node_exp_list>> exp_list;
exp_list: 
    %empty { $$ = std::make_shared<node_exp_list>();}
    | exp_list exp1 SEMICOLON {
        $$ = $1;
        $1->push_back($2);
    };
%type <std::shared_ptr<node_exp1>> exp1;
exp1:
    exp1 ASSIGN_OP exp2 {
        $$ = std::make_shared<node_exp1>();
        *$$.get() = std::make_shared<node_exp1alt>();
        (*std::get_if<std::shared_ptr<node_exp1alt>>($$.get()))->op = $2;
        (*std::get_if<std::shared_ptr<node_exp1alt>>($$.get()))->left_val = $1;
        (*std::get_if<std::shared_ptr<node_exp1alt>>($$.get()))->right_val = $3;
    }
    | exp2{
        $$ = std::make_shared<node_exp1>();
        *$$.get() = $1;
    }
%type <std::shared_ptr<node_exp2>> exp2;
exp2:
    /*exp2"?"exp2":"exp3{
        $$.val = new(node_exp2alt);
        (*std::get_if<node_exp2alt*>(&($$.val)))->val0 = $1;
        (*std::get_if<node_exp2alt*>(&($$.val)))->val1 = $3;
        (*std::get_if<node_exp2alt*>(&($$.val)))->val2 = $5;
    }
    | */exp3{
        $$ = std::make_shared<node_exp2>();
        *$$.get() = $1;
    }
%type <std::shared_ptr<node_exp3>> exp3;
exp3:
    exp3 BINARY_OP exp4{
        $$ = $1;
        (*$$.get()).ops->push_back($2); 
        (*$$.get()).vars->push_back(*$3.get()); 
    }
    |exp4 {
        $$ = std::make_shared<node_exp3>();
        (*$$.get()).ops = std::make_shared<vec_str>();
        (*$$.get()).vars = std::make_shared<std::vector<node_exp4>>();
        (*$$.get()).vars->push_back(*$1.get());
    }
%type <std::shared_ptr<node_exp4>> exp4;
exp4:
    IDENTIFIER{
        $$ = std::make_shared<node_exp4>();
        *$$.get() = $1;
    }
    |FUNCTION_CALL{
        $$ = std::make_shared<node_exp4>();
        *$$.get() = $1;
    }
    |CONST_VAR{
        $$ = std::make_shared<node_exp4>();
        *$$.get() = $1;
    }
    |LPAREN exp1 RPAREN{
        $$ = std::make_shared<node_exp4>();
        *$$.get() = $2;
    }
%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
