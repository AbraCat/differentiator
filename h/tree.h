#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#include <error.h>

const int small_buf_size = 100;

#define _EMPTY(node) returnErr(nodeCtor(node, TYPE_NUM, {.num = 0}, NULL, NULL, NULL));
#define _NODE(node, type, val, parent, lft, rgt) returnErr(nodeCtor(node, type, val, parent, lft, rgt))
#define _NUM(node, val, parent, lft, rgt) returnErr(nodeCtor(node, TYPE_NUM, {.num = val}, parent, lft, rgt))

enum NodeType
{
    TYPE_OP,
    TYPE_VAR,
    TYPE_NUM,
};

enum OpEnum
{
    #define DIFF_OP(name, n_operands, eval, priority, text, tex_fmt) OP_ ## name,
    #include <operations.h>
    OP_INVAL,
    #undef DIFF_OP
};

struct OpInfo
{
    OpEnum op_code;
    const char *op_str;
    const int op_str_len, priority;
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

ErrEnum nodeCtor(Node** node, NodeType type, NodeVal val, Node* parent, Node* lft, Node* rgt);
void nodeDtor(Node* node);

void nNodes(Node* node, int* ans);
ErrEnum nodeVerify(Node* node);

ErrEnum getOpByCode(OpEnum op_code, OpInfo** ans);
ErrEnum getOpByStr(const char* op_str, OpInfo** ans);

void printNodeDot(FILE* fout, Node* node);
ErrEnum treeMakeGraph(Node* tree);
ErrEnum treeDump(Node* tree);

void nodeWrite(FILE* fout, Node* node);
ErrEnum treeWrite(Node* node);
ErrEnum nodeRead(char* buf, int* buf_pos, Node* node, int* n_nodes, int buf_size);
ErrEnum treeRead(Node** tree);

ErrEnum nodeCopy(Node* src, Node** dest);

#endif // TREE_H