#include <stdlib.h>

#include <parser.h>
#include <utils.h>
#include <colors.h>

#define cur_chr (pars->s[pars->p])
#define INCR_P ++(pars->p)
#define SYNT_ERR(exp) {pars->synt_err = {1, pars->p, exp, cur_chr}; return ERR_OK;}
#define NO_SYNT_ERR {pars->synt_err.err = 0; return ERR_OK;}
#define RET_SYNT_ERR if (pars->synt_err.err) return ERR_OK
#define CALL_SYNT_ERR if (pars->synt_err.err) syntaxErr(&pars->synt_err)

static const char expr_path[] = "./txt/expr.txt";

ErrEnum treeParse(Node** node)
{
    myAssert(node != NULL && *node == NULL);

    Parser pars = {};
    returnErr(readFile(expr_path, (void**)(&(pars.s)), &pars.buf_size));

    returnErr(getG(&pars, node));
    return ERR_OK;
}

void syntaxErr(SyntaxErr* synt_err)
{
    myAssert(synt_err != NULL && synt_err->err);

    printf("%sSyntax error at pos %d: expected %s, got %c%s\n", 
    RED_STR, synt_err->pos, synt_err->exp, synt_err->got, DEFAULT_STR);
    exit(0);
}

ErrEnum getG(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    returnErr(getE(pars, node));
    CALL_SYNT_ERR;
    if (cur_chr != '$')
    {
        SYNT_ERR("$");
        CALL_SYNT_ERR;
    }
    INCR_P;
    return ERR_OK;
}

ErrEnum getE(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    returnErr(getT(pars, node));
    RET_SYNT_ERR;
    while (cur_chr == '+' || cur_chr == '-')
    {
        _NODE(&(*node)->parent, TYPE_OP, {.op_code = OP_ADD}, NULL, *node, NULL);
        *node = (*node)->parent;
        if (cur_chr == '-') (*node)->val.op_code = OP_SUB;
        INCR_P;
        returnErr(getT(pars, &(*node)->rgt));
        RET_SYNT_ERR;
        (*node)->rgt->parent = *node;
    }
    NO_SYNT_ERR;
}

ErrEnum getT(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    returnErr(getP(pars, node));
    RET_SYNT_ERR;
    while (cur_chr == '*' || cur_chr == '/')
    {
        _NODE(&(*node)->parent, TYPE_OP, {.op_code = OP_MUL}, NULL, *node, NULL);
        *node = (*node)->parent;
        if (cur_chr == '/') (*node)->val.op_code = OP_DIV;
        INCR_P;
        returnErr(getP(pars, &(*node)->rgt));
        RET_SYNT_ERR;
        (*node)->rgt->parent = *node;
    }
    NO_SYNT_ERR;
}

ErrEnum getP(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    if (cur_chr != '(')
    {
        returnErr(getN(pars, node));
        RET_SYNT_ERR;
        NO_SYNT_ERR;
    }
    INCR_P;
    returnErr(getE(pars, node));
    RET_SYNT_ERR;
    if (cur_chr != ')') SYNT_ERR(")");
    INCR_P;
    NO_SYNT_ERR;
}

ErrEnum getN(Parser* pars, Node** node)
{
    myAssert(pars != NULL && node != NULL && *node == NULL);

    int old_p = pars->p;
    long long val = 0;
    while (cur_chr >= '0' && cur_chr <= '9')
    {
        val = val * 10 + cur_chr - '0';
        INCR_P;
    }
    if (pars->p == old_p) SYNT_ERR("[0-9]");

    _NUM(node, (double)val, NULL);
    NO_SYNT_ERR;
}