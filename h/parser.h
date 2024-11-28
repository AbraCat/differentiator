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

ErrEnum treeParse(Node** node);
void syntaxErr(SyntaxErr* synt_err);

ErrEnum getG(Parser* pars, Node** node);
ErrEnum getE(Parser* pars, Node** node);
ErrEnum getT(Parser* pars, Node** node);
ErrEnum getP(Parser* pars, Node** node);
ErrEnum getN(Parser* pars, Node** node);

#endif // PARSER_H