#ifndef DIFF_H
#define DIFF_H

#include <tree.h>

typedef double (*EvalOneArg)(double);
typedef double (*EvalTwoArgs)(double, double);

ErrEnum nodeWriteTex(FILE* fout, Node* node, int parent_priority);
ErrEnum treeWriteTex(FILE* fout, Node* node);
ErrEnum makeArticle(Node* node, const char* fout_path);

void evaluate(Node* node, double x, double* ans);
ErrEnum diff(Node* node, Node** deriv);
ErrEnum diffWrite(FILE* fout, Node* node, Node** deriv);

void nodeIsConst(Node* node, int* ans);

int simplifyCase(Node* node, NodeChild crit_child, double crit_val, int replace_with_num, int replacement);
void simplify(Node* node);

#endif // DIFF_H