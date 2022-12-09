#include <cc.h>
#include "program.h"

Program::Program(file& input, std::ostream& output)
    : locals_offset(0), n_labels(0), input(input), output(output), lex(this->input), parse(lex)
{
    symbol_table.add("asm", S_keyword, K_asm);
    symbol_table.add("int", S_keyword, K_int);
    symbol_table.add("char", S_keyword, K_char);
    symbol_table.add("return", S_keyword, K_return);
    symbol_table.add("extern", S_keyword, K_extern);
    symbol_table.add("if", S_keyword, K_if);
    symbol_table.add("else", S_keyword, K_else);
    symbol_table.add("while", S_keyword, K_while);
    symbol_table.add("break", S_keyword, K_break);
    symbol_table.add("continue", S_keyword, K_continue);
}

Program::~Program() {
    for (auto&[string, number] : string_table.table) {
        output << 'S' << number << ": db ";
        for (char c : string)
            output << (int)c << ',';
        output << "0\n";
    }
}

void Program::define() {
    expr_tree tree = parse.expr();
    switch (tree.expr_type()) {
        case L_call: { // function
            locals_offset = 0;
            auto[n1, n2] = std::move(tree).split();
            if (n1.expr_type() != L_symbol) {
                error("bad function name");
                return;
            }
            int label = n_labels++;
            output << '_' << n1.value().as_symbol << ':' << '\n';
            output << '#' << L_put_label << ' ' << 0 << ' ' << label << '\n';
            output << 'L' << label << ':' << '\n';
            output << '#' << L_enter_function << '\n';
            enter_function(std::move(n2));
            statement();
            output << '#' << L_leave_function << '\n';
            return;
        }
        case L_symbol:
            output << '_' << tree.value().as_symbol << ": dw 0\n";
            lex.assertToken(';');
            return;
        default:
            error("bad definition");
            return;
    }
}

void Program::statement(int label) {
    int token = lex.token();
    if (token == L_semicolon) return;
    if (token == L_brace('{')) return statements(label);
    if (token != L_symbol || symbol_table.lookup(lex.value.as_symbol).kind != S_keyword) {
        lex.enqueue(token);
        put_expr(parse.expr());
        lex.assertToken(L_semicolon);
        return;
    }
    int key = symbol_table.lookup(lex.value.as_symbol).value;
    switch (key) {
        case K_asm:
            put_asm();
            break;
        case K_int: case K_char: {
            int adjust = declare(parse.expr(), key == K_int ? WORD_SIZE : 1, S_local, true);
            if (adjust)
                output << '#' << L_adjust_stack << ' ' << 0 << ' ' << adjust << '\n';
            lex.assertToken(L_semicolon);
            statement(label);
            break;
        }
        case K_return:
            if (!lex.tryToken(L_semicolon)) {
                put_expr(parse.expr());
                lex.assertToken(L_semicolon);
            }
            output << '#' << L_leave_function << '\n';
            break;
        case K_extern: {
            int size = read_primary_type();
            declare(parse.expr(), size, S_global, true);
            break;
        }
        case K_if: {
            int endif = n_labels++;
            put_expr(parse.pexpr());
            output << '#' << L_conditional_jump << ' ' << 0 << ' ' << endif << '\n';
            statement(label);
            if (lex.tryToken(L_symbol)) {
                unnamed_symbol* sym = &symbol_table.lookup(lex.value.as_symbol);
                if (sym->kind != S_keyword || sym->value != K_else) {
                    lex.enqueue(L_symbol);
                    output << 'L' << endif << ':' << '\n';
                    return;
                }
                int end_else = n_labels++;
                output << '#' << L_jump << " 0 " << end_else << '\n';
                output << 'L' << endif << ':' << '\n';
                statement(label);
                output << 'L' << end_else << ':' << '\n';
            }
            else {
                output << 'L' << endif << ':' << '\n';
            }
            break;
        }
        case K_while: {
            int begin = n_labels++;
            int end = n_labels++;
            output << 'L' << begin << ':' << '\n';
            put_expr(parse.pexpr());
            output << '#' << L_conditional_jump << ' ' << 0 << ' ' << end << '\n';
            statement(begin);
            output << '#' << L_jump << ' ' << 0 << ' ' << begin << '\n';
            output << 'L' << end << ':' << '\n';
            break;
        }
        case K_break: case K_continue:
            output << '#' << L_jump << " 0 " << label + (key == K_break) << '\n';
            break;
    }
}

void Program::put_asm() {
    lex.tryToken(L_brace('('));
    lex.assertToken(L_string);
    output << lex.value.as_string << '\n';
    lex.tryToken(L_brace(')'));
    lex.assertToken(L_semicolon);
}

int Program::pointer_size(expr_tree& expr) {
    if (expr.expr_type() != L_star) {
        return 1;
    }
    if (expr.source()->n1->type != L_symbol) {
        return expr.lvalue_size();
    }
    return symbol_table.lookup(expr.source()->n1->value.as_symbol).size;
}

int Program::read_primary_type() {
    lex.assertToken(L_symbol);
    unnamed_symbol* sym = &symbol_table.lookup(lex.value.as_symbol);
    if (sym->kind != S_keyword) {
        error("bad type declaration");
        return WORD_SIZE;
    }
    switch (sym->value) {
        case K_char: return 1;
        case K_int: return 2;
    }
    error("bad type declaration");
    return WORD_SIZE;
}

void Program::statements(int label) {
    while (!lex.tryToken(L_brace('}')))
        statement(label);
}

int Program::declare(expr_tree expr, int size, symbol_type kind, bool local) {
    int capacity = WORD_SIZE;
    int count = 0, ptr_level = 0;
    if (expr.expr_type() == L_comma) {
        auto[n1, n2] = std::move(expr).split();
        count += declare(std::move(n1), size, kind, local);
        expr = std::move(n2);
    }
    expr_tree sym_type = expr_tree::copy(expr);
    while (expr.expr_type() != L_symbol) {
        if (expr.expr_type() != L_star && expr.expr_type() != L_index && expr.expr_type() != L_call) {
            error("bad declaration");
            return count;
        }
        auto[n1, n2] = std::move(expr).split();
        expr = std::move(n1);
    }
    unnamed_symbol* sym = &symbol_table.lookup(expr.value().as_symbol);
    if (sym->kind == S_none) { /* new variable */
        if (local && kind == S_global) { /* 'extern' declaration -- manualy set local */
            symbol_table.locals.push_back(expr.value().as_symbol);
        }
        else if (local) { /* local variable -- adjust stack */
            locals_offset -= capacity;
            count += capacity;
        }
        symbol_table.add(expr.value().as_symbol, kind, locals_offset, size, std::move(sym_type));
    }
    else { /* specifying type of argument */
        if (sym->kind != S_local) error("trying to override existing variable");
        sym->type = std::move(sym_type);
        sym->size = size;
    }
    return count;
}

void Program::enter_function(expr_tree args) {
    int arg_offset = WORD_SIZE * (args.subexpr_count() + 2); /* +2 because bp+0 is bp, bp+WORD_SIZE is return arrdess */
    if (args.expr_type() == L_empty_expr)
        return;
    while (args.expr_type() == L_comma) {
        auto[n1, n2] = std::move(args).split();
        arg_offset -= WORD_SIZE;
        symbol_table.add(n2.value().as_symbol, S_local, arg_offset, WORD_SIZE);
        args = std::move(n1);
    }
    arg_offset -= WORD_SIZE;
    symbol_table.add(args.value().as_symbol, S_local, arg_offset, WORD_SIZE);
}

int Program::push_args(expr_tree args)
{
    if (args.expr_type() == L_empty_expr)
        return 0;
    int n_args = 1;
    while (args.expr_type() == L_comma) {
        auto[n1, n2] = std::move(args).split();
        put_expr(std::move(n2));
        output << '#' << L_push << '\n';
        args = std::move(n1);
        n_args++;
    }
    put_expr(std::move(args));
    output << '#' << L_push << '\n';
    return n_args;
}

/*
 * convert syntax tree to an intermediate code
 */
expr_tree Program::put_expr(expr_tree expr) {
    expr_tree type;
    int expr_operator;
    switch (expr_operator = expr.expr_type()) {
        /* values */
        case L_symbol: {
            unnamed_symbol* sym = &symbol_table.lookup(expr.value().as_symbol);
            if (sym->kind != S_global && sym->kind != S_local) {
                error("bad symbol in expression");
                return {};
            }
            output << '#';
            type = expr_tree::copy(sym->type, sym->size);
            if (type.expr_type() == L_index) { // put array by it's address, not the value itself
                output << (sym->kind == S_global ? L_put_global_reference : L_put_local_reference);
                type.expr_type() = L_star;
            }
            else {
                output << (sym->kind == S_global ? L_put_global : L_put_local);
            }
            output << ' ' << type.lvalue_size() << ' ';
            if (sym->kind == S_global) {
                output << expr.value().as_symbol << '\n';
            }
            else {
                output << sym->value << '\n';
            }
            return type;
        }
        case L_number:
            output << '#' << L_put_number << ' ' << WORD_SIZE << ' ' << expr.value().as_number << '\n';
            return expr_tree();
        case L_string:
            output << '#' << L_put_string << ' ' << WORD_SIZE << ' '  << string_table.lookup(expr.value().as_string) << '\n';
            return expr_tree();
        /* special unary operators */
        case L_addr:
            return put_lvalue(std::move(expr).split().first);
        case L_star:
            type = put_expr(std::move(expr).split().first);
            if (type.is_empty() || type.expr_type() != L_star) {
                error("trying to dereference non-pointer variable");
                return {};
            }
            type.pop(); // delete star
            output << '#' << L_star << ' ' << type.lvalue_size() << '\n';
            return type;
        /* increment/decrement */
        case L_inc_post: case L_dec_post: case L_inc_pref: case L_dec_pref:
            type = put_lvalue(std::move(expr).split().first);
            output << '#' << expr_operator << ' ' << type.lvalue_size() << ' ';
            output << pointer_size(type) << '\n';
            return type;
        /* common unary operators */
        case L_plus: case L_minus: case L_not: case L_invert:
            type = put_expr(std::move(expr).split().first);
            output << '#' << expr_operator << '\n';
            return type;
        /* member access */
        case L_dot: case L_arrow: {
            auto[n1, n2] = std::move(expr).split();
            if (n2.expr_type() != L_symbol) {
                error("bad token for member acces");
                return {};
            }
            type = put_expr(std::move(n1));
            output << '#' << expr_operator << ' ' << type.lvalue_size() << ' ' << n2.value().as_symbol << '\n';
            return type;
        }
        /* index */
        case L_index: {
            auto[n1, n2] = std::move(expr).split();
            type = put_expr(std::move(n1));
            if (type.is_empty() || type.expr_type() != L_star) {
                error("trying to index non-pointer type");
                return type;
            }
            output << '#' << L_push << '\n';
            put_expr(std::move(n2));
            type.pop();
            output << '#' << L_index << ' ' << type.lvalue_size() << '\n';
            return type;
        }
        /* call */
        case L_call: {
            auto[n1, n2] = std::move(expr).split();
            int n_args = push_args(std::move(n2));
            type = put_expr(std::move(n1));
            if (type.is_empty() || type.expr_type() != L_call) {
                error("trying to call not a function and not a pointer-to-a-function");
                return type;
            }
            type.pop();
            if (!type.is_empty() && type.expr_type() == L_star) {
                type.pop();
            }
            output << '#' << L_call << ' ' << 0 << ' ' << n_args * WORD_SIZE << '\n';
            return type;
        }
        /* assign & in-place operators */
        case L_assign: case L_imul: case L_idiv: case L_imod:
        case L_iadd: case L_isub: case L_ishl: case L_ishr:
        case L_iand: case L_ixor: case L_ior: {
            auto[n1, n2] = std::move(expr).split();
            type = put_lvalue(std::move(n1));
            output << '#' << L_push << '\n';
            put_expr(std::move(n2));
            output << '#' << expr_operator << ' ' << type.lvalue_size() << '\n';
            return type;
        }
        /* common binary operator */
        default: {
            auto[n1, n2] = std::move(expr).split();
            type = put_expr(std::move(n1));
            output << '#' << L_push << '\n';
            put_expr(std::move(n2));
            output << '#' << expr_operator << '\n';
            return type;
        }
    }
}

expr_tree Program::put_lvalue(expr_tree expr) {
    expr_tree type;
    switch (expr.expr_type()) {
        case L_star:
            type = put_expr(std::move(expr).split().first);
            if (type.is_empty() || type.expr_type() != L_star) {
                error("trying to dereference non-pointer variabled");
                return type;
            }
            type.pop();
            return type;
        case L_index: {
            auto[n1, n2] = std::move(expr).split();
            type = put_expr(std::move(n1));
            if (type.is_empty() || type.expr_type() != L_star) {
                error("trying to dereference non-pointer variabled");
                return type;
            }
            output << '#' << L_push << '\n';
            put_expr(std::move(n2));
            output << '#' << L_index_reference << ' ' << pointer_size(type) << '\n';
            type.pop();
            return type;
        }
        case L_symbol: {
            unnamed_symbol* sym = &symbol_table.lookup(expr.value().as_symbol);
            if (sym->kind != S_global && sym->kind != S_local) {
                error("bad symbol in expression");
                return {};
            }
            type = expr_tree::copy(sym->type, sym->size);
            output << '#' << (sym->kind == S_global ? L_put_global_reference : L_put_local_reference) << ' ' << WORD_SIZE << ' ';
            if (sym->kind == S_global) {
                output << expr.value().as_symbol << '\n';
            }
            else {
                output << sym->value << '\n';
            }
            if (type.expr_type() == L_index) type.expr_type() = L_star;
            return type;
        }
        default:
            error("bad lvalue");
            return {};
    }
    return {};
}
