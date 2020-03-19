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
    TK_RETURN,   // return
    TK_IF,       // if
    TK_ELSE,     // else
    TK_WHILE,    // while
    TK_FOR,      // for
    TK_EOF,      // 入力の終わりを表すトークン
};

struct Token {
    TokenKind kind;  // トークンの型
    int val;         // kindがTK_NUMの場合、その数値
    std::string str; // トークン文字列

    std::string to_string() const {
        switch (kind) {
        case TokenKind::TK_RESERVED:
            return "RESERVED: " + str;
        case TokenKind::TK_IDENT:
            return "IDENT: " + str;
        case TokenKind::TK_NUM:
            return "NUM: " + std::to_string(val);
        case TokenKind::TK_RETURN:
            return "RETURN";
        case TokenKind::TK_IF:
            return "IF";
        case TokenKind::TK_ELSE:
            return "ELSE";
        case TokenKind::TK_WHILE:
            return "WHILE";
        case TokenKind::TK_FOR:
            return "FOR";
        case TokenKind::TK_EOF:
            return "EOF";
        default:
            return "UNKNOWN";
        }
    }
};

// エラーを報告するための関数
// printfと同じ引数を取る
void error(const char *fmt, ...);

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(std::list<Token> &tokens, const std::string &op);

std::optional<Token> consume_ident(std::list<Token> &tokens);

bool consume_keyword(std::list<Token> &tokens, TokenKind kind);

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
    ND_RETURN, // "return"
    ND_IF,     // "if"
    ND_WHILE,  // "while"
    ND_FOR,    // "for"
    ND_BLOCK,  // "{ ... }"
    ND_NUM,    // 整数
};

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺

    // "if" or "while" or "for" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // Block
    std::list<Node *> *body;

    int val;    // kindがND_NUMの場合のみ使う
    int offset; // kindがND_LVARの場合のみ使う
};

struct Function {
    std::list<Node *> code;
    int stack_size;
};

Function program(std::list<Token> &tokens);

void codegen(const Function &prog);
