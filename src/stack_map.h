#pragma once

#include <vector>
#include <map>
#include <string>
#include "exception.h"

template <typename T>
class stack_map
{
    std::vector<std::map<std::string, T>> stack;

public:
    void push()
    {
        stack.push_back(std::map<std::string, T>());
    }

    void pop()
    {
        stack.pop_back();
    }

    T find_or(const std::string &name, const T &t)
    {
        for (auto rit = stack.rbegin(); rit != stack.rend(); rit++)
        {
            auto it = rit->find(name);
            if (it != rit->end())
            {
                return *it;
            }
        }
        return t;
    }

    T find(const std::string &name)
    {
        for (auto rit = stack.rbegin(); rit != stack.rend(); rit++)
        {
            auto it = rit->find(name);
            if (it != rit->end())
            {
                return it->second;
            }
        }
        throw std::string(name);
    }

    void insert(const std::string &name, const T &t)
    {
        stack.back()[name] = t;
    }

    const std::map<std::string, T> &base() const
    {
        return stack.at(0);
    }
};