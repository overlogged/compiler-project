#include "driver.h"
#include "syntax_tree.h"
#include "codegen.h"
#include "exception.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
    int res = 0;
    driver drv;
    string out_file = "a.out";
    string in_file = "a.ic";
    bool llvm_ir = false;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] == string("-o"))
        {
            if (i == argc - 1)
            {
                std::cerr << "invalid arguments\n";
                return -1;
            }
            out_file = argv[++i];
        }
        else if (argv[i] == string("-l"))
        {
            llvm_ir = true;
        }
        else if (argv[i] == string("-h") || argv[i] == string("--help"))
        {
            std::string help =
                "OVERVIEW: Ice Cola Lang 1.0\n\n"
                "USAGE: "
                "icecola xx.ic [-o xx.o] [-l]\n\n"
                "OPTIONS:\n\t-l: output the llvm ir to a.ll\n";

            std::cout << help;
            return 0;
        }
        else
        {
            in_file = argv[i];
        }
    }

    if (!drv.parse(in_file))
    {
        auto json = to_string(drv.parse_tree);
        if (debug_flag)
        {
            ofstream fout("samples/out.json");
            fout << json;
            fout.close();
        }

        syntax_module module;
        codegen_llvm llvm(module);
        try
        {
            module.syntax_analysis(drv.parse_tree);
            llvm.codegen();
            system(("clang a.ll -Wno-override-module -o " + out_file).c_str());
            if (!llvm_ir)
            {
                system("rm -rf a.ll");
            }
        }
        catch (std::string &s)
        {
            std::cerr << "internel error: " << s << std::endl;
        }
        catch (syntax_error &e)
        {
            std::cerr << e;
        }
        catch (inner_error &e)
        {
            std::cerr << "unhandled inner error: " << e.number << std::endl;
        }
        catch (std::exception &)
        {
            std::cerr << "internel error: std::exception" << std::endl;
        }
    }
    return res;
}
