#pragma once

#include <tuple>

struct expr_node {
    int type;
    lexvalue value;
    expr_node* n1;
    expr_node* n2;
};

expr_node* copy_node(expr_node* tree);
expr_node* pop_node(expr_node* tree); // pop root and return n1
void free_node(expr_node* tree);

// syntax tree for expressions

struct expr_tree {
public:
    expr_tree() : root(nullptr) {}
    expr_tree(expr_tree&& node);
    expr_tree(int type, lexvalue value);
    expr_tree(int type, expr_tree&& node);
    expr_tree(int type, expr_tree&& left, expr_tree&& right);
    std::pair<expr_tree, expr_tree> split() &&;
    void pop();
    ~expr_tree();

    static expr_tree create(expr_node* node); // used only by expr
    static expr_tree copy(const expr_tree& tree, int size = 2); // AVOID IMPLICIT COPIES

    int lvalue_size();
    int subexpr_count(); // number of sub-expressions, separated by comma

    inline bool is_empty() { return root == nullptr; }
    inline expr_node* source() { return root; } // for testing
    inline lexvalue value() { return root->value; }
    inline int& expr_type() { return root->type; }
    void operator=(expr_tree&& node);
private:
    expr_tree(expr_node* node);
    expr_tree(int type, lexvalue value, expr_node* n1, expr_node* n2);
    expr_node* root;
    int size;
};
