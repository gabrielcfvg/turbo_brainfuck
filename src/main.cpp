// built-in
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <bitset>

// local
#include "vm.hpp"
#include "operations.hpp"
#include "compiler.hpp"

// extern
#include "lib/CLI11.hpp"



void comp(const std::string& file_path, const std::string& output_path, bool ascii_default)
{
    std::ifstream file;

    if (!std::filesystem::exists({file_path}) || !std::filesystem::is_regular_file({file_path}))
    {
        panic("invalid or nonexistent file");
    }

    file.open(file_path, std::ifstream::in);

    if (file.bad() || !file.is_open())
    {
        panic("error opening file");
    }

    file.seekg(0, file.end);
    uint32_t file_size = file.tellg();
    file.seekg(0, file.beg);

    char* csfile = new char[file_size + 1];
    file.read(csfile, file_size);
    csfile[file_size] = 0;
    std::string sfile {std::move(csfile)};

    file.close();

    std::optional<Program> prog = compile(sfile, true, ascii_default);
    if (prog.has_value())
        create_binary(prog.value(), output_path.data(), true);
}

void run(const std::string& file_path, bool scompile, bool ascii_default)
{
    std::ifstream file;

    if (!std::filesystem::exists({file_path}) || !std::filesystem::is_regular_file({file_path}))
    {
        panic("invalid or nonexistent file");
    }

    file.open(file_path, std::ifstream::in);

    if (file.bad() || !file.is_open())
    {
        panic("error opening file");
    }

    file.seekg(0, file.end);
    uint32_t file_size = file.tellg();
    file.seekg(0, file.beg);

    char buffer[5];
    file.read(buffer, 4);

    if (strcmp("brfk", buffer) == 0 && file.peek() == 0x00)
    {
        file.seekg(1, file.cur);
        file_size -= 5;

        VirtualMachine vm;

        vm.program_size = file_size;
        vm.program = new uint8_t[file_size];
        file.read((char*)vm.program, file_size);
        file.close();

        vm.run();
    }
    else if (scompile)
    {
        std::cout << Color::get_color(Color::FG_LIGHT_MAGENTA)
                  << "compiling"
                  << Color::get_color(Color::FG_DEFAULT)
                  << std::endl;

        file.seekg(0, file.beg);

        char* csfile = new char[file_size + 1];
        file.read(csfile, file_size);
        csfile[file_size] = 0;
        file.close();

        std::string sfile {std::move(csfile)};
        std::optional<Program> oprog = compile(sfile, true, ascii_default);
        if (oprog.has_value())
        {
            Program prog = oprog.value();
            VirtualMachine vm;

            vm.program_size = prog.size;
            vm.program = prog.program;

            vm.run();
        }
    }
    else
    {
        panic("not a valid brainfuck binary");
    }

}


int main(int argc, char** argv)
{
    (void)token_type_repr;

    bool ascii_default = false;
    bool scompile = false;
    std::string file_path;
    std::string output_path;

    CLI::App app {"Turbo Brainfuck"};
    app.require_subcommand(1, 1);

    CLI::App* sub_run = app.add_subcommand("run","runs the program on the virtual machine");
    sub_run->add_option("file", file_path, "file to be runned")->required(true);
    sub_run->add_flag("-a, --ascii_default", ascii_default, "if a compilation is required, input and output are, by default, in ASCII mode, without the need to place the qualifier 'a'");
    sub_run->add_flag("-c, --compile", scompile, "tries to compile the file if it is not a valid brainfuck binary");

    sub_run->callback([&](){run(file_path, scompile, ascii_default);});

    CLI::App* sub_comp = app.add_subcommand("build","compiles the code file and produces a binary that can be run with the 'run' command");
    sub_comp->add_option("file", file_path, "file to be compiled")->required(true);
    sub_comp->add_option("-o, --output", output_path, "path where the binary will be placed")->default_val("a.out");
    sub_comp->add_flag("-a, --ascii_default", ascii_default, "input and output are by default in ASCII mode, without the need to place the qualifier 'a'");
    sub_comp->callback([&](){comp(file_path, output_path, ascii_default);});

    CLI11_PARSE(app, argc, argv);
}
