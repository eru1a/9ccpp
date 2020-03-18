#include "9cc.h"

static void gen(Node *node) {
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

void codegen(Node *node) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code.
    printf("  pop rax\n");
    printf("  ret\n");
}
