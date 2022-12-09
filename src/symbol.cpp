#include <cc.h>

void SymbolTable::add(const std::string& name, symbol_type kind, int value, int size, expr_tree type) {
    if (table[name].kind != S_none) {
        error("trying to redefine '%s'", name.c_str());
        return;
    }
    table[name] = std::move(unnamed_symbol{kind, value, size, std::move(type)});
    if (kind == S_local)
        locals.push_back(name);
}

void SymbolTable::deleteLocals() {
    for (auto& name : locals)
        table.erase(name);
}

unnamed_symbol& SymbolTable::lookup(const std::string& name) {
    return table[name];
}

int StringTable::lookup(const std::string& string) {
    return table.contains(string) ? table[string] : (table[string] = n_strings++);
}
