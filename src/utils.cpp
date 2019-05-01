#include "utils.h"
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cassert>

std::string myprintf(const char *format, ...)
{
    char *buf = new char[4096 * 16];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    std::string ret(buf);
    delete[] buf;
    return ret;
}

std::string obj_to_string(vec_str keys, vec_str values)
{
    std::string ret = "{";
    assert(keys.size() == values.size());
    for (auto i = 0; i < keys.size(); i++)
    {
        if (i)
        {
            ret.append(",");
        }
        ret.append("\"" + keys[i] + "\"");
        ret.append(" : ");
        ret.append(values[i]);
    }
    ret += "}";
    return ret;
}