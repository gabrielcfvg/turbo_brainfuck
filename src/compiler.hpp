#ifndef BRFK_COMPILER
#define BRFK_COMPILER

// built-in
#include <vector>
#include <cctype>
#include <array>
#include <stack>
#include <cassert>
#include <optional>

// local
#include "utils.hpp"
#include "operations.hpp"
#include "tokens.hpp"


class Lexer
{
    private:

        ErrorHandler& error_handler;
        std::string source;
        std::vector<Token>* output_ptr;


        uint32_t idx;
        uint32_t line;
        uint32_t collum;
        uint32_t source_size;

    public:

        Lexer(std::string src, ErrorHandler& eh): error_handler(eh), source(src)
        {
            this->source_size = this->source.size();
        }

        [[nodiscard]]
        std::vector<Token> lex()
        {
            std::vector<Token> output;
            output.reserve(this->source_size);
            this->output_ptr = &output;

            this->idx = 0;
            this->line = 1;
            this->collum = 1;
            
            while (!this->is_at_end())
                this->process_token();

            this->push_token(TokenType::lEOF, std::string{}, 0);
            return output;
        }

    private:

        void process_token()
        {
            char ch = this->currchar();

            switch (ch)
            {
            
                case ' ':
                case '\r':
                case '\t':
                {
                    this->advance();
                    break;
                }
                
                case '\n':
                {
                    this->line++;
                    this->collum = 1;
                    this->idx++;
                    break;
                }
                
                case '/':
                {
                    this->evaluate_comment();
                    break;
                }

                case '>':
                case '<':
                case '+':
                case '-':
                case '[':
                case ']':
                case '.':
                case ',':
                case 'f':
                case 'a':
                case 'n':
                {
                    this->single_char_token_evaluate(ch);
                    break;
                }

                default:
                    
                    if (isdigit(ch))
                        this->evaluate_number();

                    else
                    {
                        this->error_handler.add_error("invalid char", this->line, this->collum);
                        this->advance();
                    }
                    break;                        
            }
        }


        void evaluate_comment()
        {
            this->advance();
            if (!this->is_at_end())
            {
                if (this->match('/', 0))
                {
                    this->advance();
                    while (!this->is_at_end() && !this->match('\n', 0))
                        this->advance();
                    return;
                }
                else if (this->match('*', 0))
                {
                    this->advance();
                    while (!this->is_at_end(1) && !(this->match('*', 0) && this->match('/', 1)))
                    {
                        if (this->match('\n', 0))
                        {
                            this->line++;
                            this->collum = 1;
                            this->idx++;
                        }
                        else
                            this->advance();
                    }

                    if (this->is_at_end(1))
                        this->error_handler.add_warning(
                                "end of file with multi-line comment open",
                                this->line,
                                this->collum
                        );

                    this->advance<2>();
                    return;
                }
            }
            
            // caso seja o fim do arquivo ou a barra não é seguida de um '/' ou '*'
            this->error_handler.add_error("invalid char", this->line, this->collum - 1);
            this->advance();
        }

        void single_char_token_evaluate(char ch)
        {
            static const std::array<TokenType, 128> token_map = Lexer::init_single_char_token_map();
            this->push_token(token_map[ch], {ch}, 1);
            this->advance();
        }

        void evaluate_number()
        {
            uint32_t start = this->idx;

            while (!this->is_at_end() && isdigit(this->currchar()))
                this->advance();

            std::string lexeme {this->source.begin() + start, this->source.begin() + this->idx};
            
            Token tk {
                TokenType::NUMBER,
                lexeme, this->line,
                this->collum - (uint32_t)lexeme.size(),
                (uint32_t)lexeme.size()
            };

            this->output_ptr->push_back(tk);
        }


        // #################################################
        // #                                               #
        // #                     UTI                       #
        // #                                               #
        // #################################################


        [[nodiscard]]
        inline char currchar(int32_t offset = 0) const
        {
            return this->source[this->idx + offset];
        }
        
        [[nodiscard]]
        inline bool match(char ch, int32_t offset = 0) const
        {
            return this->currchar(offset) == ch;
        }

        [[nodiscard]]
        inline bool is_at_end(int32_t offset = 0) const
        {
            return this->idx + offset >= this->source_size;
        }

        template <uint32_t C = 1>
        inline void advance()
        {
            for (uint32_t i = 0; i < C; i++)
            {
                this->idx++;
                this->collum++;
            }
        }

        inline void push_token(TokenType type, std::string lexeme, uint32_t len)
        {
            Token tk {type, lexeme, this->line, this->collum, len};
            this->output_ptr->push_back(tk);
        }


        [[nodiscard]]
        std::array<TokenType, 128> init_single_char_token_map() const
        {
            std::array<TokenType, 128> token_map;

            token_map['>'] = TokenType::ADD_MPTR;
            token_map['<'] = TokenType::ADD_MPTR;
            token_map['+'] = TokenType::ADD_MEM;
            token_map['-'] = TokenType::ADD_MEM;
            token_map['['] = TokenType::LOOP_LEFT;
            token_map[']'] = TokenType::LOOP_RIGHT;
            token_map['.'] = TokenType::PRINT;
            token_map[','] = TokenType::READ;
            token_map['f'] = TokenType::FLUSH;
            token_map['a'] = TokenType::ASCII;
            token_map['n'] = TokenType::NUMERIC;

            return token_map;
        }

};


class Parser
{
    private:
        
        std::vector<PsrOperation*>* output_ptr = nullptr;
        const std::vector<Token>& token_input;
        ErrorHandler& error_handler;
        std::stack<PsrOperation*>* loop_stack;

        bool ascii_default;
        uint32_t idx;
        bool has_flush;

        const uint32_t input_size;

    public:

        uint16_t byte_idx;

        Parser(const std::vector<Token>& ti, ErrorHandler& eh, bool ad)
        : token_input(ti), error_handler(eh), ascii_default(ad), input_size(ti.size())
        {

        }

        [[nodiscard]]
        std::vector<PsrOperation*> parse()
        {
            std::vector<PsrOperation*> output;
            output.reserve(this->token_input.size());
            this->output_ptr = &output;

            std::stack<PsrOperation*> loop_stack;
            this->loop_stack = &loop_stack;

            this->idx = 0;
            this->byte_idx = 0;

            this->find_flush();
            this->process_operations();

            return output;
        }

    private:


        void find_flush()
        {
            for (const Token& oprt: this->token_input)
            {
                if (oprt.oprt == TokenType::FLUSH)
                {
                    this->has_flush = true;
                    return;
                }
            }
            this->has_flush = false;
        }

        void process_operations()
        {

            // não é necessário verificar se está no fim
            // antes de usar 'currtoken(>0)' ou 'match(..., >0)'
            // visto que o Lexer vai sempre colocar um 'TokenType::lEOF' no final

            while (this->idx < this->input_size)
            {

                const Token& tkn = this->currtoken();

                switch (tkn.oprt)
                {
                    case (TokenType::NUMBER):
                    {
                        this->error_handler.add_error("unexpected number", tkn.line, tkn.collum);
                        this->idx++;

                        break;
                    }

                    case (TokenType::ADD_MEM):
                    case (TokenType::ADD_MPTR):
                    {

                        TokenType type = tkn.oprt;

                        int32_t value = 0;
                        const Token* end;

                        do
                        {
                            const Token* itkn = &this->currtoken();

                            int32_t ivalue = 1;
                            end = itkn;

                            if (this->match(TokenType::NUMBER, 1))
                            {
                                this->idx++;
                                const Token& nxt = this->currtoken();
                                end = &nxt;
                                ivalue = std::stoi(nxt.lexeme);
                            }

                            assert(itkn->lexeme.length() == 1);
                            if (itkn->lexeme.at(0) == ((type == TokenType::ADD_MEM) ? '-' : '<'))
                                ivalue = -ivalue;

                            value += ivalue;
                            this->idx++;
                        }
                        while (this->match(type, 0));


                        if (value != 0)
                        {

                            if (value > UINT8_MAX)
                                this->error_handler.add_warning("overflow possibility", tkn.line, tkn.collum);

                            Operation* admm;
                            if (type == TokenType::ADD_MEM)
                            {
                                admm = new AddMem{this->byte_idx, (int16_t)value};
                                byte_idx += static_cast<AddMem*>(admm)->size;
                            }
                            else if (type == TokenType::ADD_MPTR)
                            {
                                admm = new AddMPTR{this->byte_idx, (int16_t)value};
                                byte_idx += static_cast<AddMPTR*>(admm)->size;
                            }
                            else
                                panic("unexpected type");

                            PsrOperation* noprt = new PsrOperation{};
                            noprt->init = &tkn;
                            noprt->end = end;
                            noprt->oprt = admm;

                            this->output_ptr->push_back(noprt);
                        }

                        // não é necessário incrementar o indexador visto que isso já é feito dentro do loop
                        break;
                    }

                    case (TokenType::PRINT):
                    {
                        uint32_t init_idx = this->output_ptr->size();
                        uint32_t end_idx = init_idx - 1;

                        do
                        {
                            Print* print = new Print{this->byte_idx};
                            PsrOperation* noprt = new PsrOperation{};
                            noprt->init = &tkn;
                            noprt->end = &tkn;
                            noprt->oprt = print;

                            if (this->ascii_default)
                                print->ascii = true;

                            this->output_ptr->push_back(noprt);
                            this->byte_idx += print->size;
                            this->idx++;
                            end_idx++;
                        }
                        while (this->match(TokenType::PRINT, 0));

                        if (!this->has_flush)
                        {
                            Flush* flush = new Flush{this->byte_idx};
                            PsrOperation* noprt = new PsrOperation{};
                            noprt->init = this->output_ptr->at(init_idx)->init;
                            noprt->end = this->output_ptr->at(end_idx)->end;
                            noprt->oprt = flush;

                            this->output_ptr->push_back(noprt);
                            this->byte_idx += flush->size;
                        }

                        if (!this->ascii_default && this->match(TokenType::ASCII, 0))
                        {
                            for (uint32_t i = init_idx; i <= end_idx; i++)
                                static_cast<Print*>(this->output_ptr->at(i)->oprt)->ascii = true;
                            this->idx++;
                        }

                        else if (this->ascii_default && this->match(TokenType::NUMERIC, 0))
                        {
                            for (uint32_t i = init_idx; i <= end_idx; i++)
                                static_cast<Print*>(this->output_ptr->at(i)->oprt)->ascii = false;
                            this->idx++;
                        }

                        break;
                    }

                    case (TokenType::READ):
                    {
                        uint32_t init_idx = this->output_ptr->size();
                        uint32_t end_idx = init_idx - 1;

                        do
                        {
                            Read* read = new Read{this->byte_idx};
                            PsrOperation* noprt = new PsrOperation{};
                            noprt->init = &tkn;
                            noprt->end = &tkn;
                            noprt->oprt = read;

                            if (this->ascii_default)
                                read->ascii = true;

                            this->output_ptr->push_back(noprt);
                            this->byte_idx += read->size;
                            this->idx++;
                            end_idx++;
                        }
                        while (this->match(TokenType::READ, 0));

                        if (!this->ascii_default && this->match(TokenType::ASCII, 0))
                        {
                            for (uint32_t i = init_idx; i <= end_idx; i++)
                                static_cast<Read*>(this->output_ptr->at(i)->oprt)->ascii = true;
                            this->idx++;
                        }

                        else if (this->ascii_default && this->match(TokenType::NUMERIC, 0))
                        {
                            for (uint32_t i = init_idx; i <= end_idx; i++)
                                static_cast<Read*>(this->output_ptr->at(i)->oprt)->ascii = false;
                            this->idx++;
                        }

                        break;
                    }

                    case (TokenType::LOOP_LEFT):
                    {
                        Loop* loop = new Loop{this->byte_idx, 0, 0};
                        PsrOperation* noprt = new PsrOperation{};
                        noprt->init = &tkn;
                        noprt->end = &tkn;
                        noprt->oprt = loop;

                        if (this->match(TokenType::NUMBER, 1))
                        {
                            this->idx++;
                            const Token& nxt = this->currtoken();
                            noprt->end = &nxt;
                            loop->comp_value = std::stoi(nxt.lexeme);
                        }

                        this->loop_stack->push(noprt);
                        this->output_ptr->push_back(noprt);
                        this->byte_idx += loop->size;
                        this->idx++;

                        break;
                    }
                    
                    case (TokenType::LOOP_RIGHT):
                    {
                        if (this->loop_stack->size() == 0)
                        {
                            this->error_handler.add_error("']' matchless", tkn.line, tkn.collum);
                            this->idx++;

                            // goto next;
                            break;
                        }

                        Loop* loop = static_cast<Loop*>(this->loop_stack->top()->oprt)->make_pair(this->byte_idx);
                        this->loop_stack->pop();

                        PsrOperation* noprt = new PsrOperation{};
                        noprt->init = &tkn;
                        noprt->end = &tkn;
                        noprt->oprt = loop;

                        this->output_ptr->push_back(noprt);
                        this->byte_idx += loop->size;
                        this->idx++;

                        break;
                    }

                    case (TokenType::FLUSH):
                    {
                        Flush* flush = new Flush{this->byte_idx};
                        PsrOperation* noprt = new PsrOperation{};
                        noprt->init = &tkn;
                        noprt->end = &tkn;
                        noprt->oprt = flush;

                        this->output_ptr->push_back(noprt);
                        this->byte_idx += flush->size;
                        this->idx++;

                        break;
                    }

                    case (TokenType::ASCII):
                    {
                        this->error_handler.add_error("unexpected ASCII identifier ('a')", tkn.line, tkn.collum);
                        this->idx++;

                        break;
                    }

                    case (TokenType::NUMERIC):
                    {
                        this->error_handler.add_error("unexpected numeric identifier ('n')", tkn.line, tkn.collum);
                        this->idx++;

                        break;
                    }

                    case (TokenType::lEOF):
                    {
                        
                        while (this->loop_stack->size() > 0)
                        {
                            PsrOperation* rem = this->loop_stack->top();
                            this->loop_stack->pop();
                            this->error_handler.add_error("'[' matchless", rem->init->line, rem->init->collum);
                        }

                        this->idx++;
                    }
                }
            }
        }


        // #################################################
        // #                                               #
        // #                     UTI                       #
        // #                                               #
        // #################################################

        [[nodiscard]]
        inline const Token& currtoken(int32_t offset = 0)
        {
            return this->token_input.at(this->idx + offset);
        }

        [[nodiscard]]
        inline bool match(TokenType type, int32_t offset = 0)
        {
            return this->currtoken(offset).oprt == type;
        }

};


struct Program
{
    uint8_t* program;
    uint32_t size;
};

std::optional<Program> compile(std::string source_code, bool insert_end, bool ascii_default)
{
    bool error = false;
    ErrorHandler eh {error};

    Lexer lex {source_code, eh};
    std::vector<Token> lres = lex.lex();

    if (error)
    {
        eh.flush();
        return {};
    }

    Parser par = Parser{lres, eh, ascii_default};
    std::vector<PsrOperation*> pres = par.parse();

    if (error)
    {
        eh.flush();
        return {};
    }
    // else
    // {
    //     for (PsrOperation* oprt: pres)
    //         std::cout << oprt->oprt->repr() << std::endl;
    // }

    uint8_t* program = new uint8_t[par.byte_idx + 1];
    uint32_t idx = 0;

    for (PsrOperation* oprt: pres)
        oprt->oprt->serialize(program, idx);

    if (insert_end)
        write_to_program(program, idx, (uint8_t)InstructionSet::END);

    for (PsrOperation* oprt: pres)
        delete oprt;

    return {Program{program, (uint32_t)(par.byte_idx + 1)}};
}

void create_binary(const Program& prog, const char* const path, bool has_end)
{
    std::ofstream file {path, std::ios::out | std::ios::binary};

    if (file.bad() || !file.is_open())
        panic("error creating file");

    file.write("brfk", 4);
    file.put(0x00);
    file.write((const char*)prog.program, prog.size);
    if (!has_end)
        file.put((char)InstructionSet::END);

    file.close();
}



#endif