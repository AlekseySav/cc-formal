#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

struct file {
    const char* name;
    std::istream& stream;
    int line;

    static file fromStdin() { return file("(stdin)", &std::cin, false); }
    static file fromMemory(const char* buffer) { return file("(memory)", new std::stringstream(buffer), true); }
    static file fromFile(const char* name) { return file(name, new std::fstream(name), true); }
private:
    std::istream* owned; /* stdin is not owned by file, while stream fromMemory or fromFile is owned */

    file(const char* name, std::istream* stream, bool own)
        : name(name), stream(*stream), line(1), owned(own ? stream : nullptr) {}
public:
    file(const file& f) = delete;

    ~file() {
        if (owned) delete owned;
    }
};
