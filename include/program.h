#pragma once

class Program {
public:
    Program(file& input, std::ostream& output);
    ~Program();
    void extractDefinition();
    void statement(int label = 0);
    inline bool eof() { return lex.tryToken(L_eof); }
private:
    void put_asm();
    int pointer_size(expr_tree& expr);
    int read_primary_type(); // char, int, struct <name>, returns size
    void statements(int label = 0);
    int declare_variable(expr_tree expr, int size, symbol_type kind, bool local);
    void enter_function(expr_tree args);
    int push_args(expr_tree args);
    expr_tree put_expr(expr_tree expr);
    expr_tree put_lvalue(expr_tree expr);
private:
    int locals_offset;
    int n_labels;
    file& input;
    std::ostream& output;
    LexerState lex;
    ParserState parse;
public:
    StringTable string_table;
    SymbolTable symbol_table;
};
