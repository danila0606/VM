#include <iostream>
#include <cassert>

#include "vm.hpp"

SadVM::SadVM(const std::vector<uint8_t>& data, const std::vector<uint8_t>& text) {
    memspace.resize(MEMSIZE);
    registers.resize(static_cast<int>(Register::REGISTER_COUNT));

    assert(DATA_SECTION_START > TEXT_SECTION_START + text.size());

    memset(memspace.data(), 0, MEMSIZE);
    int register_count = static_cast<int>(Register::REGISTER_COUNT);
    memset(registers.data(), 0, register_count * sizeof(uint32_t));
    registers[static_cast<int>(Register::PC)] = TEXT_SECTION_START;
    registers[static_cast<int>(Register::SP)] = STACK_START;

    for (size_t i = 0; i < text.size(); ++i) {
        memspace[i + TEXT_SECTION_START] = text[i];
    }

    for (size_t i = 0; i < data.size(); ++i) {
        memspace[i + DATA_SECTION_START] = data[i];
    }

    // std::copy(memspace.begin() + TEXT_SECTION_START, text.begin(), text.end());
    // std::copy(memspace.begin() + TEXT_SECTION_START, text.begin(), text.end());
}


void SadVM::run() {
    size_t pc; 
    uint32_t ins;
    while (program_halted == 0) {
        pc = registers[static_cast<int>(Register::PC)];
        ins = *((uint32_t*)(memspace.data() + pc));

        execute(ins);
    }
}


void SadVM::execute(uint32_t ins) {
    uint32_t op;
    uint32_t r1, r2;
    uint32_t imm16, imm21;

    op = get_opcode(ins);
    r1 = get_r1(ins);
    r2 = get_r2(ins);
    imm21 = get_imm21(ins);
    imm16 = get_imm16(ins);


    switch (Opcode(op)) {
        case Opcode::OPCODE_COUNT:
        case Opcode::NOT_AN_OPCODE:
            std::cout << "NOT AN OPCODE"<<std::endl;;
            exit(-1);
          
        case Opcode::HALT:
            program_halted = 1;
            break;
        case Opcode::NOP:
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::RET:
            registers[static_cast<int>(Register::SP)] += 4;
            registers[static_cast<int>(Register::PC)] = *(uint32_t*)(memspace.data() + registers[static_cast<int>(Register::SP)]);
            break;

        case Opcode::ADD:
            registers[r1] += registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::MUL:
            registers[r1] *= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::DIV:
            registers[r1] /= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::EQ:
            registers[static_cast<int>(Register::Z)] = (registers[r1] == registers[r2]);
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::NE:
            registers[static_cast<int>(Register::Z)] = (registers[r1] != registers[r2]);
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::LT:
            registers[static_cast<int>(Register::Z)] = registers[r1] < registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::LE:
            registers[static_cast<int>(Register::Z)] = registers[r1] <= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::GT:
            registers[static_cast<int>(Register::Z)] = registers[r1] > registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::GE:
            registers[static_cast<int>(Register::Z)] = registers[r1] >= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::AND:
            registers[r1] &= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::OR:
            registers[r1] |= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::XOR:
            registers[r1] ^= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::SLL:
            registers[r1] <<= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::SRL:
            registers[r1] >>= registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::MOV:
            registers[r1] = registers[r2];
            registers[static_cast<int>(Register::PC)] += 4;
            break;

        case Opcode::LW:
            registers[r1] = *((uint32_t*)(imm16 + memspace.data() + registers[r2]));
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::SW:
            *((uint32_t*)(imm16 + memspace.data() + registers[r2])) = registers[r1];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::LB:
            registers[r1] = *((uint8_t*)(imm16 + memspace.data() + registers[r2]));
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::SB:
            *((uint8_t*)(imm16 + memspace.data() + registers[r2])) = registers[r1];
            registers[static_cast<int>(Register::PC)] += 4;
            break;

        case Opcode::ADDI:
            registers[r1] += imm21;
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::MULI:
            registers[r1] *= imm21;
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::DIVI:
            registers[r1] /= imm21;
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::LI:
            registers[r1] = imm21;
            registers[static_cast<int>(Register::PC)] += 4;
            break;

        case Opcode::JR:
            registers[static_cast<int>(Register::PC)] = registers[r1];
            break;
        case Opcode::PRINTS:
            printf("%s", ((char*)(memspace.data() + registers[r1])));
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::PRINTI:
            printf("%d", registers[r1]);
            registers[static_cast<int>(Register::PC)] += 4;
            break;

        case Opcode::J:
            registers[static_cast<int>(Register::PC)] = imm21;
            break;
        case Opcode::JS:
            registers[static_cast<int>(Register::PC)] += imm21;
            break;
        case Opcode::JZ:
            if (registers[static_cast<int>(Register::Z)]) {
                registers[static_cast<int>(Register::PC)] = imm21;
                registers[static_cast<int>(Register::Z)] = 0;
            }
            else {
                registers[static_cast<int>(Register::PC)] += 4;
            }
            break;
        case Opcode::JZS:
            if (registers[static_cast<int>(Register::Z)]) {
                registers[static_cast<int>(Register::PC)] += imm21;
                registers[static_cast<int>(Register::Z)] = 0;
            }
            else {
                registers[static_cast<int>(Register::PC)] += 4;
            }
            break;
        case Opcode::CALL:
            *((uint32_t*)(memspace.data() + registers[static_cast<int>(Register::SP)])) = registers[static_cast<int>(Register::PC)] + 4;
            registers[static_cast<int>(Register::SP)] -= 4;
            registers[static_cast<int>(Register::PC)] = imm21;
            break;
        case Opcode::PRINTC:
            printf("%c", imm21);
            registers[static_cast<int>(Register::PC)] += 4;
            break;
    }
}