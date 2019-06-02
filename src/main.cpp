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
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] == string("-p"))
        {
            drv.trace_parsing = true;
        }
        else if (argv[i] == string("-s"))
        {
            drv.trace_scanning = true;
        }
        else if (!drv.parse(argv[i]))
        {
            auto json = to_string(drv.parse_tree);
            ofstream fout("samples/out.json");
            fout << json;
            fout.close();
            syntax_module module;
            codegen_llvm llvm(module);
            try
            {
                module.syntax_analysis(drv.parse_tree);
                llvm.codegen();
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
        else
        {
            res = 1;
        }
    }
    return res;
}
