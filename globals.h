#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#ifndef YYPARSER
#include "cm.tab.h"
#define ENDFILE 0
#endif

#ifndef FALSE
#define FALSE 0
#endif 

#ifndef TRUE
#define TRUE 1
#endif
#define MAXRESERVED 8

typedef int TokenType;

extern FILE* source;
extern FILE* listing;
extern FILE* code;

extern int lineno;

/* Syntax tree for parsing. */
typedef enum {StmtK, ExpK, DeclK, TypeK} NodeKind;
typedef enum {IfK, WhileK, ReturnK, CompoundK} StmtKind;
typedef enum {OpK, ConstK, IdK, CallK} ExpKind;
typedef enum {Void, Integer, Array} ExpType;
typedef enum {VarK, FunK, ParamK} DeclKind;
typedef enum {VoidK, IntK} TypeKind;

#define MAXCHILDREN 4

typedef struct treeNode {
	struct treeNode* child[MAXCHILDREN];
	struct treeNode* sibling;
	int lineno;
	NodeKind nodekind;
	union { 
		StmtKind stmt; 
		ExpKind exp; 
		DeclKind decl;
		TypeKind type;
	}kind;

	union { 
		TokenType op;
		int val;
		char* name; 
	} attr;
	ExpType type;
	int len;
} TreeNode;

extern int EchoSource;
extern int TraceScan;
extern int TraceParse;
extern int TraceAnalyze;
extern int TraceCode;
extern int Error;

#endif
