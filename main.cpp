#include "9cc.h"

int main(int argc, const char *argv[]) {
    // デバッグ用にtokensを表示する
    if (argc == 3 && std::string{argv[1]} == "p") {
        std::list<Token> tokens = tokenize(argv[2]);
        std::for_each(tokens.begin(), tokens.end(),
                      [](const Token &t) -> void { std::cout << t.to_string() << std::endl; });
        return 0;
    }

    if (argc != 2) {
        error("引数の個数が正しくありません\n");
        return 1;
    }

    std::list<Token> tokens = tokenize(argv[1]);
    auto prog = program(tokens);
    codegen(prog);
    return 0;
}
