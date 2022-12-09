#include <cc.h>
#include "expr_tree.h"

expr_node* copy_node(expr_node* tree) {
    if (!tree) return nullptr;
    expr_node* n = new expr_node(*tree);
    n->n1 = copy_node(tree->n1);
    n->n2 = copy_node(tree->n2);
    n->type = tree->type;
    n->value = tree->value;
    return n;
}

expr_node* pop_node(expr_node* tree) {
    expr_node* res = tree->n1;
    tree->n1 = nullptr;
    free_node(tree);
    return res;
}

void free_node(expr_node* tree) {
    if (!tree) return;
    free_node(tree->n1);
    free_node(tree->n2);
    delete tree;
}

expr_tree::expr_tree(expr_tree&& node) : root(node.root), size(node.size) {
    node.root = nullptr;
}

expr_tree::expr_tree(int type, lexvalue value)
    : expr_tree(type, value, nullptr, nullptr)
{}

expr_tree::expr_tree(int type, expr_tree&& node)
    : expr_tree(type, {}, node.root, nullptr)
{
    node.root = nullptr;
}

expr_tree::expr_tree(int type, expr_tree&& left, expr_tree&& right)
    : expr_tree(type, {}, left.root, right.root)
{
    left.root = right.root = nullptr;
}

expr_tree::~expr_tree() {
    free_node(root);
    root = nullptr;
}

expr_tree expr_tree::create(expr_node* node) {
    return expr_tree(copy_node(node));
}

expr_tree expr_tree::copy(const expr_tree& tree, int size) {
    expr_tree e = create(tree.root);
    e.size = size;
    return e;
}

int expr_tree::lvalue_size() {
    if (is_empty()) {
        error("using rvalue as lvalue");
        return WORD_SIZE;
    }
    return root->type == L_symbol ? size : WORD_SIZE;
}

int expr_tree::subexpr_count()
{
    if (!root) return 0;
    expr_node* node = root;
    int n = 1;
    while (node && node->type == L_comma) {
        node = node->n1;
        n++;
    }
    return n;
}

void expr_tree::operator=(expr_tree&& node) {
    this->~expr_tree();
    this->root = node.root;
    this->size = node.size;
    node.root = nullptr;
}

expr_tree::expr_tree(expr_node* node) : root(node)
{}

expr_tree::expr_tree(int type, lexvalue value, expr_node* n1, expr_node* n2) : root(new expr_node) {
    root->type = type;
    root->value = value;
    root->n1 = n1;
    root->n2 = n2;
}

std::pair<expr_tree, expr_tree> expr_tree::split() && {
    expr_tree a(root->n1), b(root->n2);
    root->n1 = root->n2 = nullptr;
    a.size = b.size = size;
    return std::make_pair(std::move(a), std::move(b));
}

void expr_tree::pop() {
    free_node(root->n2);
    root = root->n1;
}
