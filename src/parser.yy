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
  DOT     "."
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
module: %empty { }
      | module block { $1.blocks.push_back($2); $$ = std::move($1); };

%type <node_block> block;
block: functionBlock { $$ = $1;}

%type <node_function_block> functionBlock;
functionBlock: KW_FN identifier LPAREN parameters RPAREN type LANGLE statement_list RANGLE 
{
    $$ = node_function_block { 
        .fun_name = $2,
        .params = $4.params,
        .ret_type = $6,
        .statement_list = std::move(*$8)
    };
}

%type <node_parameters> parameters;
parameters: 
    %empty { $$.params = std::vector<node_var_name_type>();}
    | varNameType { 
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

%type <node_primary_expr> primaryExpr;
primaryExpr: 
    identifier { $$ = $1; } |
    constant { $$ = $1; } |
    LPAREN expression RPAREN { $$ = $2;};

%type <std::shared_ptr<node_post_expr>> postfixExpr;
postfixExpr:
    primaryExpr {
        auto data = std::make_shared<node_post_expr>();
        data->expr = $1;
        $$ = data;
    } |
    postfixExpr LPAREN arguments RPAREN {
        auto data = std::make_shared<node_post_expr>();
        data->expr = node_post_call_expr {
            .callable = $1,
            .arguments = std::move($3)
        };
        $$ = data;
    } |
    postfixExpr DOT identifier {
        auto data = std::make_shared<node_post_expr>();
        data->expr = node_post_dot_expr {
            .obj = $1,
            .attr = $3
        };
        $$ = data; 
    };

%type <node_unary_expr> unaryExpr;
unaryExpr: 
    postfixExpr {
        $$.post_expr = $1;
    } |
    UNARY_OP unaryExpr {
        $$ = std::move($2);
        $$.ops.push_back($1);
    } |
    UNARY_BINARY_OP unaryExpr {
        $$ = std::move($2);
        $$.ops.push_back($1);
    };

%type <node_binary_expr> binaryExpr;
binaryExpr:
    unaryExpr {
        $$.vars.push_back($1);
    } |
    binaryExpr BINARY_OP unaryExpr {
        $$ = std::move($1);
        $$.vars.push_back($3);
        $$.ops.push_back($2);
    } |
    binaryExpr UNARY_BINARY_OP unaryExpr {
        $$ = std::move($1);
        $$.vars.push_back($3);
        $$.ops.push_back($2);
    };

%type <std::shared_ptr<node_expression>> expression;
expression:
    binaryExpr {
        auto data = std::make_shared<node_expression>();
        data->expr = $1;
        $$ = data;
    } |
    unaryExpr ASSIGN_OP expression {
        auto data = std::make_shared<node_expression>();
        data->expr = node_assign_expr {
            .lval = $1,
            .op = $2,
            .rval = $3
        };
        $$ = data;
    };

%type <node_arguments> arguments;
arguments: 
    %empty {} |
    expression {
        $$.push_back($1);
    } |
    arguments COMMA expression {
        $$ = std::move($1);
        $$.push_back($3);
    };

%type <node_statement> statement;
statement: 
    expression SEMICOLON {
        $$ = $1;
    };

%type <std::shared_ptr<std::vector<node_statement>>> statement_list;
statement_list:
    %empty {$$ = std::make_shared<std::vector<node_statement>>();} |
    statement_list statement { $$ = $1; $$->push_back($2);};
%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
