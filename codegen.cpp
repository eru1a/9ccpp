#include "9cc.h"

static int cnt = 0;
static std::string make_label(const std::string &s) { return ".L." + s + std::to_string(cnt++); }

static void gen_lval(Node *node) {
    if (node->kind != NodeKind::ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

static void gen(Node *node) {
    if (!node)
        return;
    switch (node->kind) {
    case NodeKind::ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case NodeKind::ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case NodeKind::ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case NodeKind::ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case NodeKind::ND_IF:
        if (!node->els) {
            std::string end = make_label("end");

            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je %s\n", end.c_str());
            gen(node->then);
            printf("%s:\n", end.c_str());
        } else {
            std::string els = make_label("else");
            std::string end = make_label("end");

            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je %s\n", els.c_str());
            gen(node->then);
            printf("  jmp %s\n", end.c_str());
            printf("%s:\n", els.c_str());
            gen(node->els);
            printf("%s:\n", end.c_str());
        }
        return;
    case NodeKind::ND_WHILE: {
        std::string begin = make_label("begin");
        std::string end = make_label("end");

        printf("%s:\n", begin.c_str());
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je %s\n", end.c_str());
        gen(node->then);
        printf("  jmp %s\n", begin.c_str());
        printf("%s:\n", end.c_str());
        return;
    }
    case NodeKind::ND_FOR: {
        std::string begin = make_label("begin");
        std::string end = make_label("end");

        gen(node->init);
        printf("%s:\n", begin.c_str());
        if (node->cond) {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je %s\n", end.c_str());
        }
        gen(node->then);
        gen(node->inc);
        printf("  jmp %s\n", begin.c_str());
        printf("%s:\n", end.c_str());
        return;
    }
    case NodeKind::ND_BLOCK:
        for (auto stmt : *(node->body)) {
            gen(stmt);
            printf("  pop rax\n");
        }
        printf("  push rax\n");
        return;
    case NodeKind::ND_FUNCALL:
        printf("  call %s\n", node->funcname.c_str());
        printf("  push rax\n");
        return;
    default:
        break;
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

void codegen(const Function &prog) {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個文の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", prog.stack_size);

    // 先頭の式から順にコード生成
    for (auto node : prog.code) {
        gen(node);

        // 式の評価結果としてスタックに1つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    printf("  ret\n");
}
