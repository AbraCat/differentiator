// DIFF_OP(name, n_operands, value, priority, text, tex_fmt)
DIFF_OP(ADD,    2, add,  1, "+",      "%l+%r")
DIFF_OP(SUB,    2, sub,  1, "-",      "%l-%r")
DIFF_OP(MUL,    2, mul,  2, "*",      "%l*%r")
DIFF_OP(DIV,    2, div,  0, "/",      "\\frac{%l}{%r}")
DIFF_OP(POW,    2, pow,  3, "^",      "%l^{%r}")
DIFF_OP(EXP,    1, exp,  0, "exp",    "exp(%l)")
DIFF_OP(LN,     1, log,  0, "ln",     "ln(%l)")
DIFF_OP(SIN,    1, sin,  0, "sin",    "sin(%l)")
DIFF_OP(COS,    1, cos,  0, "cos",    "cos(%l)")
DIFF_OP(TAN,    1, tan,  0, "tan",    "tan(%l)")
DIFF_OP(ARCSIN, 1, asin, 0, "arcsin", "arcsin(%l)")
DIFF_OP(ARCCOS, 1, acos, 0, "arccos", "arccos(%l)")
DIFF_OP(ARCTAN, 1, atan, 0, "arctan", "arctan(%l)")