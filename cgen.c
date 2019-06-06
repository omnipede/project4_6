#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen (TreeNode* tree) {

	if (tree != NULL) {

	}
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
