

#include <diff.h>
#include <logs.h>
#include <utils.h>

static const int buffer_size = 300;

ErrEnum nodeWriteTex(FILE* fout, Node* node)
{
    #define OP_CASE(op)       \
        case OP_ ## op:       \
            fputs(#op, fout); \
            break

    if (node == NULL) return ERR_OK;
    if (node->type == TYPE_NUM)
    {
        fprintf(fout, "%lf", node->val.num);
        return ERR_OK;
    }
    if (node->type == TYPE_VAR)
    {
        fputc('x', fout);
        return ERR_OK;
    }
    myAssert(node->type == TYPE_OP);

    #define DIFF_OP(name, n_operands, eval, prior, text, tex_fmt) \
    if (node->val.op_code == OP_ ## name)                 \
    {                                                     \
        int priority_brackets = 0;                          \
        if (node->parent != NULL && prior != 0)          \
        {                                               \
            OpInfo* parent_op = NULL;                   \
            returnErr(getOpByCode(node->parent->val.op_code, &parent_op)) \
            if (parent_op->priority > prior)                     \
            {                                           \
                priority_brackets = 1; \
                fputc('(', fout);                       \
            }                                               \
        }                                                          \
        for (const char* fmt = tex_fmt; *fmt != '\0'; ++fmt)    \
        {                                                   \
            if (*fmt == '%')                                    \
            {                                               \
                ++fmt;                                      \
                if (*fmt == 'l') nodeWriteTex(fout, node->lft);    \
                else if (*fmt == 'r') nodeWriteTex(fout, node->rgt); \
                else myAssert(0);                                   \
            }                                                       \
            else fputc(*fmt, fout);                                 \
        }                                                         \
        if (priority_brackets) fputc(')', fout);                    \
        return ERR_OK;                                                     \
    }

    #include <operations.h>
    #undef DIFF_OP
    
    // if (node->val.op_code == OP_DIV)
    // {
    //     fputs("\\frac{", fout);
    //     nodeWriteTex(fout, node->lft);
    //     fputs("}{", fout);
    //     nodeWriteTex(fout, node->rgt);
    //     fputc('}', fout);
    //     return;
    // }

    // if (node->val.op_code == OP_ADD || node->val.op_code == OP_SUB || node->val.op_code == OP_MUL)
    // {
    //     OpInfo* op_descr = NULL;
    //     getOpByCode(node->val.op_code, &op_descr);
    //     myAssert(op_descr != NULL);

    //     int priority_brackets = 0;
    //     if ((node->val.op_code == OP_ADD || node->val.op_code == OP_SUB) && 
    //     node->parent != NULL && node->parent->val.op_code == OP_MUL)
    //         priority_brackets = 1;

    //     if (priority_brackets) fputc('(', fout);
    //     nodeWriteTex(fout, node->lft);
    //     fputs(op_descr->op_str, fout);
    //     nodeWriteTex(fout, node->rgt);
    //     if (priority_brackets) fputc(')', fout);
    //     return;
    // }

    myAssert("nodeWriteTex(): invalid node" && 0);
    return ERR_OK;
    #undef OP_CASE
}

ErrEnum treeWriteTex(Node* node)
{
    char tex_path[buffer_size] = "";
    sprintf(tex_path, "%s/tex/expr.tex", log_path);
    FILE* fout = fopen(tex_path, "a");
    if (fout == NULL) return ERR_OPEN_FILE;

    fputc('$', fout);
    returnErr(nodeWriteTex(fout, node));
    fputs("$\n", fout);

    fclose(fout);
    return ERR_OK;
}

void evaluate(Node* node, double x, double* ans)
{
    myAssert(node != NULL);

    if (node->type == TYPE_NUM)
    {
        *ans = node->val.num;
        return;
    }
    if (node->type == TYPE_VAR)
    {
        *ans = x;
        return;
    }
    myAssert(node->type == TYPE_OP);

    #define DIFF_OP(name, n_operands, eval, priority, text, tex_fmt) \
    case OP_ ## name:                                       \
    {                                                       \
        double arg1 = 0, arg2 = 0;                          \
        evaluate(node->lft, x, &arg1);                      \
        if (n_operands == 2) evaluate(node->rgt, x, &arg2); \
        *ans = arg1 eval arg2;                                     \
        return;                                             \
    }

    switch (node->val.op_code)
    {
        #include <operations.h>
        default: myAssert(0);
    }
    
    // #define OP_OPERATOR_CASE(op, operator)  \
    //     if (node->val.op_code == OP_ ## op) \
    //     {                                   \
    //         double y1 = 0, y2 = 0;          \
    //         evaluate(node->lft, x, &y1);    \
    //         evaluate(node->rgt, x, &y2);    \
    //         *ans = y1 operator y2;          \
    //         return;                         \
    //     }

    // OP_OPERATOR_CASE(ADD, +);
    // OP_OPERATOR_CASE(SUB, -);
    // OP_OPERATOR_CASE(MUL, *);
    // OP_OPERATOR_CASE(DIV, /);

    // #undef OP_OPERATOR_CASE
    return;
    #undef DIFF_OP
}

ErrEnum diff(Node* node, Node** deriv)
{
    myAssert(node != NULL && deriv != NULL && *deriv == NULL);

    if (node->type == TYPE_NUM)
    {
        returnErr(nodeCtor(deriv, TYPE_NUM, {.num = 0}, NULL, NULL, NULL));
        return ERR_OK;
    }
    if (node->type == TYPE_VAR)
    {
        returnErr(nodeCtor(deriv, TYPE_NUM, {.num = 1}, NULL, NULL, NULL));
        return ERR_OK;
    }
    myAssert(node->type == TYPE_OP);

    #define _ADD(node_name, lft, rgt) returnErr(nodeCtor(&node_name, TYPE_OP, {.op_code = OP_ADD}, NULL, lft, rgt))
    #define _SUB(node_name, lft, rgt) returnErr(nodeCtor(&node_name, TYPE_OP, {.op_code = OP_SUB}, NULL, lft, rgt))
    #define _MUL(node_name, lft, rgt) returnErr(nodeCtor(&node_name, TYPE_OP, {.op_code = OP_MUL}, NULL, lft, rgt))
    #define _DIV(node_name, lft, rgt) returnErr(nodeCtor(&node_name, TYPE_OP, {.op_code = OP_DIV}, NULL, lft, rgt))

    Node *deriv_lft = NULL, *deriv_rgt = NULL;
    diff(node->lft, &deriv_lft);
    if (node->rgt != NULL) diff(node->rgt, &deriv_rgt);
    switch (node->val.op_code)
    {
        case OP_ADD:
            _ADD(*deriv, deriv_lft, deriv_rgt);
            return ERR_OK;
        case OP_SUB:
            _SUB(*deriv, deriv_lft, deriv_rgt);
            return ERR_OK;
        case OP_MUL:
        {
            Node *lft_copy = NULL, *rgt_copy = NULL;
            returnErr(nodeCopy(node->lft, &lft_copy));
            returnErr(nodeCopy(node->rgt, &rgt_copy));

            Node *gdf = NULL, *fdg = NULL;
            _MUL(gdf, deriv_lft, rgt_copy);
            _MUL(fdg, lft_copy, deriv_rgt);
            _ADD(*deriv, gdf, fdg);
            return ERR_OK;
        }
        case OP_DIV:
        {
            return ERR_OK; // write
        }
        default: return ERR_INVAL_OP_CODE;
    }
}

int simplifyCase(Node* node, double crit_val, int result, int crit_in_rgt)
{
    myAssert(node != NULL);

    Node *crit_node = NULL, *expr_node = NULL;
    if (crit_in_rgt)
    {
        crit_node = node->rgt;
        expr_node = node->lft;
    }
    else
    {
        crit_node = node->lft;
        expr_node = node->rgt;
    }
    if (crit_node->type == TYPE_NUM && isZero(crit_node->val.num - crit_val))
    {
        if (result == 0)
        {
            node->type = TYPE_NUM;
            node->val.num = 0;
            nodeDtor(node->lft);
            nodeDtor(node->rgt);
            node->lft = NULL;
            node->rgt = NULL;
            return 1;
        }
        node->type = expr_node->type; // copyNode() ?
        node->val = expr_node->val;
        node->lft = expr_node->lft;
        node->rgt = expr_node->rgt;

        expr_node->lft = expr_node->rgt = NULL;
        if (node->lft != NULL) node->lft->parent = node;
        if (node->rgt != NULL) node->rgt->parent = node;
        nodeDtor(expr_node);
        nodeDtor(crit_node);
        return 1;
    }
    return 0;
}

void simplify(Node* node)
{
    // #define simplifyCase(crit_val, result, crit_in_right) \
    // if (crit_in_right) \
    // { \
    //     printf("%d %lf %lf %lf\n", isZero(node->rgt->val.num - crit_val), node->rgt->val.num, crit_val, node->rgt->val.num - crit_val); \
    //     if (node->rgt->type == TYPE_NUM && isZero(node->rgt->val.num - crit_val)) \
    //     { \
    //         printf("www\n"); \
    //         if (result == 0) \
    //         { \
    //             printf("eee\n"); \
    //             node->type = TYPE_NUM; \
    //             node->val.num = 0; \
    //             nodeDtor(node->lft); \
    //             nodeDtor(node->rgt); \
    //             node->lft = NULL; \
    //             node->rgt = NULL; \
    //             return; \
    //         } \
    //         node->lft = node->lft->lft;
    //         node->rgt = node->lft->rgt;
    //         node->
    //         printf("rrr\n"); \
    //         node->lft->parent = node->parent; \
    //         node->lft = NULL; \
    //         nodeDtor(node); \
    //         printf("ttt\n"); \
    //         return; \
    //     } \
    // } \
    // if (node->lft->type == TYPE_NUM && isZero(node->lft->val.num - crit_val)) \
    // { \
    //     if (result == 0) \
    //     { \
    //         node->type = TYPE_NUM; \
    //         node->val.num = 0; \
    //         nodeDtor(node->lft); \
    //         nodeDtor(node->rgt); \
    //         node->lft = NULL; \
    //         node->rgt = NULL; \
    //         return; \
    //     } \
    //     if (node->parent != NULL) \
    //     { \
    //         if (node == node->parent->lft) node->parent->lft = node->rgt;  \
    //         else node->parent->rgt = node->rgt; \
    //     } \
    //     node->rgt->parent = node->parent; \
    //     node->rgt = NULL; \
    //     nodeDtor(node); \
    //     return; \
    // }

    myAssert(node != NULL);

    if (node->type != TYPE_OP) return;
    simplify(node->lft);
    if (node->rgt != NULL) simplify(node->rgt);
    
    if (node->lft->type == TYPE_NUM && node->rgt != NULL && node->rgt->type == TYPE_NUM)
    {
        evaluate(node, 0, &(node->val.num));
        node->type = TYPE_NUM;
        nodeDtor(node->lft);
        nodeDtor(node->rgt);
        node->lft = NULL;
        node->rgt = NULL;
        return;
    }

    if (node->val.op_code == OP_ADD)
    {
        if (simplifyCase(node, 0.0, 1, 0)) return;
        if (simplifyCase(node, 0.0, 1, 1)) return;
    }
    if (node->val.op_code == OP_SUB) if (simplifyCase(node, 0, 1, 1)) return;
    if (node->val.op_code == OP_MUL)
    {
        if (simplifyCase(node, 0.0, 0, 0)) return;
        if (simplifyCase(node, 0.0, 0, 1)) return;
        if (simplifyCase(node, 1.0, 1, 0)) return;
        if (simplifyCase(node, 1.0, 1, 1)) return;
    }
}