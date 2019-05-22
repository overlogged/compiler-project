#pragma once
#include <vector>
#include <string>
#include <memory>
#include <variant>

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
    size_t size;
};

struct sum_type
{
    std::vector<std::string> alters;
    std::vector<std::shared_ptr<syntax_type>> types;
    size_t size; // include 4 bytes tag
};

struct syntax_type
{
    std::variant<primary_type, product_type, sum_type> type;
};