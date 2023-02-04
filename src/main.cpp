#include <cc.h>

int main() {
    file input = file::fromStdin();
    Logger logger(input);
    Logger::defalut_logger = &logger;
    Program program(input, std::cout);
    while (!program.eof()) {
        program.extractDefinition();
        program.symbol_table.deleteLocals();
    }
}
