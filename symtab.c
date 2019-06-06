#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

/* index of scope list. */
int scope_index = 0;

/* List of scopes. */
ScopeList scope[MAX_SCOPE];

/* Top of stack. */
static int top = 0;
/* Stack of scopes. */
ScopeList scope_stack[MAX_SCOPE];


/* Function scope_top returns
 * current scope record. 
 */
struct ScopeListRec* scope_top (void) {

	if (top > 0)
		return scope_stack[top - 1];
	else
		return NULL;
}

/* Procedure scope_push pushes
 * current scope record to stack
 */
void scope_push (struct ScopeListRec* sc) {
	if (!sc)
		return;
	
	if (top < MAX_SCOPE)
		scope_stack[top++] = sc;	
	return;
}

void scope_pop (void) {

	if (top > 0)
		top -=  1;
	return;
}

struct ScopeListRec* scope_new (void) {

	/* If space allows, */
	if (scope_index < MAX_SCOPE) {
		struct ScopeListRec* t
			= (struct ScopeListRec*) malloc (sizeof(struct ScopeListRec));

		t->level = top;
		t->parent = scope_top();
		int i;
		for (i = 0; i < SIZE; i++)
			t->hashTable[i] = NULL;

		if (t->level == 0) 
			t->varLoc = 0;
		else if (t->level == 1)
			t->varLoc = -4;
		else
			t->varLoc = t->parent->varLoc;
		t->funcLoc = 0;
		t->paramLoc = 4;

		scope[scope_index] = t;
		scope_index += 1;
		return t;
	}
	else
		return NULL;
}

/* the hash function */
static int hash (char* key) {
	if (!key)
		return 0;

	int temp = 0;
	int i = 0;
	while(key[i] != '\0') {
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert (char* name, int lineno, int loc, char VPF, int type, int len, TreeNode* params) {

	int h = hash(name);
	BucketList l = NULL;
	if(scope_top() == NULL)
		return;
	
	l = scope_top()->hashTable[h];

	while(1) {
		if (l == NULL) break;
		if (strcmp(name, l->name) == 0) break;
		l = l->next;
	}
	if (l == NULL) { /* variable not yet in table */
		l = (BucketList) malloc (sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList) malloc (sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->lines->next = NULL;
		l->memloc = loc;

		/* Aux fields. */
		l->VPF = VPF;
		l->type = type;
		l->len = len;
		l->params = params;

		l->next = scope_top()->hashTable[h];
		scope_top()->hashTable[h] = l;
	}
	else { /* found in table, so just add line number */
		LineList t = l->lines;
		if (t != NULL) {
			while(t->next) t = t->next;
			t->next = (LineList) malloc (sizeof(struct LineListRec));
			t->next->lineno = lineno;
			t->next->next = NULL;
		}
	}
}

/* Procedure st_insert_global inserts line number to 
 * the correspoinding scope that stays global.
 */
void st_insert_global (char* name, int lineno) {

	BucketList l = st_lookup(name);
	if (l == NULL) {
		/* Do nothing */;
	}
	else {
		LineList t = l->lines;
		if (t != NULL) {
			while(t->next) t = t->next;
			t->next = (LineList) malloc (sizeof(struct LineListRec));
			t->next->lineno = lineno;
			t->next->next = NULL;
		}
	}
	return;
}

/* Funcion st_lookup returns the bucket list record
 * of a variable or NULL if not found.
 */
BucketList st_lookup (char* name) {

	int h = hash(name);
	struct ScopeListRec* sc = scope_top();
	BucketList l = NULL;
	while (sc) {
		l = sc->hashTable[h];
		while(1) {
			if (l == NULL) break;
			if (strcmp(name, l->name) == 0) break;
			l = l->next;
		}

		if (l == NULL)
			sc = sc->parent;
		else
			return l;
	}
	return NULL;
}

/* Function st_lookup_local returns the bucekt list record
 * of a variable in local scope, if not found, returns NULL
 */
BucketList st_lookup_local (char* name) {

	if (scope_top() == NULL)
		return NULL;

	int h = hash(name);
	BucketList l = scope_top()->hashTable[h];
	while(1) {
		if (l == NULL) break;
		if (strcmp(name, l->name) == 0) break;
		l = l->next;
	}
	return l;
}

/* Procedure printSymTab prints a formatted listing
 * of the symbol table contents
 * to the listing file.
 */
void printSymTab(FILE* listing) {

	int i, j;
	for (i = 0; i < scope_index ; i++) {

		if (scope[i] == NULL) continue;

		fprintf(listing, "%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-16s\n", 
			"Name", "Scope", "Loc", "V/P/F", "Array?", "ArrSize", "Type", "Line numbers");
		fprintf(listing, "------------------------------------------------------------------------------------\n");

		for (j=0; j < SIZE; j++) {

			BucketList l = scope[i]->hashTable[j];
			while(l != NULL) {
				fprintf(listing, "%-8s", l->name);
				fprintf(listing, "%-8d", scope[i]->level);
				fprintf(listing, "%-8d", l->memloc);

				switch(l->VPF) {
					case 'V': fprintf(listing, "%-8s", "Var"); break;
					case 'P': fprintf(listing, "%-8s", "Par"); break;
					case 'F': fprintf(listing, "%-8s", "Func"); break;
					default: ;
				}

				if (l->type == Array) {
					fprintf(listing, "%-8s", "Array");
					fprintf(listing, "%-8d", l->len);
				}
				else {
					fprintf(listing, "%-8s", "No");
					fprintf(listing, "%-8s", "-");
				}

				switch(l->type) {
					case Void:
						fprintf(listing, "%-8s", "void"); break;
					case Integer:
						fprintf(listing, "%-8s", "int"); break;
					case Array:
						fprintf(listing, "%-8s", "array"); break;
					default: ;
				}
				
				LineList t = l->lines;
				while(t != NULL) {
					fprintf(listing, "%-4d ", t->lineno);
					t = t->next;
				}
				fprintf(listing, "\n");
				l = l->next;
			}
		}
		fprintf(listing, "\n");
	}
}
