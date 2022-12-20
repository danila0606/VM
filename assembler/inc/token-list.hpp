#pragma once

#include <memory>
#include <string>
#include <variant>

#include "opdefs.hpp"


enum class TokenType {
    TOKEN_TYPE_EOF = -1,        /* End of File */
    TOKEN_TYPE_UNKNOWN = 0,     /* Unknown token. */
    TOKEN_TYPE_PLUS,            /* '+' */
    TOKEN_TYPE_MINUS,           /* '-' */
    TOKEN_TYPE_STAR,            /* '*' */
    TOKEN_TYPE_FSLASH,          /* '/' */
    TOKEN_TYPE_COMMA,           /* ',' */
    TOKEN_TYPE_DOT,             /* '.' */
    TOKEN_TYPE_COLON,           /* ':' */
    TOKEN_TYPE_DOLLAR,          /* '$' */
    TOKEN_TYPE_LPAREN,          /* '(' */
    TOKEN_TYPE_RPAREN,          /* ')' */
    TOKEN_TYPE_IDENT,           /* Identifier */
    TOKEN_TYPE_INT_LIT,         /* Integer literal */
    TOKEN_TYPE_FLOAT_LIT,       /* Floating point literal */
    TOKEN_TYPE_STRING_LIT,      /* String literal */
    TOKEN_TYPE_CHAR_LIT,        /* Character literal */

    /* These token types are for the second pass list. */
    TOKEN_TYPE_NEW_LABEL,       /* 'label:' */
    TOKEN_TYPE_LABEL_REF,       /* 'label' */
    TOKEN_TYPE_DIRECTIVE,       /* '.directive' */
    TOKEN_TYPE_INSTR,           /* Instruction */
    TOKEN_TYPE_REGISTER,        /* Register */
    TOKEN_TYPE_IMMEDIATE,       /* Evaluated constant expression */
};

struct Token {
    std::shared_ptr<Token> prev;
    TokenType type;
    int line_num;
    std::string str;

    std::variant<Opcode, Register, Immediate> internal;
    std::shared_ptr<Token> next;

    Token(std::string new_str, int new_line_num, TokenType new_type, Opcode op) : internal(op) {
        prev = nullptr;
        line_num = new_line_num;
        type = new_type;
        str = new_str;
        next = nullptr;
    }

    Token(std::string new_str, int new_line_num, TokenType new_type, Register r) : internal(r) {
        prev = nullptr;
        line_num = new_line_num;
        type = new_type;
        str = new_str;
        next = nullptr;
    }

    Token(std::string new_str, int new_line_num, TokenType new_type, Immediate imm) : internal(imm) {
        prev = nullptr;
        line_num = new_line_num;
        type = new_type;
        str = new_str;
        next = nullptr;
    }
};

struct TokenList {
    std::shared_ptr<Token> head, tail;
    size_t length;

    TokenList() {
        head = nullptr;
        tail = nullptr;
        length = 0;
    }

    std::shared_ptr<Token> push_back_token(std::shared_ptr<Token> token) {
        ++length;
        if (!head) {
            head = token;
            tail = token;
        } else {
            tail->next = token;
            token->prev = tail;
            tail = token;
        }
        return token;
    }
};