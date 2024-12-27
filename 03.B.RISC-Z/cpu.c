#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "misc.h"
#include "cpu.h"
#include "memory.h"

#define FUNC3_OFFS 12
#define FUNC7_OFFS 24
#define OPCODE_MASK 0b1111111u
#define FUNC3_MASK  (0b111u << FUNC3_OFFS)
#define FUNC7_MASK  (0b1111111u << FUNC7_OFFS)


#define X0 0
#define A0 10
#define A1 11

/**
 * @brief Opcodes to determine instruction format
 * 
 */
enum rz_formats { // C23
    LUI_FORMAT    = 0b0110111u,
    AUIPC_FORMAT  = 0b0010111u,
    B_FORMAT  = 0b1100011u,
    J_FORMAT  = 0b1101111u,
    JALR_FORMAT = 0b1100111u, // JALR
    R_FORMAT  = 0b0110011u,
    S_FORMAT  = 0b0100011u,
    L_FORMAT =  0b0000011u, // Load, I-format
    I_FORMAT = 0b0010011u, // ADDI..ANDI, SLLI, SRLI, SRAI
    MEM_FORMAT = 0b0001111u, // e.g. FENCE
    SYS_FORMAT = 0b1110011u, // e.g. ECALL, EBREAK
};

/**
 * @brief R-format CODE | F3 | F7
 * 
 */
 enum rz_r_codes {
    ADD_CODE = R_FORMAT | ( 0b000u << FUNC3_OFFS ) | ( 0b0000000u << FUNC7_OFFS ),
    SUB_CODE = R_FORMAT | ( 0b000u << FUNC3_OFFS ) | ( 0b0100000u << FUNC7_OFFS ),
    XOR_CODE = R_FORMAT | ( 0b100u << FUNC3_OFFS ) | ( 0b0000000u << FUNC7_OFFS ),
    MUL_CODE = R_FORMAT | ( 0b000u << FUNC3_OFFS ) | ( 0b0000010u << FUNC7_OFFS ),
 };

 /**
  * @brief I-format code | F3 [ | F7 ]
  * 
  */
enum rz_i_codes {
    ADDI_CODE = I_FORMAT | ( 0b000u << FUNC3_OFFS ),
    SLLI_CODE = I_FORMAT | ( 0b001u << FUNC3_OFFS ),
};

/**
 * @brief B-format code
 * 
 */
enum rz_b_codes {
    BEQ_CODE  = B_FORMAT | ( 0b000u << FUNC3_OFFS ),
    BNE_CODE  = B_FORMAT | ( 0b001u << FUNC3_OFFS ),
    BLT_CODE  = B_FORMAT | ( 0b100u << FUNC3_OFFS ),
    BGE_CODE  = B_FORMAT | ( 0b101u << FUNC3_OFFS ),
    BLTU_CODE = B_FORMAT | ( 0b110u << FUNC3_OFFS )
};

/**
 * @brief U-format code
 * 
 */
enum rz_u_codes {
    LUI_CODE = LUI_FORMAT,
    AUIPC_CODE = AUIPC_FORMAT,
};

enum rz_sys_codes {
    ECALL_CODE = SYS_FORMAT,
    EBREAK_CODE = SYS_FORMAT | ( 1u << 20 ),
};

/**
 * @brief Represent RISC-V machine code for LE host-machines
 * 
 */
typedef union {
    rz_register_t whole;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned rs2: 5;
        unsigned f7: 7;
    } r;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned imm0_11: 12;
    } i;
    struct {
        unsigned op: 7;
        unsigned imm0_4: 5;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned rs2: 5;
        unsigned imm5_11: 7;
    } s;
    struct {
        unsigned op: 7;
        unsigned imm11: 1;
        unsigned imm1_4: 4;
        unsigned f3: 3;
        unsigned rs1: 5;
        unsigned rs2: 5;
        unsigned imm5_10: 6;
        unsigned imm12: 1;
    } b;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned imm12_31: 20;
    } u;
    struct {
        unsigned op: 7;
        unsigned rd: 5;
        unsigned imm12_19: 8;
        unsigned imm11: 1;
        unsigned imm1_10: 10;
        unsigned imm20: 1;
    } j;
} rz_instruction_t;

struct rz_cpu_s {
    const char *info;
    char std_out[512];
    rz_register_t r_pc, r_x[32];
};

const char *rz_stdout(const rz_cpu_p pcpu)
{
    return pcpu->std_out;
}

const char *rz_cpu_info(const rz_cpu_p pcpu)
{
    return pcpu->info;
}

rz_cpu_p rz_create_cpu(void){
    rz_cpu_p pcpu = malloc(sizeof(rz_cpu_t));
    pcpu->info = "RISC-Z.32.2023";

    pcpu->r_pc = TEXT_OFFSET;

    memset(pcpu->r_x, 0, sizeof(pcpu->r_x));
    pcpu->r_x[2] = STACK_OFFSET + STACK_SIZE - (unsigned) sizeof(rz_register_t); // sp
    pcpu->r_x[3] = DATA_OFFSET; // gp

    return pcpu;
}

void rz_free_cpu(rz_cpu_p pcpu){
    free(pcpu);
}

bool rz_r_cycle(rz_cpu_p pcpu, rz_instruction_t instr) {
    switch(instr.whole & (OPCODE_MASK | FUNC3_MASK | FUNC7_MASK)) {
        case ADD_CODE:
            pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] + pcpu->r_x[instr.r.rs2];
        break;
        case SUB_CODE:
            pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] - pcpu->r_x[instr.r.rs2];
        break;

        case MUL_CODE:
            pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] * pcpu->r_x[instr.r.rs2];
        break;
        case XOR_CODE:
            pcpu->r_x[instr.r.rd] = pcpu->r_x[instr.r.rs1] ^ pcpu->r_x[instr.r.rs2];
        break;
        default:
            return false;
    }
    return true;
}

static inline rz_register_t sign_extend(unsigned some_bits, int how_many_bits) {
    rz_register_t result = some_bits;
    if(1 << (how_many_bits - 1) & result) // then negative
        result |= -1 << how_many_bits; // -1 is full of 1s
    return result;
}

bool rz_i_cycle(rz_cpu_p pcpu, rz_instruction_t instr) {
    switch(instr.whole & (OPCODE_MASK | FUNC3_MASK)) {
        case ADDI_CODE:
            pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] + sign_extend(instr.i.imm0_11, 12);
        break;
        case SLLI_CODE:
            pcpu->r_x[instr.i.rd] = pcpu->r_x[instr.i.rs1] << instr.r.rs2;
        break;
        break;
        default:
            return false;
    }
    return true;
}


bool rz_j_cycle(rz_cpu_p pcpu, rz_instruction_t instr) {
    rz_register_t offset = sign_extend(
        (instr.j.imm20 << 19) | (instr.j.imm12_19 << 11) | (instr.j.imm11 << 10) | (instr.j.imm1_10 << 1),
        20
    );
    switch(instr.whole & (OPCODE_MASK)){
        case J_FORMAT:
            pcpu->r_x[instr.j.rd] = pcpu->r_pc + 4;
            pcpu->r_pc += offset;
            return true;
        default:
            return false;
    }
}

bool rz_b_cycle(rz_cpu_p pcpu, rz_instruction_t instr) {
    rz_register_t rs1 = pcpu->r_x[instr.b.rs1];
    rz_register_t rs2 = pcpu->r_x[instr.b.rs2];
    rz_register_t offset = sign_extend(
        (instr.b.imm12 << 11) | (instr.b.imm11 << 10) | (instr.b.imm5_10 << 4) | (instr.b.imm1_4 << 1),
        12
    );
    switch(instr.whole & (OPCODE_MASK | FUNC3_MASK)){
        case BEQ_CODE:
            if(rs1 == rs2) {
                pcpu->r_pc += offset;
            }
            return true;
        case BNE_CODE:
            if(rs1 != rs2) {
                pcpu->r_pc += offset;
            }
            return true;
        case BLT_CODE:
            if((int) rs1 < (int) rs2) {
                pcpu->r_pc += offset;
            }
            return true;
        case BGE_CODE:
            if((int) rs1 >= (int) rs2) {
                pcpu->r_pc += offset;
            }
            return true;
        case BLTU_CODE:
            if(pcpu->r_x[instr.b.rs1] < pcpu->r_x[instr.b.rs2]) {
                pcpu->r_pc += offset;
            }
            return true;
        default:
            return false;
    }
}


bool rz_ebreak(rz_cpu_p pcpu) {
    getchar();
    return true;
}


bool rz_ecall(rz_cpu_p pcpu) {
    switch(pcpu->r_x[A0]) {
        case 1: // print integer
            sprintf(pcpu->std_out, "%d", pcpu->r_x[A1]);
            printf("%d", pcpu->r_x[A1]);
            return true;
        case 17:
            printf("Program exited with code %u", pcpu->r_x[A1]);
            return false;
        default:
            return false;
    }
}

bool rz_sys_cycle(rz_cpu_p pcpu, rz_instruction_t instr) {
    switch(instr.whole){
        case ECALL_CODE:
            return rz_ecall(pcpu);
        case EBREAK_CODE:
            return rz_ebreak(pcpu);
        default:
            return false;
    }
}

bool rz_cycle(rz_cpu_p pcpu) {
    pcpu->r_x[0] = 0u;
    rz_instruction_t instr = *((rz_instruction_t *)mem_access(pcpu->r_pc));
    bool goon = true;

    switch(instr.whole & OPCODE_MASK) {
        case R_FORMAT:
            goon = rz_r_cycle(pcpu, instr);
        break;
        case I_FORMAT:
            goon = rz_i_cycle(pcpu, instr);
        break;
        case L_FORMAT:
        break;
        case S_FORMAT:
        break;
        case LUI_FORMAT:
            pcpu->r_x[instr.u.rd] = instr.u.imm12_31 << 12;
        break;
        case AUIPC_FORMAT:
            pcpu->r_x[instr.u.rd] = (instr.u.imm12_31 << 12) + pcpu->r_pc;
        break;

        case B_FORMAT:
            goon = rz_b_cycle(pcpu, instr);
        break;

        case J_FORMAT:
            goon = rz_b_cycle(pcpu, instr);
        break;
        case JALR_FORMAT:
        break;

        case MEM_FORMAT:
        break;
        case SYS_FORMAT:
            goon = rz_sys_cycle(pcpu, instr);
        break;
        default:
            fprintf(stderr, "Invalid instruction %08X format, opcode %08X\n", instr.whole, instr.whole & OPCODE_MASK);
            goon = false;
    }
    pcpu->r_pc += sizeof(rz_instruction_t);
    return goon;
}
