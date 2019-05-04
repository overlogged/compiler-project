#include "driver.h"
#include <iostream>

int main(int argc, char *argv[])
{
    int res = 0;
    driver drv;
    for (int i = 1; i < argc; ++i)
        if (argv[i] == std::string("-p"))
        {
            drv.trace_parsing = true;
        }
        else if (argv[i] == std::string("-s"))
        {
            drv.trace_scanning = true;
        }
        else if (!drv.parse(argv[i]))
        {
            std::cout<<"mid get_if<node_expalt>: "<<std::get_if<node_exp1alt*>(&(drv.test->val))<<std::endl;
            std::cout<<"mid get_if<node_exp2>: "<<std::get_if<node_exp2*>(&(drv.test->val))<<std::endl;
            std::cout<<"mid node_addr: "<<drv.test<<std::endl;
            auto json = to_string(drv.parse_tree);
            std::cout << json;
        }
        else
        {
            res = 1;
        }
    return res;
}
