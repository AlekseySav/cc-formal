#include <gtest/gtest.h>
#include <cc.h>

struct test_expr {
    int type;
    lexvalue value;
    test_expr(int t) : type(t) {}
    test_expr(int t, lexvalue v) : type(t), value(v) {}
};

#define num(x) test_expr(L_number, {.as_number = x})
#define str(x) test_expr(L_string, {.as_string = x})
#define sym(x) test_expr(L_symbol, {.as_symbol = x})

std::pair<bool, int> tree_eq(expr_node* a, test_expr* b) {
    if (a->type != b->type) return {false, 0};
    if (a->type == L_number) return {a->value.as_number == b->value.as_number, 1};
    if (a->type == L_symbol) return {a->value.as_symbol == b->value.as_symbol, 1};
    if (a->type == L_string) return {a->value.as_string == b->value.as_string, 1};
    if (a->type == L_empty_expr) return {b->type == L_empty_expr, 1};
    auto[ok1, c1] = tree_eq(a->n1, b + 1);
    if (!ok1) return {false, 0};
    auto[ok2, c2] = a->n2 ? tree_eq(a->n2, b + 1 + c1) : std::make_pair(true, 0);
    if (!ok2) return {false, 0};
    return {true, c1 + c2 + 1};
}

// temporary
std::string strexpr(expr_node* a) {
    if (a->type == L_number) return std::to_string(a->value.as_number);
    if (a->type == L_symbol) return a->value.as_symbol;
    if (a->type == L_string) return a->value.as_string;
    if (a->n2) return "(" + strexpr(a->n1) + "x" + strexpr(a->n2) + ")";
    return "";
}

#define TREE_EQ(a, ...) ({ test_expr x[] = { __VA_ARGS__ }; tree_eq(a.source(), x).first; })

TEST(expr, values) {
    file f = file::fromMemory("1; \"string\"; symbol; ");
    LexerState lex(f);
    ParserState parse(lex);
    ASSERT_TRUE(TREE_EQ(parse.expr(), { num(1) }));
    ASSERT_EQ(lex.token(), L_semicolon);
    ASSERT_TRUE(TREE_EQ(parse.expr(), { str("string") }));
    ASSERT_EQ(lex.token(), L_semicolon);
    ASSERT_TRUE(TREE_EQ(parse.expr(), { sym("symbol") }));
    ASSERT_EQ(lex.token(), L_semicolon);
}

TEST(expr, operators) {
    file f = file::fromMemory("1+2*4/5%++--a, ~!*&x>>y+-2<=1>>3<<2&&\"a\"||0!=1^2+\"b\"&1|2;");
    Logger log(f);
    Logger::defalut_logger = &log;
    LexerState lex(f);
    ParserState parse(lex);
    ASSERT_TRUE(TREE_EQ(parse.expr(),
        
        L_comma,    L_add,  num(1),
                            L_mod,  L_div,  L_mul,  num(2),
                                            num(4),
                                    num(5),
                            L_inc_pref, L_dec_pref, sym("a"),
                    L_orl,  L_andl, L_le,   L_shr,  L_invert, L_not, L_star, L_addr, sym("x"),
                                                    L_add,  sym("y"),
                                                            L_minus, num(2),
                                            L_shl,  L_shr,  num(1),
                                                            num(3),
                                                    num(2),
                                    str("a"),
                            L_or,   L_xor,  L_neq,  num(0),
                                                    num(1),
                                            L_and,  L_add,  num(2),
                                                            str("b"),
                                                    num(1),
                                    num(2),

    ));
}

TEST(expr, inplace) {
    file f = file::fromMemory("1, &a=b/2+=c-=d*=e&&1/=f%=g<<=h>>=i&=j^=k|=2+l;");
    LexerState lex(f);
    ParserState parse(lex);
    ASSERT_TRUE(TREE_EQ(parse.expr(),
        
        L_comma,    num(1),
                    L_assign,   L_addr, sym("a"),
                                L_iadd, L_div,  sym("b"),
                                                num(2),
                                        L_isub, sym("c"),
                                                L_imul, sym("d"),
                                                        L_idiv, L_andl, sym("e"),
                                                                        num(1),
                                                                L_imod, sym("f"),
                                                                        L_ishl, sym("g"),
                                                                                L_ishr, sym("h"),
                                                                                        L_iand, sym("i"),
                                                                                                L_ixor, sym("j"),
                                                                                                        L_ior,  sym("k"),
                                                                                                                L_add,  num(2),
                                                                                                                        sym("l"),  
                    
    ));
}

TEST(expr, pexpr) {
    file f = file::fromMemory("(!1)");
    LexerState lex(f);
    ParserState parse(lex);
    ASSERT_TRUE(TREE_EQ(parse.pexpr(), L_not, num(1)));
    ASSERT_EQ(lex.token(true), L_eof);
}

TEST(expr, braces) {
    file f = file::fromMemory("1+(2&&3)*a(1, b()+a()()[]((*(*qq)())[1]));");
    LexerState lex(f);
    ParserState parse(lex);
    ASSERT_TRUE(TREE_EQ(parse.expr(),
        L_add,  num(1),
                L_mul,  L_andl, num(2),
                                num(3),
                        L_call, sym("a"),
                                L_comma,    num(1),
                                            L_add,  L_call, sym("b"),
                                                            L_empty_expr,
                                                    L_call, L_index,    L_call, L_call, sym("a"),
                                                                                        L_empty_expr,
                                                                                L_empty_expr,
                                                                        L_empty_expr,
                                                            L_index,    L_star, L_call, L_star, sym("qq"),
                                                                                        L_empty_expr,
                                                                        num(1)
    ));
    ASSERT_EQ(lex.token(true), L_semicolon);
}

TEST(expr, ternary) {
    file f = file::fromMemory("c = 1 ? 2 : 3 ? 4 : 5, c==1?one:c==2?two:c!=3?:three;");
    LexerState lex(f);
    ParserState parse(lex);
    ASSERT_TRUE(TREE_EQ(parse.expr(),
        L_comma,    L_assign,   sym("c"),
                                L_question, num(1),
                                L_colon,    num(2),
                                            L_question, num(3),
                                            L_colon,    num(4),
                                                        num(5),
                    L_question, L_equ, sym("c"), num(1),
                    L_colon,    sym("one"),
                                L_question, L_equ, sym("c"), num(2),
                                L_colon,    sym("two"),
                                            L_question, L_neq, sym("c"), num(3),
                                            L_colon,    L_empty_expr,
                                                        sym("three")
                                                                                
    ));
    ASSERT_EQ(lex.token(true), L_semicolon);
}
