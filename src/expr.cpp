#include <cc.h>
#include "parse.h"

#define op_bad(t)       ((t) > L_n_operators)
#define op_right(t)     (optab[(t)] & 0x0800)
#define op_brace(t)     (optab[(t)] & 0x1000)
#define op_value(t)     (optab[(t)] & 0x2000)
#define op_binary(t)    (optab[(t)] & 0x8000)
#define op_prec(t)      (optab[(t)] & 0x00ff)

static u_int16_t optab[L_n_operators] = {
    0x00ff,                         /* (eof) used as separator */
    /* values */
    0x2000, 0x2000, 0x2000,
    /* brackets */
    0x1000, 0x1000, 0x1000, 0x1000,
    /* postfix */
    0x0001, 0x0001,                 /* a++ a-- */
    0x8001, 0x8001,                 /* a() a[] */
    0x8001, 0x8001,                 /* a. a-> */
    /* prefix */
    0x0802, 0x0802, 0x0802, 0x0802, 0x0802, 0x0802, 0x0802, 0x0802, 0x0802,
    /* binary operators */
    0x8003, 0x8003, 0x8003,         /* * / % */
    0x8004, 0x8004,                 /* + - */
    0x8005, 0x8005,                 /* << >> */
    0x8008, 0x8009, 0x800a,         /* & ^ | */
    0x8006, 0x8006, 0x8006, 0x8006, /* < <= > >= NOTE: & ^ | have lower precedence */
    0x8007, 0x8007,                 /* == != */
    0x800b, 0x800c,                 /* && || */
    0x980d, 0x980d,                 /* ? : */
    /* '=' & in-place operators */
    0x880e,                         /* = */
    0x880e, 0x880e, 0x880e,         /* *= /= %= */
    0x880e, 0x880e,                 /* += -= */
    0x880e, 0x880e,                 /* <<= >>= */
    0x880e, 0x880e, 0x880e,         /* &= ^= |= */

    0x800f,                         /* , */
};

int op_prefix(int token) {
    switch (token) {
        case L_inc_post: return L_inc_pref;
        case L_dec_post: return L_dec_pref;
        case L_and: return L_addr;
        case L_mul: return L_star;
        case L_add: return L_plus;
        case L_sub: return L_minus;
    }
    return token;
}

expr_node* ParserState::vnode(int type, lexvalue value) {
    expr_node* node = &node_pool[used_nodes++];
    if (used_nodes == MAX_EXPR_NODES) fatal("expression overflow");
    node->type = type;
    node->value = value;
    node->n1 = node->n2 = nullptr;
    return node;
}

expr_node* ParserState::node(int type) {
    expr_node* node = &node_pool[used_nodes++];
    if (used_nodes == MAX_EXPR_NODES) fatal("expression overflow");
    node->type = type;
    if (op_binary(type)) {
        node->n2 = pop_node();
        node->n1 = pop_node();
    }
    else {
        node->n1 = pop_node();
        node->n2 = nullptr;
    }
    return node;
}

expr_node* ParserState::pop_node() {
    if (*qp == nullptr) {
        error("expression fault. too many binary operators (?)");
        return vnode(L_number, {});
    }
    return *qp--;
}

void ParserState::push_operator(int token) {
    while (op_prec(*sp) < op_prec(token) || (op_prec(*sp) == op_prec(token) && !op_right(token))) {
        pop_operator();
    }
    *++sp = token;
}

void ParserState::pop_operator() {
    *++qp = node(*sp--);
}

void ParserState::enqueue_bracket(int token, bool expect_value) {
    if (expect_value && token == L_brace('(')) return;
    switch (token) {
        case L_brace('('): push_operator(L_call); pop_operator(); break;
        case L_brace('['): push_operator(L_index); pop_operator(); break;
        case L_question:   push_operator(L_colon); break;
    }
}

expr_tree ParserState::expr(int end) {
    used_nodes = 0;
    sp = op_stack - 1;
    qp = node_queue - 1;
    _expr(end);
    return expr_tree::create(*qp);
}

expr_tree ParserState::pexpr()
{
    lex.assertToken(L_brace('('));
    expr_tree node = expr(L_brace(')'));
    return node;
}

void ParserState::_expr(int end) {
    *++sp = L_eof;
    *++qp = nullptr;
    int token;
    bool expect_value = true;
    while ((token = lex.token()) != end) {
        if (expect_value) token = op_prefix(token);
        if (op_bad(token)) {
            if (end) error("brace balance corruption");
            lex.enqueue(token);
            break;
        }
        if (op_brace(token)) {
            if (token == L_question) push_operator(L_question);
            _expr(L_closebrace(token));
            enqueue_bracket(token, expect_value);
            expect_value = token == L_question;
        }
        else if (op_value(token)) {
            if (!expect_value) { /* special case, if repeated value, it is ok */
                lex.enqueue(token);
                break;
            }
            *++qp = vnode(token, lex.value);
            expect_value = false;
        }
        else {
            push_operator(token);
            if (op_binary(token))
                expect_value = true;
        }
    }
    while (*sp != L_eof) {
        pop_operator();
    }
    sp--; // erase L_eof
    if (*qp == nullptr) {
        *++qp = vnode(L_empty_expr, {});
    }
    if (*(qp - 1) != nullptr) {
        error("expression fault. too many values (?)");
        while (*(qp - 1) != nullptr) pop_node();
    }
    *(qp - 1) = *qp;
    qp--; // erase nullptr
}
