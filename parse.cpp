#include "9cc.h"

// ローカル変数 (変数の名前, オフセット)
static std::map<std::string, int> locals;

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

static Node *new_node_lvar(const std::string &name) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_LVAR;
    if (locals.count(name) == 0) {
        locals[name] = (locals.size() + 1) * 8;
        node->offset = locals.at(name);
    } else {
        node->offset = locals.at(name);
    }
    return node;
}

/*

   program    = stmt*
   stmt       = expr ";"
   expr       = assign
   assign     = equality ("=" assign)?
   equality   = relational ("==" relational | "!=" relational)*
   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
   add        = mul ("+" mul | "-" mul)*
   mul        = unary ("+" unary | "/" unary)*
   unary      = ("+" | "-")? primary
   primary    = num | ident | "(" expr ")"

*/

static Node *stmt(std::list<Token> &tokens);
static Node *expr(std::list<Token> &tokens);
static Node *assign(std::list<Token> &tokens);
static Node *equality(std::list<Token> &tokens);
static Node *relational(std::list<Token> &tokens);
static Node *add(std::list<Token> &tokens);
static Node *mul(std::list<Token> &tokens);
static Node *unary(std::list<Token> &tokens);
static Node *primary(std::list<Token> &tokens);

Function program(std::list<Token> &tokens) {
    std::list<Node *> code;
    // tokens.empty()使えばTK_EOFいらないのでは?
    while (tokens.front().kind != TokenKind::TK_EOF) {
        code.push_back(stmt(tokens));
    }
    return Function{.code = code, .stack_size = int(locals.size()) * 8};
}

static Node *stmt(std::list<Token> &tokens) {
    Node *node = expr(tokens);
    expect(tokens, ";");
    return node;
}

static Node *expr(std::list<Token> &tokens) { return assign(tokens); }

static Node *assign(std::list<Token> &tokens) {
    Node *node = equality(tokens);
    if (consume(tokens, "="))
        node = new_node(NodeKind::ND_ASSIGN, node, assign(tokens));
    return node;
}

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
    auto t = consume_ident(tokens);
    if (t) {
        Node *node = new_node_lvar(t.value().str);
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number(tokens));
}
