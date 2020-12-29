#ifndef BRFK_UTILS
#define BRFK_UTILS

// built-in
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdint>
#include <list>
#include <sstream>

class Color
{
    public:

        enum
        {
            FG_DEFAULT       = 39,
            FG_BLACK         = 30, 
            FG_RED           = 31, 
            FG_GREEN         = 32, 
            FG_YELLOW        = 33, 
            FG_BLUE          = 34,
            FG_MAGENTA       = 35, 
            FG_CYAN          = 36, 
            FG_LIGHT_GRAY    = 37, 
            FG_DARK_GRAY     = 90, 
            FG_LIGHT_RED     = 91, 
            FG_LIGHT_GREEN   = 92, 
            FG_LIGHT_YELLOW  = 93, 
            FG_LIGHT_BLUE    = 94, 
            FG_LIGHT_MAGENTA = 95, 
            FG_LIGHT_CYAN    = 96, 
            FG_WHITE         = 97, 
            
            BG_RED = 41, 
            BG_GREEN = 42, 
            BG_BLUE = 44, 
            BG_DEFAULT = 49
        };

        static std::string get_color(uint32_t md)
        {
            std::string out {"\033["};
            out.append(std::to_string(md));
            out.append("m");
            return out;
        } 
};

void panic(const char* const message)
{
    std::cout << Color::get_color(Color::FG_LIGHT_RED)
              << "[PANIC]: " 
              << message 
              << Color::get_color(Color::FG_DEFAULT) 
              << std::endl;
    
    exit(1);
}


struct Error
{
    enum class ErrorType
    {
        Error,
        Warning
    };

    ErrorType type;
    std::string message;
    uint32_t line;
    uint32_t collum;

    Error(ErrorType type, std::string message, uint32_t line, uint32_t collum)
    :   type(type), message(message), line(line), collum(collum)
    {

    }

    std::string to_string() 
    {

        uint32_t color;
        const char* box_str = nullptr;

        switch (this->type)
        {
            case ErrorType::Error:
            {
                color = Color::FG_LIGHT_RED;
                box_str = "ERROR";
                break;
            }
            case ErrorType::Warning:
            {
                color = Color::FG_LIGHT_MAGENTA;
                box_str = "WARNING";
                break;
            }
        }


        std::stringstream out;

        out << Color::get_color(color);
        out << '[' << box_str << ']';
        out << "[ln: " << this->line << ", col: " << this->collum << "] -> " << this->message;
        out << Color::get_color(Color::FG_DEFAULT);

        return out.str();

    }
};


class ErrorHandler
{
    private:
        bool& had_error;
        std::list<Error> errors;

    public:

        ErrorHandler(bool& had_error): had_error(had_error)
        {

        }

        void add_event(Error::ErrorType type, std::string message, uint32_t line, uint32_t collum)
        {
            this->errors.push_back(Error {type, std::string{message}, line, collum});
        }

        void add_error(std::string message, uint32_t line, uint32_t collum)
        {
            this->add_event(Error::ErrorType::Error, message, line, collum);
            this->had_error = true;
        }
        void add_warning(std::string message, uint32_t line, uint32_t collum)
        {
            this->add_event(Error::ErrorType::Warning, message, line, collum);
        }

        void flush()
        {
            if (this->errors.size() == 0)
                return;
                
            for (Error& err: this->errors)
            {
                std::cout << err.to_string() << "\n";
            }
            std::cout << std::flush;
            this->errors.clear();
        }

};


#endif