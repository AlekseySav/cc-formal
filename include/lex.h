#pragma once
#include <queue>

struct lexvalue {
    int as_number;
    std::string as_string;
    std::string as_symbol;
    lexvalue& operator=(const lexvalue&) = default;
};

class LexerState {
public:
    LexerState(file& input) : input(input) {}
    int token(bool eof_is_ok = false);
    bool tryToken(int to);
    void assertToken(int to);
    void enqueue(int token, lexvalue* value = nullptr);
public:
    std::queue<std::pair<int, lexvalue>> queue;
    lexvalue value;
    file& input;
private:
    int read_number(char first);
    int read_quoted(char quote);
    int read_symbol(char first);
    int if_sequence(char guess, int if_true, int if_false);
    bool is_comment();
};

enum {
    L_eof,
    L_symbol,
    L_number,
    L_string,

    /* brackets */
    L_open_bracket,     /* [ */
    L_close_bracket,    /* ] */
    L_open_paran,       /* ( */
    L_close_paran,      /* ) */

    /* postfix operators */
    L_inc, L_inc_post = L_inc,
    L_dec, L_dec_post = L_dec,
    L_call,             /* a() */
    L_index,            /* a[] */
    L_dot,
    L_arrow,

    /* prefix operators */
    L_inc_pref,
    L_dec_pref,
    L_plus,
    L_minus,
    L_not,              /* !a */
    L_invert,           /* ~a */
    L_cast,
    L_star,             /* *a */
    L_addr,             /* &a */
    
    /* binary operators */
    L_mul,
    L_div,
    L_mod,

    L_add,
    L_sub,

    L_shl,
    L_shr,

    L_and,
    L_xor,
    L_or,

    L_lt,               /* < */
    L_le,               /* <= */
    L_gt,               /* > */
    L_ge,               /* >= */

    L_equ,
    L_neq,

    L_andl,             /* && */
    L_orl,              /* || */

    L_question,
    L_colon,            /* : */

    L_assign,
    L_imul, L_idiv, L_imod,
    L_iadd, L_isub,
    L_ishl, L_ishr,
    L_iand, L_ixor, L_ior,

    L_comma,

    L_n_operators,

    /* tokens, that are unused in expressions */
    L_open_curly,       /* { */
    L_close_curly,      /* } */
    L_semicolon,        /* ; */

    /* call with no args pushed */
    L_empty_expr,

    /* more opcodes */
    L_put_local, /* mov ax, [bp+value] */
    L_put_local_reference, /* lea ax, [bp+value] */
    L_put_global, /* mov ax, [value] */
    L_put_global_reference, /* lea ax, value */
    L_put_number, /* mov ax, value */
    L_put_string, /* mov ax, Svalue */
    L_index_reference, /* &b[a] */

    L_push, /* link for binary operator */

    L_enter_function,
    L_leave_function,
    L_adjust_stack,

    L_jump,
    L_conditional_jump,

    L_put_label,
    L_define_string,
    
    /* internal */
    L_bad,
    L_space,
    L_newline,
    L_quote,
};

// shortcut for getting bracket
#define L_brace(c) ( \
    (c) == '(' ? L_open_paran : \
    (c) == ')' ? L_close_paran : \
    (c) == '[' ? L_open_bracket : \
    (c) == ']' ? L_close_bracket : \
    (c) == '{' ? L_open_curly : \
    (c) == '}' ? L_close_curly : 0)

#define L_closebrace(op) ((op) + 1)

// shortcut for inplace version of operator
#define L_inplace(op) \
    ((op) + L_imul - L_mul)

// have inplace alternative
#define L_inplacable(op) \
    ((op) >= L_mul && (op) <= L_or)
