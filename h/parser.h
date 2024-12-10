#ifndef PARSER_H
#define PARSER_H

#include <tree.h>

struct SyntaxErr
{
    int err, pos;
    const char *exp;
    char got;
};

struct Parser
{
    const char* s;
    int p, buf_size;
    SyntaxErr synt_err;
};

/*
grammar

G = E0$
E0 = E1{[+-]E1}*
E1 = E2{[*(slash)]E2}*
E2 = P{^P}*
P = (E0)|N|V|F
F = NAME(E0)
N = {-}[0-9]+{.[0-9]+}
V = x
NAME = [exp, ln, sin, cos, tan, arcsin, arccos, arctan]
*/

// TODO: static functions

ErrEnum treeParse(Node** node, const char *expr_txt_path);

#endif // PARSER_H