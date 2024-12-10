#include <stdio.h>
#include <string.h>

#include <diff.h>
#include <error.h>
#include <utils.h>
#include <parser.h>

#include <stdlib.h>

static const char expr_txt_path[] = "./txt/expr.txt";
static const char article_path[] = "./log/tex/article.tex";

int main(int argc, const char* argv[])
{
    Node* root = NULL;
    handleErr(treeParse(&root, expr_txt_path));

    handleErr(makeArticle(root, article_path));

    nodeDtor(root);
    return 0;
}