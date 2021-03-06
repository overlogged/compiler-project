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
extern bool verbose_flag;

std::string myprintf(const char *format, ...);
std::string obj_to_string(vec_str keys, vec_str values);

std::string bin_type(const std::string &s);
std::string number_type(const std::string &s, int base, std::variant<unsigned long long, double, float, std::string> &val);
std::string float_type(const std::string &s, int base, std::variant<unsigned long long, double, float, std::string> &val);
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

inline int hex_to_int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    return -1;
}

inline char trans_code(const std::string &s, int &n)
{
    // '"abfnrtv

    std::string simple_trans_left = "'\"abfnrtv";
    std::string simple_trans_right = "'\"\a\b\f\n\r\t\v";

    if (s[n] == '\\')
    {
        n++;
        for (auto i = 0; i < simple_trans_left.size(); i++)
        {
            auto l = simple_trans_left[i];
            auto r = simple_trans_right[i];
            if (s[n] == l)
            {
                n++;
                return r;
            }
        }
        if (s[n] == 'x')
        {
            n++;
            auto h1 = hex_to_int(s[n]);
            auto h2 = hex_to_int(s[n + 1]);
            if (h1 == -1 || h2 == -1)
            {
                n = -1;
                return 0;
            }
            auto v = h1 * 16 + h2;
            n += 2;
            return (char)v;
        }
        else if (s[n] >= '0' && s[n] <= '7')
        {
            int v = s[n] - '0';
            n++;
            if (s[n] >= '0' && s[n] <= '7')
            {
                v = v * 8 + s[n] - '0';
                n++;
            }
            if (s[n] >= '0' && s[n] <= '7')
            {
                v = v * 8 + s[n] - '0';
                n++;
            }
            return (char)v;
        }
        else
        {
            n = -1;
            return 0;
        }
    }
    else
    {
        return s[n++];
    }
}

inline bool trans_string(const std::string &s, std::string &out)
{
    auto str = s.substr(1, s.size() - 2);
    auto ret = str;
    int n = 0;
    int ret_p = 0;
    while (n < str.size())
    {
        ret[ret_p++] = trans_code(str, n);
        if (n == -1)
        {
            return false;
        }
    }
    out = ret.substr(0, ret_p);
    return true;
}

inline bool trans_char(const std::string &s, char &ch)
{
    int n = 1;
    ch = trans_code(s, n);
    return n != -1;
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

//to_string for 7 types variant
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::string to_string(const std::variant<T1, T2, T3, T4, T5, T6, T7> &node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    auto pval3 = std::get_if<T3>(&node);
    auto pval4 = std::get_if<T4>(&node);
    auto pval5 = std::get_if<T5>(&node);
    auto pval6 = std::get_if<T6>(&node);
    auto pval7 = std::get_if<T7>(&node);
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
    else if (pval7)
        return to_string(*pval7);
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