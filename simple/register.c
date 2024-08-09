#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#define REGISTER_NUM 16

struct {
    uint16_t *ip;

    uint64_t reg[REGISTER_NUM];

    uint64_t result;
} vm;

typedef enum {
    OP_LOADI,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MUL,
    OP_MOV_RES,
    OP_DONE
} opcode;

typedef enum interpret_result {
    SUCCESS,
    ERROR_DIVISION_BY_ZERO,
    ERROR_UNKNOWN_OPCODE
} interpret_result;

void vm_reset(void) {
    puts("Reset");
}

void decode(uint16_t instruction, uint8_t *op, uint8_t *reg0, uint8_t *reg1, uint8_t *reg2, uint8_t *imm) {
    *op = (instruction & 0xF000) >> 12;
    *reg0 = (instruction & 0x0F00) >> 8;
    *reg1 = (instruction & 0x00F0) >> 4;
    *reg2 = (instruction & 0x000F);
    *imm = (instruction & 0x00FF);
}

interpret_result vm_interpret(uint16_t *bytecode) {
    vm_reset();
    puts("Start interpreting");
    vm.ip = bytecode;

    uint8_t op, r0, r1, r2, immediate;
    for (;;) {
        uint16_t instruction = *vm.ip++;
        decode(instruction, &op, &r0, &r1, &r2, &immediate);
        switch (op) {
            case OP_LOADI:
                vm.reg[r0] = immediate;
                break;
            case OP_ADD:
                vm.reg[r2] = vm.reg[r0] + vm.reg[r1];
                break;
            case OP_SUB:
                vm.reg[r2] = vm.reg[r0] - vm.reg[r1];
                break;
            case OP_DIV:
                if (vm.reg[r1] == 0)
                    return ERROR_DIVISION_BY_ZERO;
                vm.reg[r2] = vm.reg[r0] / vm.reg[r1];
                break;
            case OP_MUL:
                vm.reg[r2] = vm.reg[r0] * vm.reg[r1];
                break;
            case OP_MOV_RES:
                vm.result = vm.reg[r0];
                break;
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
#define ENCODE_OP(op) ((op) << 12)
#define ENCODE_OP_REG(op, reg) (ENCODE_OP(op) | ((reg) << 8))
#define ENCODE_OP_REG_IMM(op, reg, imm) (ENCODE_OP(op) | ((reg) << 8) | (imm))
#define ENCODE_OP_REGS(op, reg0, reg1, reg2) (ENCODE_OP(op) | ((reg0) << 8) | ((reg1) << 4) | (reg2))
    {
        uint16_t code[] = {
            ENCODE_OP_REG_IMM(OP_LOADI, 3, 5),
            ENCODE_OP_REG(OP_MOV_RES, 3),
            ENCODE_OP(OP_DONE)
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 5);
    }

    {
        uint16_t code[] = {
            ENCODE_OP_REG_IMM(OP_LOADI, 3, 5),
            ENCODE_OP_REG_IMM(OP_LOADI, 2, 10),
            ENCODE_OP_REGS(OP_ADD, 2, 3, 1),
            ENCODE_OP_REG(OP_MOV_RES, 1),
            ENCODE_OP(OP_DONE)
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 15);
    }

    {
        uint16_t code[] = {
            ENCODE_OP_REG_IMM(OP_LOADI, 0, 7),
            ENCODE_OP_REG_IMM(OP_LOADI, 1, 3),
            ENCODE_OP_REGS(OP_SUB, 0, 1, 2),
            ENCODE_OP_REG(OP_MOV_RES, 2),
            ENCODE_OP(OP_DONE)
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 4);
    }

    {
        uint16_t code[] = {
            ENCODE_OP_REG_IMM(OP_LOADI, 0, 6),
            ENCODE_OP_REG_IMM(OP_LOADI, 1, 2),
            ENCODE_OP_REGS(OP_DIV, 0, 1, 2),
            ENCODE_OP_REG(OP_MOV_RES, 2),
            ENCODE_OP(OP_DONE)
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 3);
    }

    {
        uint16_t code[] = {
            ENCODE_OP_REG_IMM(OP_LOADI, 0, 6),
            ENCODE_OP_REG_IMM(OP_LOADI, 1, 2),
            ENCODE_OP_REGS(OP_MUL, 0, 1, 2),
            ENCODE_OP_REG(OP_MOV_RES, 2),
            ENCODE_OP(OP_DONE)
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 12);
    }

    {
        uint16_t code[] = {
            ENCODE_OP_REG_IMM(OP_LOADI, 1, 11),
            ENCODE_OP_REG_IMM(OP_LOADI, 2, 3),
            ENCODE_OP_REGS(OP_ADD, 1, 2, 3),
            ENCODE_OP_REG_IMM(OP_LOADI, 2, 2),
            ENCODE_OP_REGS(OP_MUL, 2, 3, 0),
            ENCODE_OP_REG(OP_MOV_RES, 0),
            ENCODE_OP(OP_DONE)
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.result);
        assert(result == SUCCESS);
        assert(vm.result == 28);
    }

    return EXIT_SUCCESS;

#undef ENCODE_OP
#undef ENCODE_OP_REG
#undef ENCODE_OP_REG_IMM
#undef ENCODE_OP_REGS
}