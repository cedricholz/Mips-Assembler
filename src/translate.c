#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tables.h"
#include "translate_utils.h"
#include "translate.h"

/* SOLUTION CODE BELOW */
const int TWO_POW_SEVENTEEN = 131072;    // 2^17

/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and other pseudoinstructions. Your pseudoinstruction
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number    //Did we do this??? -RW
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into
        a lui-ori pair.

   If you are going to use the $zero or $0, use $0, not $zero.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(FILE *output, const char *name, char **args, int num_args) {

    if (strcmp(name, "li") == 0) {
        if (num_args != 2) {
            return -1;
        }
        long int imm;
        int err = translate_num(&imm, args[1], INT64_MIN, INT64_MAX);
        if (err == -1) {
            return -1;
        }
        if (imm < INT16_MAX && imm > INT16_MIN) {
            fprintf(output, "addiu %s $0 %s\n", args[0], args[1]);
            return 1;
        } else {
            //lui
            uint32_t luiImmediate = imm >> 16;

            fprintf(output, "lui $at %i\n", luiImmediate);

            //ori
            uint32_t oriImmediate = 0x0000FFFF;
            oriImmediate = oriImmediate & imm;
            fprintf(output, "ori %s $at %i\n", args[0], oriImmediate);
            return 2;
        }
    } else if (strcmp(name, "push") == 0) {
        fprintf(output, "addiu $sp $sp -4\n");
        fprintf(output, "sw %s 0($sp)\n", args[0]);
        return 2;
    } else if (strcmp(name, "pop") == 0) {
        fprintf(output, "lw %s 0($sp)\n", args[0]);
        fprintf(output, "addiu $sp $sp 4\n");
        return 2;
    } else if (strcmp(name, "mod") == 0) {
        fprintf(output, "div %s %s\n", args[1], args[2]);
        fprintf(output, "mfhi %s\n", args[0]);
        return 2;
    } else if (strcmp(name, "subu") == 0) {
        fprintf(output, "addiu $at $0 -1\n");
        fprintf(output, "xor $at $at %s\n", args[2]);
        fprintf(output, "addiu $at $at 1\n");
        fprintf(output, "addu %s %s $at\n", args[0], args[1]);
        return 4;
    } else {
        write_inst_string(output, name, args, num_args);
    }
    return 1;
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros.

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

   Some function declarations for the write_*() functions are provided in translate.h, and you MUST implement these
   and use these as function headers. You may use helper functions, but you still must implement
   the provided write_* functions declared in translate.h.

   Returns 0 on success and -1 on error.
 */
int translate_inst(FILE *output, const char *name, char **args, size_t num_args, uint32_t addr,
                   SymbolTable *symtbl, SymbolTable *reltbl) {
    if (strcmp(name, "addu") == 0) return write_rtype(0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0) return write_rtype(0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0) return write_rtype(0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0) return write_rtype(0x2b, output, args, num_args);
    else if (strcmp(name, "xor") == 0) return write_rtype(0x26, output, args, num_args);
    else if (strcmp(name, "sll") == 0) return write_shift(0x00, output, args, num_args);
    else if (strcmp(name, "jr") == 0) return write_jr(0x08, output, args, num_args);
    else if (strcmp(name, "addiu") == 0) return write_addiu(0x9, output, args, num_args);
    else if (strcmp(name, "ori") == 0) return write_ori(0xd, output, args, num_args);
    else if (strcmp(name, "lui") == 0) return write_lui(0xf, output, args, num_args);
    else if (strcmp(name, "lb") == 0) return write_mem(0x20, output, args, num_args);
    else if (strcmp(name, "lbu") == 0) return write_mem(0x24, output, args, num_args);
    else if (strcmp(name, "lw") == 0) return write_mem(0x23, output, args, num_args);
    else if (strcmp(name, "sb") == 0) return write_mem(0x28, output, args, num_args);
    else if (strcmp(name, "sw") == 0) return write_mem(0x2b, output, args, num_args);
    else if (strcmp(name, "beq") == 0) return write_branch(0x4, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "bne") == 0) return write_branch(0x5, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "j") == 0) return write_jump(0x2, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "jal") == 0) return write_jump(0x3, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "mult") == 0) return write_mult_div(0x18, output, args, num_args);
    else if (strcmp(name, "div") == 0) return write_mult_div(0x1a, output, args, num_args);
    else if (strcmp(name, "mfhi") == 0) return write_mfhi_mflo(0x10, output, args, num_args);
    else if (strcmp(name, "mflo") == 0) return write_mfhi_mflo(0x12, output, args, num_args);
    return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?

    if (num_args != 3 || !args[0] || !args[1] | !args[2]) {
        return -1;
    }

    int rd = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int rt = translate_reg(args[2]);

    if (rs == -1 || rd == -1 || rt == -1) {
        return -1;
    }

    int op = 0 << 26;
    rs = rs << 21;
    rt = rt << 16;
    rd = rd << 11;
    int shamt = 0 << 6;

    uint32_t instruction = op | rs | rt | rd | shamt | funct;

    write_inst_hex(output, instruction);
    return 0;
}

/* A helper function for writing shift instructions. You should use
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_shift(uint8_t funct, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 3 || !args[0] || !args[1] || !args[2]) {
        return 0;
    }

    int rd = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    if (rd == -1 || rt == -1) {
        return -1;
    }

    long int shamt;
    int err = translate_num(&shamt, args[2], 0, 31);
    if (err == -1) {
        return -1;
    }

    int op = 0 << 26;
    int rs = 0 << 21;
    rt = rt << 16;
    rd = rd << 11;
    shamt = shamt << 6;

    uint32_t instruction = op | rs | rt | rd | shamt | funct;
    write_inst_hex(output, instruction);
    return 0;
}

/* The rest of your write_*() functions below */

int write_jr(uint8_t funct, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 1) {
        return -1;
    }
    int rs = translate_reg(args[0]);

    int op = 0 << 26;
    rs = rs << 21;
    int rt = 0 << 16;
    int rd = 0 << 11;
    int shamt = 0 << 6;

    uint32_t instruction = op | rs | rt | rd | shamt | funct;

    write_inst_hex(output, instruction);
    return 0;
}

int write_addiu(uint8_t opcode, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 3) {
        return -1;
    }

    long int imm;

    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);

    int err = translate_num(&imm, args[2], INT16_MIN, INT16_MAX);

    if (rs == -1 || rt == -1 || err == -1) {
        return -1;
    }

    int op = opcode << 26;
    rs = rs << 21;
    rt = rt << 16;
    imm = imm & 0xFFFF;

    uint32_t instruction = op | rs | rt | imm;

    write_inst_hex(output, instruction);
    return 0;
}

int write_ori(uint8_t opcode, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 3) {
        return -1;
    }

    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int err = translate_num(&imm, args[2], 0, UINT16_MAX);

    if (rt == -1 || rs == -1 || err == -1) {
        return -1;
    }

    int op = opcode << 26;
    rs = rs << 21;
    rt = rt << 16;

    uint32_t instruction = op | rs | rt | imm;

    write_inst_hex(output, instruction);
    return 0;
}


int write_mult_div(uint8_t funct, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 2) {
        return -1;
    }

    int rs = translate_reg(args[0]);
    int rt = translate_reg(args[1]);

    if (rs < 0 || rt < 0) {
        return -1;
    }

    int op = 0 << 26;
    rs = rs << 21;
    rt = rt << 16;
    int rd = 0 << 11;
    int shamt = 0 << 6;


    uint32_t instruction = op | rs | rt | rd | shamt | funct;
    write_inst_hex(output, instruction);
    return 0;
}

int write_mfhi_mflo(uint8_t funct, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 1) {
        return -1;
    }

    int rd = translate_reg(args[0]);
    if (rd < 0) {
        return -1;
    }

    int op = 0 << 26;
    int rs = 0 << 21;
    int rt = 0 << 16;
    rd = rd << 11;
    int shamt = 0 << 6;

    uint32_t instruction = op | rs | rt | rd | shamt | funct;
    write_inst_hex(output, instruction);
    return 0;
}

int write_lui(uint8_t opcode, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 2) {
        return -1;
    }

    long int imm;
    int rt = translate_reg(args[0]);
    int err = translate_num(&imm, args[1], 0, UINT16_MAX);

    if (rt == -1 || err == -1) {
        return -1;
    }

    int op = opcode << 26;
    int rs = 0 << 21;
    rt = rt << 16;

    uint32_t instruction = op | rs | rt | imm;

    write_inst_hex(output, instruction);
    return 0;
}

int write_mem(uint8_t opcode, FILE *output, char **args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args != 3) {
        return -1;
    }

    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[2]);
    int err = translate_num(&imm, args[1], INT16_MIN, INT16_MAX);

    if (rt == -1 || rs == -1 || err == -1) {
        return -1;
    }

    int op = opcode << 26;
    rs = rs << 21;
    rt = rt << 16;
    imm = imm & 0xFFFF;

    uint32_t instruction = op | rs | rt | imm;

    write_inst_hex(output, instruction);
    return 0;
}

/*  A helper function to determine if a destination address
    can be branched to
*/
static int can_branch_to(uint32_t src_addr, uint32_t dest_addr) {
    int32_t diff = dest_addr - src_addr;
    return (diff >= 0 && diff <= TWO_POW_SEVENTEEN) || (diff < 0 && diff >= -(TWO_POW_SEVENTEEN - 4));
}


int write_branch(uint8_t opcode, FILE *output, char **args, size_t num_args, uint32_t addr, SymbolTable *symtbl) {
    // Perhaps perform some error checking?
    if (num_args != 3) {
        return -1;
    }

    int rs = translate_reg(args[0]);
    int rt = translate_reg(args[1]);;
    int label_addr = get_addr_for_symbol(symtbl, args[2]);

    if (rs == -1 || rt == -1 || label_addr == -1 || !can_branch_to(addr, label_addr)) {
        return -1;
    }

    //Please compute the branch offset using the MIPS rules.

    int32_t offset = (label_addr - addr - 4) >> 2;
    offset = offset & 0xFFFF;

    //If a branch offset cannot fit inside the immediate field, you should treat it as an error.

    //0b1111111111111111 to decimal = 65535
/*    if (offset > INT16_MAX) {
        return -1;
    }

    if (offset % 4 != 0) {
        return -1;
    }*/

    int op = opcode << 26;
    rs = rs << 21;
    rt = rt << 16;
    uint32_t instruction = op | rs | rt | offset;

    write_inst_hex(output, instruction);
    return 0;
}

int write_jump(uint8_t opcode, FILE *output, char **args, size_t num_args, uint32_t addr, SymbolTable *reltbl) {

    if (num_args != 1) {
        return -1;
    }
    add_to_table(reltbl, args[0], addr);
    uint32_t instruction = opcode << 26;

    write_inst_hex(output, instruction);
    return 0;
}
