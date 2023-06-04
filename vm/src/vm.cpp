#include <iostream>
#include <cassert>
#include <cstring>

#include "vm.hpp"

static ClassDescription parse_class(const std::vector<uint8_t>& memspace, size_t class_num) {

    ClassDescription result;

    size_t offset = DATA_SECTION_START + class_num * CLASS_DEF_SIZE + sizeof(uint32_t); // first 4 bytes - classes count field

    offset += 1; // 1 byte - number of class, we assume that they go in order

    static const size_t fields_count = 7;
    for (uint32_t i = 0; i < fields_count; ++i) {
        ClassField f;
        if (memspace[offset + i] == BAD_FLAG) { // no fields more
            break;
        }
        else if (memspace[offset + i] == INT32_FLAG) {
            f.type = FieldType::INT32;
            f.field = 0xFF;
        }
        else {
            f.type = FieldType::CLASS;
            f.field = memspace[offset + i];
        }
        result.fields.push_back(f);
    }

    offset += fields_count;

    result.ctor_addr = *(uint32_t*)(memspace.data() + offset);

    offset += sizeof(uint32_t);

    for (uint32_t i = 0; i < fields_count; ++i) {
        if (memspace[offset + i * sizeof(uint32_t)] == BAD_FLAG) { // no methods more
            break;
        }
        uint32_t method_addr = *(uint32_t*)(memspace.data() + offset + i * sizeof(uint32_t));
        result.methods_addrs.push_back(method_addr);
    }
    
    return result;
}

// -------------------------------- heap methods --------------------------------

static uint32_t give_first_free_place(const std::vector<std::pair<uint32_t, uint32_t>>& heap_places, uint32_t ob_size) {

    if (heap_places.empty()) {
        return 0;
    }

    if (heap_places.size() == 1) {
        uint32_t end = heap_places[0].first + heap_places[0].second;
        if (ob_size < HEAP_STACK_SECTION_SIZE - end) {
            return end;
        }
        else if (heap_places[0].first >= ob_size) {
            return 0;
        }
        else
            return uint32_t(-1);
    }

    size_t i = 0;
    while (i < heap_places.size() - 1) {

        uint32_t interval = heap_places[i + 1].first - (heap_places[i].first + heap_places[i].second);
        if (ob_size <= interval) {
            return heap_places[i + 1].first;
        }
        else 
            ++i;
    }

    uint32_t last = heap_places.size() - 1;
    uint32_t right_end = heap_places[last].first + heap_places[last].second;
    if (ob_size < HEAP_STACK_SECTION_SIZE - right_end) {
        return right_end;
    }
    else
        return uint32_t(-1);
}

// for allocating memory
static void insert_new_place(std::vector<std::pair<uint32_t, uint32_t>>& heap_places, std::pair<uint32_t, uint32_t> new_place) {

    size_t old_count = heap_places.size();
    for (size_t i = 0; i < old_count; ++i) {
        if (new_place.first + new_place.second < heap_places[i].first) {
            heap_places.insert(heap_places.begin() + i, new_place);
            return;
        }
        else if (new_place.first + new_place.second == heap_places[i].first) {
            heap_places[i].first   = new_place.first;
            heap_places[i].second += new_place.second;
            return;
        }
        else if (new_place.first == heap_places[i].first + heap_places[i].second) {
            if (i + 1 < old_count) { // next place exists
                if (new_place.first + new_place.second == heap_places[i + 1].first) {
                    heap_places[i].second += (new_place.second + heap_places[i + 1].second);
                    heap_places.erase(heap_places.begin() + i + 1);
                    return;
                }
            }
            
            heap_places[i].second += new_place.second;
            return;
        }
    }

    heap_places.push_back(new_place);
}

// for free memory
static void erase_place(std::vector<std::pair<uint32_t, uint32_t>>& heap_places, std::pair<uint32_t, uint32_t> place) {
    
    size_t old_size = heap_places.size();
    for (size_t i = 0; i < old_size; ++i) {
         if (place.first == heap_places[i].first) {

            if (place.second == heap_places[i].second) {
                heap_places.erase(heap_places.begin() + i);
                return;
            }
            else {
                //  assume that place.second < heap_places[i].second
                heap_places[i].first += place.second;
                return;
            }
        }
        else if ((place.first > heap_places[i].first) && (place.first < heap_places[i].first + heap_places[i].first)) {
            if (place.first + place.second == heap_places[i].first + heap_places[i].first) {
                heap_places[i].second -= place.second;
                return;
            }
            else {
                auto new_place = std::make_pair<uint32_t, uint32_t>(place.first + place.second, (heap_places[i].first + heap_places[i].second) - (place.first + place.second));
                heap_places[i].second = place.first - heap_places[i].first;
                heap_places.insert(heap_places.begin() + i + 1, new_place);
                return;
            }
        }
    }

}

// ------------------------------------------------------------------------------

SadVM::SadVM(const std::vector<uint8_t>& data, const std::vector<uint8_t>& text) {
    memspace.resize(MEMSIZE);
    registers.resize(static_cast<int>(Register::REGISTER_COUNT));
    reg_types.resize(registers.size(), 0u);

    assert(DATA_SECTION_START > TEXT_SECTION_START + text.size());

    //memset(memspace.data(), 0, MEMSIZE);
    int register_count = static_cast<int>(Register::REGISTER_COUNT);
    //memset(registers.data(), 0, register_count * sizeof(int32_t));
    registers[static_cast<int>(Register::PC)] = TEXT_SECTION_START;
    registers[static_cast<int>(Register::FP)] = STACK_START;

    for (size_t i = 0; i < text.size(); ++i) {
        memspace[i + TEXT_SECTION_START] = text[i];
    }

    for (size_t i = 0; i < data.size(); ++i) {
        memspace[i + DATA_SECTION_START] = data[i];
    }

    classes_count = *(uint32_t*)(memspace.data() + DATA_SECTION_START);
    class_descriptions.resize(classes_count);

    for (size_t i = 0; i < classes_count; ++i) {
        auto class_des = parse_class(memspace, i);

        class_des.cl_size = 0;
        for (auto f : class_des.fields) {
            if (f.type == FieldType::INT32) {
                class_des.cl_size += sizeof(int32_t);
            }
            else if (f.type == FieldType::CLASS) {
                class_des.cl_size += sizeof(uint32_t); //class_descriptions[f.field].cl_size;
            }
        }

        class_descriptions[i] = class_des;
    }
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

    Frame prev_fr, fr;

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
            registers[static_cast<int>(Register::FP)] -= sizeof(Frame);
            prev_fr = *(Frame*)(memspace.data() + registers[static_cast<int>(Register::FP)]);

            registers[static_cast<int>(Register::PC)] = prev_fr.source_pc;

            memcpy(registers.data() + 2 * sizeof(uint32_t), prev_fr.saved_registers, 12 * sizeof(int32_t));
            memcpy(reg_types.data() + 2 * sizeof(uint32_t), prev_fr.saved_types, 12 * sizeof(uint32_t));
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
        case Opcode::REM:
            registers[r1] %= registers[r2];
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
            fr.source_pc = registers[static_cast<int>(Register::PC)] + 4;
            memcpy(fr.saved_registers, registers.data() + 2 * sizeof(int32_t), 12 * sizeof(int32_t));
            memcpy(fr.saved_types, reg_types.data() + 2 * sizeof(uint32_t), 12 * sizeof(uint32_t));
            memcpy(memspace.data() + registers[static_cast<int>(Register::FP)], &fr, sizeof(Frame));

            //*((Frame*)(memspace.data() + registers[static_cast<int>(Register::FP)])) = fr;
            registers[static_cast<int>(Register::FP)] += sizeof(Frame);
            registers[static_cast<int>(Register::PC)] = imm21;
            break;
        case Opcode::PRINTC:
            printf("%c", imm21);
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        case Opcode::NOB: {
            uint32_t obj_size = class_descriptions[imm21].cl_size;

            auto start = give_first_free_place(heap_places, obj_size + 1);
            insert_new_place(heap_places, {start, obj_size + 1});

            uint32_t obj_addr = HEAP_STACK_SECTION_START + start;
            std::vector<unsigned char> obj(obj_size + 1, 0);
            obj[0] = imm21;

            registers[r1] = obj_addr;
            reg_types[r1] = 1; // pointer to obj
            registers[static_cast<int>(Register::R1)] = obj_addr;

            memcpy(memspace.data() + obj_addr, obj.data(), obj_size + 1); // allocating

            // calling ctor
            fr.source_pc = registers[static_cast<int>(Register::PC)] + 4;
            memcpy(fr.saved_registers, registers.data() + 2 * sizeof(int32_t), 12 * sizeof(int32_t));
            memcpy(fr.saved_types, reg_types.data() + 2 * sizeof(uint32_t), 12 * sizeof(uint32_t));
            memcpy(memspace.data() + registers[static_cast<int>(Register::FP)], &fr, sizeof(Frame));

            //*((Frame*)(memspace.data() + registers[static_cast<int>(Register::FP)])) = fr;
            registers[static_cast<int>(Register::FP)] += sizeof(Frame);
            registers[static_cast<int>(Register::PC)] = class_descriptions[imm21].ctor_addr;
            
            break;
        }
        case Opcode::NOBA: { // allocating array of pointers

            // 1 byte - type of class, next 4 bytes - number of pointer
            int N = registers[r1];
            uint32_t array_size = sizeof(uint32_t) * N; // N pointers

            auto start = give_first_free_place(heap_places, array_size + sizeof(uint32_t));
            insert_new_place(heap_places, {start, array_size + sizeof(uint32_t)});

            uint32_t obj_addr = HEAP_STACK_SECTION_START + start;

            std::vector<unsigned char> pointers_array(sizeof(uint32_t) + array_size, 0);
            auto N_bytes = int_to_bytes(N);
            std::copy(N_bytes.begin(), N_bytes.end(), pointers_array.begin());

            registers[r2] = obj_addr;
            reg_types[r2] = 2; // pointer to array of pointers

            memcpy(memspace.data() + obj_addr, pointers_array.data(), array_size + sizeof(uint32_t)); // allocating pointers array

            registers[static_cast<int>(Register::PC)] += 4;
            break;
        }
        case Opcode::SFD: {
            
            uint32_t offset = registers[r2];
            unsigned char obj_type = *(memspace.data() + offset);

            const auto& field = class_descriptions[obj_type].fields[imm16];
            if (field.type == FieldType::INT32) {
                *(int32_t*)(memspace.data() + offset + 1 + imm16 * sizeof(int32_t)) = registers[r1];
            }
            else {
                *(uint32_t*)(memspace.data() + offset + 1 + imm16 * sizeof(uint32_t)) = (uint32_t)registers[r1];
            }

            registers[static_cast<int>(Register::PC)] += 4;
            break;
        }
        case Opcode::GFD: {
            
            uint32_t offset = registers[r2];
            unsigned char obj_type = *(memspace.data() + offset);

            const auto& field = class_descriptions[obj_type].fields[imm16];
            if (field.type == FieldType::INT32) {
                registers[r1] = *(int32_t*)(memspace.data() + offset + 1 + imm16 * sizeof(int32_t));
            }
            else {
                registers[r1] = *(uint32_t*)(memspace.data() + offset + 1 + imm16 * sizeof(uint32_t));
            }

            registers[static_cast<int>(Register::PC)] += 4;
            break;
        }
        case Opcode::GOA: {
            registers[static_cast<int>(Register::R1)] = *(uint32_t*)(memspace.data() + registers[r2] + sizeof(uint32_t) + registers[r1] * sizeof(uint32_t));
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        }
        case Opcode::SOA: {
            *(uint32_t*)(memspace.data() + registers[static_cast<int>(Register::R1)] + sizeof(uint32_t) + registers[r2] * sizeof(uint32_t)) = registers[r1];
            registers[static_cast<int>(Register::PC)] += 4;
            break;
        }
        case Opcode::CLM: {
            uint32_t offset = registers[static_cast<int>(Register::R1)];//registers[r1];
            unsigned char obj_type = *(memspace.data() + offset);

            // calling ctor
            fr.source_pc = registers[static_cast<int>(Register::PC)] + 4;
            memcpy(fr.saved_registers, registers.data() + 2 * sizeof(int32_t), 12 * sizeof(int32_t));
            memcpy(fr.saved_types, reg_types.data() + 2 * sizeof(uint32_t), 12 * sizeof(uint32_t));
            memcpy(memspace.data() + registers[static_cast<int>(Register::FP)], &fr, sizeof(Frame));

            registers[static_cast<int>(Register::FP)] += sizeof(Frame);
            registers[static_cast<int>(Register::PC)] = class_descriptions[obj_type].methods_addrs[imm21];
            break;
        }

    }
}