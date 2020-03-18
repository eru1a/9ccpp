#include "9cc.h"

void error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(std::list<Token> &tokens, const std::string &op) {
    auto token = tokens.front();
    if (token.kind != TokenKind::TK_RESERVED || token.str != op) {
        return false;
    }
    tokens.pop_front();
    return true;
}

void expect(std::list<Token> &tokens, const std::string &op) {
    auto token = tokens.front();
    if (token.kind != TokenKind::TK_RESERVED || token.str != op)
        error("'%c'ではありません", op);

    tokens.pop_front();
}

int expect_number(std::list<Token> &tokens) {
    auto token = tokens.front();
    if (token.kind != TokenKind::TK_NUM)
        error("数ではありません");
    int val = token.val;
    tokens.pop_front();
    return val;
}

static bool startswith(const std::string &s, int i, const std::string &t) {
    return s.substr(i, t.size()) == t;
}

std::list<Token> tokenize(const std::string &s) {
    std::list<Token> tokens;
    size_t i = 0;
    size_t len = s.size();
    while (i < len) {
        if (isspace(s[i])) {
            i++;
            continue;
        }

        // Multi-letter punctuator
        if (startswith(s, i, "==") || startswith(s, i, "!=") || startswith(s, i, "<=") ||
            startswith(s, i, ">=")) {
            tokens.push_back(Token{.kind = TokenKind::TK_RESERVED, .str = s.substr(i, 2)});
            i += 2;
            continue;
        }

        // Single-letter punctuator
        if (startswith(s, i, "+") || startswith(s, i, "-") || startswith(s, i, "*") ||
            startswith(s, i, "/") || startswith(s, i, "(") || startswith(s, i, ")") ||
            startswith(s, i, "<") || startswith(s, i, ">")) {
            tokens.push_back(Token{.kind = TokenKind::TK_RESERVED, .str = s.substr(i, 1)});
            i++;
            continue;
        }

        if (isdigit(s[i])) {
            size_t j = i;
            int n = 0;
            while (isdigit(s[j])) {
                n = n * 10 + (s[j] - '0');
                j++;
            }
            tokens.push_back(Token{.kind = TokenKind::TK_NUM, .val = n});
            i = j;
            continue;
        }
    }
    tokens.push_back(Token{.kind = TokenKind::TK_EOF});
    return tokens;
}
