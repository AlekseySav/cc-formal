#include <cc.h>
#include <gtest/gtest.h>
#include <sstream>
#include <stdarg.h>

#define run(source, res) { \
    file f = file::fromMemory(source); \
    std::stringstream ss; \
    Program p(f, ss); \
    p.statement(); \
    ASSERT_STREQ(ss.str().c_str(), res); \
}

TEST(statement, asm) {
    run("asm \"\\tmov %ax, %bx\";", "\tmov %ax, %bx\n");
}

int read_number(std::istream& is) {
    int n;
    is >> n;
    return n;
}

std::string format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[2000];
    vsprintf(buf, fmt, ap);
    va_end(ap);
    return buf;
}

std::string getline(std::istream& is) {
    std::string s;
    std::getline(is, s);
    return s;
}

TEST(statement, auto) {
    file f = file::fromMemory("char **c, *f(); int x[]; 1; ");
                                                    /*  ^ need at least one another statement */
    std::stringstream ss;
    Program p(f, ss);
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_adjust_stack, 0, 4));

    ASSERT_EQ(getline(ss), format("#%d %d %d", L_adjust_stack, 0, 2));
    ASSERT_EQ(p.symbol_table.lookup("c").kind, S_local);
    ASSERT_EQ(p.symbol_table.lookup("c").value, -WORD_SIZE);
    ASSERT_EQ(p.symbol_table.lookup("c").size, 1);
    ASSERT_EQ(p.symbol_table.lookup("f").kind, S_local);
    ASSERT_EQ(p.symbol_table.lookup("f").size, 1);
    ASSERT_EQ(p.symbol_table.lookup("f").value, -2 * WORD_SIZE);
    ASSERT_EQ(p.symbol_table.lookup("x").kind, S_local);
    ASSERT_EQ(p.symbol_table.lookup("x").size, WORD_SIZE);
    ASSERT_EQ(p.symbol_table.lookup("x").value, -3 * WORD_SIZE);

    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
}

TEST(statement, expr) {
    file f = file::fromMemory(
"char *ptr, c, (*f)();"
"c+=*ptr++; ptr+=ptr*=++c/2*c;"
"{ f(); f(1); f(2,3); }"
"{ f(ptr[f(f(), 1)*&c+&*ptr]); }"
"ptr[*ptr];"
"&ptr[*ptr];"
);
    std::stringstream ss;
    Program p(f, ss);
    /* variables */
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_adjust_stack, 0, 6));
    /* c=*ptr++ */
    // p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local_reference, WORD_SIZE, p.symbol_table.lookup("c").value));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local_reference, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_inc_post, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d %d", L_star, 1));
    ASSERT_EQ(getline(ss), format("#%d %d", L_iadd, 1));
    /* ptr+=ptr*=c++/2; */
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local_reference, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local_reference, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local_reference, WORD_SIZE, p.symbol_table.lookup("c").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_inc_pref, 1, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 2));
    ASSERT_EQ(getline(ss), format("#%d", L_div));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, 1, p.symbol_table.lookup("c").value));
    ASSERT_EQ(getline(ss), format("#%d", L_mul));
    ASSERT_EQ(getline(ss), format("#%d %d", L_imul, 2));
    ASSERT_EQ(getline(ss), format("#%d %d", L_iadd, 2));
    /* { f(); f(1); f(2,3); } */
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("f").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_call, 0, 0));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("f").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_call, 0, WORD_SIZE));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 3));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 2));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("f").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_call, 0, 2 * WORD_SIZE));
    /* { f(ptr[f(f(), 1)*&c+&*ptr]); } */
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("f").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_call, 0, 0));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("f").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_call, 0, 2 * WORD_SIZE));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local_reference, WORD_SIZE, p.symbol_table.lookup("c").value));
    ASSERT_EQ(getline(ss), format("#%d", L_mul));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d", L_add));
    ASSERT_EQ(getline(ss), format("#%d %d", L_index, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("f").value));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_call, 0, WORD_SIZE));
    //ptr[*ptr]
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d %d", L_star, 1));
    ASSERT_EQ(getline(ss), format("#%d %d", L_index, 1));
    //&ptr[*ptr]
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_local, WORD_SIZE, p.symbol_table.lookup("ptr").value));
    ASSERT_EQ(getline(ss), format("#%d %d", L_star, 1));
    ASSERT_EQ(getline(ss), format("#%d %d", L_index_reference, 1));
}

TEST(statement, if) {
    file f = file::fromMemory(
"if (1 == 0);"
"if (1 == 0) 2; else 1;"
);
    std::stringstream ss;
    Program p(f, ss);
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 0));
    ASSERT_EQ(getline(ss), format("#%d", L_equ));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_conditional_jump, 0, "0"));
    ASSERT_EQ(getline(ss), "L0:");
    p.statement();
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 0));
    ASSERT_EQ(getline(ss), format("#%d", L_equ));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_conditional_jump, 0, "1"));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 2));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_jump, 0, "2"));
    ASSERT_EQ(getline(ss), "L1:");
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), "L2:");
}

TEST(statement, while) {
    file f = file::fromMemory(
"while (1 == 0);"
"while (1 == 0) 2;"
"while (1 == 0) { continue; break; }"
);
    std::stringstream ss;
    Program p(f, ss);
    p.statement();
    ASSERT_EQ(getline(ss), "L0:");
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 0));
    ASSERT_EQ(getline(ss), format("#%d", L_equ));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_conditional_jump, 0, "1"));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_jump, 0, "0"));
    ASSERT_EQ(getline(ss), "L1:");
    p.statement();
    ASSERT_EQ(getline(ss), "L2:");
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 0));
    ASSERT_EQ(getline(ss), format("#%d", L_equ));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_conditional_jump, 0, "3"));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 2));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_jump, 0, "2"));
    ASSERT_EQ(getline(ss), "L3:");
    p.statement();
    ASSERT_EQ(getline(ss), "L4:");
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 1));
    ASSERT_EQ(getline(ss), format("#%d", L_push));
    ASSERT_EQ(getline(ss), format("#%d %d %d", L_put_number, WORD_SIZE, 0));
    ASSERT_EQ(getline(ss), format("#%d", L_equ));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_conditional_jump, 0, "5"));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_jump, 0, "4"));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_jump, 0, "5"));
    ASSERT_EQ(getline(ss), format("#%d %d %s", L_jump, 0, "4"));
    ASSERT_EQ(getline(ss), "L5:");
}
