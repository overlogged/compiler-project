#pragma once

#include <string>
#include <vector>
#include <cassert>

using vec_str = std::vector<std::string>;

std::string myprintf(const char* format,...);
std::string obj_to_string(vec_str keys,vec_str values);
