#include "driver.h"
#include "syntax_tree.h"
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
            syntax_analysis(drv.parse_tree);
        }
        else
        {
            res = 1;
        }
    }
    return res;
}
