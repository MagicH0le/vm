#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#define STACK_MAX 256

struct {
    uint8_t *ip;
    uint64_t stack[STACK_MAX];
    uint64_t *stack_top;
    uint64_t result;
} vm;

typedef enum {
    OP_PUSHI,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MUL,
    OP_POP_RES,
    OP_DONE
} opcode;

typedef enum interpret_result {
    SUCCESS,
    ERROR_DIVISION_BY_ZERO,
    ERROR_UNKNOWN_OPCODE
} interpret_result;

void vm_reset(void) {
    puts("Reset vm state");
    vm = (typeof(vm)) { NULL };
    vm.stack_top = vm.stack;
}

void vm_stack_push(uint64_t value) {
    *vm.stack_top = value;
    vm.stack_top++;
}

uint64_t vm_stack_pop(void) {
    vm.stack_top--;
    return *vm.stack_top;
}

interpret_result vm_interpret(uint8_t *bytecode) {
    vm_reset();

    puts("Start interpreting");
    vm.ip = bytecode;
    for (;;) {
        uint8_t instruction = *vm.ip++;
        switch (instruction) {
            case OP_PUSHI: {
                uint8_t arg = *vm.ip++;
                vm_stack_push(arg);
                break;
            }
            case OP_ADD: {
                uint64_t arg_right = vm_stack_pop();
                uint64_t arg_left = vm_stack_pop();
                uint64_t res = arg_left + arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_SUB: {
                uint64_t arg_right = vm_stack_pop();
                uint64_t arg_left = vm_stack_pop();
                uint64_t res = arg_left - arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_DIV: {
                uint64_t arg_right = vm_stack_pop();
                if (arg_right == 0)
                    return ERROR_DIVISION_BY_ZERO;
                uint64_t arg_left = vm_stack_pop();
                uint64_t res = arg_left / arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_MUL: {
                uint64_t arg_right = vm_stack_pop();
                uint64_t arg_left = vm_stack_pop();
                uint64_t res = arg_left * arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_POP_RES: {
                uint64_t res = vm_stack_pop();
                vm.result = res;
                break;
            }
            case OP_DONE:
                return SUCCESS;
            default:
                return ERROR_UNKNOWN_OPCODE;
        }
    }

    return SUCCESS;
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    {
        uint8_t code[] = { OP_PUSHI, 5, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 5);
    }

    {
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 5, OP_ADD, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 15);
    }

    {
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 6, OP_SUB, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 4);
    }

    {
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 5, OP_DIV, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 2);
    }

    {
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 0, OP_DIV, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == ERROR_DIVISION_BY_ZERO);
    }

    {
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 2, OP_MUL, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 20);
    }

    {
        uint8_t code[] = { OP_PUSHI, 2, OP_PUSHI, 11, OP_PUSHI, 3, OP_ADD, OP_MUL, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 28);
    }

    return EXIT_SUCCESS;
}