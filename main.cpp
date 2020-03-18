#include "9cc.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        error("引数の個数が正しくありません\n");
        return 1;
    }

    std::list<Token> tokens = tokenize(argv[1]);
    Node *node = expr(tokens);
    codegen(node);
    return 0;
}
