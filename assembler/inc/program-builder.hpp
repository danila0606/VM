#pragma once

#include <vector>

#include "token-list.hpp"

#include "rest.hpp"

enum class SymbolType {
    SYM_LABEL
};

enum class Section {
    DATA,
    TEXT
};

struct Symbol {
    SymbolType type;
    Section section;
    std::string name;
    unsigned int address;
};

size_t hash(const char *str) {
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

struct SymTable {
    std::vector<std::shared_ptr<Symbol>> data;
    int len = 0;

    std::shared_ptr<Symbol> lookup_symbol(std::string str) {
        size_t h_idx = hash(str.c_str()) % len;
        if (data[h_idx]) {
            if (data[h_idx]->name == str) {
                return data[h_idx];
            }
        }
        for (int i = h_idx; i < len; ++i) {
            if (data[i]) {
                if (data[i]->name == str) {
                    return data[i];
                }
            }
        }
        for (int i = 0; i < h_idx; ++i) {
            if (data[i]) {
                if (data[i]->name == str) {
                    return data[i];
                }
            }
        }
        return nullptr;
    }
};

struct ProgramInfo {
    size_t data_len;
    size_t data_idx;
    std::vector<unsigned char> data;

    size_t text_len;
    size_t text_idx;
    std::vector<unsigned char> text;

    Section current_section = Section::TEXT;
    SymTable sym_table;

    ProgramInfo(size_t new_sym_table_len) {
        data_idx = 0;
        data_len = 128;
        data = std::vector<unsigned char>(data_len);

        text_idx = 0;
        text_len = 128;
        text = std::vector<unsigned char>(text_len);

        sym_table.len = new_sym_table_len;
        sym_table.data = std::vector<std::shared_ptr<Symbol>>(new_sym_table_len);
    }
};

int get_section_offset(std::shared_ptr<ProgramInfo> prog_info) {
    if (prog_info->current_section == Section::TEXT) {
        return TEXT_SECTION_START;
    }
    if (prog_info->current_section == Section::DATA) {
        return DATA_SECTION_START;
    }
    return 0;
}

std::shared_ptr<Symbol> insert_symbol(std::shared_ptr<ProgramInfo> prog_info, std::shared_ptr<Token> token) {
    auto len = prog_info->sym_table.len;
    size_t index = hash(token->str.c_str()) % len;

    std::shared_ptr<Symbol> symbol = nullptr;

    if ((symbol = prog_info->sym_table.lookup_symbol(token->str))) { 
        return symbol;
    }

    symbol = std::make_shared<Symbol>();
    symbol->section = prog_info->current_section;
    symbol->name = token->str;
    if (prog_info->current_section == Section::TEXT) {
        symbol->address = prog_info->text_idx + get_section_offset(prog_info);
    }
    else if (prog_info->current_section == Section::DATA) {
        symbol->address = prog_info->data_idx + get_section_offset(prog_info);
    }

    for (size_t i = index; i < len ; ++i) {
        if (!prog_info->sym_table.data[i]) {
            prog_info->sym_table.data[i] = symbol;
            return symbol;
        }
    }
    for (size_t i = 0; i < index; ++i) {
        if (!prog_info->sym_table.data[i]) {
            prog_info->sym_table.data[i] = symbol;
            return symbol;
        }
    }
    return nullptr;
}

bool is_pc_relative(Opcode op) {
    return op == Opcode::JZS || op == Opcode::JS;
}

int lookup_label_offset(std::shared_ptr<ProgramInfo> prog_info, Opcode opcode, std::shared_ptr<Token> token, bool &passfail) {
    std::shared_ptr<Symbol> symbol = prog_info->sym_table.lookup_symbol(token->str);
    int offset = -1;
    if (!symbol) {
        throw std::runtime_error("Label reference not found" + std::to_string(token->line_num));
    } else if (is_pc_relative(opcode)) {
        if (prog_info->current_section == Section::DATA) {
            passfail = false;
            return symbol->address - prog_info->data_idx - DATA_SECTION_START;
        }
        else if (prog_info->current_section == Section::TEXT) {
            passfail = false;
            return symbol->address - prog_info->text_idx - TEXT_SECTION_START;
        }
    } else {
        passfail = false;
        return symbol->address;
    }
    passfail = true;
    return offset;
}

void insert_text_or_data(std::shared_ptr<ProgramInfo> prog_info, uint8_t* thing, size_t thing_size) {
    if (prog_info->current_section == Section::DATA) {

        while (prog_info->data_idx + thing_size > prog_info->data_len) {
            prog_info->data_len *= 2;
            prog_info->data.resize(prog_info->data_len);
        }

        
        memcpy(prog_info->data.data() + prog_info->data_idx, thing, thing_size);
        prog_info->data_idx += thing_size;

    } else if (prog_info->current_section == Section::TEXT) {

        while (prog_info->text_idx + thing_size >= prog_info->text_len) {
            prog_info->text_len *= 2;
            prog_info->text.resize(prog_info->text_len);
        }

        memcpy(prog_info->text.data() + prog_info->text_idx, thing, thing_size);
        prog_info->text_idx += thing_size;
    }
}

int compile_new_label(std::shared_ptr<ProgramInfo> prog_info, std::shared_ptr<Token>& token) {
    if (prog_info->sym_table.lookup_symbol(token->str)) {
        throw std::runtime_error("Label already defined" + std::to_string(token->line_num));
    }
    auto s = insert_symbol(prog_info, token);
    token = token->next;
    return s->address;
}

int compile_directive(std::shared_ptr<ProgramInfo> prog_info, std::shared_ptr<Token>& token) {
    Immediate imm = 0;

    if (token->str == ".data") {
        prog_info->current_section = Section::DATA;

    } else if (token->str == ".text") {
        prog_info->current_section = Section::TEXT;

    } else if  (token->str == ".i32") {
        token = token->next;
        compile_new_label(prog_info, token);
        imm = std::get<Immediate>(token->internal);
        auto bytes = int_to_bytes(imm);
        insert_text_or_data(prog_info, bytes.data(), bytes.size());

    } else if (token->str == ".asciiz") {
        token = token->next;
        compile_new_label(prog_info, token);
        auto bytes = string_to_bytes(token->str);
        insert_text_or_data(prog_info, bytes.data(), bytes.size() - 1);

    // } else if (token->str, ".raw") {
    //     token = token->next;
    //     compile_new_label(prog_info, token);
    //     imm = std::get<Immediate>(token->internal);
    //     uint8_t* raw_data = new uint8_t[imm]();
    //     insert_text_or_data(prog_info, raw_data, imm);
    //     delete raw_data;

    } else {
        throw std::runtime_error("Unknown directive" + std::to_string(token->line_num));
    }

    token = token->next;
    return 0;
}

int compile_instruction(std::shared_ptr<ProgramInfo> prog_info, std::shared_ptr<Token>& token) {
    auto op = std::get<Opcode>(token->internal);
    int num_operands_reg = get_num_operands_reg(op);
    int num_operands_imm = get_num_operands_imm(op);
    int num_operands = num_operands_reg + num_operands_imm;
    std::shared_ptr<Instruction> instr = nullptr;
    
    bool passfail = false;
    int32_t operand1, operand2, operand3;

    switch(num_operands) {
        case 0:
            token = token->next;
            instr = std::make_shared<Instruction>(op);
            break;
        case 1:
            token = token->next;
            if (token->type == TokenType::TOKEN_TYPE_LABEL_REF) {
                operand1 = lookup_label_offset(prog_info, op, token, passfail);
            }
            else {
                if (num_operands_reg > 0) {
                    Register r = std::get<Register>(token->internal);
                    operand1 = static_cast<int32_t>(r);
                    --num_operands_reg;
                } else if (num_operands_imm > 0) {
                    Immediate imm = std::get<Immediate>(token->internal);
                    operand1 = imm;
                    --num_operands_imm;
                }   
            }
            instr = std::make_shared<Instruction>(op, operand1);
            token = token->next;
            break;
        case 2:
            token = token->next;

            if (num_operands_reg > 0) {
                Register r = std::get<Register>(token->internal);
                operand1 = static_cast<int32_t>(r);
                --num_operands_reg;
            } else if (num_operands_imm > 0) {
                Immediate imm = std::get<Immediate>(token->internal);
                operand1 = imm;
                --num_operands_imm;
            }   

            token = token->next;

            if (token->type == TokenType::TOKEN_TYPE_LABEL_REF) {
                operand2 = lookup_label_offset(prog_info, op, token, passfail);
            }
            else {
                if (num_operands_reg > 0) {
                    Register r = std::get<Register>(token->internal);
                    operand2 = static_cast<int32_t>(r);
                    --num_operands_reg;
                } else if (num_operands_imm > 0) {
                    Immediate imm = std::get<Immediate>(token->internal);
                    operand2 = imm;
                    --num_operands_imm;
                }   
            }
            instr = std::make_shared<Instruction>(op, operand1, operand2);
            token = token->next;
            break;
        case 3:
            token = token->next;

            if (num_operands_reg > 0) {
                Register r = std::get<Register>(token->internal);
                operand1 = static_cast<int32_t>(r);
                --num_operands_reg;
            } else if (num_operands_imm > 0) {
                Immediate imm = std::get<Immediate>(token->internal);
                operand1 = imm;
                --num_operands_imm;
            }

            token = token->next;

            if (num_operands_reg > 0) {
                Register r = std::get<Register>(token->internal);
                operand2 = static_cast<int32_t>(r);
                --num_operands_reg;
            } else if (num_operands_imm > 0) {
                Immediate imm = std::get<Immediate>(token->internal);
                operand2 = imm;
                --num_operands_imm;
            }

            token = token->next;

            if (std::holds_alternative<Register>(token->internal)) {
                Register r = std::get<Register>(token->internal);
                operand3 = static_cast<int32_t>(r);
                //--num_operands_reg; // <- not used
            } else if (std::holds_alternative<Immediate>(token->internal)) {
                Immediate imm = std::get<Immediate>(token->internal);
                operand3 = imm;
                //--num_operands_imm; // <- not used
            } else {
                // handle error
            }

            instr = std::make_shared<Instruction>(op, operand1, operand2, operand3);

            token = token->next;
            break;
    }

    if (!passfail) {
        passfail = (instr == nullptr);
        if (!passfail) {
            auto bytes = int_to_bytes(instr->assembled_value);
            insert_text_or_data(prog_info, bytes.data(), bytes.size());
        }
    }

    return passfail;
}

void generate_symbols(std::shared_ptr<ProgramInfo> prog_info, std::shared_ptr<TokenList> list) {
    auto token = list->head;
    int num_operands = 0;
    while (token) {
        if (token->type == TokenType::TOKEN_TYPE_DIRECTIVE) {
            compile_directive(prog_info, token);
            continue;
        }
        else if (token->type == TokenType::TOKEN_TYPE_NEW_LABEL) {
            compile_new_label(prog_info, token);
            continue;
        }
        else if (token->type == TokenType::TOKEN_TYPE_INSTR) {
            Opcode op = std::get<Opcode>(token->internal);
            num_operands = get_num_operands_reg(op) + get_num_operands_imm(op);
            switch (num_operands) {
                case 0:
                    break;
                case 1:
                    token = token->next;
                    break;
                case 2:
                    token = token->next->next;
                    break;
                case 3:
                    token = token->next->next->next;
                    break;
            }
            if (prog_info->current_section == Section::TEXT) {
                prog_info->text_idx += 4;
            }
        }
        else if (prog_info->current_section == Section::DATA) {
            /* TODO: Get size of data symbols. */
        }
        token = token->next;
    }
    prog_info->text_len = prog_info->text_idx;
    prog_info->text.resize(prog_info->text_len);

    prog_info->data_len = prog_info->data_idx;
    prog_info->data.resize(prog_info->data_len);

    prog_info->text_idx = 0;
    prog_info->data_idx = 0;
    prog_info->current_section = Section::TEXT;
}

int compile_token(std::shared_ptr<ProgramInfo> prog_info, std::shared_ptr<Token>& token) {
    switch (token->type) {
        default:
            throw std::runtime_error("Invalid token: " + token->str);
            token = token->next;
            return -1;
        case TokenType::TOKEN_TYPE_CHAR_LIT:
        case TokenType::TOKEN_TYPE_STRING_LIT:
        case TokenType::TOKEN_TYPE_NEW_LABEL:
        case TokenType::TOKEN_TYPE_DIRECTIVE:
        case TokenType::TOKEN_TYPE_IMMEDIATE:
            token = token->next;
            return 0;
        case TokenType::TOKEN_TYPE_INSTR:
            return compile_instruction(prog_info, token);
    }
}
