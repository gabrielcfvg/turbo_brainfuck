#ifndef BRFK_TOKENS
#define BRFK_TOKENS


enum class TokenType
{
    // atualizar 'token_type_repr' ao alterar!!!

    NUMBER,

    ADD_MPTR,
    ADD_MEM,
    PRINT,
    READ,
    LOOP_LEFT,
    LOOP_RIGHT,
    FLUSH,
    ASCII,
    NUMERIC,
    lEOF
};


static const char* token_type_repr[] =
{
    "NUMBER",

    "ADD_MPRT",
    "ADD_MEM",
    "PRINT",
    "READ",
    "LOOP_LEFT",
    "LOOP_RIGHT",
    "FLUSH",
    "ASCII",
    "NUMERIC",
    "lEOF"
};


struct Token
{
    TokenType oprt;
    std::string lexeme;
    uint32_t line, collum;
    uint32_t len;
};


// class Operation;
//
// struct PsrOperation
// {
//     Operation* oprt;
//
//     const Token* init;
//     const Token* end;
//
//     ~PsrOperation()
//     {
//         delete this->oprt;
//     }
// };


enum class OperationType
{
    ADD_MEM,
    ADD_MPTR,
    LOOP,
    PRINT,
    READ,
    FLUSH
};


enum class InstructionSet
{
    ADD_MEM,         // tamanho: 3 bytes, params: int16
    ADD_MP,          // tamanho: 3 bytes, params: int16
    JUMP,            // tamanho: 3 bytes, params: uint16
    JUMP_IF_EQ,      // tamanho: 4 bytes, params: uint8, uint16
    JUMP_IF_DIFF,    // tamanho: 4 bytes, params: uint8, uint16
    ASSIGN_MEM,      // tamanho: 2 bytes, params: uint8
    ASSIGN_MP,       // tamanho: 3 bytes, params: uint16
    READ_CHAR,       // tamanho: 1 byte
    READ_NUM,        // tamanho: 1 byte
    PRINT_NUM,       // tamanho: 1 byte
    PRINT_ASCII,     // tamanho: 1 byte
    FLUSH,           // tamanho: 1 byte
    END              // tamanho: 1 byte
};

#endif