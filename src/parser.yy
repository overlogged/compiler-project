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

# define YYLLOC_DEFAULT(Cur, Rhs, N)                      \
do                                                        \
  if (N)                                                  \
    {                                                     \
      (Cur).begin   = YYRHSLOC(Rhs, 1).begin;             \
      (Cur).end = YYRHSLOC(Rhs, N).end;                   \
    }                                                     \
  else                                                    \
    {                                                     \
      (Cur)   = YYRHSLOC(Rhs, 0);                         \
    }                                                     \
while (0)

}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  KW_FN   "fn"
  KW_RETURN "return"
  KW_FOR "for"
  KW_IN  "in"
  KW_WHILE "while"
  KW_LOOP "loop"
  KW_IF   "if"
  KW_ELSE  "else"
  KW_VAR   "var"
  KW_VAL  "val"
  KW_TYPE "type"  
  KW_NEW  "new"
  KW_DELETE "delete"
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
  UNION_OP      "|"
  EQUAL "="
  QUESTION_MARK "?"
  POINTER_OP "*"
;

%token <std::string> OCT_INT_CONST
%token <std::string> BIN_INT_CONST
%token <std::string> HEX_INT_CONST
%token <std::string> DEC_INT_CONST
%token <std::string> DEC_FLOAT_CONST
%token <std::string> HEX_FLOAT_CONST
%token <std::string> CHAR_CONST
%token <std::string> STRING_CONST
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
constant: DEC_INT_CONST {
            $$.is_const = true;
            $$.ori = $1;
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            type_name.val = number_type($1, 10, $$.val);
            $$.type->type_val = type_name;
            $$.loc = @$;
        }
        | BIN_INT_CONST {
            $$.is_const = true;
            $$.ori = $1; 
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            type_name.val = bin_type($1);
            $$.type->type_val = type_name;
            $$.val = bin_to_value($1);
            $$.loc = @$;
        }
        | OCT_INT_CONST {
            $$.is_const = true;
            $$.ori = $1;
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            auto value = $1.substr(1);
            type_name.val = number_type(value, 8, $$.val);
            $$.type->type_val = type_name;
            $$.loc = @$;
        }
        | HEX_INT_CONST {
            $$.is_const = true;
            $$.ori = $1;
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            auto value = $1.substr(2);
            type_name.val = number_type(value, 16, $$.val);
            $$.type->type_val = type_name;
            $$.loc = @$;
        }
        | DEC_FLOAT_CONST { 
            $$.is_const = true;
            $$.ori = $1;
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            type_name.val = float_type($1, 10, $$.val);
            $$.type->type_val = type_name;
            $$.loc = @$;
        }
        | HEX_FLOAT_CONST {
            $$.is_const = true;
            $$.ori = $1;
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            auto value = $1.substr(2);
            type_name.val = float_type(value, 16, $$.val);
            $$.type->type_val = type_name;
            $$.loc = @$;
        }
        | CHAR_CONST { 
            $$.is_const = true;
            $$.ori = $1; 
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            type_name.val = "char";
            $$.type->type_val = type_name;
            $$.val = to_char($1);
            $$.loc = @$;
        }
        | STRING_CONST { 
            $$.is_const = true;
            $$.ori = trim($1, '\"');
            $$.type = std::make_shared<node_type>();
            $$.type->is_pointer = false;
            node_identifier type_name;
            type_name.val = "char&";
            $$.type->type_val = type_name;
            $$.val = trim($1, '\"');
            $$.loc = @$;
        };

%type <node_identifier> identifier;
identifier: IDENTIFIER { 
            $$.val = $1;
            $$.loc = @$;
    };

%type <node_module> module;
module: %empty {
            $$.loc = @$;
        }
      | module block { 
            $1.blocks.push_back($2);
            $$ = std::move($1); 
            $$.loc = @$;
        };

%type <node_block> block;
block: 
    functionBlock 
    {
        $$.val = $1;
        $$.loc = @$;
    }|
    global_var_def_block
    {
        $$.val = $1;
        $$.loc = @$;
    }|
    global_type_def_block
    {
        $$.val = $1;
        $$.loc = @$;
    } ;

%type <node_type_def_statement> type_def_statement;
type_def_statement:
    KW_TYPE identifier EQUAL type
    {
        $$.type_name = $2.val;
        $$.type =*$4;
        $$.loc = @$;
    };

%type <node_global_type_def_block> global_type_def_block;
global_type_def_block:
    global_type_def_block type_def_statement SEMICOLON
    {
        $1.arr.push_back($2);
        $$ = std::move($1);
        $$.loc = @$;
    }|
    type_def_statement SEMICOLON
    {
        $$.arr.push_back($1);
        $$.loc = @$;
    };

%type <node_global_var_def_block> global_var_def_block;
global_var_def_block:
    global_var_def_block var_def_statement SEMICOLON
    {
        $1.arr.push_back($2);
        $$ = std::move($1);
        $$.loc = @$;
    }|
    var_def_statement SEMICOLON
    {
        $$.arr.push_back($1);
        $$.loc = @$;
    };

%type <node_fun_param> fun_param;
fun_param:
    LPAREN RPAREN{
        $$.empty_flag = true;
        $$.loc = @$;
    } | 
    product_type{
        $$.empty_flag = false;
        $$.params = $1;
        $$.loc = @$;
    }

%type <node_function_block> functionBlock;
functionBlock: KW_FN identifier fun_param type LANGLE statement_list RANGLE 
{
    $$ = node_function_block { 
        .loc = @$,
        .fun_name = $2,
        .params = $3,
        .ret_type = *$4.get(),
        .statement_list = std::move($6),
        .no_ret =false
    };
}| 
KW_FN identifier fun_param LANGLE statement_list RANGLE 
{
    $$ = node_function_block { 
        .loc = @$,
        .fun_name = $2,
        .params = $3,
        .ret_type = node_type {
            .loc = @$,
            .is_pointer = false,
            .type_val = node_identifier {
                .loc = @$,
                .val = "auto"
            }
        },
        .statement_list = std::move($5),
        .no_ret = true
    };   
};

%type <node_primary_expr> primaryExpr;
primaryExpr: 
    identifier { 
        $$.val = $1; 
        $$.loc = @$;
    } |
    constant { 
        $$.val = $1; 
        $$.loc = @$;
    } |
    LPAREN expression RPAREN { 
        $$.val = $2;
        $$.loc = @$;
    };

%type <std::shared_ptr<node_post_expr>> postfixExpr;
postfixExpr:
    primaryExpr {
        auto data = std::make_shared<node_post_expr>();
        data->expr = $1;
        $$ = data;
        $$->loc = @$;
    } |
    postfixExpr LPAREN expression_list RPAREN {
        auto data = std::make_shared<node_post_expr>();
        data->expr = node_post_call_expr {
            .loc = @$,
            .callable = $1,
            .exp_list = std::move($3)
        };
        $$ = data;
        $$->loc = @$;
    } |
    postfixExpr DOT identifier {
        auto data = std::make_shared<node_post_expr>();
        data->expr = node_post_dot_expr {
            .loc = @$,
            .obj = $1,
            .attr = $3
        };
        $$ = data; 
        $$->loc = @$;
    }|
    postfixExpr DOT identifier QUESTION_MARK{
        auto data = std::make_shared<node_post_expr>();
        data->expr = node_post_check_expr {
            .loc = @$,
            .check_lable = $3,
            .check_exp = $1
        };
        $$ = data; 
        $$->loc = @$;
    };
%type <node_unary_expr> unaryExpr;
unaryExpr: 
    postfixExpr {
        $$.post_expr = $1;
        $$.loc = @$;
    } |
    UNARY_OP unaryExpr {
        $$ = std::move($2);
        $$.ops.push_back($1);
        $$.loc = @$;
    } |
    UNARY_BINARY_OP unaryExpr {
        $$ = std::move($2);
        $$.ops.push_back($1);
        $$.loc = @$;
    }|
    POINTER_OP unaryExpr {
        $$ = std::move($2);
        $$.ops.push_back("*");
        $$.loc = @$;
    };

%type <node_binary_expr> binaryExpr;
binaryExpr:
    unaryExpr {
        $$.vars.push_back($1);
        $$.loc = @$;
    } |
    binaryExpr BINARY_OP unaryExpr {
        $$ = std::move($1);
        $$.vars.push_back($3);
        $$.ops.push_back($2);
        $$.loc = @$;
    } |
    binaryExpr UNARY_BINARY_OP unaryExpr {
        $$ = std::move($1);
        $$.vars.push_back($3);
        $$.ops.push_back($2);
        $$.loc = @$;
    } |
    binaryExpr POINTER_OP unaryExpr{
        $$ = std::move($1);
        $$.vars.push_back($3);
        $$.ops.push_back("*");
        $$.loc = @$;
    } |
    binaryExpr UNION_OP unaryExpr{
        $$ = std::move($1);
        $$.vars.push_back($3);
        $$.ops.push_back("|");
        $$.loc = @$;
    };
%type<node_construct_expr> construct_expr;
construct_expr:
    LANGLE construct_element RANGLE
    {
        $$ = $2;
        $$.loc = @$;
    }|
    LANGLE no_lable_construct_element RANGLE
    {
        $$ = $2;
        $$.loc = @$;
    }
%type<node_construct_expr> no_lable_construct_element;
no_lable_construct_element:
    no_lable_construct_element COLON expression
    {
        $1.lable.push_back("_"+std::to_string($$.lable.size()));
        $1.init_val.push_back($3);
        $$ =$1;
        $$.loc = @$;
    }|
    expression
    {
        $$.lable.push_back("_"+std::to_string($$.lable.size()));
        $$.init_val.push_back($1);
        $$.loc = @$;
    }

%type<node_construct_expr> construct_element;
construct_element:
    construct_element COLON identifier EQUAL expression
    {
        $1.lable.push_back($3.val);
        $1.init_val.push_back($5);
        $$ = $1;
        $$.loc = @$;
    }|
    identifier EQUAL expression
    {
        $$.lable.push_back($1.val);
        $$.init_val.push_back($3);
        $$.loc = @$;
    }

%type <std::shared_ptr<node_expression>> expression;
expression:
    binaryExpr {
        auto data = std::make_shared<node_expression>();
        data->expr = $1;
        $$ = data;
        $$->loc = @$;
    } |
    unaryExpr ASSIGN_OP expression {
        auto data = std::make_shared<node_expression>();
        data->expr = node_assign_expr {
            .loc = @$,
            .lval = $1,
            .op = $2,
            .rval = $3
        };
        $$ = data;
        $$->loc = @$;
    }|
    unaryExpr EQUAL expression {
        auto data = std::make_shared<node_expression>();
        data->expr = node_assign_expr {
            .loc = @$,
            .lval = $1,
            .op = "=",
            .rval = $3
        };
        $$ = data;
        $$->loc = @$;
    }|
    construct_expr{
        auto data = std::make_shared<node_expression>();
        data->expr = $1;
        $$ = data;
        $$->loc = @$;
    }|
    new_expression{
        auto data = std::make_shared<node_expression>();
        data->expr = $1;
        $$ = data;
        $$->loc = @$;
    }

%type <node_new_expr> new_expression;
new_expression:
    KW_NEW type
    {
        $$.new_type = *$2;
        $$.loc = @$;
    }

%type <node_expression_list> expression_list;
expression_list: 
    %empty {
        $$.loc = @$;
    } |
    expression {
        $$.arr.push_back($1);
        $$.loc = @$;
    } |
    expression_list COMMA expression {
        $$ = std::move($1);
        $$.arr.push_back($3);
        $$.loc = @$;
    };

%type <node_statement> statement;
statement: 
    expression SEMICOLON {
        $$.statement = *$1;
        $$.loc = @$;
    }|
    return_statement SEMICOLON{
        $$.statement = $1;
        $$.loc = @$;
    }|
    for_statement{
        $$.statement = $1;
        $$.loc = @$;
    }|
    while_statement{
        $$.statement = $1;
        $$.loc = @$;
    }|
    if_statement{
        $$.statement = $1;
        $$.loc = @$;
    }|
    var_def_statement SEMICOLON{
        $$.statement = $1;
        $$.loc = @$;
    }|
    delete_statement SEMICOLON{
        $$.statement = $1;
        $$.loc = @$;
    }

%type <node_delete_statement> delete_statement;
delete_statement:
    KW_DELETE expression
    {
        $$.delete_expr = *$2;
        $$.loc = @$;
    }
%type <node_var_def_statement> var_def_statement;
var_def_statement:
    KW_VAR var_list COLON type EQUAL expression
    {
        $$.is_immutable = false;
        $$.var_list = $2;
        $$.var_type = *$4;
        $$.initial_exp = *$6;
        $$.loc = @$;
    }|
    KW_VAR var_list EQUAL expression
    {
        $$.is_immutable = false;
        $$.initial_exp = *$4;
        $$.var_type = node_type_auto;
        $$.var_list = $2;
        $$.loc = @$;
    }|
    KW_VAL var_list EQUAL expression
    {
        $$.is_immutable = true;
        $$.initial_exp = *$4;
        $$.var_type = node_type_auto;
        $$.var_list = $2;
        $$.loc = @$;
    }|
    KW_VAL var_list COLON type EQUAL expression
    {
        $$.is_immutable = true;
        $$.var_list = $2;
        $$.var_type = *$4;
        $$.initial_exp = *$6;
        $$.loc = @$;
    };
%type <std::vector<node_identifier>> var_list;
var_list:
    identifier
    {
        $$ = std::vector<node_identifier>();
        $$.push_back($1);
    }|
    var_list COMMA identifier
    {
        $1.push_back($3);
        $$ = std::move($1);
    } 

%type <node_return_statement> return_statement;
return_statement:
    KW_RETURN expression{
        $$.expr = *$2;
        $$.loc = @$;
    };
%type <node_for_statement> for_statement;
for_statement:
    KW_FOR identifier KW_IN expression LANGLE statement_list RANGLE
    {
        $$.id = $2;
        $$.for_range = *$4;
        $$.for_statement = std::move($6);
        $$.loc = @$;
    };
%type <node_while_statement> while_statement;
while_statement:
    KW_WHILE expression LANGLE statement_list RANGLE
    {
        $$.while_condition = *$2;
        $$.loop_statement = std::move($4);
        $$.loc = @$;
    }|
    KW_LOOP LANGLE statement_list RANGLE
    {
        $$.loop_statement = std::move($3);
        $$.loc = @$;
    };
%type <node_if_statement> if_statement;
if_statement:
    KW_IF expression LANGLE statement_list RANGLE else_if_statement KW_ELSE LANGLE statement_list RANGLE
    {
        $$.if_condition = *$2;
        $$.if_statement = std::move($4);
        $$.else_statement = std::move($9);
        $$.else_if_statement = $6;
        $$.loc = @$;
    };
%type <node_else_if_statement> else_if_statement;
else_if_statement:
    else_if_statement KW_ELSE KW_IF expression LANGLE statement_list RANGLE
    {
        $1.else_if_condition.push_back(*$4);
        $1.else_if_statement.emplace_back(std::move($6));
        $$ = std::move($1);
        $$.loc = @$;
    }|
    %empty
    {
        $$.loc = @$;
    }
%type <std::vector<node_statement>> statement_list;
statement_list:
    %empty {} |
    statement_list statement { 
        $$ = std::move($1);
        $$.push_back($2);
    };

%type <node_product_type> product_type;
product_type:
    LPAREN product_type_tuple RPAREN{
        $$ = $2;
        $$.loc = @$;
    };
%type <node_product_type> product_type_tuple;
product_type_tuple:
    product_type_tuple COMMA identifier COLON type{
        $1.lables.push_back($3.val);
        $1.element.push_back($5);
        $$ = std::move($1);
        $$.loc = @$;
    }|
    identifier COLON type{
        $$.lables.push_back($1.val);
        $$.element.push_back($3);
        $$.loc = @$;
    }|
    product_type_tuple COMMA type{
        $1.lables.push_back("_"+std::to_string($1.lables.size()));
        $1.element.push_back($3);
        $$ = std::move($1);
        $$.loc = @$;
    }|
    type{
        $$.lables.push_back("_0");
        $$.element.push_back($1);
        $$.loc = @$;
    };    

%type <node_sum_type> sum_type;
sum_type:
    LPAREN sum_type_tuple UNION_OP identifier COLON type RPAREN
    {
        $$ = std::move($2);
        $$.lables.push_back($4.val);
        $$.element.push_back($6);
        $$.loc = @$;
    }|
    LPAREN sum_type_tuple UNION_OP type RPAREN
    {
        $$ = std::move($2);
        $$.lables.push_back("_"+std::to_string($$.lables.size()));
        $$.element.push_back($4);
        $$.loc = @$;
    }

%type <node_sum_type> sum_type_tuple;
sum_type_tuple:
    sum_type_tuple UNION_OP identifier COLON type{
        $1.lables.push_back($3.val);
        $1.element.push_back($5);
        $$ = std::move($1);
        $$.loc = @$;
    }|
    identifier COLON type{
        $$.lables.push_back($1.val);
        $$.element.push_back($3);
        $$.loc = @$;
    }|
    sum_type_tuple UNION_OP type{
        $1.lables.push_back("_"+std::to_string($$.lables.size()));
        $1.element.push_back($3);
        $$ = std::move($1);
        $$.loc = @$;
    }|
    type{
        $$.lables.push_back("_"+std::to_string($$.lables.size()));
        $$.element.push_back($1);
        $$.loc = @$;
    };

%type <std::shared_ptr<node_type>> type;
type:
    identifier
    {
        auto data = std::make_shared<node_type>();
        data->is_pointer = false;
        data->type_val = $1;
        $$ = data;
        $$->loc = @$;
    }|
    identifier POINTER_OP
    {
        auto data = std::make_shared<node_type>();
        data->is_pointer = true;
        data->type_val = $1;
        $$ = data;
        $$->loc = @$;
    }|
    sum_type
    {
        auto data = std::make_shared<node_type>();
        data->is_pointer = false;
        data->type_val = $1;
        $$ = data;
        $$->loc = @$;
    }|
    sum_type POINTER_OP
    {
        auto data = std::make_shared<node_type>();
        data->is_pointer = true;
        data->type_val = $1;
        $$ = data;
        $$->loc = @$;
    }|
    product_type
    {
        auto data = std::make_shared<node_type>();
        data->is_pointer = false;
        data->type_val = $1;
        $$ = data;
        $$->loc = @$;
    }|
    product_type POINTER_OP
    {
        auto data = std::make_shared<node_type>();
        data->is_pointer = true;
        data->type_val = $1;
        $$ = data;
        $$->loc = @$;
    };


%%

void yy::parser::error (const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
