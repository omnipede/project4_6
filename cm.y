/* 
 * 	CM Yacc/Bison specification file.
 */

%{
#define YYPARSER

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode*
static char* savedName;
static int savedLineno;
static int savedNum;
static TreeNode* savedTree;

int yyerror(char*);
static int yylex(void);

%}

%token ELSE IF INT RETURN VOID WHILE
%token PLUS MINUS TIMES OVER 
%token LT LE GT GE EQ NE ASSIGN
%token SEMI COMMA LPAREN RPAREN LSQUARE RSQUARE LCURLY RCURLY
%token ID NUM
%token ERROR_IN_COMMENT
%token ERROR
/*
%left PLUS MINUS
%left TIMES OVER
*/
%nonassoc RPAREN
%nonassoc ELSE

%start program

%%

program: declaration_list
			{ savedTree = $1; }
	   ;

declaration_list: declaration_list declaration 
					{
					  YYSTYPE t = $1;
					  if (t){
						for(; t->sibling != NULL; t = t->sibling)
							;
						t->sibling = $2;
						$$ = $1;
					  }
					  else
						  $$ = $2;
				  	}
				| declaration 
					{ $$ = $1;}
				;

declaration: var_declaration 
				{ $$ = $1;}
		   | fun_declaration
			    { $$ = $1;}		
		   ;

id: ID 
	{ 
		savedName = copyString(tokenString); 
		savedLineno = lineno;

		$$ = newExpNode(IdK);
		$$->attr.name = savedName;
		$$->lineno = savedLineno;
	} 
  ;
num: NUM 
	{ 
		savedNum = atoi(tokenString); 
	} 
   ;

var_declaration: type_specifier id SEMI 
				{
					$$ = newDeclNode(VarK);
					$$->attr.name = $2->attr.name;
					$$->lineno = $2->lineno;

					$$->child[0] = $1;
				}		
			   | type_specifier id LSQUARE num RSQUARE SEMI
			    {
					$$ = newDeclNode(VarK);
					$$->attr.name = $2->attr.name;
					$$->lineno = $2->lineno;
					$1->type = Array;
					$1->len = savedNum;

					$$->child[0] = $1;
				}
			   ;

type_specifier: INT 
				{ 
					$$ = newTypeNode(IntK); 
					$$->type = Integer;
					$$->len = 0;

					$$->lineno = lineno;
				}
			  | VOID
			  	{ 
					$$ = newTypeNode(VoidK);
					$$->type = Void;
					$$->len = 0;

					$$->lineno = lineno;
				}
			  ;

fun_declaration: type_specifier id LPAREN params RPAREN compound_stmt
				{
					$$ = newDeclNode(FunK);
					$$->attr.name = $2->attr.name;
					$$->lineno = $2->lineno;
					$$->child[0] = $1;
					$$->child[1] = $4;
					$$->child[2] = $6;
				}
			   ;

params: param_list
		{ $$ = $1; }
      | VOID
	  	{ 
			//$$ = newDeclNode(ParamK);
			$$ = NULL;
		}
	  ;

param_list: param_list COMMA param
			{
				YYSTYPE t = $1;
				if (t) {
					for (; t->sibling; t = t->sibling)
						;
					t->sibling = $3;
					$$ = $1;
				}
				else
					$$ = $3;
			}
          | param
		  	{ $$ = $1; }
		  ;

param: type_specifier id
		{
			$$ = newDeclNode(ParamK);
			$$->attr.name = $2->attr.name;
			$$->lineno = $2->lineno;
			$$->child[0] = $1;
		}
     | type_specifier id LSQUARE RSQUARE
		{
			$$ = newDeclNode(ParamK);
			$$->attr.name = $2->attr.name;
			$$->lineno = $2->lineno;
			$$->child[0] = $1;

			$1->type = Array;
			$1->len = 0;
		}
	 ;

compound_stmt: LCURLY local_declarations statement_list RCURLY
				{
					$$ = newStmtNode(CompoundK);
					$$->child[0] = $2;
					$$->child[1] = $3;
				}
			 ;

local_declarations: local_declarations var_declaration
					{
						YYSTYPE t = $1;
						if (t) {
							for (; t->sibling; t = t->sibling)
								;
							t->sibling = $2;
							$$ = $1;
						}
						else
							$$ = $2;
					}
				  | { $$ = NULL; } 
				  ;

statement_list: statement_list statement
				{ 
					YYSTYPE t = $1;
					if (t) {
						for (; t->sibling; t = t->sibling)
							;
						t->sibling = $2;
						$$ = $1;
					}
					else
						$$ = $2;
				}
			  | { $$ = NULL; }
			  ;

statement: expression_stmt
			{ $$ = $1; }
		 | compound_stmt
		 	{ $$ = $1; }
		 | selection_stmt
		 	{ $$ = $1; }
		 | iteration_stmt
		 	{ $$ = $1; }
		 | return_stmt
		 	{ $$ = $1; }
		 ;

expression_stmt: expression SEMI
				{ $$ = $1; }
			   | SEMI
			   	{ $$ = NULL; }
			   ;

selection_stmt: IF LPAREN expression RPAREN statement
				{ 
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
				}
			  | IF LPAREN expression RPAREN statement ELSE statement
			  	{
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = $7;
				}
			  ;

iteration_stmt: WHILE LPAREN expression RPAREN statement
				{
					$$ = newStmtNode(WhileK);
					$$->child[0] = $3;
					$$->child[1] = $5;
				}
			  ;

return_stmt: RETURN SEMI 
			{
				$$ = newStmtNode(ReturnK);
				$$->lineno = lineno;
			}
		   | RETURN expression SEMI
		   	{
				$$ = newStmtNode(ReturnK);
				$$->child[0] = $2;
				$$->lineno = lineno;
			}
		   ;

expression: var ASSIGN expression 
			{
				$$ = newExpNode(OpK);
				$$->attr.op = ASSIGN;
				$$->lineno = lineno;
				$$->child[0] = $1;
				$$->child[1] = $3;
			}
		  | simple_expression
		  	{ $$ = $1; }
		  ;

var: id
	{ 
		$$ = $1;
	}
   | id LSQUARE expression RSQUARE
    {
		$$ = $1;
		$$->child[0] = $3;
	}
   ;

simple_expression: additive_expression relop additive_expression
					{
						$$ = $2;
						$$->child[0] = $1;
						$$->child[1] = $3;
					}
				 | additive_expression
				 	{
						$$ = $1;
					}
				 ;

relop: LE { $$ = newExpNode(OpK); $$->attr.op = LE ; $$->lineno = lineno; } 
	 | LT { $$ = newExpNode(OpK); $$->attr.op = LT ; $$->lineno = lineno; }
	 | GT { $$ = newExpNode(OpK); $$->attr.op = GT ; $$->lineno = lineno; }
	 | GE { $$ = newExpNode(OpK); $$->attr.op = GE ; $$->lineno = lineno; } 
	 | EQ { $$ = newExpNode(OpK); $$->attr.op = EQ ; $$->lineno = lineno; }
	 | NE { $$ = newExpNode(OpK); $$->attr.op = NE ; $$->lineno = lineno; }
	 ;

additive_expression: additive_expression addop term 
					{
						$$ = $2;
						$$->child[0] = $1;
						$$->child[1] = $3;
					}
				   | term
				    { $$ = $1; }
				   ;

addop: PLUS  { $$ = newExpNode(OpK); $$->attr.op = PLUS; } 
	 | MINUS { $$ = newExpNode(OpK); $$->attr.op = MINUS; }
	 ;

term: term mulop factor 
	 {
		 $$ = $2;
		 $$->child[0] = $1;
		 $$->child[1] = $3;
	 }
	| factor
	 { $$ = $1;}
	;

mulop: TIMES { $$ = newExpNode(OpK); $$->attr.op = TIMES; } 
	 | OVER  { $$ = newExpNode(OpK); $$->attr.op = OVER; } 
	 ;

factor: LPAREN expression RPAREN
		{ $$ = $2;}
	  | var
	  	{ $$ = $1;}
	  | call
	  	{ $$ = $1;}
	  | NUM
	  	{ $$ = newExpNode(ConstK); 
		  $$->attr.val = atoi(tokenString); 
		  $$->type = Integer;
		}
	  ;

call: id LPAREN args RPAREN
	 {
		 $$ = newExpNode(CallK);
		 $$->attr.name = $1->attr.name;
		 $$->lineno = $1->lineno;
		 $$->child[0] = $3;
	 }
	;

args: arg_list 
	  { $$ = $1; }
	| { $$ = NULL; }
	;

arg_list: arg_list COMMA expression 
		 {
			 YYSTYPE t = $1;
			 if (t) {
				 for(; t->sibling; t = t->sibling)
					 ;
				 t->sibling = $3;
				 $$ = $1;
			 }
			 else
				 $$ = $3;
		 }
		| expression
		 { $$ = $1; }
		;

%%

int yyerror(char* msg) {

	fprintf(listing, "Syntax error at line %d: %s\n", lineno, msg);
	fprintf(listing, "Current token: ");
	printToken(yychar, tokenString);
	Error = TRUE;
	return 0;
}

static int yylex(void) {

	TokenType t = getToken();
	return t;
}

TreeNode* parse(void) {

	yyparse();
	return savedTree;
}
