#pragma once

enum symbol_type {
    S_none,
    S_keyword,        /* special */
    S_constant,       /* $value */
    S_string,         /* S<value> */
    S_global,         /* (name) */
    S_local,          /* value(bp) */
};

enum keyword {
    K_asm,
    K_int,
    K_char,
    K_return,
    K_extern,
    K_if,
    K_else,
    K_while,
    K_break,
    K_continue,
};

struct unnamed_symbol {
    symbol_type kind;
    int value;
    int size;
    expr_tree type;
};

struct SymbolTable {
    std::unordered_map<std::string, unnamed_symbol> table;
    std::vector<std::string> locals;

    void add(const std::string& name, symbol_type kind, int value, int size = 0, expr_tree type = {});

    void deleteLocals();

    unnamed_symbol& lookup(const std::string& name);
};

struct StringTable {
    std::unordered_map<std::string, int> table;
    int n_strings;

    int lookup(const std::string& string);
};
