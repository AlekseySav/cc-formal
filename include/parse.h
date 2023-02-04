#pragma once

class ParserState {
public:
    ParserState(LexerState& lex) : lex(lex) {}
    expr_tree expr(int end = 0);
    expr_tree pexpr(); /* parenthesized expression, pexpr is wierd naming, it's official notation, though */
private:
    void _expr(int end);
    expr_node* vnode(int type, lexvalue value);
    expr_node* node(int type);
    expr_node* pop_node();
    void push_operator(int token);
    void pop_operator();
    void enqueue_bracket(int token, bool expect_value);
private:
    LexerState& lex;
    expr_node node_pool[MAX_EXPR_NODES];
    int op_stack[MAX_EXPR_NODES], * sp;
    expr_node* node_queue[MAX_EXPR_NODES], ** qp;
    int used_nodes;
};
