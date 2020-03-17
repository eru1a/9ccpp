#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <list>
#include <sstream>
#include <string>

// エラーを報告するための関数
// printfと同じ引数を取る
void error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

enum class TokenKind {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
};

struct Token {
    TokenKind kind;  // トークンの型
    int val;         // kindがTK_NUMの場合、その数値
    std::string str; // トークン文字列
};

bool startswith(const std::string &s, int i, const std::string &t) {
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

// 抽象構文木のノードの種類
enum class NodeKind {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // 整数
};

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_NUM;
    node->val = val;
    return node;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(std::list<Token> &tokens, const std::string &op) {
    auto token = tokens.front();
    if (token.kind != TokenKind::TK_RESERVED || token.str != op) {
        return false;
    }
    tokens.pop_front();
    return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(std::list<Token> &tokens, const std::string &op) {
    auto token = tokens.front();
    if (token.kind != TokenKind::TK_RESERVED || token.str != op)
        error("'%c'ではありません", op);

    tokens.pop_front();
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する
int expect_number(std::list<Token> &tokens) {
    auto token = tokens.front();
    if (token.kind != TokenKind::TK_NUM)
        error("数ではありません");
    int val = token.val;
    tokens.pop_front();
    return val;
}

/*

   expr       = equality
   equality   = relational ("==" relational | "!=" relational)*
   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
   add        = mul ("+" mul | "-" mul)*
   mul        = unary ("+" unary | "/" unary)*
   unary      = ("+" | "-")? primary
   primary    = num | "(" expr ")"

*/

Node *expr(std::list<Token> &tokens);
Node *equality(std::list<Token> &tokens);
Node *relational(std::list<Token> &tokens);
Node *add(std::list<Token> &tokens);
Node *mul(std::list<Token> &tokens);
Node *unary(std::list<Token> &tokens);
Node *primary(std::list<Token> &tokens);

Node *expr(std::list<Token> &tokens) { return equality(tokens); }

Node *equality(std::list<Token> &tokens) {
    Node *node = relational(tokens);
    for (;;) {
        if (consume(tokens, "=="))
            node = new_node(NodeKind::ND_EQ, node, relational(tokens));
        else if (consume(tokens, "!="))
            node = new_node(NodeKind::ND_NE, node, relational(tokens));
        else
            return node;
    }
}

Node *relational(std::list<Token> &tokens) {
    Node *node = add(tokens);

    for (;;) {
        if (consume(tokens, "<"))
            node = new_node(NodeKind::ND_LT, node, add(tokens));
        else if (consume(tokens, "<="))
            node = new_node(NodeKind::ND_LE, node, add(tokens));
        else if (consume(tokens, ">"))
            node = new_node(NodeKind::ND_LT, add(tokens), node);
        else if (consume(tokens, ">="))
            node = new_node(NodeKind::ND_LE, add(tokens), node);
        else
            return node;
    }
}

Node *add(std::list<Token> &tokens) {
    Node *node = mul(tokens);

    for (;;) {
        if (consume(tokens, "+"))
            node = new_node(NodeKind::ND_ADD, node, mul(tokens));
        else if (consume(tokens, "-"))
            node = new_node(NodeKind::ND_SUB, node, mul(tokens));
        else
            return node;
    }
}

Node *mul(std::list<Token> &tokens) {
    Node *node = unary(tokens);

    for (;;) {
        if (consume(tokens, "*"))
            node = new_node(NodeKind::ND_MUL, node, unary(tokens));
        else if (consume(tokens, "/"))
            node = new_node(NodeKind::ND_DIV, node, unary(tokens));
        else
            return node;
    }
}

Node *unary(std::list<Token> &tokens) {
    if (consume(tokens, "+"))
        return unary(tokens);
    if (consume(tokens, "-"))
        return new_node(NodeKind::ND_SUB, new_node_num(0), unary(tokens));
    return primary(tokens);
}

Node *primary(std::list<Token> &tokens) {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume(tokens, "(")) {
        Node *node = expr(tokens);
        expect(tokens, ")");
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number(tokens));
}

void gen(Node *node) {
    if (node->kind == NodeKind::ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case NodeKind::ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case NodeKind::ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case NodeKind::ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case NodeKind::ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case NodeKind::ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case NodeKind::ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case NodeKind::ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case NodeKind::ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    default:
        exit(1);
    }

    printf("  push rax\n");
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        error("引数の個数が正しくありません\n");
        return 1;
    }

    std::list<Token> tokens = tokenize(argv[1]);
    Node *node = expr(tokens);

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
