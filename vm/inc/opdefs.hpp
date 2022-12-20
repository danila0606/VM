#pragma once

#include <cstdint>
#include <cassert>

#define OPCODE_MASK (0x3f)
#define REGISTER_MASK (0x1f)
#define IMMEDIATE_MASK_21BIT (0x1fffff)
#define IMMEDIATE_MASK_16BIT (0xffff)
#define SIGN_BIT_MASK_32BIT (0x80000000)
#define SIGN_BIT_MASK_21BIT (0x100000)
#define SIGN_BIT_MASK_21BIT_TO_32BIT (0xfff00000)
#define SIGN_BIT_MASK_16BIT (0x8000)
#define SIGN_BIT_MASK_16BIT_TO_32BIT (0xffff8000)

#define R1_SHIFT (5 + 16)
#define R2_SHIFT (16)
#define OPCODE_SHIFT (5 + 5 + 16)
#define SIGN_BIT_SHIFT_21BIT (11)
#define SIGN_BIT_SHIFT_16BIT (16)

using Immediate = int32_t;

enum class Opcode {
    NOT_AN_OPCODE = -1,
    // NO_OPERANDS 
    HALT = 0,
    NOP,
    RET,
    SYSCALL,

    //REGISTER_REGISTER 
    ADD,
    MUL,
    DIV,
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    AND,
    OR,
    XOR,
    SLL,
    SRL,
    MOV,
    // REGISTER_REGISTER_OFFSET
    LW,
    SW,
    LB,
    SB,

    /* REGISTER_IMMEDIATE */
    ADDI,
    MULI,
    DIVI,
    LI,

    /* REGISTER_NO_IMMEDIATE */
    JR,
    PUSH,
    POP,
    PRINTS,
    PRINTI,

    /* IMMEDIATE_NO_REGISTER */
    J,
    JS,
    JZ,
    JZS,
    CALL,
    PUSHI,
    PRINTC,

    OPCODE_COUNT
};

enum class Register {
    NOT_A_REGISTER = -1,
    NUL = 0,                                    
    // General registers
    G0, G1, G2, G3, G4, G5, G6, G7,             
    G8, G9, G10, G11,                          
    // float registers
    F0, F1, F2, F3,                             
    // ret registers
    R0, R1,                                     
    
    SYS,                                      
    
    Z,                                          

    SP,                                         

    FP,                                         

    PC,                                         

    REGISTER_COUNT
};     