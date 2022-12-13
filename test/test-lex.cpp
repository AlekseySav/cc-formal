#include <gtest/gtest.h>
#include <cc.h>

TEST(lexer, operators) {
    file f = file::fromMemory("++--()[].->++--+-!~*&*/%+-<<>><=<>=> ==!=&^|&&||?:=+=-=*=/=%=<<=>>=&=^=|=,{};");
    LexerState lex(f);
    ASSERT_EQ(lex.token(), L_inc);
    ASSERT_EQ(lex.token(), L_dec);
    ASSERT_EQ(lex.token(), L_brace('('));
    ASSERT_EQ(lex.token(), L_brace(')'));
    ASSERT_EQ(lex.token(), L_brace('['));
    ASSERT_EQ(lex.token(), L_brace(']'));
    ASSERT_EQ(lex.token(), L_dot);
    ASSERT_EQ(lex.token(), L_arrow);
    ASSERT_EQ(lex.token(), L_inc);
    ASSERT_EQ(lex.token(), L_dec);
    ASSERT_EQ(lex.token(), L_add);
    ASSERT_EQ(lex.token(), L_sub);
    ASSERT_EQ(lex.token(), L_not);
    ASSERT_EQ(lex.token(), L_invert);
    ASSERT_EQ(lex.token(), L_mul);
    ASSERT_EQ(lex.token(), L_and);
    ASSERT_EQ(lex.token(), L_mul);
    ASSERT_EQ(lex.token(), L_div);
    ASSERT_EQ(lex.token(), L_mod);
    ASSERT_EQ(lex.token(), L_add);
    ASSERT_EQ(lex.token(), L_sub);
    ASSERT_EQ(lex.token(), L_shl);
    ASSERT_EQ(lex.token(), L_shr);
    ASSERT_EQ(lex.token(), L_le);
    ASSERT_EQ(lex.token(), L_lt);
    ASSERT_EQ(lex.token(), L_ge);
    ASSERT_EQ(lex.token(), L_gt);
    ASSERT_EQ(lex.token(), L_equ);
    ASSERT_EQ(lex.token(), L_neq);
    ASSERT_EQ(lex.token(), L_and);
    ASSERT_EQ(lex.token(), L_xor);
    ASSERT_EQ(lex.token(), L_or);
    ASSERT_EQ(lex.token(), L_andl);
    ASSERT_EQ(lex.token(), L_orl);
    ASSERT_EQ(lex.token(), L_question);
    ASSERT_EQ(lex.token(), L_colon);
    ASSERT_EQ(lex.token(), L_assign);
    ASSERT_EQ(lex.token(), L_iadd);
    ASSERT_EQ(lex.token(), L_isub);
    ASSERT_EQ(lex.token(), L_imul);
    ASSERT_EQ(lex.token(), L_idiv);
    ASSERT_EQ(lex.token(), L_imod);
    ASSERT_EQ(lex.token(), L_ishl);
    ASSERT_EQ(lex.token(), L_ishr);
    ASSERT_EQ(lex.token(), L_iand);
    ASSERT_EQ(lex.token(), L_ixor);
    ASSERT_EQ(lex.token(), L_ior);
    ASSERT_EQ(lex.token(), L_comma);
    ASSERT_EQ(lex.token(), L_brace('{'));
    ASSERT_EQ(lex.token(), L_brace('}'));
    ASSERT_EQ(lex.token(), L_semicolon);
    ASSERT_EQ(lex.token(true), L_eof);
}

TEST(lexer, symbols) {
    file f = file::fromMemory("\"\"\"123\"\"\\n\\n\\n\"'\\n'123 0x123 0123 0 1'\\0''0'abc a_b _a 0x1a a01 a0 0;");
    LexerState lex(f);
    ASSERT_EQ(lex.token(), L_string);
    ASSERT_EQ(lex.value.as_string, "");
    ASSERT_EQ(lex.token(), L_string);
    ASSERT_EQ(lex.value.as_string, "123");
    ASSERT_EQ(lex.token(), L_string);
    ASSERT_EQ(lex.value.as_string, "\n\n\n");
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, '\n');
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 123);
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 0x123);
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 0123);
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 0);
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 1);
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, '\0');
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, '0');
    ASSERT_EQ(lex.token(), L_symbol);
    ASSERT_EQ(lex.value.as_symbol, "abc");
    ASSERT_EQ(lex.token(), L_symbol);
    ASSERT_EQ(lex.value.as_symbol, "a_b");
    ASSERT_EQ(lex.token(), L_symbol);
    ASSERT_EQ(lex.value.as_symbol, "_a");
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 0x1a);
    ASSERT_EQ(lex.token(), L_symbol);
    ASSERT_EQ(lex.value.as_symbol, "a01");
    ASSERT_EQ(lex.token(), L_symbol);
    ASSERT_EQ(lex.value.as_symbol, "a0");
    ASSERT_EQ(lex.token(), L_number);
    ASSERT_EQ(lex.value.as_number, 0);
    ASSERT_EQ(lex.token(), L_semicolon);
    ASSERT_EQ(lex.token(true), L_eof);
}

TEST(lexer, escape) {
    const char* a = "\0\a\b\e\n\r\t\\\'\"";
    const char* b = "\"\\0\\a\\b\\e\\n\\r\\t\\\\\\'\\\"\"";
    file f = file::fromMemory(b);
    LexerState lex(f);
    ASSERT_EQ(lex.token(), L_string);
    ASSERT_STREQ(lex.value.as_string.c_str(), a);
    ASSERT_EQ(lex.token(true), L_eof);
}

TEST(lexer, lines) {
    file f = file::fromMemory("a/*lalala\n*/\n2;\n//\n/*\n\n*///");
    LexerState lex(f);
    ASSERT_EQ(lex.token(), L_symbol); ASSERT_EQ(f.line, 1);
    ASSERT_EQ(lex.token(), L_number); ASSERT_EQ(f.line, 3);
    ASSERT_EQ(lex.token(), L_semicolon); ASSERT_EQ(f.line, 3);
    ASSERT_EQ(lex.token(true), L_eof); ASSERT_EQ(f.line, 7);
}

TEST(lexer, queue) {
    file f = file::fromMemory(".;");
    LexerState lex(f);
    ASSERT_EQ(lex.token(), L_dot);
    lex.enqueue(L_add);
    lex.enqueue(L_sub);
    ASSERT_EQ(lex.token(), L_add);
    ASSERT_EQ(lex.token(), L_sub);
    ASSERT_EQ(lex.token(), L_semicolon);
    ASSERT_EQ(lex.token(true), L_eof);
}

TEST(lexer, trytok) {
    file f = file::fromMemory("++--");
    LexerState lex(f);
    ASSERT_TRUE(lex.tryToken(L_inc));
    ASSERT_TRUE(lex.tryToken(L_dec));
    ASSERT_TRUE(lex.tryToken(L_eof));
    file f2 = file::fromMemory("++--");
    LexerState lex2(f2);
    ASSERT_FALSE(lex2.tryToken(L_dec));
    ASSERT_FALSE(lex2.tryToken(L_dec));
    ASSERT_FALSE(lex2.tryToken('a'));
    ASSERT_FALSE(lex2.tryToken(L_eof));
    ASSERT_TRUE(lex2.tryToken(L_inc));
    ASSERT_FALSE(lex2.tryToken(L_inc));
    ASSERT_TRUE(lex2.tryToken(L_dec));
    ASSERT_FALSE(lex2.tryToken('a'));
    ASSERT_TRUE(lex2.tryToken(L_eof));
}

TEST(lexer, assert) {
    file f = file::fromMemory("++--a/a.");
    LexerState lex(f);
    lex.assertToken(L_inc);
    lex.assertToken(L_dec);
    lex.assertToken(L_symbol);
    lex.assertToken(L_div);
    lex.assertToken(L_symbol);
    lex.assertToken(L_dot);
    lex.assertToken(L_eof);
}
