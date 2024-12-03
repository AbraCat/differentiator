#include <math.h>

#include <diff.h>
#include <logs.h>
#include <utils.h>

static const int buffer_size = 300;

static double add(double a, double b) { return a + b; }
static double sub(double a, double b) { return a - b; }
static double mul(double a, double b) { return a * b; }
static double div(double a, double b) { return a / b; }

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
        {                                                       \
            if (*fmt == '%')                                    \
            {                                                   \
                ++fmt;                                          \
                if (*fmt == 'l') nodeWriteTex(fout, node->lft);    \
                else if (*fmt == 'r') nodeWriteTex(fout, node->rgt); \
                else myAssert("Invalid format specifier" && 0);      \
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

    fputs("\\[", fout);
    returnErr(nodeWriteTex(fout, node));
    fputs("\\]\n", fout);

    fclose(fout);
    return ERR_OK;
}

ErrEnum treeWriteTex(FILE* fout, Node* node)
{
    fputs("\\[", fout);
    returnErr(nodeWriteTex(fout, node));
    fputs("\\]\n", fout);

    return ERR_OK;
}

ErrEnum makeArticle(Node* node)
{
    char article_path[buffer_size] = "";
    sprintf(article_path, "%s/tex/article.tex", log_path);
    FILE* fout = fopen(article_path, "w");
    if (fout == NULL) return ERR_OPEN_FILE;

    fputs("\\begin{document}\nИсходное выражение:\n", fout);
    treeWriteTex(node);
    Node* deriv = NULL;
    // change diff() so it writes node to tex after call and before return ?
    returnErr(diff(node, &deriv));
    fputs("Ответ:\n", fout);
    treeWriteTex(deriv);
    fputs("\\end{document}\n", fout);

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
        if (n_operands == 1) {*ans = 0; "((EvalOneArg)eval)(arg1);";} \
        else if (n_operands == 2)                           \
        {                                                   \
            evaluate(node->rgt, x, &arg2); \
            *ans = 0; "((EvalTwoArgs)eval)(arg1, arg2);";                        \
        }                                                   \
        else myAssert(0);                                   \
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

void nodeIsConst(Node* node, int* ans)
{
    myAssert(ans != NULL);
    if (node == NULL)
    {
        *ans = 1;
        return;
    }
    int ans_child = 0;
    nodeIsConst(node->lft, &ans_child);
    if (!ans_child)
    {
        *ans = 0;
        return;
    }
    nodeIsConst(node->rgt, &ans_child);
    if (!ans_child)
    {
        *ans = 0;
        return;
    }
    if (node->type == TYPE_VAR)
    {
        *ans = 0;
        return;
    }
    *ans = 1;
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

    Node *deriv_lft = NULL, *deriv_rgt = NULL;
    diff(node->lft, &deriv_lft);
    if (node->rgt != NULL) diff(node->rgt, &deriv_rgt);
    switch (node->val.op_code)
    {
        case OP_ADD:
            _ADD(deriv, NULL, deriv_lft, deriv_rgt);
            return ERR_OK;
        case OP_SUB:
            _SUB(deriv, NULL, deriv_lft, deriv_rgt);
            return ERR_OK;
        case OP_MUL:
        {
            Node *lft_copy = NULL, *rgt_copy = NULL;
            returnErr(nodeCopy(node->lft, &lft_copy)); // DSL for copy?
            returnErr(nodeCopy(node->rgt, &rgt_copy));

            Node *gdf = NULL, *fdg = NULL;
            _MUL(&gdf, NULL, deriv_lft, rgt_copy);
            _MUL(&fdg, NULL, lft_copy, deriv_rgt);
            _ADD(deriv, NULL, gdf, fdg);
            return ERR_OK;
        }
        case OP_DIV:
        {
            Node *lft_copy = NULL, *rgt_copy = NULL;
            returnErr(nodeCopy(node->lft, &lft_copy));
            returnErr(nodeCopy(node->rgt, &rgt_copy));

            Node *gdf = NULL, *fdg = NULL, *gdf_minus_fdg = NULL;
            _MUL(&gdf, NULL, deriv_lft, rgt_copy);
            _MUL(&fdg, NULL, lft_copy, deriv_rgt);
            _SUB(&gdf_minus_fdg, NULL, gdf, fdg);

            Node *num2 = NULL, *rgt_copy2 = NULL, *g_pow_2 = NULL;
            _NUM(&num2, 2, NULL);
            returnErr(nodeCopy(node->rgt, &rgt_copy2));
            _POW(&g_pow_2, NULL, rgt_copy2, num2);
            _DIV(deriv, NULL, gdf_minus_fdg, g_pow_2);
            return ERR_OK;
        }
        case OP_POW:
        {
            int f_const = 0, g_const = 0;
            nodeIsConst(node->lft, &f_const);
            nodeIsConst(node->rgt, &g_const);
            if (f_const && g_const)
            {
                // (c1 ^ c2)' = 0
                _NUM(deriv, 0, NULL);
                return ERR_OK;
            }
            if (!f_const && g_const)
            {
                // (f ^ c)' = c * f ^ {c - 1}
                double g_val = 0;
                evaluate(node->rgt, 0, &g_val);
                Node *c_minus_1 = NULL, *f_copy = NULL, *f_pow_cm1 = NULL, *c = NULL;
                _NUM(&c_minus_1, g_val - 1, NULL);
                returnErr(nodeCopy(node->lft, &f_copy));
                _POW(&f_pow_cm1, NULL, f_copy, c_minus_1);
                _NUM(&c, g_val, NULL);
                _MUL(deriv, NULL, c, f_pow_cm1);
                return ERR_OK;
            }
            if (f_const && !g_const)
            {
                // (c ^ g)' = lnc * c ^ g
                double f_val = 0;
                evaluate(node->lft, 0, &f_val);
                Node *g_copy = NULL, *c = NULL, *c_pow_g = NULL, *lnc = NULL;
                returnErr(nodeCopy(node->rgt, &g_copy));
                _NUM(&c, f_val, NULL);
                _POW(&c_pow_g, NULL, c, g_copy);
                _NUM(&lnc, log(f_val), NULL); // calculate lnc or create ln node ?
                _MUL(deriv, NULL, lnc, c_pow_g);
                return ERR_OK;
            }
            // (f ^ g)' = (exp(g * ln(f)))' = f ^ g * (g' * lnf + g * f' / f)

            Node *f_copy_1 = NULL, *lnf = NULL, *dg_mul_lnf = NULL, *g_copy_1 = NULL, 
            *gdf = NULL, *f_copy_2 = NULL, *gdf_div_f = NULL;

            returnErr(nodeCopy(node->lft, &f_copy_1));
            _LN(&lnf, NULL, f_copy_1);
            _MUL(&dg_mul_lnf, NULL, deriv_rgt, lnf);
            returnErr(nodeCopy(node->rgt, &g_copy_1));
            _MUL(&gdf, NULL, g_copy_1, deriv_lft);
            returnErr(nodeCopy(node->lft, &f_copy_2));
            _DIV(&gdf_div_f, NULL, gdf, f_copy_2);

            Node *crocodile = NULL, *f_copy_3 = NULL, *g_copy_2 = NULL, *f_pow_g = NULL;

            _ADD(&crocodile, NULL, dg_mul_lnf, gdf_div_f);
            returnErr(nodeCopy(node->lft, &f_copy_3));
            returnErr(nodeCopy(node->rgt, &g_copy_2));
            _POW(&f_pow_g, NULL, f_copy_3, g_copy_2);
            _MUL(deriv, NULL, f_pow_g, crocodile);

            return ERR_OK;
        }
        case OP_EXP:
        {
            Node *exp_f = NULL;
            returnErr(nodeCopy(node, &exp_f));
            _MUL(deriv, NULL, deriv_lft, exp_f);
            return ERR_OK;
        }
        case OP_LN:
        {
            Node *f_copy = NULL;
            returnErr(nodeCopy(node->lft, &f_copy));
            _DIV(deriv, NULL, deriv_lft, f_copy);
            return ERR_OK;
        }
        case OP_SIN:
        {
            Node *f_copy = NULL, *cos_f = NULL;
            returnErr(nodeCopy(node->lft, &f_copy));
            _COS(&cos_f, NULL, f_copy);
            _MUL(deriv, NULL, cos_f, deriv_lft);
            return ERR_OK;
        }
        case OP_COS:
        {
            // unary minus / *(-1)
            Node *f_copy = NULL, *sin_f = NULL;
            returnErr(nodeCopy(node->lft, &f_copy));
            _SIN(&sin_f, NULL, f_copy);
            _MUL(deriv, NULL, sin_f, deriv_lft);
            return ERR_OK;
        }
        case OP_TAN:
        {
            Node *f_copy = NULL, *cosf = NULL, *num2 = NULL, *cosf_squared = NULL;
            _COPY(node->lft, &f_copy);
            _COS(&cosf, NULL, f_copy);
            _NUM(&num2, 2, NULL);
            _POW(&cosf_squared, NULL, cosf, num2);
            _DIV(deriv, NULL, deriv_lft, cosf_squared);
            return ERR_OK;
        }
        case OP_ARCSIN:
        {
            // add OP_SQRT ?

            Node *f_copy = NULL, *num2 = NULL, *f_sqared = NULL, *num1 = NULL, 
            *one_minus_f_sq = NULL, *num_0p5 = NULL, *sqrt_crocodile = NULL;

            _COPY(node->lft, &f_copy);
            _NUM(&num2, 2, NULL);
            _POW(&f_sqared, NULL, f_copy, num2);
            _NUM(&num1, 1, NULL);
            _SUB(&one_minus_f_sq, NULL, num1, f_sqared);
            _NUM(&num_0p5, 0.5, NULL);
            _POW(&sqrt_crocodile, NULL, one_minus_f_sq, num_0p5);
            _DIV(deriv, NULL, deriv_lft, sqrt_crocodile);
            return ERR_OK;
        }
        case OP_ARCCOS:
        {
            // avoid copypasting from arcsin
            // unary - or *(-1)

            Node *f_copy = NULL, *num2 = NULL, *f_sqared = NULL, *num1 = NULL, 
            *one_minus_f_sq = NULL, *num_0p5 = NULL, *sqrt_crocodile = NULL;

            _COPY(node->lft, &f_copy);
            _NUM(&num2, 2, NULL);
            _POW(&f_sqared, NULL, f_copy, num2);
            _NUM(&num1, 1, NULL);
            _SUB(&one_minus_f_sq, NULL, num1, f_sqared);
            _NUM(&num_0p5, 0.5, NULL);
            _POW(&sqrt_crocodile, NULL, one_minus_f_sq, num_0p5);
            _DIV(deriv, NULL, deriv_lft, sqrt_crocodile);
            return ERR_OK;
            return ERR_OK;
        }
        case OP_ARCTAN:
        {
            Node *f_copy = NULL, *num2 = NULL, *f_squared = NULL, *num1 = NULL, *one_plus_f2 = NULL;
            _COPY(node->lft, &f_copy);
            _NUM(&num2, 2, NULL);
            _POW(&f_squared, NULL, f_copy, num2);
            _NUM(&num1, 1, NULL);
            _ADD(&one_plus_f2, NULL, num1, f_squared);
            _DIV(deriv, NULL, deriv_lft, one_plus_f2);
            return ERR_OK;
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
        if (result != -1) // enum ?
        {
            node->type = TYPE_NUM;
            node->val.num = result;
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
        if (simplifyCase(node, 0, -1, 0)) return; // macro for "if (...) return;" ?
        if (simplifyCase(node, 0, -1, 1)) return;
    }
    if (node->val.op_code == OP_SUB) if (simplifyCase(node, 0, -1, 1)) return; // 0 - f ?
    if (node->val.op_code == OP_MUL)
    {
        if (simplifyCase(node, 0, 0,  0)) return;
        if (simplifyCase(node, 0, 0,  1)) return;
        if (simplifyCase(node, 1, -1, 0)) return;
        if (simplifyCase(node, 1, -1, 1)) return;
    }
    if (node->val.op_code == OP_DIV) if (simplifyCase(node, 1, -1, 1)) return;
    if (node->val.op_code == OP_POW)
    {
        if (simplifyCase(node, 0, 1,  1)) return;
        if (simplifyCase(node, 1, -1,  1)) return;
        if (simplifyCase(node, 0, 0,  0)) return;
        if (simplifyCase(node, 1, 1,  0)) return;
    }
}