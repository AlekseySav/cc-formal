#include <cc.h>
#include "lex.h"

static char _ctype[128] = {
    L_bad, L_bad, L_bad, L_bad, L_bad, L_bad, L_bad, L_bad,
    L_bad, L_space, L_newline, L_space, L_bad, L_bad, L_bad, L_bad,
    L_bad, L_bad, L_bad, L_bad, L_bad, L_bad, L_bad, L_bad,
    L_bad, L_bad, L_bad, L_bad, L_bad, L_bad, L_bad, L_bad,
    L_space, L_not, L_quote, L_bad, L_bad, L_mod, L_and, L_quote,
    L_brace('('), L_brace(')'), L_mul, L_add, L_comma, L_sub, L_dot, L_div,
    L_number, L_number, L_number, L_number, L_number, L_number, L_number, L_number,
    L_number, L_number, L_colon, L_semicolon, L_lt, L_assign, L_gt, L_question,
    L_bad, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol,
    L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol,
    L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol,
    L_symbol, L_symbol, L_symbol, L_brace('['), L_bad, L_brace(']'), L_xor, L_symbol,
    L_bad, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol,
    L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol,
    L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol, L_symbol,
    L_symbol, L_symbol, L_symbol, L_brace('{'), L_or, L_brace('}'), L_invert, L_bad
};

static char cdigit[128] = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  16, 16, 16, 16, 16, 16,
    16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

static char ctype(char c, bool eof_is_ok = false) {
    if (c == EOF) {
        if (eof_is_ok) return L_eof;
        fatal("unexpected (eof)");
    }
    return _ctype[c];
}

static bool is_ident(char c) {
    return ctype(c) == L_symbol || ctype(c) == L_number;
}

static char escaped(char c) {
    switch (c) {
        case EOF: fatal("do you want to escape (eof)? what? why..."); break;
        case '0':  return '\0';
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'e':  return '\e';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '"';
    }
    error("bad escape character '\\%c'", c);
    return c;
}

int LexerState::token(bool eof_is_ok) {
    if (!queue.empty()) {
        auto[tok, value] = queue.front();
        queue.pop();
        this->value = value;
        return tok;
    }
    char c;
    int token;
lex:
    c = input.stream.get();
    switch (token = ctype(c, eof_is_ok)) {
        case L_eof: return L_eof;
        case L_bad: error("bad character"); goto lex;
        case L_newline: input.line++;
        case L_space: goto lex;
        case L_quote: return read_quoted(c);
        case L_number: return read_number(c);
        case L_symbol: return read_symbol(c);
        case L_not: return if_sequence('=', L_neq, L_not);
        case L_assign: return if_sequence('=', L_equ, L_assign);
        case L_and: token = if_sequence('&', L_andl, L_and); break;
        case L_or: token = if_sequence('|', L_orl, L_or); break;
        case L_add: token = if_sequence('+', L_inc, L_add); break;
        case L_sub: token = if_sequence('-', L_dec, L_sub); break;
        case L_lt: token = if_sequence('<', L_shl, L_lt); break;
        case L_gt: token = if_sequence('>', L_shr, L_gt); break;
    }
    if (c == '/' && is_comment()) goto lex;
    if (token == L_lt) return if_sequence('=', L_le, L_lt);
    if (token == L_gt) return if_sequence('=', L_ge, L_gt);
    if (token == L_sub && if_sequence('>', 1, 0)) return L_arrow;
    if (L_inplacable(token) && if_sequence('=', 1, 0))
        return L_inplace(token);
    return token;
}

bool LexerState::tryToken(int to)
{
    int t;
    lexvalue prev = value;
    if ((t = token(true)) == to)
        return true;
    enqueue(t);
    value = prev;
    return false;
}

void LexerState::assertToken(int to)
{
    if (!tryToken(to))
        error("expected another token");
}

void LexerState::enqueue(int token, lexvalue* value)
{
    if (!value) value = &this->value;
    queue.emplace(token, *value);
}

int LexerState::read_number(char first) {
    int base = 10;
    char c;
    value.as_number = first - '0';
    if (first == '0') {
        base = 8;
        if (if_sequence('x', 1, 0) || if_sequence('X', 1, 0))
            base = 16;
    }
    while (is_ident(c = input.stream.get())) {
        if (cdigit[c] >= base) error("bad number");
        value.as_number = value.as_number * base + cdigit[c];
    }
    input.stream.unget();
    return L_number;
}

int LexerState::read_quoted(char quote) {
    std::string buffer;
    char c;
    while (ctype(c = input.stream.get()) != L_newline) {
        if (c == quote) {
            value.as_string = buffer;
            value.as_number = buffer[0];
            if (buffer.size() > 1 && quote == '\'')
                error("character constant overflow");
            return quote == '"' ? L_string : L_number;
        }
        if (c == '\\') c = escaped(input.stream.get());
        buffer.push_back(c);
    }
    error("non-terminated string");
    return L_number;
}

int LexerState::read_symbol(char first) {
    std::string buffer(1, first);
    char c;
    while (is_ident(c = input.stream.get()))
        buffer.push_back(c);
    input.stream.unget();
    value.as_symbol = buffer;
    return L_symbol;
}

int LexerState::if_sequence(char guess, int if_true, int if_false) {
    char c = input.stream.get();
    if (c == guess) return if_true;
    input.stream.unget();
    return if_false;
}

bool LexerState::is_comment() {
    char c;
    if (if_sequence('/', 1, 0)) {
        while ((c = input.stream.get()) != EOF && c != '\n');
        input.line += c == '\n';
        return true;
    }
    if (!if_sequence('*', 1, 0)) return false;
    bool read_star = false;

    while ((c = input.stream.get()) != EOF) {
        if (c == '/' && read_star) return true;
        if (c == '\n') input.line++;
        read_star = c == '*';
    }
    fatal("non-terminated comment");
    return true;
}
