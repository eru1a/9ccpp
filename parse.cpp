#include "9cc.h"

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_num(int val) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_NUM;
    node->val = val;
    return node;
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

static Node *equality(std::list<Token> &tokens);
static Node *relational(std::list<Token> &tokens);
static Node *add(std::list<Token> &tokens);
static Node *mul(std::list<Token> &tokens);
static Node *unary(std::list<Token> &tokens);
static Node *primary(std::list<Token> &tokens);

Node *expr(std::list<Token> &tokens) { return equality(tokens); }

static Node *equality(std::list<Token> &tokens) {
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

static Node *relational(std::list<Token> &tokens) {
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

static Node *add(std::list<Token> &tokens) {
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

static Node *mul(std::list<Token> &tokens) {
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

static Node *unary(std::list<Token> &tokens) {
    if (consume(tokens, "+"))
        return unary(tokens);
    if (consume(tokens, "-"))
        return new_node(NodeKind::ND_SUB, new_node_num(0), unary(tokens));
    return primary(tokens);
}

static Node *primary(std::list<Token> &tokens) {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume(tokens, "(")) {
        Node *node = expr(tokens);
        expect(tokens, ")");
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number(tokens));
}

