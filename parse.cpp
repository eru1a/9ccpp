#include "9cc.h"

// ローカル変数 (変数の名前, オフセット)
// paramsもここに含まれる
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

static Node *new_node_unary(NodeKind kind, Node *expr) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = kind;
    node->lhs = expr;
    return node;
}

static Node *new_node_if(Node *cond, Node *then, Node *els) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_IF;
    node->cond = cond;
    node->then = then;
    node->els = els;
    return node;
}

static Node *new_node_while(Node *cond, Node *then) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_WHILE;
    node->cond = cond;
    node->then = then;
    return node;
}

static Node *new_node_for(Node *init, Node *cond, Node *inc, Node *then) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_FOR;
    node->init = init;
    node->cond = cond;
    node->inc = inc;
    node->then = then;
    return node;
}

static Node *new_node_block(std::vector<Node *> *body) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_BLOCK;
    node->body = body;
    return node;
}

static Node *new_node_funcall(const std::string &funcname, std::vector<Node *> *args) {
    Node *node = static_cast<Node *>(calloc(1, sizeof(Node)));
    node->kind = NodeKind::ND_FUNCALL;
    node->funcname = funcname;
    node->args = args;
    return node;
}

/*

   program    = function*
   function   = ident "(" params? ")" "{" stmt* "}"
   params     = ident ("," ident)*
   stmt       = "return" expr ";"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "{" stmt* "}"
              | expr ";"
   expr       = assign
   assign     = equality ("=" assign)?
   equality   = relational ("==" relational | "!=" relational)*
   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
   add        = mul ("+" mul | "-" mul)*
   mul        = unary ("+" unary | "/" unary)*
   unary      = ("+" | "-")? primary
   primary    = num
              | ident ("(" (assign ("," assign)*)? ")")?
              | "(" expr ")"

*/

static Function function(std::list<Token> &tokens);
static std::vector<Node *> read_func_params(std::list<Token> &tokens);
static Node *stmt(std::list<Token> &tokens);
static Node *expr(std::list<Token> &tokens);
static Node *assign(std::list<Token> &tokens);
static Node *equality(std::list<Token> &tokens);
static Node *relational(std::list<Token> &tokens);
static Node *add(std::list<Token> &tokens);
static Node *mul(std::list<Token> &tokens);
static Node *unary(std::list<Token> &tokens);
static Node *primary(std::list<Token> &tokens);

std::vector<Function> program(std::list<Token> &tokens) {
    std::vector<Function> prog;
    // tokens.empty()使えばTK_EOFいらないのでは?
    while (tokens.front().kind != TokenKind::TK_EOF) {
        prog.push_back(function(tokens));
    }
    return prog;
}

static std::vector<Node *> read_func_params(std::list<Token> &tokens) {
    if (consume(tokens, ")"))
        return {};

    std::vector<Node *> params;
    params.push_back(new_node_lvar(expect_ident(tokens)));

    while (!consume(tokens, ")")) {
        expect(tokens, ",");
        params.push_back(new_node_lvar(expect_ident(tokens)));
    }
    return params;
}

static Function function(std::list<Token> &tokens) {
    locals.clear();

    std::string name = expect_ident(tokens);
    expect(tokens, "(");
    std::vector<Node *> params = read_func_params(tokens);
    expect(tokens, "{");

    std::vector<Node *> code;
    while (!consume(tokens, "}")) {
        code.push_back(stmt(tokens));
    }

    return Function{
        .code = code, .name = name, .params = params, .stack_size = int(locals.size()) * 8};
}

static Node *stmt(std::list<Token> &tokens) {
    if (consume_keyword(tokens, TokenKind::TK_RETURN)) {
        Node *node = new_node_unary(NodeKind::ND_RETURN, expr(tokens));
        expect(tokens, ";");
        return node;
    }
    if (consume_keyword(tokens, TokenKind::TK_IF)) {
        expect(tokens, "(");
        Node *cond = expr(tokens);
        expect(tokens, ")");
        Node *then = stmt(tokens);
        Node *els = nullptr;
        if (consume_keyword(tokens, TokenKind::TK_ELSE))
            els = stmt(tokens);
        return new_node_if(cond, then, els);
    }
    if (consume_keyword(tokens, TokenKind::TK_WHILE)) {
        expect(tokens, "(");
        Node *cond = expr(tokens);
        expect(tokens, ")");
        Node *then = stmt(tokens);
        return new_node_while(cond, then);
    }
    if (consume_keyword(tokens, TokenKind::TK_FOR)) {
        Node *init = nullptr;
        Node *cond = nullptr;
        Node *inc = nullptr;
        expect(tokens, "(");
        if (!consume(tokens, ";")) {
            init = expr(tokens);
            expect(tokens, ";");
        }
        if (!consume(tokens, ";")) {
            cond = expr(tokens);
            expect(tokens, ";");
        }
        if (!consume(tokens, ")")) {
            inc = expr(tokens);
            expect(tokens, ")");
        }
        Node *then = stmt(tokens);
        return new_node_for(init, cond, inc, then);
    }
    if (consume(tokens, "{")) {
        std::vector<Node *> *body = new std::vector<Node *>();
        while (!consume(tokens, "}")) {
            body->push_back(stmt(tokens));
        }
        return new_node_block(body);
    }

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

static std::vector<Node *> *func_args(std::list<Token> &tokens) {
    std::vector<Node *> *args = new std::vector<Node *>();
    if (consume(tokens, ")"))
        return args;

    args->push_back(expr(tokens));
    while (consume(tokens, ",")) {
        args->push_back(expr(tokens));
    }

    expect(tokens, ")");
    return args;
}

static Node *primary(std::list<Token> &tokens) {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume(tokens, "(")) {
        Node *node = expr(tokens);
        expect(tokens, ")");
        return node;
    }
    // ident
    auto t = consume_ident(tokens);
    if (t) {
        // 関数呼び出し
        if (consume(tokens, "(")) {
            std::vector<Node *> *args = func_args(tokens);
            return new_node_funcall(t.value().str, args);
        }
        return new_node_lvar(t.value().str);
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number(tokens));
}
