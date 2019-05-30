#pragma once

#include "location.hh"
#include "utils.h"
#include <string>
#include <iostream>

struct syntax_error
{
    yy::location loc;
    std::string msg;

    syntax_error() = default;

    syntax_error(yy::location location, std::string message = "unknown error") : loc(location), msg(message) {}
};

inline std::ostream &operator<<(std::ostream &out, const syntax_error &err)
{
    out << to_string(err.loc) << " \033[1;31m syntax error: \033[0m" << err.msg << std::endl;
    return out;
}

const int INNER_NOT_INFER_TYPE = 1;
struct inner_error
{
    int number;

    inner_error() : number() {}
    inner_error(int x) : number(x) {}
};