#pragma once
#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <exception>
#include "parse_tree.h"
#include "exception.h"
#include <iostream>

struct syntax_type;

struct primary_type
{
    std::string name;
    size_t size;
};

struct product_type
{
    std::vector<std::string> fields;
    std::vector<std::shared_ptr<syntax_type>> types;
    std::vector<size_t> offsets;

    int get_index(const std::string &name)
    {
        for (auto i = 0; i < fields.size(); i++)
        {
            if (fields[i] == name)
            {
                return i;
            }
        }
        return -1;
    }
};

struct sum_type
{
    std::vector<std::string> alters;
    std::vector<std::shared_ptr<syntax_type>> types;
};

struct top_graph
{
    int in_node_num;
    int seq_num;
    std::map<int, std::string> name_table;
    std::map<std::string, int> type_reg_table;
    std::map<int, node_type> arr;
    std::map<int, std::vector<int>> adj_list;
    bool changed;
    top_graph() : seq_num(0), in_node_num(0), changed(false) {}

    void add_node(std::string type_name)
    {
        if (type_reg_table.count(type_name) == 0)
        {
            ++seq_num;
            type_reg_table[type_name] = seq_num;
            name_table[seq_num] = type_name;
            adj_list[seq_num].push_back(in_node_num);
        }
        else
        {
            adj_list[type_reg_table[type_name]].push_back(in_node_num);
        }
    }

    void add_node(std::string type_name, const node_type &node)
    {
        if (type_reg_table.count(type_name) == 0)
        {
            type_reg_table[type_name] = in_node_num;
            name_table[in_node_num] = type_name;
            arr[in_node_num] = node;
        }
        else
        {
            arr[type_reg_table[type_name]] = node;
        }
        ++seq_num;
        in_node_num = seq_num;
    }

    bool contain(std::string type_name)
    {
        return type_reg_table.count(type_name) != 0;
    }

    void set_internal_index(std::string type_name)
    {
        seq_num--;
        in_node_num = type_reg_table[type_name];
    }
    
    void reset_internal_index()
    {
        seq_num++;
        in_node_num = seq_num;
    }
};

struct syntax_type
{
    std::variant<primary_type, product_type, sum_type> type;
    bool is_ref;

    syntax_type() = default;

    bool is_auto()
    {
        return get_primary() == "auto";
    }

    // 失败返回 ""
    std::string get_primary() const
    {
        if (auto p = std::get_if<primary_type>(&type))
            return p->name;
        else
            return "";
    }

    int get_primary_size() const
    {
        if (auto p = std::get_if<primary_type>(&type))
            return p->size;
        else
            return 0;
    }

    bool subtyping(const syntax_type &t) const;
    bool type_equal(const syntax_type &t) const;
};

class type_table
{
public:
    std::map<std::string, syntax_type> user_def_type;

    const syntax_type primary_unit{primary_type{"unit", 1}};

    void add_type(std::string type_name, const syntax_type &t)
    {
        user_def_type[type_name] = t;
    }

    syntax_type get_type(std::string name);
    syntax_type type_check(const node_type &node, top_graph *dependency_graph = nullptr);
};