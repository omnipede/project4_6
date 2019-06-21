#ifndef _ANALYZE_H_
#define _ANALYZE_H_

/* Function insertIOFunctions inserts
 * input() and output() functions to the syntax tree
 */
void insertIOFunctions (TreeNode **);

/* Function buidSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab (TreeNode *);

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck (TreeNode *);
#endif
