#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

static int regSize = 4;
static char buffer[100];

static void cGen(TreeNode*);

static int genLabel (void) {
	static int label = 0;
	return (label += 1);
}

static void genStmtCode (TreeNode* t) {
	switch (t->kind.stmt) {
		case CompoundK:
			break;
		default: ;
	}
}

static void genExpCode (TreeNode* t) {

}

static void genDeclCode (TreeNode* t) {
	
	int i;
	int clean_label = 0;
	if (t == NULL) 
		return;

	switch(t->kind.decl) {
		case VarK:
			break;
		case FunK:
			/* Set function label. */
			emitComment("Function declaration");
			sprintf(buffer, "%s:", t->attr.name);
			emitCode(buffer);

			/* Allocate stack frame */
			emitComment("Stack frame allocation");
			sprintf(buffer, "addiu $sp, $sp, %d", -10 * regSize);
			emitCode(buffer);

			/* Save registers */
			emitComment("Save registers");
			sprintf(buffer, "sw $fp, %d($sp)", 0);
			emitCode(buffer);			
			for (i = 0; i < 8; i++) {
				sprintf(buffer, "sw $s%d, %d($sp)", i, (i+1) * regSize);
				emitCode(buffer);
			}
			sprintf(buffer, "sw $ra, %d($sp)", 9 * regSize);
			emitCode(buffer);
			sprintf(buffer, "addiu $fp, $sp, %d", 10 * regSize);
			emitCode(buffer);

			/* Body of function */
			clean_label = genLabel();
			emitComment("Function body");
			cGen(t->child[2]);
			
			/* Stack frame recovery */
			emitComment("Recover stack");
			sprintf(buffer, "L%d:", clean_label);
			emitCode(buffer);
			sprintf(buffer, "addiu $sp, $fp, %d\n", -10 * regSize);
			emitCode(buffer);

			/* Recover registers */
			sprintf(buffer, "lw $fp, %d($sp)", 0 * regSize);
			emitCode(buffer);
			for (i = 0; i < 8; i++) {
				sprintf(buffer, "lw $s%d, %d($sp)", i, (i+1) * regSize);
				emitCode(buffer);
			}
			sprintf(buffer, "lw $ra, %d($sp)", 9 * regSize);
			emitCode(buffer);
			sprintf(buffer, "addiu $sp, $sp, %d", 10 * regSize);
			emitCode(buffer);

			/* Return to caller. */
			emitComment("Return");
			sprintf(buffer, "jr $ra\n");
			emitCode(buffer);
			break;
		case ParamK:
			break;
		default: ;
	}
}

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen (TreeNode* t) {

	if (t == NULL) 
		return;

	switch(t ->nodekind) {
		case StmtK:
			genStmtCode(t);
			break;
		case ExpK:
			genExpCode(t);
			break;
		case DeclK:
			genDeclCode(t);
			break;
		default: ;
	}
	cGen(t->sibling);
	return;
}

/* Procedure codeGen generates code to a code file
 * by traversal of the syntax tree.
 * The second parameter (codefile) is the file name of
 * the codefile, and is used to print the
 * file name as a comment in the code file.
 */
void codeGen (TreeNode* syntaxTree, char* codefile) {

	if (syntaxTree == NULL || codefile == NULL)
		return;

	emitCode(".data");
	emitCode("newline: .asciiz \"\\n\"");
	emitCode("output_str: .asciiz \"Output : \"");
	emitCode("input_str: .asciiz \"Input : \"");

	emitCode("\n.text");

	cGen(syntaxTree);
	return;
}
