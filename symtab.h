#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* SIZE is the size of the hash table. */
#define SIZE 211

/* SHIFT is the power of two used as multiplier */
#define SHIFT 4

/* Maximum scope level. */
#define MAX_SCOPE 200

/* the list of line numbers of the source code
 * in which a variable is referenced
 */
typedef struct LineListRec {

	int lineno;
	struct LineListRec* next;
}* LineList;

/* The record in the bucket lists for
 * each variable, including name, assigned memory loc, 
 * and the list of line numbers 
 * in which it appears in the source code. 
 */
typedef struct BucketListRec {

	char* name;
	LineList lines;
	int memloc;

	char VPF;
	int type;
	int len;
	TreeNode* params;
	struct BucketListRec* next;
}* BucketList;

/* Wrapping structure of BucketList. */
typedef struct ScopeListRec {

	/* locations. */
	int varLoc;
	int funcLoc;
	int paramLoc;
	/* hash table */
	BucketList hashTable[SIZE];
	/* scope level */
	int level;
	/* Parent ptr. */
	struct ScopeListRec* parent;
}* ScopeList;

/* List of scopes. */
extern ScopeList scope[];

/* Procedure scope_top returns
 * current scope record. 
 */
struct ScopeListRec* scope_top (void);

/* Procedure scope_push pushes
 * current scope record to stack
 */
void scope_push (struct ScopeListRec* sc);

/* Procedure scope_pop pops
 * crrent scope from scope stack.
 */
void scope_pop(void);

/* Function scope_new creates
 * new scope
 */
struct ScopeListRec* scope_new (void);

/* Procedure xt_insert inserts line numbers and
 * memory locations into the symbol table
 */
void st_insert (char* name, int lineno, int loc, char VPF, int type, int len, TreeNode*);

/* Procedure st_insert_global inserts line number to 
 * the correspoinding scope that stays global.
 */
void st_insert_global (char* name, int lineno);

/* Function st_lookup returns the bucketlist record
 * of a variable or NULL if not found
 */
BucketList st_lookup (char* name);

/* Function st_lookup_local returns the bucketlist record
 * of a variable in local scope, if not found returns NULL
 */
BucketList st_lookup_local (char* name);

/* Procedure printSymTab prints a formatted listing 
 * of the symbol table contents
 * to the listing file
 */
void printSymTab (FILE* listing);

#endif
