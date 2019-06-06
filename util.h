#ifndef _UTIL_H_
#define _UTIL_H_

/* 
 *	Prints a token and its lexeme ot the 
 *	listing file.
 */
void printToken(TokenType, const char*);

/*
 *	Creates a new statement node ofr syntax tree 
 *	construction.
 */
TreeNode* newStmtNode (StmtKind);

/* 
 *	Creates a new expression node
 * 	for syntax tree construction.
 */
TreeNode* newExpNode (ExpKind);

/*
 * Creates a new declaration node
 * for syntax tree construction.
 */
TreeNode* newDeclNode (DeclKind);

/*
 * Creates a new type node
 * for syntax tree construction.
 */
TreeNode* newTypeNode (TypeKind);

/* 
 *	Allocates and makes a new copy of
 *	an existing string.
 */
char* copyString(char*);

/*
 *	Prints a syntax tree to the listing file
 *	using indentation to indicate subtrees. 
 */
void printTree(TreeNode*);
#endif
