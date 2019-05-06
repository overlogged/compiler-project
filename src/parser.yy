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
%token <std::string> UNARY_BINARY_OP
%token <std::string> UNARY_OP
%token <std::string> BINARY_OP
%token <std::string> ASSIGN_OP
%token <std::string> IDENTIFIER "identifier"
%%
%start module_first;

module_first: module { 
    drv.parse_tree = $1;
};


%type <node_constant> constant;
constant: CONST_VAR { $$.val = $1;};

%type <node_identifier> identifier;
identifier: IDENTIFIER { $$.val = $1;};

%type <node_module> module;
module: %empty { $$.blocks = std::vector<node_block>();}
      | module block { $1.blocks.push_back($2); $$ = std::move($1); };

%type <node_block> block;
block: functionBlock { $$ = $1;}

%type <node_function_block> functionBlock;
functionBlock: KW_FN identifier LPAREN parameters RPAREN type LANGLE RANGLE 
{
    $$ = node_function_block { 
        .fun_name = $2,
        .params = $4.params,
        .ret_type = $6
        // .exp_list = $8
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
varNameType: identifier COLON type 
{
    $$ = node_var_name_type {
        .name = $1,
        .type = $3
    };
}

%type <node_type> type;
type: identifier 
{ 
    // todo
    $$.val = $1.val; 
}

%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
