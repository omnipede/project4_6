#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

static int regSize = 4;
static char buffer[100];

static int cGen(TreeNode*);

static int allocGlobal (int inc) {
	static int ret = 0x10000000;
	/*
	if (inc >= 0) {
		ret += inc;
		return ret - inc;
	}
	else {
		return ret;
	}*/
	ret += inc;
	return ret - inc;
}

static int retGlobal (int addr, int size) {
	int org = 0x10000000;
	return (org + addr - size);
}

static int genLabel (void) {
	static int label = 0;
	return (label += 1);
}

static int genStmtCode (TreeNode* t) {
	int current_stack = 0;
	int L_exit, L_false;
	int L_cmp, L_loop;
	if (t == NULL)
		return current_stack;
	switch (t->kind.stmt) {
		/* Compound statement code */
		case CompoundK:
			emitComment("Compound statement");
			emitComment("Local var declaration");
			int allocated_stack = cGen(t->child[0]);
			cGen(t->child[1]);

			emitComment("Recover local stack");
			sprintf(buffer, "addiu $sp, $sp, %d", allocated_stack);
			emitCode(buffer);
			break;
		/* Selection statement code */
		case IfK: 
			/* Compute expression */
			emitComment ("Selection statement");
			emitComment ("Selection statement Expression");
			cGen(t->child[0]);
			L_exit = genLabel();
			L_false = genLabel();
			sprintf(buffer, "beqz $v0, L%d", L_false);
			emitCode(buffer);

			/* If part */
			emitComment ("Selection statement If part");
			cGen(t->child[1]);
			sprintf(buffer, "j L%d", L_exit);
			emitCode(buffer);

			/* ELSE part */
			sprintf(buffer, "L%d:", L_false);
			emitCode(buffer);
			emitComment ("Selection statement ELSE part");
			cGen(t->child[2]);
			sprintf(buffer, "L%d:", L_exit);
			emitCode(buffer);
			break;

		/* Iteration statement. */
		case WhileK: 
			emitComment ("While statement");
			L_cmp = genLabel ();
			L_loop = genLabel ();
			sprintf(buffer, "j L%d", L_cmp);
			emitCode(buffer);

			sprintf(buffer, "L%d:", L_loop);
			emitCode(buffer);
			cGen(t->child[1]);

			sprintf(buffer, "L%d:", L_cmp);
			emitCode(buffer);
			cGen(t->child[0]);
			sprintf(buffer, "bnez $v0, L%d", L_loop);
			emitCode(buffer);
			break;
		default: ;
	}
	return current_stack;
}

static int genExpCode (TreeNode* t) {
	int current_stack = 0;
	if (t == NULL)
		return current_stack;
	switch (t->kind.exp) {
		case OpK:
			if (t->child[0] == NULL || t->child[1] == NULL) break;
			/* Assign operation */
			if (t->attr.op == ASSIGN) {
				/* Generate right operand code */
				cGen(t->child[1]);
				/* If left operand is not array */
				if (t->child[0]->child[0] == NULL) {
					/* If left operand is global var */
					if (t->child[0]->isGlobal)
						sprintf(buffer, "sw $v0, %d", retGlobal(t->child[0]->memloc, sizeof(int)));
					/* If left operand is local var. */
					else
						sprintf(buffer, "sw $v0, %d($fp)", t->child[0]->memloc - 9 * regSize);
					emitCode(buffer);
				}
				/* Else if array */
				else {
					sprintf(buffer, "move $s1, $v0"); 
					emitCode(buffer);
					/* Generate left operand's array subscription code */
					cGen(t->child[0]->child[0]);
					sprintf(buffer, "li $s0, %lu", sizeof(int));
					emitCode(buffer);
					sprintf(buffer, "mul $s0, $v0, $s0");
					emitCode(buffer);
					/* If left operand is global */
					if (t->child[0]->isGlobal)
						sprintf(buffer, "li $v0, %d", 
								retGlobal(t->child[0]->memloc, t->child[0]->len * sizeof(int)) );
					/* Else if left operand is local */
					else
						sprintf(buffer, "addiu $v0, $fp, %d", t->child[0]->memloc - 9 * regSize);
					emitCode(buffer);
					sprintf(buffer, "add $v0, $v0, $s0");
					emitCode(buffer);

					sprintf(buffer, "sw $s1, 0($v0)");
					emitCode(buffer);
					sprintf(buffer, "move $v0, $s1");
					emitCode(buffer);
				}
				break;
			}
			/* Left operand */
			cGen(t->child[0]);
			sprintf(buffer, "addiu $sp, $sp, -%lu", sizeof(int));
			emitCode(buffer);
			sprintf(buffer, "sw $v0, 0($sp)");
			emitCode(buffer);
			/* Right operand */
			cGen(t->child[1]);
			sprintf(buffer, "lw $s0, 0($sp)");
			emitCode(buffer);
			sprintf(buffer, "addiu $sp, $sp, %lu", sizeof(int));
			emitCode(buffer);

			switch(t->attr.op) {
				case PLUS: sprintf(buffer, "add $v0, $s0, $v0"); break;
				case MINUS: sprintf(buffer, "sub $v0, $s0, $v0"); break;
				case TIMES: sprintf(buffer, "mul $v0, $s0, $v0"); break;
				case OVER: sprintf(buffer, "div $v0, $s0, $v0"); break;
				case LT: sprintf(buffer, "slt $v0, $s0, $v0"); break;
				case LE: sprintf(buffer, "sle $v0, $s0, $v0"); break;
				case GT: sprintf(buffer, "sgt $v0, $s0, $v0"); break;
				case GE: sprintf(buffer, "sge $v0, $s0, $v0"); break;
				case EQ: sprintf(buffer, "seq $v0, $s0, $v0"); break;
				case NE: sprintf(buffer, "sne $v0, $s0, $v0"); break;
				default: ;
			}
			emitCode(buffer);
			break;
		case IdK:
			if (t->child[0] == NULL) {
				if (t->isGlobal)
					sprintf(buffer, "lw $v0, %d", retGlobal(t->memloc, sizeof(int)));
				else
					sprintf(buffer, "lw $v0, %d($fp)", (t->memloc - 9 * regSize));	
				emitCode(buffer);
			}
			else { 
				/* Array subscription calc */
				cGen(t->child[0]);
				sprintf(buffer, "li $s0, %lu", sizeof(int));
				emitCode(buffer);
				sprintf(buffer, "mul $s0, $v0, $s0");
				emitCode(buffer);
				/* Array calc */
				if (t->isGlobal)
					sprintf(buffer, "li $v0, %d", retGlobal(t->memloc, t->len * sizeof(int)) );
				else
					sprintf(buffer, "addiu $v0, $fp, %d", (t->memloc - 9 * regSize));
				emitCode(buffer);
				sprintf(buffer, "add $v0, $v0, $s0");
				emitCode(buffer);
				sprintf(buffer, "lw $v0, 0($v0)");
				emitCode(buffer);
			}
			break;
		case ConstK:
			sprintf(buffer, "li $v0, %d", t->attr.val);
			emitCode(buffer);
			break;
		case CallK:
			if (strcmp(t->attr.name, "output") == 0) {

			}
			if (strcmp(t->attr.name, "input") == 0) {
				sprintf(buffer, "input: ");
				emitCode(buffer);
				sprintf(buffer, "li $v0, 4");
				emitCode(buffer);
				sprintf(buffer, "la $a0, input_str");
				emitCode(buffer);
				sprintf(buffer, "syscall");
				emitCode(buffer);
				sprintf(buffer, "li $v0, 5");
				emitCode(buffer);
				sprintf(buffer, "syscall");
				emitCode(buffer);
			}
			break;
		default: ;
	}

	return current_stack;
}

static int genDeclCode (TreeNode* t) {
	
	int i;
	int clean_label = 0;
	int current_stack = 0;
	if (t == NULL) 
		return current_stack;

	switch(t->kind.decl) {
		case VarK:
			if (t->child[0] == NULL) break;
			if (t->isGlobal) {
				/* Global variable declaration */
				if (t->child[0]->type != Array) 
					t->memloc = allocGlobal (sizeof(int));
				else
					t->memloc = allocGlobal (sizeof(int) * t->len);
			}
			else {
				/* Local variable declaration */
				if (t->child[0]->type != Array) {
					sprintf(buffer, "addiu $sp, $sp, %d", -sizeof(int));
					emitCode(buffer);
					current_stack += sizeof(int);
				}
				else {
					sprintf(buffer, "addiu $sp, $sp, %d", -regSize * t->child[0]->len);
					emitCode(buffer);
					current_stack += regSize * t->child[0]->len;
				}
			}
			break;
		case FunK:
			/* If output or input, ignore. */
			if (strcmp(t->attr.name, "output") == 0 || strcmp(t->attr.name, "input") ==0)
				break;

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
			/* Save fp */
			sprintf(buffer, "sw $fp, %d($sp)", 0);
			emitCode(buffer);
			/* Save s0 ~ s7 */
			for (i = 0; i < 8; i++) {
				sprintf(buffer, "sw $s%d, %d($sp)", i, (i+1) * regSize);
				emitCode(buffer);
			}
			/* Save ra */
			sprintf(buffer, "sw $ra, %d($sp)", 9 * regSize);
			emitCode(buffer);
			/* Set fp to top of stack frame */
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
			/* Set sp to end of stack frame */
			sprintf(buffer, "addiu $sp, $fp, %d\n", -10 * regSize);
			emitCode(buffer);
			/* Recover fp */
			sprintf(buffer, "lw $fp, %d($sp)", 0 * regSize);
			emitCode(buffer);
			/* Recover s0 ~ s7 */
			for (i = 0; i < 8; i++) {
				sprintf(buffer, "lw $s%d, %d($sp)", i, (i+1) * regSize);
				emitCode(buffer);
			}
			/* Recover ra */
			sprintf(buffer, "lw $ra, %d($sp)", 9 * regSize);
			emitCode(buffer);
			/* Recover stack frame */
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

	return current_stack;
}

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static int cGen (TreeNode* t) {

	int current_stack = 0;
	if (t == NULL) 
		return current_stack;

	switch(t ->nodekind) {
		case StmtK:
			current_stack += genStmtCode(t);
			break;
		case ExpK:
			current_stack += genExpCode(t);
			break;
		case DeclK:
			current_stack += genDeclCode(t);
			break;
		default: ;
	}
	current_stack += cGen(t->sibling);

	return current_stack;
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
