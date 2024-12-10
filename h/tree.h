#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#include <error.h>

extern const int n_ops;
const int small_buf_size = 100;

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
    #undef DIFF_OP
};

enum NodeChild
{
    LFT_NODE,
    RGT_NODE
};

struct OpInfo
{
    OpEnum op_code;
    const char *op_str, *tex_fmt;
    const int op_str_len, priority;
};

union NodeVal
{
    double num;
    char var_id;
    OpEnum op_code;
};

struct Node
{
    NodeType type;
    NodeVal val;
    Node *parent, *lft, *rgt;

    // debug
    int visited;
};

ErrEnum nodeCtor(Node** node, NodeType type, NodeVal val, Node* parent, Node* lft, Node* rgt);
void nodeDtor(Node* node);

void nNodes(Node* node, int* ans);
ErrEnum nodeVerify(Node* node);

ErrEnum getOpByCode(OpEnum op_code, OpInfo** ans);
ErrEnum getOpByStr(const char* op_str, OpInfo** ans);

ErrEnum printNodeDot(FILE* fout, Node* node);
ErrEnum treeMakeGraph(Node* tree);
ErrEnum treeDump(Node* tree);

void nodeWrite(FILE* fout, Node* node);
ErrEnum treeWrite(Node* node, const char* expr_brackets_path);
ErrEnum nodeRead(char* buf, int* buf_pos, Node* node, int buf_size);
ErrEnum treeRead(Node** tree, const char* expr_brackets_path);

ErrEnum nodeCopy(Node* src, Node** dest);

#endif // TREE_H