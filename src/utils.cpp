#include "utils.h"
#include <iostream>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cassert>

bool debug_flag = true;
bool verbose_flag = false;

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

std::string bin_type(const std::string &s)
{
    auto pos = s.find('1');
    if (pos == std::string::npos)
        pos = 0;

    auto bitwidth = s.size() - pos;
    if (bitwidth <= 8)
        return "u8";
    else if (bitwidth <= 16)
        return "u16";
    else if (bitwidth <= 32)
        return "u32";
    else if (bitwidth <= 64)
        return "u64";
    else
        assert(false);
}

unsigned long long bin_to_value(const std::string &s)
{
    unsigned long long tmp = 0;
    std::string bin_seq = s.substr(2);
    for (auto it : bin_seq)
    {
        tmp *= 2;
        tmp += (it - '0');
    }

    return tmp;
}

std::string number_type(const std::string &s, int base, std::variant<unsigned long long, double, float, long double, char, std::string> &val)
{
    auto pos = s.find_last_not_of("uUlL");
    auto suffix = s.substr(pos + 1);
    bool unsigned_flag = is_unsigned(suffix);
    assert(pos != std::string::npos);
    auto value = number_to_value(s, base);

    val = value;

    if (unsigned_flag)
    {
        if (value <= 255)
        {
            if (suffix == "ul" || suffix == "uL" || suffix == "UL" ||
                suffix == "Ul" || suffix == "lu" || suffix == "Lu" ||
                suffix == "LU" || suffix == "lU")
                return "u32";
            else if (suffix == "ull" || suffix == "uLL" || suffix == "ULL" ||
                     suffix == "Ull" || suffix == "llu" || suffix == "LLu" ||
                     suffix == "LLU" || suffix == "llU")
                return "u64";
            else
                return "u8";
        }
        else if (value <= 65535)
        {
            if (suffix == "ul" || suffix == "uL" || suffix == "UL" ||
                suffix == "Ul" || suffix == "lu" || suffix == "Lu" ||
                suffix == "LU" || suffix == "lU")
                return "u32";
            else if (suffix == "ull" || suffix == "uLL" || suffix == "ULL" ||
                     suffix == "Ull" || suffix == "llu" || suffix == "LLu" ||
                     suffix == "LLU" || suffix == "llU")
                return "u64";
            else
                return "u16";
        }
        else if (value <= 4294967295UL)
        {
            if (suffix == "ull" || suffix == "uLL" || suffix == "ULL" ||
                suffix == "Ull" || suffix == "llu" || suffix == "LLu" ||
                suffix == "LLU" || suffix == "llU")
                return "u64";
            else
                return "u32";
        }
        else if (value <= 18446744073709551615ULL)
        {
            return "u64";
        }
        else
            assert(false);
    }
    else
    {
        if (value <= 127)
        {
            if (suffix == "l" || suffix == "L")
                return "i32";
            else if (suffix == "ll" || suffix == "LL")
                return "i64";
            else
                return "u7";
        }
        else if (value <= 255)
        {
            if (suffix == "l" || suffix == "L")
                return "i32";
            else if (suffix == "ll" || suffix == "LL")
                return "i64";
            else
                return "u8";
        }
        else if (value <= 32767)
        {
            if (suffix == "l" || suffix == "L")
                return "i32";
            else if (suffix == "ll" || suffix == "LL")
                return "i64";
            else
                return "u15";
        }
        else if (value <= 65535)
        {
            if (suffix == "l" || suffix == "L")
                return "i32";
            else if (suffix == "ll" || suffix == "LL")
                return "i64";
            else
                return "u16";
        }
        else if (value <= 2147483647UL)
        {
            if (suffix == "ll" || suffix == "LL")
                return "i64";
            else
                return "u31";
        }
        else if (value <= 4294967295UL)
        {
            if (suffix == "ll" || suffix == "LL")
                return "i64";
            else
                return "u32";
        }
        else if (value <= 9223372036854775807ULL)
            return "u63";
        else if (value <= 18446744073709551615ULL)
            return "u64";
        else
            assert(false);
    }
}

unsigned long long number_to_value(const std::string &s, int base)
{
    unsigned long long tmp = 0;
    unsigned long long pre = 0;
    auto pos = s.find_last_not_of("uUlL");
    assert(pos != std::string::npos);
    for (auto i = 0; i <= pos; i++)
    {
        pre = tmp;
        tmp *= base;
        tmp += char_to_integer(s[i]);
        //overflow sholud generate error
        assert(tmp >= pre);
    }
    return tmp;
}

std::string float_type(const std::string &s, int base, std::variant<unsigned long long, double, float, long double, char, std::string> &val)
{
    auto pos = s.find_last_of("fFlL");
    if (pos != std::string::npos)
    {
        auto data = s.substr(0, pos);
        if (s[pos] == 'f' || s[pos] == 'F')
        {
            float tmp = 0.0f;
            cal_float(data, base, tmp);
            val = tmp;
            return "f32";
        }
        else if (s[pos] == 'l' || s[pos] == 'L')
        {
            long double tmp = 0.0l;
            cal_float(data, base, tmp);
            val = tmp;
            return "f128";
        }
        else
            assert(false);
    }
    else
    {
        double tmp = 0.0;
        cal_float(s, base, tmp);
        val = tmp;
        return "f64";
    }
}