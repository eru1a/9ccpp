#pragma once

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>

enum class TokenKind {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
};

struct Token {
    TokenKind kind;  // トークンの型
    int val;         // kindがTK_NUMの場合、その数値
    std::string str; // トークン文字列
};

// エラーを報告するための関数
// printfと同じ引数を取る
void error(const char *fmt, ...);

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(std::list<Token> &tokens, const std::string &op);

std::optional<Token> consume_ident(std::list<Token> &tokens);

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(std::list<Token> &tokens, const std::string &op);

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する
int expect_number(std::list<Token> &tokens);

std::list<Token> tokenize(const std::string &s);

// 抽象構文木のノードの種類
enum class NodeKind {
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_ASSIGN, // =
    ND_LVAR,   // ローカル変数
    ND_NUM,    // 整数
};

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
    int offset;    // kindがND_LVARの場合のみ使う
};

struct Function {
    std::list<Node *> code;
    int stack_size;
};

Function program(std::list<Token> &tokens);

void codegen(const Function &prog);
