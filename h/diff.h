#ifndef DIFF_H
#define DIFF_H

#include <tree.h>

typedef double (*EvalOneArg)(double);
typedef double (*EvalTwoArgs)(double, double);

ErrEnum nodeWriteTex(FILE* fout, Node* node);
ErrEnum treeWriteTex(Node* node);

void evaluate(Node* node, double x, double* ans);
ErrEnum diff(Node* node, Node** deriv);

void nodeIsConst(Node* node, int* ans);

int simplifyCase(Node* node, double crit_val, int result, int rgt);
void simplify(Node* node);

#endif // DIFF_H