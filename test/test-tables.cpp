#include <cc.h>
#include <gtest/gtest.h>

TEST(strtab, lookup) {
    StringTable tab;
    ASSERT_EQ(tab.lookup("a"),  0);
    ASSERT_EQ(tab.lookup("a"),  0);
    ASSERT_EQ(tab.lookup("b"),  1);
    ASSERT_EQ(tab.lookup("a"),  0);
    ASSERT_EQ(tab.lookup(""),   2);
    ASSERT_EQ(tab.lookup("b"),  1);
    ASSERT_EQ(tab.lookup("a"),  0);
    ASSERT_EQ(tab.lookup(""),   2);
    ASSERT_EQ(tab.lookup("xx"), 3);
    ASSERT_EQ(tab.lookup("x"),  4);
    ASSERT_EQ(tab.lookup("xx"), 3);
    ASSERT_EQ(tab.lookup("b"),  1);
    ASSERT_EQ(tab.lookup("a"),  0);
    ASSERT_EQ(tab.lookup(""),   2);
}

TEST(symtab, all) {
    SymbolTable tab;
    tab.add("lalaala", S_constant, 13);
    ASSERT_EQ(tab.lookup("lalaala").kind, S_constant);
    ASSERT_EQ(tab.lookup("lalaala").value, 13);
    tab.add(":-)", S_local, 10);
    ASSERT_EQ(tab.lookup(":-)").kind, S_local);
    ASSERT_EQ(tab.lookup(":-)").value, 10);
    tab.deleteLocals();
    ASSERT_EQ(tab.lookup(":-)").kind, 0);
    ASSERT_EQ(tab.lookup(":-)").value, 0);
}
