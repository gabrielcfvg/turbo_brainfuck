#ifndef BRFK_VM
#define BRFK_VM

// local
#include "utils.hpp"
#include <cstring>
#include "tokens.hpp"


struct VirtualMachine
{
    uint16_t pc = 0;
    uint16_t mp = 0;

    uint8_t* program;
    uint16_t program_size;

    uint8_t* mem;
    static const uint32_t mem_size = UINT16_MAX;

    std::string bstdout;
    std::string bstdin;

    VirtualMachine()
    {
        this->mem = new uint8_t[this->mem_size];
        this->clear_memory();
    }

    void run()
    {

        while (true)
        {

            InstructionSet inst = (InstructionSet)(this->read_program<uint8_t>()); 
        
            switch (inst)
            {
                case InstructionSet::ADD_MEM:
                {
                    this->mem[this->mp] += this->read_program<int16_t>();
                    break;
                }
                case InstructionSet::ADD_MP:
                {
                    this->mp += this->read_program<int16_t>();
                    break;
                }
                case InstructionSet::JUMP:
                {
                    uint16_t loc = this->read_program<uint16_t>();
                    this->pc = loc;
                    break;
                }
                case InstructionSet::JUMP_IF_EQ:
                {
                    uint8_t val = this->read_program<uint8_t>();
                    uint16_t loc = this->read_program<uint16_t>();
                    if (val == this->mem[this->mp])
                    {
                        this->pc = loc;
                    } 
                    break;
                }
                case InstructionSet::JUMP_IF_DIFF:
                {
                    uint8_t val = this->read_program<uint8_t>();
                    uint16_t loc = this->read_program<uint16_t>();
                    if (val != this->mem[this->mp])
                    {
                        this->pc = loc;
                    }
                    break;
                }
                case InstructionSet::ASSIGN_MEM:
                {
                    this->mem[this->mp] = this->read_program<uint8_t>();
                    break;
                }
                case InstructionSet::ASSIGN_MP:
                {
                    this->mp = this->read_program<uint16_t>();
                    break;
                }
                case InstructionSet::READ_CHAR:
                {
                    this->mem[this->mp] = this->read_ch();
                    break;
                }
                case InstructionSet::READ_NUM:
                {
                    this->mem[this->mp] = this->read_num();
                    break;
                }
                case InstructionSet::PRINT_NUM:
                {
                    this->bstdout.append(std::to_string(this->mem[this->mp]));
                    break;
                }
                case InstructionSet::PRINT_ASCII:
                {
                    this->bstdout.push_back(this->mem[this->mp]);
                    break;
                }
                case InstructionSet::FLUSH:
                {
                    std::cout << this->bstdout << std::flush;
                    this->bstdout.clear();
                    break;
                }
                case InstructionSet::END:
                {
                    goto fim;
                }

                default:
                {
                    panic(std::string {"non-existent instruction: "}.append(std::to_string((int)inst)).data());
                }

            }
        
        }
        fim:;

    }

    template <typename T>
    T read_program()
    {
        static_assert(std::is_integral<T>::value);

        T res = 0;
        for (uint8_t i = sizeof(T); i; i--)
            res |= this->program[this->pc++] << ((i - 1) * 8);
        return res;

        // uma alternativa mais curta seria:
        //      return *((T*)(this->program + this->pc));
        //
        // mas além de ser mais feio e gambiarrento, 
        // o código gerado tem o dobro de instruções
    }

    char read_ch()
    {
        init:;
        if (this->bstdin.length() > 0)
        {
            char ch = this->bstdin.front();
            this->bstdin.erase(0, 1);
            return ch;
        }
        else
        {
            std::cin >> this->bstdin;
            goto init;
        } 
    }

    uint8_t read_num()
    {
        init:;
        if (this->bstdin.length() > 0)
        {
            if (!isdigit(this->bstdin.front()))
                panic("value received by READ_NUM is not a number");

            size_t end;
            uint8_t number = std::stoul(this->bstdin, &end);
            this->bstdin.erase(0, end);
            return number;
        }
        else
        {
            std::cin >> this->bstdin;
            goto init;
        }
    }

    void clear_memory()
    {
        memset(this->mem, 0, this->mem_size);
    }
    
    void push_back_program(const uint8_t* new_part, uint32_t np_size)
    {
        uint8_t* np = new uint8_t[this->program_size + np_size + 1];
        np[this->program_size + np_size] = 0;

        memcpy(np, this->program, this->program_size);
        memcpy(np + this->program_size, new_part, np_size);

        delete[] this->program;

        this->program_size += np_size;
        this->program = np;
    }
};



#endif