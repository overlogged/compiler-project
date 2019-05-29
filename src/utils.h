#pragma once

#include <string>
#include <stack>
#include <vector>
#include <cmath>
#include <cassert>
#include <variant>
#include <memory>
#include <sstream>
#include "location.hh"

using vec_str = std::vector<std::string>;

extern bool debug_flag;

std::string myprintf(const char *format, ...);
std::string obj_to_string(vec_str keys, vec_str values);

std::string bin_type(const std::string &s);
std::string number_type(const std::string &s, int base, std::variant<unsigned long long, double, float, long double, char, std::string> &val);
std::string float_type(const std::string &s, int base, std::variant<unsigned long long, double, float, long double, char, std::string> &val);
float cal_float(const std::string &s, int base);
double cal_double(const std::string &s, int base);
long double cal_ldouble(const std::string &s, int base);
unsigned long long bin_to_value(const std::string &s);
unsigned long long number_to_value(const std::string &s, int base);

inline int char_to_integer(char c)
{
    int tmp = 0;
    if (c >= '0' && c <= '9')
        tmp = c - '0';
    else if (c >= 'a' && c <= 'f')
        tmp = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        tmp = c - 'A' + 10;
    else
        assert(false);
    return tmp;
}

inline std::string to_string(const std::string &s)
{
    return "\"" + s + "\"";
}

inline std::string to_string(int x)
{
    return std::to_string(x);
}

inline std::string to_string(long double x)
{
    return std::to_string(x);
}

// to_string for long long
inline std::string to_string(unsigned long long x)
{
    return std::to_string(x);
}

// to_string for double
inline std::string to_string(double x)
{
    return std::to_string(x);
}

// to_string for float
inline std::string to_string(float x)
{
    return std::to_string(x);
}

// to_string for bool
inline std::string to_string(bool x)
{
    return (x ? std::string("\"true\"") : std::string("\"false\""));
}

inline bool is_unsigned(const std::string &s)
{
    if (s.find("u") != std::string::npos)
        return true;
    if (s.find("U") != std::string::npos)
        return true;
    return false;
}

inline std::string trim(const std::string &s, char c)
{
    auto pos_front = s.find_first_not_of(c, 0);
    auto pos_end = s.find_last_not_of(c);
    if (pos_front != std::string::npos && pos_end != std::string::npos)
        return s.substr(pos_front, pos_end - pos_front + 1);
    else
        return std::string("");
}

inline char to_char(const std::string &s)
{
    assert(s.size() <= 3);
    return s[1];
}

// to_string for vector
template <typename T>
std::string to_string(const std::vector<T> &arr)
{
    if (arr.empty())
        return "[]";
    std::string ret = "[";
    for (auto i = 0; i < arr.size(); i++)
    {
        if (i)
        {
            ret.append(",");
        }
        auto s = to_string(arr[i]);
        ret.append(s);
    }
    ret.append("]");
    return ret;
}

// to_string for 1 types variant
template <typename T1>
std::string to_string(const std::variant<T1> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    if (pval1)
        return to_string(*pval1);
    else
        assert(false);
}

// to_string for 2 types variant
template <typename T1, typename T2>
std::string to_string(const std::variant<T1, T2> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    if (pval1)
        return to_string(*pval1);
    else if (pval2)
        return to_string(*pval2);
    else
        assert(false);
}

//to_string for 3 types variant
template <typename T1, typename T2, typename T3>
std::string to_string(const std::variant<T1, T2, T3> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    auto pval3 = std::get_if<T3>(&node);
    if (pval1)
        return to_string(*pval1);
    else if (pval2)
        return to_string(*pval2);
    else if (pval3)
        return to_string(*pval3);
    else
        assert(false);
}
//

//to_string for 4 types variant
template <typename T1, typename T2, typename T3, typename T4>
std::string to_string(const std::variant<T1, T2, T3, T4> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    auto pval3 = std::get_if<T3>(&node);
    auto pval4 = std::get_if<T4>(&node);
    if (pval1)
        return to_string(*pval1);
    else if (pval2)
        return to_string(*pval2);
    else if (pval3)
        return to_string(*pval3);
    else if (pval4)
        return to_string(*pval4);
    else
        assert(false);
}

//to_string for 5 types variant
template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::string to_string(const std::variant<T1, T2, T3, T4, T5> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    auto pval3 = std::get_if<T3>(&node);
    auto pval4 = std::get_if<T4>(&node);
    auto pval5 = std::get_if<T5>(&node);
    if (pval1)
        return to_string(*pval1);
    else if (pval2)
        return to_string(*pval2);
    else if (pval3)
        return to_string(*pval3);
    else if (pval4)
        return to_string(*pval4);
    else if (pval5)
        return to_string(*pval5);
    else
        assert(false);
}

//to_string for 6 types variant
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::string to_string(const std::variant<T1, T2, T3, T4, T5, T6> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    auto pval3 = std::get_if<T3>(&node);
    auto pval4 = std::get_if<T4>(&node);
    auto pval5 = std::get_if<T5>(&node);
    auto pval6 = std::get_if<T6>(&node);
    if (pval1)
        return to_string(*pval1);
    else if (pval2)
        return to_string(*pval2);
    else if (pval3)
        return to_string(*pval3);
    else if (pval4)
        return to_string(*pval4);
    else if (pval5)
        return to_string(*pval5);
    else if (pval6)
        return to_string(*pval6);
    else
        assert(false);
}

//to_string for sharad_ptr<>
template <typename T>
std::string to_string(const std::shared_ptr<T> &node)
{
    assert(node.get());
    return to_string(*node.get());
}

template <typename T>
void cal_float(const std::string &s, int base, T &res)
{
    T exp_part = 0.0;
    T exp_base = (base == 10) ? 10.0 : 2.0;
    bool exp_flag = false;
    bool mul = true;
    std::stack<int> fraction;
    bool integer_flag = true;
    for (auto i = 0; i < s.size(); i++)
    {
        if (s[i] == '.')
            integer_flag = false;
        else if (s[i] == 'p' || s[i] == 'e' || s[i] == 'E')
            exp_flag = true;
        else if (s[i] == '+')
            mul = true;
        else if (s[i] == '-')
            mul = false;
        else
        {
            if (exp_flag)
            {
                exp_part *= 10;
                exp_part += char_to_integer(s[i]);
            }
            else if (integer_flag)
            {
                res *= base;
                res += char_to_integer(s[i]);
            }
            else //fraction_part
            {
                fraction.push(char_to_integer(s[i]));
            }
        }
    }
    T fraction_part = 0.0;
    while (!fraction.empty())
    {
        fraction_part += fraction.top();
        fraction_part /= base;
        fraction.pop();
    }
    res += fraction_part;

    if (exp_flag)
    {
        if (mul)
            res *= pow(exp_base, exp_part);
        else
            res /= pow(exp_base, exp_part);
    }
}

inline std::string to_string(const yy::location &loc)
{
    std::stringstream ostr;
    std::string s;
    unsigned end_col = 0 < loc.end.column ? loc.end.column - 1 : 0;

    ostr << "\033[1m";
    if (loc.begin.filename)
    {
        ostr << *loc.begin.filename << ':';
    }
    ostr << loc.begin.line << ":" << loc.begin.column << ": \033[0m";

    ostr >> s;
    return s;
}