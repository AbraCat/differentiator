#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#include <error.h>

const int small_buf_size = 100;

enum NodeType
{
    TYPE_OP,
    TYPE_VAR,
    TYPE_NUM,
};

enum OpEnum
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
};

union NodeVal
{
    double num;
    char var_id; // change
    OpEnum op_code;
};

struct Node
{
    NodeType type;
    NodeVal val;
    Node *parent, *lft, *rgt;
    int n_nodes;
};

struct OpDescr
{
    OpEnum op_code;
    const char *op_str;
    const int op_str_len;
};

ErrEnum nodeCtor(Node** node, NodeType type, NodeVal val, Node* parent, Node* lft, Node* rgt);
void nodeDtor(Node* node);

void nNodes(Node* node, int* ans);
ErrEnum nodeVerify(Node* node);

ErrEnum getOpByCode(OpEnum op_code, OpDescr** ans);
ErrEnum getOpByStr(const char* op_str, OpDescr** ans);

void printNodeDot(FILE* fout, Node* node);
ErrEnum treeMakeGraph(Node* tree);
ErrEnum treeDump(Node* tree);

void nodeWrite(FILE* fout, Node* node);
ErrEnum treeWrite(Node* node);
ErrEnum nodeRead(char* buf, int* buf_pos, Node* node, int* n_nodes, int buf_size);
ErrEnum treeRead(Node** tree);

void nodeWriteTex(FILE* fout, Node* node);
ErrEnum treeWriteTex(Node* node);

ErrEnum nodeCopy(Node* src, Node** dest);

void evaluate(Node* node, double x, double* ans);
ErrEnum diff(Node* node, Node** deriv);

#endif // TREE_H