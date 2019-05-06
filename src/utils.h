#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <variant>

using vec_str = std::vector<std::string>;

std::string myprintf(const char *format, ...);
std::string obj_to_string(vec_str keys, vec_str values);

inline std::string to_string(const std::string &s)
{
    return "\"" + s + "\"";
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

//to_string for sharad_ptr<>
template <typename T>
std::string to_string(const std::shared_ptr<T> &node)
{
    assert(node.get());
    return to_string(*node.get());
}