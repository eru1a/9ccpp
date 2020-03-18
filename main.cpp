#include "9cc.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        error("引数の個数が正しくありません\n");
        return 1;
    }

    auto print_token = [](Token t) {
        switch (t.kind) {
        case TokenKind::TK_RESERVED:
            std::cout << "RESERVED: " << t.str << std::endl;
            break;
        case TokenKind::TK_IDENT:
            std::cout << "IDENT: " << t.str << std::endl;
            break;
        case TokenKind::TK_NUM:
            std::cout << "NUM: " << t.val << std::endl;
            break;
        default:
            break;
        }
    };

    std::list<Token> tokens = tokenize(argv[1]);
    // std::for_each(tokens.begin(), tokens.end(), print_token);
    auto prog = program(tokens);
    codegen(prog);
    return 0;
}
