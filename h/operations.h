// DIFF_OP(name, n_operands, value, priority, text, tex_fmt)

DIFF_OP(ADD, 2, +, 1, "+", "%l+%r")
DIFF_OP(SUB, 2, -, 1, "-", "%l-%r")
DIFF_OP(MUL, 2, *, 2, "*", "%l*%r")
DIFF_OP(DIV, 2, /, 0, "/", "\\frac{%l}{%r}")