#ifndef BRFK_OPERATIONS
#define BRFK_OPERATIONS

// local
#include "utils.hpp"
#include "tokens.hpp"

template <typename T>
inline void write_to_program(uint8_t* program, uint32_t& idx, T value)
{
    static_assert(std::is_integral<T>::value);

    for (int8_t i = 0; i < (int32_t)sizeof(T); i++)
        program[idx + i] = value >> ((sizeof(T) - 1 - i) * 8);

    idx += sizeof(T);
}


class Operation
{
    public:

        uint16_t byte_idx;
        OperationType type;

        Operation(uint16_t bi): byte_idx(bi)
        {

        }

        virtual void serialize(uint8_t*, uint32_t&) = 0;
        virtual std::string repr() = 0;
        virtual ~Operation() = default;
};

class AddMem: public Operation
{
    public:

        static const uint8_t size = 3;

        int16_t value;

        AddMem(uint16_t bi, int16_t v): Operation(bi), value(v)
        {
            this->type = OperationType::ADD_MEM;
        }

        void serialize(uint8_t* prog, uint32_t& idx) override
        {
            write_to_program(prog, idx, (uint8_t)InstructionSet::ADD_MEM);
            write_to_program(prog, idx, this->value);
        }

        std::string repr() override
        {
            std::ostringstream out;
            out << "AddMEM value: ";
            out << this->value;

            return out.str();
        }

        ~AddMem() = default;
};

class AddMPTR: public Operation
{
    public:

        static const uint8_t size = 3;

        int16_t value;

        AddMPTR(uint16_t bi, int16_t v): Operation(bi), value(v)
        {
            this->type = OperationType::ADD_MPTR;            
        }

        void serialize(uint8_t* prog, uint32_t& idx) override
        {
            write_to_program(prog, idx, (uint8_t)InstructionSet::ADD_MP);
            write_to_program(prog, idx, this->value);
        }
        std::string repr() override
        {
            std::ostringstream out;
            out << "AddMPTR value: ";
            out << this->value;
            return out.str();
        }

        ~AddMPTR() = default;
};

class Loop: public Operation
{
    public:

        static const uint8_t size = 4;

        uint8_t comp_value;
        uint16_t jump_destination;

        Loop(uint16_t bi, uint8_t cmpv, uint16_t dest)
        : Operation(bi), comp_value(cmpv), jump_destination(dest)
        {
            this->type = OperationType::LOOP;
        }

        void serialize(uint8_t* prog, uint32_t& idx) override
        {
            if (this->jump_destination > this->byte_idx) // caso seja um loop direito
                write_to_program(prog, idx, (uint8_t)InstructionSet::JUMP_IF_EQ);
            else // caso seja um esquerdo
                write_to_program(prog, idx, (uint8_t)InstructionSet::JUMP_IF_DIFF);

            write_to_program(prog, idx, this->comp_value);
            write_to_program(prog, idx, this->jump_destination);
        }

        std::string repr() override
        {
            std::ostringstream out;
            out << "Loop cmp_value: ";
            out << (int)this->comp_value;
            out << ", destination: ";
            out << this->jump_destination;

            return out.str();
        }

        Loop* make_pair(uint16_t byte_idx)
        {
            Loop* other = new Loop{byte_idx, this->comp_value, this->byte_idx};
            this->jump_destination = byte_idx + this->size;
            return other;
        }

        ~Loop() = default;
};

class Print: public Operation
{
    public:

        static const uint8_t size = 1;
        bool ascii = false;

        Print(uint16_t bi): Operation(bi)
        {
            this->type = OperationType::PRINT;
        }

        void serialize(uint8_t* prog, uint32_t& idx) override
        {
            if (this->ascii)
                write_to_program(prog, idx, (uint8_t)InstructionSet::PRINT_ASCII);
            else
                write_to_program(prog, idx, (uint8_t)InstructionSet::PRINT_NUM);

        }
        
        std::string repr() override
        {
            std::stringstream out;
            out << "PRINT ";
            out << "ASCII: ";
            out << ((this->ascii) ? "TRUE" : "FALSE");

            return out.str();
        }

        ~Print() = default;
};

class Read: public Operation
{
    public:

        static const uint8_t size = 1;
        bool ascii = false;

        Read(uint16_t bi): Operation(bi)
        {
            this->type = OperationType::READ;
        }

        void serialize(uint8_t* prog, uint32_t& idx) override
        {
            if (this->ascii)
                write_to_program(prog, idx, (uint8_t)InstructionSet::READ_CHAR);
            else
                write_to_program(prog, idx, (uint8_t)InstructionSet::READ_NUM);

        }

        std::string repr() override
        {
            std::stringstream out;
            out << "READ ";
            out << "ASCII: ";
            out << ((this->ascii) ? "TRUE" : "FALSE");

            return out.str();
        }

        ~Read() = default;
};

class Flush: public Operation
{
    public:

        static const uint8_t size = 1;

        Flush(uint16_t bi): Operation(bi)
        {

        }

        void serialize(uint8_t* prog, uint32_t& idx) override
        {
            write_to_program(prog, idx, (uint8_t)InstructionSet::FLUSH);
        }

        std::string repr() override
        {
            return "FLUSH";
        }


        ~Flush() = default;
};

struct PsrOperation
{
    Operation* oprt;

    const Token* init;
    const Token* end;

    ~PsrOperation()
    {
        delete this->oprt;
    }
};


#endif