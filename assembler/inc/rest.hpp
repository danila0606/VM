#pragma once

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <variant>

#include "opdefs.hpp"

#define MEMSIZE (0x10000)
#define TEXT_SECTION_START (0x100)
#define TEXT_SECTION_SIZE (0x1000)
#define DATA_SECTION_START (TEXT_SECTION_START + TEXT_SECTION_SIZE + 1)
#define DATA_SECTION_SIZE (0x2000)
#define HEAP_STACK_SECTION_START (DATA_SECTION_START + DATA_SECTION_SIZE + 1)
#define HEAP_STACK_SECTION_STOP (MEMSIZE - 1)
#define STACK_START (0x5000)

static std::vector<unsigned char> int_to_bytes(int32_t value) {
    std::vector<unsigned char> array_of_bytes(4, 0);
    for (int i = 0; i < array_of_bytes.size(); ++i) {
        array_of_bytes[i] = (value >> (i * 8));
    }
    return array_of_bytes;
}

static std::vector<unsigned char> string_to_bytes(std::string value) {
    std::vector<unsigned char> array_of_bytes(value.size() + 1, 0);
    for (int i = 0; i < array_of_bytes.size(); ++i) {
        array_of_bytes[i] = static_cast<unsigned char>(value[i]);
    }
    return array_of_bytes;
}

static int32_t get_opcode(int32_t ins) {
    return (ins >> OPCODE_SHIFT) & OPCODE_MASK;
}
static int32_t get_r1(int32_t ins) {
    return (ins >> R1_SHIFT) & REGISTER_MASK;
}
static int32_t get_r2(int32_t ins) {
    return (ins >> R2_SHIFT) & REGISTER_MASK;
}
static int32_t get_imm21(int32_t ins) {
    int32_t sign_bit = ins & SIGN_BIT_MASK_21BIT;
    if (sign_bit) {
        sign_bit |= SIGN_BIT_MASK_21BIT_TO_32BIT;
    }
    return (ins & IMMEDIATE_MASK_21BIT) | sign_bit;
}
static int32_t get_imm16(int32_t ins) {
    int32_t sign_bit = ins & SIGN_BIT_MASK_16BIT;
    if (sign_bit) {
        sign_bit |= SIGN_BIT_MASK_16BIT_TO_32BIT;
    }
    return (ins & IMMEDIATE_MASK_16BIT) | sign_bit;
}

enum class InstructionType {
    INVALID_INSTRUCTION_TYPE = 0,
    /* oooooo 00000000000000000000000000 */
    /*    6              26              */
    NO_OPERANDS,

    /* oooooo rrrrr rrrrr 0000000000000000 */
    /*    6     5     5         16         */
    REGISTER_REGISTER,

    /* oooooo rrrrr rrrrr mmmmmmmmmmmmmmmm */
    /*    6     5     5         16         */
    REGISTER_REGISTER_OFFSET,

    /* oooooo rrrrr mmmmmmmmmmmmmmmmmmmmm */
    /*    6     5           21            */
    REGISTER_IMMEDIATE,

    /* oooooo rrrrr 000000000000000000000 */
    /*    6     5            21           */
    REGISTER_NO_IMMEDIATE,

    /* oooooo 00000 mmmmmmmmmmmmmmmmmmmmm  */
    /*    6                  21            */
    IMMEDIATE_NO_REGISTER,

    INSTRUCTION_TYPE_COUNT
};

static InstructionType get_type(Opcode opcode) {
    switch(opcode) {
        case Opcode::OPCODE_COUNT:
        case Opcode::NOT_AN_OPCODE:
            return InstructionType::INVALID_INSTRUCTION_TYPE;

        case Opcode::HALT:
        case Opcode::NOP:
        case Opcode::RET:
            return InstructionType::NO_OPERANDS;

        case Opcode::ADD:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::EQ:
        case Opcode::NE:
        case Opcode::LT:
        case Opcode::LE:
        case Opcode::GT:
        case Opcode::GE:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::SLL:
        case Opcode::SRL:
        case Opcode::MOV:
            return InstructionType::REGISTER_REGISTER;

        case Opcode::LW:
        case Opcode::SW:
        case Opcode::LB:
        case Opcode::SB:
        case Opcode::GETFIELD:
            return InstructionType::REGISTER_REGISTER_OFFSET;

        case Opcode::ADDI:
        case Opcode::MULI:
        case Opcode::DIVI:
        case Opcode::LI:
        case Opcode::SETFIELD:
            return InstructionType::REGISTER_IMMEDIATE;

        case Opcode::JR:
        case Opcode::PRINTS:
        case Opcode::PRINTI:
            return InstructionType::REGISTER_NO_IMMEDIATE;

        case Opcode::J:
        case Opcode::JS:
        case Opcode::JZ:
        case Opcode::JZS:
        case Opcode::CALL:
        case Opcode::NEW:
        case Opcode::PRINTC:
            return InstructionType::IMMEDIATE_NO_REGISTER;
    }

    return InstructionType::INVALID_INSTRUCTION_TYPE;
}

static int get_num_operands_reg(Opcode op) {
    switch (get_type(op)) {
        case InstructionType::INSTRUCTION_TYPE_COUNT:
        case InstructionType::INVALID_INSTRUCTION_TYPE: return -1;

        case InstructionType::NO_OPERANDS: return 0;

        case InstructionType::REGISTER_NO_IMMEDIATE: return 1;
        case InstructionType::IMMEDIATE_NO_REGISTER: return 0;

        case InstructionType::REGISTER_REGISTER: return 2;
        case InstructionType::REGISTER_IMMEDIATE: return 1;

        case InstructionType::REGISTER_REGISTER_OFFSET: return 2;
    }
    return -1;
}

static int get_num_operands_imm(Opcode op) {
    switch (get_type(op)) {
        case InstructionType::INSTRUCTION_TYPE_COUNT:
        case InstructionType::INVALID_INSTRUCTION_TYPE: return -1;

        case InstructionType::NO_OPERANDS: return 0;

        case InstructionType::REGISTER_NO_IMMEDIATE: return 0;
        case InstructionType::IMMEDIATE_NO_REGISTER: return 1;

        case InstructionType::REGISTER_REGISTER: return 0;
        case InstructionType::REGISTER_IMMEDIATE: return 1;

        case InstructionType::REGISTER_REGISTER_OFFSET: return 1;
    }
    return -1;
}

static int32_t assemble_no_operands(Opcode opcode) {
    return static_cast<int32_t>(opcode) << OPCODE_SHIFT;
}
static int32_t assemble_register_register(Opcode op, Register r1, Register r2 ) {
    int32_t ins = 0;
    ins |= (static_cast<int32_t>(op) << OPCODE_SHIFT);
    ins |= (static_cast<int32_t>(r1) << R1_SHIFT);
    ins |= (static_cast<int32_t>(r2) << R2_SHIFT);
    return ins;
}
static int32_t assemble_register_immediate(Opcode op, Register r, int32_t imm) {
    int32_t ins = 0;
    int32_t sign_bit = (imm & SIGN_BIT_MASK_32BIT) >> SIGN_BIT_SHIFT_21BIT;
    imm &= IMMEDIATE_MASK_21BIT;
    imm |= sign_bit;

    ins |= (static_cast<int32_t>(op) << OPCODE_SHIFT);
    ins |= (static_cast<int32_t>(r) << R1_SHIFT);
    ins |= imm;
    return ins;
}
static int32_t assemble_register_no_immediate(Opcode op, Register r) {
    int32_t ins = 0;
    ins |= (static_cast<int32_t>(op) << OPCODE_SHIFT);
    ins |= (static_cast<int32_t>(r) << R1_SHIFT);
    return ins;
}
static int32_t assemble_register_register_offset(Opcode op, Register r1, Register r2, int32_t imm) {
    int32_t ins = assemble_register_register(op, r1, r2);

    int32_t sign_bit = (imm & SIGN_BIT_MASK_32BIT) >> SIGN_BIT_SHIFT_16BIT;
    imm &= IMMEDIATE_MASK_16BIT;
    imm |= sign_bit;

    ins |= imm;
    return ins;
}
static int32_t assemble_immediate_no_register(Opcode op, int32_t imm) {
    int32_t ins = 0;
    int32_t sign_bit = (imm & SIGN_BIT_MASK_32BIT) >> SIGN_BIT_SHIFT_21BIT;
    imm &= IMMEDIATE_MASK_21BIT;
    imm |= sign_bit;


    ins |= (static_cast<int32_t>(op) << OPCODE_SHIFT);
    ins |= imm;
    return ins;
}

static int32_t assemble_instruction(Opcode opcode, int32_t operand1, int32_t operand2, int32_t operand3) {
    InstructionType ty = get_type(opcode);

    switch(ty) {
        case InstructionType::INSTRUCTION_TYPE_COUNT:
        case InstructionType::INVALID_INSTRUCTION_TYPE:
            return -1;
        case InstructionType::NO_OPERANDS:
            return assemble_no_operands(opcode);
        case InstructionType::REGISTER_REGISTER:
            return assemble_register_register(opcode, static_cast<Register>(operand1), static_cast<Register>(operand2));
        case InstructionType::REGISTER_REGISTER_OFFSET:
            return assemble_register_register_offset(opcode, static_cast<Register>(operand1), static_cast<Register>(operand2), operand3);
        case InstructionType::REGISTER_IMMEDIATE:
            return assemble_register_immediate(opcode, static_cast<Register>(operand1), operand2);
        case InstructionType::REGISTER_NO_IMMEDIATE:
            return assemble_register_no_immediate(opcode, static_cast<Register>(operand1));
        case InstructionType::IMMEDIATE_NO_REGISTER:
            return assemble_immediate_no_register(opcode, operand1);
    }
}

struct Instruction {
    InstructionType type;

    Opcode opcode;
    std::variant<Register, int32_t> operand1;
    std::variant<Register, int32_t> operand2;
    std::variant<Register, int32_t> operand3;

    int32_t assembled_value;

    std::string opcode_str, operand1_str, operand2_str, operand3_str;
    std::string disassembled_str;

    Instruction(Opcode new_opcode, int32_t operand1, int32_t operand2, int32_t operand3) {
        type = get_type(new_opcode);
        opcode = new_opcode;
        assembled_value = assemble_instruction(new_opcode, operand1, operand2, operand3);
    }

    Instruction(Opcode new_opcode, int32_t operand1, int32_t operand2):
        Instruction(new_opcode, operand1, operand2, 0) {
        }

    Instruction(Opcode new_opcode, int32_t operand1):
        Instruction(new_opcode, operand1, 0, 0) {
        }

    Instruction(Opcode new_opcode):
        Instruction(new_opcode, 0, 0, 0) {
        }
};

static Register str_to_reg(std::string& str) {
    Register reg = Register::NOT_A_REGISTER;
    std::transform(str.begin(), str.end(), str.begin(),
    [](unsigned char c){ return std::tolower(c); });
    
    if      (str == "nul") { reg = Register::NUL; }
    else if (str == "g0") { reg = Register::G0; }
    else if (str == "g1") { reg = Register::G1; }
    else if (str == "g2") { reg = Register::G2; }
    else if (str == "g3") { reg = Register::G3; }
    else if (str == "g4") { reg = Register::G4; }
    else if (str == "g5") { reg = Register::G5; }
    else if (str == "g6") { reg = Register::G6; }
    else if (str == "g7") { reg = Register::G7; }
    else if (str == "g8") { reg = Register::G8; }
    else if (str == "g9") { reg = Register::G9; }
    else if (str == "g10") { reg = Register::G10; }
    else if (str == "g11") { reg = Register::G11; }
    else if (str == "r0") { reg = Register::R0; }
    else if (str == "r1") { reg = Register::R1; }
    else if (str == "z")  { reg = Register::Z; }
    else if (str == "pc") { reg = Register::PC; }

    return reg;
}

static Opcode str_to_op(std::string& str) {
    Opcode op = Opcode::NOT_AN_OPCODE;
    std::transform(str.begin(), str.end(), str.begin(),
    [](unsigned char c){ return std::tolower(c); });

    if      (str == "halt") { op = Opcode::HALT; }
    else if (str == "nop")  { op = Opcode::NOP; }
    else if (str == "ret")  { op = Opcode::RET; }
    else if (str == "add") { op = Opcode::ADD; }
    else if (str == "mul") { op = Opcode::MUL; }
    else if (str == "div") { op = Opcode::DIV; }
    else if (str == "eq") { op = Opcode::EQ; }
    else if (str == "ne") { op = Opcode::NE; }
    else if (str == "lt") { op = Opcode::LT; }
    else if (str == "le") { op = Opcode::LE; }
    else if (str == "gt") { op = Opcode::GT; }
    else if (str == "ge") { op = Opcode::GE; }
    else if (str == "and") { op = Opcode::AND; }
    else if (str == "or")  { op = Opcode::OR; }
    else if (str == "xor") { op = Opcode::XOR; }
    else if (str == "sll") { op = Opcode::SLL; }
    else if (str == "srl") { op = Opcode::SRL; }
    else if (str == "mov") { op = Opcode::MOV; }
    else if (str == "lw") { op = Opcode::LW; }
    else if (str == "sw") { op = Opcode::SW; }
    else if (str == "lb") { op = Opcode::LB; }
    else if (str == "sb") { op = Opcode::SB; }
    else if (str == "addi") { op = Opcode::ADDI; }
    else if (str == "muli") { op = Opcode::MULI; }
    else if (str == "divi") { op = Opcode::DIVI; }
    else if (str == "li")   { op = Opcode::LI; }
    else if (str == "jr")     { op = Opcode::JR; }
    else if (str == "prints") { op = Opcode::PRINTS; }
    else if (str == "printi") { op = Opcode::PRINTI; }
    else if (str == "j")     { op = Opcode::J; }
    else if (str == "js")    { op = Opcode::JS; }
    else if (str == "jz")    { op = Opcode::JZ; }
    else if (str == "jzs")   { op = Opcode::JZS; }
    else if (str == "call")  { op = Opcode::CALL; }
    else if (str == "new")  { op = Opcode::NEW;}
    else if (str == "setfield")  { op = Opcode::SETFIELD;}
    else if (str == "getfield")  { op = Opcode::GETFIELD;}
    else if (str == "printc"){ op = Opcode::PRINTC; }

    return op;
}